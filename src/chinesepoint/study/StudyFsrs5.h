#pragma once

// Derived from ma-r-s/crossplay StudyFsrs at commit
// 858a0bf1345b4bb65db0f8499f78ec7c61d36b3b. Copyright and MIT terms are in
// THIRD-PARTY-NOTICES.md.

#include <cstdint>

#include "chinesepoint/study/StudyTypes.h"

namespace ChinesePoint::Study {

// FSRS-5 compatibility engine. It accepts exactly 19 parameters; FSRS-6 is
// intentionally not routed here because it has different semantics.
inline constexpr int kFsrs5ParameterCount = 19;
extern const float kFsrs5DefaultParameters[kFsrs5ParameterCount];

struct FsrsMemory {
  float stability = 0.0f;
  float difficulty = 0.0f;
  bool learned = false;
};

class Fsrs5 final {
 public:
  explicit Fsrs5(const float* parameters = nullptr, float desiredRetention = 0.9f);

  float retrievability(const FsrsMemory& memory, float elapsedDays) const;
  FsrsMemory review(const FsrsMemory& memory, Rating rating, int elapsedDays) const;
  int intervalDays(const FsrsMemory& memory) const;
  void previewIntervals(const FsrsMemory& memory, int elapsedDays, int output[4]) const;

  float desiredRetention() const { return desiredRetention_; }
  void setDesiredRetention(float value) { desiredRetention_ = value; }
  void setMaximumInterval(int days) { maximumInterval_ = days; }

 private:
  float initialStability(Rating rating) const;
  float initialDifficulty(Rating rating) const;
  float nextDifficulty(float difficulty, Rating rating) const;
  float shortTermStability(float stability, Rating rating) const;
  float recallStability(float difficulty, float stability, float retrievability, Rating rating) const;
  float forgetStability(float difficulty, float stability, float retrievability) const;

  const float* parameters_;
  float desiredRetention_;
  int maximumInterval_ = 36500;
};

}  // namespace ChinesePoint::Study
