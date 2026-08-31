#include <gtest/gtest.h>

#include <Preferences.h>

#include "chinesepoint/CjkSafetyGuard.h"

namespace Guard = ChinesePoint::CjkSafetyGuard;

class CjkSafetyGuardTest : public ::testing::Test {
 protected:
  void SetUp() override { Preferences::reset(); }
};

TEST_F(CjkSafetyGuardTest, FreshBootStartsEnabled) {
  const Guard::Status status = Guard::reconcileBoot(false);
  EXPECT_TRUE(status.storageAvailable);
  EXPECT_TRUE(status.learnerEnabled());
  EXPECT_EQ(status.crashStreak, 0);
}

TEST_F(CjkSafetyGuardTest, TwoConsecutiveLearnerPanicsEnterSafeMode) {
  ASSERT_TRUE(Guard::startLearnerSession());
  const Guard::Status firstCrash = Guard::reconcileBoot(true);
  EXPECT_TRUE(firstCrash.learnerEnabled());
  EXPECT_EQ(firstCrash.crashStreak, 1);

  ASSERT_TRUE(Guard::startLearnerSession());
  const Guard::Status secondCrash = Guard::reconcileBoot(true);
  EXPECT_FALSE(secondCrash.learnerEnabled());
  EXPECT_EQ(secondCrash.crashStreak, 2);
  EXPECT_FALSE(Guard::startLearnerSession());
}

TEST_F(CjkSafetyGuardTest, CleanExitBreaksTheCrashSequence) {
  ASSERT_TRUE(Guard::startLearnerSession());
  ASSERT_EQ(Guard::reconcileBoot(true).crashStreak, 1);

  ASSERT_TRUE(Guard::startLearnerSession());
  Guard::finishLearnerSession();
  EXPECT_EQ(Guard::reconcileBoot(false).crashStreak, 0);

  ASSERT_TRUE(Guard::startLearnerSession());
  const Guard::Status crash = Guard::reconcileBoot(true);
  EXPECT_TRUE(crash.learnerEnabled());
  EXPECT_EQ(crash.crashStreak, 1);
}

TEST_F(CjkSafetyGuardTest, SafeModePersistsUntilExplicitlyCleared) {
  ASSERT_TRUE(Guard::startLearnerSession());
  Guard::reconcileBoot(true);
  ASSERT_TRUE(Guard::startLearnerSession());
  ASSERT_FALSE(Guard::reconcileBoot(true).learnerEnabled());

  EXPECT_FALSE(Guard::reconcileBoot(false).learnerEnabled());
  Guard::clearSafeMode();
  EXPECT_TRUE(Guard::reconcileBoot(false).learnerEnabled());
}

TEST_F(CjkSafetyGuardTest, StorageFailuresAreFailOpen) {
  Preferences::setBeginSucceeds(false);
  const Guard::Status status = Guard::reconcileBoot(true);
  EXPECT_FALSE(status.storageAvailable);
  EXPECT_TRUE(status.learnerEnabled());
  EXPECT_TRUE(Guard::startLearnerSession());
}

TEST_F(CjkSafetyGuardTest, FailedWritesAreReportedFailOpen) {
  Preferences::setWritesSucceed(false);
  const Guard::Status status = Guard::reconcileBoot(false);
  EXPECT_FALSE(status.storageAvailable);
  EXPECT_TRUE(status.learnerEnabled());
  EXPECT_TRUE(Guard::startLearnerSession());
}
