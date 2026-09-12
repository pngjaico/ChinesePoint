# Learner study-clock build check — 2026-09-12

This record covers the durable learner-clock foundation only. It does not make
local flashcard review, Anki schedule synchronization, or a release claim.

## Change under test

- A `StudyClock` maintains a non-decreasing logical time across reboot.
- A trusted wall-clock value can advance that time, but a backwards wall-clock
  adjustment cannot make cards due early.
- The learner journal now persists the clock as a sequenced `StudyClock`
  record, replays it before entry snapshots, and keeps it through compaction.

## Automated evidence

- WSL host build: `ctest --test-dir /tmp/chinesepoint-host-test --output-on-failure`
  passed **217/217** tests.
- The directly affected clock, journal, and repository suites passed **15/15**.
- X4 Pro environment: `pio run -e chinesepoint_x4pro` produced a valid
  application image and byte-identical `firmware.bin` / `update.bin` aliases.
- Artifact verifier result: target `xteink-x4-pro`, board tag
  `CROSSPOINT-BOARD-V1:x4pro;`, version `ChinesePoint-v0.6-x4pro`,
  `installable: false`.

## Artifact observation

The build-worktree application images were 5,379,744 bytes with SHA-256:

```text
ec4e2f86df129ab48e6b2709703e3a7bd7e885e07a95a0e7fc4ea9ea84b6c4a8
```

This hash is not a release artifact: it was built from uncommitted source and
must be rebuilt after the committing source revision is known.

`esp_idf_size` reported DIRAM 157,078/341,760 bytes and IRAM
16,384/16,384 bytes. The change did not create IRAM headroom; the zero-byte
IRAM margin remains a physical-release blocker.

## Remaining work

No firmware activity consumes this clock yet. The next change must atomically
persist a rating together with the updated clock, then expose only local,
due cards in a bounded review activity. Anki scheduling remains deliberately
unimplemented until a versioned conflict policy exists.
