#include "chinesepoint/cjk/CjkLookupCandidates.h"

#include <cstring>

namespace ChinesePoint::Cjk {
namespace {

bool decodeUtf8(const std::string_view text, size_t& offset, uint32_t& codepoint) {
  if (offset >= text.size()) return false;
  const auto first = static_cast<uint8_t>(text[offset]);
  if (first < 0x80) {
    codepoint = first;
    offset++;
    return true;
  }
  size_t length = 0;
  uint32_t value = 0;
  if ((first & 0xE0u) == 0xC0u) {
    length = 2;
    value = first & 0x1Fu;
  } else if ((first & 0xF0u) == 0xE0u) {
    length = 3;
    value = first & 0x0Fu;
  } else if ((first & 0xF8u) == 0xF0u) {
    length = 4;
    value = first & 0x07u;
  } else {
    return false;
  }
  if (offset + length > text.size()) return false;
  for (size_t index = 1; index < length; ++index) {
    const auto byte = static_cast<uint8_t>(text[offset + index]);
    if ((byte & 0xC0u) != 0x80u) return false;
    value = (value << 6u) | (byte & 0x3Fu);
  }
  static constexpr uint32_t kMinimumByLength[] = {0, 0, 0x80, 0x800, 0x10000};
  if (value < kMinimumByLength[length] || value > 0x10FFFF || (value >= 0xD800 && value <= 0xDFFF)) return false;
  codepoint = value;
  offset += length;
  return true;
}

bool isCjkCodepoint(const uint32_t codepoint) {
  return (codepoint >= 0x3400 && codepoint <= 0x4DBF) || (codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||
         (codepoint >= 0xF900 && codepoint <= 0xFAFF);
}

bool isCjkToken(const std::string_view text, uint16_t& codepoints) {
  codepoints = 0;
  size_t offset = 0;
  while (offset < text.size()) {
    uint32_t codepoint = 0;
    if (!decodeUtf8(text, offset, codepoint) || !isCjkCodepoint(codepoint) || codepoints == UINT16_MAX) return false;
    ++codepoints;
  }
  return codepoints > 0;
}

bool appendToken(const std::string_view text, char* const destination, size_t& bytes, uint16_t& codepoints) {
  uint16_t tokenCodepoints = 0;
  if (!isCjkToken(text, tokenCodepoints) || bytes + text.size() > kMaxLookupCandidateBytes ||
      codepoints > kMaxLookupCandidateCodepoints - tokenCodepoints) {
    return false;
  }
  memcpy(destination + bytes, text.data(), text.size());
  bytes += text.size();
  codepoints = static_cast<uint16_t>(codepoints + tokenCodepoints);
  return true;
}

void insertCandidate(const LookupCandidate& candidate, LookupCandidate* const output, const size_t capacity,
                     size_t& count) {
  for (size_t index = 0; index < count; ++index) {
    if (output[index].bytes == candidate.bytes &&
        memcmp(output[index].text, candidate.text, candidate.bytes) == 0) {
      return;
    }
  }
  size_t insertAt = count;
  while (insertAt > 0 && output[insertAt - 1].codepoints < candidate.codepoints) --insertAt;
  if (count < capacity) ++count;
  if (insertAt >= count) return;
  for (size_t index = count - 1; index > insertAt; --index) output[index] = output[index - 1];
  output[insertAt] = candidate;
}

}  // namespace

size_t buildCjkLookupCandidates(const SelectableToken* const tokens, const size_t tokenCount,
                                const size_t selectedTokenIndex, LookupCandidate* const output,
                                const size_t outputCapacity) {
  if (tokens == nullptr || output == nullptr || outputCapacity == 0 || tokenCount == 0 ||
      selectedTokenIndex >= tokenCount) {
    return 0;
  }

  uint16_t selectedCodepoints = 0;
  if (!isCjkToken(tokens[selectedTokenIndex].text, selectedCodepoints)) return 0;

  size_t runStart = selectedTokenIndex;
  while (runStart > 0) {
    uint16_t ignored = 0;
    if (!isCjkToken(tokens[runStart - 1].text, ignored)) break;
    --runStart;
  }
  size_t runEnd = selectedTokenIndex;
  while (runEnd + 1 < tokenCount) {
    uint16_t ignored = 0;
    if (!isCjkToken(tokens[runEnd + 1].text, ignored)) break;
    ++runEnd;
  }

  size_t count = 0;
  for (size_t first = runStart; first <= selectedTokenIndex; ++first) {
    for (size_t last = selectedTokenIndex; last <= runEnd; ++last) {
      if (first == selectedTokenIndex && last == selectedTokenIndex) continue;
      LookupCandidate candidate;
      bool valid = true;
      for (size_t index = first; index <= last; ++index) {
        if (!appendToken(tokens[index].text, candidate.text, candidate.bytes, candidate.codepoints)) {
          valid = false;
          break;
        }
      }
      if (!valid) continue;
      candidate.text[candidate.bytes] = '\0';
      insertCandidate(candidate, output, outputCapacity, count);
    }
  }
  return count;
}

}  // namespace ChinesePoint::Cjk
