# Community hardware testing for ChinesePoint X4 Pro

Status: **preparation only**. There is no public ChinesePoint firmware release
or installable test binary at this time. This document does not authorize
flashing the currently blocked artifact.

Community reports are the only practical way to cover the three X4 Pro panel
controller paths when the maintainer has no unlocked device available. They
can add real evidence, but they cannot turn a report into a release by
assertion. Each panel row and the recovery route require its own reproducible
record.

## What maintainers must provide before inviting a test

A maintainer must first publish, in a pinned issue or test assignment:

1. a specific commit, application-only `firmware.bin`/`update.bin` SHA-256,
   byte count, and `verify_artifact.py` output;
2. a declaration that it is a **hardware-test candidate**, never a release;
3. the exact known-good X4 Pro recovery application SHA-256 required for the
   drill; and
4. a link to the candidate's simulator matrix and its unresolved risks.

No request may point testers at a branch head, a GitHub Actions artifact, a
simulator screenshot, `installable: false` artifact, or a USB rescue image.
The candidate remains non-installable until the release policy evidence gate is
satisfied.

## Who should not test

Do not volunteer if the X4 Pro is locked and irreplaceable, if it is your only
reader, if DOWN + POWER recovery has not already been proven with known-good
firmware, or if you cannot stop and document a failure. A candidate can blank,
mirror, invert, stop accepting input, reboot, or fail to recover. No simulator
or compilation result removes those risks.

## Required report

Use the **X4 Pro hardware-test report** issue form and include all required
fields. The report must contain the full candidate commit and SHA-256,
`x4pro-panel-probe.txt`, the panel path selected by FreeInk, observations for
boot/orientation/refresh/touch/frontlights/reader/sleep/Wi-Fi, and the recovery
result. Attach photos or logs when they help diagnose a fault. Never publish
an Anki token, Wi-Fi password, private IP address, or personal reading data.

A missing probe report is a stop condition, not an acceptable substitute for a
panel classification. Reports for an unassigned candidate are triaged as
invalid and cannot be used in the release manifest.

## Evidence handling

Maintainers transcribe only verifiable observations into the appropriate files
under `docs/chinesepoint/evidence/`. A pass from one device covers only the
controller path reported by that device. Conflicting or incomplete reports
remain pending. The full matrix, recovery drill, and reader acceptance gates
are defined in [physical-validation.md](physical-validation.md) and
[reader-acceptance.md](reader-acceptance.md).