#include <gtest/gtest.h>

#include "chinesepoint/CjkSafetyGuard.h"

namespace Guard = ChinesePoint::CjkSafetyGuard;

class CjkSafetyGuardSimulatorTest : public ::testing::Test {
 protected:
  void SetUp() override { Guard::clearSafeMode(); }
};

TEST_F(CjkSafetyGuardSimulatorTest, SessionCrashSequenceMatchesTheDevicePolicy) {
  ASSERT_TRUE(Guard::startLearnerSession());
  EXPECT_EQ(Guard::reconcileBoot(true).crashStreak, 1);

  ASSERT_TRUE(Guard::startLearnerSession());
  const Guard::Status secondCrash = Guard::reconcileBoot(true);
  EXPECT_TRUE(secondCrash.storageAvailable);
  EXPECT_TRUE(secondCrash.safeMode);
  EXPECT_FALSE(Guard::startLearnerSession());
}

TEST_F(CjkSafetyGuardSimulatorTest, NormalBootClearsAnUnfinishedSession) {
  ASSERT_TRUE(Guard::startLearnerSession());
  const Guard::Status status = Guard::reconcileBoot(false);
  EXPECT_TRUE(status.learnerEnabled());
  EXPECT_EQ(status.crashStreak, 0);
}
