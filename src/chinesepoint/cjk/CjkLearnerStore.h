#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "chinesepoint/cjk/CjkLearnerRepository.h"
#include "chinesepoint/cjk/CjkLearnerStats.h"

namespace ChinesePoint::Cjk {

// SD-card persistence for the optional learner only. It never reads or writes
// CrossPoint book caches, settings, firmware, or boot state.
class LearnerStore final {
 public:
  explicit LearnerStore(std::string rootPath = "/.chinesepoint/learner/v1");

  bool load();
  bool recordEncountered(std::string_view headword, std::string_view sentence, std::string_view bookPath,
                         const TextAnchor& anchor, int64_t nowMs);
  bool recordSaved(std::string_view headword, std::string_view sentence, std::string_view bookPath,
                   const TextAnchor& anchor, int64_t nowMs);
  bool compact();

  const LearnerRepository& repository() const { return repository_; }
  LearnerStats stats() const;
  bool loaded() const { return loaded_; }

 private:
  bool restoreBackupIfNeeded();
  bool append(const Journal::EncodedRecord& record);
  bool record(std::string_view headword, std::string_view sentence, std::string_view bookPath,
              const TextAnchor& anchor, int64_t nowMs, WordStatus requestedStatus);

  std::string rootPath_;
  std::string journalPath_;
  std::string tempPath_;
  std::string backupPath_;
  LearnerRepository repository_;
  bool loaded_ = false;
};

LearnerStore& learnerStore();

}  // namespace ChinesePoint::Cjk
