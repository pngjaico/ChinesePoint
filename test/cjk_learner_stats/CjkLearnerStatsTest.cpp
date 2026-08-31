#include <gtest/gtest.h>

#include <array>

#include "chinesepoint/cjk/CjkLearnerStats.h"

namespace {
using ChinesePoint::Cjk::LearnerEntry;
using ChinesePoint::Cjk::WordStatus;

TEST(CjkLearnerStats, EmptyOrMissingEntriesProduceAnEmptySummary) {
  const auto missing = ChinesePoint::Cjk::computeLearnerStats(nullptr, 0);
  EXPECT_EQ(missing.vocabularyCount, 0u);
  EXPECT_EQ(missing.encounterTotal, 0u);
  EXPECT_EQ(missing.sourceBookCount, 0u);
}

TEST(CjkLearnerStats, CountsStatusesEncountersAndDistinctSourceBooks) {
  std::array<LearnerEntry, 4> entries{};
  const auto setEntry = [&entries](const size_t index, const uint64_t id, const char* word, const WordStatus status,
                                   const uint32_t encounters, const int64_t first, const int64_t last,
                                   const char* book) {
    auto& entry = entries[index];
    entry.wordId = id;
    entry.headword = word;
    entry.status = status;
    entry.encounterCount = encounters;
    entry.firstSeenStudyMs = first;
    entry.lastSeenStudyMs = last;
    entry.bookPath = book;
  };
  setEntry(0, 1, "书", WordStatus::Encountered, 2, 10, 20, "/books/a.epub");
  setEntry(1, 2, "字", WordStatus::Saved, 3, 15, 25, "/books/a.epub");
  setEntry(2, 3, "学习", WordStatus::Learning, 5, 12, 30, "/books/b.epub");
  setEntry(3, 4, "中文", WordStatus::Known, 7, 0, 35, "");

  const auto stats = ChinesePoint::Cjk::computeLearnerStats(entries.data(), entries.size());
  EXPECT_EQ(stats.vocabularyCount, 4u);
  EXPECT_EQ(stats.encounteredCount, 1u);
  EXPECT_EQ(stats.savedCount, 1u);
  EXPECT_EQ(stats.learningCount, 1u);
  EXPECT_EQ(stats.knownCount, 1u);
  EXPECT_EQ(stats.encounterTotal, 17u);
  EXPECT_EQ(stats.sourceBookCount, 2u);
  EXPECT_EQ(stats.firstSeenStudyMs, 10);
  EXPECT_EQ(stats.lastSeenStudyMs, 35);
}

}  // namespace
