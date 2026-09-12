# ChinesePoint

ChinesePoint is an open-source, learning-first e-reader firmware for the **Xteink X4 Pro only**. It is a focused fork of CrossPoint Reader, built around high-quality CJK reading, local vocabulary learning, trustworthy progress data, and later opt-in Anki Desktop sync.

> **Current status: development foundation. Do not flash an experimental ChinesePoint build until it is explicitly marked hardware-validated for your X4 Pro.** A simulator result or a successful compilation does not prove compatibility with every physical display controller.

## Supported hardware

ChinesePoint v1 supports **Xteink X4 Pro only**: ESP32-S3, 16 MB flash, 8 MB PSRAM, touch, dual frontlight, and current FreeInk panel detection.

ChinesePoint does not publish binaries, installation instructions, or support claims for X3, original X4, X4 Classic, Sticky, PaperMono, or other devices.
Its release workflows build only `chinesepoint_x4pro`; an upstream-shaped multi-board release must not be treated as a ChinesePoint release.

## What ChinesePoint is becoming

- EPUB, TXT, and XTC reading on the current CrossPoint reader base.
- CJK token lookup, local dictionaries, source-aware vocabulary, and local review for cards with a saved answer.
- Inspectable reading and learning statistics with export and import.
- Optional token-authenticated LAN Anki Desktop flashcard import; no cloud relay, background device sync, or schedule-state pull.
- A versioned Linux/WSL simulator evidence bundle with scripted captures for every release.
- A release dashboard with hashes, rollback instructions, and clear status labels.

The architecture and safety constraints are in docs/chinesepoint/v1-architecture.md.

## Safety before features

The X4 Pro has display-controller variants. ChinesePoint stays on current CrossPoint and FreeInk X4 Pro support instead of importing old CrossPlay display code. Recovery remains **DOWN plus POWER**; UP is GPIO0 and is not a recovery key.

On hardware builds, after FreeInk has selected a panel driver, ChinesePoint writes
`/x4pro-panel-probe.txt` to the SD card. It records the selected SSD1677,
UC8179, or UC8279 controller and the raw probe fields, so a bad-screen attempt
can be diagnosed from USB mass-storage or a card reader. The report never
changes the driver decision and a failed write never delays boot or recovery.
It also captures the boot-time byte-addressable internal heap (`free`,
`largest`, and `minimum`) required for the physical memory acceptance record.

Every release must include:

- an X4 Pro application image for SD or OTA update;
- a separately labelled X4 Pro full USB image, never offered to the on-device updater;
- hashes and an auditable manifest;
- a simulator artifact; and
- a confirmed rollback route.

## Community hardware reports

The public repository can collect controlled hardware evidence without
pretending that the current build is installable. There is **no open flashing
call** yet. Before a maintainer assigns an explicit hardware-test candidate,
the candidate must have a fixed commit and verified application hash, a proven
known-good recovery image, and a documented simulator result. The reporting
rules and stop conditions are in
[community-hardware-testing.md](docs/chinesepoint/community-hardware-testing.md).

## Browser USB installer

The release dashboard contains an ESP Web Tools installer for an approved X4
Pro release. It is intentionally absent for the current blocked artifact. A
tagged release must pass every simulator and physical panel row and the
DOWN+POWER recovery drill before the release workflow may create its merged
ESP32-S3 web-install image and manifest. The tool accepts no local `.bin`,
cannot distinguish an X4 Pro from another ESP32-S3 by itself, and must never be
used on a locked or irreplaceable reader.
## Verified baseline

The initial source base is CrossPoint develop commit e7a3bb48817f1cb951b521ca958562723159c2f6. ChinesePoint currently pins FreeInk `7f6bd0f47a766eea18206dd19f723f3707b6c9d3`, which contains the upstream X4 Pro display-driver correction.

The unmodified X4 Pro baseline compiled on 2026-08-30 and produced a valid ESP32-S3 application image. It is not yet a ChinesePoint CJK release.

The current local X4 Pro diagnostic build has an ESP32-S3 image header, the
sole `CROSSPOINT-BOARD-V1:x4pro;` tag, a valid Espressif checksum and validation
hash, and SHA-256 `60457af20c3d9e16f96c4853af08f8e11866dfd6d7bed7c1446401aef2a1062b`
(5,390,320 bytes). `firmware.bin` and its generated `update.bin` have that
same hash. The build regenerates the application from a stripped temporary ELF
so debug paths from different worktrees cannot change the app-descriptor hash.
It passed artifact validation and script checks; native host and simulator
evidence is recorded separately for the exact sources it covers.

It includes word selection, sentence-context saving even for a local dictionary
miss, optional local StarDict lookup, and Matcha-inspired EPUB language
routing: a valid `dc:language` may choose a dictionary under
`/dictionaries/<language>/<dictionary>/`, otherwise the existing global
selection remains in force. It also has learner statistics, a read-only
vocabulary/context browser, a local review queue with Again/Hard/Good/Easy for
due cards that have a saved local dictionary answer, an opt-in verified
CC-CEDICT installer, and a manually triggered token-authenticated Anki Desktop
bridge. The local review queue uses a durable logical study clock and refuses
cards without an answer or cards under Anki authority. Its bounded Anki
export, upload, and response waits service a subscribed watchdog; target socket
operations are capped at three seconds while the complete HTTP response has a
60-second deadline. It is explicitly **not installable yet**: no physical
panel, Anki transfer, or recovery drill has been performed.

## Current diagnostic build

Artifact validation reports `installable: false`; it is not a GitHub release
asset or a recovery image. A deterministic local artifact does not establish
that it is safe to boot, refresh a panel, or recover a locked device.

This candidate adds bounded, allocation-free CJK phrase lookup after an exact
StarDict miss. It tries no more than 12 phrases of up to 8 Han code points and
64 UTF-8 bytes, then preserves dictionary SD, decompression, and low-memory
errors instead of presenting them as a miss.

It also bounds TXT page-index layout for an 8 KiB unspaced CJK chunk to
logarithmic-width probes, protects UTF-8 boundaries, and services the
subscribed watchdog around expensive text measurement. This reduces a known
indexing-risk path; it is not proof that an X4 Pro cannot freeze.

The build used 27.5% RAM and 82.0% flash. The size report's 16 KiB
dedicated-IRAM region is fully assigned, but the ESP32-S3 linker map also has
341,760 bytes of shared D/IRAM and 184,682 bytes remain after static code and
data. New ISR or flash-cache-sensitive code still requires an IRAM/DIRAM
budget review; the actual runtime heap is lower after SDK startup and must be
measured on hardware. Host-native tests, formatting, static analysis,
simulator coverage, every physical panel path, reader acceptance, and the
DOWN+POWER recovery drill remain release gates.

On 2026-09-12, the FreeInk pin was advanced to
`7f6bd0f47a766eea18206dd19f723f3707b6c9d3`, including the upstream X4 Pro
driver update implicated by the CrossPlay mirrored-display report. The X4 Pro
application rebuilt from an isolated cache and verified as target
`xteink-x4-pro`. DOWN+POWER first reaches the SD firmware picker before normal
reader state, settings, optional services, or frontlight initialization; a
continuous 2.5-second hold attempts only the separately hashed backup contract
documented below. This is still not physical display or recovery evidence. The
16 KiB dedicated-IRAM figure is already allocated, while the relevant shared
D/IRAM static remainder is 184,682 bytes; both values need runtime validation
rather than a blind source-level "optimization". On real X4 Pro hardware, a
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

The canonical build deliberately does not reuse PlatformIO's global object
cache. It previously combined incompatible cached objects after source changes
and produced missing symbols only at link time. The normal `.pio` directory
still makes unchanged local rebuilds incremental. After changing a dependency,
run `pio run -e chinesepoint_x4pro -t clean` once before building; never treat a
cache-reused binary as a release candidate without the artifact validator.

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
# Compile only, without opening the simulator window:
.\tools\chinesepoint\run_simulator_x4pro.ps1 -Panel uc8179 -BuildOnly
~~~

The launcher updates an isolated WSL working clone and stores its toolchain,
cache, and build tree in the `ChinesePoint-Emulator` WSL VHDX on D:. On its
first run it creates an isolated Python environment and installs PlatformIO
there; it never changes the Windows PlatformIO installation. It never calls
the USB flasher, the SD updater, or an X4 Pro.

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
manual picker. Before copying, the preparation tool also rejects a file that
is too small or too large for the OTA slot, lacks ESP application magic, is not
ESP32-S3, or has no X4 Pro board tag. This is a desktop preflight; it does not
replace the firmware's streamed integrity validation. Prepare the SD card from
Windows without flashing a device:

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
4. v0.9: opt-in Anki card import — implemented in source and collection-API smoke tested; Anki Desktop UI and X4 Pro Wi-Fi transfer pending.
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
