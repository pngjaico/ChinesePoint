#include "chinesepoint/cjk/CjkAnkiClient.h"

#include <Arduino.h>
#include <HalStorage.h>
#include <WiFi.h>

#include <cstdio>
#include <cctype>
#include <cstring>
#include <string>
#include <string_view>

#include "chinesepoint/cjk/CjkAnkiProtocol.h"
#include "chinesepoint/cjk/CjkLearnerStore.h"

namespace ChinesePoint::Cjk {
namespace {

constexpr size_t kMaxExportBytes = 2 * 1024 * 1024;
constexpr uint32_t kTimeoutMs = 60 * 1000;

bool writeText(WiFiClient& client, const std::string& value) {
  return client.write(reinterpret_cast<const uint8_t*>(value.data()), value.size()) == value.size();
}

bool readLine(WiFiClient& client, char* output, const size_t capacity, bool* const cancelRequested) {
  size_t length = 0;
  const uint32_t started = millis();
  while (millis() - started < kTimeoutMs) {
    if (cancelRequested != nullptr && *cancelRequested) return false;
    while (client.available()) {
      const int character = client.read();
      if (character < 0) break;
      if (character == '\n') {
        if (length > 0 && output[length - 1] == '\r') --length;
        output[length] = '\0';
        return true;
      }
      if (length + 1 >= capacity) return false;
      output[length++] = static_cast<char>(character);
    }
    if (!client.connected() && !client.available()) return false;
    delay(1);
  }
  return false;
}

bool isExpectedBatchHeader(const char* const line, const std::string_view batchId) {
  constexpr std::string_view prefix = "X-ChinesePoint-Batch:";
  const std::string_view header(line);
  if (header.size() < prefix.size()) return false;
  for (size_t index = 0; index < prefix.size(); ++index) {
    if (std::tolower(static_cast<unsigned char>(header[index])) !=
        std::tolower(static_cast<unsigned char>(prefix[index]))) {
      return false;
    }
  }
  size_t valueStart = prefix.size();
  while (valueStart < header.size() && header[valueStart] == ' ') ++valueStart;
  return header.substr(valueStart) == batchId;
}

}  // namespace

CjkAnkiClient::Result CjkAnkiClient::pushVocabulary(LearnerStore& store, const AnkiBridgeConfig& config,
                                                      const ProgressCallback& progress,
                                                      bool* const cancelRequested) const {
  AnkiBridgeUrl url;
  if (!config.configured() || !parseAnkiBridgeUrl(config.serverUrl, url)) return Result::NotConfigured;
  if (cancelRequested != nullptr && *cancelRequested) return Result::Cancelled;
  if (!store.exportJsonl()) return Result::ExportFailed;

  HalFile exportFile;
  if (!Storage.openFileForRead("CJK", store.exportPath(), exportFile)) return Result::ExportFailed;
  const size_t size = exportFile.fileSize();
  if (size == 0 || size > kMaxExportBytes) {
    exportFile.close();
    return Result::ExportFailed;
  }

  const std::string batchId =
      ankiBridgeBatchId(config.clientId, store.repository().lastSequence(), store.repository().entries().size());
  if (batchId.empty()) {
    exportFile.close();
    return Result::NotConfigured;
  }

  WiFiClient client;
#if !defined(SIMULATOR)
  // NetworkClient in the official desktop simulator intentionally has no
  // timeout setter. The simulator does not perform a real LAN Anki transfer.
  client.setTimeout(kTimeoutMs);
#endif
  if (!client.connect(url.host.c_str(), url.port)) {
    exportFile.close();
    return Result::ConnectionFailed;
  }

  const std::string headers = "POST " + url.path + " HTTP/1.1\r\nHost: " + url.host + ":" +
                              std::to_string(url.port) + "\r\nUser-Agent: ChinesePoint/1\r\nAuthorization: Bearer " +
                              config.apiToken + "\r\nX-ChinesePoint-Client: " + config.clientId +
                              "\r\nX-ChinesePoint-Batch: " + batchId +
                              "\r\nContent-Type: application/x-ndjson\r\nContent-Length: " + std::to_string(size) +
                              "\r\nConnection: close\r\n\r\n";
  if (!writeText(client, headers)) {
    client.stop();
    exportFile.close();
    return Result::ConnectionFailed;
  }

  uint8_t buffer[1024];
  size_t sent = 0;
  while (exportFile.available()) {
    if (cancelRequested != nullptr && *cancelRequested) {
      client.stop();
      exportFile.close();
      return Result::Cancelled;
    }
    const int read = exportFile.read(buffer, sizeof(buffer));
    if (read <= 0 || client.write(buffer, static_cast<size_t>(read)) != static_cast<size_t>(read)) {
      client.stop();
      exportFile.close();
      return Result::ConnectionFailed;
    }
    sent += static_cast<size_t>(read);
    if (progress) progress(sent, size);
  }
  const bool closeOk = exportFile.close();
  if (!closeOk || sent != size) {
    client.stop();
    return Result::ExportFailed;
  }

  char line[256];
  if (!readLine(client, line, sizeof(line), cancelRequested)) {
    client.stop();
    return cancelRequested != nullptr && *cancelRequested ? Result::Cancelled : Result::ProtocolFailed;
  }
  int status = 0;
  if (std::sscanf(line, "HTTP/%*u.%*u %d", &status) != 1) {
    client.stop();
    return Result::ProtocolFailed;
  }
  size_t headerBytes = 0;
  bool batchMatched = false;
  while (true) {
    if (!readLine(client, line, sizeof(line), cancelRequested)) {
      client.stop();
      return cancelRequested != nullptr && *cancelRequested ? Result::Cancelled : Result::ProtocolFailed;
    }
    headerBytes += std::strlen(line) + 2;
    if (headerBytes > 4096) {
      client.stop();
      return Result::ProtocolFailed;
    }
    if (line[0] == '\0') break;
    batchMatched = batchMatched || isExpectedBatchHeader(line, batchId);
  }
  client.stop();
  if (status < 200 || status >= 300) return Result::Rejected;
  // A matching response binds a 2xx success to this exact idempotent batch.
  // Without it, a nearby HTTP service could be mistaken for the Anki bridge.
  return batchMatched ? Result::Ok : Result::ProtocolFailed;
}

}  // namespace ChinesePoint::Cjk
