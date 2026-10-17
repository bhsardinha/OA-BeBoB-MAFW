# BUILD-PLAN — D2: full clone, written from scratch from the system map

**Status: APPROVED — gate G1 passed by the user on 2026-10-14 ("i approve the plan, start building"). Code resumed; progress is tracked in §8 and in the `Build` tab of `/PROVENANCE-MATRIX.xlsx`.** Decision record: the user chose **D2 (full resumed clone)** on 2026-10-14 ("wipe D1 plan. go full D2"); D1 (modified reference bundle) and D3 are withdrawn. (Before G1: no source files, no builds, no build-PC experiments — lifted 2026-10-14.)

**Relationship to other docs.** This file is the *plan* (order, gates, acceptance, risks). `docs/BUILD-MANUAL.md` holds per-milestone work orders (input docs, output files, pseudocode); it was written before the 2026-10-14 findings, so §1 below lists the corrections that **override** it. `docs/system-map/` is the spec; every value in the code cites it (provenance rule).

**What "from scratch" means.** All pre-pivot code was removed on 2026-10-14 (recover only via `git show d1-approved-2026-10-14:<path>`). `openaudio-fw1814/` is empty. The clone mirrors the official v1.11.x driver's design — class chain, DCL geometry, wire protocol, property surface — and deviates only where 10.13.6 mechanically forces it (each deviation commented with *what* forced it). The reference kext in `_extracted/` is never loaded on the build PC; on the SL rig (v1.9) it is the **behavioural oracle** (captures 62–75 in `B5-sl-dtrace-truth/captures/`).

## 0. Execution rules added after G1 (user directives, 2026-10-15)

1. **Full stack.** The product is kext + LaunchDaemon helper + control-panel .app + CoreMIDI plug-in + installer; every part is required and is built in the order of `docs/flowchart driver.drawio` (memory `feedback_full_stack_irreducible_complexity`).
2. **Airliner rule.** The driver works only with every part installed and working, so **no partial hardware tests** and no per-milestone loads of unfinished builds. Verification = conformance of the construction to this plan and to the system-map (cited values, plan-headed skeleton filled one file at a time, clean builds, `kextutil -t -n` staging), then **one integrated hardware run** (`tests/hw-smoke/integrated.sh`, task IN.3). The per-milestone "acceptance (a)/(b)" runs of §3 therefore apply only to M-A/M-FW, which were run before the rule. Hardware-touching steps stay user-owned (power-cycle, no hot-plug).
3. **Dev local.** All edits on the dev Mac; `openaudio-fw1814/scripts/sync-to-buildpc.sh [--build]` copies to the build PC; bare remote `buildpc` exists; nothing is edited on the build PC.
4. **No flash writes (scope cut).** The driver never writes the device's flash: H.2 and H.5 cancelled; M-FW reduced to `MAFirmwareInfo`, `MAFirmwareStart` (incl. the bootloader `Go`) and the 2500 ms re-register. The reference update protocol stays documented in `Q-P3` only.
5. **Skeleton + sheet.** `openaudio-fw1814/` holds a plan-headed stub for every remaining task (kext classes, helper Gating/Settings, panel sources, MIDI plug-in, installer, tests); fill them one by one in sheet order; `scripts/plan-status.py` updates the `Build` tab.

## 1. Corrections that override BUILD-MANUAL / D-decision §7 (apply when writing code)

| Topic | Use this | Source |
|---|---|---|
| Parent of `FW1814Device` | `FWRev1AudioDevice` → `FWAVCAudioDevice` → `FWAudioDevice` → `FWDiscreteDevice` → `IOAudioDevice` (not Rev2) | A1 Q16 Correction; `Q-SDK5` (probe override lives in `FWAVCAudioDevice`) |
| Stream format AV/C | plain 8-byte CONTROL `00 FF 19 00 90 <SFC> FF FF` (INPUT) and `00 FF 18 …` (OUTPUT); `0x2F` is Rev2-only | Q3a Correction |
| Stream-start order | (hypothesis: the "program start" slots are `Connect`, see Q-MB §7) TX program start → 500 ms → INPUT `0x19` → 500 ms → RX program start → OUTPUT `0x18` (`ConnectAudioMIDIChannels @0x12156`, asm `0x121c9..0x122bb`); **not** OUTPUT→100 ms→INPUT | R1, Q3a |
| HSCI clock frame | 16-byte buffer (`0x10` @0xe441), bytes 7/8/9 = `MAClockSource` bits 3/4/5; bytes 10–15 unverified | R3, Q5 |
| Mixer write path | **one**: group batch → `CustomMixerWriteQuads` → async block write `0xFFC7:0x00700000 + {0x00,0x08,0x10,0x34,0x38}`; `0xBA9/0xBE9` are virtual-member pointers, not opcodes | Q15f Correction |
| DCL geometry | TX 20 segments (4 banks × (4 one-packet + 1 big) + 4 jumps); RX 64 segments × 4 packets + 64 jumps | Q-DCL-geometry-table |
| TX segment layout | `AddSetTagSyncBits(1,0)` on the **first primer of every bank** (not the last segment); each bank holds `P` packets but only the last `n` are sent per lap (jump re-targeted past ≤ 4 primers); DCL transfer *k* ↔ pool packet `(k+1) mod N`; `takeTimeStamp` from the bank-3 TX callback (lap ≥ 3) | `Q-MB-dcl-callbacks.md` (R41) |
| Segment interrupt | from the **CallProc** callback; the timestamp node does not raise it; where `takeTimeStamp` is called is unverified | Q-OHCI-interrupt |
| Packet fill | pool fill `0xCF`/`0x80`, refilled each TX callback, then converters write `0x40` (audio) / `0x80` (MIDI); effect of `0xCF` on the device = hypothesis | Q17 Correction R10 |
| User client | `FWUserClient` derives from live `FWKIPCUserClient`; legacy `IOExternalMethod` table, selectors 500–507 (506/507 = `WaitForTermination`/`SetInitializedProperty`); meter page `uint32 peak[52]`; notification type 0 = properties, type 1 = MIDI-in | Q-UC-surface |
| Control plane | `IORegistryEntrySetCFProperties` of 46 `MA*` keys on the device entry, not `IOConnectCallMethod` | B5 §10, Q15b |
| Plug map | counts third-party-verified; plug 0 in both directions is reasoned, not proven | STATE Plug-map Correction |
| **Lifecycle (new, R31)** | `FWMetaNub::start` registers the nub; the **first** matching round is *always* rejected by `FWAVCAudioDevice::probe` (`[+0xf3]==0`); only the helper's `MAFirmwareStart` property → `FirmwareStart` sets it and registers again; the product class then starts. Mirror this (it is the reference's firmware-state design) **and** make every `free()`/destructor NULL-safe — 10.13.6 `operator delete` has no NULL test; a rejected, never-started candidate is freed on every attach | `Q-SDK5-first-round-probe-reject.md` |
| Firmware path | physical BeBoB bootloader protocol lives in the **kext** (`FWMetaNub::FirmwareUpdate*`, `FirmwareStart` boot cue `0xC8021000`); the helper only writes `MAFirmwareStart/Halt/UpgradeImage` and reads `MAFirmwareInfo` (80 B) | A4 H2, H12 |
| Classic DCL | the clone also uses classic DCL (`createLocalIsochPort(talking, head, NULL,0,0,0)`); whether 10.13.6's `AppleFWOHCI_DCLProgram::init` accepts the chain is **unverified** — Q1b's "the 2014 kext proves it" is withdrawn | `Q-D1-round-two-static-review.md` S9, R-1 |

## 2. Pre-work (static, dev Mac, no hardware) — authorised now

| # | Task | Acceptance |
|---|---|---|
| P1 | Read `IODCLProgram::generateBufferMap` (10.13.6 `IOFireWireFamily.bin`) and decide what the clone's DCL chain must look like to be accepted (R-1) | **DONE 2026-10-14 (R36)**: `Q-P1P2-dcl-acceptance-and-pool-memory.md` §2 — six constraints (buffers + timestamp slots in one allocation, non-NULL, 32-bit DMA, order of operations); run-time acceptance verified at M-B |
| P2 | `IOBufferMemoryDescriptor::inTaskWithPhysicalMask` semantics on 10.13.6 vs the pool parameters in Q17 (R-2) | **DONE 2026-10-14 (R36)**: `Q-P1P2-…` §3 — mask `bus->getFireWirePhysicalBufferMask() & ~PAGE_SIZE+1`, options 0, KMA_LOMEM (non-contiguous), sizes 0.1–0.7 MB; run-time success verified at M-B |
| P3 | Document the kext-side firmware path: `FirmwareUpdate*`, `DownloadSectionStart/SendNextBlock/Stop`, `FirmwareHalt`, `MAFirmwareInfo` 80-byte layout | **DONE 2026-10-14 (R37)**: `A1-ghidra/Q-P3-kext-firmware-path.md` — command words, registers, 512-byte block loop, dispatch on `bcd[0x34]`, 80-byte info blob checked against a live SL value and FFADO |
| P4 | Finish Q-CMP-CONNECT per-rate parameters (bandwidth, channel, speed, packetSize per rate) and Q3b mixer defaults (partial) | **DONE 2026-10-14 (R38)**: `A1-ghidra/Q-P4-cmp-per-rate-and-mixer-power-on.md` — per-rate/per-mode table verified against captures 72/74 and `MABytesPerCycle`; init sequence and default wire words. Four wire items remain open and labelled (pan order, `0x98` image, two AV/C frames): resolve at M-E |
| P5 | Consolidate C1–C6 bodies (old text still sits under Correction blocks) so the C docs read as one consistent spec | **DONE 2026-10-14 (R35)**: each C doc consistent, change logs at the end, originals in git (`a9cb63c`); the 7 open audit flags need hardware and stay labelled unverified |
| P6 | Define the **verification oracle**: which SL captures/DTrace probes give the bit-exact reference for CIP headers, AM824 labels, FCP frames, property writes, meter page | harness spec in `tests/` plan; captures listed per milestone. **DONE 2026-10-14 (R39): `system-map/P6-verification-oracle.md`** |

## 3. Milestones (kext track; each gated on the previous loading cleanly on 10.13.6)

Order follows D-decision §10 with the corrections above. Each milestone: work order in `BUILD-MANUAL.md` §3 (apply §1 overrides) → build on the build PC → stage in `/tmp` → `kextutil -n -t` → load only on green → acceptance test → `close-item`.

| M | Scope | Acceptance (pass criterion) |
|---|---|---|
| **M-A** | Skeleton: Info.plist (Q-INFO: `FWMetaNub` on `IOFireWireDevice`, product personality on the nub, FW1814 go-away personalities), `FWMetaNub` (6-state machine, Q7), `FWAVCAudioDevice::probe` with the reference's two-round logic, **NULL-safe `free()`/dtors**, `os_log` tracing | (a) loads from `/tmp` with the interface off, clean unload; (b) interface on (power-cycle, no hot-plug) → **no panic in 5 min**; round 1 rejected quietly; after helper's `MAFirmwareStart` the product class instantiates; `AppleFWAudio` not bound |
| **M-FW** | Kext firmware path (P3) + `MAFirmwareInfo/Start/Halt/UpgradeImage` properties; minimal helper that mirrors `MAFireWireLoader` (A4 H2/H11) | bootloader→runtime transition via properties only; update path tested last, user present |
| **M-B** | `TPacketPool`, DCL builders (TX 20 / RX 64×4), CallProc-driven callbacks, continuous AM824 TX | DCL accepted by `createLocalIsochPort` (non-NULL); TX packet geometry matches capture 74 (424-byte packets, DBS 13) |
| **M-C** | CMP (IRM channel, oPCR/iPCR lock, Q-CMP-connect), AV/C stream format + the reference start order | device locks; isoch in both directions; stable 5 min @48 kHz/512 |
| **M-D** | All 6 rates (44.1–192 kHz), FDF/SYT tables (Q2, Q15e), format-change chain, latency recompute | each rate streams; `SampleOffset`/`OutputSampleLatency` compared with capture 72 |
| **M-E** | Mixer: `FWPropertyDict` dispatcher, `CustomMixerWriteQuads`, factory reset (`MAClockSource=0x40000003`), HSCI clock frame | register writes byte-identical to SL captures for the same property writes |
| **M-F** | User client: selectors 500–507, meter page `peak[52]`, notification type 0/1, `clientMemoryForType` | panel stub reads live meters; MIDI selectors respond |
| **M-H** | MIDI (CFPlugIn: reuse reference plug-in if it loads on 10.13 — **unverified** — else rewrite from A3) | loopback via DIN ports; SysEx round-trip; sleep/wake |
| **M-I** | AC3 passthrough (`cac3` fork) | AC3 stream reaches S/PDIF (hardware check) |
| **M-J** | 4-engine family + `MAEnableStereoPairEngines` | engine counts match reference (SL ioreg) |
| **M-K** | Multi-interface | two units, two engines, independent (needs a second unit) |
| **M-L** | Latency validation at every rate | ≤ reference (C6 §9), measured on a loopback jig |
| **M-M** | Modernisation (UX only) | no capability regressions |

## 4. Parallel tracks

- **Helper (from scratch):** M-FW minimal version early (needed for M-A step "second round"), full version (update dispatch, sleep/login gating, status UI hand-off, settings replay H11) later.
- **Panel (from scratch, AppKit + Swift on Xcode 9.4.1):** starts after M-F; full parity per the scope memory (every reference control and all 46 `MA*` keys; UI may modernise). **Wireframe needs the user's sign-off before work starts.**
- **Installer:** last; `.pkg` with postinstall `kextutil -n -t` gate; Developer ID signing; kext-approval (UAKEL) instructions. **SIP-on signing behaviour (B3.2) must be known before the installer is promised** — applies to the clone as well.

## 5. Risks (D2-specific, ranked) and mitigations

| ID | Risk | Mitigation |
|---|---|---|
| D2-1 | Reimplementation error: the device only streams when sync, comm and control are all exactly right (the pre-pivot clone failed at TX silence) | strictly mirror; **bit-exact comparison against SL captures** at every milestone (P6); no value without a citation |
| D2-2 | Classic-DCL acceptance / physical memory on 10.13.6 (R-1, R-2) | P1/P2 before M-B; M-B acceptance requires non-NULL port |
| D2-3 | Map holes: HSCI bytes 10–15, Q3b defaults (partial), 0xCF effect, timings | label in code; resolve by SL capture before the milestone that needs it; do not guess |
| D2-4 | Signing with SIP on (R-3) | user check in Recovery before the installer |
| D2-5 | `AppleFWAudio` competition (R-4) | clone ships the FW1814 go-away personality (Q8 extension); verify at M-A |
| D2-6 | Scope: kernel C++ restrictions (no exceptions/RTTI/STL) and breadth (4 engines, UC, mixer, MIDI) | follow BUILD-MANUAL layout; milestones small and individually shippable |

## 6. Gates and what the user decides

- **G1 — approve this plan** (also: panel wireframe sign-off; who holds the Developer ID; whether to build the optional shared `core/` library from `docs/Proposal.md` — default **no**, mirror-only).
- **G2 — user present** for first hardware loads (M-A T3 and each later power-cycle test); no hot-plug ever.
- Authorised now: P1–P6 only.

## 7. Tracking

Done 2026-10-14: `PROVENANCE-MATRIX.xlsx` has a `Build` tab with one row per task above (38 tasks: P1–P6, M-A…M-M, H.1–H.6, PN.1–PN.7, IN.1–IN.6) and an acceptance test each; the Painel roadmap is split into a research block (rows 22–28) and a build block (rows 30–34), with separate research/build progress bars. The old single-line E1–E4/V/R rows are gone. `close-item` updates each task's status (see the skill, step 3b).


## 8. Progress log (2026-10-15)

| Task | State | Evidence |
|---|---|---|
| G1 | **passed 2026-10-14** | user message; sheet statuses moved from "Aguarda aprovação" to "Não iniciado" |
| M-A | **done** — loads/unloads clean, round 1 rejected, round 2 `FW1814Device` shell, no panic, survives install + reboot; go-away personalities untested (no AV/C sub-unit appears) | `build-pc-runs/2026-10-15-M-A-M-FW-install.md`, R40 |
| M-FW | **done (reduced scope, no flash)** — Info/Start/`Go`/2500 ms re-register verified; Halt->bootloader->back verified before the cut | same, R40, R42 |
| H.1 | **done** — LaunchDaemon helper writes `MAFirmwareStart` at boot and on attach | same |
| H.2, H.5 | **cancelled** (flash cut) | R42 |
| H.6 | in progress — plist + `install.sh`/`uninstall.sh`; unload not yet verified | sheet |
| M-B | **in progress** — decoded (`Q-MB`, R41); written: `FWSharedSync`, `FWCip`, `TPacketPool` (+index), `FWDCLBuilder` (+`ModifyJumpLabel`), `FWDCLProgram.h`; **`FWDCLProgram.cpp` still the old part-1 version, so the kext tree does not build until it is ported**; not loaded on hardware | `Q-MB-dcl-callbacks.md` §9 |
| M-C … M-M, H.3-H.4, PN.*, IN.* | skeleton created, not started | `openaudio-fw1814/**` stubs, sheet notes |

Next, in order (flowchart): finish M-B (`FWDCLProgram.cpp`, sample-ring hooks), then M-C (decompile the start slots first, Q-START), M-D, M-E, M-F, then M-J/M-H/M-I, H.3/H.4, panel after the PN.1 wireframe sign-off, installer, integrated run.
