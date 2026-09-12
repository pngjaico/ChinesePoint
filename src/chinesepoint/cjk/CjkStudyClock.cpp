#include "chinesepoint/cjk/CjkStudyClock.h"

#include <algorithm>
#include <limits>

namespace ChinesePoint::Cjk {

bool validStudyClockState(const StudyClockState& state) {
  return state.logicalMs >= 0 && state.lastTrustedWallMs >= 0 && state.lastTrustedWallMs <= state.logicalMs;
}

StudyClock::StudyClock(const StudyClockState persisted, const int64_t bootUptimeMs)
    : state_(validStudyClockState(persisted) ? persisted : StudyClockState{}),
      bootUptimeMs_(std::max<int64_t>(0, bootUptimeMs)),
      bootLogicalMs_(state_.logicalMs) {}

StudyClockReading StudyClock::observe(const int64_t uptimeMs, const int64_t wallClockMs) {
  const int64_t safeUptime = std::max(bootUptimeMs_, uptimeMs);
  const int64_t sessionElapsed = safeUptime - bootUptimeMs_;
  int64_t candidate = bootLogicalMs_;
  if (sessionElapsed <= std::numeric_limits<int64_t>::max() - candidate) candidate += sessionElapsed;
  else candidate = std::numeric_limits<int64_t>::max();

  const bool wallClockTrusted = wallClockMs >= kMinTrustedWallClockMs;
  if (wallClockTrusted) {
    // Never allow a manual/NTP rollback to make previously scheduled cards
    // appear due early. A corrected clock catches up naturally.
    state_.lastTrustedWallMs = std::max(state_.lastTrustedWallMs, wallClockMs);
    candidate = std::max(candidate, state_.lastTrustedWallMs);
  }
  state_.logicalMs = std::max(state_.logicalMs, candidate);
  return {.nowMs = state_.logicalMs, .wallClockTrusted = wallClockTrusted, .state = state_};
}

}  // namespace ChinesePoint::Cjk
