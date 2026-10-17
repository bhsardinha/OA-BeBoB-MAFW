# PROVENANCE-AUDIT — docs/system-map provenance sweep (2026-10-14)

Read-only audit of every `.md` under `docs/system-map/`. Each claim was checked against the raw disassembly (`otool-exports/mafw.asm`), the slice bytes, vtables resolved from `__DATA,__const` plus `otool -rv`, the decompile logs, the B5 captures and `_refs/`. **No system-map doc was edited by this audit.** The fixes are listed below; each needs a dated `**Correction (2026-10-14):**` block in the affected doc.

## Resume (summary)

**Totals:** 129 flags.

| Category | Meaning | Count |
|---|---|---|
| 1 | Inference stated as fact | 26 |
| 2 | Third-party-only fact | 9 |
| 3 | Decompile-only on a sensitive value | 2 |
| 4 | Citation does not hold, or the binary contradicts the claim | 85 |
| 5 | Number with no source | 7 |

41 claims were spot-checked against the binary: 25 verified OK, 16 distinct mismatches. Most high-severity rows are **stale propagation**. Today's corrections in Q3a §3, Q-CMP and Q-UC-surface were never carried into C1–C6, D, Q15c, Q4, Q5 or Q6.

**Top 10:**
1. **Stream kick.** "OUTPUT 0x18 → 100 ms → INPUT 0x19" is wrong (C1/C2/C3/C4/C6/D).
   - The kext's only 100 ms sleep is the CMP lock retry (`0x24690`).
   - Reference order: TX start → 500 ms → INPUT 0x19 → 500 ms → RX start → OUTPUT 0x18 (`0x121c9..0x122bb`).
2. **`vt[0xce0]` is not an AV/C rate commit.** It is `FWEngine2::PerformSampleRateChangeCore @0x117e6`.
3. **`vt[0xaf8]` is not an HSCI emit.** It is `FW1814Device::ResetRearrangementProcs @0xcad4`.
4. **Format-change barriers are misnamed.**
   - `vt[0xb00]` = `IOAudioEngine::beginConfigurationChange`.
   - `vt[0xb10]` = `cancelConfigurationChange`, reached on the error path only.
   - `vt[0xb58]` = `IOAudioEngine::setSampleRate`.
5. **The HSCI clock-source frame is 16 bytes, not 12** (length `0x10` @0xe441). The 12-byte layout comes from snd-bebob.
6. **C5's user-client section is stale.** `FWKIPCUserClient` is the live base class. Selector 506 = `WaitForTermination` and 507 = `SetInitializedProperty`; they are not ProTools no-ops.
7. **C2's rate table has wrong columns.** The FDF argument is an enum 0..6. The value 8/16/32 is the SYT interval, not a channel count. Real channel counts come from the kext's static `DeviceChannels` tables.
8. **The CIP header-1 formula is wrong.** Correct: `(node&0x3f)<<24 | DBS<<16`, where DBS = audio+MIDI quadlets.
9. **Q4's ReconnectP2P decode is wrong.**
   - `vt[0xcb8]` = `ConnectAllPlugs`.
   - The tail condition is inverted.
   - `0xBF1` = a virtual pointer to `ReconnectP2P`, so BusReset runs ReconnectP2P on a new thread.
   - The tail resends the clock source over HSCI; it is not an AV/C kick.
10. **Two more misreads.**
    - `vt[0xbe0]` = `SwitchDigitalSignals`, not an AV/C stream kick.
    - C1:201 is wrong: `FirmwareHalt` skips the write on a bootloader-mode device (`0x1c007`).

Also covered in the full report:
- meter page = `uint32_t[52]`, written by `ReceiveControlPacket`
- HSCI completion runs on a new thread
- the OHCI-interrupt mechanism is inference
- DCL label/SetTagSyncBits opcodes swapped (label = 9, SetTagSync = 11)
- C6 cites v1.11.3 for values captured on v1.9

---

**Reading the full report.** In the table below, columns C1–C5 are the audit categories 1–5, not the C1–C5 docs. Severity is high when a claim would change clone wire bytes, timing, ordering, sizes or class layout. Claims already superseded by a top-of-file `Correction (2026-10-14)` block were not flagged. Those blocks cover 0xBA9/0xBE9, Rev1 vs Rev2, the 20-segment TX ring and breaker #3.

## Summary table (flags per file by category)

| File | C1 | C2 | C3 | C4 | C5 | Total |
|---|---|---|---|---|---|---|
| C1-lifecycle-state-machine.md | 3 | 0 | 0 | 9 | 0 | 12 |
| C2-sync-path.md | 3 | 1 | 1 | 7 | 0 | 12 |
| C3-comm-path.md | 2 | 1 | 0 | 9 | 2 | 14 |
| C4-transport-control-path.md | 5 | 2 | 0 | 9 | 1 | 17 |
| C5-userclient-surface.md | 0 | 0 | 0 | 8 | 0 | 8 |
| C6-latency-budget.md | 2 | 0 | 0 | 6 | 3 | 11 |
| D-decision.md | 1 | 0 | 0 | 7 | 0 | 8 |
| A1-ghidra/Q15c-changerate-performformat.md | 0 | 0 | 0 | 8 | 0 | 8 |
| A1-ghidra/Q4-reconnectp2p-busreset.md | 1 | 0 | 0 | 5 | 0 | 6 |
| A1-ghidra/Q5-clock-source.md | 0 | 0 | 0 | 3 | 0 | 3 |
| A1-ghidra/Q6-fwhsci.md | 0 | 0 | 1 | 3 | 0 | 4 |
| A1-ghidra/Q1 / Q1a / Q1b | 1 | 0 | 0 | 2 | 0 | 3 |
| A1-ghidra/Q7, Q16, Q17, README, A1-reference-kext | 4 | 0 | 0 | 3 | 0 | 7 |
| B1 / B2 / B4 / B5 | 2 | 4 | 0 | 2 | 0 | 8 |
| A2 / A3 / A4 / A5 / A6 / system-map README (sample) | 2 | 1 | 0 | 4 | 1 | 8 |
| **Total** | **26** | **9** | **2** | **85** | **7** | **129** |

Category 4 spot checks: 41 claims opened against the asm, slice bytes, decompile log or capture. 25 were verified OK (list at the end). 16 distinct mismatches were found and are listed as category-4 rows. Most of them propagate into several docs.

---

## Top 15 highest-severity flags

1. **Stream-kick "OUTPUT 0x18 → 100 ms → INPUT 0x19" is stated as reference behaviour, but the binary disagrees.**
   - Where: C1:85,179,208; C2:268-291,325; C3:55,267; C4:505-528,548; C6:106,115; D:112,146,176.
   - Binary: the kext's only `IOSleep(0x64)` is at `0x24690` in `FWAVCConnectionManager::updateP2PCount`. That is the CMP lock retry, up to 4 tries.
   - The real reference sequence is in `ConnectAudioMIDIChannels` (Q3a §3 Correction, re-verified):
     - TX start
     - `IOSleep(500)` @0x121d7
     - INPUT `0x19` (esi=1 @0x12211)
     - `IOSleep(500)`
     - RX start
     - OUTPUT `0x18` (esi=0 @0x122ae)
   - The "100 ms between" pattern exists only in snd-bebob `bebob_maudio.c:312-337`.
2. **`vt[0xce0]` is not an "AV/C PLUG SIGNAL FORMAT commit".**
   - Where: Q15c:33,55,118,155,165; C1:78,207; C2:49,196,204,287; C3:267; C4:234,245,528; C6:105,213; D:75,212.
   - Binary: FWEngine2 vtable (0x391c0) slot 0xce0 = `FWEngine2::PerformSampleRateChangeCore @0x117e6`. It calls `DisconnectAllPlugs` if connected, then `FWBaseEngine::PerformSampleRateChangeCore @0x479e`. C2 lists the same call twice, as stage 12 and stage 15.
3. **`vt[0xaf8]` is not an "HSCI envelope emit".**
   - Where: Q15c:52,151,166; C2:50,195,204; C4:243; C6:107.
   - Binary: FW1814Device vtable (0x369b0) slot 0xaf8 = `FW1814Device::ResetRearrangementProcs(FWBaseEngine*) @0xcad4`, called on `[+0x138]` with rsi = engine (asm 0x435b).
4. **The SetIOFormat barriers are misnamed.**
   - Where: Q15c:47,59,152-154; C2:193,197-198; C4:15,30,236,241,244,247,251; C6:104,110.
   - Binary, on the FWEngine2 vtable:

     | Slot | Docs say | Resolves to |
     |---|---|---|
     | `vt[0xb00]` | StopStreams / BeginFormatChange | `IOAudioEngine::beginConfigurationChange` |
     | `vt[0xb10]` | EndFormatChange / StartStreams | `IOAudioEngine::cancelConfigurationChange` |
     | `vt[0xb08]` | — | `completeConfigurationChange` |
     | `vt[0xb58]` | ApplyRateToIsoch | `IOAudioEngine::setSampleRate` |

   - `vt[0xb10]` is reached only on the `SetStreamFormat` failure path (0x43c7..0x43d4).
5. **The HSCI clock-source frame is called "12-byte" but is 16 bytes.**
   - Where: C3:22,41-53; D:114; system-map/README:47; A1-ghidra/README:19.
   - Binary: `SetClockSourceInternal` zeroes 16 bytes (`bzero 0x10` @0xe284) and passes length `0x10` (@0xe441) to `[+0x268]->vt[0x8b8]`.
   - The 12-byte layout is snd-bebob `bebob_maudio.c:183-198`. C2 already says 16, so the docs contradict each other.
6. **C5's selector and KIPC sections are superseded by Q-UC-surface, but C5 has no correction block for them.**
   - Where: C5:18-22,35-43,47,346-347,352; D:79,116-117,216; C3:184-189,310-311.
   - Binary: `FWUserClient` vtable 0x33c50 slot 0x968 = `WaitForTermination @0x761e` and slot 0x970 = `SetInitializedProperty @0x71f6`. These are selectors 506 and 507, not ProTools no-ops.
   - `FWKIPCUserClient` is the live base class: slot 0x960 = `FWKIPCUserClient::AddModifiedProperties`.
   - "506 MUST remain a no-op" is wrong.
7. **C2 §5 rate table: the "FDF value = hex rate" and "AM824 channels 8/8/8/16/16/32/32" columns are wrong.**
   - Where: C2:216-226; C6:75,166; Q15c:70-76,133-138.
   - Binary: SetupCIP's FDFValue argument is the enum 0..6 (r8d @0x27339..0x273fe). `[+0x224]` = 8/16/32 equals the SYT interval.
   - The real per-mode channel counts are kext static tables:

     | Table | Values |
     |---|---|
     | `sSPDIF44_96` @0x363c0 | 10/10/6/6 |
     | `sADAT44_48` @0x364a0 | 16/16/12/12 |
     | `sADAT88_96` @0x36580 | 12/12/8/8 |
     | `s176_192` @0x4ab80 | 2/2/4/4 |

8. **The CIP header-1 template is wrong.**
   - Where: C2:232,331.
   - Docs say `(hubID<<24)|((dbc_start<<16)&0x3F0000)`.
   - Binary (asm 0x27173..0x2718e): `((nodeID&0x3f)<<24) | (([+0x64]+[+0x68])<<16)`. That is the DBS (audio+MIDI quadlets), with no mask.
9. **The ReconnectP2P / BusReset decode is wrong in several places.**
   - Where: Q4:9,16-17,20,29,46,56; C1:179,208,210,252.
   - `vt[0xcb8]` is `ConnectAllPlugs` (not Quiesce) and `vt[0xcc0]` is `DisconnectAllPlugs`.
   - The tail condition is inverted. It calls `SetClockSourceInternal(fMode, true)` when engine `[+0x2d0]==0` (0xe22c..0xe23c).
   - The `0xBF1` value in BusReset (@0xe5ab) is a virtual PMF to `vt[0xbf0]` = `ReconnectP2P`. So BusReset runs ReconnectP2P on a new thread; the two are not separate paths.
   - The tail is an HSCI clock-source resend with blankSlate=1, which skips `vt[0xbe0]`. It is not an AV/C stream kick.
10. **`vt[0xbe0]` is not an "AV/C stream-kick entry".**
    - Where: C2:38,287; Q5:274,315; B1:85.
    - Binary: FW1814Device slot 0xbe0 = `SwitchDigitalSignals(uint) @0xe5dc`, called at 0xe4a0.
11. **FirmwareHalt does not write anything on a bootloader-mode device.**
    - Where: C1:27,201.
    - Binary: `FirmwareHalt` at 0x1c007 does `cmpb $0,0xf0; jne` and returns 0 with no write when `fBootloaderActive` is set. HALT only writes from runtime mode. C3:208 has it right.
12. **The meter page layout and writer are wrong.**
    - Where: C5:89-109; C3:27,160,174; A2-P9:212; D:157.
    - Binary: the page is `uint32_t[52]` (`GetPeakLevelCount` returns 0x34 @0x1d30), allocated by `withOptions(0x10000,0x1000,0x1000)` @0x3598.
    - It is filled by `ReceiveControlPacket @0xe9ea`: 38 hardware slots and 14 SW-return slots.
    - It is not "inCh 18 + outCh 14 + swRet", and it is not written by a 30 Hz kext timer.
13. **The OHCI interrupt mechanism is stated as fact with no citation.**
    - Where: C2:242,333; C4:133-135,478; D:110,175; Q1:177,187; CLAUDE.md.
    - "The DCLPtrTimeStamp node makes OHCI raise the interrupt" has no binary or Apple-source citation.
    - Q1:187 credits the CallProc `s` bit instead, so the docs contradict each other. Both are IOKit-semantics inference.
14. **The HSCI threading and "no async reads" claims are wrong.**
    - Where: C3:57,261; C1:250.
    - HSCI completion runs `InvokeOnNewThread(HSCISafelyComplete)` (@0x1d89e), on a new thread rather than the workloop. The re-arm is `50 + [+0xd4]` ms (@0x1d50d..0x1d513).
    - "No async reads outside the HSCI window" is contradicted by `updateP2PCount`'s PCR read (Q-CMP §3) and by `FireWireReadCommand`.
15. **DCL opcode numbers for label and SetTagSyncBits are swapped.**
    - Where: Q1a:85; Q1b:156,158; C4:102-103,143.
    - Binary: `AddLabel` emits `$0x9` (@0x6bdd) and `AddSetTagSyncBits` emits `$0xb` (@0x6d34).

---

## Per-file flags

Row format: `line | quoted claim | cat | sev | what check verifies it`

### C1-lifecycle-state-machine.md
- 27, 201 | "writes MAFirmwareHalt … FirmwareHalt @ 0x1bff2 writes RESET opcode 0x01020000 | subcode=1" (on bootloader device) | 4 | high | asm 0x1c007-0x1c00e: write skipped when `[+0xf0]` (bootloader active) ≠ 0.
- 78, 207 | "commit via vt[0xce0] + HSCI emit via vt[0xaf8]" | 4 | high | vtable 0x391c0/0x369b0: ce0 = FWEngine2::PerformSampleRateChangeCore, af8 = ResetRearrangementProcs.
- 82-85, 179, 208 | "STREAM_KICK OUTPUT 0x18 → 100 ms → INPUT 0x19 FCP" | 4 | high | Q3a §3 Correction; only IOSleep(0x64) is at 0x24690 (CMP retry).
- 140 | "DCL ring is 64-TX-segments + 64-RX-segments" | 4 | med | C1 lacks the DCL Correction block; Q-DCL: TX 20 segments.
- 145 | "40-ioreg-playback.txt:4287..4324: five passive … IOAudioEngineUserClients (coreaudiod, loginwindow, UserEventAgent…)" | 4 | low | That range holds 5 IOAudioControlUserClients (coreaudiod, SystemUIServer, Panel, System Preferences, afplay).
- 165 | "polls 0xFFC7:00600000 for fRate>>2 quadlets" | 3 | low | `[+0xc0]` at 0x1d623 is a byte length, not a rate.
- 179, 208, 210, 252 | "ReconnectP2P runs InvokeOnNewThread CallCenter selector 0xBF1" | 4 | high | 0xBF1 is set in BusReset @0xe5ab; PMF → vt[0xbf0] = ReconnectP2P; ReconnectP2P itself has no thread hop.
- 187 | "Panel and helper notification subscribers both see the message" | 1 | low | Trace FWMetaNub::message → global hook target; not shown in Q7.
- 228 | "Panel … retain 7 at SL idle" | 4 | low | 12-ioreg-full.txt:5812 shows Panel UC retain 8.
- 244 | "The off-workloop dispatch is deliberate — keeps the FireWire workloop free" | 1 | low | Design intent with no evidence.
- 250 | "broadcast … fires from this timer context" (HSCI) | 4 | med | Listener dispatch runs on InvokeOnNewThread (0x1d89e), not the timer context.
- 275, 284 | "vtable slots whose indices moved … ~20 patch sites" | 4 | med | Superseded by Q-SDK; C1 has no breaker-#3 correction block.

### C2-sync-path.md
- 38, 287 | "if !blankSlate: vt[0xbe0] AV/C stream-kick" | 4 | high | vt[0xbe0] = SwitchDigitalSignals @0xe5dc.
- 49-50, 193-198, 204 | "vt[0xce0] AV/C commit"; "vt[0xaf8] HSCI emit"; "vt[0xb58] ApplyRateToIsoch"; "vt[0xb10] EndFormatChange/StartStreams" | 4 | high | Resolved slots, see top-15 #2-#4.
- 102 | "engine vt[0xce8] = SetDigitalInputActive(false)" | 4 | low | No such symbol; FWEngine2 vt[0xce8] = SetExternalSyncEnabled(bool) @0x11bc0.
- 174, 325 | "These values are the device-measured PLL settle time" | 1 | med | The values are verified (0x12c ×2, 0x9c4); "device-measured" has no source.
- 216-226 | "FDF value* = hex of rate"; "AM824 channels 8/8/8/16/16/32/32" | 4 | high | FDF arg = enum (0x27339..); channels from DeviceChannels tables.
- 232, 331 | "cip_header1 = (hubID << 24) \| ((dbc_start << 16) & 0x3F0000)" | 4 | high | asm 0x27173-0x2718e: DBS = [+0x64]+[+0x68], unmasked.
- 232 | "0x90000000 … (AM824 tag/channel/tcode bits)" | 1 | low | This is CIP Q1 EOH/FMT; check against IEC 61883-1, not the isoch header.
- 242, 333 | "DCLPtrTimeStamp node is what makes OHCI raise an interrupt" | 1 | high | Needs IOFireWireFamily DCL→OHCI compiler source; Q1:187 says CallProc.
- 268-291, 325 | stream-kick sequence, "Clone must mirror 100 ms" | 4/2 | high | Q3a §3 Correction (500 ms, INPUT before OUTPUT).
- 283 | "100 ms gap … is empirically the device's internal plug-state settle time" | 1 | med | Labelled "synthesis", not an approved marker; no capture.
- 59 | "poll … fRate>>2" | 3 | low | Same as C1:165.
- 66 | "B5 §6c cap 50 … Studio One" | 4 | low | The file is `50-ioreg-logic-playback.txt`; values verified (48/231); confirm which DAW.

### C3-comm-path.md
- 22, 41-53 | "12-byte HSCI vendor-dependent CONTROL … 10..11 padding" | 4/2 | high | 16-byte buffer, len 0x10 @0xe441; 12-byte is snd-bebob only.
- 55, 267 | "OUTPUT … 0x18 → IOSleep(0x64) = 100 ms → INPUT … 0x19 … on-workloop" | 4 | high | No IOSleep(0x64) on this path (only 0x24690).
- 57 | "firmware is read-shy and may freeze on arbitrary async reads … no async reads outside the HSCI window" | 2/4 | med | FFADO special_avdevice.cpp:161 says only "can't respond against frequent requests"; the kext reads PCRs (0x244f0).
- 59 | "The kext checks this inside AVCCommand" | 1 | low | Find response-code compare after `[+0x268]->vt[0x8b8]`.
- 26, 131-142, 154 | bodyType table 0/1 peak/2 rate-lock; "three bodyTypes represent real kext-side events" | 4 | med | Q-UC §4: type-1 port = MIDI-in, type 2 rejected (correction only at L308).
- 27, 160, 174, 269 | "Kext writes peak-meter samples into it on a 30 Hz timer" | 4 | high | Writer = ReceiveControlPacket @0xe9ea (Q-UC §3).
- 86 | "AV/C transactions cost ~1 ms each; … ~88 ms per edit" | 5 | low | No capture.
- 172 | "Type 0 is reserved for the serialized dirty-dict handoff" | 4 | med | clientMemoryForType @0x7220 accepts only 1, 0x400+n, 0x800+n; others return 0xE00002C7.
- 184-189 | selector table incl. "506/507 … literal xor eax; ret no-ops" | 4 | med | Q-UC §2 (sMethods 0x339e0, vt 0x968/0x970).
- 194 | "timestamped at +0x010 and realtime at +0x808" | 4 | med | Contradicts C5:269 (+0x000); A3-M4:23 sets this[0x10]=ptr, so offset 0 in the mapping.
- 214, 283 | "exact 12-byte cue appears somewhere inside the kext … open" | 4 | low | Resolved: FirmwareStart @0x1befa-0x1bf34 writes 12 bytes {[+0x98], 0x01110000, 0} to 0xFFFF:C8021000.
- 261 | "isoch RX/TX callbacks fire from DCL callproc nodes — same workloop context (A1-Q1)" | 1 | med | Q1 does not show this; trace the callproc context.
- 265 | "AVCCommand blocks until the response arrives (~1 ms)" | 5 | low | No capture.

### C4-transport-control-path.md
- 15, 30, 236, 241, 247, 251 | "BeginFormatChange → stopStreams … startStreams → EndFormatChange … vt[+0xb00]/vt[+0xb10]" | 4 | high | b00 = beginConfigurationChange, b10 = cancelConfigurationChange (error path only).
- 29 | "fIsAC3=0xa5" | 4 | med | Conflicts with Q15c:162 / C2:208 (AC3 at +0x2d4); `[+0xa5]` gates beginConfigurationChange @0x42c4; meaning unverified.
- 70-76 | per-mode channel counts cited to snd-bebob only | 2 | low | Values verified OK against kext tables (0x363c0/0x364a0/0x36580/0x4ab80); cite those instead.
- 80 | "early return kIOReturnBusy if already connected" | 4 | low | asm 0x12488 returns 0xE00002BC; bandwidth failure returns 0xE00002BE.
- 85 | "snd-bebob special-firmware-notes.md:85-87" | 4 | low | That file is macfw's.
- 85-92 | "Start order — hardware-proven … Reversing this fails the second CMP connect" | 2 | med | The reference starts TX, then INPUT format, then RX, then OUTPUT format (0x121c9-0x122bb); no capture for "fails".
- 102-103, 143 | "label[i] (op 11)", "SetTagSyncBits (op 9)" | 4 | med | AddLabel $0x9 @0x6bdd; AddSetTagSyncBits $0xb @0x6d34.
- 133-135, 478 | "timestamp node is what makes OHCI raise an interrupt"; "classic DCL compilers set [s] on CallProc + the timestamp node" | 1 | high | Same as C2:242.
- 204, 209 | "the DM1000 firmware is tuned against the 0xCF idle signature" | 1 | med | B2:88 labels this a hypothesis; C4:535 also does.
- 234, 243-245 | "vt[+0xaf8] … HSCI envelope emit", "vt[+0xb58] ApplyRateToIsoch", "vt[+0xce0] … AV/C OUTPUT PLUG SIGNAL FORMAT" | 4 | high | Top-15 #2-#4.
- 312 | "each AV/C transaction is ~1 ms on this bus" | 5 | low | No capture.
- 320 | "write-only" register window | 1 | low | No source.
- 371 | "0x8000 on the R-side applies the mute-mask flag per channel per AV/C" | 1 | low | Check the AV/C Audio Subunit spec.
- 455, 464 | "ceil(N / floor(4096/S)) + 2 … +2 pages are head/tail padding"; "& ~0xFFF enforces page alignment (BeBoB DMA requirement)" | 4/1 | low | asm 0xa45c-0xa467: floor([+0x18]/[+0x28])+2; the mask is bus->vt[0x140] physical mask & ~0xFFF.
- 468 | "bzero'd → invalid AM824 label → PLL could not lock → audible clicks" | 1 | med | No capture ties the clicks to the label.
- 505-528, 548 | stream-kick section (also internally swaps INPUT 0x18 / OUTPUT 0x19 at 511-513) | 4 | high | Q3a §3 Correction.

### C5-userclient-surface.md
- 18-22, 47, 346 | "FWKIPCUserClient … dead compiled code"; "only needs to implement one UC class" | 4 | high | Q-UC §1: base class of FWUserClient (MetaClass superclass; vt slot 0x960).
- 35-43, 347 | 503/504/505 counts; "506 … MUST remain no-op … TerminationWaitThread"; "507 … xor eax; ret" | 4 | high | sMethods legacy IOExternalMethod; 506 = WaitForTermination, 507 = SetInitializedProperty.
- 54, 352 | "1 = peak (reserved, unused); 2 = rate-lock (reserved, unused)" | 4 | low | Type 1 = MIDI-in port; type 2 rejected (Q-UC §4).
- 67, 79 | dirty-dict BMD mapped "probably type=0" | 4 | med | clientMemoryForType rejects type 0 (0x7220).
- 89-109, 349 | "Kext DSP writer continuous u32 … inCh=18, outCh=14 … 3 regions" | 4 | high | uint32_t[52]: 38 hardware + 14 SW-return slots; writer ReceiveControlPacket.
- 196-197, 218, 244 | "vtable[0xba8](this, 0x90, 1, &[+0x1308])" called "Opcode byte" | 4 | low | vt[0xba8] = CustomMixerWriteQuads(offset,count,src); 0x90 is a register offset (no correction block in C5).
- 269 | "timestamped at offset +0x000" | 4 | med | Inconsistent with C3:194; kext side is an engine descriptor ([+0x2a8], Q-UC §3) with unverified layout.
- 92 | "LPeriodical period … ≈ 33.3 ms" | — | — | Panel-side, cited; OK.

### C6-latency-budget.md
- 11, 122 | "the shipping v1.11.3 reference publishes 48 … 231 … 32" | 4 | med | The captures are SL v1.9 (version-skew memory); values verified in 12/40/50-ioreg.
- 11, 31 | "32-frame input latency … matches the one-frame-of-host-latency semantics of Q9" | 1 | med | Partly labelled at :31 but stated as fact at :11.
- 42, 45, 52, 57 | "~85 us … measured on comparable kexts", "~20 us", "~5-15 us", "≈616 us" | 5 | med | No capture; label as estimates.
- 57 | "This matches Q10's 225-frame round-trip" | 1 | low | Arithmetic coincidence, not shown.
- 75, 166 | "per-segment slot count scales … 8/8/8/16/16/32/32 channel-count" | 4 | med | That value is the SYT interval; channel tables differ.
- 81 | "the workaround is always on for a FW1814" | 4 | med | At 176.4/192 kHz the input is 2 ch (s176_192 table), so `[+0x2e4]==2` skips it (Q9:26).
- 104-110 | "BeginFormatChange / StopStreams … ≥2 isoch cycles"; "EndFormatChange / StartStreams" | 4 | high | Slot misnames (top-15 #4).
- 105 | "~5-10 ms FCP … (B5 §5 FCP captures: typical response within one 125-us cycle)" | 5 | med | Self-contradictory; no B5 FCP timing capture.
- 106 | "Stream-kick wait 100 ms — A1-Q16 §SyncStream" | 4 | high | Q16 has no 100 ms; SyncStream is Rev2-only.
- 107 | "HSCI envelope emit via vt[+0xaf8] … FWHSCI::SetClockSource" | 4 | high | No such symbol; af8 = ResetRearrangementProcs.
- 118 | "N=4 observed TX+RX+idle-lock+?" | 5 | low | No source.
- 213 | IOAudioFamily vtable-drift risk paragraph | 4 | med | Superseded by Q-SDK; no correction block in C6.

### D-decision.md
- 75, 212 | "Q15e — exact byte layout at vt[+0xce0] AV/C commit" | 4 | med | ce0 is not AV/C.
- 79, 117, 216 | "FWKIPCUserClient live or dead … ship it only if live" | 4 | med | Resolved: live base class.
- 112, 146, 176 | "Stream-kick post-CMP: OUTPUT 0x18 → wait 100 ms → INPUT 0x19" | 4 | high | Q3a §3 Correction.
- 114 | "HSCI envelope: 12-byte CONTROL" | 4 | high | 16 bytes.
- 116 | "506 ProToolsOpen no-op, 507 ProToolsErrorReport no-op" | 4 | high | Q-UC §2.
- 157 | "Shared-ring meter … struct layout labelled hypothesis" | 4 | low | Now decoded (Q-UC §3).
- 175 | "timestamp node fires IOAudioEngine::takeTimeStamp" | 1 | med | C2 §8.3 lists the takeTimeStamp site as open.
- 30 | "v1.11.3 has SHA-1…" while the doc's target is v1.11.2 | 4 | low | Name the bundle and version per the version-skew rule.

### A1-ghidra/Q15c-changerate-performformat.md (no correction blocks; root source of many C-doc errors)
- 26, 35, 147-148 | "vt[0xa50] Acquire/Lock", "vt[0xa58] Release/Unlock" | 4 | med | FWBaseEngine::pauseAudioEngine @0x3d54 / resumeAudioEngine @0x3dae.
- 33, 55, 118, 155, 165 | "vt[0xce0] … AV/C PLUG SIGNAL FORMAT write" | 4 | high | FWEngine2::PerformSampleRateChangeCore.
- 46 vs 162 | "fIsAC3 = this->field_0xa5" vs "[+0x2d4] = AC3-mode bool" | 4 | med | Contradictory.
- 47, 59, 152-153 | b00 "BeginFormatChange / StopStreams", b10 "EndFormatChange / StartStreams" | 4 | high | IOAudioEngine begin/cancelConfigurationChange.
- 52, 151, 166 | "vt[0xaf8] … HSCI envelope emit" | 4 | high | ResetRearrangementProcs.
- 54, 154 | "vt[0xb58] ApplyRateToIsoch" | 4 | high | IOAudioEngine::setSampleRate.
- 70-76, 135, 138, 162 | "fChannelsPerRate@0x224 = 8/8/8/16/16/32/32 … channel count" | 4 | high | The value is verified (0x47f2..0x48ad) but it is the SYT interval, not channels.
- 133 | "FDF 0x7D00 …" | 4 | low | The FDF is enum 0..6 (GetRoundTripLatencyForFDF switches on 0..6 at 0xcf34).

### A1-ghidra/Q4-reconnectp2p-busreset.md (no correction blocks)
- 9 | "ReconnectP2P is driven from user space … BusReset … different code paths" | 4 | high | BusReset PMF 0xBF1 → vt[0xbf0] = ReconnectP2P.
- 16-17 | "vtable[0xcc0] DisconnectAllPlugs; vtable[0xcb8] Quiesce / StopDCL" | 4 | high | cb8 = ConnectAllPlugs.
- 20 | "if (!this->fEngine \|\| !…fRunning [0x2d0]) return" | 4 | high | Inverted: it calls when [+0x2d0]==0 (0xe22c-0xe233).
- 29 | "1100 ms … PLL-lock recovery window … tail-call is the AV/C stream-kick" | 1/4 | med | The tail is SetClockSourceInternal(mode, blankSlate=1); the 1100 ms rationale has no source.
- 35 | "vtable[0x688] getBusResetContext()" | 4 | low | Slot 0x688 = IOAudioDevice::getWorkLoop.
- 46, 56 | "selector 0xBF1 — bus-reset selector … likely FWEngine::BusResetWorker" | 4 | med | Resolved: ReconnectP2P.

### A1-ghidra/Q5-clock-source.md
- 135 | "then calls the Rev1 vtable[0xb70] (plug-rate-set)" | 4 | med | `__ZTV17FWRev1AudioDevice`+0xb70 = slot 0xb60 = FWAVCAudioDevice::SetClockSourceInternal @0x23148 (super-call).
- 274, 315 | "this->vtable[0xbe0](this, mode); // AV/C stream kick" | 4 | high | SwitchDigitalSignals.
- 16 | "SetDigitalInputActive branch" | 4 | low | Symbol absent; SetExternalSyncEnabled.

### A1-ghidra/Q6-fwhsci.md
- 5 | "HSCIReadComplete 0x1d81e, Release 0x1d8c4, Stop 0x1d8dc, Start 0x1d8fb" | 4 | med | Actual: 0x1d80e, 0x1d8a4, 0x1d8fe, 0x1d94e.
- 23, 95 | "+0xd4 fLastErrCode … != 0: increment fLastErrCode" | 4 | med | Error path 0x1d843 only re-arms 1000 ms; +0xd4 is zeroed and added to the 50 ms re-arm (0x1d4e2, 0x1d50d-0x1d513).
- 9 | "N = this->rate >> 2" | 3 | low | [+0xc0] is a byte length.
- 94 | "dispatch … re-arm timer in 50 ms" | 4 | low | Via InvokeOnNewThread(HSCISafelyComplete); 50+[+0xd4].

### A1-ghidra/Q1, Q1a, Q1b
- Q1:177, 187 | "createLocalIsochPort … setting the OHCI s bit on kDCLCallProcOp nodes" | 1 | high | IOKit-internal claim with no source; contradicts C2/C4 "timestamp node".
- Q1a:85 | "9 \| SetTagSyncBits" | 4 | med | AddSetTagSyncBits emits 0xb.
- Q1b:156, 158 | "9 kDCLSetTagSyncBitsOp … 11 kDCLLabelOp" | 4 | med | AddLabel emits 0x9; verify against the 10.13 SDK `IOFireWireFamilyCommon.h` on the build PC.

### Other A1 docs
- Q7:65 | "presumably so the panel/prefpane can observe" | 1 | low | Resolve the global at 0x14467c(%rip).
- Q7:247 | "Likely the user-space firmware updater talks via FWUserClient::externalMethod" | 1 | low | Contradicted by A4-H2 (setProperty path); needs a correction block.
- Q16:39, 75 | "AVC opcode 0x1A is the AV/C stream-input-connect" | 1 | low | AV/C General spec: 0x1A = SIGNAL SOURCE; also Rev2-only.
- Q17:114 | "the DM1000 firmware is tuned against 0xCF" | 1 | med | Unlabelled hypothesis.
- A1-ghidra/README:17, 19, 38 | "1100 ms stream-kick", "12-byte envelope", "0x18 STATUS read-back" | 4 | low | Stale index rows.
- A1-reference-kext.md:55 | "com_m_audio_FW1814Device → likely FWRev2AudioDevice" | 4 | low | Rev1 (0xf473); no correction here.

### B docs
- B1:85 | "AV/C stream-kick after clock change (vtable[0xbe0])" | 4 | med | vt[0xbe0] = SwitchDigitalSignals.
- B1:132 | "probably the mechanism behind A1-Q6's 50 ms floor" | 1 | low | Speculation.
- B2:298-304 | "cross-validates project_fw1814_stream_kick as the correct FW1814 path" | 2 | med | Third-party only; contradicted by Q3a §3 Correction.
- B2:304 | "FWHSCI::ChangeRate (A1-Q15c)" | 4 | low | No such symbol (Q15c:6).
- B4:20, 59, 61, 280 | "plug map + stream-kick we locked are hardware truth" | 2 | med | macfw/Linux only; the kext order differs.
- B5:408 | "SetDigitalInputActive(false) on the engine via vtable[0xce8]" | 4 | low | SetExternalSyncEnabled.
- B5:530 | "MIDIServer userclients … presumably" | 1 | low | —

### A2 / A3 / A4 / A5 / A6 / README (sample)
- A2-P9:212 | "≈ 18 … inputs, ≈ 14 outputs" | 4 | med | Kext page is 52 slots (38 hardware + 14 SW-return).
- A2-P6:51 | "aux-send is unavailable above 48 kHz (standard BeBoB limitation)" | 2 | low | No source.
- A2-P-ClockUI-encoding:48 | "Value 3 also calls SetDigitalInputActive(false)" | 4 | low | Symbol name.
- A3-M4:23, 42 | timestamped ring at "+0x010" vs this[0x10]=ptr | 4 | low | Ambiguous; it is the root of the C3/C5 split.
- A4-H6:45 | "Likely a few hundred ms, acceptable" | 5 | low | —
- A5:110-111 | "UART1/UART2 (MIDI In/Out — likely)" | 1 | low | —
- A6:76 | "the NIB probably has an empty placeholder view" | 1 | low | —
- system-map/README:47 | "12-byte HSCI envelope per-byte" | 4 | low | 16 bytes.

---

## Category 4: spot checks verified OK

1. C1:34. FirmwareStart writes `0x01110000` to `0xFFFF:0xC8021000` (asm 0x1befa / 0x1bf0e / 0x1bf14).
2. C1:36. 2500 ms re-register timer: `$0x9c4` at 0x1bc45 in ReregisterDeviceAfterFirmwareReset.
3. C2:31. Gate: `cmpl $0x3fffffff` → `0xE00002C2` (0xe2ea-0xe2f3).
4. C2:32. Bytes 0-5 = `00 FF 00 04 00 04` (0xe31a-0xe36e).
5. C2 §2.4. Byte-6 permutation {0→3, 1→1, 2→2, 3→0} (0xe371-0xe3d1).
6. C2:34 / C3:50-52. Bytes 7/8/9 = MAClockSource bits 3/4/5 (0xe3e2-0xe425).
7. C2:37. Two `IOSleep(0x12c)` calls, fMode stored at +0x144, FWRev1 super-call (0xe450-0xe487).
8. C2:174. SetBlankSlateClockSource `IOSleep(0x9c4)` @0xe522. Side note: it passes `(fMode & 0x1c) + 0x40000003`, which drops S/MUX bit 5. Not documented.
9. C2:87, 118, 122. `clock-bijection.log` lines 218-220, 248-276 and 97-113 match the claims.
10. C2:156-160. MARateLocked accesses at mafw.asm:2068/2074/2800. Minor: "@0x2bc3" and "@0x3644" are mid-function `leaq` sites in DetectExternalSyncChanges and FWAudioDevice::initHardware.
11. C2:136 / Q6. Poll address `0xFFC7:0x600000` (0x1d63b-0x1d641); 50 ms at 0x1d464 / 0x1d9f8; 1000 ms at 0x1d776 / 0x1d851.
12. C4 §5 rate ladder. Rate / SYTInterval / SYTIncrement / FDF-index (0x2732a-0x273fe).
13. C2:230. Accumulator jump table at 0x272d8 (slice bytes): FDF 0 {1,2,1,1}; 1/3/5 {199,640,339,441}; 2/4/6 {1,4,1,1}.
14. C2:232. CIP header-2 `0x90000000` @0x27195. The template value is right; the semantics label is not.
15. C2 §4 addresses. 0x3e54, 0x3e43, 0x124cc, 0x2a84, 0x42aa, 0x479e. Slots: a78 = RequestFormatUpdateForSampleRate, a68 = GetDeviceChannelsForSampleRate, c88 = FWEngine2::PerformFormatChange.
16. C2:202. ResetLatency @0x48e7 and `orb $0x1d, 0x346` @0x48ec. Also: SetIOFormat does `orb $0x6f` @0x4310, which is not documented.
17. Q10 / C6 col 8. GetRoundTripLatencyForFDF @0xcf30 with jump table at 0xcf78: 210/210/225/358/378/642/698.
18. C4:80-83. ConnectAllPlugs `[+0x2d3]` gate, SufficientBandwidthAvailable, fConnected set/clear. Only the return code is wrong.
19. C4:159. Overrun callbacks at 0x2533a / 0x25340 are empty.
20. Q17 / C4:197-198. `0xCF` @0xa6da and `0x80` @0xa701; `inTaskWithPhysicalMask` @0xa49e.
21. Q-DCL. RX `$0x4` @0x25a99; TX 4 banks with `cmpl $0x4` loops (0x25b8c...); P = `[+0x4e0]` @0x25e00.
22. Q-CMP. oPCR `0xF0000904+4n` (0x23fbd); iPCR `0xF0000984+4n` (0x23f5e); `0xC1003C7A` @0x2486e; 4 retries with `IOSleep(100)` @0x24690; SetPacketSize +12/+8, 0x100 and 4P (0x260bc-0x260f3).
23. Q3a §3 Correction. `or $0x18` / `setne` @0x137a5-0x137ae; `IOSleep(500)` @0x121d7; INPUT esi=1 @0x12211; OUTPUT esi=0 @0x122ae.
24. Q-UC. GetPeakLevelCount = 0x34 @0x1d30; withOptions(0x10000,0x1000,0x1000) @0x3598; FWUserClient vt 0x960/0x968/0x970. The C3/C4/D correction blocks hold: vt[0xba8] = CustomMixerWriteQuads @0x22c7a, vt[0xbe8] = AVCControlSetLEDStatus @0xdaaa.
25. B5 captures and third-party cites:
    - `12-ioreg-full.txt:5807-5814`: two FWUserClient instances, pid 64 and pid 172.
    - FWEngine2 latency 40/216/32 @44.1k (`12-ioreg-full.txt:3976+`, `40-ioreg-playback.txt:4200+`) and 48/231/32 @48k (`50-ioreg-logic-playback.txt:4242+`).
    - snd-bebob `bebob_maudio.c:312-337` (`msleep(100)` at :326) and `:183-198` (12-byte buffer).
    - FFADO `special_avdevice.h:55-69` offsets, `special_mixer.cpp:89` `0x7ffe8000`, `special_avdevice.cpp:161` comment.
    - Kext DeviceChannels tables match C4 §2 counts.

## General notes
- **"synthesis" / "computed from" labels.** C1/C2/C6 use these instead of the approved markers (`guess`, `hypothesis`, `unverified`). Several of these rows are carrying the errors listed above (C2:242, C2:283, C6 §2).
- **Missing correction blocks.** The 2026-10-14 findings in Q3a §3, Q-CMP, Q-UC-surface and this audit's vtable resolutions have no correction block in C1–C6, D, or Q15c/Q4/Q5/Q6. Q15c is the upstream source of most of the slot mislabels.
