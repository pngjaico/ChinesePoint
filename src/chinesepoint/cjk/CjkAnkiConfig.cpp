#include "chinesepoint/cjk/CjkAnkiConfig.h"

#include "chinesepoint/cjk/CjkAnkiProtocol.h"

#if !defined(SIMULATOR)
#include <Preferences.h>
#include <esp_system.h>
#endif

#include <cstdio>

namespace ChinesePoint::Cjk {
namespace {

constexpr char kNamespace[] = "cpanki";
constexpr char kUrlKey[] = "url";
constexpr char kTokenKey[] = "token";
constexpr char kClientIdKey[] = "client";

#if defined(SIMULATOR)
AnkiBridgeConfig simulatorConfig;
#endif

}  // namespace

bool AnkiBridgeConfig::configured() const {
  AnkiBridgeUrl parsed;
  return parseAnkiBridgeUrl(serverUrl, parsed) && validAnkiBridgeToken(apiToken) && validAnkiBridgeClientId(clientId);
}

bool AnkiBridgeConfigStore::ensureClientId() {
  if (validAnkiBridgeClientId(config_.clientId)) return true;
#if defined(SIMULATOR)
  config_.clientId = "0123456789abcdef0123456789abcdef";
  return true;
#else
  char id[33];
  const uint32_t first = esp_random();
  const uint32_t second = esp_random();
  const uint32_t third = esp_random();
  const uint32_t fourth = esp_random();
  const int written = std::snprintf(id, sizeof(id), "%08lx%08lx%08lx%08lx", static_cast<unsigned long>(first),
                                    static_cast<unsigned long>(second), static_cast<unsigned long>(third),
                                    static_cast<unsigned long>(fourth));
  if (written != 32) return false;
  config_.clientId = id;
  return true;
#endif
}

bool AnkiBridgeConfigStore::load() {
#if defined(SIMULATOR)
  config_ = simulatorConfig;
  return ensureClientId();
#else
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return false;
  config_.serverUrl = prefs.getString(kUrlKey, "").c_str();
  config_.apiToken = prefs.getString(kTokenKey, "").c_str();
  config_.clientId = prefs.getString(kClientIdKey, "").c_str();
  prefs.end();
  const bool hadClientId = validAnkiBridgeClientId(config_.clientId);
  if (!ensureClientId()) return false;
  return hadClientId || save();
#endif
}

bool AnkiBridgeConfigStore::save() {
  if (!ensureClientId()) return false;
#if defined(SIMULATOR)
  simulatorConfig = config_;
  return true;
#else
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return false;
  const bool ok = prefs.putString(kUrlKey, config_.serverUrl.c_str()) == config_.serverUrl.size() &&
                  prefs.putString(kTokenKey, config_.apiToken.c_str()) == config_.apiToken.size() &&
                  prefs.putString(kClientIdKey, config_.clientId.c_str()) == config_.clientId.size();
  prefs.end();
  return ok;
#endif
}

bool AnkiBridgeConfigStore::setServerUrl(const std::string& value) {
  if (!value.empty()) {
    AnkiBridgeUrl parsed;
    if (!parseAnkiBridgeUrl(value, parsed)) return false;
  }
  config_.serverUrl = value;
  return true;
}

bool AnkiBridgeConfigStore::setApiToken(const std::string& value) {
  if (!value.empty() && !validAnkiBridgeToken(value)) return false;
  config_.apiToken = value;
  return true;
}

AnkiBridgeConfigStore& ankiBridgeConfigStore() {
  static AnkiBridgeConfigStore store;
  return store;
}

}  // namespace ChinesePoint::Cjk
