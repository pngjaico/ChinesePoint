#include "chinesepoint/cjk/CjkSentenceSelection.h"

#include <cstring>
#include <limits>

namespace ChinesePoint::Cjk {
namespace {

SentenceCompleteness completenessFor(bool completeStart, bool completeEnd) {
  if (completeStart && completeEnd) return SentenceCompleteness::Complete;
  if (!completeStart && !completeEnd) return SentenceCompleteness::TruncatedBoth;
  return completeStart ? SentenceCompleteness::TruncatedEnd : SentenceCompleteness::TruncatedStart;
}

}  // namespace

uint16_t utf8CodepointCount(const std::string_view text) {
  uint32_t count = 0;
  for (const unsigned char ch : text) {
    if ((ch & 0xC0u) != 0x80u) ++count;
  }
  return count > std::numeric_limits<uint16_t>::max() ? std::numeric_limits<uint16_t>::max()
                                                        : static_cast<uint16_t>(count);
}

uint32_t selectionFingerprint(const std::string_view text) {
  uint32_t hash = 0x811C9DC5u;
  for (const unsigned char ch : text) {
    hash ^= ch;
    hash *= 0x01000193u;
  }
  return hash;
}

bool isSentenceTerminal(const std::string_view token) {
  if (token.empty()) return false;
  const char last = token.back();
  if (last == '.' || last == '!' || last == '?') return true;
  constexpr std::string_view terminals[] = {"。", "！", "？"};
  for (const auto terminal : terminals) {
    if (token.size() >= terminal.size() && token.compare(token.size() - terminal.size(), terminal.size(), terminal) == 0) {
      return true;
    }
  }
  return false;
}

bool buildSentenceSelection(const SelectableToken* tokens, const size_t tokenCount, const size_t selectedTokenIndex,
                            const uint16_t spineIndex, const bool startsAtSectionBoundary,
                            const bool endsAtSectionBoundary, char* output, const size_t outputCapacity,
                            SentenceSelection& selection) {
  selection = {};
  if (output != nullptr && outputCapacity > 0) output[0] = '\0';
  if (tokens == nullptr || tokenCount == 0 || tokenCount > std::numeric_limits<uint16_t>::max() ||
      selectedTokenIndex >= tokenCount || output == nullptr || outputCapacity == 0) {
    return false;
  }

  size_t first = selectedTokenIndex;
  bool completeStart = false;
  while (first > 0) {
    if (isSentenceTerminal(tokens[first - 1].text)) {
      completeStart = true;
      break;
    }
    --first;
  }
  if (first == 0) completeStart = startsAtSectionBoundary;

  size_t last = selectedTokenIndex;
  bool completeEnd = false;
  for (; last < tokenCount; ++last) {
    if (isSentenceTerminal(tokens[last].text)) {
      completeEnd = true;
      break;
    }
  }
  if (last == tokenCount) {
    last = tokenCount - 1;
    completeEnd = endsAtSectionBoundary;
  }

  size_t requiredBytes = 1;
  uint32_t requiredCodepoints = 0;
  uint32_t selectedCodepoint = 0;
  bool emitted = false;
  for (size_t index = first; index <= last; ++index) {
    const auto& token = tokens[index];
    const bool addSpace = emitted && !token.joinWithoutSpaceBefore;
    if (index == selectedTokenIndex) selectedCodepoint = requiredCodepoints + (addSpace ? 1u : 0u);
    requiredBytes += token.text.size() + (addSpace ? 1u : 0u);
    requiredCodepoints += (token.visibleCodepointLength > 0 ? token.visibleCodepointLength : utf8CodepointCount(token.text)) +
                          (addSpace ? 1u : 0u);
    emitted = emitted || !token.text.empty();
  }
  if (requiredBytes > outputCapacity || selectedCodepoint > std::numeric_limits<uint16_t>::max()) return false;

  size_t position = 0;
  emitted = false;
  for (size_t index = first; index <= last; ++index) {
    const auto& token = tokens[index];
    if (emitted && !token.joinWithoutSpaceBefore) output[position++] = ' ';
    if (!token.text.empty()) {
      std::memcpy(output + position, token.text.data(), token.text.size());
      position += token.text.size();
      emitted = true;
    }
  }
  output[position] = '\0';

  const auto& selected = tokens[selectedTokenIndex];
  selection.anchor = {spineIndex, selected.visibleCodepointOffset,
                      selected.visibleCodepointLength > 0 ? selected.visibleCodepointLength : utf8CodepointCount(selected.text),
                      selectionFingerprint(selected.text)};
  selection.firstTokenIndex = static_cast<uint16_t>(first);
  selection.lastTokenIndex = static_cast<uint16_t>(last);
  selection.selectedSentenceCodepoint = static_cast<uint16_t>(selectedCodepoint);
  selection.completeness = completenessFor(completeStart, completeEnd);
  return true;
}

}  // namespace ChinesePoint::Cjk
