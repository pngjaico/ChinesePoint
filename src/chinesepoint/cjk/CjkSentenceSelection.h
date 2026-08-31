#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "chinesepoint/cjk/CjkLearnerModel.h"

namespace ChinesePoint::Cjk {

struct SelectableToken {
  std::string_view text;
  uint32_t visibleCodepointOffset = 0;
  uint16_t visibleCodepointLength = 0;
  bool joinWithoutSpaceBefore = false;
};

enum class SentenceCompleteness : uint8_t {
  Complete,
  TruncatedStart,
  TruncatedEnd,
  TruncatedBoth,
};

struct SentenceSelection {
  TextAnchor anchor{};
  uint16_t firstTokenIndex = 0;
  uint16_t lastTokenIndex = 0;
  uint16_t selectedSentenceCodepoint = 0;
  SentenceCompleteness completeness = SentenceCompleteness::Complete;
};

bool isSentenceTerminal(std::string_view token);
uint16_t utf8CodepointCount(std::string_view text);
uint32_t selectionFingerprint(std::string_view text);

// Builds the sentence around `selectedTokenIndex` into caller-owned storage.
// It returns false rather than truncating: a partial sentence must never be
// written to a learner record as if it were complete.
bool buildSentenceSelection(const SelectableToken* tokens, size_t tokenCount, size_t selectedTokenIndex,
                            uint16_t spineIndex, bool startsAtSectionBoundary,
                            bool endsAtSectionBoundary, char* output, size_t outputCapacity,
                            SentenceSelection& selection);

}  // namespace ChinesePoint::Cjk
