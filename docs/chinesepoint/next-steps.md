# Path to ChinesePoint 1.0

Status on 2026-09-12: the current candidate is diagnostic only. Its simulator
matrix passes, but physical panel, recovery, Anki Desktop, and long-session
evidence are absent. It must not be flashed as a 1.0 release.

Source commit `b94daef` additionally services a subscribed task watchdog
during bounded Anki export, upload, and response waits. The X4 Pro target
build and focused static analysis pass, but this source revision has not yet
received its own retained artifact, simulator matrix, or physical test record.
It therefore does not replace the separately validated diagnostic artifact.

1. **Prepare a reversible hardware session.** Keep the known-good X4 Pro
   CrossPoint application plus its strict backup checksum on the SD card. From
   the known-good firmware, prove the DOWN+POWER recovery picker before testing
   ChinesePoint. Record the backup hash and a photo of that picker.
2. **Validate each actual X4 Pro controller.** SSD1677, UC8179, and UC8279
   each need their own boot, orientation, touch, frontlight, sleep/wake, and
   100-page reading record. A passing simulator profile does not fill any row.
3. **Run the reader stability corpus.** Use the CJK EPUB and long unspaced TXT
   cases in `reader-acceptance.md`, from a clean cache, with serial `MEM`
   samples. Test Wi-Fi scans during reading and sleep/wake. Any reset, image
   inversion, ghosting that obscures text, low-PSRAM fault, or declining heap
   stops release work on that candidate.
4. **Prove learning and Anki on real endpoints.** Install the pinned
   dictionary, save direct hits and misses with context, export/reimport the
   learner journal, then perform the Anki Desktop first-sync, idempotent-sync,
   conflict, and offline-retry cases on the user's real deck.
   The workstation listener smoke is recorded in
   `evidence/anki-desktop-smoke-2026-09-12.md`; it did not import cards. Its
   pre-existing AnkiMCP startup failure must be handled separately before the
   live session. Repeat this protocol after the Anki-transfer watchdog change,
   including a deliberately slow LAN response and a cancellation attempt.
5. **Perform recovery only after normal testing.** Trigger the automatic
   2.5-second DOWN+POWER backup path and the manual picker, verify the exact
   backup is accepted and a wrong image is rejected, then record the complete
   return to the known-good firmware.
6. **Publish 1.0 only when evidence is complete.** Update the manifest with
   physical records for all three controllers and recovery, attach the exact
   binary and hashes, run the release checks again, and change `installable`
   only after those records are independently reviewable.

## Decisions from external projects

- Matcha Reader is a useful product reference for explicit dictionary language
  selection and split lookup presentation. Its code is not a drop-in CJK
  implementation here.
- CrossPlay must not donate X4 Pro display initialization: its earlier FreeInk
  pin was missing the upstream X4 Pro display-driver update implicated in
  mirrored output on some controllers.
- Papyrix hard-codes an SSD1677-oriented recovery path, so importing it would
  weaken the required three-controller X4 Pro gate.

The current technical risk that cannot be solved by documentation is the X4
Pro image's 100% IRAM allocation. The firmware compiles, but the remaining
margin is zero; only a measured device session can establish whether its
watchdog, sleep/wake, Wi-Fi, and panel behavior are acceptable.
