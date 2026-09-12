# ChinesePoint

ChinesePoint is an open-source, learning-first e-reader firmware for the **Xteink X4 Pro only**. It is a focused fork of CrossPoint Reader, built around high-quality CJK reading, local vocabulary learning, trustworthy progress data, and later opt-in Anki Desktop sync.

> **Current status: development foundation. Do not flash an experimental ChinesePoint build until it is explicitly marked hardware-validated for your X4 Pro.** A simulator result or a successful compilation does not prove compatibility with every physical display controller.

## Supported hardware

ChinesePoint v1 supports **Xteink X4 Pro only**: ESP32-S3, 16 MB flash, 8 MB PSRAM, touch, dual frontlight, and current FreeInk panel detection.

ChinesePoint does not publish binaries, installation instructions, or support claims for X3, original X4, X4 Classic, Sticky, PaperMono, or other devices.
Its release workflows build only `chinesepoint_x4pro`; an upstream-shaped multi-board release must not be treated as a ChinesePoint release.

## What ChinesePoint is becoming

- EPUB, TXT, and XTC reading on the current CrossPoint reader base.
- CJK token lookup, local dictionaries, source-aware vocabulary, and review.
- Inspectable reading and learning statistics with export and import.
- Optional token-authenticated LAN Anki Desktop sync where Anki remains the scheduler; no cloud relay or background device sync.
- A versioned Linux/WSL simulator evidence bundle with scripted captures for every release.
- A release dashboard with hashes, rollback instructions, and clear status labels.

The architecture and safety constraints are in docs/chinesepoint/v1-architecture.md.

## Safety before features

The X4 Pro has display-controller variants. ChinesePoint stays on current CrossPoint and FreeInk X4 Pro support instead of importing old CrossPlay display code. Recovery remains **DOWN plus POWER**; UP is GPIO0 and is not a recovery key.

Every release must include:

- an X4 Pro application image for SD or OTA update;
- a separately labelled X4 Pro full USB image, never offered to the on-device updater;
- hashes and an auditable manifest;
- a simulator artifact; and
- a confirmed rollback route.

## Verified baseline

The initial source base is CrossPoint develop commit e7a3bb48817f1cb951b521ca958562723159c2f6 with FreeInk f831c1e447a21cfc7def620bb7c9c783e416a0a4.

The unmodified X4 Pro baseline compiled on 2026-08-30 and produced a valid ESP32-S3 application image. It is not yet a ChinesePoint CJK release.

The current ChinesePoint pre-physical build is source commit `da0511e`. Its X4
Pro artifact has an ESP32-S3 image header, the current
`CROSSPOINT-BOARD-V1:x4pro;` tag, a valid Espressif checksum and validation
hash, and SHA-256 `761324065c89cc08d40577af300fc0694e712a8d80e97df15a3dc36cb473e9fe`
(5,379,216 bytes). Two consecutive X4 Pro builds produced that same SHA-256
for both `firmware.bin` and the generated `update.bin`; the latter was also
validated after safe staging into a local test SD root. It passed artifact
validation, 208 native host tests, eight release-structure tests, and
simulator Home captures in the SSD1677, UC8179, and UC8279 profiles.

It includes word selection, sentence-context saving even for a local dictionary
miss, optional local StarDict lookup, and Matcha-inspired EPUB language
routing: a valid `dc:language` may choose a dictionary under
`/dictionaries/<language>/<dictionary>/`, otherwise the existing global
selection remains in force. It also has learner statistics, a read-only
vocabulary/context browser, an opt-in verified CC-CEDICT installer, and a
manually triggered token-authenticated Anki Desktop bridge. Its bounded Anki
export, upload, and response waits service a subscribed watchdog; target socket
operations are capped at three seconds while the complete HTTP response has a
60-second deadline. It is explicitly **not installable yet**: no physical
panel, Anki transfer, or recovery drill has been performed.

## Current diagnostic build

The `da0511e` artifact above is the current diagnostic build. Artifact
validation reports `installable: false`; it is not a GitHub release asset or a
recovery image.

This candidate adds bounded, allocation-free CJK phrase lookup after an exact
StarDict miss. It tries no more than 12 phrases of up to 8 Han code points and
64 UTF-8 bytes, then preserves dictionary SD, decompression, and low-memory
errors instead of presenting them as a miss.

It also bounds TXT page-index layout for an 8 KiB unspaced CJK chunk to
logarithmic-width probes, protects UTF-8 boundaries, and services the
subscribed watchdog around expensive text measurement. This reduces a known
indexing-risk path; it is not proof that an X4 Pro cannot freeze.

The build used 27.5% RAM and 82.0% flash. Its IRAM total is fully allocated,
so adding ISR or flash-cache-sensitive code requires an IRAM budget review
before it can be considered safe. Host-native tests, formatting, static
analysis, simulator coverage, every physical panel path, reader acceptance,
and the DOWN+POWER recovery drill remain release gates.

On 2026-09-12, the FreeInk pin was advanced to
`7f6bd0f47a766eea18206dd19f723f3707b6c9d3`, including the upstream X4 Pro
driver update implicated by the CrossPlay mirrored-display report. The X4 Pro
application rebuilt from an isolated cache and verified as target
`xteink-x4-pro`. DOWN+POWER first reaches the SD firmware picker before normal
reader state, settings, optional services, or frontlight initialization; a
continuous 2.5-second hold attempts only the separately hashed backup contract
documented below. This is still not physical display or recovery evidence, and
its 100% IRAM allocation remains a release risk. Its listed consumers are
ESP-IDF flash/PSRAM, FreeRTOS and interrupt paths rather than a movable
ChinesePoint feature, so it requires physical watchdog and sleep/wake evidence
instead of a blind source-level "optimization". On real X4 Pro hardware, a
reported PSRAM total below 6 MiB now stops normal startup with a visible
recovery instruction; the DOWN+POWER SD route remains available before that
check.

The automatic backup checksum contract is deliberately strict: its `.sha256`
file is exactly 64 lowercase hexadecimal characters and has no trailing
newline, filename, or whitespace. This is generated by
`prepare_recovery_backup.ps1`; a differently formatted checksum falls back to
the manual recovery picker without writing an OTA slot.
The image and backup digest scans service the subscribed task watchdog and
yield every 100 ms, so a slow readable SD card does not starve the main task
before an update decision is made.

~~~powershell
$env:PYTHONUTF8 = '1'
$env:PLATFORMIO_CORE_DIR = 'D:\Usuario-pc\Ferramentas\PlatformIO' # keep toolchains on D:
$env:PLATFORMIO_HOME_DIR = $env:PLATFORMIO_CORE_DIR
pio run -e chinesepoint_x4pro
# Produces .pio\build\chinesepoint_x4pro\firmware.bin and update.bin with the same SHA-256.
# This only stages an already validated application into an existing SD-card root; it does not flash USB.
python tools\chinesepoint\stage_x4pro_update.py `
  --firmware .pio\build\chinesepoint_x4pro\firmware.bin `
  --sd-root E:\
~~~

## Simulator

ChinesePoint uses the official CrossPoint X4 Pro simulator for deterministic CI screenshots. It runs on Linux or WSL in CI; upstream has no Windows-native simulator path. Each milestone delivers the three verified captures, checksums and a pinned Linux/WSL re-execution bundle. This is evidence only and never flashes hardware.

On 2026-09-11, source commit `dc3a7367d16cbc7501f7e709b48a358c746821d3`
compiled and boot-captured locally through all three X4 Pro profiles: SSD1677,
UC8179, and UC8279. Every BMP passed
`tools/chinesepoint/verify_simulator_screenshot.py`; the captures are at
`D:\Usuario-pc\Projetos\ChinesePoint\simulator-evidence\2026-09-11-dc3a736`.
The identical Home screenshots demonstrate only the simulated boot and render
paths. They do not validate physical panel timing, orientation, deep sleep,
power sequencing, touch, or recovery.

For the configured Windows workstation, start an interactive profile with:

~~~powershell
cd D:\Usuario-pc\Projetos\ChinesePoint\firmware
.\tools\chinesepoint\run_simulator_x4pro.ps1 -Panel ssd1677
# Other profiles: -Panel uc8179  or  -Panel uc8279
~~~

The launcher updates an isolated WSL working clone and stores its toolchain,
cache, and build tree in the `ChinesePoint-Emulator` WSL VHDX on D:. It never
calls the USB flasher, the SD updater, or an X4 Pro.

## Emergency CrossPoint restore

ChinesePoint keeps the ordinary **DOWN + POWER** recovery picker. A separate,
deliberate restore gesture holds those two keys continuously for **2.5 seconds**
from power-off. It then restores only this exact SD-card contract:

~~~text
/backup/crosspoint-x4pro.bin
/backup/crosspoint-x4pro.bin.sha256
~~~

The `.sha256` file contains exactly the lowercase SHA-256 of the application
file. The firmware validates that digest twice, plus the ESP image structure,
chip family, OTA partition size, and X4 Pro board tag before writing. Missing,
corrupt, wrong-board, or changed files do not flash; recovery falls back to the
manual picker. Prepare the SD card from Windows without flashing a device:

~~~powershell
cd D:\Usuario-pc\Projetos\ChinesePoint\firmware
.\tools\chinesepoint\prepare_recovery_backup.ps1 `
  -SourceFirmware D:\Downloads\crosspoint-1.6.0-x4pro.bin `
  -SdRoot E:\
~~~

The automatic route is compiled and locally validated, but its first use must
be part of the physical recovery drill. It is not evidence that a particular
CrossPoint binary is safe until its exact hash has been recorded.

## Development map

1. v0.6: X4 Pro-only build identity, safe CJK isolation, simulator/release gate, site manifest — implemented; local three-panel simulator smoke passed, physical evidence pending.
2. v0.7: current-base CJK Learner port — implemented in source; physical validation pending.
3. v0.8: statistics and deterministic export — implemented in source; physical validation pending.
4. v0.9: opt-in Anki synchronization — implemented in source; Anki Desktop and X4 Pro integration pending.
5. v1.0: physical X4 Pro matrix, recovery drill, installers, hashes, site, and simulators.

No milestone is called installable until build, artifact validation, simulator, recovery path, and physical X4 Pro tests have passed.

The concrete sequence, release blockers, and external-project decisions are in
[`docs/chinesepoint/next-steps.md`](docs/chinesepoint/next-steps.md).

The exact preflight, panel matrix, learner checks, and evidence format are in
[`docs/chinesepoint/physical-validation.md`](docs/chinesepoint/physical-validation.md).

## Upstream and license

ChinesePoint inherits from CrossPoint Reader and FreeInk, both MIT-licensed. Upstream stays configured as crosspoint-reader/crosspoint-reader so ChinesePoint changes remain auditable and selectively rebaseable.

The current dependency decision log is in
[`docs/chinesepoint/upstream-watch-2026-09-12.md`](docs/chinesepoint/upstream-watch-2026-09-12.md).
