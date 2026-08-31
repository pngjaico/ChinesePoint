#pragma once

#include <cstddef>

#include "chinesepoint/cjk/CjkLearnerModel.h"

namespace ChinesePoint::Cjk {

// A streaming NDJSON writer. The export layer owns neither SD storage nor UI,
// so its encoding can be checked on the host and reused by a future Anki
// bridge. Returning false asks the exporter to stop immediately.
using ExportWriteCallback = bool (*)(void* context, const char* bytes, size_t size);

struct ExportSink {
  void* context = nullptr;
  ExportWriteCallback write = nullptr;
};

// Writes a deterministic, versioned NDJSON stream. It rejects invalid UTF-8
// and non-finite scheduling numbers instead of emitting a syntactically valid
// but semantically corrupted backup. It never mutates learner state.
bool writeLearnerExportJsonl(const LearnerEntry* entries, size_t entryCount, ExportSink sink);

}  // namespace ChinesePoint::Cjk
