# D — Build decision

> **Reading guide (2026-10-14):** everything below the DECISION block is the 2026-10-13 draft plus Correction blocks. Where the draft says "proposed"/"not approved", read "D2 chosen". D1/D3 text was withdrawn.

> **DECISION (2026-10-14): D2 — FULL RESUMED CLONE — chosen by the user** ("wipe D1 plan. go full D2"). D1 (modified reference bundle) and D3 (hybrid) are **withdrawn**; their plan text was removed. The plan is `docs/BUILD-PLAN.md` (DRAFT, gate G1 = user approval). **Freeze status:** choosing D2 does not start coding — no source files, builds or build-PC experiments until the user approves the plan. The clone mirrors the official v1.11.x driver's design and deviates only where 10.13.6 forces it. History: the D1 option was approved earlier the same day and reversed after the user weighed ownership/maintainability/debuggability against the cost of binary-patching a source-less kext.

> **Facts established since the 2026-10-13 draft that D2 must honour (R31–R33; they replace the draft's breaker argument):** (1) the reference panics on every attach on 10.13.6 because a rejected, never-started candidate is freed and `operator delete(NULL)` has no NULL test in 10.13.6 (`Q-SDK5-first-round-probe-reject.md`) — the clone must be NULL-safe in every `free()`/destructor, and must mirror the two-round lifecycle (first registration rejected in `probe`; helper's `MAFirmwareStart` triggers the second); (2) classic-DCL acceptance by `AppleFWOHCI_DCLProgram::init` on 10.13.6 is unverified (`Q-D1-round-two-static-review.md`, R-1; Q1b's "the 2014 kext proves it" is withdrawn); (3) `AppleFWAudio` may also claim the FW1814, so the clone ships a FW1814 go-away personality (Q8 extension); (4) signing with SIP on is untested (B3.2) and applies to the clone too; (5) the firmware protocol lives in the kext and the helper only writes `MAFirmwareStart/Halt/UpgradeImage` (H2, H12).


> **Re-run (2026-10-14, user directive) — D1/D2/D3 compared again with every corrected fact.** This block supersedes the §2 verdict table, §5, §6 and the "Proposed decision" line (kept below for history). Part D is still **PROPOSED, NOT APPROVED**; code stays frozen. Evidence: `PROVENANCE-RESOLUTIONS.md` R1–R29 (re-run itself = R30), `B3-sierra-hack-delta.md` B3.2 block, `A1-ghidra/Q-SDK-reserved-slots.md`, Q-DCL-geometry-table, Q3a, Q-UC-surface, Q15f, Q-CMP-connect, Q3b.
>
> **What changed since the 2026-10-13 draft (observations, each cited):**
> 1. **Breaker #3 (reserved-slot drift) is refuted** — IOAudio/IOFireWire vtables are slot-identical to the reference's SDK; only `IOService` LP64 slots 0–1 were consumed and the reference does not override them (Q-SDK §3).
> 2. **Breakers #1/#2 are not demonstrated as blockers** (B3.2). The shipped 1.11.3 signature is invalid on 10.13.6 (`-67007`, Sealed Resources v1 envelope), but with **SIP disabled** the unmodified kext passes `kextutil -n` incl. linkage (R26) and loads cleanly with no device (R27). Not verified: any run with SIP **on**; whether UAKEL needs a Team ID for this bundle. These two apply to **every** option (a new kext also has to be signed and approved) — they do not discriminate D1/D2/D3. The old "D2: free" cell was an unsupported claim (**reasoning, unverified** — same check as B3.2: `csrutil status` + `kextutil -n -t` with SIP on, user action in Recovery).
> 3. **The reference panics on 10.13.6 as soon as a device-backed candidate is matched**, without any user action (R29, capture 75; R22: four of six build-PC panics have the same chain `probeCandidates → m_audio_b_FWAudioDevice::free() @0x23b7 → delete(NULL [+0x110])`). The panic site is a teardown bug (two NULL deletes: `0x23b7`, `0x13ab3`). `[+0x110]` is only allocated near the end of `initHardware` (`0x3635`, Q3b §1), so the candidate was released **before `initHardware` finished** — **which step failed first is unidentified** (R29; the device's Model_ID/state at the time of capture 75 was not recorded — grep of the capture finds no `10070/10071`).
> 4. **The reference design facts the clone would copy changed**: `FW1814Device` derives from `FWRev1AudioDevice`; stream kick = TX start → 500 ms → INPUT `0x19` → 500 ms → RX start → OUTPUT `0x18` (plain 8-byte AV/C, no `0x2F`); HSCI frame is 16 bytes with bytes 7/8/9 = `MAClockSource` bits 3/4/5; **one** mixer write path (`CustomMixerWriteQuads` → `0xFFC7:0x00700000+`); `FWUserClient` derives from the live `FWKIPCUserClient`, selectors 506/507 = `WaitForTermination`/`SetInitializedProperty`; meter page `uint32 peak[52]`; TX ring 20 segments / RX 64 × 4; interrupt from the CallProc callback; `0xCF` fill is overwritten by `0x40`/`0x80` converters each callback and its device effect is a **hypothesis**.
>
> **Comparison (all cells cite the items above; "reasoning" = my inference, not a measurement):**
>
> | Criterion | D1 modified reference | D2 resumed clone | D3 hybrid |
> |---|---|---|---|
> | Signing / approval (#1, #2) | Re-sign needed (`-67007`); any plist/binary edit invalidates the CodeDirectory (B3.2) | Same: new kext also needs signing + approval (**reasoning, unverified**) | Same as D1 |
> | Reserved-slot drift (#3) | None (Q-SDK) | None | None |
> | Known panic site | Patchable: 2 NULL checks (Q-SDK §5) | Avoided by construction (NULL guards; a 10.13-forced deviation to comment) | As D1 |
> | **Unknown earlier init failure** | **Blocks D1**: binary-only; no per-step tracing without patched instrumentation (R29) | **Not eliminated** — D2 re-implements the same init sequence, so an environmental cause (device state, matching, 10.13 behaviour) could recur; but source + `os_log` localise it quickly in M-A | As D1 |
> | Parity with v1.11.x | Exact by construction **if it runs** | Depends on map completeness; still unverified: HSCI bytes 10–15, Q17.2 (0xCF effect), Q3b defaults (partial), 13 open audit flags | Exact only where reference code runs |
> | Provenance burden | Inherited | Every value cited; the map is the spec | Mixed |
> | Work | Smallest **if** the failure is small and localised; unknown otherwise | Largest: 4-engine family, DCL, UC, mixer, MIDI, panel, helper glue | **No identified insertion point** (no KPI divergence exists, Q-SDK) |
> | Panel / helper | Panel forced rewrite (Carbon 32-bit); helper reusable | Same | Same |
>
> **D3 no longer exists as a distinct option.** Its premise was "our code where 10.13 KPI genuinely diverges"; Q-SDK found no divergence in the families the reference subclasses. D3 is D1 plus, at most, one replaced function if the unknown failure localises there.
>
> **(Withdrawn 2026-10-14)** The Re-run block's recommendation, discriminator and D1 user-directive paragraphs were removed with the D1 plan; the decision is D2 (top).
>
>
>
> ~~**Process note:**~~ (resolved by the directive above) auto-memory `feedback_patch_original_kext.md` ("shipping target is the modified reference bundle") predates the 2026-10-05 pivot and conflicts with `feedback_clone_scope_full_parity_modern_ui.md` and `NEW_PLAN.md`; the user should confirm which governs when approving D.

> **Correction (2026-10-14) — OHCI interrupt.** The timestamp node does **not** raise the interrupt or fire `takeTimeStamp`; the CallProc-derived callback does (A1-ghidra/Q-OHCI-interrupt.md, `AppleFWOHCI` 5.5.9 on 10.13.6). Where this document attributes segment interrupts to the timestamp node, read 'CallProc callback'. Where `takeTimeStamp` is called from is still **unverified** (C2 §8.3).


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - **Stream kick.** The reference does **not** do OUTPUT 0x18 → 100 ms → INPUT 0x19. `ConnectAudioMIDIChannels @0x12156` runs: start TX program → `IOSleep(500)` → INPUT 0x19 → `IOSleep(500)` → start RX program → OUTPUT 0x18 (`0x121c9..0x122bb`, R1). The only `IOSleep(0x64)` is the CMP lock retry `0x24690`. The 100 ms pattern is snd-bebob `bebob_maudio.c:312-337` only.
> - **HSCI clock-source frame is 16 bytes, not 12:** `bzero(…,0x10)` @0xe284/0xe2a2, length `0x10` passed to `[+0x268]->vt[0x8b8]` @0xe441 (R3). The 12-byte layout is snd-bebob `bebob_maudio.c:183-198`. Bytes 10-15 content is **unverified**.
> - **User-client:** `FWKIPCUserClient` is the live base class of `FWUserClient`; selectors 506/507 are `WaitForTermination @0x761e` / `SetInitializedProperty @0x71f6` (vt `0x33c50` slots 0x968/0x970), **not** ProTools no-ops (R6). The meter page is `uint32_t[52]` (`GetPeakLevelCount` = 0x34 @0x1d30), written by `ReceiveControlPacket @0xe9ea`, not by a 30 Hz kext timer. `clientMemoryForType` accepts only type 1 / 0x400+n / 0x800+n (type 0 rejected). Type-1 notification port = MIDI-in.
> - **Slot names** (R2): `vt[0xce0]` = `FWEngine2::PerformSampleRateChangeCore @0x117e6` (not an AV/C commit); `vt[0xaf8]` = `FW1814Device::ResetRearrangementProcs @0xcad4` (not an HSCI emit); `vt[0xb00]` = `IOAudioEngine::beginConfigurationChange`; `vt[0xb10]` = `cancelConfigurationChange` (error path only, `0xe30b`); `vt[0xb58]` = `IOAudioEngine::setSampleRate`; `vt[0xbe0]` = `FW1814Device::SwitchDigitalSignals @0xe5dc` (not an AV/C stream kick); `vt[0xce8]` = `SetExternalSyncEnabled(bool) @0x11bc0`; `vt[0xa50]/[0xa58]` = `pauseAudioEngine/resumeAudioEngine`.
> - **Bundle naming:** values cited from `*-ioreg*.txt` captures come from the **SL v1.9** rig, not v1.11.2/v1.11.3 (version-skew rule).
> - Q15e 'exact layout at vt[+0xce0] AV/C commit' is moot: `vt[0xce0]` is not AV/C. `FWKIPCUserClient` is live (ship it). Meter-ring layout is now decoded (Q-UC §3). Bundle/version of the 'SHA-1' statement must be named per the version-skew rule — **unverified** which build.


> **Correction (2026-10-14) — `0xBA9` / `0xBE9` are not AV/C opcodes.** They are Itanium-ABI virtual member-function pointers (vtable byte offset + 1) stored in batch functors by `SetupBatchChangeMethod @ 0xe184`. `0xBA9` → FW1814 `vt[0xba8]` = `CustomMixerWriteQuads @ 0x22c7a`, a direct async block write to `0xFFC7:0x00700000 + offset`. `0xBE9` → `vt[0xbe8]` = `FW1814Device::AVCControlSetLEDStatus` (A/B-switch LEDs). There is **one** mixer write path (direct register writes, same window as FFADO/macfw/Rust). Group offsets `0x00/0x08/0x10/0x34/0x38` match FFADO `special_avdevice.h:55–69`. Full decode: `A1-ghidra/Q15f-mixer-control-plane-wire.md` Correction 2026-10-14. Any "AV/C batch opcode 0xBA9" or "two mixer paths" statement below is superseded.

> **Correction (2026-10-14) — device class chain.** `com_m_audio_FW1814Device` derives from **`FWRev1AudioDevice`**, not `FWRev2AudioDevice` (OSMetaClass superclass at `0xf473`; ctor `0xf04a` → `FWRev1AudioDevice` ctor at `0xf060`). See the A1-Q16 Correction. Every "FWRev2AudioDevice → FW1814Device" below should read "FWRev1AudioDevice → FW1814Device".

> **Correction (2026-10-14) — DCL ring shape.** Per `A1-ghidra/Q-DCL-geometry-table.md` (raw disassembly): **TX** = 4 banks × (4 one-packet primer segments + 1 large segment of `P−4` packets that carries the callback) + 4 jumps = **20 segments** (`BuildTransmittingDCLProgram @ 0x25ae4`; `P = *(this+0x4e0)`, per rate). **RX** = **64 segments × 4 packets** + 64 jumps (`BuildReceivingDCLProgram @ 0x25962`; `packetCount` immediate `$0x4` at `0x25a99`, fixed at all rates). Any "64-segment TX" statement below is wrong; RX 64-segment statements stand. Timing derived from "64 segments × ~1 ms" must be recomputed: 4 packets = 4 isoch cycles ≈ 500 µs per RX segment.

**Status (2026-10-14): D2 CHOSEN by the user; plan awaiting approval (`docs/BUILD-PLAN.md`).** Code work stays frozen until the user approves that plan.

**Decision:** **D2 — Resumed clone.** New kext + new control panel + new fwhelper, built against the 10.13 SDK (Xcode 9.4.1), mirroring the `_extracted/` v1.11.2 design exactly where the map dictates and deviating only where 10.13.6 mechanically forbids the reference approach.

**Date:** 2026-10-13 (closes Parts A + B + C from `NEW_PLAN.md`).

**Scope anchor:** `feedback_clone_scope_full_parity_modern_ui.md` — clone ships every function of the official 2014 v1.11.2 driver (routing matrix, faders, pan, mutes, solos, aux sends, headphone source, main level, direct monitor, clock-source selector, S/PDIF port, optical mode, S/MUX, AC3 passthrough, stereo-pair engines, buffer-size ladder, every supported sample rate, ADAT at every rate, MIDI, firmware upload, multi-interface). Panel UI may modernize visually (AppKit + Swift, dark mode, modern meter rendering, keyboard shortcuts, per-channel history, saved presets) but no capability may shrink.

## 1. Options reviewed

| ID | Option | One-line description |
|---|---|---|
| D1 | Modified reference bundle | Keep the 2014 Mach-O; patch plist + vtable binaries; re-sign. |
| D2 | Resumed clone | New kext + new panel + new helper, compiled against 10.13 SDK, mirrors `_extracted/` design. |
| D3 | Hybrid | 2014 Mach-O as IOKit shell; our code where 10.13 KPI genuinely diverges. |

## 2. The three cumulative 10.13 breakers (from B3)

Any option must resolve all three. These are independent, each on its own enough to kill the reference.

1. **UAKEL** (10.13.0 enforced from 10.13.4) requires Team-ID approval in `/var/db/SystemPolicyConfiguration/KextPolicy`. Reference bundle is unsigned → no Team ID → cannot be approved. Only SIP + kext-consent disabled from Recovery would sidestep it. Source: `B3-sierra-hack-delta.md` §3.1, memory `project_sierra_hack_mechanism_and_1013_breakers.md`.
2. **SHA-256 CMS v2 + Sealed Resources v2** required on 10.13.5+. v1.11.3 has SHA-1 + Sealed Resources v=1 (Developer ID — inMusic Brands QGX2722FLV, 2014-05-02). Source: `B3-sierra-hack-delta.md` §3.2.
3. **IOAudioFamily / IOFireWireFamily reserved-slot drift**. Apple consumed `_RESERVEDIOAudioEngine*` / `_RESERVEDIOFWIsochPort*` vtable slots between 10.11 and 10.13; subclass virtual overrides shift by one or more slots. First virtual call during `probe()` / `init()` on a shifted slot → NULL deref → panic. Source: `B3-sierra-hack-delta.md` §3.3, memory `feedback_reference_kext_panic_risk.md`.

> **Correction (2026-10-14) — breaker #3 is not supported.** `A1-ghidra/Q-SDK-reserved-slots.md`: no IOAudio/IOFireWire reserved slot was consumed between the reference's build SDK and 10.13. The used-slot counts are identical; only `IOService` LP64 slots 0–1 were consumed, and the reference does not override them. The observed 10.13 panics are `operator delete(NULL)` in teardown (`FWAudioDevice::free() @ 0x23b7`, build-PC logs 2026-10-01/10-05; Q-SDK §5). They come *after* an earlier, still-unidentified init/probe/start failure. **Consequence for this decision:** every statement below that D1/D3 "fail breaker #3" or that "breaker #3 has no clean solution" is unsupported. D1/D3 now face breakers #1 + #2, a patchable NULL-delete (two sites), and the unknown earlier failure. The D2-vs-D1/D3 comparison must be re-run with this before approval.

**How each option fares against the three breakers:**

| Option | UAKEL | CMS-v2 | Slot drift | Verdict |
|---|---|---|---|---|
| D1 modified reference | Needs Team-ID re-sign (feasible with user's own cert) | Needs re-sign with modern CMS (feasible) | **Hard** — binary vtable patch per shifted slot across both families; no provenance; brittle; cannot verify parity | **Fails on #3** |
| D2 clone | Free — Xcode 9.4.1 signs normally | Free — current codesign tool | Free — compile against current KPI headers; linker places correct slot layout | **All three free** |
| D3 hybrid | As D1 for reference piece | As D1 for reference piece | Still inherits slot drift wherever reference code calls into the shifted families | **Fails on #3** |

Only D2 resolves all three without binary archaeology.

> **Correction (2026-10-14, D re-run):** this table and its "Fails on #3" / "All three free" verdicts are withdrawn. #3 is refuted (Q-SDK); #1/#2 are unproven as blockers and apply equally to a new kext (B3.2); item 1 above is also wrong that the reference is "unsigned" — it carries a v1-envelope signature that is invalid on 10.13.6 (`-67007`, R26) and loaded with SIP off (R26/R27). See the Re-run block at the top.

## 3. Feature-parity constraint vs. option

Full-parity (per the scope memory) means **every** function in `_extracted/` ships. The C synthesis gives us a decoded design surface for every function. Table of what each option costs to deliver full parity:

| Function | D1 cost | D2 cost | D3 cost |
|---|---|---|---|
| 46 MA* control-plane keys (A2-P3, C5 §6) | Zero code — reference already does it; but survives slot-drift panic? Unknown | Full implementation in `FWPropertyDict::SetProperties` dispatcher | As D1 for the dispatcher; still inherits drift |
| Clock source (C2 §2) with composite bitfield | Zero — reference does it | Full HSCI envelope emission | As D1 |
| Routing matrix 22×4 + 22×2 (C4 §8) | Zero — reference does it | Full typed-property dict + 40-quadlet register map from B1 | As D1 |
| Peak meters (`clientMemoryForType(1)` shared ring, C5 §4) | Zero — reference does it | Full implementation of shared-memory meter writer | As D1 |
| 8 UC selectors (C5 §2) | Zero — reference does it | Full `externalMethod` dispatch | As D1 |
| MIDI (A3 + C5 §7) | Zero — reference does it | Full MIDI RPC + CFPlugIn | As D1 |
| ADAT S/MUX at 88.2/96/176.4/192 kHz (C2 §6, C4 §4) | Zero — reference does it (if it loads at all) | Full HSCI byte 7/8 + S/MUX bit 5 + FDF table | As D1 |
| AC3 passthrough (`cac3` fork at `FWEngine2::PerformFormatChange @ 0x124cc`, C4 §12) | Zero | Full `cac3` branch mirroring | As D1 |
| Stereo-pair engines (`MAEnableStereoPairEngines`, Q13 4-engine family, C1 §2) | Zero | Full 4-engine family (FWEngine1 + FWEngine2 + FWPairEngine + FWTimeSlaveEngine) | As D1 |
| Firmware upload (A4) | Not in reference kext — in helper, which survives D1 unchanged | Full `.bcd` chunking + verification (A4 wire protocol A4-H4/H9/H10) | Same as D2 |
| Multi-interface (several FW1814s on one bus) | Polymorphic `FWMetaNub` already supports it (A1-Q7) | Mirror the same polymorphic nub pattern | As D1 |
| Modern UI | Reference panel is Carbon (32-bit, dead on 10.13); UI must be re-written anyway | New AppKit + Swift panel + modern UX | Same as D2 |

The panel is a forced rewrite in every option (Carbon is 32-bit, 10.13 is 64-bit only). So the "zero-cost UI" column for D1/D3 is a myth — they also require a new panel.

**Net:** D1 and D3 save kext code but still require full panel + helper rewrites AND still fail breaker #3. D2 pays a kext-write cost but gets a working, signable, boot-safe stack that mirrors the reference design under the map's provenance.

> **Correction (2026-10-14, D re-run) — §3 table and Net.** "Zero code — reference already does it" in the D1/D3 columns is conditional on the reference initialising a device at all on 10.13.6, which it does not (R29). "Slot-drift panic" wording in row 1 is obsolete (Q-SDK). Row "Firmware upload": the kext does not upload firmware; the helper does (A4), unchanged in every option. The sentence "D1 and D3 … still fail breaker #3" is withdrawn.

## 4. Open questions from C that affect D (none are blockers)

> **Correction (2026-10-14, D re-run) — status of the table below.** Q15f **resolved**: one mixer path, `CustomMixerWriteQuads @0x22c7a` → `0xFFC7:0x00700000 + {0x00,0x08,0x10,0x34,0x38}`; `0xBA9` is a C++ virtual-member pointer, not an AV/C opcode (Q15f Correction). Q15e **moot**: `vt[0xce0]` = `FWEngine2::PerformSampleRateChangeCore @0x117e6`, not an AV/C commit (R2). Q3a **resolved**: plain 8-byte AV/C `00 FF 19/18 00 90 <SFC> FF FF` (Rev1); `0x2F` is Rev2-only. Q3b **partial**: `ResetToFactorySettings @0xd190`, `MAClockSource = 0x40000003`; power-on defaults of the 40 mixer quadlets still open. HSCI byte 9 **resolved** (S/MUX, bit 5). `FWKIPCUserClient` **live** (base of `FWUserClient`). Per-rate latency: captured on **SL v1.9** at six rates (capture 72, R25) — not on v1.11.x. Loopback round-trip and the v1.11.x rate-change-while-streaming behaviour remain open. New open items: **Q-SDK.5** (which init step fails first on 10.13.6), HSCI bytes 10–15, Q17.2.

The C synthesis flagged open questions. None change the D decision; all are resolvable inside D2 before the kext ships.

| Open question | Source | Resolution path in D2 | Blocks shipping? |
|---|---|---|---|
| **Q15f** — second mixer path (`CustomMixerWriteQuads @ 0x22c7a` writes to `0xFFC7:0x00700000+` directly, parallel to AV/C `0xBA9`) | B1 §3, B4 §1, C3 §4, C4 §10 | Decompile `CustomMixerWriteQuads` xrefs → identify which Panel events use direct pokes vs AV/C | No — clone can implement both; wire format is register-poke per FFADO/Rust/macfw |
| **Q15e** — exact byte layout at `vt[+0xce0]` AV/C commit on `FWEngine2` | A1-Q15c tail, C2 §4 | Decompile the vtable entry — straightforward Ghidra | No |
| **Q3a** — ExtendedPlugInfo probe in `ConnectAllPlugs` | B1 §4, C4 §2 | Follow A1-Q3 code path for AV/C `ExtendedPlugInfo` | No |
| **Q3b** — mixer power-on defaults | B1 §5, C4 §9 | Grep `initHardware @ 0x3644` for 40-quadlet writes before first `setProperties` | No |
| ~~HSCI byte 9 `clk_lock`~~ **resolved 2026-10-14 — byte 9 = S/MUX bit 5** latch semantics | B2 §5.2, C3 §11 | Decompile `SetClockSourceInternal @ 0xe25c` | No — can mirror snd-bebob's write pattern |
| **`FWKIPCUserClient` live or dead code in v1.11.2?** | C5 §3, C3 §5 | `newUserClient` xref | No — clone can compile only the live classes |
| Per-rate `SampleOffset` / `OutputSampleLatency` for 88.2/96/176.4/192 kHz | C6 §1, B5 §4 | SL rig ioreg capture at each rate | No — can measure after D2 kext loads |
| Hardware round-trip loopback at 48k/512 | C6 §11 | Loopback jig on build PC | No — measurement |
| v1.11.2 rate-change-while-streaming panic parity with v1.9 | C2 §8, memory `project_v19_rate_change_while_streaming_panics.md` | Will emerge the first time D2 kext loads and the Panel does a hot rate change | No — clone can gate client-busy regardless |

## 5. Why D2 and not D3

> **Correction (2026-10-14, D re-run) — §5 and §6 are superseded.** Their central argument ("the reserved-slot drift hits during `probe()`/`init()`", "breaker #3 has no clean solution", "three independent breakers") is refuted by Q-SDK. D3 has no identified insertion point and collapses into D1; D1 vs D2 now turns on the unlocated init failure (Re-run block, top). The text below is kept for history only.

D3 (hybrid) looks attractive on paper: "use the reference binary wherever it works, write code only where the KPI diverged." In practice:

- The reserved-slot drift (breaker #3) hits during `probe()` and `init()` — i.e. before any of our code runs. The hybrid cannot interpose there.
- Signing a hybrid still means re-signing the Mach-O (breaker #2), which means we're already in binary-patch territory.
- The time cost of correctly identifying which slots shifted + patching them + validating no slot was missed is comparable to writing the clone from the map.

D3 inherits D1's risk without D2's clean-signed-binary gain. Rejected.

## 6. Why not D1

- Three independent breakers; breaker #3 (slot drift) has no clean solution.
- Full parity cannot be verified against a binary-patched Mach-O — we have no provenance for the patched slots.
- The feature-parity constraint from `feedback_clone_scope_full_parity_modern_ui.md` makes "mostly works" unacceptable.
- The panel + helper would be rewritten anyway, so the alleged "save the kext code" win is partial.

Rejected.

## 7. D2 build topology (the plan post-decision)

Scaffolded from `docs/Proposal.md` (2026-10-09, pre-code design input) + the system map. Three bundles:

### 7.1 Kext — `openaudio-fw1814.kext`

> **Correction (2026-10-14, D re-run) — bullets below that no longer match the map** (read them with these substitutions):
> - Class chain: `FWRev2AudioDevice` → **`FWRev1AudioDevice`** → `FW1814Device` (A1 Q16 Correction, superclass at `0xf473`).
> - Isoch: the segment interrupt comes from the **CallProc callback**, not `DCLPtrTimeStamp` (Q-OHCI-interrupt); where `takeTimeStamp` is called is unverified (C2 §8.3).
> - Packet pool: `0xCF`/`0x80` is the pool fill, re-run each TX callback and then overwritten by `0x40` (audio) / `0x80` (MIDI) by the converters (Q17 Correction R10); "the clone must mirror 0xCF for PLL lock" is a **hypothesis**.
> - Plug map: counts are third-party-verified (macfw), plug-0 usage is reasoned (STATE → Plug map Correction). **Stream kick: TX start → 500 ms → INPUT `0x19` → 500 ms → RX start → OUTPUT `0x18`** (`ConnectAudioMIDIChannels @0x12156`, R1), not OUTPUT → 100 ms → INPUT.
> - Mixer: **one** path — group batch → `CustomMixerWriteQuads`; `0xBA9` is not an opcode; delete "also implement the direct register-poke path" (it *is* the path).
> - HSCI: **16-byte** frame (`0x10` @0xe441), not 12; bytes 7/8/9 = `MAClockSource` bits 3/4/5; bytes 10–15 unverified.
> - User client: `FWUserClient` derives from live `FWKIPCUserClient` (one class chain); selectors 506/507 = `WaitForTermination`/`SetInitializedProperty` (not ProTools no-ops); legacy `IOExternalMethod` table; meter page `uint32 peak[52]`, written by `ReceiveControlPacket @0xe9ea`; "second UC class only if live" → it is live.
> - `IOAudioFamily` KPI bullet: "correct `_RESERVED*` slot alignment → no probe-time panic" is withdrawn; the probe-time failure is unidentified (R29), and every `free()`/dtor path needs NULL guards (10.13-forced deviation, comment each).

- Class chain mirrors A1: `FWMetaNub` (polymorphic, matches `IOFireWireUnit` Vendor_ID `0x000D6C`, Model_ID `0x10070` bootloader / `0x10071` runtime) → `FWRev2AudioDevice` → `FW1814Device` → 4-engine family (`FWEngine1` + `FWEngine2` + `FWPairEngine` + `FWTimeSlaveEngine`). Multi-interface works for free from the polymorphic nub pattern.
- Isoch stack mirrors A1-Q1: classic `IODCLProgram`, 20-seg (4-bank) TX + 64-seg × 4-pkt RX (Q-DCL), each segment uses `AddTimestampedTransferPackets` + `DCLPtrTimeStamp` + callproc + jump. **No NuDCL** — 10.13.6 kernel vintage returns `kIOReturnNoResources` from `createLocalIsochPort` (CLAUDE.md deviation note).
- Packet pool mirrors A1-Q17: `TPacketPool` with page-aligned IOBMD, silence pre-fill `0x000000CF` (data region) + `0x00000080` (pad region). The on-wire label is `0xCF` not standard MBLA `0x40` (per the 2026-10-13 Correction block in Q17 + B2 cross-ref).
- CMP + plug map mirrors A1-Q3: 2 oPCRs + 3 iPCRs, plug 0 = PCM. Stream-kick post-CMP: OUTPUT 0x18 → wait 100 ms → INPUT 0x19 to FCP command register (memory `project_fw1814_stream_kick.md`, confirmed in B2 cross-ref against `special_set_rate`).
- Mixer control plane mirrors A1-Q15: typed-property dict (`FWPropertyDict`), `ChannelGroupProperty` entries, delta batch via `TFunctor` on `FW1814Device +0x38`, vendor opcode `0xBA9`. Single generic dispatcher at `FWPropertyDict::SetProperties`. **Also implement** the direct register-poke path (A1-Q15f gap) at `0xFFC7:0x00700000+` — both paths target the same register window; implement both wire-protocol primitives, let the dispatcher decide per-key (resolution during D2 implementation).
- HSCI envelope (C3 §2): 12-byte CONTROL to FCP command register, company_id `0x040004`, 7-rate FDF table from A1-Q2.
- `MakeAppleFWAudioGoAway` — same mechanism as A1-Q8 but **extended to cover FW1814** (reference only covers "FW Solo"; our Info.plist adds `FireWire Product Name = M-Audio FireWire 1814` with score `0x7FFFFF`).
- User-client surface mirrors C5: one `com_openaudio_FW1814UserClient` with 8 selectors (500 MIDIStart, 501 MIDIStop, 502 MIDIStopAlias, 503 MIDISend, 504 DiceSet, 505 DiceGet, 506 ProToolsOpen no-op, 507 ProToolsErrorReport no-op) + `clientMemoryForType(1)` for the shared-ring meter + `IORegistryEntrySetCFProperties` dispatch via Q15b.
- Second UC class `FWKIPCUserClient` — ship it only if the live/dead-code question (C5 §3) resolves to "live".
- Notification protocol mirrors A1-Q14: 44-byte Mach message, no peak/rate-lock body types — type 0 props, type-1 port = MIDI-in (corrected 2026-10-14, A1 Q-UC-surface §4), broadcast via static `sUserClients` OSSet.
- `IOAudioFamily` KPI: 10.13 current. Clone links against modern headers → correct `_RESERVED*` slot alignment → no probe-time panic.
- Signing: Developer-ID codesign via Xcode 9.4.1, user's own Team ID.

### 7.2 Panel — `OpenAudio FireWire 1814.app`

- AppKit + Swift (Carbon is dead; this is a forced rewrite in every option).
- Talks to the kext directly via `IORegistryEntrySetCFProperties` on the device registry entry for all 46 MA* keys (B5 §10 + C5 §5). **Not** via `IOConnectCallMethod`.
- Opens `com_openaudio_FW1814UserClient` for the shared-ring meter (`clientMemoryForType(1)`) + MIDI RPC (selectors 500–503) + `IOConnectSetNotificationPort`.
- Multi-interface: lists all matched nubs; one window per device OR one tabbed window with a device picker (modern-UI decision).
- Scope per the scope memory: routing matrix (22×4 main + 22×2 aux), faders, pan, mutes, solos, aux sends, headphone source (3-valued per B2a), main level, direct monitor, clock source (composite bitfield per C2 §2), S/PDIF port, optical mode, S/MUX, AC3 passthrough, stereo-pair engines toggle, buffer-size ladder, sample-rate selector.
- Modernization touches (over reference): dark mode, resizable window, saved presets, per-channel meter history, keyboard shortcuts for mixer navigation. **No** capability shrink — every reference control exists.

### 7.3 Helper — `openaudio-fw1814-fwhelper`

- Already built and A4-verified. Survives D2 unchanged (CLAUDE.md: "the only live buildable component during the system-map phase").
- LaunchDaemon. Opens `com_openaudio_FW1814UserClient` purely for `IOConnectSetNotificationPort` (A4-H1); no `IOConnectCallMethod` imports. Receives bootloader-detected notification, uploads `fw1814.bcd` chunk-wise (A4-H4/H9/H10).

### 7.4 Prefpane — dropped

A6 shows the reference prefpane is a trampoline that launches the panel via `[NSWorkspace launchAppWithBundleIdentifier:]`. On modern macOS, prefpane bundles are deprecated. The panel ships as a standalone app; the "open panel" button goes in the panel itself and in a menu-bar helper (optional).

## 8. What ships from the system map directly

Everything below is a decoded primitive. The map is the spec.

- Vendor OUI, Model_IDs, plug map (memories).
- 7-rate FDF table (A1-Q2).
- Stream-kick choreography (memory + B2 cross-ref).
- Buffer-size ladder (memory + A1).
- HSCI envelope byte layout (A1-Q6 + C3 §2).
- MAClockSource composite bitfield (memory + C2 §2).
- 46 MA* control-plane keys (A2-P3 + C5 §6).
- 40-quadlet mixer register map (B1 `special_avdevice.h:50..134` + C4 §9).
- DCL ring geometry (TX 20-seg/4-bank, RX 64-seg × 4-pkt) (memory + A1-Q1 + C4 §3/§4).
- TPacketPool silence pre-fill `0xCF` / `0x80` (A1-Q17 corrected + C4 §6).
- Format-change chain (A1-Q15c + C2 §4).
- `AC3 'cac3'` fork (A1-Q15c + C4 §12).
- 4-engine family (A1-Q13 + C1 §2).
- Shared-ring meter protocol (B5 §11 + C5 §4 — struct layout labelled hypothesis; decode during D2).
- Per-rate latency table measured ground truth (B5 §4 + C6 §1) — only 44.1 + 48 kHz captured; capture the four high rates during D2 bringup.

## 9. What D2 implementation adds beyond the map

Only what the user explicitly approved under the scope memory + Proposal.md:

- Modern panel UX (visual only, no capability drop).
- `os_log` + `os_signpost` tracing throughout (replaces the ad-hoc printf trail in `_extracted/`).
- `core/` C++17 library hosting all transport-agnostic protocol code (AV/C, HSCI, mixer model, `.bcd` parser) so a future 10.15+ `AudioServerPlugIn` port relinks `core/` without touching protocol logic.
- XPC (replacing NSConnection DO) only if a future helper-panel RPC path is needed; not needed in D2 since the panel talks to the kext directly per B5 §10.
- Explicit multi-interface support in the panel (iterate matched nubs, one engine per nub).

## 10. Build sequencing (post-D milestones)

> **Correction (2026-10-14, D re-run):** M-A's goal is no longer "validates breaker #3 is resolved" but "**localise and avoid the unidentified init failure**" — instrument every init/probe/start step with `os_log` and load only the staged build. M-B: timestamp-node and `0xCF`-idle-lock wording per §7.1 Correction. M-C: use the reference kick order (TX start → 500 ms → INPUT `0x19` → 500 ms → RX start → OUTPUT `0x18`). M-E: single mixer path. M-F: selectors 506/507 per §7.1 Correction.

Order chosen so each milestone leaves the device in a known-good state. All milestones are gated on the previous one loading cleanly on 10.13.6 build PC per the auto-memory staging rule.

1. **M-A — Kext skeleton + probe.** `FWMetaNub` matches, `FWRev2AudioDevice` instantiates, `MakeAppleFWAudioGoAway` keeps `AppleFWAudio` away, no probe-time panic. Validates breaker #3 is actually resolved.
2. **M-B — DCL rings + continuous AM824 silence TX.** 20-seg (4-bank) TX + 64-seg × 4-pkt RX (Q-DCL), timestamp node fires `IOAudioEngine::takeTimeStamp`, idle lock works (continuous AM824 TX with `0xCF`-pre-filled packets per memory `project_fw1814_idle_lock_mechanism.md`).
3. **M-C — CMP connect + stream-kick + first streaming @ 48k.** OUTPUT 0x18 → 100 ms → INPUT 0x19. CoreAudio can play.
4. **M-D — All 7 rates.** FDF table emission + format-change chain + `ResetLatency`. Measure `SampleOffset` / `OutputSampleLatency` for the four high rates and update C6.
5. **M-E — Mixer control plane.** `FWPropertyDict::SetProperties` dispatcher + 40-quadlet register map + both wire paths (AV/C `0xBA9` + direct pokes). Resolve Q15f during M-E — the gap closes under test.
6. **M-F — User-client surface.** 8 selectors + shared-ring meter + 44-byte Mach notification. First point where the panel can meaningfully talk to the kext.
7. **M-G — Panel baseline.** AppKit + Swift panel driving all 46 MA* keys via `setProperties`. Routing matrix, faders, mutes, solos, aux, HP source, main level, direct monitor, clock-source composite dropdown, S/PDIF port, optical mode, S/MUX toggle, AC3 toggle, stereo-pair-engines toggle, buffer-size, sample-rate.
8. **M-H — MIDI.** CFPlugIn + MIDI RPC selectors 500–503.
9. **M-I — AC3 passthrough.** `cac3` fork wired.
10. **M-J — Stereo-pair engines + 4-engine family.** `MAEnableStereoPairEngines` branch in probe-time engine construction (A1-Q13).
11. **M-K — Multi-interface.** Two FW1814s on the same bus, two engines, two panel windows.
12. **M-L — Latency validation.** Hardware round-trip loopback at every rate. Pass bar: `≤` reference (C6 §9).
13. **M-M — Modernization pass.** Dark mode, saved presets, keyboard shortcuts, resizable window. UX only.

Each M-X ships a signed, kextcache-safe build. The build PC's auto-memory staging rule (`feedback_stage_kext_before_autoload.md`) applies to every milestone — stage from `build/`, `make verify`, kextutil-test, THEN `sudo make load` into `/Library/Extensions`.

## 11. Risks (post-decision, pre-code)

| Risk | Likelihood | Mitigation |
|---|---|---|
| 10.13.6 KPI differs from Xcode 9.4.1 SDK in some corner that breaks M-A probe | Low — SDK matches target | Have fallback 10.14 SDK stub on hand; if 9.4.1 misses something we're on current KPI anyway |
| Q15f wire-path ambiguity blocks M-E | Low — implement both paths and let the dispatcher table pick | Resolution during M-E implementation; cheap to defer |
| Shared-ring meter struct layout not fully decoded (C5 §4 hypothesis) | Medium | Decode during M-F from `_extracted/panel/` disassembly of the Panel's reader |
| Reference panics on hot rate-change (memory) persist in v1.11.2 | Low — Panel can gate client-busy | Gate client-busy in `setProperties` dispatcher per C2 §8 |
| ADAT S/MUX at 176.4 / 192 kHz untested on SL rig | Medium | Deferred to M-L hardware validation; manual-reference §375-376 reminds S/PDIF jacks are always mirrored |

## 12. What this unlocks

- Code work resumes **immediately** after D is approved. The map is the spec; every M-X milestone has a decoded design surface.
- `_legacy_pre_ghidra/` stays archived — informs but does not drive. The pre-Ghidra 15-selector UC surface and the `FW1814Mixer.cpp` register-poking sketch are known-wrong per C5/C4; the clone starts fresh.
- Build-PC autoload safety (memory `feedback_reference_kext_panic_risk.md`) continues to apply: never `sudo make load` before `make verify` + `kextutil`-test from `build/`.
- All non-kext work (panel scaffolding, `core/` library, MIDI plugin) can start in parallel with M-A.

## 13. Open follow-up Qs (not blockers; address during D2)

Promoted from the C open-questions list; each is a task:

- **Q15f** — second mixer write path resolution. Decompile `CustomMixerWriteQuads @ 0x22c7a` xrefs. Milestone M-E.
- **Q15e** — vt[+0xce0] AV/C commit byte layout. Decompile. Milestone M-D.
- **Q3a** — ExtendedPlugInfo probe in `ConnectAllPlugs`. Milestone M-C.
- **Q3b** — mixer power-on defaults from `initHardware @ 0x3644`. Milestone M-E.
- ~~**HSCI byte 9 `clk_lock`** semantics.~~ **Resolved 2026-10-14:** bytes 7/8/9 = `MAClockSource` bits 3/4/5 (raw `0xe3e2..0xe425`; A1-Q5 Correction).
- **FWKIPCUserClient live/dead** — `newUserClient` xref. Milestone M-F.
- **Per-rate latency sweep** 88.2/96/176.4/192 kHz. Milestone M-D.
- **Loopback round-trip** at every rate. Milestone M-L.

---

**D2 chosen by the user (2026-10-14).** The comparison in the Re-run block stays as history; the working plan is `docs/BUILD-PLAN.md`.
