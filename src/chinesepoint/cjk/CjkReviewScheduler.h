#pragma once

#include <cstdint>

namespace ChinesePoint::Cjk {

// This is learner-domain code only. It has no dependency on CrossPoint boot,
// display, storage, or navigation and therefore cannot affect reader startup.
enum class Rating : uint8_t { Again = 1, Hard = 2, Good = 3, Easy = 4 };
enum class ReviewPhase : uint8_t { New, Learning, Review, Relearning };
enum class ScheduleAuthority : uint8_t { Local, Anki };

struct ReviewState {
  ReviewPhase phase = ReviewPhase::New;
  ScheduleAuthority authority = ScheduleAuthority::Local;
  int64_t dueAtMs = 0;
  int64_t lastReviewAtMs = 0;
  float difficulty = 5.0f;
  float stabilityDays = 0.02f;
  uint32_t reps = 0;
  uint32_t lapses = 0;
  uint64_t ankiCardId = 0;
};

struct ReviewResult {
  ReviewState state;
  int64_t intervalMs = 0;
};

class ReviewScheduler final {
 public:
  bool isDue(const ReviewState& state, int64_t nowMs) const;
  ReviewResult rate(const ReviewState& state, Rating rating, int64_t nowMs) const;
};

}  // namespace ChinesePoint::Cjk
