# Path to ChinesePoint 1.0

Status on 2026-09-12: the current candidate is diagnostic only. Its simulator
matrix passes, but physical panel, recovery, Anki Desktop, and long-session
evidence are absent. It must not be flashed as a 1.0 release.

Source commit `da0511e` retains the bounded Anki watchdog behavior and adds
bounded local-dictionary routing from EPUB `dc:language` metadata. It limits
the route to one language folder and falls back to the existing global
dictionary when the tag is absent or invalid. Its X4 Pro artifact is
reproducible across two consecutive builds; `firmware.bin` and `update.bin`
share SHA-256 `761324065c89cc08d40577af300fc0694e712a8d80e97df15a3dc36cb473e9fe`.
The exact-source SSD1677/UC8179/UC8279 simulator matrix passes, but it proves
neither a real dictionary lookup, Anki transfer, nor a physical test record.
The artifact remains diagnostic and non-installable.

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
   live session. Repeat this protocol for the current Anki-transfer watchdog
   change, including a deliberately slow LAN response and a cancellation
   attempt.
5. **Perform recovery only after normal testing.** Trigger the automatic
   2.5-second DOWN+POWER backup path and the manual picker, verify the exact
   backup is accepted and a wrong image is rejected, then record the complete
   return to the known-good firmware.
6. **Publish 1.0 only when evidence is complete.** Update the manifest with
   physical records for all three controllers and recovery, attach the exact
   binary and hashes, run the release checks again, and change `installable`
   only after those records are independently reviewable.

## Decisions from external projects

- Matcha Reader contributed one bounded reader improvement: an EPUB may select
  a local StarDict beneath `/dictionaries/<language>/<dictionary>/` from its
  `dc:language` primary tag. ChinesePoint accepts only one nested level and
  falls back to the user's current global dictionary on absent or malformed
  metadata. The existing flat CC-CEDICT folder remains valid. This is compiled
  source evidence; it still needs an SD-card EPUB acceptance case.
- CrossPlay must not donate X4 Pro display initialization: its earlier FreeInk
  pin was missing the upstream X4 Pro display-driver update implicated in
  mirrored output on some controllers.
- Papyrix hard-codes an SSD1677-oriented recovery path, so importing it would
  weaken the required three-controller X4 Pro gate.

## Mini plan from here

1. **Close the reproducible-build gate.** Rebuild the committed X4 Pro source
   twice with UTF-8 PlatformIO output, verify identical SHA-256 values for
   `firmware.bin` and generated `update.bin`, then stage only the app image to
   a test SD root. Never place it on the device during this diagnostic phase.
2. **Capture emulator evidence again.** Rebuild and photograph the SSD1677,
   UC8179, and UC8279 simulator profiles from that exact commit. This is a
   code-path check, not a panel result.
3. **Exercise the reader/learning path off-device.** Add an EPUB whose
   `dc:language` selects a nested dictionary, then verify fallback, malformed
   metadata, CJK lookup candidates, learner journal export, and the Anki bridge
   protocol. The first real Anki import must use a disposable collection.
4. **Establish physical recovery before feature work.** On the locked X4 Pro,
   demonstrate the existing CrossPoint state, the DOWN+POWER picker, and a
   rejected bad backup before flashing any ChinesePoint candidate.
5. **Run the real three-controller matrix.** Each controller variant needs
   cold boot, orientation, refresh, touch, frontlight, sleep/wake, reading,
   slow Wi-Fi, Anki cancellation, and a successful backup restore record.
   The current zero-byte IRAM margin is a release blocker until this evidence
   exists.

The current technical risk that cannot be solved by documentation is the X4
Pro image's 100% IRAM allocation. The firmware compiles, but the remaining
margin is zero; only a measured device session can establish whether its
watchdog, sleep/wake, Wi-Fi, and panel behavior are acceptable.
