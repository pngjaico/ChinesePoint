#include <array>
#include <cmath>

#include <gtest/gtest.h>

#include "chinesepoint/study/StudyFsrs5.h"

namespace {

TEST(StudyFsrs5, DefaultFirstReviewAndForgettingCurveAreStable) {
  using namespace ChinesePoint::Study;
  Fsrs5 fsrs;
  const FsrsMemory first = fsrs.review(FsrsMemory{}, Rating::Good, 0);
  EXPECT_TRUE(first.learned);
  EXPECT_NEAR(first.stability, 3.173f, 0.00001f);
  EXPECT_GE(first.difficulty, 1.0f);
  EXPECT_LE(first.difficulty, 10.0f);

  const FsrsMemory known{10.0f, 5.0f, true};
  EXPECT_NEAR(fsrs.retrievability(known, 10.0f), 0.9f, 0.00001f);
  EXPECT_FLOAT_EQ(fsrs.retrievability(known, -1.0f), 1.0f);
}

TEST(StudyFsrs5, SameDayAndLapseRulesDoNotRaiseStability) {
  using namespace ChinesePoint::Study;
  Fsrs5 fsrs;
  const FsrsMemory memory{12.0f, 6.0f, true};
  const FsrsMemory sameDayAgain = fsrs.review(memory, Rating::Again, 0);
  const FsrsMemory sameDayGood = fsrs.review(memory, Rating::Good, 0);
  EXPECT_LE(sameDayAgain.stability, memory.stability);
  EXPECT_GE(sameDayGood.stability, memory.stability);

  const FsrsMemory lapse = fsrs.review(memory, Rating::Again, 30);
  EXPECT_LE(lapse.stability, memory.stability);
  EXPECT_GE(lapse.difficulty, 1.0f);
  EXPECT_LE(lapse.difficulty, 10.0f);
}

TEST(StudyFsrs5, IntervalPreviewHonorsConfiguredMaximum) {
  using namespace ChinesePoint::Study;
  Fsrs5 fsrs;
  fsrs.setMaximumInterval(7);
  const FsrsMemory memory{100.0f, 5.0f, true};
  EXPECT_EQ(fsrs.intervalDays(memory), 7);
  std::array<int, 4> preview{};
  fsrs.previewIntervals(memory, 10, preview.data());
  for (const int interval : preview) {
    EXPECT_GE(interval, 1);
    EXPECT_LE(interval, 7);
  }
}

}  // namespace
