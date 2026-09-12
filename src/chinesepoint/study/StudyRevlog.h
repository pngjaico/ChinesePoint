#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "chinesepoint/study/StudyTypes.h"

namespace ChinesePoint::Study {

constexpr size_t kReviewEntryBytes = 64;
constexpr size_t kReviewEntryIdBytes = 16;


enum class ReviewDecodeStatus : uint8_t {
  Ok,
  BadArgument,
  BadRating,
  BadState,
  UnsupportedFlags,
  NonFiniteValue,
  BadChecksum,
};

// Wire representation only. The study store owns file framing and append/flush
// discipline; this codec makes every on-disk record independently checkable.
struct ReviewEntry {
  std::array<uint8_t, kReviewEntryIdBytes> id{};
  int64_t cardId = 0;
  int64_t reviewedAtMs = 0;
  uint8_t rating = 0;
  CardState previousState = CardState::New;
  CardState nextState = CardState::New;
  uint8_t flags = 0;
  float previousStability = 0.0f;
  float nextStability = 0.0f;
  float previousDifficulty = 0.0f;
  float nextDifficulty = 0.0f;
  int32_t intervalDays = 0;
  uint16_t elapsedDays = 0;
  uint16_t durationMs = 0;
};

bool encodeReviewEntry(const ReviewEntry& entry, std::array<uint8_t, kReviewEntryBytes>& output);
ReviewDecodeStatus decodeReviewEntry(const uint8_t* input, size_t size, ReviewEntry& output);

}  // namespace ChinesePoint::Study
