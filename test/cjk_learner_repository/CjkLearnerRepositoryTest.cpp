#include <gtest/gtest.h>

#include <array>
#include <vector>

#include "chinesepoint/cjk/CjkLearnerRepository.h"

namespace {
using ChinesePoint::Cjk::Journal::EncodedRecord;
using ChinesePoint::Cjk::LearnerRepository;
using ChinesePoint::Cjk::TextAnchor;

TEST(CjkLearnerRepository, EncounterRetainsLearnerStatusAndProducesSnapshot) {
  LearnerRepository repository;
  ASSERT_TRUE(repository.recordEncountered("学习", "我每天学习中文。", "/books/a.epub", {1, 5, 2, 7}, 100));
  ASSERT_EQ(repository.entries().size(), 1u);
  EXPECT_EQ(repository.entries()[0].encounterCount, 1u);
  EXPECT_EQ(repository.entries()[0].status, ChinesePoint::Cjk::WordStatus::Encountered);

  ASSERT_TRUE(repository.recordEncountered("学习", "我每天学习中文。", "/books/a.epub", {1, 5, 2, 7}, 150));
  EXPECT_EQ(repository.entries().size(), 1u);
  EXPECT_EQ(repository.entries()[0].encounterCount, 2u);
  EXPECT_EQ(repository.entries()[0].lastSeenStudyMs, 150);

  EncodedRecord record;
  ASSERT_TRUE(repository.prepareSnapshot(repository.entries()[0], record));
  EXPECT_EQ(record.bytes[8], 1u);
  repository.markSnapshotCommitted();
  EXPECT_EQ(repository.lastSequence(), 1u);
}

TEST(CjkLearnerRepository, ReplayStopsAtTornTailButKeepsLastDurableEntry) {
  LearnerRepository writer;
  ASSERT_TRUE(writer.recordEncountered("书", "这是一本书。", "/books/a.epub", {0, 1, 1, 2}, 20));
  EncodedRecord record;
  ASSERT_TRUE(writer.prepareSnapshot(writer.entries()[0], record));

  std::vector<uint8_t> bytes(record.bytes.begin(), record.bytes.begin() + record.size);
  bytes.push_back('C');
  bytes.push_back('J');

  LearnerRepository reader;
  ASSERT_TRUE(reader.replay(bytes.data(), bytes.size()));
  ASSERT_EQ(reader.entries().size(), 1u);
  EXPECT_EQ(reader.entries()[0].headword, "书");
  EXPECT_TRUE(reader.needsRepair());
  EXPECT_EQ(reader.lastSequence(), 1u);
}

TEST(CjkLearnerRepository, ReplayRejectsSequenceRollbackWithoutDroppingPriorState) {
  LearnerRepository writer;
  ASSERT_TRUE(writer.recordEncountered("字", "一个字。", "/books/a.epub", TextAnchor{}, 1));
  EncodedRecord first;
  ASSERT_TRUE(writer.prepareSnapshot(writer.entries()[0], first));

  std::vector<uint8_t> bytes(first.bytes.begin(), first.bytes.begin() + first.size);
  bytes.insert(bytes.end(), first.bytes.begin(), first.bytes.begin() + first.size);

  LearnerRepository reader;
  ASSERT_TRUE(reader.replay(bytes.data(), bytes.size()));
  EXPECT_EQ(reader.entries().size(), 1u);
  EXPECT_TRUE(reader.needsRepair());
}

}  // namespace
