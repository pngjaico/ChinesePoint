# Path to ChinesePoint 1.0

Status on 2026-09-12: the current candidate is diagnostic only. Its simulator
matrix passes, but physical panel, recovery, Anki Desktop, and long-session
evidence are absent. It must not be flashed as a 1.0 release.

The current source retains the bounded Anki watchdog behavior and bounded
local-dictionary routing from EPUB `dc:language` metadata. It limits the route
to one language folder and falls back to the existing global dictionary when
the tag is absent or invalid. The X4 Pro post-build step now removes
worktree-dependent debug metadata before regenerating the application image:
the observed normalized `firmware.bin` and `update.bin` share SHA-256
`60457af20c3d9e16f96c4853af08f8e11866dfd6d7bed7c1446401aef2a1062b`.
Two clean source links had identical loaded segments and yielded the same
normalized image. This establishes local artifact determinism for those
inputs, not hardware compatibility. The SSD1677/UC8179/UC8279 simulator matrix
passes, but it proves neither a real dictionary lookup, Anki transfer, nor a
physical test record. The artifact remains diagnostic and non-installable.

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
- Papyrix currently documents UC8179/UC8279 detection, so the earlier
  SSD1677-only recovery conclusion is withdrawn. Its `/force_update.bin` route
  still auto-flashes before the UI and removes the file after an attempt; it
  cannot replace ChinesePoint's deliberate, separately hashed backup restore.

## Mini plan from here

1. **Keep the reproducible-build gate closed.** The post-build normalizer now
   produces a canonical app image before `update.bin` is copied. Add a clean
   CI rebuild from a second checkout and require the canonical SHA-256 for both
   files before any candidate is staged to a test SD root. Never place it on
   the device during this diagnostic phase.
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
   The 16 KiB dedicated-IRAM allocation and the shared D/IRAM runtime margin
   must be measured during this matrix.

The current technical risk that cannot be solved by documentation is the X4
Pro image's unmeasured runtime internal-RAM margin. Its 16 KiB dedicated IRAM
is allocated, while the linker reports 184,682 bytes of shared D/IRAM after
static code and data. SDK startup, Wi-Fi, USB-MSC, display, and reader use
reduce the usable heap; only a measured device session can establish whether
watchdog, sleep/wake, Wi-Fi, and panel behavior are acceptable.

## Critical correction: local review exists; Anki schedule integration does not

The learner now has a bounded in-device review activity. It selects only due,
locally authoritative entries that have a durable answer captured from a
successful local dictionary lookup, and records `Again`, `Hard`, `Good`, or
`Easy` through one checksummed review mutation with its study-clock state.
Entries without an answer stay available in the vocabulary browser but cannot
be rated as flashcards. The journal keeps answers in their own record so old
entry snapshots remain readable and later snapshots do not erase an answer.

This is a local scheduler, not an Anki scheduler. The LAN bridge imports the
saved answer into a project-owned Anki note type and creates a real Anki card
only when that answer is non-empty; Anki Desktop then owns scheduling. The
bridge does not import card state, due dates, or reviews back to the device.
Calling it two-way Anki schedule synchronization would be false.

Before Anki becomes a schedule authority, implement and test this sequence:

1. Define a versioned ownership and conflict contract for future schedule
   state pull. It must state which scheduler owns a card and reject every
   unsupported transition.
2. Build a bounded import/export protocol with idempotency, cancellation,
   offline retry, malformed-response, duplicate-card, and clock-skew tests.
3. Run it first against a disposable Anki Desktop collection, then on a real
   deck only after a user-visible backup/export is available.
4. Add simulator and physical X4 Pro review sessions for all controller
   profiles, including four ratings, no-due state, sleep/wake, CJK answer
   rendering, and recovery after an interrupted SD write.
