#pragma once

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

class HalFile {
 public:
  HalFile() = default;
  explicit HalFile(std::filesystem::path path) : path_(std::move(path)), open_(std::filesystem::exists(path_)) {}

  explicit operator bool() const { return open_; }
  bool isDirectory() const { return open_ && std::filesystem::is_directory(path_); }

  void rewindDirectory() {
    entries_.clear();
    index_ = 0;
    if (!isDirectory()) return;
    for (const auto& entry : std::filesystem::directory_iterator(path_)) entries_.push_back(entry.path());
    std::sort(entries_.begin(), entries_.end());
  }

  HalFile openNextFile() {
    if (entries_.empty() && index_ == 0) rewindDirectory();
    if (index_ >= entries_.size()) return {};
    return HalFile(entries_[index_++]);
  }

  size_t getName(char* name, const size_t length) const {
    if (!name || length == 0) return 0;
    const std::string leaf = path_.filename().string();
    const size_t copied = std::min(leaf.size(), length - 1);
    std::memcpy(name, leaf.data(), copied);
    name[copied] = '\0';
    return copied;
  }

 private:
  std::filesystem::path path_;
  bool open_ = false;
  std::vector<std::filesystem::path> entries_;
  size_t index_ = 0;
};

class HalStorage {
 public:
  static HalStorage& getInstance() {
    static HalStorage instance;
    return instance;
  }

  void setTestRoot(std::filesystem::path root) { root_ = std::move(root); }

  HalFile open(const char* path) const { return HalFile(resolve(path)); }
  bool exists(const char* path) const { return std::filesystem::exists(resolve(path)); }

 private:
  std::filesystem::path resolve(const char* path) const {
    std::filesystem::path relative = path ? path : "";
    if (relative.is_absolute()) relative = relative.relative_path();
    return root_ / relative;
  }

  std::filesystem::path root_;
};

#define Storage HalStorage::getInstance()
