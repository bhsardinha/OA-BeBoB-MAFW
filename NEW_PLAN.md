# NEW_PLAN.md — Full-system reverse-engineering pivot (2026-10-05)

> **Removal notice (2026-10-14):** all pre-pivot code and docs (`_legacy_pre_ghidra/`, `openaudio-fw1814/firmware-helper/`, `openaudio-fw1814/legacy-mod/`) were deleted from the working tree at the user's request. They are recoverable from git: `git show d1-approved-2026-10-14:<old path>` (tag `d1-approved-2026-10-14`, commit `a9cb63c`). References to those paths in this file or in `docs/system-map/` are historical. `openaudio-fw1814/` is intentionally empty; new code is built from scratch per `docs/BUILD-PLAN.md` once the user approves it.

## Why this exists

The from-scratch clone under `openaudio-fw1814/kext/` reached the point where every subsystem looks right in isolation — IOKit matching, classic DCL rings, AV/C transport, HSCI envelope, PCR connect, buffer ladder — yet the device never produces audible audio. TX stays silent; RX never calls back; clock lock is unverified. The 2026-10-01 "patch the reference binary" directive was the first attempt to escape the symptom-chasing loop. Loading the 2014 reference kext as-is on 10.13.6 was deemed too risky (panic-on-autoload, KPI drift, Info.plist incompatibilities) without first understanding *why* the reference works where our clone doesn't.

The decision as of **2026-10-05**: both the from-scratch kext and the ad-hoc "patch the binary" workflow are paused. Before any more code is written or any binary is patched, we produce a **complete, cross-referenced system map** of the FW1814 driver stack by disassembling and correlating every reference we have. The next kext — whether greenfield, forked clone, or surgically modified reference — is built *from that map*, not from guesses or partial understanding of individual subsystems.

> **The driver works only when sync + comm + control/transport are correct from the start, as one state machine.** Fixing one layer at a time has not worked and will not work. We produce the whole picture first, then build.

## Guiding principles

1. **No code until the map is done.** No new source files, no binary patches, no plist edits beyond the strict minimum needed to keep `_extracted/` loadable for runtime inspection.
2. **The map is the spec.** It supersedes `docs/01-PLAN.md` through `docs/14-UNIFIED-DEV-PLAN.md` as the authoritative design document. Those docs become historical.
3. **Every behavior has a provenance.** Each value, register write, timing, buffer size, DCL shape, and FCP command in the final design must cite its origin: a reference binary offset, FFADO file + line, snd-bebob file + line, or a dated hardware capture. "Because I think so" is forbidden; "because macfw does X" is forbidden unless macfw's source is cited.
4. **Mirror the reference exactly.** The deeper CLAUDE.md rule still stands: deviations only when 10.13.6 mechanically forbids the reference approach, each deviation annotated.
5. **CoreAudio + low latency is non-negotiable.** The map must prove the latency path end-to-end before implementation begins.

## Scope — what the map must contain

The deliverable is a new numbered doc tree under `docs/system-map/` with the following parts. Each part is a standalone markdown file with annotated code excerpts, Ghidra pseudocode, and (where relevant) FCP/HSCI byte traces.

### Part A — Static disassembly of `_extracted/`

- **A1. Reference kext (`M-AudioFireWireBeBoB.x86_64-slice`).** Ghidra pass over every class in the chain: match / probe / start / stop / willTerminate / performFormatChange / performSampleRateChange / clipOutputSamples / convertInputSamples / plus the HSCI, DCL, and clock methods. Each method gets a pseudocode block and a plain-English description. Virtual tables and inheritance mapped (`FW1814Device` → `IOAudioDevice` chain, `FW1814Engine` → `IOAudioEngine` chain).
- **A2. Reference control panel (Carbon `.nib` + binary).** Every control exposed in the panel traced to the kext call it drives, so we know the complete runtime control surface — not just the headline mixer.
- **A3. Reference prefpane.** Clock source UI, device-selection UI, panel-launch IPC. Identify any CoreAudio HAL property it sets.
- **A4. Reference MIDI plugin (`.plugin`).** CoreMIDI CFPlugIn entry points, how it opens the kext, MIDI wire format over the FireWire-isoch MIDI channel or the async FCP path.
- **A5. Reference LaunchDaemon / helper.** Firmware upload wire protocol, bootloader-mode detection, retry/backoff.
- **A6. Firmware (`fw1814.bcd`).** BCD section layout, upload chunking, verification signature. Not reverse-engineering the DSP code — just the loader-visible envelope.

### Part B — Cross-reference with open-source references

- **B1. FFADO `libffado/src/bebob/maudio/`.** Each HSCI register, clock-source selector, mixer path, and sample-rate command correlated with A1. Mark FFADO entries that disagree with the Mac reference (FFADO is Linux-userspace; the Mac kext is authoritative for CoreAudio semantics).
- **B2. ALSA `snd-bebob`.** AM824 packing, isoch channel allocation, CMP/PCR handshake sequencing, plug/subunit enumeration.
- **B3. Community Sierra kextload hack (`_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/`).** What exactly did they change to force load on Sierra? Those are the known-good 10.11→10.12 deltas; they bound what 10.13 will need.
- **B4. macfw (if applicable).** User-cited reference. Catalog every subsystem it covers and cross-link to A1.
- **B5. Snow Leopard live reference (192.168.100.17).** DTrace / ioreg snapshots of the real driver running against the real hardware. Any FCP capture here is ground truth and outranks disassembly guesses.

### Part C — Unified system map

Produced *after* A and B, this is the whole-system view:

- **C1. Lifecycle state machine.** Every state the device can be in (bootloader, enumerated-not-opened, opened-no-clock, clock-locked-no-plugs, plugs-connected-no-stream, streaming, format-changing, terminating) with the exact IOKit / FCP / HSCI transitions between them. Each edge annotated with its trigger and its side effects.
- **C2. Sync path.** Clock source selection → PLL lock → sample rate propagation → isoch cycle-timer alignment → CoreAudio sample-counter wrap. End-to-end. This is where the current clone is weakest and where silence likely originates.
- **C3. Comm path.** FCP command queue, HSCI envelope, async notifications, serialization against isoch start/stop. Includes the "stream kick" sequence verified in auto-memory.
- **C4. Transport / control path.** CMP/PCR plug map, DCL ring shapes (TX + RX, exact segment layout as decoded in auto-memory), buffer-ladder effects on both DCL ring and CoreAudio PCM buffer, format-change choreography.
- **C5. Userclient surface.** Full selector table drawn from A2+A3+A4 (panel + prefpane + MIDI plugin). Supersedes `docs/12-USERCLIENT-DESIGN.md`.
- **C6. Latency budget.** Round-trip microseconds attributed to each stage (CoreAudio IO → clip → DCL send → cable → device → device loop → DCL receive → convert → CoreAudio IO). Must match or beat the 2014 driver.

### Part D — Build decision

Once C is complete we re-evaluate the three paths with evidence:

- **[Decided 2026-10-14: D2 chosen by the user; D1/D3 withdrawn - see `docs/BUILD-PLAN.md`]** **D1.** Ship the modified reference bundle (2026-10-01 directive) — now feasible because we know exactly which methods KPI drift affects.
- **D2.** Resume the from-scratch clone — only if C exposes a concrete, mechanical blocker in the reference binary on 10.13.6 that patching cannot address.
- **D3.** Hybrid — reference binary as the IOKit shell, our code only for the subsystems where the reference is genuinely incompatible with 10.13.6 kernel ABI.

**No code before D is decided.** The decision is a one-page `docs/system-map/D-decision.md` with the chosen path and the specific pieces of C it leans on.

## Order of operations

1. **Freeze current state.** Build PC kext already unloaded + removed (done 2026-10-05). Pre-Ghidra `kext/` + `control-panel/` scaffolds archived to `_legacy_pre_ghidra/` (2026-10-09 sanitization) — reference only, no edits. `openaudio-fw1814/firmware-helper/` + `openaudio-fw1814/legacy-mod/` remain in place.
2. **Scaffold `docs/system-map/`.** Create the directory with placeholder files for A1–A6, B1–B5, C1–C6, D.
3. **Execute Part A.** Ghidra sessions on each `_extracted/` binary. Each A-file lands with pseudocode + prose on commit.
4. **Execute Part B.** Open-source correlation pass. B-files reference specific A-file sections.
5. **Execute Part C.** Synthesize. This is the hard thinking.
6. **Decide D.** Write the one-pager. Only then does code work resume.

## What this changes in CLAUDE.md

- The 2026-10-01 "patch the reference first" workflow is **superseded** by the map-first workflow. Patching the reference may still be the final answer, but only after Part C.
- The "mirror the reference, don't invent" rule is preserved and strengthened — now backed by a formal provenance requirement.
- Milestone list (`docs/06-MILESTONES.md`) is paused. The new near-term milestones are the sections of this doc, in order.
- No new code under `openaudio-fw1814/firmware-helper/`, no revived code under `_legacy_pre_ghidra/`, no fresh `openaudio-fw1814/kext/` or `openaudio-fw1814/control-panel/` tree, until D-decision lands.

## Non-goals of this pivot

- Rewriting the extracted driver "better." We are documenting it, not critiquing it.
- Adding features beyond what the 2014 stack shipped (feature parity first).
- Modernizing APIs (AudioServerPlugIn, DriverKit, Metal, etc.) — out of scope until the kext works.
- Supporting any macOS version other than 10.13.6 on the shipping path.

## Resume anchors

If a session resumes mid-map, read in order:
1. This file (`NEW_PLAN.md`).
2. `docs/system-map/README.md` (once scaffolded) — shows which Part N file is in flight.
3. The in-flight Part N file itself — each one carries its own "status / next step" header.
4. Auto-memory as usual.

Pre-existing resume-anchor docs (`docs/10`, `docs/11`, `docs/12`, `docs/13`, `docs/14-UNIFIED-DEV-PLAN.md`) are now historical. `docs/14-UNIFIED-DEV-PLAN.md` is on disk at `_legacy_pre_ghidra/docs/`; 10–13 are only in git history. They may still inform Part C but are not the current spec.
