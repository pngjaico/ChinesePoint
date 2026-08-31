# ChinesePoint v0.6 Foundation Implementation Plan

> For agentic workers: REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Deliver the X4 Pro-only ChinesePoint foundation with recovery-safe CJK isolation and repeatable simulator and release evidence.

**Architecture:** ChinesePoint stays a narrow downstream of current CrossPoint. Current FreeInk owns X4 Pro panel detection and CrossPoint owns image validation. ChinesePoint adds a CJK crash guard after recovery routing, a dedicated X4 Pro build identity, non-root screenshots, and a manifest-gated simulator/site delivery path.

**Tech Stack:** C++20, Arduino Preferences, PlatformIO ESP32-S3, CrossPoint simulator, Python, GitHub Actions, static web assets.

**Spec:** docs/superpowers/specs/2026-08-31-chinesepoint-v0.6-foundation-design.md

## Global Constraints

- Publish and support Xteink X4 Pro only.
- Preserve DOWN plus POWER recovery and existing image validation.
- Do not initialize CJK before recovery routing.
- Application and full USB images are distinct.
- Simulator proof is required for every milestone but is not physical-panel proof.

---

### Task 1: Fork identity and public scope

**Files:** README.md, docs/chinesepoint/release-policy.md, test/chinesepoint/test_release_scope.py

- [ ] Write a failing test asserting README contains Xteink X4 Pro only and excludes inherited X3/X4 support claims.
- [ ] Run python -m pytest -q test/chinesepoint/test_release_scope.py and observe failure.
- [ ] Replace inherited public claims with ChinesePoint X4 Pro policy, status labels, recovery distinction, and upstream attribution.
- [ ] Add release-policy.md defining app image, full USB image, hash, simulator, physical test, and rollback requirements.
- [ ] Re-run the scope test and commit docs: define ChinesePoint X4 Pro release boundary.

### Task 2: Dedicated X4 Pro artifact gate

**Files:** platformio.ini, scripts/chinesepoint/verify_x4pro_artifact.py, test/chinesepoint/test_artifact_gate.py

- [ ] Write a failing test for a missing chinesepoint_x4pro environment and reject a manifest using environment default.
- [ ] Run the test and observe failure.
- [ ] Add chinesepoint_x4pro with FREEINK_DEVICE_X4PRO, CHINESEPOINT, CHINESEPOINT_X4PRO_ONLY, ChinesePoint USB metadata, and X4 Pro version metadata.
- [ ] Implement a verifier requiring ESP32-S3, X4 Pro board tag, app versus full classification, SHA-256, and environment identity.
- [ ] Build with pio run -e chinesepoint_x4pro and run the verifier on firmware.bin.
- [ ] Commit build: add X4 Pro-only ChinesePoint artifact gate.

### Task 3: CJK crash isolation

**Files:** src/chinesepoint/CjkSafetyGuard.h, src/chinesepoint/CjkSafetyGuard.cpp, src/main.cpp, test/chinesepoint/cjk_safety_guard_test.cpp, test/chinesepoint/test_boot_order.py

- [ ] Write failing host tests for first panic, second panic safe mode, ordinary exit, NVS error, and explicit clear.
- [ ] Write a failing source test proving recoveryFirmwareMode is resolved before cjk_safety::reconcileBoot.
- [ ] Run both tests and observe failure.
- [ ] Implement the small NVS state machine specified in the design.
- [ ] Call reconcileBoot only after recovery decision; never redirect boot or instantiate CJK code from main.cpp.
- [ ] Re-run host/source tests and build chinesepoint_x4pro.
- [ ] Commit feat: isolate CJK failures behind safe mode.

### Task 4: Recovery-clean screenshots

**Files:** src/util/ScreenshotUtil.cpp, test/chinesepoint/test_screenshot_path.py

- [ ] Write a failing source test requiring /.chinesepoint/screenshots and rejecting /screenshots.
- [ ] Run it and observe failure.
- [ ] Change only the screenshot directory prefix; preserve filename safety and partial-write cleanup.
- [ ] Run the test and build chinesepoint_x4pro.
- [ ] Commit fix: isolate screenshots from recovery media.

### Task 5: Mandatory X4 Pro simulator

**Files:** platformio.sim.ini, platformio.ini, scripts/chinesepoint/run_simulator_smoke.py, test/chinesepoint/test_simulator_config.py, .github/workflows/chinesepoint-ci.yml

- [ ] Write a failing configuration test requiring simulator_x4pro, SIMULATOR_DEVICE_X4_PRO, and excluded firmware-flashing sources.
- [ ] Run it and observe failure.
- [ ] Add official simulator configuration and a runner which fails when Home and Settings BMP screenshots are absent or empty.
- [ ] Add Linux CI to build chinesepoint_x4pro, build simulator_x4pro, run smoke, and upload QA artifacts.
- [ ] Run static tests locally and run simulator build in Linux CI.
- [ ] Commit test: require X4 Pro simulator evidence.

### Task 6: Manifest-gated site source

**Files:** site/, scripts/chinesepoint/build_site_manifest.py, test/chinesepoint/test_site_manifest.py

- [ ] Write a failing test that rejects missing hash, wrong board, non-ChinesePoint environment, full image as on-device update, and hardware_verified false as installable.
- [ ] Run it and observe failure.
- [ ] Import only clean HTML, CSS, JS, and assets from the supplied site reference. Exclude caches, embedded checkpoints, stale manifest, and unreproducible QA images.
- [ ] Generate site/releases/manifest.json only from verified build metadata. Hide install buttons for diagnostic status.
- [ ] Re-run site tests and commit feat: add manifest-gated ChinesePoint release site.

### Task 7: Foundation verification and publication

- [ ] Run python -m pytest -q test/chinesepoint.
- [ ] Run pio run -e chinesepoint_x4pro and inspect firmware.bin with esptool image-info.
- [ ] Run artifact verifier and verify its generated manifest.
- [ ] Run simulator smoke on Linux CI.
- [ ] Run git diff upstream/develop...HEAD --check and git status --short.
- [ ] Push cjk/part2.5-safety and create a draft PR to develop with build hash, simulator result, physical-test status, recovery route, and remaining device checklist.
