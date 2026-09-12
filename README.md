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

The latest ChinesePoint pre-physical build passed locally on 2026-09-01 at source commit `04338ee`. Its X4 Pro artifact has an ESP32-S3 image header, the current `CROSSPOINT-BOARD-V1:x4pro;` tag, a valid Espressif checksum and validation hash, and SHA-256 `5febfdceeebba5c7a575601f843773fac81bc4647a651580305191f8aa3fb26b`. It passed 198 native tests and artifact validation; the three-panel simulator CI for this exact commit is pending. It includes word selection, sentence-context saving even for a local dictionary miss, optional local StarDict lookup, learner statistics, a read-only vocabulary/context browser, an opt-in verified CC-CEDICT installer, and a manually triggered token-authenticated Anki Desktop bridge. It is explicitly **not installable yet**: no physical panel or recovery drill has been performed.

## Current diagnostic build

On 2026-09-11, the current `chinesepoint_x4pro` working tree compiled as an
ESP32-S3 X4 Pro application image with `CROSSPOINT-BOARD-V1:x4pro;` and
SHA-256 `5b1f6bcfee933033903ee4762bd4f6743da38ff452d273d373d297a85a40ab4b`
(5,370,144 bytes). Artifact validation reports `installable: false`; it is a
diagnostic build only and is not a GitHub release asset or recovery image.

This candidate adds bounded, allocation-free CJK phrase lookup after an exact
StarDict miss. It tries no more than 12 phrases of up to 8 Han code points and
64 UTF-8 bytes, then preserves dictionary SD, decompression, and low-memory
errors instead of presenting them as a miss.

The build used 27.5% RAM and 81.9% flash. Its IRAM total is fully allocated,
so adding ISR or flash-cache-sensitive code requires an IRAM budget review
before it can be considered safe. Host-native tests, formatting, static
analysis, simulator coverage, every physical panel path, reader acceptance,
and the DOWN+POWER recovery drill remain release gates.

~~~powershell
$env:PYTHONUTF8 = '1'
$env:PLATFORMIO_CORE_DIR = 'D:\Usuario-pc\Ferramentas\PlatformIO' # keep toolchains on D:
$env:PLATFORMIO_HOME_DIR = $env:PLATFORMIO_CORE_DIR
pio run -e chinesepoint_x4pro
~~~

## Simulator

ChinesePoint uses the official CrossPoint X4 Pro simulator for deterministic CI screenshots. It runs on Linux or WSL in CI; upstream has no Windows-native simulator path. Each milestone delivers the three verified captures, checksums and a pinned Linux/WSL re-execution bundle. This is evidence only and never flashes hardware.

## Development map

1. v0.6: X4 Pro-only build identity, safe CJK isolation, simulator/release gate, site manifest — implemented; current simulator evidence remains pending.
2. v0.7: current-base CJK Learner port — implemented in source; physical validation pending.
3. v0.8: statistics and deterministic export — implemented in source; physical validation pending.
4. v0.9: opt-in Anki synchronization — implemented in source; Anki Desktop and X4 Pro integration pending.
5. v1.0: physical X4 Pro matrix, recovery drill, installers, hashes, site, and simulators.

No milestone is called installable until build, artifact validation, simulator, recovery path, and physical X4 Pro tests have passed.

The exact preflight, panel matrix, learner checks, and evidence format are in
[`docs/chinesepoint/physical-validation.md`](docs/chinesepoint/physical-validation.md).

## Upstream and license

ChinesePoint inherits from CrossPoint Reader and FreeInk, both MIT-licensed. Upstream stays configured as crosspoint-reader/crosspoint-reader so ChinesePoint changes remain auditable and selectively rebaseable.
