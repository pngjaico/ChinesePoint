// Derived from ma-r-s/crossplay StudyFsrs.cpp at commit
// 858a0bf1345b4bb65db0f8499f78ec7c61d36b3b. Copyright and MIT terms are in
// THIRD-PARTY-NOTICES.md.
#include "chinesepoint/study/StudyFsrs5.h"

#include <cmath>

namespace ChinesePoint::Study {

const float kFsrs5DefaultParameters[kFsrs5ParameterCount] = {
    0.40255f, 1.18385f, 3.173f,  15.69105f, 7.1949f, 0.5345f, 1.4604f,
    0.0046f,  1.54575f, 0.1192f, 1.01925f,  1.9395f, 0.11f,   0.29605f,
    2.2698f,  0.2315f,  2.9898f, 0.51655f,  0.6621f,
};

namespace {

constexpr float kDecay = -0.5f;
const float kFactor = std::pow(0.9f, 1.0f / kDecay) - 1.0f;
constexpr float kMinimumStability = 0.01f;
constexpr float kMaximumStability = 36500.0f;

float clamp(const float value, const float minimum, const float maximum) {
  return value < minimum ? minimum : (value > maximum ? maximum : value);
}

int ratingIndex(const Rating rating) { return static_cast<int>(rating); }

}  // namespace

Fsrs5::Fsrs5(const float* parameters, const float desiredRetention)
    : parameters_(parameters != nullptr ? parameters : kFsrs5DefaultParameters), desiredRetention_(desiredRetention) {}

float Fsrs5::initialStability(const Rating rating) const {
  return clamp(parameters_[ratingIndex(rating) - 1], kMinimumStability, kMaximumStability);
}

float Fsrs5::initialDifficulty(const Rating rating) const {
  return clamp(parameters_[4] - std::exp(parameters_[5] * static_cast<float>(ratingIndex(rating) - 1)) + 1.0f, 1.0f,
               10.0f);
}

float Fsrs5::nextDifficulty(const float difficulty, const Rating rating) const {
  const float delta = -parameters_[6] * static_cast<float>(ratingIndex(rating) - 3);
  const float damped = difficulty + delta * ((10.0f - difficulty) / 9.0f);
  const float reverted = parameters_[7] * initialDifficulty(Rating::Easy) + (1.0f - parameters_[7]) * damped;
  return clamp(reverted, 1.0f, 10.0f);
}

float Fsrs5::shortTermStability(const float stability, const Rating rating) const {
  float increase = std::exp(parameters_[17] * (static_cast<float>(ratingIndex(rating)) - 3.0f + parameters_[18]));
  if (ratingIndex(rating) >= 3) {
    if (increase < 1.0f) increase = 1.0f;
  } else if (increase > 1.0f) {
    increase = 1.0f;
  }
  return stability * increase;
}

float Fsrs5::recallStability(const float difficulty, const float stability, const float retrievability,
                             const Rating rating) const {
  const float hardPenalty = rating == Rating::Hard ? parameters_[15] : 1.0f;
  const float easyBonus = rating == Rating::Easy ? parameters_[16] : 1.0f;
  return stability *
         (1.0f + std::exp(parameters_[8]) * (11.0f - difficulty) * std::pow(stability, -parameters_[9]) *
                       (std::exp((1.0f - retrievability) * parameters_[10]) - 1.0f) * hardPenalty * easyBonus);
}

float Fsrs5::forgetStability(const float difficulty, const float stability, const float retrievability) const {
  return parameters_[11] * std::pow(difficulty, -parameters_[12]) *
         (std::pow(stability + 1.0f, parameters_[13]) - 1.0f) * std::exp((1.0f - retrievability) * parameters_[14]);
}

float Fsrs5::retrievability(const FsrsMemory& memory, const float elapsedDays) const {
  if (!memory.learned || memory.stability <= 0.0f) return 1.0f;
  const float elapsed = elapsedDays < 0.0f ? 0.0f : elapsedDays;
  return std::pow(1.0f + kFactor * elapsed / memory.stability, kDecay);
}

FsrsMemory Fsrs5::review(const FsrsMemory& memory, const Rating rating, const int elapsedDays) const {
  FsrsMemory output{};
  output.learned = true;
  if (!memory.learned) {
    output.stability = initialStability(rating);
    output.difficulty = initialDifficulty(rating);
    return output;
  }

  if (elapsedDays <= 0) {
    output.stability = shortTermStability(memory.stability, rating);
  } else {
    const float retention = retrievability(memory, static_cast<float>(elapsedDays));
    if (rating == Rating::Again) {
      const float forgotten = forgetStability(memory.difficulty, memory.stability, retention);
      output.stability = forgotten < memory.stability ? forgotten : memory.stability;
    } else {
      output.stability = recallStability(memory.difficulty, memory.stability, retention, rating);
    }
  }
  output.stability = clamp(output.stability, kMinimumStability, kMaximumStability);
  output.difficulty = nextDifficulty(memory.difficulty, rating);
  return output;
}

int Fsrs5::intervalDays(const FsrsMemory& memory) const {
  if (!memory.learned || memory.stability <= 0.0f) return 1;
  const float days = (memory.stability / kFactor) * (std::pow(desiredRetention_, 1.0f / kDecay) - 1.0f);
  const int rounded = static_cast<int>(days + 0.5f);
  if (rounded < 1) return 1;
  return rounded > maximumInterval_ ? maximumInterval_ : rounded;
}

void Fsrs5::previewIntervals(const FsrsMemory& memory, const int elapsedDays, int output[4]) const {
  for (int index = 0; index < 4; ++index) {
    output[index] = intervalDays(review(memory, static_cast<Rating>(index + 1), elapsedDays));
  }
}

}  // namespace ChinesePoint::Study
