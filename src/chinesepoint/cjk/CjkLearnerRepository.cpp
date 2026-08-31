#include "chinesepoint/cjk/CjkLearnerRepository.h"

#include <algorithm>
#include <limits>

namespace ChinesePoint::Cjk {

size_t LearnerRepository::findIndex(const uint64_t wordId, const std::string_view headword) const {
  for (size_t index = 0; index < entries_.size(); ++index) {
    if (entries_[index].wordId == wordId && (headword.empty() || entries_[index].headword == headword)) return index;
  }
  return entries_.size();
}

const LearnerEntry* LearnerRepository::find(const uint64_t wordId, const std::string_view headword) const {
  const size_t index = findIndex(wordId, headword);
  return index < entries_.size() ? &entries_[index] : nullptr;
}

bool LearnerRepository::applySnapshot(const LearnerEntry& entry) {
  if (!validHeadword(entry.headword) || !validSentence(entry.sourceSentence) || !validBookPath(entry.bookPath)) return false;
  const size_t index = findIndex(entry.wordId, entry.headword);
  if (index < entries_.size()) entries_[index] = entry;
  else entries_.push_back(entry);
  return true;
}

bool LearnerRepository::replay(const uint8_t* journalBytes, const size_t journalSize) {
  entries_.clear();
  sequence = 0;
  repairNeeded = false;
  if (journalSize == 0) return true;
  if (journalBytes == nullptr) return false;

  size_t position = 0;
  while (position < journalSize) {
    Journal::RecordView record;
    const auto status = Journal::decodeRecord(journalBytes + position, journalSize - position, record);
    if (status != Journal::DecodeStatus::Ok || record.sequence <= sequence ||
        record.type != Journal::RecordType::EntrySnapshot) {
      repairNeeded = true;
      break;
    }
    LearnerEntry entry;
    if (!Journal::decodeEntry(record.payload, record.payloadSize, entry) || !applySnapshot(entry)) {
      repairNeeded = true;
      break;
    }
    sequence = record.sequence;
    position += record.totalSize;
  }
  return true;
}

bool LearnerRepository::record(const std::string_view headword, const std::string_view sentence,
                               const std::string_view bookPath, const TextAnchor& anchor, const int64_t nowMs,
                               const WordStatus requestedStatus) {
  if (!validHeadword(headword) || !validSentence(sentence) || !validBookPath(bookPath)) return false;
  const uint64_t wordId = stableWordId(headword);
  const size_t index = findIndex(wordId, headword);
  LearnerEntry next;
  if (index < entries_.size()) next = entries_[index];
  else {
    next.wordId = wordId;
    next.headword.assign(headword);
    next.firstSeenStudyMs = nowMs;
  }
  if (next.encounterCount < std::numeric_limits<uint32_t>::max()) ++next.encounterCount;
  if (static_cast<uint8_t>(requestedStatus) > static_cast<uint8_t>(next.status)) next.status = requestedStatus;
  next.lastSeenStudyMs = std::max(next.lastSeenStudyMs, nowMs);
  next.bookPath.assign(bookPath);
  next.sourceAnchor = anchor;
  next.sourceSentence.assign(sentence);
  return applySnapshot(next);
}

bool LearnerRepository::recordEncountered(const std::string_view headword, const std::string_view sentence,
                                          const std::string_view bookPath, const TextAnchor& anchor,
                                          const int64_t nowMs) {
  return record(headword, sentence, bookPath, anchor, nowMs, WordStatus::Encountered);
}

bool LearnerRepository::recordSaved(const std::string_view headword, const std::string_view sentence,
                                    const std::string_view bookPath, const TextAnchor& anchor,
                                    const int64_t nowMs) {
  return record(headword, sentence, bookPath, anchor, nowMs, WordStatus::Saved);
}

bool LearnerRepository::prepareSnapshot(const LearnerEntry& entry, Journal::EncodedRecord& output) const {
  if (sequence == std::numeric_limits<uint32_t>::max()) return false;
  Journal::PayloadBuffer payload;
  return Journal::encodeEntry(entry, payload) &&
         Journal::encodeRecord(Journal::RecordType::EntrySnapshot, sequence + 1, payload.bytes.data(), payload.size, output);
}

void LearnerRepository::markSnapshotCommitted() {
  if (sequence < std::numeric_limits<uint32_t>::max()) ++sequence;
}

}  // namespace ChinesePoint::Cjk
