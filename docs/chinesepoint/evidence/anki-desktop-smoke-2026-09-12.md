# Anki Desktop bridge smoke evidence — 2026-09-12

This record covers the Windows workstation only. It does not claim an X4 Pro
transfer or a completed vocabulary import.

- Anki Desktop 26.8.1 was available with the `Usuário 1` profile.
- The development copy of `chinesepoint_anki_bridge` was installed under
  `Anki2/addons21/chinesepoint_anki_bridge` after every source file hash matched.
- The packaged add-on contained the expected five files. Its SHA-256 was
  `b627e2bfeefce71048cb32e40c48b5cd03e7f99a10e84c7043c4888f19b7bde0`.
- With the profile open, the bridge generated its local token, bound
  `0.0.0.0:5051`, and rejected an unauthenticated request to
  `/v1/cjk/vocabulary` with HTTP 401.

The test intentionally did not send a valid vocabulary batch: doing so would
create or update cards in the user's real collection. It therefore does not
prove first import, idempotent retry, update, conflict handling, or offline
retry. Those are still required with the connected X4 Pro.

## Disposable real-collection runtime smoke

After the user-profile smoke, a fresh disposable base was created with Anki
26.8.1's own `Collection` runtime. No file in `%APPDATA%\\Anki2` was opened or
modified. The current bridge source was instantiated against that real
collection and received three authenticated HTTP requests on a local temporary
port:

1. the first batch returned HTTP 200 with `added: 1, updated: 0`;
2. an identical retry returned HTTP 200 with `added: 0, updated: 0`;
3. a later batch for the same `ChinesePointId` returned HTTP 200 with
   `added: 0, updated: 1`.

The collection contained exactly one note, its `Sentence` field held the later
value, and the `ChinesePoint Vocabulary` model existed. This closes the gap
between the fake-collection unit test and Anki's actual collection API.

The rebuilt add-on package containing the profile-readiness gate has SHA-256
`d1993e53fa72a1680598eafd21e36149fe321548ce329c913262eddcbab69860`.

It does **not** prove that Anki Desktop's GUI has opened the add-on against a
fresh profile in this automation environment, or that an X4 Pro can reach the
workstation across Wi-Fi. The latter remains a physical release gate. Commit
`b969845` also prevents the add-on from listening before an Anki profile is
open, so a reader cannot wait behind a profile-selection or collection-opening
screen.

An unrelated existing add-on, AnkiMCP 0.28.0, showed `AnkiMCP Server - Setup
Failed` on this Anki 26.8.1 installation and blocked normal profile startup.
It was disabled only for this smoke session, then restored to its prior enabled
state. ChinesePoint does not depend on AnkiMCP. Before a real sync session, the
user must update, disable, or dismiss that failing add-on without treating its
failure as a ChinesePoint result.
