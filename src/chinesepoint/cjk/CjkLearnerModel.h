#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "chinesepoint/cjk/CjkReviewScheduler.h"

namespace ChinesePoint::Cjk {

constexpr size_t kMaxHeadwordBytes = 64;
constexpr size_t kMaxSentenceBytes = 768;
constexpr size_t kMaxBookPathBytes = 240;

enum class WordStatus : uint8_t { Encountered, Saved, Learning, Known };

struct TextAnchor {
  uint16_t spineIndex = 0;
  uint32_t visibleCodepointOffset = 0;
  uint16_t codepointLength = 0;
  uint32_t fingerprint = 0;
};

constexpr uint64_t stableWordId(const std::string_view text) {
  uint64_t hash = 0xcbf29ce484222325ULL;
  for (const unsigned char character : text) {
    hash ^= character;
    hash *= 0x100000001b3ULL;
  }
  return hash;
}

constexpr bool validHeadword(const std::string_view value) {
  return !value.empty() && value.size() <= kMaxHeadwordBytes;
}
constexpr bool validSentence(const std::string_view value) { return value.size() <= kMaxSentenceBytes; }
constexpr bool validBookPath(const std::string_view value) { return value.size() <= kMaxBookPathBytes; }

struct LearnerEntry {
  uint64_t wordId = 0;
  std::string headword;
  WordStatus status = WordStatus::Encountered;
  uint32_t encounterCount = 0;
  int64_t firstSeenStudyMs = 0;
  int64_t lastSeenStudyMs = 0;
  std::string bookPath;
  TextAnchor sourceAnchor{};
  std::string sourceSentence;
  ReviewState review{};
};

}  // namespace ChinesePoint::Cjk
