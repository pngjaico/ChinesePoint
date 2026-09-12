#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ChinesePoint::RecoveryBackup {

constexpr size_t kSha256HexLength = 64;
constexpr size_t kSha256Bytes = kSha256HexLength / 2;

// The automatic recovery route has an intentionally narrow SD-card contract:
// exactly 64 lowercase hexadecimal characters, with no filename, whitespace,
// or alternate digest syntax. A manual update remains available for every
// other supported firmware file.
inline bool parseExactLowercaseSha256(const std::string_view encoded, std::array<uint8_t, kSha256Bytes>& digest) {
  if (encoded.size() != kSha256HexLength) return false;

  for (size_t i = 0; i < encoded.size(); ++i) {
    const char character = encoded[i];
    if (!((character >= '0' && character <= '9') || (character >= 'a' && character <= 'f'))) return false;
    const uint8_t nibble = static_cast<uint8_t>(character <= '9' ? character - '0' : character - 'a' + 10);
    if ((i & 1U) == 0) {
      digest[i / 2] = static_cast<uint8_t>(nibble << 4U);
    } else {
      digest[i / 2] |= nibble;
    }
  }
  return true;
}

}  // namespace ChinesePoint::RecoveryBackup
