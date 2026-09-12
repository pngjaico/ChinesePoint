# CrossPlay Study adoption record

Reference audited: `ma-r-s/crossplay`, branch `xteink`, commit
`858a0bf1345b4bb65db0f8499f78ec7c61d36b3b`, retrieved 2026-09-12.

## Reuse boundary

| CrossPlay area | ChinesePoint decision |
| --- | --- |
| `src/apps_local/study/StudyDeck`, `StudyFsrs`, `StudyScheduler` | Strong source-level reference for a freestanding, bounded offline core. Port only after format/vector tests exist. |
| `StudySync`, pairing, root handling | Strong protocol reference. Reimplement with streamed SHA-256, fixed/bounded buffers, and ChinesePoint paths. |
| `tools_local/study` conversion and FSRS vectors | Reference and fixture source. Preserve MIT notices if copied. |
| `server/study-bridge` | Strong private-bridge reference. Start from an isolated ChinesePoint service; do not inherit its public-service assumptions. |
| CrossPlay display, app lifecycle, games, networking, full-image flasher | Do not import. They alter unrelated firmware behavior and weaken reviewability. |

## Improvements required over the audited reference

1. Verify streamed SHA-256 on every downloaded deck file. CrossPlay's
   `StudySync.h` explicitly records that it parsed manifest hashes without
   reading them on the device; length-only verification is insufficient.
2. Use whole-build staging plus an atomic directory switch. `deck.dat` and
   `cards.dat` are index-coupled and must never be individually promoted.
3. Store a hash alongside every ACK offset. An offset alone cannot distinguish
   an append-only journal from a replaced or rolled-back SD file.
4. Keep the current FreeInk controller path and ChinesePoint recovery gesture.
   CrossPlay functioning on one X4 Pro is useful evidence for that device, not
   permission to transplant display initialization into all panel variants.
5. Do not claim desktop-LAN export is AnkiWeb sync. Existing `CjkAnkiClient`
   remains a separate, manually invoked migration/export feature.

## License handling

CrossPlay is MIT. Before copying any source, add its copyright/license text to
`THIRD-PARTY-NOTICES.md`, name the imported file and commit in the PR, and keep
the copied file's required copyright header. Architecture, public protocol
facts, and independently written tests do not need a code-copy notice, but
must continue to cite this audit in the design history.
