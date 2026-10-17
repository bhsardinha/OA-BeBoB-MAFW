# C4 — Transport + mixer-control synthesis (CMP/PCR map, DCL rings, AM824 payload, format change, mixer model)

> **Correction (2026-10-15, M-C / R44):** §2.1 pseudo-code has the talker/listener roles reversed (local DCL port = talker, remote = listener) and §2.4's "program start" is `Connect`; the `[+0x268]` AV/C object is `FWAVCUnit` (`vt[0x8b8]` = retrying `AVCCommandWithErrorResponse`). Evidence: [`A1-ghidra/Q-MC-cmp-isoch-avc-start.md`](A1-ghidra/Q-MC-cmp-isoch-avc-start.md).

> **Correction (2026-10-15, M-B / R41):** §3 "per-segment node order" — `SetTagSyncBits(1,0)` goes on the **first primer of each TX bank**, not "once, on the last-segment-flag segment"; §11 / C2 §6.1 open item "who calls `takeTimeStamp`" is closed: the **TX callback of bank 3** (lap ≥ 3), see [`A1-ghidra/Q-MB-dcl-callbacks.md`](A1-ghidra/Q-MB-dcl-callbacks.md) §3. Packet buffers are assigned to the chain offset by one (`Q-MB` §4). The callback's cadence/SYT/DBC logic is decoded there (the C4 "callback flow" paragraph is a summary only).


**Status:** consolidated 2026-10-14 (task P5 of `docs/BUILD-PLAN.md`). All 2026-10-14 Correction blocks that sat on top of this file are applied to the body and listed in the change log at the end; the 2026-10-13 original is in git history (`git show a9cb63c:docs/system-map/C4-transport-control-path.md`).

**Sources:** A1-Q1/Q1a/Q1b/Q1c (DCL), **Q-DCL-geometry-table**, **Q-CMP-connect**, Q2/Q15e (SetupCIP, SYT), Q3/Q3a/Q3b (engine, AV/C, defaults), Q12 (AC3), Q15/Q15b/Q15c/Q15f + **Q-MAMAP-key-crosswalk** (mixer), Q17 (TPacketPool), **Q-OHCI-interrupt**; B1 (FFADO register map), B2 (snd-bebob), B2a (Rust), B4 (macfw); capture 74 (SL TX descriptors). Bundle: kext 1.11.3 (`otool-exports/mafw.asm`). "SL" = Snow Leopard rig, driver **v1.9** (version-skew rule). Third-party material (FFADO/snd-bebob/Rust/macfw) decodes opaque bytes; where it disagrees with the kext, the kext wins.

**Scope:** transport + mixer-control model. Lifecycle = C1; sync/clock = C2; comm envelope = C3; UC selectors = C5; latency numbers = C6.

---

## 1. Transport overview

```
CoreAudio HAL ── performFormatChange / clip callbacks ──┐
                                                        v
FWEngine2 / FWBaseEngine (IOAudioEngine)
  fRate [+0x220]  fRateIdx [+0x228]  SYT interval [+0x224]  AC3 flag [+0x2d4]  gate = vt[0x688]
  DCL program list [+0x178]   output program [+0x350]   input program [+0x358]
        │                                                       │
FWDCLOutputProgram (object 0x620 B)   shared sync [+0x240]   FWDCLInputProgram (object 0x518 B)
  TX: 4 banks × (4 one-packet segs        ◀── RX program points back at TX ──▶  RX: 64 segs × 4 packets
      + 1 big seg of P-4 packets)            program at [+0x508]                     + 64 jumps
      + 4 jumps = 20 segments                                                       TPacketPool (RX side)
  TPacketPool (TX side)
        │ DCL chain                                              │ DCL chain
        v                                                        v
   bus->createLocalIsochPort(talking, head, NULL, 0, 0, 0)   (classic DCL, §3)
        │ OHCI IT / IR contexts: interrupt bits only on transfer DCLs that carry a callback (CallProc), §3
        v
   1394 isoch ──── FW1814 (DM1000): plug 0 = PCM in both directions (§2)
```

The two sizes `0x620` / `0x518` are the **class instance sizes** of the output and input programs (`OSMetaClass` size argument and `OSObject::operator new(0x620)` in `FWEngine2::initHardware @0x12622`), **not** packet-buffer sizes (the 2026-10-13 text and Q-DCL read them as a fixed 1568/1304-byte packet buffer; that is retracted — packet size is rate/mode dependent, §2.2). The RX program holds a pointer to the TX program at `+0x508` so the RX callback can read TX-side dbc/SYT history (initHardware, A1-Q3). Format changes, DCL rebuilds and rate commits run on the engine command gate (§7).

---

## 2. CMP / PCR

### 2.1 Plug map and call chain
Counts: `oMPR = 0xbfff0002` (2 output plugs, device→host) and `iMPR = 0x80ff0003` (3 input plugs), `oPCR[0] = 0x80000080`, `iPCR[0] = 0x80000000` — read by macfw on macOS (`devices/fw1814/analysis/special-firmware-notes.md:16-20`; third-party hardware). **The kext never reads the MPRs**; it reads/locks PCR quadlets (§2.3) and calls `FWDCLOutputProgram::Connect(0)` and `FWDCLInputProgram::Connect(0)` with a literal 0 (`xorl %esi,%esi` @`0x121c5`/`0x1225f`) — most plausibly plug 0 in both directions (**the uint could also be a channel hint; IRM allocates the channel, so plug is the likelier reading — not proven**). Plug 0 carries PCM (+ MIDI slot) both ways; MIDI is a slot inside the plug-0 stream (macfw formations: capture `10 PCM + 1 MIDI`, DBS 11; playback `6 PCM + 1 MIDI`, DBS 7; the kext's DBS = audio + MIDI quadlets). The earlier "plug 1 = MIDI, plug 2 = AC3" is unsupported.

`ConnectAllPlugs @0x12476` (`vt[0xcb8]`): gate on `fConnected [+0x2d3]` (returns `0xE00002BC` when already connected, `0x12488`) → `SufficientBandwidthAvailable()` (bandwidth failure `0xE00002BE`) → `ConnectAudioMIDIChannels @0x12156` → `fConnected = (err == 0)`. The kext does not send `ExtendedPlugInfo` (Q3a §4).

```
ConnectAudioMIDIChannels
 ├─ output program Connect(ch) @0x25790 → CreateIsochChannelAndPorts(talking=true,  ch, packetSize) @0x255c2
 └─ input  program Connect(ch) @0x257a4 → CreateIsochChannelAndPorts(talking=false, ch, packetSize)
CreateIsochChannelAndPorts: local = bus->vt[0x858](talking, dclProgram, …)   ← createLocalIsochPort
   remote = new FWRemoteIsochPort (init with connection manager [[prog+0x70]+0xf0], !talking, ch)
   chan   = new FWIsochChannel (doIRM = true, packetSize, speed = nub->vt[0x850]())
   chan->addListener(local); chan->setTalker(remote); chan->allocateChannel()
```
- **Channel:** chosen by the IRM (`FWRemoteIsochPort::getSupported` returns speed `0x7fffffff`, channel mask all ones); no fixed channel. **Bandwidth:** derived by IOFireWireFamily from `packetSize` and speed. **Speed:** `nub->vt[0x850]` = `IOFireWireNub::FWSpeed() const @0xef38` (IOFireWireFamily 4.7.2; R8).
- Return paths and the 122-quadlet host-PCR payload meaning are unverified (below).

### 2.2 Packet size per direction (`[prog+0x60]`, `FWDCLProgram::SetPacketSize @0x25e56`)
| Direction | Formula (bytes) | Count |
|---|---|---|
| RX | `4 · (a + m) · SYTInterval + 12` (`lea 0xc(,%rax,4)`, ~`0x260ac`) | `0x100` = 256 = 64 × 4 |
| TX | `4 · (a + m) · SYTInterval + 8` (`lea 0x8(,%rax,4)`, ~`0x260d6`) | `[+0x4e0] << 2` = 4 · P |

`SYTInterval` = `[prog+0x4ac]`; `a + m` = audio + MIDI quadlets per data block (the DBS written into CIP header 1 from `[+0x64] + [+0x68]`). Example confirmed live: capture 74 (SL v1.9, idle) shows **424-byte TX packets = 8-byte CIP + 8 events × 13 quadlets** (DBS 13 = 12 audio + 1 MIDI at the current clock bits; CIP template `01 0d 00 00 | 90 ff ff ff`). `SetPacketSize` panics `"IsochStreamRunning during SetPacketSize"` if `[+0x8d]` is set. The extra 4 bytes on RX are the isoch packet header delivered to the buffer (**reasoning**).

### 2.3 PCR lock sequence (`updateP2PCount @0x244f0`; Q-CMP-connect §3)
Device **oPCR[n] = `0xFFFF:F0000904 + 4n`**, **iPCR[n] = `0xFFFF:F0000984 + 4n`** (`CoreMakeP2POutputConnection @0x23f86` / `…Input… @0x23f50`). Read the quadlet (`[dev+0xe0]->vt[0x888]`), byte-swap, set online (`0x80000000`), set channel bits 21:16 if given, speed bits 15:14 if given, point-to-point count bits 29:24 (+1 on connect, cleared on break), then compare-and-swap (`vt[0x8a0]`); up to **4 attempts with `IOSleep(100)` between them**; `0xE0008014` is retryable. Overhead-ID/payload of the device oPCR are not modified. **Host-local iPCR** (`Connect_iPCR @0x247ba`): value `ch << 16 | 0xC1003C7A` (online, broadcast, p2p, overhead-ID `0xF`, payload `0x7A` = 122 quadlets — meaning of the fixed payload **unverified**).

### 2.4 Start order
The reference starts TX first: `ConnectAudioMIDIChannels` runs TX program start → `IOSleep(500)` → INPUT plug-signal-format → `IOSleep(500)` → RX program start → OUTPUT plug-signal-format (R1; C2 §7). macfw independently found that host→device (playback) must run before capture and that the INPUT rate command fails while the host stream carries only NODATA packets (`duplex_lifecycle.h:106-116`, `special-firmware-notes.md`) — consistent with the reference. The claim "reversing the order fails the second CMP connect" has **no capture**; treat as third-party/unverified.

---

## 3. DCL rings (classic DCL; `Q-DCL-geometry-table.md`)

**TX** (`BuildTransmittingDCLProgram @0x25ae4`, caller `RebuildDCLProgramCore @0x25df4`; arena `SetBufferSize(0x14000)` = 80 KiB at `0x25b22`): **4 banks** (refcon 0..3). Per bank: 4 × `BuildPacketDCLSegment(packetCount = 1, callback = NULL)` ("primers") + 1 × `BuildPacketDCLSegment(packetCount = P−4, callback = OutputDCLStaticCallback)` ("big segment"), then `AddJump`; linker (`0x25d9d..0x25dde`): `jumps[k-1].label = labels_of_bank_k` for k = 1..4, bank 3 wraps to bank 0. **20 segments + 4 jumps.** `P = ⌊65,536,000 / (rate·4)⌋ + 2` (`SetupCIP @0x27113..0x27133`, `+0x4e0`; rate in Hz): 32 k → 514, 44.1 k → 373, 48 k → 343, 88.2 k → 187, 96 k → 172, 176.4 k → 94, 192 k → 87 (TX packets per bank; ×4 per ring: 2056 … 348).

**RX** (`BuildReceivingDCLProgram @0x25962`, caller `0x25a86`; `SetBufferSize(0x10000)` = 64 KiB): **64 flat segments**, each `BuildPacketDCLSegment(packetCount = 4, callback = InputDCLStaticCallback)` (the `$0x4` immediate at `0x25a99` is fixed at every rate) + one jump; `jumps[i].label = labels[(i+1) & 0x3f]` (`0x25a2f`). **64 segments + 64 jumps = 256 packets**; 4 packets = 4 isoch cycles ≈ 500 µs per segment. `Start()` stores `current_seg = 0x3f` so the first callback expects segment 0.

**Per-segment node order** (Q1a): `AddLabel` (opcode `$0x9`, `0x6bdd`) → [`AddSetTagSyncBits(1,0)` (opcode `$0xb`, `0x6d34`) once, on the segment built with the last-segment flag — TX only; RX tag/sync come from the IR context register] → N × transfer packet (`kDCLSendPacketStart` TX / `kDCLReceivePacketStart` RX) → `DCLPtrTimeStamp` (14) → `DCLUpdateDCLList` (12) → `DCLCallProc` (8, only when a callback is given) → jump. (Opcode numbers: R5 corrected an earlier swap; cross-check against the SDK header `IOFireWireFamilyCommon.h` on the build PC is **unverified**.)

**Interrupt mechanism (resolved, Q-OHCI-interrupt):** on 10.13.6 (`AppleFWOHCI` 5.5.9) `translateLegacyProgram @0x8d5e` folds a `kDCLCallProcOp` into the preceding transfer DCL as its callback (case `0x8f69`), and `SendDCL::compile @0xd53f..0xd60e` / `ReceiveDCL::compile @0xcce7..0xceac` OR the `0x300000` interrupt bits into the last descriptor only when a callback is present. The timestamp node only calls `setTimeStampPtr()` on the preceding DCL (`0x901d`); a timestamp pointer alone *suppresses* the interrupt bits. Capture 74 agrees: only a repeating subset of the TX descriptors carries the extra interrupt bits (`0x183c` vs `0x100c`). Earlier statements that "the timestamp node makes OHCI raise the interrupt" / "without it callbacks never fire" are wrong; the pre-Ghidra TX-silence root cause is a **hypothesis** (e.g. a missing/dropped CallProc, or a 4-segment ring). Where `takeTimeStamp` is called from is **unverified** (C2 §6.1).

**Callback flow.** `OutputDCLStaticCallback(cmd)` dereferences `cmd->procData` = `{self, segIdx}` and tail-calls `OutputDCLCallbackPrepareNextSegment`; the TX callback also re-clears the next group (`ZeroElements`, §6) and refills it from the engine's output buffer. The RX callback `InputDCLCallbackCopyPacketGroupToSamples @0x26e3e` copies samples (§6.3). The **overrun callbacks** (`InputDCLStaticOverrunCallback`, `InputDCLCallbackOverrun`, `0x25340/0x2533a`) are empty (Q1c); the clone must not add recovery the reference omits.

**Classic DCL, not NuDCL.** The reference hands the raw DCL chain head to `bus->createLocalIsochPort(talking, head, NULL, 0, 0, 0)` (`bus->vt[0x858]`, no opcode translation; Q1b). CLAUDE.md records that the NuDCL / `IOFWDCLPool` path returns `kIOReturnNoResources` from kernel space on 10.13.6. **Q1b's statement "classic DCL works on 10.13.6, the 2014 kext proves it" is withdrawn**: the 2014 kext never got past matching on 10.13.6. Verified only that `createLocalIsochPort → AppleFWOHCI::createDCLProgram @0x75a4 → AppleFWOHCI_DCLProgram::init @0x89d0` still exist in 10.13.6 with a kernel-task branch for `info == NULL`; whether `generateBufferMap` accepts the chain is **unverified at run time**. **P1 result (R36):** the acceptance test is `generateBufferMap`; it passes if every transfer buffer and every timestamp slot is a non-NULL kernel address inside one fully mapped, 32-bit-DMA-addressable span — which the reference guarantees by placing both in the single `TPacketPool` allocation (timestamp slots in its last page). See `Q-P1P2-dcl-acceptance-and-pool-memory.md` §2.

---

## 4. Rate table, buffer ladder and pool pages

**Rates** (`SetupCIP`, A1-Q2/Q15e): same 7-row table as C2 §5 (FDF index 0..6 = 32/44.1/48/88.2/96/176.4/192 kHz; SYT interval 8/8/8/16/16/32/32; SYT increment `0x1800/0x116A/0x1000/0x116A/0x1000/0x116A/0x1000`; default fallthrough = 44.1 kHz).

**Channel counts** (R12; `SetupAudioEngine @0xed34`, kext `DeviceChannels` tables `0x363c0/0x364a0/0x36580/0x4ab80`): outputs 6 or 12 by `MAClockSource` bit 4, inputs 10 or 16 by bit 3 at 44.1/48 kHz (S/PDIF base in 10 / out 6; ADAT in 16 / out 12); at 88.2/96 kHz ADAT in 12 / out 8 (SL captures 67a/67b, `sADAT88_96 {12,12,8,8}`). Counts at 176.4/192 kHz are 4 out / 2 in in every mode (`s176_192DeviceChannels`, P4/R38). The complete per-mode, per-rate table (channels, packet bytes, pool pages, bandwidth bytes per cycle) is `A1-ghidra/Q-P4-cmp-per-rate-and-mixer-power-on.md` §A.2; `m = 1` MIDI quadlet per block is verified (`SetupAudioEngine` `0xed7a`). Hardware arithmetic: in = 8 analog + (2 S/PDIF | 8 ADAT); out = 4 analog + (2 | 8).

**Packet bytes per rate/mode** = `8 + 4·DBS·SYTInterval` (TX) / `+12` RX (§2.2). The 2026-10-13 table of "fixed 1568/1304-byte packets" and the derived "multi-packet-per-frame" inference are withdrawn.

**Pool pages.** `TPacketPool::Initialize @0xa3e6` allocates `⌊[+0x18] / [+0x28]⌋ + 2` physical 4 KiB pages (R8; the earlier `⌈N/⌊4096/S⌋⌉ + 2` wording is superseded) through `IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, size, physMask & ~0xFFF)` with `packetsPerPage = ⌊4096 / packet_bytes⌋`. Per-rate page counts are computed in `Q-P1P2-dcl-acceptance-and-pool-memory.md` §3.2 (TX 60–167 pages, RX 25–53 pages, i.e. ≈ 0.1–0.7 MB per pool; DBS at 176.4/192 kHz inferred); the per-rate table in Q-DCL (fixed 1568/1304-byte packets) is withdrawn. **P2 result (R36):** the allocation is `inTaskWithPhysicalMask(kernel_task, 0, pages<<12, bus mask & ~0xFFF)`, which on 10.13.6 becomes a wired, virtually contiguous, 4 KiB-aligned, below-4-GiB, **not physically contiguous** block (`kernel_memory_allocate` with `KMA_LOMEM`); run-time success is unverified until M-B.

**Buffer ladder:** `{64, 128, 256, 512, 1024, 2048, 4096}` frames, default 512 (memory `project_fw1814_buffer_sizes`; parity with the 2014 Panel). On macOS the size is driven by CoreAudio through `IOBufferFrameSize`; the kext has no per-rate buffer-size cookie (cookies `0x5ff`/`0x613` are Windows-only, memory `project_mac_vs_windows_scope`).

---

## 5. Format change — transport view

The chain is documented once, in **C2 §4** (slot names per R2). Transport-side effects: `pauseAudioEngine` and `beginConfigurationChange` stop work before the cache update; `FWBaseEngine::PerformSampleRateChangeCore @0x479e` iterates the DCL programs (`[+0x178]`) calling `vt[0x128]` `ApplyRateChange`, which re-derives each ring's geometry for the new SYT interval/packet size (the reference re-allocates the pools on a rate change, Q15c); `ResetLatency` runs unconditionally and sets the dirty flags (`[+0x346] |= 0x1d`). `InitGroupCIPsForOutput(group, extraCount, cipTemplate*)` re-seeds the two CIP headers in each DCL group after a rate change without rebuilding the ring (Q17). The AV/C plug-signal-format frames are **not** part of this chain; they are sent at stream start (C2 §7). AC3 enter/leave: §9.

**Hazard (not a requirement):** v1.9 page-faults in IOAudioFamily buffer re-allocation when `MASampleRate` is written mid-stream (B5 cap 65). Clone: gate the write (C3 §9).

---

## 6. AM824 payload

### 6.1 Layout
Per packet: quadlet 0 = CIP header 1 `((nodeID & 0x3f) << 24) | (([+0x64]+[+0x68]) << 16)` (`SetupCIP 0x27173..0x2718e`, stored `[+0x498]`, DBS = audio + MIDI quadlets, no `0x3F0000` mask); quadlet 1 = CIP header 2 `0x90000000` (`[+0x49c]`, `0x27195`; the CIP Q1 word EOH/FMT, not "AM824 tag bits") plus FDF/SYT; then N data blocks of `(cipDataQuadlets + cipPadQuadlets)` quadlets from packet offset +8. Non-sync packets carry the NODATA SYT value (A1-Q2 `CalcIsochPacketHeaders`).

### 6.2 Fill and labels (R10; capture 74)
`TPacketPool::ZeroElements(uint)` (asm 11694; `movl $0xcf` at `0xa6da` for data quadlets, `movl $0x80` at `0xa701` for pad/MIDI quadlets) runs at pool init **and from every TX callback** (`FWDCLOutputProgram::OutputDCLCallback`, `0x26874` primary pool, `0x268b8` secondary), clearing the next group before `SetValidPackets`/`InitGroupCIPsForOutput`. The live output converters then overwrite the slots they handle: audio label **`0x40`** (e.g. `LocalRearrangeSampleFrame16Bit2` stores `((s<<16)+s)&0xFFFF00 + 0x40`, `0x1197f-0x11986`; silent slots `movl $0x40`, `0x1195d`) and MIDI/other slots **`0x80`** (`0x119bd`). So `0xCF` is visible on the wire only for slots no converter rewrites (and before the first callback); the silent *handled* audio quadlet is `0x40` with zero data — the same label as snd-bebob's `0x40000000` (`amdtp-am824.c:217`). **What the DM1000 does with label `0xCF` is unverified**: firmware strings (R23: `inLockDetection`, `LOCK detection is PLL_LOCK / FIX_LOCK`, `pllInLock`, `EmptyPktGen`, `ISO_NOT_INLOCK`) never mention AM824 labels or `0xCF`; the live label bytes could not be read on the SL (capture 74). The 2026-10-13 claims "firmware tuned against `0xCF`", "PLL locks because of `0xCF`", "bzero'd label caused the clicks" and "write-only register window" are **hypotheses**. Clone: mirror the code (init fill, per-callback re-fill, converter overwrite); do not describe `0xCF` as proven necessary.

### 6.3 RX: empty packets and DBC
`InputDCLCallbackCopyPacketGroupToSamples @0x26e3e` reads, per packet, the first CIP quadlet at packet `+4` (DBC = byte 7; bit 7 of byte 4 = EOH set → packet skipped, `0x26f84`) and the SYT half-word at `+0xa`. If SYT == `0xFFFF` (empty/NODATA) it increments `[+0x510]` and jumps over the packet **before** the DBC is used (`cmpw $-0x1` `0x26fc1`, `incl 0x510(%rbx)` `0x26fc7`). For data packets the DBC is compared with an expected value `[+0x505]` (flag `[+0x504]`); a mismatch re-seeds `[+0x505]` (`0x27016-0x2702f`), then `[+0x505] += SYTInterval ([+0x4ac])` (`0x27041`); there is no lost-packet error path. So snd-bebob's `CIP_EMPTY_HAS_WRONG_DBC` quirk (`bebob.c:170-175`, `bebob_stream.c:449-452`) is invisible to the kext (B2 Correction). **Clone: do not add strict DBC-continuity checks the reference lacks.** *Unverified:* that the FW1814's wrong-DBC empties carry SYT `0xFFFF`.

---

## 7. Mixer control model

**Framework (Q15 + Q-MAMAP §1).** `FW1814Device::InitializeUserSettableProperties @0xe6ca` builds six channel groups and four settings objects:

| Group (device offset) | Size | Role | Register block |
|---|---|---|---|
| `[+0x1668]` | `<4>` | SW return | `0x00` |
| `[+0x1318]` | `<4>` | Output | `0x08` |
| `[+0x13e0]` | `<18>` | Input | `0x10` |
| `[+0x1730]` | `<4>` | Headphone | `0x38` |
| `[+0x1b00]` | `<2>` | Aux output | `0x34` |
| `[+0x17f8]` | `<22>` | Aux send | `0x64` (`CustomMixerWriteWithRotatedAuxChannels`) |

Objects: `FWSettingsRotaryControl` (+0x470), `FWSettingsAuxSend` (+0xf98), `FWSettingsLevels` (+0x538), `FWSettingsABSwitch` (+0x2d8). Properties `MA<P>Levels/Mute/Solo/Link` are `FWChannelGroupProperty` objects (0x78 B: key, kind, group, partner); level range −255..0. Dispatch is the single generic `FWPropertyDict::SetProperties @0xa0e0` (no `FWMixer::setProperties`): `SetOneProperty @0x9f74` → `ModifyProperty` (queue) → workloop `checkForWork` → `ApplyModifiedProperties @0x9b40` → `CallSetter @0x940e` → `SendLevelsToDevice @0x1ef64` (delta apply against an `AdjustedLevels` snapshot). The full per-key crosswalk (43 settable keys, registration function, setter, wire, default, unverified cells) is **Q-MAMAP-key-crosswalk.md**; the wire forms are in **C3 §3**.

**Topology (B2a §1, Rust `special.rs:14-41`): 22 mixer rows × 4 main outputs + 22 × 2 aux** — 18 physical inputs (8 analog + 2 S/PDIF + 8 ADAT) + 4 stream returns; main mix → mixer-output pairs 0/1; aux mix → aux pair 0; routed through the headphone/analog-out selectors to 4 analog + 4 headphone outputs. **Stream inputs 5..12 bypass the mixer** and go straight to digital outputs 1..8 (B2a §5); the clone cannot route them through the mixer and publishes them only as IOAudioStream channels.

**Effects of the typed model:** mute/solo/dim have no register — they are level operations (minimum / −10 dB; C3 §3). `MARateLocked` is a one-way broadcast (C2 §3). Delta-only writes mirror Rust `partial_update` (`special.rs:1053-1093`, B2a).

---

## 8. Mixer register map (third-party reference; kext encodings where decoded)

Window `0xFFC7:0x00700000 + offset`, 40 quadlets = 160 bytes = Rust `CACHE_SIZE` (`special.rs:308`), composite of FFADO `special_avdevice.h:50..134`, macfw `special_mixer.h:14-39` and Rust offset ranges:

| Offset | FFADO macro | Meaning |
|---|---|---|
| 0x00 / 0x04 | `GAIN_STM_12_IN` / `GAIN_STM_34_IN` | stream 1/2, 3/4 gain (SW return) |
| 0x08 / 0x0C | `GAIN_ANA_12_OUT` / `GAIN_ANA_34_OUT` | analog 1/2, 3/4 output |
| 0x10–0x1C | `GAIN_ANA_{12,34,56,78}_IN` | analog input monitor gains |
| 0x20 | `GAIN_SPDIF_12_IN` | S/PDIF 1/2 input |
| 0x24–0x30 | `GAIN_ADAT_{12,34,56,78}_IN` | ADAT input gains |
| 0x34 | `GAIN_AUX_12_OUT` | aux 1/2 output master |
| 0x38 / 0x3C | `GAIN_HP_12_OUT` / `GAIN_HP_34_OUT` | headphone 1/2, 3/4 |
| 0x40–0x4C | `LR_ANA_{12,34,56,78}_IN` | analog L/R balance (pan) |
| 0x50 | `LR_SPDIF_12_IN` | S/PDIF L/R |
| 0x54–0x60 | `LR_ADAT_{12,34,56,78}_IN` | ADAT L/R |
| 0x64 | `AUX_STM_12_IN` | aux send: stream 1/2 |
| 0x68 | `AUX_STM_34_IN` | aux send: stream 3/4 |
| 0x6C–0x78 | `AUX_ANA_{12,34,56,78}_IN` | aux sends: analog |
| 0x7C | `AUX_SPDIF_12_IN` | aux send: S/PDIF |
| 0x80–0x8C | `AUX_ADAT_{12,34,56,78}_IN` | aux sends: ADAT |
| 0x90 | `MIX_ANA_DIG_IN` | routing bitmap: analog + digital → mixer banks |
| 0x94 | `MIX_STM_IN` | routing bitmap: stream returns → mixer banks |
| 0x98 | `SRC_HP_OUT` | headphone source per pair |
| 0x9C | `SRC_ANA_OUT` | analog-out source per pair |

The kext's own group offsets (`0x00/0x08/0x10/0x34/0x38/0x64`, and `0x40+`, `0x90`, `0x98`, `0x9c` for pan/routing/headphone/aux-monitor) match this map (Q-MAMAP; B4 §5 corroborates).

**Encodings.** *Kext (decoded, Q-MAMAP §1):* level int16 = property level (−255..0, 0.5 dB) × `0x80`, big-endian, two channels per quadlet; minimum (−255) = `0x8080`; unity = 0. *FFADO/B1 §4.3 (third-party):* AV/C 8.8 dB stereo-packed `(L<<16)|R`, `0x8000` = mute, `0x0000` = unity — **the kext sends `0x8080` as its minimum, not `0x8000`**; use the kext. *Pan:* FFADO/macfw give `0x7FFE` hard left, `0x0000` centre, `0x8000` hard right (`special_mixer_model.h:20-24`) and FFADO writes `0x7ffe8000` for the nine LR quadlets at init (B1 §6.6); the kext's `MAInputPan2` encoding is `((−p>>1)&0xff) + p·(−0x8000)` into an image at `[+0x12b6/0x12b8 + 2(ch−1)]` with default pairs `{−255, +255}`; **the wire half-word order is not derived** (Q-MAMAP §4). Whether the kext issues an FFADO-style 160-byte init write: `ResetToFactorySettings @0xd190` resets the settings objects and resends (Q3b); no immediate `0x7ffe8000` was found — **open**.

**Routing bitfields (third-party semantics):** `0x90` (FFADO/macfw `special_mixer_model.h:163-174`, Rust `special.rs:883-911`): bits 0–3 analog pairs → mixer 1/2, 4–7 analog pairs → mixer 3/4, 8–11 ADAT → mixer 1/2, 12–15 ADAT → mixer 3/4, 16/17 S/PDIF → mixer 1/2 / 3/4. `0x94`: bit 0 stream 3/4→mixer 1/2, bit 1 stream 3/4→mixer 3/4, bit 2 stream 1/2→mixer 1/2, bit 3 stream 1/2→mixer 3/4. `0x98`: HP pair 0 bits 0..2 (`0x01` mixer 1/2, `0x02` mixer 3/4, `0x04` aux 1/2), HP pair 1 bits 16..18. `0x9C`: bit 0 = analog 1/2 source, bit 1 = analog 3/4 source (0 mixer, 1 aux 1/2). The kext's `MAInputRoutingV2` (9 two-bit masks) sets bits `n−1`/`n+3` for pairs 1–4, `0x100`/`0x200` for pair 5 and `n+2`/`n+6` for pairs 6–9 in a **big-endian memory image** `[+0x1308]` (Q-MAMAP). Analog and ADAT agree with FFADO; pair 5 (S/PDIF) is `0x100/0x200` in the image versus FFADO bits 16/17 — consistent **if** the image is byte-swapped (`0x100` ↔ `0x00010000`), **not proven**. The Panel's `MAMonitorAuxOnOutput` = both `0x9C` bits is likewise inferred.

**Clone action:** ship the 40-quadlet map as constants with each cell labelled kext-decoded or third-party; confirm per-group masks by decoding `FWChannelGroupProperty::init @0x93a4` and by SL write captures (the mixer-write capture 62 shows the property level domain; wire captures of the register window are still open).

---

## 9. AC3 passthrough fork (A1-Q12)

Fork inside `FWEngine2::PerformFormatChange @0x124cc` (asm `21037..21101`, `0x124e7..0x125ff`): `enteringAC3 = (fmtTag == 'cac3') && ([+0x2d4] == 0)`, `leavingAC3 = (fmtTag != 'cac3') && ([+0x2d4] != 0)` (`'cac3'` = `kIOAudioStreamSampleFormatAC3`; the AC3 flag is `[+0x2d4]` — `EnableAC3Format @0x44ab`, `DisableAC3Format @0x59b7`, init `0x5de7` — **not** `[+0xa5]`, which is `IOAudioEngine::configurationChangeInProgress` (10.13.6 IOAudioFamily `beginConfigurationChange` writes it, `0xa3d8`; R8). Entering AC3 commits at 48 kHz when the Panel gives no rate (`_IOAudioSampleRate{0xBB80,0}`). `SupportsAC3` returns 1 on the FW1814 (A2/A1 audit batch).

Transport consequences (Q12): (1) three AV/C transactions — `AVCVendorCommand(subunit 0, attr 1, value enable?1:0)` → read-back via `vt[0xb48]` → `AVCSelectorCommand(subunit, !enable)` (the selector flips the digital-out formatter: 0 = IEC 61937 passthrough, 1 = PCM); (2) `ClipAC3Output` replaces the normal clip callback — a 16-bit stereo byte reshuffle + `memcpy` into the DCL packet buffer (no float conversion, mixer or AM824 32-bit packing); (3) gates: format non-mixable, exactly 2 channels, `firstFrame + numFrames ≤ 0x2000`, else `kIOReturnBusy`. Clone: mirror the three transactions in order and the clip routine verbatim.

---

## 10. Stream start

The old "stream-kick choreography" section is replaced by the reference's actual order (R1) in **C2 §7** and C1 §3.7: TX start → 500 ms → INPUT `00 FF 19 …` → 500 ms → RX start → OUTPUT `00 FF 18 …`. OUTPUT=`0x18`, INPUT=`0x19`. snd-bebob's `special_set_rate` (`bebob_maudio.c:312-337`: OUTPUT → 100 ms → INPUT) and macfw's "Linux programs OUTPUT first, waits 100 ms" describe the *Linux* order; the generic BeBoB path (`bebob_stream.c:95..117`: both then 300 ms) is different again. The "BeBoB firmware requires both commits before PLL lock" rationale is **unsourced**.

---

## 11. Open questions

1. ~~Channel counts at 176.4/192 kHz per mode~~ **Resolved (P4, R38).**
2. Per-rate packet sizes/pool pages recomputed from §2.2 (Q-DCL tables withdrawn); `IOBufferMemoryDescriptor` mask/size acceptance on 10.13.6 (BUILD-PLAN P2).
3. Classic-DCL acceptance by `AppleFWOHCI_DCLProgram::init` (P1).
4. Meaning of the 122-quadlet host-iPCR payload; plug-0 reading of `Connect(0)` (§2.1/2.3) — read the MPRs on SL.
5. Whether the DM1000 reacts to label `0xCF` (§6.2) — bus capture or ARM analysis of `fw1814.bcd` (Q17.2).
6. Kext power-on mixer defaults vs FFADO's 160-byte init (§8) — Q3b partial.
7. Wire half-word order of `MAInputPan2`; routing-bit meanings; `AVCMixerConnectPair` frame (Q-MAMAP §4).
8. FDF-index-5/6 packet counts and sub-packet behaviour at ≥ 88 kHz — observe OHCI descriptor count per DCL node at M-B.

## 12. Clone-build implications (D2)

1. Classic DCL + NULL ranges via `createLocalIsochPort` (7-argument call); TX = 20 segments (4 banks), RX = 64 × 4; RX program holds `+0x508` → TX program. Gate M-B on a non-NULL port (P1).
2. Per-segment node order as §3; interrupts come from the CallProc callback; empty overrun callbacks; per-callback `ZeroElements` then converter overwrite (labels `0x40`/`0x80`).
3. `TPacketPool` with page-aligned physical backing (`physMask & ~0xFFF`), page count `⌊[+0x18]/[+0x28]⌋ + 2`, packet bytes from §2.2.
4. CMP: IRM-allocated channel, device oPCR/iPCR lock with ≤ 4 tries and 100 ms back-off, packet sizes `4·DBS·SYT + 8/12`, plug 0 both directions; start order per C2 §7.
5. Format change on the command gate; re-seed CIP headers after rate change; gate `MASampleRate` against streaming.
6. Mixer: `FWPropertyDict`-style single dispatcher (or a flatter equivalent exposing the same 43 settable keys), six channel groups with the register offsets of §7, level encoding of C3 §3, **both** wires (register block + the two AV/C-routed keys), constants of §8 labelled by source.
7. AC3 fork per §9. Stream pairs 2/3+ bypass the mixer (§7).
8. RX: no strict DBC checks; skip SYT `0xFFFF` packets before using DBC (§6.3).

## Cross-links
C1 (§3.7 start chain), C2 (§4 format change, §5 rate table, §7 stream start), C3 (§3 mixer wires, §4 AV/C), C5 (UC), C6 (latency). A1-Q1/Q1a/Q1b/Q1c, Q-DCL-geometry-table, Q-CMP-connect, Q-OHCI-interrupt, Q2, Q3/Q3a/Q3b, Q12, Q15–Q15f, Q-MAMAP, Q17; B1 §4, B2 §2/§4, B2a §1–§3, B4 §1/§2/§5; R1, R2, R5, R8, R10, R12, R23, R25, R28; capture 74.

## Change log (2026-10-14 corrections folded in)
Live TX buffer attempt (R28, capture 74) → §2.2, §3, §6.2; stream-start chain and sleep sites (R25) → §10; firmware lock-detection strings (R23) → §6.2; MA* crosswalk, channel counts (R12/R16), `0xCF`/`ZeroElements` (R10), `[+0xa5]` (R8), audit items (R1, R2, R5), `0xBA9`/`0xBE9` (Q15f), DCL ring shape (Q-DCL), OHCI interrupt → §1–§10. Removed: "64 TX segments", fixed 1568/1304-byte packet buffers and the page table built on them, "firmware tuned against 0xCF", "hardware-proven start order", the OUTPUT→100 ms→INPUT kick, the open "second mixer path" section (resolved: one register path plus two AV/C-routed keys), D1/D2/D3 wording (R34).
