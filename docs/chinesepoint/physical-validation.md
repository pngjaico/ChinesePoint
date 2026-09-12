# ChinesePoint X4 Pro physical-validation protocol

Status: **not performed**. This is a controlled test protocol, not an
installation guide for the current blocked artifact.

The current binary must remain blocked until the simulator matrix and this
protocol have records for its exact source commit and SHA-256. A successful
compile, a prior CrossPoint boot, or a result from one different panel does
not satisfy this protocol.

## Preconditions

1. Confirm the device is an Xteink **X4 Pro**, not original X4, X4 Classic,
   Sticky, PaperMono, X3, or another device.
2. Confirm battery charge, a reliable USB cable, and a separately labelled SD
   card containing a known-good, X4 Pro-only recovery application image whose
   origin and SHA-256 have been recorded. Do not use a full USB image in the
   SD updater.
3. Before any ChinesePoint candidate, prove that holding **DOWN + POWER** from
   power-off opens the SD Firmware Update recovery screen on the current
   known-good firmware. Photograph that screen and record its firmware hash.
   UP is GPIO0 and is not a substitute for DOWN.
4. Verify the candidate with `verify_artifact.py` on the build machine and
   compare the displayed SHA-256 with the evidence manifest. Stop on any
   mismatch.
5. Capture the early serial line `X4 Pro PSRAM: total=... free=...`. A total
   below 6 MiB must show the PSRAM fault screen and normal reader testing must
   stop; prove the DOWN+POWER recovery route from that state before retrying a
   normal candidate.
6. After a candidate reaches its first display initialization, retrieve
   `/x4pro-panel-probe.txt` from the SD card or USB mass-storage. Preserve its
   `selected_controller`, raw probe fields, and `internal_8bit_free`,
   `internal_8bit_largest`, and `internal_8bit_min` values in the evidence
   record. These are observations, not evidence that later reader, Wi-Fi, or
   USB-MSC activity has enough heap.

## Stop conditions

Stop the test immediately if boot hangs, the screen is blank/mirrored/inverted,
touch is uncontrolled, the device reboots repeatedly, PSRAM is reported below
6 MiB, or the recovery screen does not appear. Do not retry a blind flash.
Record the condition, remove the candidate SD card, and use the previously
proven recovery route.

## Per-panel matrix

ChinesePoint cannot call a general X4 Pro release installable until all three
known controller paths have independent records. One physical unit normally
covers only one row; the other rows must remain `pending`, not guessed.

| Panel path | Required observations | Evidence record |
| --- | --- | --- |
| SSD1677 | cold boot; orientation; full and partial refresh; touch; both frontlights; reader open/close | `docs/chinesepoint/evidence/ssd1677.md` |
| UC8179 | cold boot; orientation; full and partial refresh; touch; both frontlights; reader open/close | `docs/chinesepoint/evidence/uc8179.md` |
| UC8279 | cold boot; orientation; full and partial refresh; touch; both frontlights; reader open/close | `docs/chinesepoint/evidence/uc8279.md` |

For each tested row, run cold-only and warm-only frontlight for five minutes,
then mixed light through three brightness levels, an off/on cycle, and a
sleep/wake cycle while Wi-Fi is connected. Record any disappearing channel,
flicker, reset, or change in touch responsiveness. These checks specifically
address recent upstream X4 Pro reports; a successful boot frame is insufficient.

After recovery and normal reader checks pass, record battery percentage and
serial memory values before an overnight sleep period, then after wake. A
material unexplained drain, a reset, or a lower memory floor stops release
work for that candidate. Font download is deferred until this baseline is
stable; when tested, capture free heap and maximum allocation before and after
the operation. Also preserve the three `internal_8bit_*` values from the SD
probe report at first boot; later tests must capture equivalent values after
Wi-Fi/Anki, USB transfer, reading, refresh, and sleep/wake before a release
can claim an acceptable runtime internal-memory floor.

For the detected physical panel, continue with the learner checks below. Do
not force a simulated controller identifier onto the device.

## Learner and Anki checks

1. Open EPUB and TXT files; verify ordinary reading, sleep/wake, Wi-Fi,
   frontlight, touch, settings, and file handling before opening learner UI.
2. Select a CJK word and sentence; save it with and without a local dictionary
   hit; reboot; confirm its context and stats remain intact.
3. Download the pinned CC-CEDICT release over trusted Wi-Fi; verify lookup;
   reboot and verify that a failed or cancelled download did not displace the
   prior dictionary.
4. Export saved vocabulary to SD and validate that the expected NDJSON file is
   bounded and readable on a desktop.
5. Install the ChinesePoint Anki bridge in a disposable Anki Desktop profile.
   Configure a private-LAN URL and token manually, tap **Sync now**, then
   confirm exactly one `ChinesePoint` deck/note type and idempotent re-send.
   No automatic/background sync is expected or allowed.
6. Record the result, exact source commit, firmware SHA-256, panel identifier,
   photos/logs, and anomalies in the appropriate evidence file.

## Recovery drill after candidate testing

Power off, use **DOWN + POWER**, and enter the SD updater. Verify that it
still rejects a non-X4-Pro image and lists only the known-good X4 Pro recovery
application. Complete a restore only after the candidate's behavior and hash
are documented. Record the whole drill in
`docs/chinesepoint/evidence/recovery.md`.

To exercise the automatic backup route, first record the SHA-256 of the
operator-selected CrossPoint application. Place it only at
`/backup/crosspoint-x4pro.bin`, with its lowercase digest at
`/backup/crosspoint-x4pro.bin.sha256`. From power-off, hold DOWN + POWER for
2.5 seconds without releasing either key. The firmware must reject a missing,
wrong-digest, corrupt, oversized, wrong-chip, or wrong-board backup without
altering the OTA boot target. Only then test a verified backup restore.

Only evidence files with observed results may be referenced by a `passed`
state in `site/release-manifest.json`. The release verifier rejects an enabled
download without those records.
