#include "chinesepoint/cjk/CcCedictInstaller.h"

#include <HalStorage.h>
#include <ZipFile.h>
#include <esp_rom_crc.h>
#include <mbedtls/sha256.h>

#include <array>
#include <cstring>
#include <string>
#include <string_view>

#include "network/HttpDownloader.h"

namespace ChinesePoint::Cjk {
namespace {

constexpr const char* kUrl =
    "https://github.com/scillidan/share_cc-cedict/releases/download/20260731/CC-CEDICT-20260731-stardict-mergesyns.zip";
constexpr const char* kZipPath = "/.chinesepoint/downloads/cc-cedict-20260731.zip";
constexpr const char* kStageDir = "/.dictionaries/.cc-cedict-20260731.stage";
constexpr const char* kTargetDir = "/dictionaries/cc-cedict-20260731";
constexpr const char* kStem = "CC-CEDICT-20260731-stardict-mergesyns";
constexpr std::array<uint8_t, 32> kZipSha256 = {0x85, 0x84, 0x9e, 0xfd, 0xb8, 0xf4, 0x6a, 0x7b,
                                                  0x6f, 0xa4, 0x22, 0x2a, 0x4d, 0xc1, 0x74, 0x81,
                                                  0xad, 0xd7, 0xd1, 0x1e, 0x15, 0x6b, 0xba, 0xd9,
                                                  0x35, 0xbd, 0x27, 0x81, 0xf9, 0x14, 0x37, 0x7b};

struct RequiredFile { const char* suffix; size_t bytes; uint32_t crc; };
constexpr RequiredFile kFiles[] = {{".ifo", 128, 0x5a12315b}, {".dict", 10506165, 0x8cd3e046},
                                   {".idx", 11258318, 0x3748c263}};

std::string archiveName(const RequiredFile& file) { return std::string(kStem) + file.suffix; }
std::string stagedPath(const RequiredFile& file) { return std::string(kStageDir) + "/" + kStem + file.suffix; }

void removeStage() {
  for (const auto& file : kFiles) Storage.remove(stagedPath(file).c_str());
  Storage.rmdir(kStageDir);
}

bool hasExpectedFile(const char* directory, const RequiredFile& expected) {
  const std::string path = std::string(directory) + "/" + kStem + expected.suffix;
  HalFile file;
  if (!Storage.openFileForRead("CJK", path, file) || file.fileSize() != expected.bytes) return false;
  uint8_t buffer[1024];
  uint32_t crc = 0;
  bool ok = true;
  while (file.available()) {
    const int count = file.read(buffer, sizeof(buffer));
    if (count <= 0) {
      ok = false;
      break;
    }
    crc = esp_rom_crc32_le(crc, buffer, static_cast<uint32_t>(count));
  }
  return file.close() && ok && crc == expected.crc;
}

bool targetIsComplete() {
  if (!Storage.exists(kTargetDir)) return false;
  for (const auto& file : kFiles) {
    if (!hasExpectedFile(kTargetDir, file)) return false;
  }
  return true;
}

bool verifySha256(const char* path) {
  HalFile file;
  if (!Storage.openFileForRead("CJK", path, file)) return false;
  mbedtls_sha256_context context;
  mbedtls_sha256_init(&context);
  mbedtls_sha256_starts(&context, 0);
  uint8_t buffer[1024];
  bool ok = true;
  while (file.available()) {
    const int count = file.read(buffer, sizeof(buffer));
    if (count <= 0) { ok = false; break; }
    mbedtls_sha256_update(&context, buffer, static_cast<size_t>(count));
  }
  uint8_t digest[32]{};
  if (ok) mbedtls_sha256_finish(&context, digest);
  mbedtls_sha256_free(&context);
  return file.close() && ok && std::memcmp(digest, kZipSha256.data(), sizeof(digest)) == 0;
}

class CrcFileWriter final : public Print {
 public:
  CrcFileWriter(HalFile& file, const CcCedictInstaller::ProgressCallback& progress, bool* cancel, size_t total)
      : file(file), progress(progress), cancel(cancel), total(total) {}
  size_t write(const uint8_t* bytes, size_t count) override {
    if ((cancel && *cancel) || file.write(bytes, count) != count) return 0;
    crc = esp_rom_crc32_le(crc, bytes, static_cast<uint32_t>(count));
    written += count;
    if (progress) progress(written, total);
    return count;
  }
  size_t write(uint8_t byte) override { return write(&byte, 1); }
  uint32_t crc = 0;
  size_t written = 0;
 private:
  HalFile& file;
  const CcCedictInstaller::ProgressCallback& progress;
  bool* cancel;
  size_t total;
};

}  // namespace

CcCedictInstaller::Result CcCedictInstaller::install(const ProgressCallback& progress, bool* const cancelRequested) const {
  // Never overwrite a pre-existing target automatically. If it is not the
  // known-good package, leave it for manual inspection instead of risking a
  // user's unrelated dictionary data.
  if (Storage.exists(kTargetDir)) return targetIsComplete() ? Result::AlreadyInstalled : Result::StorageFailed;
  if (!Storage.ensureDirectoryExists("/.chinesepoint/downloads") || !Storage.ensureDirectoryExists("/.dictionaries")) {
    return Result::StorageFailed;
  }
  removeStage();
  if (!Storage.mkdir(kStageDir)) return Result::StorageFailed;

  const auto downloaded = HttpDownloader::downloadToFile(kUrl, kZipPath, progress, cancelRequested);
  if (downloaded != HttpDownloader::OK) {
    Storage.remove(kZipPath); removeStage();
    return downloaded == HttpDownloader::ABORTED ? Result::Cancelled : Result::DownloadFailed;
  }
  if (!verifySha256(kZipPath)) { Storage.remove(kZipPath); removeStage(); return Result::ChecksumFailed; }

  // ZipFile stores a reference to this path, so it must outlive every
  // enumerate/extract call. Passing kZipPath directly would create a dangling
  // temporary std::string and intermittently corrupt archive access.
  const std::string zipPath{kZipPath};
  ZipFile zip(zipPath);
  size_t archiveEntries = 0;
  bool expectedOnly = true;
  const bool enumerated = zip.enumerateFileEntries([&](std::string_view path, uint32_t, uint32_t) {
    ++archiveEntries;
    bool known = false;
    for (const auto& file : kFiles) known = known || path == archiveName(file);
    expectedOnly = expectedOnly && known;
  });
  if (!enumerated || !expectedOnly || archiveEntries != sizeof(kFiles) / sizeof(kFiles[0])) {
    Storage.remove(kZipPath); removeStage(); return Result::ArchiveInvalid;
  }

  for (const auto& file : kFiles) {
    if (cancelRequested && *cancelRequested) { Storage.remove(kZipPath); removeStage(); return Result::Cancelled; }
    HalFile output;
    const std::string destination = stagedPath(file);
    if (!Storage.openFileForWrite("CJK", destination, output)) { Storage.remove(kZipPath); removeStage(); return Result::StorageFailed; }
    CrcFileWriter writer(output, progress, cancelRequested, file.bytes);
    const bool extracted = zip.readFileToStream(archiveName(file).c_str(), writer, 1024);
    const bool closed = output.close();
    if (!extracted || !closed || writer.written != file.bytes || writer.crc != file.crc) {
      Storage.remove(kZipPath); removeStage();
      return cancelRequested && *cancelRequested ? Result::Cancelled : Result::ArchiveInvalid;
    }
  }
  Storage.remove(kZipPath);
  if (!Storage.ensureDirectoryExists("/dictionaries") || !Storage.rename(kStageDir, kTargetDir)) { removeStage(); return Result::StorageFailed; }
  return Result::Installed;
}

bool CcCedictInstaller::isInstalled() { return targetIsComplete(); }

}  // namespace ChinesePoint::Cjk
