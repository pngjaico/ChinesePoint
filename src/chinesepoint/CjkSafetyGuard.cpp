#include "CjkSafetyGuard.h"

#if defined(CHINESEPOINT)

#if !defined(SIMULATOR)
#include <Preferences.h>
#endif

namespace ChinesePoint::CjkSafetyGuard {
namespace {

constexpr uint8_t kSafeModeThreshold = 2;

struct PersistedState {
  bool sessionActive = false;
  uint8_t crashStreak = 0;
  bool safeMode = false;
};

#if !defined(SIMULATOR)
Status failOpen() { return {.storageAvailable = false, .safeMode = false, .crashStreak = 0}; }
#endif

Status toStatus(const PersistedState& state, bool storageAvailable) {
  return {.storageAvailable = storageAvailable, .safeMode = state.safeMode, .crashStreak = state.crashStreak};
}

void reconcileState(PersistedState& state, bool rebootedFromPanic) {
  if (rebootedFromPanic && state.sessionActive) {
    if (state.crashStreak < kSafeModeThreshold) ++state.crashStreak;
  } else {
    // A normal boot or a panic outside CJK breaks the CJK crash sequence.
    state.crashStreak = 0;
  }
  state.sessionActive = false;
  state.safeMode = state.safeMode || state.crashStreak >= kSafeModeThreshold;
}

#if defined(SIMULATOR)

// The official simulator has no ESP NVS implementation. This is intentionally
// process-local: it exercises the learner route without claiming reboot-safe
// persistence on a real device.
PersistedState simulatorState;

#else

constexpr char kNamespace[] = "cpcjk";
constexpr char kSessionActive[] = "active";
constexpr char kCrashStreak[] = "crashes";
constexpr char kSafeMode[] = "safe";

PersistedState readState(Preferences& prefs) {
  return {
      .sessionActive = prefs.getBool(kSessionActive, false),
      .crashStreak = prefs.getUChar(kCrashStreak, 0),
      .safeMode = prefs.getBool(kSafeMode, false),
  };
}

bool writeState(Preferences& prefs, const PersistedState& state) {
  return prefs.putBool(kSessionActive, state.sessionActive) == 1 &&
         prefs.putUChar(kCrashStreak, state.crashStreak) == 1 && prefs.putBool(kSafeMode, state.safeMode) == 1;
}

#endif

}  // namespace

Status reconcileBoot(bool rebootedFromPanic) {
#if defined(SIMULATOR)
  reconcileState(simulatorState, rebootedFromPanic);
  return toStatus(simulatorState, true);
#else
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return failOpen();

  PersistedState state = readState(prefs);
  reconcileState(state, rebootedFromPanic);
  const bool persisted = writeState(prefs, state);
  prefs.end();
  return persisted ? toStatus(state, true) : failOpen();
#endif
}

bool startLearnerSession() {
#if defined(SIMULATOR)
  if (simulatorState.safeMode) return false;
  simulatorState.sessionActive = true;
  return true;
#else
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return true;

  PersistedState state = readState(prefs);
  if (!state.safeMode) {
    // The marker is best-effort. If NVS cannot write, optional learner state
    // must not deny access or influence the core device boot route.
    state.sessionActive = true;
    writeState(prefs, state);
  }
  prefs.end();
  return !state.safeMode;
#endif
}

void finishLearnerSession() {
#if defined(SIMULATOR)
  simulatorState.sessionActive = false;
  simulatorState.crashStreak = 0;
#else
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return;
  PersistedState state = readState(prefs);
  state.sessionActive = false;
  state.crashStreak = 0;
  writeState(prefs, state);
  prefs.end();
#endif
}

void clearSafeMode() {
#if defined(SIMULATOR)
  simulatorState = {};
#else
  Preferences prefs;
  if (!prefs.begin(kNamespace, false)) return;
  const bool persisted = writeState(prefs, {});
  (void)persisted;
  prefs.end();
#endif
}

}  // namespace ChinesePoint::CjkSafetyGuard

#endif  // defined(CHINESEPOINT)
