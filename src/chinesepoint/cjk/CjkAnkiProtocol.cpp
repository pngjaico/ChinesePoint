#include "chinesepoint/cjk/CjkAnkiProtocol.h"

#include <charconv>
#include <cctype>
#include <system_error>

namespace ChinesePoint::Cjk {
namespace {

bool printableAscii(const std::string_view value, const size_t maximum) {
  if (value.empty() || value.size() > maximum) return false;
  for (const unsigned char character : value) {
    if (character < 0x21u || character > 0x7eu) return false;
  }
  return true;
}

bool privateIpv4(const std::string_view host) {
  unsigned long octets[4]{};
  size_t position = 0;
  for (size_t index = 0; index < 4; ++index) {
    const size_t end = host.find(index == 3 ? '\0' : '.', position);
    const size_t actualEnd = end == std::string_view::npos ? host.size() : end;
    if (actualEnd == position || actualEnd - position > 3) return false;
    const auto result = std::from_chars(host.data() + position, host.data() + actualEnd, octets[index]);
    if (result.ec != std::errc{} || result.ptr != host.data() + actualEnd || octets[index] > 255) return false;
    if (index < 3 && (end == std::string_view::npos || end != actualEnd)) return false;
    position = actualEnd + 1;
  }
  if (position != host.size() + 1) return false;
  return octets[0] == 10 || (octets[0] == 192 && octets[1] == 168) ||
         (octets[0] == 172 && octets[1] >= 16 && octets[1] <= 31);
}

bool localName(const std::string_view host) {
  constexpr std::string_view suffix = ".local";
  return host.size() > suffix.size() && host.ends_with(suffix);
}

}  // namespace

bool parseAnkiBridgeUrl(const std::string_view value, AnkiBridgeUrl& output) {
  constexpr std::string_view scheme = "http://";
  if (!value.starts_with(scheme) || value.size() > 192) return false;
  const std::string_view remainder = value.substr(scheme.size());
  const size_t pathStart = remainder.find('/');
  const std::string_view authority = remainder.substr(0, pathStart);
  const std::string_view path = pathStart == std::string_view::npos ? "/v1/cjk/vocabulary" : remainder.substr(pathStart);
  if (authority.empty() || authority.find('@') != std::string_view::npos || path.empty() || path.size() > 96 ||
      path.find('\r') != std::string_view::npos || path.find('\n') != std::string_view::npos) {
    return false;
  }

  const size_t colon = authority.rfind(':');
  const std::string_view host = colon == std::string_view::npos ? authority : authority.substr(0, colon);
  uint16_t port = 5051;
  if (colon != std::string_view::npos) {
    unsigned long parsed = 0;
    const std::string_view portText = authority.substr(colon + 1);
    const auto result = std::from_chars(portText.data(), portText.data() + portText.size(), parsed);
    if (portText.empty() || result.ec != std::errc{} || result.ptr != portText.data() + portText.size() || parsed == 0 ||
        parsed > 65535) {
      return false;
    }
    port = static_cast<uint16_t>(parsed);
  }
  if (host.size() > 63 || !(privateIpv4(host) || localName(host))) return false;
  for (const unsigned char character : host) {
    if (!(std::isalnum(character) || character == '.' || character == '-')) return false;
  }
  output = {.host = std::string(host), .path = std::string(path), .port = port};
  return true;
}

bool validAnkiBridgeToken(const std::string_view value) { return printableAscii(value, 128); }

bool validAnkiBridgeClientId(const std::string_view value) {
  if (value.size() != 32) return false;
  for (const unsigned char character : value) {
    if (!std::isxdigit(character)) return false;
  }
  return true;
}

std::string ankiBridgeBatchId(const std::string_view clientId, const uint32_t journalSequence,
                              const size_t vocabularyCount) {
  if (!validAnkiBridgeClientId(clientId)) return {};
  return "cp-v1-" + std::string(clientId) + "-" + std::to_string(journalSequence) + "-" +
         std::to_string(vocabularyCount);
}

}  // namespace ChinesePoint::Cjk
