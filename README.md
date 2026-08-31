<p align="center">
  <img src="assets/chinesepoint-hero.png" alt="ChinesePoint running on an XTEINK X4 Pro" width="100%" />
</p>

<h1 align="center">ChinesePoint</h1>
<p align="center"><strong>Read Chinese. Learn naturally.</strong></p>
<p align="center">A Chinese-learning firmware project for the XTEINK X4 Pro, built on CrossPoint.</p>

<p align="center">
  <a href="https://app.royalty.dev/pngjaico/ChinesePoint"><img alt="Fund contributors" src="https://img.shields.io/badge/%F0%9F%91%91_Fund_contributors-royalty.dev-BB953A?style=for-the-badge&labelColor=1a1a1a" /></a>
</p>

<p align="center">
  <img alt="Target" src="https://img.shields.io/badge/target-XTEINK_X4_Pro-9f2f25" />
  <img alt="Status" src="https://img.shields.io/badge/status-v0.5_active_development-BB953A" />
  <img alt="Foundation" src="https://img.shields.io/badge/built_on-CrossPoint-222222" />
  <img alt="Learning core" src="https://img.shields.io/badge/learning-offline--first-56634f" />
</p>

> **Development status**  
> ChinesePoint v0.5 is under active native development. The source/checkpoint currently covers the native reader bridge, Chinese lookup, crash-safe vocabulary persistence, stable source anchors and local spaced-repetition reviews. Translation, trusted time and Anki sync are roadmap work. **A public stable installable firmware release is not being claimed yet.**

---

## What is ChinesePoint?

ChinesePoint is a custom e-reader firmware project built on top of **CrossPoint**, focused on turning the **XTEINK X4 Pro** into a dedicated device for learning Chinese through extensive reading.

The idea is simple: keep everything CrossPoint already does well as an e-reader, then add a Chinese-learning layer directly inside the reading experience.

```text
Read → Understand → Save → Learn → Review → Back to the book
```

Instead of switching between an e-reader, dictionary, phone and flashcard app, ChinesePoint aims to keep that loop on the device.

## What's different in this fork?

CrossPoint is the platform. ChinesePoint adds a narrowly scoped learning layer instead of replacing the reader or hardware stack.

### Current v0.5 source highlights

- **Native Chinese lookup inside EPUB reading** — touch a rendered word without leaving the reader.
- **Smart multi-character matching** — longer useful candidates are preferred before shorter matches.
- **CC-CEDICT-aware CJK index** — compact lookup index for Chinese candidate detection.
- **CrossPoint StarDict definitions** — dictionary definitions remain owned by CrossPoint's existing dictionary stack.
- **Word / sentence / breakdown context** — keep the lookup tied to what you were actually reading.
- **Vocabulary states** — `Encountered`, `Saved`, `Learning`, and `Known`.
- **Crash-safe learner storage** — journaled SD-card persistence designed to recover from interrupted writes.
- **Stable source anchors** — vocabulary can point back to the logical location where it was found.
- **View in Book** — jump back to the saved source context.
- **Offline spaced-repetition reviews** — local `Again / Hard / Good / Easy` review loop and history.
- **Large-Hanzi e-ink review UI** — scalable rendering without bundling multiple giant CJK font copies.
- **Offline-first architecture** — the learning loop does not require Wi-Fi.

### Planned

- Sentence translation with an on-device cache
- Trusted time / NTP-aware scheduling
- CJK Relay + Anki / AnkiConnect synchronization over Wi-Fi
- Deeper reading and learning statistics
- Memory / heap profiling and physical-device hardening
- Public release packaging after build and hardware acceptance gates

---

## Supported target

### XTEINK X4 Pro

ChinesePoint is currently being designed and integrated specifically around the **XTEINK X4 Pro** path in CrossPoint/FreeInk.

The project does **not** assume that hardware-specific changes for unrelated forks are safe on the Pro. Boot, recovery, display, touch, storage, power and update behavior stay as close to CrossPoint upstream as possible.

> ChinesePoint is an independent community project. It is not affiliated with XTEINK and is not an official CrossPoint release.

---

## How it works

```text
XTEINK X4 Pro
      │
      ▼
FreeInk / HAL
      │
      ▼
CrossPoint
├── EPUB reader
├── StarDict
├── UI / touch
├── storage
├── Wi-Fi
├── sleep / power
└── recovery / update
      │
      ▼
ChinesePoint learning layer
├── TextAnchor bridge
├── CJKLEX2 candidate index
├── ChineseEngine
├── LearnerStore
├── vocabulary states
├── local flashcards
└── review history
```

The important design decision is what ChinesePoint **doesn't** rewrite. CrossPoint and FreeInk continue to own the platform and recovery-critical pieces. ChinesePoint hooks into the reader through small, auditable interfaces.

---

## Learning flow

### 1. Read
Open a normal EPUB in CrossPoint.

### 2. Understand
Tap Chinese text. ChinesePoint uses the reader's logical text offsets to find the tapped context and resolve useful word candidates.

### 3. Save
Save the resolved headword together with its sentence, book and stable anchor.

### 4. Learn
Move vocabulary through `Saved → Learning → Known` and review it locally.

### 5. Review
Use the on-device spaced-repetition queue, then jump back to the original book context when useful.

---

## Why not build another reader?

CrossPoint already provides a mature foundation for EPUB rendering, StarDict dictionaries, file management, custom fonts, Wi-Fi workflows, sleep/recovery and the hardware abstraction used by these devices. Its own README intentionally describes community forks as layers that add more specialized behavior while CrossPoint itself moves conservatively for stability.

ChinesePoint follows that philosophy: **specialize the learning experience without duplicating the platform**.

---

## Project status

| Area | Status |
| --- | --- |
| Native reader / text-anchor bridge | ✅ v0.5 source |
| Chinese candidate matching | ✅ v0.5 source |
| CC-CEDICT index + StarDict handoff | ✅ v0.5 source |
| Saved / Learning / Known | ✅ v0.5 source |
| Crash-safe LearnerStore | ✅ v0.5 source |
| Stable anchors / View in Book | ✅ v0.5 source |
| Local SRS reviews + history | ✅ v0.5 source |
| Sentence translation cache | 🟡 Planned |
| Trusted time / NTP | 🟡 Planned |
| Anki sync / CJK Relay | 🟡 Planned |
| Public stable firmware `.bin` | ⏳ Not released yet |

---

## Roadmap

### v0.5 — Native foundation

1. **Native Reader & Dictionary** — text anchors, CJK matching, CC-CEDICT index, StarDict definitions.
2. **Learning & Persistence** — vocabulary states, crash-safe SD journal, lookup/source context.
3. **Flashcards & Anchors** — local review queue, history, large Hanzi UI, View in Book.
4. **Translation & Trusted Time** — cache-first sentence translation, NTP-aware scheduling.
5. **Anki & Hardening** — CJK Relay, Anki sync, diagnostics, heap profiling and physical-device QA.

The goal is not to add every possible app to the X4 Pro. ChinesePoint stays focused on **reading Chinese better**.

---

## Funding

If ChinesePoint is useful to you, you can help fund development, hardware testing and future releases:

[![👑 Fund contributors](https://img.shields.io/badge/%F0%9F%91%91_Fund_contributors-royalty.dev-BB953A?style=for-the-badge&labelColor=1a1a1a)](https://app.royalty.dev/pngjaico/ChinesePoint)

**Royalty.dev:** https://app.royalty.dev/pngjaico/ChinesePoint

GitHub funding metadata is also included in `.github/FUNDING.yml`:

```yaml
custom: ["https://app.royalty.dev/pngjaico/ChinesePoint"]
```

---

## Website

The repository includes a dependency-free static project site in the repository root:

```bash
python -m http.server 8080
```

Then open `http://localhost:8080`.

It can be deployed directly to **GitHub Pages, Netlify, Vercel, Cloudflare Pages**, or any static host.

---

## Development philosophy

ChinesePoint borrows two principles from the projects around it:

- From **CrossPoint**: preserve a small, stable e-reader core and keep recovery-critical code conservative.
- From **CrossInk**: specialize the reader experience through focused additions rather than reinventing the entire platform.

For ChinesePoint that means:

- reuse CrossPoint's existing systems whenever possible;
- isolate Chinese-specific code;
- keep the learning core offline-first;
- write data to the SD card defensively;
- prefer small native hooks over invasive reader rewrites;
- never ship a fake or unverified firmware artifact just to call a milestone “done”.

---

## Credits

ChinesePoint is possible because of the open-source work around the XTEINK ecosystem.

- **[CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader)** — the e-reader firmware foundation.
- **[FreeInk](https://github.com/Free-Ink/freeink-sdk)** — hardware / UI / HAL foundation used by CrossPoint.
- **[CC-CEDICT](https://www.mdbg.net/chinese/dictionary?page=cedict)** — open Chinese-English dictionary data used by the CJK lookup workflow.
- **[CrossInk](https://github.com/uxjulia/CrossInk)** — useful reference for focused reader extensions, typography, statistics and e-ink UX patterns.
- The wider CrossPoint/XTEINK community for testing, reverse engineering, bug reports and ideas.

ChinesePoint is not affiliated with XTEINK or any device manufacturer.

---

<p align="center"><strong>ChinesePoint</strong><br />Read Chinese. Learn naturally.</p>
