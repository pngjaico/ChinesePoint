#include "chinesepoint/cjk/CjkJournal.h"

#include <cstring>
#include <utility>

namespace ChinesePoint::Cjk::Journal {
namespace {

constexpr uint8_t kMagic[] = {'C', 'J', 'K', 'L'};

bool isKnownRecordType(uint8_t value) {
  return value >= static_cast<uint8_t>(RecordType::EntrySnapshot) &&
         value <= static_cast<uint8_t>(RecordType::StudyClock);
}

void writeU16(uint8_t* output, uint16_t value) {
  output[0] = static_cast<uint8_t>(value & 0xFFu);
  output[1] = static_cast<uint8_t>((value >> 8) & 0xFFu);
}

void writeU32(uint8_t* output, uint32_t value) {
  for (size_t index = 0; index < 4; ++index) {
    output[index] = static_cast<uint8_t>((value >> (index * 8)) & 0xFFu);
  }
}

uint16_t readU16(const uint8_t* input) {
  return static_cast<uint16_t>(input[0]) |
         (static_cast<uint16_t>(input[1]) << 8);
}

uint32_t readU32(const uint8_t* input) {
  uint32_t value = 0;
  for (size_t index = 0; index < 4; ++index) {
    value |= static_cast<uint32_t>(input[index]) << (index * 8);
  }
  return value;
}

class PayloadWriter {
 public:
  explicit PayloadWriter(PayloadBuffer& output) : output(output) { output.size = 0; }

  bool u8(uint8_t value) { return raw(&value, sizeof(value)); }
  bool u16(uint16_t value) {
    uint8_t bytes[2];
    writeU16(bytes, value);
    return raw(bytes, sizeof(bytes));
  }
  bool u32(uint32_t value) {
    uint8_t bytes[4];
    writeU32(bytes, value);
    return raw(bytes, sizeof(bytes));
  }
  bool u64(uint64_t value) {
    uint8_t bytes[8];
    for (size_t index = 0; index < sizeof(bytes); ++index) {
      bytes[index] = static_cast<uint8_t>((value >> (index * 8)) & 0xFFu);
    }
    return raw(bytes, sizeof(bytes));
  }
  bool i64(int64_t value) { return u64(static_cast<uint64_t>(value)); }
  bool f32(float value) {
    uint32_t rawValue = 0;
    static_assert(sizeof(rawValue) == sizeof(value));
    std::memcpy(&rawValue, &value, sizeof(value));
    return u32(rawValue);
  }
  bool string(std::string_view value) {
    return value.size() <= UINT16_MAX && u16(static_cast<uint16_t>(value.size())) &&
           raw(reinterpret_cast<const uint8_t*>(value.data()), value.size());
  }

 private:
  bool raw(const uint8_t* data, size_t size) {
    if (size > output.bytes.size() - output.size) return false;
    if (size > 0) std::memcpy(output.bytes.data() + output.size, data, size);
    output.size += size;
    return true;
  }

  PayloadBuffer& output;
};

class PayloadReader {
 public:
  PayloadReader(const uint8_t* data, size_t size) : data(data), size(size) {}

  bool u8(uint8_t& value) { return raw(&value, sizeof(value)); }
  bool u16(uint16_t& value) {
    uint8_t bytes[2];
    if (!raw(bytes, sizeof(bytes))) return false;
    value = readU16(bytes);
    return true;
  }
  bool u32(uint32_t& value) {
    uint8_t bytes[4];
    if (!raw(bytes, sizeof(bytes))) return false;
    value = readU32(bytes);
    return true;
  }
  bool u64(uint64_t& value) {
    uint8_t bytes[8];
    if (!raw(bytes, sizeof(bytes))) return false;
    value = 0;
    for (size_t index = 0; index < sizeof(bytes); ++index) {
      value |= static_cast<uint64_t>(bytes[index]) << (index * 8);
    }
    return true;
  }
  bool i64(int64_t& value) {
    uint64_t rawValue = 0;
    if (!u64(rawValue)) return false;
    value = static_cast<int64_t>(rawValue);
    return true;
  }
  bool f32(float& value) {
    uint32_t rawValue = 0;
    if (!u32(rawValue)) return false;
    static_assert(sizeof(rawValue) == sizeof(value));
    std::memcpy(&value, &rawValue, sizeof(value));
    return true;
  }
  bool string(std::string& value, size_t maximum) {
    uint16_t length = 0;
    if (!u16(length) || length > maximum || length > remaining()) return false;
    value.assign(reinterpret_cast<const char*>(data + position), length);
    position += length;
    return true;
  }
  bool done() const { return position == size; }

 private:
  bool raw(uint8_t* output, size_t count) {
    if (count > remaining()) return false;
    if (count > 0) std::memcpy(output, data + position, count);
    position += count;
    return true;
  }
  size_t remaining() const { return size - position; }

  const uint8_t* data;
  size_t size;
  size_t position = 0;
};

bool validReviewState(const ReviewState& state) {
  return static_cast<uint8_t>(state.phase) <= static_cast<uint8_t>(ReviewPhase::Relearning) &&
         static_cast<uint8_t>(state.authority) <= static_cast<uint8_t>(ScheduleAuthority::Anki);
}

bool encodeEntryToWriter(const LearnerEntry& entry, PayloadWriter& writer) {
  if (!validHeadword(entry.headword) || !validSentence(entry.sourceSentence) || !validBookPath(entry.bookPath) ||
      !validReviewState(entry.review)) {
    return false;
  }
  return writer.u64(entry.wordId) && writer.u8(static_cast<uint8_t>(entry.status)) &&
         writer.u32(entry.encounterCount) && writer.i64(entry.firstSeenStudyMs) && writer.i64(entry.lastSeenStudyMs) &&
         writer.u16(entry.sourceAnchor.spineIndex) && writer.u32(entry.sourceAnchor.visibleCodepointOffset) &&
         writer.u16(entry.sourceAnchor.codepointLength) && writer.u32(entry.sourceAnchor.fingerprint) &&
         writer.u8(static_cast<uint8_t>(entry.review.phase)) && writer.u8(static_cast<uint8_t>(entry.review.authority)) &&
         writer.i64(entry.review.dueAtMs) && writer.i64(entry.review.lastReviewAtMs) &&
         writer.f32(entry.review.difficulty) && writer.f32(entry.review.stabilityDays) && writer.u32(entry.review.reps) &&
         writer.u32(entry.review.lapses) && writer.u64(entry.review.ankiCardId) && writer.string(entry.headword) &&
         writer.string(entry.bookPath) && writer.string(entry.sourceSentence);
}

bool decodeEntryFromReader(PayloadReader& reader, LearnerEntry& entry) {
  uint8_t status = 0;
  uint8_t phase = 0;
  uint8_t authority = 0;
  if (!reader.u64(entry.wordId) || !reader.u8(status) || status > static_cast<uint8_t>(WordStatus::Known) ||
      !reader.u32(entry.encounterCount) || !reader.i64(entry.firstSeenStudyMs) || !reader.i64(entry.lastSeenStudyMs) ||
      !reader.u16(entry.sourceAnchor.spineIndex) || !reader.u32(entry.sourceAnchor.visibleCodepointOffset) ||
      !reader.u16(entry.sourceAnchor.codepointLength) || !reader.u32(entry.sourceAnchor.fingerprint) ||
      !reader.u8(phase) || phase > static_cast<uint8_t>(ReviewPhase::Relearning) ||
      !reader.u8(authority) || authority > static_cast<uint8_t>(ScheduleAuthority::Anki) ||
      !reader.i64(entry.review.dueAtMs) || !reader.i64(entry.review.lastReviewAtMs) ||
      !reader.f32(entry.review.difficulty) || !reader.f32(entry.review.stabilityDays) ||
      !reader.u32(entry.review.reps) || !reader.u32(entry.review.lapses) || !reader.u64(entry.review.ankiCardId) ||
      !reader.string(entry.headword, kMaxHeadwordBytes) || !reader.string(entry.bookPath, kMaxBookPathBytes) ||
      !reader.string(entry.sourceSentence, kMaxSentenceBytes)) {
    return false;
  }
  entry.status = static_cast<WordStatus>(status);
  entry.review.phase = static_cast<ReviewPhase>(phase);
  entry.review.authority = static_cast<ScheduleAuthority>(authority);
  return validHeadword(entry.headword) && validSentence(entry.sourceSentence) && validBookPath(entry.bookPath) &&
         validReviewState(entry.review);
}

}  // namespace

uint32_t crc32(const uint8_t* data, size_t size, uint32_t seed) {
  uint32_t crc = seed;
  for (size_t index = 0; index < size; ++index) {
    crc ^= data[index];
    for (int bit = 0; bit < 8; ++bit) {
      const uint32_t mask = -(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & mask);
    }
  }
  return crc;
}

bool encodeRecord(RecordType type, uint32_t sequence, const uint8_t* payload,
                  size_t payloadSize, EncodedRecord& output) {
  if (!isKnownRecordType(static_cast<uint8_t>(type)) || payloadSize > kMaxPayloadBytes ||
      (payloadSize > 0 && payload == nullptr)) {
    return false;
  }

  auto* bytes = output.bytes.data();
  bytes[0] = kMagic[0];
  bytes[1] = kMagic[1];
  bytes[2] = kMagic[2];
  bytes[3] = kMagic[3];
  bytes[4] = kVersion;
  bytes[5] = static_cast<uint8_t>(type);
  writeU16(bytes + 6, static_cast<uint16_t>(payloadSize));
  writeU32(bytes + 8, sequence);

  for (size_t index = 0; index < payloadSize; ++index) {
    bytes[kHeaderBytes + index] = payload[index];
  }

  uint32_t checksum = crc32(bytes + 4, 8);
  checksum = crc32(bytes + kHeaderBytes, payloadSize, checksum);
  writeU32(bytes + 12, checksum ^ 0xFFFFFFFFu);
  output.size = kHeaderBytes + payloadSize;
  return true;
}

DecodeStatus decodeRecord(const uint8_t* data, size_t size, RecordView& output) {
  if (data == nullptr || size < kHeaderBytes) {
    return DecodeStatus::NeedMoreData;
  }
  if (data[0] != kMagic[0] || data[1] != kMagic[1] || data[2] != kMagic[2] ||
      data[3] != kMagic[3]) {
    return DecodeStatus::BadMagic;
  }
  if (data[4] != kVersion) {
    return DecodeStatus::UnsupportedVersion;
  }
  if (!isKnownRecordType(data[5])) {
    return DecodeStatus::UnknownRecordType;
  }

  const size_t payloadSize = readU16(data + 6);
  if (payloadSize > kMaxPayloadBytes) {
    return DecodeStatus::PayloadTooLarge;
  }
  const size_t totalSize = kHeaderBytes + payloadSize;
  if (size < totalSize) {
    return DecodeStatus::NeedMoreData;
  }

  uint32_t checksum = crc32(data + 4, 8);
  checksum = crc32(data + kHeaderBytes, payloadSize, checksum);
  if ((checksum ^ 0xFFFFFFFFu) != readU32(data + 12)) {
    return DecodeStatus::BadChecksum;
  }

  output.type = static_cast<RecordType>(data[5]);
  output.sequence = readU32(data + 8);
  output.payload = data + kHeaderBytes;
  output.payloadSize = payloadSize;
  output.totalSize = totalSize;
  return DecodeStatus::Ok;
}

bool encodeEntry(const LearnerEntry& entry, PayloadBuffer& output) {
  PayloadWriter writer(output);
  return encodeEntryToWriter(entry, writer);
}

bool decodeEntry(const uint8_t* data, size_t size, LearnerEntry& output) {
  if (data == nullptr) return false;
  PayloadReader reader(data, size);
  LearnerEntry decoded;
  if (!decodeEntryFromReader(reader, decoded) || !reader.done()) return false;
  output = std::move(decoded);
  return true;
}

}  // namespace ChinesePoint::Cjk::Journal
