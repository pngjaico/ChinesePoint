#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

// Finds the longest UTF-8 prefix whose measured width fits maxWidth.  The
// caller supplies the renderer-specific measurement, and this helper only
// manages byte boundaries.  It temporarily writes a terminator into the
// already-owned string instead of allocating a prefix for every probe.
namespace Utf8LineBreaker {

inline bool isContinuationByte(const char value) {
  return (static_cast<uint8_t>(value) & 0xC0U) == 0x80U;
}

inline size_t previousBoundary(const std::string& text, size_t offset) {
  if (offset >= text.size()) return text.size();
  while (offset > 0 && isContinuationByte(text[offset])) --offset;
  return offset;
}

inline size_t nextBoundary(const std::string& text, size_t offset) {
  if (offset >= text.size()) return text.size();
  ++offset;
  while (offset < text.size() && isContinuationByte(text[offset])) ++offset;
  return offset;
}

template <typename Measure>
size_t longestPrefixWithinWidth(std::string& text, const int maxWidth, Measure measure) {
  if (text.empty() || maxWidth < 0) return 0;

  size_t low = 0;
  size_t high = text.size();
  while (low < high) {
    const size_t midpoint = low + (high - low + 1) / 2;
    size_t candidate = previousBoundary(text, midpoint);
    if (candidate <= low) candidate = nextBoundary(text, low);
    if (candidate == 0) break;

    int width = 0;
    if (candidate == text.size()) {
      width = measure(text.c_str());
    } else {
      const char saved = text[candidate];
      text[candidate] = '\0';
      width = measure(text.c_str());
      text[candidate] = saved;
    }

    if (width <= maxWidth) {
      low = candidate;
    } else {
      high = previousBoundary(text, candidate - 1);
    }
  }
  return low;
}

}  // namespace Utf8LineBreaker
