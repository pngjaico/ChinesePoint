# ChinesePoint v0.6 Foundation Design

## Goal

Create the X4 Pro-only safety and delivery foundation needed to port CJK Learner onto current CrossPoint without weakening recovery, panel detection, update validation, or future simulator evidence.

## Included

- ChinesePoint identity in README, firmware metadata, release material, and site copy.
- One dedicated ChinesePoint X4 Pro build environment.
- A small NVS CJK crash guard which runs only after recovery routing is decided.
- Screenshot storage under /.chinesepoint/screenshots.
- Official CrossPoint desktop simulator configuration for X4 Pro CI.
- A browser simulator release path and a generated, manifest-gated site.
- Tests for scope, boot order, safety transitions, screenshot isolation, artifact identity, and site manifests.

## Excluded

- Raw automatic flashing, partitions, bootloader modifications, or display-driver ports.
- CJK UI, stats, Anki networking, Bluetooth, BookFusion, games, and multi-device support.
- A claim that simulator output proves physical waveform or panel behavior.

## Safety guard interface

The new module has one responsibility: isolate CJK session crashes.

namespace chinesepoint::cjk_safety

Access is Enabled or SafeMode.

State has consecutiveCrashes, sessionActive, and safeMode.

reconcileBoot(rebootedFromPanic) runs only after main.cpp has calculated recoveryFirmwareMode. It increments only if the previous CJK session remained active and CrossPoint reports a panic. The second consecutive panic sets SafeMode. Non-panic reboot clears a stale active marker without counting it. NVS errors log and return Enabled; storage trouble cannot block the reader.

beginLearnerSession persists the marker before CJK allocation. finishLearnerSession clears it after normal exit. clearSafeMode requires explicit user confirmation in a later CJK diagnostic activity.

## Persistence

Checkpoint-compatible learner state remains under /.cjk/learner/v1. ChinesePoint diagnostics live under /.chinesepoint. Future migrations copy, checksum, verify, then activate; they never overwrite a journal in place.

## Simulator

The simulator target is simulator_x4pro. It uses SIMULATOR_DEVICE_X4_PRO and excludes update code so a scripted test cannot alter a device. Linux CI builds it and stores deterministic Home and Settings screenshots. Windows-native support is unavailable upstream, so the later browser simulator is the runnable Windows delivery.

## Acceptance

1. chinesepoint_x4pro builds an ESP32-S3 application with an X4 Pro board tag.
2. Existing DOWN plus POWER recovery remains first-class.
3. CJK safety code appears after recovery decision and cannot redirect normal boot.
4. Host tests cover first panic, second panic, normal exit, NVS failure, and clear.
5. Screenshot paths never use the SD root.
6. Site downloads require verified manifest fields and remain diagnostic before physical validation.
7. Public docs identify Xteink X4 Pro as the sole supported hardware.
