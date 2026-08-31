#pragma once

#include <cstddef>
#include <cstdint>

#include "chinesepoint/cjk/CjkLearnerModel.h"

namespace ChinesePoint::Cjk {

// A deliberately small, derived summary. It never writes the journal and
// excludes no records based on sync state, so the learner can explain every
// displayed number from its durable local entries.
struct LearnerStats {
  uint32_t vocabularyCount = 0;
  uint32_t encounteredCount = 0;
  uint32_t savedCount = 0;
  uint32_t learningCount = 0;
  uint32_t knownCount = 0;
  uint64_t encounterTotal = 0;
  uint32_t sourceBookCount = 0;
  int64_t firstSeenStudyMs = 0;
  int64_t lastSeenStudyMs = 0;
};

LearnerStats computeLearnerStats(const LearnerEntry* entries, size_t entryCount);

}  // namespace ChinesePoint::Cjk
