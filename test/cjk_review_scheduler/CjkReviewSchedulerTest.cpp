#include <gtest/gtest.h>

#include "chinesepoint/cjk/CjkReviewScheduler.h"

namespace {
constexpr int64_t kMinute = 60LL * 1000;
constexpr int64_t kDay = 24LL * 60LL * kMinute;
constexpr int64_t kNow = 5LL * kDay;
}  // namespace

TEST(CjkReviewScheduler, NewCardRatingsProduceBoundedLocalIntervals) {
  ChinesePoint::Cjk::ReviewScheduler scheduler;
  const ChinesePoint::Cjk::ReviewState fresh{};

  EXPECT_TRUE(scheduler.isDue(fresh, kNow));
  EXPECT_EQ(scheduler.rate(fresh, ChinesePoint::Cjk::Rating::Again, kNow).intervalMs, kMinute);
  EXPECT_EQ(scheduler.rate(fresh, ChinesePoint::Cjk::Rating::Hard, kNow).intervalMs, 6 * kMinute);
  EXPECT_EQ(scheduler.rate(fresh, ChinesePoint::Cjk::Rating::Good, kNow).intervalMs, kDay);
  EXPECT_EQ(scheduler.rate(fresh, ChinesePoint::Cjk::Rating::Easy, kNow).intervalMs, 4 * kDay);
}

TEST(CjkReviewScheduler, ReviewRatingsKeepStateCoherent) {
  ChinesePoint::Cjk::ReviewScheduler scheduler;
  ChinesePoint::Cjk::ReviewState review{};
  review.phase = ChinesePoint::Cjk::ReviewPhase::Review;
  review.dueAtMs = kNow;
  review.stabilityDays = 10.0f;
  review.difficulty = 5.0f;
  review.reps = 9;

  const auto again = scheduler.rate(review, ChinesePoint::Cjk::Rating::Again, kNow);
  EXPECT_EQ(again.state.phase, ChinesePoint::Cjk::ReviewPhase::Relearning);
  EXPECT_EQ(again.intervalMs, static_cast<int64_t>(2.5 * kDay));
  EXPECT_EQ(again.state.reps, 10u);
  EXPECT_EQ(again.state.lapses, 1u);

  const auto easy = scheduler.rate(review, ChinesePoint::Cjk::Rating::Easy, kNow);
  EXPECT_EQ(easy.intervalMs, 30 * kDay);
  EXPECT_LT(easy.state.difficulty, review.difficulty);
}

TEST(CjkReviewScheduler, AnkiAuthorityIsNeverLocallyDue) {
  ChinesePoint::Cjk::ReviewScheduler scheduler;
  ChinesePoint::Cjk::ReviewState state{};
  state.authority = ChinesePoint::Cjk::ScheduleAuthority::Anki;

  EXPECT_FALSE(scheduler.isDue(state, kNow));
}
