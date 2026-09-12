# ChinesePoint Study / Anki sync v1

Status: design baseline. No AnkiWeb credential, remote bridge, or physical X4
Pro test is implemented by this document.

## Decision and scope

ChinesePoint adopts CrossPlay Study as the strong architectural reference for
offline decks, FSRS, versioned deck builds, append-only review logs, pairing,
and a job-based bridge. The reference was audited at CrossPlay `xteink`
commit `858a0bf1345b4bb65db0f8499f78ec7c61d36b3b` on 2026-09-12. It is MIT.

ChinesePoint does **not** replace its X4 Pro hardware base with CrossPlay.
FreeInk controller selection, `display.begin()` ordering, the X4 Pro-only
build target, and DOWN+POWER recovery remain ChinesePoint code. CrossPlay
source is considered only inside a new `src/chinesepoint/study/` subsystem and
its server/tooling counterparts. A copied source file must retain the required
MIT attribution in `THIRD-PARTY-NOTICES.md`; otherwise we reimplement its
protocol-level behavior from this specification.

The current `CjkAnkiClient` is a manually triggered, private-LAN Anki Desktop
import bridge. It is useful for exporting learner cards, but it has no
AnkiWeb pairing, remote deck download, FSRS-compatible two-way sync, or
server-durable review acknowledgement. It remains a separate transitional
export path and must not share tokens or files with Study v1.

## Product boundary

The first deploy is a private bridge for one operator or a small allowlist.
It can run on a home server, VPS, or SBC. The device never speaks AnkiWeb's
collection protocol and never holds an AnkiWeb password, cookie, or host key.

```text
Anki Desktop / Mobile <-> AnkiWeb <-> ChinesePoint Sync Bridge <-> X4 Pro
                                      HTTPS + device token
```

The bridge owns Anki's Python library, SQLite collection mirror, media
conversion, and account session credential. The X4 Pro owns rendering, local
scheduling, local durable reviews, and incremental download verification.

The maximum in v1 is eight chosen decks and sixteen retained local deck slots.
This limit is a capacity guard, not an assumption that a collection has eight
decks.

## Data layout on the SD card

```text
/study/
  .bridge                 pairing state, token, per-deck build and ACK state
  <deck-slug>/
    current/              one complete, verified server build
      meta.dat
      deck.dat
      cards.dat
      images.dat          optional in the first media-capable release
      fonts/              optional subset files
    previous/             last known-good build; one rollback generation
    revlog.dat            device-owned, append-only, never replaced by a build
    state.dat             device-owned card-state overlay and commit marker
```

Server builds never overwrite `revlog.dat` or `state.dat`. A deck update is a
new build directory downloaded to `.part`, verified as a complete set, then
renamed as a directory swap. If a power loss happens at any point, `current/`
or `previous/` remains complete. The device rejects a build whose version,
deck slug, file size, or SHA-256 does not match the signed-in bridge manifest.

`cards.dat` in a downloaded build is a compact snapshot. Local changes are
written immediately to `state.dat` and appended to `revlog.dat`; the bridge
merges them into the Anki mirror and provides a later fresh build. This avoids
writing arbitrary records in-place inside a build that can be replaced.

## Binary formats

All multibyte integer and IEEE-754 fields are little-endian. Every file begins
with an ASCII magic and `u16 schema_version`; unknown future versions are
rejected before any state is changed. Lengths are byte lengths. Text is valid
UTF-8 and is rejected when malformed or over the format limit.

### `meta.dat`

Header: `CPMETA1\0`, `u16 version`, `u16 flags`, `u32 body_bytes`, followed by:

| Field | Type | Purpose |
| --- | --- | --- |
| deck id | `i64` | Anki deck identity |
| build id | 20-byte ASCII | Immutable bridge build identity |
| collection created | `i64` | Anki collection epoch seconds |
| rollover hour | `u8` | Anki day boundary |
| desired retention | `f32` | FSRS target |
| maximum interval | `i32` | FSRS cap in days |
| new/review daily limits | `u16`, `u16` | Queue limits |
| FSRS algorithm / parameter count | `u8`, `u8` | `5,19` or `6,21`; unknown combinations are rejected |
| FSRS parameters | 21 × `f32` | FSRS-5 uses the first 19 and requires the remaining two to be zero |
| learning/relearning steps | up to 6 × `f32` each | Minute steps |
| deck name | `u8 length` + UTF-8 | Display only, maximum 63 bytes |

The bridge supplies the scheduler parameters used by the matching Anki deck.
FSRS-6 (`6,21`) is the current Anki path. FSRS-5 (`5,19`) remains supported
only as an explicit compatibility mode for older collections; its two unused
parameter slots are zero. The bridge and device must select the matching
engine from this pair, never reinterpret 19 values as FSRS-6 or pad them with
invented values. If the bridge cannot extract a supported configuration, it
marks the deck unsupported instead of silently scheduling with guessed values.

### `cards.dat`

Header: `CPCARDS1`, `u16 version`, `u16 record_bytes`, `u32 record_count`.
Records are fixed **56-byte** snapshots, indexed by position. The bridge must
write `record_bytes = 56`; a different value is rejected.

| Offset | Field | Type |
| --- | --- | --- |
| 0 | Anki card id / note id | `i64`, `i64` |
| 16 | state / step / flags | `u8`, `u8`, `u16` |
| 20 | due day / due minute / last review day | `i32`, `u16`, `i32` |
| 30 | stability / difficulty | `f32`, `f32` |
| 38 | elapsed / scheduled days | `u16`, `u16` |
| 42 | reps / lapses | `u16`, `u16` |
| 46 | last review ms | `i64` |
| 54 | reserved, zero | `u16` |

The reader uses card IDs rather than a position as its durable identity. A
server rebuild may reorder cards; a resume record or local review therefore
must resolve its card ID against the new snapshot before use.

### `deck.dat`

Header: `CPDECK1\0`, `u16 version`, `u8 field_count`, `u8 flags`, `u32 note_count`,
then a `u32` note-offset index. Each note is a bounded sequence of
`u16 byte_length + UTF-8 bytes` fields. v1 fields are `front`, `back`,
`extra`, `reading`, `meaning`, `example`, `example_reading`, and
`cloze_question`.

The bridge sanitizes Anki HTML, resolves fields and clozes, and emits this
representation. The device does not execute HTML, JavaScript, CSS, templates,
or arbitrary media names. Generic `front`/`back` is mandatory; CJK field
recognition improves rendering but never makes a deck unopenable.

### `revlog.dat`

Header: `CPREV1\0`, `u16 version`, followed by fixed **64-byte** entries. The
header is written only when the log is created. The exact entry layout is:

| Offset | Field | Type |
| --- | --- | --- |
| 0 | entry UUID | 16 raw random bytes |
| 16 | Anki card id | `i64` |
| 24 | review time | `i64` Unix milliseconds |
| 32 | rating / prior state / new state / flags | `u8`, `u8`, `u8`, `u8` |
| 36 | prior stability / new stability | `f32`, `f32` |
| 44 | prior difficulty / new difficulty | `f32`, `f32` |
| 52 | interval days | `i32` |
| 56 | elapsed days / duration milliseconds | `u16`, `u16` |
| 60 | CRC32 over bytes 0–59 | `u32` |

Ratings are 1–4. Reserved flag bits are zero in v1 and unrecognized flags are
rejected.

Entries are appended and flushed before the UI reports a rating complete. A
truncated final entry or bad CRC is ignored; earlier valid entries stay
available. The log is not compacted automatically. `ack_offset` is a byte
offset on an entry boundary and `ack_prefix_hash` authenticates bytes `[0,
ack_offset)` so an SD rollback or replacement cannot falsely claim reviews
were acknowledged.

### `state.dat`

Header: `CPSTATE1`, `u16 version`, `u16 record_bytes = 56`, `u32 record_count`,
`u64 generation`, `u32 records_crc32`; total 28 bytes. Each 56-byte overlay
record has: `i64 card_id`; `u8 state`; `u8 step`; `u16 flags = 0`;
`i64 due_at_ms`; `i64 last_review_at_ms`; `f32 stability`; `f32 difficulty`;
`u32 reps`; `u32 lapses`; `u64 last_revlog_offset`; and a trailing `u32 CRC32`
over the first 52 bytes.

A write creates and flushes a complete temporary `state.dat` with the records
CRC and then atomically replaces the prior file. The loader accepts only a
header whose count, records CRC, every record CRC, state value, and reserved
flags validate. It otherwise retains the prior state file and replays the
valid review log. `generation` increments only after the temporary file is
complete; it is not a substitute for the review journal.

## Pairing state machine

```text
Unpaired
  -> WiFi selection
  -> POST /api/pair/start
  -> Code shown / Polling
  -> Claimed(username, device_token)
  -> Physical confirmation
  -> Paired
  -> Revoked or Cancelled -> Unpaired
```

Pair codes expire after five minutes and polling is rate limited. The QR only
contains a pairing code URL, never a device token. The bridge claim page uses
normal browser authentication, CSRF protection, and an explicit confirmation.
The X4 Pro displays the returned account identity and stores the token only
after a physical confirm action. Cancel calls best-effort abandonment; TTL is
the fallback.

## Sync state machine

```text
Idle -> Connecting WiFi -> Uploading review tail -> Job queued -> Polling
  -> Downloading staged build -> Verifying all files -> Atomic swap -> Complete
                                      |                         |
                                      v                         v
                              Keep current build          Keep local revlog

Any state -> Needs attention (full sync / login / revoked / protocol error)
```

`POST /api/sync` contains the complete local state overlay and only the
unacknowledged tail of each `revlog.dat`. The bridge durably records every
valid review in its journal before returning an ACK offset. The device updates
its ACK only after it has durably saved the response and recomputed the prefix
hash. Retrying the same tail is safe because the bridge deduplicates by device
entry UUID, not by timestamp alone.

The job then synchronizes its Anki mirror, reapplies journaled reviews,
pushes them, and creates versioned builds only for changed decks. A job status
is `queued`, `receiving`, `syncing_anki`, `applying_reviews`, `building_decks`,
`ready`, `done`, `error`, or `needs_attention`. Wi-Fi is user initiated and
foreground only; it is disabled after the operation unless another foreground
feature needs it.

## HTTP contract

All requests use HTTPS, a pinned hostname, a bundled CA set plus optional
`/study/.bridge-roots.pem`, body limits, request timeouts, and no insecure TLS
mode. Device tokens are 32 random bytes, transported as bearer credentials and
stored as hashes server-side. The device token is revocable; physical SD
access can still steal it, so token rotation/revocation is the practical
response rather than a false claim of hardware-backed secrecy.

| Endpoint | Auth | Result |
| --- | --- | --- |
| `GET /healthz` | none | liveness only |
| `POST /api/pair/start` | none | code and polling token |
| `GET /api/pair/poll` | polling token | pending or claimed identity/token |
| `POST /api/pair/confirm` | browser session | binds claimed device |
| `GET /api/decks` | device token | bounded deck list/counts |
| `POST /api/decks/choose` | device token | selected deck names, max eight |
| `POST /api/sync` | device token | job id and durable ACK offsets |
| `GET /api/sync/status/{job}` | device token | state and versioned manifests |
| `GET /api/deck/{slug}/{build}/{file}` | device token | one declared file |
| `POST /account/devices/{id}/revoke` | browser session | revoke device token |

The bridge never returns a filesystem path supplied by the client. Deck,
build, and filename components are validated against an account-owned manifest
before opening a file.

## Conflict and rollback policy

Anki full-sync requests, unsupported schema changes, expired sessions, or
deck-identity conflicts produce `needs_attention`. The bridge never chooses
upload or download automatically. The device keeps the old deck, local
`state.dat`, and every review entry. A future successful sync replays the
journal idempotently.

Deck rollback swaps `previous/` back to `current/` only if `previous/` has a
verified manifest. Review rollback never occurs. Firmware recovery is entirely
separate: DOWN+POWER and the hashed X4 Pro backup retain their existing
contract and Study never writes firmware/update paths.

## Threat model

| Threat | Required control |
| --- | --- |
| Stolen pairing code | short expiry, polling limits, browser confirmation, device-side identity confirmation |
| Stolen SD card | revocable token, no password/cookie/host key, no false hardware-encryption claim |
| Bridge compromise | encrypted credential at rest, allowlist, per-user isolation, non-root/read-only container, request limits, logs without secrets |
| Credential stuffing | per-IP/global limits, username backoff, no password logging |
| Malformed deck/media | size limits, path allowlist, subprocess conversion, SHA-256 streamed on device |
| Wi-Fi loss or reboot | append-only journal, ACK after durable server commit, idempotent replay, staged build swap |
| AnkiWeb full-sync demand | freeze destructive changes, show needs-attention, retain journal |
| Wrong device / panel | Study never changes boot, recovery, FreeInk probe, or display initialization |

## Delivery phases and acceptance tests

1. **A — offline Study core.** Port/reimplement freestanding deck parser,
   FSRS-5 compatibility and FSRS-6 schedulers, `state.dat`, and `revlog.dat`.
   Tests: malformed files, all ratings, reboot after a rating, SD full/write
   failure, wrong clock, and separate Anki-derived FSRS-5/FSRS-6 vectors.
2. **B — deterministic bridge converter.** Private FastAPI bridge imports an
   Anki mirror and builds bounded generic/CJK decks. Tests: binary format,
   unsupported templates, cloze, media bounds, glyph subset cache, SHA-256.
3. **C — pairing and deck choice.** Device QR/confirm screens and allowlisted
   bridge. Tests: expiry, wrong code, duplicate claim, cancellation, revoked
   token, cross-user access.
4. **D — durable sync.** Job/poll, journal ingestion, ACK prefix hash,
   downloads and atomic directory swap. Tests: duplicate payload, every
   network cut point, server crash, interrupted download, bad hash, deck
   deletion/rename, full-sync required.
5. **E — Anki round trip.** A disposable Anki collection proves device-only
   reviews survive bridge and AnkiWeb synchronization. A test with only
   device reviews must appear in a fresh downstream Anki sync.
6. **F — reader integration.** Add-to-Anki creates outbox entries only after
   Study core and round trip are stable. It never makes normal reading depend
   on Wi-Fi or bridge availability.

Every phase must build the X4 Pro target and the three simulator controller
profiles. No phase clears the existing physical panel/recovery release gates.
