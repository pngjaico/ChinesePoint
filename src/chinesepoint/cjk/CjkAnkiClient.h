#pragma once

#include <cstddef>
#include <functional>

#include "chinesepoint/cjk/CjkAnkiConfig.h"

namespace ChinesePoint::Cjk {

class LearnerStore;

// Pushes the existing, versioned learner NDJSON export to the companion Anki
// Desktop add-on. It never imports arbitrary server data and never edits the
// learner journal or local scheduler state.
class CjkAnkiClient final {
 public:
  enum class Result { Ok, Cancelled, NotConfigured, ExportFailed, ConnectionFailed, ProtocolFailed, Rejected };
  using ProgressCallback = std::function<void(size_t completed, size_t total)>;

  Result pushVocabulary(LearnerStore& store, const AnkiBridgeConfig& config, const ProgressCallback& progress = nullptr,
                        bool* cancelRequested = nullptr) const;
};

}  // namespace ChinesePoint::Cjk
