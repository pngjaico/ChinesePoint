#pragma once

#include <cstddef>
#include <cstdint>

#include "chinesepoint/cjk/CjkSentenceSelection.h"

namespace ChinesePoint::Cjk {

// The candidate buffer is deliberately fixed: dictionary lookup happens while
// the reader page and its font caches are resident, so a CJK fallback must not
// add dynamic allocation pressure to that path.
constexpr size_t kMaxLookupCandidateBytes = 64;
constexpr uint16_t kMaxLookupCandidateCodepoints = 8;
constexpr size_t kMaxLookupCandidates = 12;

struct LookupCandidate {
  char text[kMaxLookupCandidateBytes + 1] = {};
  size_t bytes = 0;
  uint16_t codepoints = 0;
};

// Builds distinct, longest-first contiguous CJK phrases around the selected
// token. The selected token itself is intentionally excluded because the
// caller always tries the normal StarDict lookup first. A malformed UTF-8
// token, punctuation, Latin text, or an overlong phrase is never emitted.
size_t buildCjkLookupCandidates(const SelectableToken* tokens, size_t tokenCount, size_t selectedTokenIndex,
                                LookupCandidate* output, size_t outputCapacity);

}  // namespace ChinesePoint::Cjk
