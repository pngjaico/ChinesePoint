#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include "chinesepoint/cjk/CjkJournal.h"

namespace ChinesePoint::Cjk {

// In-memory authoritative view of the append-only learner journal. Storage is
// intentionally outside this class so replay and recovery rules are testable
// without a device SD card or any reader dependency.
class LearnerRepository final {
 public:
  bool replay(const uint8_t* journalBytes, size_t journalSize);
  bool recordEncountered(std::string_view headword, std::string_view sentence, std::string_view bookPath,
                         const TextAnchor& anchor, int64_t nowMs);

  bool prepareSnapshot(const LearnerEntry& entry, Journal::EncodedRecord& output) const;
  void markSnapshotCommitted();

  const std::vector<LearnerEntry>& entries() const { return entries_; }
  const LearnerEntry* find(uint64_t wordId, std::string_view headword = {}) const;
  bool needsRepair() const { return repairNeeded; }
  uint32_t lastSequence() const { return sequence; }

 private:
  bool applySnapshot(const LearnerEntry& entry);
  size_t findIndex(uint64_t wordId, std::string_view headword) const;

  std::vector<LearnerEntry> entries_;
  uint32_t sequence = 0;
  bool repairNeeded = false;
};

}  // namespace ChinesePoint::Cjk
