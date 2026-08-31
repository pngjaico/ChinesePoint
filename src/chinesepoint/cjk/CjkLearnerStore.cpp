#include "chinesepoint/cjk/CjkLearnerStore.h"

#include <HalStorage.h>
#include <Logging.h>

#include <limits>
#include <utility>
#include <vector>

namespace ChinesePoint::Cjk {
namespace {

constexpr size_t kMaxJournalBytes = 256 * 1024;

bool writeAll(HalFile& file, const uint8_t* bytes, const size_t size) {
  return size == 0 || file.write(bytes, size) == size;
}

}  // namespace

LearnerStore::LearnerStore(std::string rootPath)
    : rootPath_(std::move(rootPath)),
      journalPath_(rootPath_ + "/learner.journal"),
      tempPath_(rootPath_ + "/learner.journal.tmp"),
      backupPath_(rootPath_ + "/learner.journal.bak") {}

bool LearnerStore::restoreBackupIfNeeded() {
  if (Storage.exists(journalPath_.c_str())) return true;
  if (!Storage.exists(backupPath_.c_str())) return true;
  if (!Storage.rename(backupPath_.c_str(), journalPath_.c_str())) {
    LOG_ERR("CJK", "Could not restore learner journal backup");
    return false;
  }
  LOG_DBG("CJK", "Restored learner journal backup after interrupted compaction");
  return true;
}

bool LearnerStore::load() {
  loaded_ = false;
  if (!restoreBackupIfNeeded()) return false;
  if (!Storage.exists(journalPath_.c_str())) {
    loaded_ = repository_.replay(nullptr, 0);
    return loaded_;
  }

  HalFile file;
  if (!Storage.openFileForRead("CJK", journalPath_, file)) return false;
  const size_t size = file.size();
  if (size > kMaxJournalBytes) {
    LOG_ERR("CJK", "Learner journal exceeds safety limit: %u bytes", static_cast<unsigned>(size));
    return false;
  }
  std::vector<uint8_t> bytes(size);
  const bool readOk = size == 0 || file.read(bytes.data(), size) == static_cast<int>(size);
  const bool closeOk = file.close();
  if (!readOk || !closeOk) return false;
  loaded_ = repository_.replay(bytes.data(), bytes.size());
  return loaded_;
}

bool LearnerStore::append(const Journal::EncodedRecord& record) {
  if (!Storage.ensureDirectoryExists(rootPath_.c_str())) return false;
  HalFile file = Storage.open(journalPath_.c_str(), O_WRITE | O_CREAT);
  if (!file) return false;
  const size_t end = file.size();
  const bool withinLimit = end <= kMaxJournalBytes && record.size <= kMaxJournalBytes - end;
  const bool ok = withinLimit && file.seekSet(end) && writeAll(file, record.bytes.data(), record.size);
  file.flush();
  const bool closeOk = file.close();
  if (!withinLimit) LOG_ERR("CJK", "Learner journal reached safety limit; compaction is required");
  if (!ok || !closeOk) LOG_ERR("CJK", "Could not append learner journal record");
  return ok && closeOk;
}

bool LearnerStore::recordEncountered(const std::string_view headword, const std::string_view sentence,
                                     const std::string_view bookPath, const TextAnchor& anchor, const int64_t nowMs) {
  return record(headword, sentence, bookPath, anchor, nowMs, WordStatus::Encountered);
}

bool LearnerStore::recordSaved(const std::string_view headword, const std::string_view sentence,
                                const std::string_view bookPath, const TextAnchor& anchor, const int64_t nowMs) {
  return record(headword, sentence, bookPath, anchor, nowMs, WordStatus::Saved);
}

bool LearnerStore::record(const std::string_view headword, const std::string_view sentence,
                          const std::string_view bookPath, const TextAnchor& anchor, const int64_t nowMs,
                          const WordStatus requestedStatus) {
  if ((!loaded_ && !load()) || (repository_.needsRepair() && !compact())) return false;

  LearnerRepository candidate = repository_;
  const bool recorded = requestedStatus == WordStatus::Saved
                            ? candidate.recordSaved(headword, sentence, bookPath, anchor, nowMs)
                            : candidate.recordEncountered(headword, sentence, bookPath, anchor, nowMs);
  if (!recorded) return false;
  const LearnerEntry* entry = candidate.find(stableWordId(headword), headword);
  Journal::EncodedRecord record;
  if (entry == nullptr || !candidate.prepareSnapshot(*entry, record) || !append(record)) return false;
  candidate.markSnapshotCommitted();
  repository_ = std::move(candidate);
  return true;
}

bool LearnerStore::compact() {
  if (!loaded_ && !load()) return false;
  if (!Storage.ensureDirectoryExists(rootPath_.c_str())) return false;
  Storage.remove(tempPath_.c_str());

  {
    HalFile temp;
    if (!Storage.openFileForWrite("CJK", tempPath_, temp)) return false;
    uint32_t sequence = 0;
    size_t bytesWritten = 0;
    bool writeOk = true;
    for (const auto& entry : repository_.entries()) {
      Journal::PayloadBuffer payload;
      Journal::EncodedRecord record;
      if (sequence == std::numeric_limits<uint32_t>::max() || !Journal::encodeEntry(entry, payload) ||
          !Journal::encodeRecord(Journal::RecordType::EntrySnapshot, ++sequence, payload.bytes.data(), payload.size,
                                 record) || record.size > kMaxJournalBytes - bytesWritten ||
          !writeAll(temp, record.bytes.data(), record.size)) {
        writeOk = false;
        break;
      }
      bytesWritten += record.size;
    }
    temp.flush();
    const bool closeOk = temp.close();
    if (!writeOk || !closeOk) {
      LOG_ERR("CJK", "Could not write compact learner journal");
      return false;
    }
  }

  const bool hadJournal = Storage.exists(journalPath_.c_str());
  if (hadJournal) {
    Storage.remove(backupPath_.c_str());
    if (!Storage.rename(journalPath_.c_str(), backupPath_.c_str())) return false;
  }
  if (!Storage.rename(tempPath_.c_str(), journalPath_.c_str())) {
    if (hadJournal && !Storage.rename(backupPath_.c_str(), journalPath_.c_str())) {
      LOG_ERR("CJK", "Learner journal replacement and backup restore both failed");
    }
    return false;
  }
  Storage.remove(backupPath_.c_str());
  return load();
}

LearnerStats LearnerStore::stats() const {
  const auto& entries = repository_.entries();
  return computeLearnerStats(entries.data(), entries.size());
}

LearnerStore& learnerStore() {
  static LearnerStore store;
  return store;
}

}  // namespace ChinesePoint::Cjk
