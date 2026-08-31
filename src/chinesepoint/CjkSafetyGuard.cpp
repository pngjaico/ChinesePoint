#include "CjkSafetyGuard.h"

#if defined(CHINESEPOINT)

#include <Preferences.h>

namespace ChinesePoint::CjkSafetyGuard {
namespace {

constexpr char kNamespace[] = "cpcjk";
constexpr char kSessionActive[] = "active";
constexpr char kCrashStreak[] = "crashes";
constexpr char kSafeMode[] = "safe";
constexpr uint8_t kSafeModeThreshold = 2;

Status failOpen() { return {.storageAvailable = false, .safeMode = false, .crashStreak = 0}; }

bool writeState(Preferences& prefs, bool sessionActive, uint8_t crashStreak, bool safeMode) {
  return prefs.putBool(kSessionActive, sessionActive) == 1 && prefs.putUChar(kCrashStreak, crashStreak) == 1 &&
         prefs.putBool(kSafeMode, safeMode) == 1;
}

}  // namespace

Status reconcileBoot(bool rebootedFromPanic) {
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return failOpen();

  const bool sessionWasActive = prefs.getBool(kSessionActive, false);
  uint8_t crashStreak = prefs.getUChar(kCrashStreak, 0);
  const bool safeModeWasSet = prefs.getBool(kSafeMode, false);

  if (rebootedFromPanic && sessionWasActive) {
    if (crashStreak < kSafeModeThreshold) ++crashStreak;
  } else {
    // A normal boot or a panic outside CJK breaks the CJK crash sequence.
    crashStreak = 0;
  }

  const bool safeMode = safeModeWasSet || crashStreak >= kSafeModeThreshold;
  const bool persisted = writeState(prefs, false, crashStreak, safeMode);
  prefs.end();
  return persisted ? Status{.storageAvailable = true, .safeMode = safeMode, .crashStreak = crashStreak} : failOpen();
}

bool startLearnerSession() {
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return true;

  const bool safeMode = prefs.getBool(kSafeMode, false);
  if (!safeMode) {
    // The marker is best-effort. If NVS cannot write, optional learner state
    // must not deny access or influence the core device boot route.
    prefs.putBool(kSessionActive, true);
  }
  prefs.end();
  return !safeMode;
}

void finishLearnerSession() {
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return;
  writeState(prefs, false, 0, prefs.getBool(kSafeMode, false));
  prefs.end();
}

void clearSafeMode() {
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return;
  writeState(prefs, false, 0, false);
  prefs.end();
}

}  // namespace ChinesePoint::CjkSafetyGuard

#endif  // defined(CHINESEPOINT)
