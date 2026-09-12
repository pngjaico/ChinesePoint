#pragma once

#include <cstdint>

namespace ChinesePoint::Cjk {

// The X4 Pro may have no trusted wall clock until Wi-Fi has completed an NTP
// sync. Review due dates therefore use this monotonic logical clock instead of
// raw millis(), which restarts at zero on every boot.
constexpr int64_t kMinTrustedWallClockMs = 1704067200000LL;  // 2024-01-01 UTC

struct StudyClockState {
  int64_t logicalMs = 0;
  int64_t lastTrustedWallMs = 0;
};

bool validStudyClockState(const StudyClockState& state);

struct StudyClockReading {
  int64_t nowMs = 0;
  bool wallClockTrusted = false;
  StudyClockState state{};
};

// Session-local adapter around a durable state. A caller persists state() when
// it commits a review mutation. With no wall time, powered-off time deliberately
// does not advance: that is conservative, stable across reboot, and preferable
// to making every card immediately due from a reset millis() counter.
class StudyClock final {
 public:
  explicit StudyClock(StudyClockState persisted = {}, int64_t bootUptimeMs = 0);

  StudyClockReading observe(int64_t uptimeMs, int64_t wallClockMs = 0);
  const StudyClockState& state() const { return state_; }

 private:
  StudyClockState state_;
  int64_t bootUptimeMs_ = 0;
  int64_t bootLogicalMs_ = 0;
};

}  // namespace ChinesePoint::Cjk
