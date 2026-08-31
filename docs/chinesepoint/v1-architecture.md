# ChinesePoint v1 Architecture

## Product boundary

ChinesePoint is a learning-first fork of CrossPoint Reader for the Xteink X4 Pro only. The firmware is ChinesePoint; CJK Learner is its integrated Chinese, Japanese, and Korean learning workspace.

ChinesePoint retains the current X4 Pro CrossPoint reader and platform as-is: library, EPUB/TXT/XTC reader, touch, Wi-Fi, frontlight, KOReader Sync, settings, updates, panel detection, and recovery remain CrossPoint responsibilities. ChinesePoint work is additive and limited to the planned CJK learner, learning data, statistics/export, and optional Anki Desktop bridge; it does not replace the reader, boot, display, update, or flasher implementations.

ChinesePoint v1 does not claim support for X3, original X4, X4 Classic, Sticky, PaperMono, or any other board. Their upstream source may remain for rebaseability, but ChinesePoint CI, releases, site, installer, documentation, and support build and publish only the X4 Pro.

## Verified base

| Item | Value |
| --- | --- |
| CrossPoint base | e7a3bb48817f1cb951b521ca958562723159c2f6 |
| FreeInk pin | f831c1e447a21cfc7def620bb7c9c783e416a0a4 |
| Baseline environment | x4pro |
| Baseline app image | 5,324,464 bytes |
| Baseline app SHA-256 | 0ae10b6ed58940d64d9c0faf4871f1b60fb2b6de2425537e30a7b18a660c96da |
| ChinesePoint foundation environment | chinesepoint_x4pro |
| Foundation source commit | cbaa480985a77050920de96687b43da975b029fb |
| Foundation app image | 5,326,320 bytes |
| Foundation app SHA-256 | a9fcde0f15acffe8c43d80f4192c4742f1f7f1f01f59142a971c4dc8026ab48f |
| Foundation build checks | ESP32-S3 header, valid checksum/hash, one X4 Pro tag, 191 host tests, 4 artifact tests, and [successful SSD1677/UC8179/UC8279 simulator CI](https://github.com/pngjaico/ChinesePoint/actions/runs/33411936353) |

The baseline and foundation build are valid ESP32-S3 application images. They prove the starting platform and early safety integration compile, not that CJK changes are hardware-safe. The foundation artifact is not published as installable because it lacks physical display/recovery evidence and its required safe site bundle. The source-side verifier record is `artifacts/chinesepoint-x4pro-foundation.json`; it deliberately carries `installable: false`.

## Safety invariants

1. Preserve X4 Pro recovery: hold DOWN plus POWER. UP is GPIO0 and must not become a recovery requirement.
2. CJK code must not run before CrossPoint has resolved recovery mode and routed to the SD firmware updater.
3. Dictionary, data, font, migration, or network failures must fail open to the ordinary reader and recovery screen.
4. Keep CrossPoint image validation and X4 Pro board-tag validation intact.
5. Publish application images for SD or OTA separately from full USB images. Never pass a full image to the on-device updater.
6. Keep learner persistence append-safe, checksummed, bounded, versioned, and recoverable after a torn final record.
7. Two consecutive crashes during an active CJK session disable CJK until the user explicitly re-enables it. They never disable the reader.
8. A simulator or a compilation is not physical display proof.

## Panel policy

ChinesePoint uses the current FreeInk X4 Pro detection path for SSD1677, UC8179, and UC8279 panel controllers. It does not import CrossPlay platform or display code.

CrossPlay v1.8.0 used FreeInk 2b717e95, a divergent line missing the UC8279 orientation correction 3d90c8a. That is credible evidence for the earlier mirrored, inverted, or white-screen incident, but only physical X4 Pro testing can confirm the current result on this unit.

## Delivery model

| Artifact | Use |
| --- | --- |
| X4 Pro application image | SD Firmware Update or compatible OTA path |
| X4 Pro full USB image | USB rescue install only |
| ELF, map, manifest, SHA-256 | audit and debugging |
| Desktop simulator screenshots | deterministic CI evidence |
| Browser simulator bundle | inspect release UI without flashing |

Every release manifest records ChinesePoint commit, upstream commit, FreeInk pin, environment, artifact hashes, panel-test status, and rollback steps.

## Roadmap

| Milestone | Outcome |
| --- | --- |
| v0.6 Foundation | X4 Pro boundary, safety guard, simulator, site manifest |
| v0.7 Learner | current-base CJK lookup, journal, review, and provisioning |
| v0.8 Insight | trustworthy stats, export, import, and offline editor |
| v0.9 Anki | token-authenticated LAN pull and idempotent push |
| v1.0 | recovery drill, physical matrix, installers, site, hashes, simulators |

## Donor policy

CrossPoint and FreeInk remain the platform base. The CJK checkpoint contributes pure learning code and tests, not its old SDK pin or builder. CrossPlay contributes app isolation and browser-simulator ideas, not display or boot code. CrossInk and vCodex contribute analytics design, not C3 platform code. xteink-anki contributes its token, NDJSON, and idempotent-batch protocol while Anki Desktop remains scheduler authority. BLE, games, raw auto-flashing, and unconfirmed device ports are excluded from v1.

## Site policy

The site is a release dashboard. It shows X4 Pro-only scope, simulator availability, hashes, app versus full image, panel-test status, recovery instructions, and a visible diagnostic label. The supplied site ZIP is a reference; cache files, stale release names, and unverified downloads cannot be published.
