#include "chinesepoint/cjk/CjkLearnerExport.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string_view>

namespace ChinesePoint::Cjk {
namespace {

bool writeBytes(const ExportSink sink, const char* bytes, const size_t size) {
  return sink.write != nullptr && (size == 0 || sink.write(sink.context, bytes, size));
}

bool writeLiteral(const ExportSink sink, const char* text) { return writeBytes(sink, text, std::strlen(text)); }

bool validUtf8(const std::string_view text) {
  for (size_t i = 0; i < text.size();) {
    const uint8_t first = static_cast<uint8_t>(text[i]);
    if (first <= 0x7F) {
      ++i;
      continue;
    }
    size_t continuationCount = 0;
    uint32_t codepoint = 0;
    if (first >= 0xC2 && first <= 0xDF) {
      continuationCount = 1;
      codepoint = first & 0x1Fu;
    } else if (first >= 0xE0 && first <= 0xEF) {
      continuationCount = 2;
      codepoint = first & 0x0Fu;
    } else if (first >= 0xF0 && first <= 0xF4) {
      continuationCount = 3;
      codepoint = first & 0x07u;
    } else {
      return false;
    }
    if (continuationCount > text.size() - i - 1) return false;
    for (size_t j = 1; j <= continuationCount; ++j) {
      const uint8_t next = static_cast<uint8_t>(text[i + j]);
      if ((next & 0xC0u) != 0x80u) return false;
      codepoint = (codepoint << 6u) | (next & 0x3Fu);
    }
    // Reject overlong encodings, surrogate halves, and values outside Unicode.
    if ((continuationCount == 2 && codepoint < 0x800u) || (continuationCount == 3 && codepoint < 0x10000u) ||
        (continuationCount == 2 && codepoint >= 0xD800u && codepoint <= 0xDFFFu) || codepoint > 0x10FFFFu) {
      return false;
    }
    i += continuationCount + 1;
  }
  return true;
}

bool writeJsonString(const ExportSink sink, const std::string_view value) {
  if (!validUtf8(value) || !writeLiteral(sink, "\"")) return false;
  for (const unsigned char byte : value) {
    switch (byte) {
      case '\"':
        if (!writeLiteral(sink, "\\\"")) return false;
        break;
      case '\\':
        if (!writeLiteral(sink, "\\\\")) return false;
        break;
      case '\b':
        if (!writeLiteral(sink, "\\b")) return false;
        break;
      case '\f':
        if (!writeLiteral(sink, "\\f")) return false;
        break;
      case '\n':
        if (!writeLiteral(sink, "\\n")) return false;
        break;
      case '\r':
        if (!writeLiteral(sink, "\\r")) return false;
        break;
      case '\t':
        if (!writeLiteral(sink, "\\t")) return false;
        break;
      default:
        if (byte < 0x20u) {
          char escaped[7];
          const int count = std::snprintf(escaped, sizeof(escaped), "\\u%04x", byte);
          if (count != 6 || !writeBytes(sink, escaped, static_cast<size_t>(count))) return false;
        } else if (!writeBytes(sink, reinterpret_cast<const char*>(&byte), 1)) {
          return false;
        }
        break;
    }
  }
  return writeLiteral(sink, "\"");
}

bool writeUnsigned(const ExportSink sink, const uint64_t value) {
  char text[24];
  const int count = std::snprintf(text, sizeof(text), "%llu", static_cast<unsigned long long>(value));
  return count > 0 && static_cast<size_t>(count) < sizeof(text) && writeBytes(sink, text, static_cast<size_t>(count));
}

bool writeSigned(const ExportSink sink, const int64_t value) {
  char text[24];
  const int count = std::snprintf(text, sizeof(text), "%lld", static_cast<long long>(value));
  return count > 0 && static_cast<size_t>(count) < sizeof(text) && writeBytes(sink, text, static_cast<size_t>(count));
}

bool writeFloat(const ExportSink sink, const float value) {
  if (!std::isfinite(value)) return false;
  char text[24];
  const int count = std::snprintf(text, sizeof(text), "%.9g", static_cast<double>(value));
  return count > 0 && static_cast<size_t>(count) < sizeof(text) && writeBytes(sink, text, static_cast<size_t>(count));
}

const char* statusName(const WordStatus status) {
  switch (status) {
    case WordStatus::Encountered:
      return "encountered";
    case WordStatus::Saved:
      return "saved";
    case WordStatus::Learning:
      return "learning";
    case WordStatus::Known:
      return "known";
  }
  return nullptr;
}

const char* phaseName(const ReviewPhase phase) {
  switch (phase) {
    case ReviewPhase::New:
      return "new";
    case ReviewPhase::Learning:
      return "learning";
    case ReviewPhase::Review:
      return "review";
    case ReviewPhase::Relearning:
      return "relearning";
  }
  return nullptr;
}

const char* authorityName(const ScheduleAuthority authority) {
  switch (authority) {
    case ScheduleAuthority::Local:
      return "local";
    case ScheduleAuthority::Anki:
      return "anki";
  }
  return nullptr;
}

bool writeEntry(const LearnerEntry& entry, const ExportSink sink) {
  const char* status = statusName(entry.status);
  const char* phase = phaseName(entry.review.phase);
  const char* authority = authorityName(entry.review.authority);
  if (status == nullptr || phase == nullptr || authority == nullptr || !validHeadword(entry.headword) ||
      !validSentence(entry.sourceSentence) || !validBookPath(entry.bookPath)) {
    return false;
  }

  return writeLiteral(sink, "{\"type\":\"vocabulary\",\"word_id\":\"") &&
         [&] {
           char id[17];
           const int count = std::snprintf(id, sizeof(id), "%016llx", static_cast<unsigned long long>(entry.wordId));
           return count == 16 && writeBytes(sink, id, static_cast<size_t>(count));
         }() &&
         writeLiteral(sink, "\",\"headword\":") && writeJsonString(sink, entry.headword) &&
         writeLiteral(sink, ",\"status\":\"") && writeLiteral(sink, status) &&
         writeLiteral(sink, "\",\"encounters\":") && writeUnsigned(sink, entry.encounterCount) &&
         writeLiteral(sink, ",\"first_seen_study_ms\":") && writeSigned(sink, entry.firstSeenStudyMs) &&
         writeLiteral(sink, ",\"last_seen_study_ms\":") && writeSigned(sink, entry.lastSeenStudyMs) &&
         writeLiteral(sink, ",\"source\":{\"book_path\":") && writeJsonString(sink, entry.bookPath) &&
         writeLiteral(sink, ",\"spine_index\":") && writeUnsigned(sink, entry.sourceAnchor.spineIndex) &&
         writeLiteral(sink, ",\"visible_codepoint_offset\":") &&
         writeUnsigned(sink, entry.sourceAnchor.visibleCodepointOffset) && writeLiteral(sink, ",\"codepoint_length\":") &&
         writeUnsigned(sink, entry.sourceAnchor.codepointLength) && writeLiteral(sink, ",\"fingerprint\":") &&
         writeUnsigned(sink, entry.sourceAnchor.fingerprint) && writeLiteral(sink, "},\"sentence\":") &&
         writeJsonString(sink, entry.sourceSentence) && writeLiteral(sink, ",\"review\":{\"phase\":\"") &&
         writeLiteral(sink, phase) && writeLiteral(sink, "\",\"authority\":\"") && writeLiteral(sink, authority) &&
         writeLiteral(sink, "\",\"due_at_ms\":") && writeSigned(sink, entry.review.dueAtMs) &&
         writeLiteral(sink, ",\"last_review_at_ms\":") && writeSigned(sink, entry.review.lastReviewAtMs) &&
         writeLiteral(sink, ",\"difficulty\":") && writeFloat(sink, entry.review.difficulty) &&
         writeLiteral(sink, ",\"stability_days\":") && writeFloat(sink, entry.review.stabilityDays) &&
         writeLiteral(sink, ",\"reps\":") && writeUnsigned(sink, entry.review.reps) &&
         writeLiteral(sink, ",\"lapses\":") && writeUnsigned(sink, entry.review.lapses) &&
         writeLiteral(sink, ",\"anki_card_id\":\"") && writeUnsigned(sink, entry.review.ankiCardId) &&
         writeLiteral(sink, "\"}}\n");
}

}  // namespace

bool writeLearnerExportJsonl(const LearnerEntry* const entries, const size_t entryCount, const ExportSink sink) {
  if (sink.write == nullptr || (entryCount > 0 && entries == nullptr) ||
      !writeLiteral(sink, "{\"schema\":\"chinesepoint-learner-export\",\"version\":1,\"format\":\"ndjson\"}\n")) {
    return false;
  }
  for (size_t index = 0; index < entryCount; ++index) {
    if (!writeEntry(entries[index], sink)) return false;
  }
  return true;
}

}  // namespace ChinesePoint::Cjk
