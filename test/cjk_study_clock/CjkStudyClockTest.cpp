#include <gtest/gtest.h>

#include "chinesepoint/cjk/CjkStudyClock.h"

namespace {
using ChinesePoint::Cjk::StudyClock;
using ChinesePoint::Cjk::StudyClockState;

TEST(CjkStudyClock, MonotonicSessionTimeSurvivesRebootWithoutWallClock) {
  StudyClock first({}, 100);
  const auto beforeReboot = first.observe(6100);
  EXPECT_EQ(beforeReboot.nowMs, 6000);
  EXPECT_FALSE(beforeReboot.wallClockTrusted);

  StudyClock second(beforeReboot.state, 20);
  const auto afterReboot = second.observe(1020);
  EXPECT_EQ(afterReboot.nowMs, 7000);
  EXPECT_GE(afterReboot.nowMs, beforeReboot.nowMs);
}

TEST(CjkStudyClock, TrustedWallClockAdvancesLogicalTimeAndRejectsRollback) {
  constexpr int64_t kWall = ChinesePoint::Cjk::kMinTrustedWallClockMs + 10'000;
  StudyClock clock({}, 0);
  const auto first = clock.observe(5, kWall);
  EXPECT_TRUE(first.wallClockTrusted);
  EXPECT_EQ(first.nowMs, kWall);

  const auto rollback = clock.observe(10, kWall - 5'000);
  EXPECT_TRUE(rollback.wallClockTrusted);
  EXPECT_EQ(rollback.nowMs, kWall);
  EXPECT_EQ(rollback.state.lastTrustedWallMs, kWall);
}

TEST(CjkStudyClock, InvalidPersistedStateIsFailClosedToZero) {
  StudyClock invalid(StudyClockState{100, 200}, 0);
  EXPECT_EQ(invalid.observe(50).nowMs, 50);
}

TEST(CjkStudyClock, StateValidationRejectsNegativeAndInconsistentValues) {
  EXPECT_FALSE(ChinesePoint::Cjk::validStudyClockState({-1, 0}));
  EXPECT_FALSE(ChinesePoint::Cjk::validStudyClockState({10, 11}));
  EXPECT_TRUE(ChinesePoint::Cjk::validStudyClockState({10, 10}));
}
}  // namespace
