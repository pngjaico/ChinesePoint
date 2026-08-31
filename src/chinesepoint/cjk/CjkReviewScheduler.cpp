#include "chinesepoint/cjk/CjkReviewScheduler.h"

#include <algorithm>
#include <cmath>

namespace ChinesePoint::Cjk {
namespace {
constexpr int64_t kMinuteMs = 60LL * 1000;
constexpr int64_t kHourMs = 60LL * kMinuteMs;
constexpr int64_t kDayMs = 24LL * kHourMs;
constexpr float kMaxStabilityDays = 3650.0f;

float clampDifficulty(float value) { return std::clamp(value, 1.0f, 10.0f); }
float clampStability(float value) { return std::clamp(value, 1.0f / 144.0f, kMaxStabilityDays); }
int64_t daysToMs(float days) {
  return static_cast<int64_t>(std::llround(std::clamp<double>(days, 0.0, kMaxStabilityDays) * kDayMs));
}
}  // namespace

bool ReviewScheduler::isDue(const ReviewState& state, const int64_t nowMs) const {
  return state.authority == ScheduleAuthority::Local && state.dueAtMs <= nowMs;
}

ReviewResult ReviewScheduler::rate(const ReviewState& input, const Rating rating, const int64_t nowMs) const {
  ReviewResult result{input};
  result.state.authority = ScheduleAuthority::Local;
  result.state.lastReviewAtMs = nowMs;
  result.state.reps = input.reps + 1;
  const auto setInterval = [&](int64_t interval) {
    result.intervalMs = std::max(kMinuteMs, interval);
    result.state.dueAtMs = nowMs + result.intervalMs;
  };

  if (input.phase == ReviewPhase::New) {
    switch (rating) {
      case Rating::Again: result.state.phase = ReviewPhase::Learning; result.state.lapses++; result.state.difficulty = clampDifficulty(input.difficulty + .8f); result.state.stabilityDays = clampStability(1.0f / 144.0f); setInterval(kMinuteMs); break;
      case Rating::Hard: result.state.phase = ReviewPhase::Learning; result.state.difficulty = clampDifficulty(input.difficulty + .3f); result.state.stabilityDays = clampStability(6.0f / 1440.0f); setInterval(6 * kMinuteMs); break;
      case Rating::Good: result.state.phase = ReviewPhase::Review; result.state.difficulty = clampDifficulty(input.difficulty - .1f); result.state.stabilityDays = 1.0f; setInterval(kDayMs); break;
      case Rating::Easy: result.state.phase = ReviewPhase::Review; result.state.difficulty = clampDifficulty(input.difficulty - .5f); result.state.stabilityDays = 4.0f; setInterval(4 * kDayMs); break;
    }
    return result;
  }

  if (input.phase == ReviewPhase::Learning || input.phase == ReviewPhase::Relearning) {
    switch (rating) {
      case Rating::Again: result.state.phase = input.phase; result.state.lapses++; result.state.difficulty = clampDifficulty(input.difficulty + .8f); result.state.stabilityDays = clampStability(std::min(input.stabilityDays, 5.0f / 1440.0f)); setInterval(5 * kMinuteMs); break;
      case Rating::Hard: result.state.phase = input.phase; result.state.difficulty = clampDifficulty(input.difficulty + .3f); result.state.stabilityDays = clampStability(.5f); setInterval(12 * kHourMs); break;
      case Rating::Good: result.state.phase = ReviewPhase::Review; result.state.difficulty = clampDifficulty(input.difficulty - .1f); result.state.stabilityDays = 3.0f; setInterval(3 * kDayMs); break;
      case Rating::Easy: result.state.phase = ReviewPhase::Review; result.state.difficulty = clampDifficulty(input.difficulty - .5f); result.state.stabilityDays = 7.0f; setInterval(7 * kDayMs); break;
    }
    return result;
  }

  const float stability = input.stabilityDays > 0.0f ? input.stabilityDays : 1.0f;
  switch (rating) {
    case Rating::Again: { const float next = clampStability(stability * .25f); result.state.phase = ReviewPhase::Relearning; result.state.lapses++; result.state.difficulty = clampDifficulty(input.difficulty + .8f); result.state.stabilityDays = std::max(10.0f / 1440.0f, next); setInterval(std::max(10 * kMinuteMs, daysToMs(next))); break; }
    case Rating::Hard: { const float next = clampStability(stability * .8f); result.state.phase = ReviewPhase::Review; result.state.difficulty = clampDifficulty(input.difficulty + .3f); result.state.stabilityDays = next; setInterval(daysToMs(next)); break; }
    case Rating::Good: { const float next = clampStability(stability * 2.0f); result.state.phase = ReviewPhase::Review; result.state.difficulty = clampDifficulty(input.difficulty - .1f); result.state.stabilityDays = next; setInterval(daysToMs(next)); break; }
    case Rating::Easy: { const float next = clampStability(stability * 3.0f); result.state.phase = ReviewPhase::Review; result.state.difficulty = clampDifficulty(input.difficulty - .5f); result.state.stabilityDays = next; setInterval(daysToMs(next)); break; }
  }
  return result;
}

}  // namespace ChinesePoint::Cjk
