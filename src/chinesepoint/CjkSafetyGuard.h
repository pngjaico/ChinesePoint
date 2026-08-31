#pragma once

#include <cstdint>

namespace ChinesePoint::CjkSafetyGuard {

// This guard owns only the learner feature's crash marker. It never changes the
// firmware recovery route and must be consulted before starting any learner UI.
struct Status {
  bool storageAvailable;
  bool safeMode;
  uint8_t crashStreak;

  bool learnerEnabled() const { return !safeMode; }
};

// Reconcile the prior learner-session marker once normal boot classification is
// complete. Storage failures deliberately leave the learner enabled; a failed
// optional feature must never alter boot or recovery behavior.
Status reconcileBoot(bool rebootedFromPanic);

// Start returns false only for an already-persisted learner safe mode. A storage
// failure is fail-open so the device cannot be trapped by optional CJK state.
bool startLearnerSession();

// A clean exit breaks the consecutive-crash sequence. It does not clear a safe
// mode; that is an intentional, separately-confirmed recovery action.
void finishLearnerSession();

// For a future settings/recovery UI after the learner data has been reviewed.
void clearSafeMode();

}  // namespace ChinesePoint::CjkSafetyGuard
