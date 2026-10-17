# System map — resume index

> **Removal notice (2026-10-14):** all pre-pivot code and docs (`_legacy_pre_ghidra/`, `openaudio-fw1814/firmware-helper/`, `openaudio-fw1814/legacy-mod/`) were deleted from the working tree at the user's request. They are recoverable from git: `git show d1-approved-2026-10-14:<old path>` (tag `d1-approved-2026-10-14`, commit `a9cb63c`). References to those paths in this file or in `docs/system-map/` are historical. `openaudio-fw1814/` is intentionally empty; new code is built from scratch per `docs/BUILD-PLAN.md` once the user approves it.

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - The HSCI clock-source envelope is **16 bytes** (len `0x10` @0xe441), not 12; the 12-byte layout is snd-bebob `bebob_maudio.c:183-198`. Full evidence log: `PROVENANCE-RESOLUTIONS.md`.


Current spec: `/NEW_PLAN.md`. This tree is the deliverable it describes.

Rule: no new source files, no binary patches, no plist edits anywhere in the repo until Part D lands. The map is produced first; the next kext is built from the map.

Provenance is non-negotiable. Every value, register write, DCL shape, buffer size, FCP command, and timing recorded here must cite its origin: binary offset in `_extracted/`, FFADO file:line, snd-bebob file:line, or dated DTrace/FCP capture from the SL reference Mac at 192.168.100.17.

## Status column meaning

- **todo** — not started.
- **in-progress** — file has a `## Status / next step` header pointing at the exact next action. Open the file to resume.
- **done** — section is complete and cited; later parts may reference it.

## Parts

### Part A — Static disassembly of `_extracted/`

| File | Target | Status |
|---|---|---|
| `A1-ghidra/` | `_extracted/kext/` — class chain, probe/start/stop, DCL, HSCI, clock, mixer, user-client | done (Q1–Q17, Q-INFO, Q-P1P2…Q-P4, Q-MB landed; see `A1-ghidra/README.md`) |
| `build-pc-runs/` | dated logs of what the **clone** did on the 10.13.6 build PC (M-A/M-FW/install 2026-10-15). Observations of our code, not of the reference | M-A, M-FW, H.1 verified; M-B+ not run (airliner rule) |
| `A2-panel/` | `_extracted/panel/` — Carbon `.nib` + binary, every control → kext call | done (P1–P10 landed; see `A2-panel/README.md`) |
| `A3-midi/` | `_extracted/midi/` — CoreMIDI CFPlugIn entry points, wire format, endpoint model, sleep | done (M1–M9 landed, M10 closed; see `A3-midi/README.md`) |
| `A4-helper/` | `_extracted/helper/` — LaunchDaemon + status app, firmware upload protocol, bootloader detection | done 2026-10-09 (H1–H10 landed; see `A4-helper/README.md`) |
| `A5-firmware.md` | `_extracted/firmware/fw1814.bcd` — BCD section layout, upload chunking, verification | done 2026-10-09 (full header + both CRCs verified; BridgeCo attribute namespace enumerated) |
| `A6-prefpane.md` | `_extracted/prefpane/` — clock source UI, device selection, panel-launch IPC | done 2026-10-09 (trampoline-only; launches panel via `[NSWorkspace launchAppWithBundleIdentifier:]`; no IPC/IOKit/FireWire) |

**Part A complete 2026-10-09.** All 6 components decoded; corrections to A4-H4/H9/H10 propagated. Next: Part B — the only hardware-input step (B5 live DTrace against the SL Mac Pro at 192.168.100.17) is the gate for Part C sync/comm paths.

### Part B — Cross-reference with open-source references

| File | Source | Status |
|---|---|---|
| `B1-ffado-correlate.md` | `libffado/src/bebob/maudio/` — HSCI regs, clock selectors, mixer, sample-rate | **done 2026-10-13** (Q2/Q5/Q6/Q15/Q15c cross-referenced; FFADO stubs clock+HSCI+rate so A1 is sole source there; found candidate A1-Q15f = second mixer path at `0xffc7:0x00700000+` via `CustomMixerWriteQuads @ 0x22c7a`; 40-quadlet register-offset table lifted from `special_avdevice.h:50..134`) |
| `B2-snd-bebob-correlate.md` | ALSA snd-bebob — AM824 packing, isoch channels, CMP/PCR, plug/subunit | **done 2026-10-13** (Linux uses NuDCL so A1-Q1 is sole source; AM824 silence delta — `_extracted/`=`0xCF` vendor idle, snd-bebob=`0x40` standard MBLA; three A1 gaps flagged — `CIP_EMPTY_HAS_WRONG_DBC` at ≥88 kHz, post-registration bus-reset gap-count fix, HSCI byte 9 = `clk_lock` latch; stream-kick 100 ms matches `special_set_rate`) |
| `B2a-rust-bebob-correlate.md` | snd-firewire-ctl-services (Rust userspace, Takashi Sakamoto) — 46 MA* keys onto 22×4 + 22×2 mixer; A1-Q15b dispatcher | **done 2026-10-13** (160-byte `MaudioSpecialStateCache` + 18 contiguous offset-ranges validate A1-Q15b single-dispatcher; `Fw1814ClkProtocol` deliberately omits clock-src selection — confirms `MAClockSource` is HSCI-only on macOS; stream pairs 2/3+ bypass mixer — explains Q13 4-engine split; `HeadphoneSource` resolved to 3-valued enum) |
| `B3-sierra-hack-delta.md` | `_vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/` — exact 10.11→10.12 deltas, and why that path doesn't carry to 10.13 | **done 2026-10-13** (same unsigned binary split into two plists for sequential kextload; same binary as installer v1.10.5; three cumulative 10.13 breakers = UAKEL + SHA-256 CMS + IOAudioFamily/IOFireWireFamily reserved-slot drift; recommends clone-rebuild for Part D) |
| `B4-macfw-correlate.md` | macfw project — subsystem coverage cross-link to A1 | **done 2026-10-13** (macfw bypasses HSCI entirely for mixer — uses the same `0xffc7:0x00700000+` window as FFADO — **independent corroboration of A1-Q15f gap**; macfw uses NuDCL from userspace — non-starter for our kernel 10.13.6 kext, but validates per-rate packet geometry; macfw does not upload firmware — only sends boot-from-flash cue; macfw has no clock-source dropdown and no ADAT, so Part C digital surface relies entirely on A1 + FFADO + snd-bebob) |
| `B5-sl-dtrace-truth/` | 192.168.100.17 live captures — FCP, ioreg, DTrace of running v1.9 driver | **done 2026-10-13** (10 headline findings including composite `MAClockSource` bitfield + `clientMemoryForType(1)` meter path; remaining rate-change + rate-locked questions reclassified as A1-Q15c/Q15d static follow-ups per B5 §13; see `B5-sl-dtrace-truth/README.md`) |

### Part C — Unified system map (synthesis — do after A and B)

| File | Scope | Status |
|---|---|---|
| `C1-lifecycle-state-machine.md` | All device states + IOKit/FCP/HSCI transitions, triggers, side effects | **consolidated 2026-10-14 (P5, R35)** — one consistent version; old text in git at `a9cb63c`; unverified items stay labelled |
| `C2-sync-path.md` | Clock source → PLL lock → sample rate → cycle timer → CoreAudio counter | **consolidated 2026-10-14 (P5, R35)** — one consistent version; old text in git at `a9cb63c`; unverified items stay labelled |
| `C3-comm-path.md` | FCP queue, HSCI envelope, async notifications, isoch serialization | **consolidated 2026-10-14 (P5, R35)** — one consistent version; old text in git at `a9cb63c`; unverified items stay labelled |
| `C4-transport-control-path.md` | CMP/PCR map, DCL TX/RX ring shapes, buffer ladder, format-change choreography | **consolidated 2026-10-14 (P5, R35)** — one consistent version; old text in git at `a9cb63c`; unverified items stay labelled |
| `C5-userclient-surface.md` | Full selector table from panel + prefpane + MIDI plugin (supersedes `docs/12`) | **consolidated 2026-10-14 (P5, R35)** — one consistent version; old text in git at `a9cb63c`; unverified items stay labelled |
| `C6-latency-budget.md` | Round-trip microseconds per stage; must match or beat 2014 driver | **consolidated 2026-10-14 (P5, R35)** — one consistent version; old text in git at `a9cb63c`; unverified items stay labelled |

### Part D — Build decision

| File | Scope | Status |
|---|---|---|
| `D-decision.md` | One page: modified reference bundle / resumed clone / hybrid, with citations from C | **D2 CHOSEN by the user (2026-10-14)**; D1/D3 withdrawn; working plan `docs/BUILD-PLAN.md` (DRAFT). Older text in the file is the 2026-10-13 draft + Correction blocks. |

## Resume order

1. `/NEW_PLAN.md`
2. This file — find the current in-progress Part N.
3. The in-progress Part N file — its `## Status / next step` header has the exact resume anchor.
4. Auto-memory as usual.
