#pragma once

#include <cstddef>
#include <functional>

namespace ChinesePoint::Cjk {

// Installs one reviewed CC-CEDICT release only. It is deliberately not a
// generic archive installer: arbitrary ZIP paths or URLs would broaden the
// firmware's attack surface and make rollback guarantees impossible to audit.
class CcCedictInstaller final {
 public:
  enum class Result { Installed, AlreadyInstalled, DownloadFailed, ChecksumFailed, ArchiveInvalid, StorageFailed, Cancelled };
  using ProgressCallback = std::function<void(size_t completed, size_t total)>;

  Result install(const ProgressCallback& progress = nullptr, bool* cancelRequested = nullptr) const;
  // True only when the fixed target contains all three reviewed files with
  // their published sizes and CRCs. A mere directory is not an installation.
  static bool isInstalled();
  static constexpr const char* dictionaryFolder() { return "cc-cedict-20260731"; }
};

}  // namespace ChinesePoint::Cjk
