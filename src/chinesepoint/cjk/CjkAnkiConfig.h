#pragma once

#include <string>

namespace ChinesePoint::Cjk {

struct AnkiBridgeConfig {
  std::string serverUrl;
  std::string apiToken;
  std::string clientId;

  bool configured() const;
};

// Credentials live in ESP NVS rather than the removable SD card. This is not
// hardware-backed encryption, but it avoids copying a LAN bearer token into
// user exports or the ordinary file browser.
class AnkiBridgeConfigStore final {
 public:
  bool load();
  bool save();
  bool setServerUrl(const std::string& value);
  bool setApiToken(const std::string& value);
  const AnkiBridgeConfig& config() const { return config_; }

 private:
  bool ensureClientId();
  AnkiBridgeConfig config_;
};

AnkiBridgeConfigStore& ankiBridgeConfigStore();

}  // namespace ChinesePoint::Cjk
