#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ChinesePoint::Cjk {

// The Anki bridge intentionally accepts only a local clear-text endpoint.
// The bridge is a companion to Anki Desktop, not a general cloud client: TLS
// on an arbitrary desktop certificate would be less safe than an explicit,
// token-protected private-LAN opt-in.
struct AnkiBridgeUrl {
  std::string host;
  std::string path;
  uint16_t port = 0;
};

bool parseAnkiBridgeUrl(std::string_view value, AnkiBridgeUrl& output);
bool validAnkiBridgeToken(std::string_view value);
bool validAnkiBridgeClientId(std::string_view value);

// A stable batch id makes a timeout safe to retry. The desktop bridge records
// accepted ids, so the same local export can never create duplicate notes.
std::string ankiBridgeBatchId(std::string_view clientId, uint32_t journalSequence, size_t vocabularyCount);

}  // namespace ChinesePoint::Cjk
