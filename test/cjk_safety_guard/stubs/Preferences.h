#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

class Preferences {
 public:
  bool begin(const char* name, bool) {
    namespace_ = name;
    return beginSucceeds_;
  }

  void end() { namespace_.clear(); }

  bool getBool(const char* key, bool defaultValue = false) const {
    const auto it = values_.find(composeKey(key));
    return it == values_.end() ? defaultValue : it->second != 0;
  }

  uint8_t getUChar(const char* key, uint8_t defaultValue = 0) const {
    const auto it = values_.find(composeKey(key));
    return it == values_.end() ? defaultValue : it->second;
  }

  std::size_t putBool(const char* key, bool value) {
    if (!writesSucceed_) return 0;
    values_[composeKey(key)] = value ? 1 : 0;
    return 1;
  }

  std::size_t putUChar(const char* key, uint8_t value) {
    if (!writesSucceed_) return 0;
    values_[composeKey(key)] = value;
    return 1;
  }

  static void reset() {
    values_.clear();
    beginSucceeds_ = true;
    writesSucceed_ = true;
  }
  static void setBeginSucceeds(bool value) { beginSucceeds_ = value; }
  static void setWritesSucceed(bool value) { writesSucceed_ = value; }

 private:
  std::string composeKey(const char* key) const { return namespace_ + ":" + key; }

  std::string namespace_;
  inline static std::map<std::string, uint8_t> values_;
  inline static bool beginSucceeds_ = true;
  inline static bool writesSucceed_ = true;
};
