# X4 Pro simulator controller build check — 2026-09-12

## Source under test

The WSL emulator checkout fetched `origin/feature/x4pro-continuacao` and was
checked out detached at `581962b` (`feat: persist local review mutations`).
This includes the durable learner study clock and atomic local review journal
mutation, but no user-facing review activity yet.

## Build results

Each X4 Pro controller profile completed PlatformIO's final native link step:

| Profile | Result | Duration |
| --- | --- | --- |
| `chinesepoint_simulator_x4pro_ssd1677` | success | 9.164 s (incremental) |
| `chinesepoint_simulator_x4pro_uc8179` | success | 182.403 s |
| `chinesepoint_simulator_x4pro_uc8279` | success | 174.519 s |

Each produced its `program` executable. The full UC8179 and UC8279 builds
compiled the learner clock, learner journal, reader activities, input mapping,
and the selected simulator display path before linking.

## What this proves

It proves that the checked-out source builds under all three simulator display
profiles associated with the X4 Pro selection path. It is a compile/link test,
not a screen-orientation, input, wake/sleep, or actual-controller test.

## What it does not prove

The simulator cannot reproduce which controller an individual X4 Pro chooses
at boot, nor validate panel orientation, touch, partial refresh, standby,
recovery, SD flashing, or a device that has been locked by its original
firmware. The release remains blocked by the separate X4 Pro physical test
matrix and the zero-byte IRAM margin already recorded in the learner build
evidence.