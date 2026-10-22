# C2 — Sync path: clock source → PLL lock → sample rate → cycle timer → CoreAudio counter

> **Correction (2026-10-22, R131 / capture 98):** §3's lock rule is confirmed on the FW1814 with the official driver, and the status word says more than lock: within the interface's current speed range (44.1/48 or 88.2/96) the rate field is the **external source's exact rate** (48 + 44.1 source -> `01 01`, `MASampleRate` 44100, `MARateLocked` 0); outside it, lock 0 and a stale or `ff` field. The reference never follows the source (engine stays at its rate). The clone adds `MAExternalSyncRate` and a panel-side follower (`docs/RATE_FOLLOW_PLAN_v1.md`), a deliberate deviation decided by the user; `MASampleRate`/`MARateLocked` keep the reference behaviour.

> **Correction (2026-10-15, Q-START / R43):** the hypothesis below is closed: channel start is `FWEngine2::StartAudio` -> `StartPCMAudio` -> `ActivatePCMInput` (RX `Start`) then `ActivatePCMOutput` (TX `Start`, primes 4 banks first); `vt[0x120]` is `Connect`. Evidence: [`Q-START-channel-start-order.md`](A1-ghidra/Q-START-channel-start-order.md).

> **Correction (2026-10-15, M-B / R41):** (1) §6.1 "**Open:** which function calls `IOAudioEngine::takeTimeStamp`" — closed: `FWDCLOutputProgram::OutputDCLCallback @0x263c2` calls it (engine at shared-sync `+0x78`, vtable slot `0xae0`, args `(1, &abstime)`) when `bank == 3` and `lap >= 3`; on lap 2 it only records `resyncCycleTime` (`Q-MB` §3). (2) The SYT/DBC/cadence rules (§5 constants in use) are decoded in [`A1-ghidra/Q-MB-dcl-callbacks.md`](A1-ghidra/Q-MB-dcl-callbacks.md) §3. (3) §7 / R1 start order: `ConnectAudioMIDIChannels` invokes `vt[0x120](…, 0)` on the programs, which is `Connect(channel)` (IRM channel + port wiring), **not** the DCL `Start`; the wording "start the TX DCL program" in steps 1 and 5 is therefore **unverified** — hypothesis: the channels are started by `StartIsochStreams` (`vt[0xd08]/[0xd10]` of `FWEngine2::StartAudio`). Check at M-C by decompiling those slots; the 500 ms gaps and the INPUT/OUTPUT frame order are unaffected.


**Status:** consolidated 2026-10-14 (task P5 of `docs/BUILD-PLAN.md`). All 2026-10-14 Correction blocks that used to sit on top of this file are applied to the body and listed in the change log at the end; the 2026-10-13 original is in git history (`git show a9cb63c:docs/system-map/C2-sync-path.md`). Scope: sync plane only. Transport/mixer control = C4, comm envelope = C3, lifecycle = C1.

**Rule applied:** every value, offset and sleep cites a kext offset (bundle 1.11.3, `otool-exports/mafw.asm`), an FFADO/snd-bebob file:line, or a dated capture. Captures named "SL" are the Snow Leopard rig running driver **v1.9** (version-skew rule). Combination-only claims carry **synthesis**; open items carry **unverified / open** and the check that would settle them.

---

## 1. End-to-end sync chain

From a Panel click to CoreAudio's sample counter advancing at the new rate.

```
 (user click in Panel: Sync-Source radio / S/MUX / optical-out / sample rate)
 1 Panel → IORegistryEntrySetCFProperties(deviceEntry, {MAClockSource: 0x4000????})     [B5 §10, caps 61/66/68/69]
 2 MIG is_io_registry_entry_set_properties → OSUnserializeXML → OSDictionary           [B5 §10]
 3 FW1814Device::setProperties → FWAVCAudioDevice::setProperties (fast-paths
   "MASampleRate"; otherwise tail-calls the parent)                                    [A1-Q15b @0xa0e0, B5 §10]
 4 FWPropertyDict::SetProperties @0xa0e0 — single generic dispatcher, queues work      [Q15b]
 5 workloop checkForWork → ApplyModifiedProperties                                     [Q15b]
   ├─ MAClockSource branch:
   │   6a FW1814Device::SetClockSourceInternal(mode, blankSlate) @0xe25c  (§2.7)
   └─ MASampleRate branch:
       6b IOAudioEngine::performFormatChange → FWBaseEngine::performFormatChange @0x3e54
          → IOCommandGate::runAction(ActionPerformFormatChange @0x3e43) → vt[0xc88]
          = FWEngine2::PerformFormatChange @0x124cc → … (full chain in §4)             [Q15c]
 7 On hardware the DM1000 PLL re-locks against the selected source; the HSCI poll
   (@0xFFC7:0x00600000, re-armed every 50+[+0xd4] ms) reports the new rate;
   DetectExternalSyncChanges @0x2bc3 updates MARateLocked                              [Q6, Q15d xref #1]
 8 The DCL rings keep emitting AM824 at the new SYT interval; segment completions come
   from the CallProc-derived callback (§6.1)                                           [Q-OHCI-interrupt]
 9 CoreAudio's sample counter advances; IOAudioEngineSampleOffset and
   OutputSampleLatency are republished by ResetLatency (§6.2)                          [Q15c, B5 §6c, capture 72]
```

Stages 1–5 and 6b are standard IOKit/CoreAudio plumbing; 6a is the FW1814 vendor path for the clock source; 7–9 are the hardware feedback loop.

---

## 2. Clock source selection

### 2.1 Panel surface
Four Sync-Source radio buttons (manual §2.3) plus orthogonal digital-mode toggles, all encoded in one 32-bit property `MAClockSource` on `com_m_audio_FW1814Device`, written with `IORegistryEntrySetCFProperties` on the device registry entry — **not** `IOConnectCallMethod` on the vendor UC (B5 §10: zero UC events on the clock path; caps 61/66/68/69 captured the `setProperties` path).

### 2.2 `MAClockSource` composite bitfield
Locked by B5 captures 66/68/69 and the kext decompile of `SetClockSourceInternal @0xe25c` (A1-Q5):

| Bit(s) | Field | Encoding | Provenance |
|---|---|---|---|
| bit 30 (`0x40000000`) | required gate | must be 1; `0xE00002C2` otherwise | A1-Q5 (`clock-bijection.log:218..220`). **Note:** Q5's pseudo-code writes the compare as `mode > 0x3fffffff → error`, which would reject every accepted value; captures (R21: `0x40000014`, `0x40000017` accepted) show bit 30 must be *set*. The direction of the compare at the entry is therefore **unverified in the asm sketch** (check: raw `cmp/j*` at the function entry) |
| bits [1:0] | Sync-Source radio | `00` Int-unmuted, `01` Ext-digital, `10` WordClock, `11` Int-muted | B5 caps 68/69, A1-Q5 |
| bit 2 (`0x4`) | S/PDIF input port | 0 optical, 1 coaxial | B5 cap 69 |
| bit 3 (`0x8`) | Optical-IN mode | 0 S/PDIF, 1 ADAT | B5 cap 68 |
| bit 4 (`0x10`) | Optical-OUT mode | 0 S/PDIF, 1 ADAT | B5 cap 66 |
| bit 5 (`0x20`) | S/MUX (ADAT 4-ch) | 0 off, 1 on | B5 cap 66 |
| bits 6..29 | unused, always 0 | — | all captures |

Factory default after reset: `MAClockSource = 0x40000003` (`ResetToFactorySettings @0xd190`, Q3b).

### 2.3 Where each bit is consumed (no single bit-mask demux)
The whole word is stored in `fMode [+0x144]`; there is no single dispatcher:
- **bits [1:0]** → immediately, as the HSCI wire byte 6 (§2.4).
- **bits 3 and 4** → the channel counts at engine setup: `FW1814Device::SetupAudioEngine @0xed34` — bit 4 selects the **output** counts `[+0x2e0]/[+0x2ec]` = 6 or 12; bit 3 selects the **input** counts `[+0x2e4]/[+0x2f0]` = 10 or 16 (R12; SL v1.9 captures 67a/67b agree: 48 kHz out 12 / in 16, 96 kHz out 8 / in 12). The earlier "6/12 in, 10/16 out" text was swapped.
- **bits 3, 4, 5 (HSCI)** → bytes 7/8/9 of every clock-source frame (§2.4).
- **`SwitchDigitalSignals @0xe5dc`** (`vt[0xbe0]`, called from `SetClockSourceInternal` when `!blankSlate`): re-runs `SetIOFormat` and picks the peak template by the engine's **output** channel count — 6 → `_Sample1814PeaksSPDIF6`, 8 → `_Sample1814PeaksADAT8`, 12 → `_Sample1814PeaksADAT12`, otherwise `_Sample1814Peaks4All` (Q5, R8). It is **not** an AV/C stream kick.
- **bit 2 (S/PDIF port)** → consumer **open** (backlog; check: xref reads of `fMode[+0x144]` with `& 4`).
- **bit 30** → entry gate.
- **Removed claim:** "bits[1:0]==3 calls `vt[0xce8] = SetDigitalInputActive(false)`" — `vt[0xce8]` is `FWEngine2::SetExternalSyncEnabled(bool) @0x11bc0` (R2), unrelated; whether Int-muted suppresses the digital input by some other path is **unverified**.

### 2.4 HSCI wire bytes
The device-facing byte 6 is a permutation of `mode & 3`:

| Radio | `mode & 3` | HSCI byte 6 |
|---|---|---|
| Internal, digital inputs unmuted | 0 | **3** |
| External digital | 1 | **1** |
| Word Clock | 2 | **2** |
| Internal, digital inputs muted | 3 | **0** |

The clock-source frame is **16 bytes** (`bzero(…,0x10)` at `0xe284/0xe2a2`; length `0x10` passed to `[+0x268]->vt[0x8b8]` at `0xe441`; R3): `{00 FF 00 04 00 04 <wire6> <bit3> <bit4> <bit5> …}` with byte 7 = `(mode>>3)&1` (`0xe3e2`), byte 8 = `(mode>>4)&1` (`0xe3fe`), byte 9 = `(mode>>5)&1` (`0xe41a`). Bytes 10–15 content is **unverified** (the 12-byte layout with a `clk_lock` byte is snd-bebob's, `bebob_maudio.c:183-198`, not the kext's). Envelope framing and company_id: C3. Permutation provenance: `clock-bijection.log:248..276`.

### 2.5 Legacy translator `SetClockSource @0x23858`
Callers passing a bare 2-bit enum (not gated by `0x40000000`) hit a translator first; only Internal-unmuted and External-digital (each with S/PDIF-port variant) are reachable (`clock-bijection.log:97..113`).

### 2.6 No parallel register path for the clock source
Rust `Fw1814ClkProtocol` deliberately omits `SamplingClockSourceOperation` (`special.rs:69-82`, B2a); FFADO has no clock-source wire protocol for these devices (B1 §4); macfw has no clock-source dropdown (B4). With A1-Q5 as the only source, the clock source is **HSCI-only**; the `0xFFC7:0x00700000+` window is the mixer's (C4). *Open (low priority):* confirm no `0xFFC7:0x0070????` write on a clock change (DTrace on write-quad probes during a cap-61-style click).

### 2.7 `SetClockSourceInternal(mode, blankSlate) @0xe25c` — sequence
1. Zero the 16-byte out/in buffers; pause the engine (`vt[0xa50]`); return if an update is already in progress (`[0x1b88]`).
2. `IOAudioEngine::beginConfigurationChange` (`vt[0xb00]`, `0xe2dc`); set the in-progress flag.
3. Validate the mode word (§2.2 gate); on failure `cancelConfigurationChange` (`vt[0xb10]`, `0xe30b`) and return `0xE00002C2`.
4. Build and send the 16-byte HSCI frame (§2.4) through `[+0x268]->vt[0x8b8]`.
5. `IOSleep(0x12c)` = **300 ms** (`0xe482`); store `fMode = mode`; call the parent: Rev1 `vtable[0xb70]` = slot `0xb60` = `FWAVCAudioDevice::SetClockSourceInternal @0x23148` (R2/Q5); `IOSleep(0x12c)` = **300 ms** (`0xe450`).
6. If `!blankSlate`: `SwitchDigitalSignals` (`vt[0xbe0]`, §2.3) and the settings-levels resend (`FWSettingsLevels` → mixer).
7. Clear the flag; end the configuration change; resume the engine.

`SetBlankSlateClockSource @0xe4f0` (cold-boot variant) passes `(fMode & 0x1c) + 0x40000003` (drops S/MUX, bit 5), then `IOSleep(0x9c4)` = **2500 ms**. **Observed (R21, SL v1.9, idle, `0x40000014 → 0x40000017 → 0x40000014`):** each change runs exactly two `IOSleep(300)` on the workloop thread; no 2500 ms sleep; the property reads back the new value ~0.324 s after the write; `MARateLocked` stayed 1. The sleeps are **fixed constants**, not a poll of the lock bit. That they exist *for* PLL/plug settling is **unverified** (no capture ties them to a hardware event).

---

## 3. PLL lock and `MARateLocked`

### 3.1 What sets the flag
External-sync state is read by the HSCI timer: `FWHSCI::HSCITimerFired @0x1d600` reads `0xFFC7:0x00600000` (`fRate>>2` quadlets) and re-arms after `50 + [+0xd4]` ms (`0x1d50d..0x1d513`, R6; 1 s on error). Completion runs on a new thread (`InvokeOnNewThread` @0x1d89e). `DetectExternalSyncChanges(bool externalSync, FDFValue) @0x2bc3` (A1-Q15d xref #1), called from `FW1814Device::ReadHSCIData @0xd952` with `externalSync = (packet word at byte offset 0x50 >> 24) & 1` and `fdf = (that word >> 16) & 7` (P4/R38):

```c
if (externalSync) bl = (fEngine && fEngine->field_0x228 == this->field_0x25c) ? 1 : 0;
if (bl != this->field_0x25a) { this->field_0x25a = bl; SetStaticPropertyAndNotifyUserClients("MARateLocked", bl); }
```
(`mafw.asm:2068..2081`.)

### 3.2 Write-only audit
Accesses to `[+0x25a]`: `0x2bab` `cmpb` (self-compare, change-detect guard), `0x2bb9` `movb %bl` (write, `DetectExternalSyncChanges`), `0x363c` `movb $1` (write, `initHardware` seed). **Two writers, zero functional readers** — no downstream gate on rate writes, routing, mixer or lifecycle (A1-Q15d).

### 3.3 Conclusion and clone rule
PLL lock is **Panel-UX cosmetic**: the kext broadcasts `MARateLocked` over the `FWKIPCUserClient` property channel (type-0, A1-Q14, Q-UC-surface) and the Panel greys out the rate selector while externally synced; a direct `setProperties({MASampleRate: X})` bypasses it (B5 §9, A1-Q15d). **Clone:** emit `MARateLocked` on each sync-change tick and once at `initHardware`; do not gate rate writes on it.

**Hazard (not a requirement):** v1.9 kernel-panics in `IOAudioFamily` buffer re-allocation when a rate change arrives mid-stream (B5 cap 65; memory `project_v19_rate_change_while_streaming_panics`). Behaviour of 1.11.3 is **unverified**; the clone gates the change while a client is streaming (C1 §3.9).

---

## 4. Rate-change propagation (A1-Q15c, slot names per R2)

| # | Call | Offset | Role |
|---|---|---|---|
| 1 | `IOAudioEngine::performFormatChange(stream, format, rate)` | IOKit | CoreAudio entry |
| 2 | `FWBaseEngine::performFormatChange` | `0x3e54` | grabs the command gate (`vt[0x688]` = `getWorkLoop`) |
| 3 | `runAction(ActionPerformFormatChange, …)` | `0x3e43` | serialises on the engine workloop |
| 4 | `jmp vt[0xc88]` → `FWEngine2::PerformFormatChange` | `0x124cc` | FW1814-concrete; first `pauseAudioEngine` (`vt[0xa50]`) |
| 5 | AC3 fork (`'cac3'`, flag `[+0x2d4]`) | asm 21037..21101 | passthrough forces 48 kHz (`_IOAudioSampleRate{0xBB80,0}`); A1-Q12 |
| 6 | `fDevice->vt[0xa78]` = `FWAudioDevice::RequestFormatUpdateForSampleRate(uint)` | `0x2a84` | no `ChangeRate` symbol exists |
| 7 | `GetDeviceChannelsForSampleRate` (`vt[0xa68]`) | asm 1991 | per-rate channel table |
| 8 | `FWBaseEngine::SetIOFormat(channels, rateIdx)` | `0x42aa` | engine format commit |
| 9 | `IOAudioEngine::beginConfigurationChange` (`vt[0xb00]`) unless AC3 | asm 3804 | begin |
| 10 | stash `[+0x2e0]` rate, `[+0x2ec]` frac, `[+0x2e4]` chan, `[+0x2f0]` | asm 3785+ | format cache |
| 11 | `fDevice->vt[0xaf8]` = `FW1814Device::ResetRearrangementProcs @0xcad4` | asm 3830 | **not** an HSCI emit |
| 12 | if the rate changed: `IOAudioEngine::setSampleRate` (`vt[0xb58]`) and `FWEngine2::PerformSampleRateChangeCore @0x117e6` (`vt[0xce0]`) | asm 3837/3841 | **not** an AV/C commit |
| 13 | `SetStreamFormat` on the output (`[+0x210]`) and input (`[+0x218]`) streams | asm 3785 tail | |
| 14 | slot `0xb10` at the end of `SetIOFormat` — resolves to `cancelConfigurationChange` (R2); `completeConfigurationChange` is `0xb08` | asm 3863 | **suspicious; which is intended is unverified** |
| 15 | `FWBaseEngine::PerformSampleRateChangeCore @0x479e`: stores `[+0x220]`, iterates the DCL programs (`[+0x178]`) calling `vt[0x128]` `ApplyRateChange`, then `ResetLatency` | asm 4138 | how the base routine at `0x479e` and the `FWEngine2` override at `0x117e6` call each other is **unverified** |
| 16 | `field_0x346 |= 0x1d` | asm 4194 | dirty flags |
| 17 | `resumeAudioEngine` (`vt[0xa58]`) | | |

The AV/C PLUG SIGNAL FORMAT frames are **not** in this chain: they are sent by `ConnectAudioMIDIChannels` at stream start (§7). **Clone rule:** every step runs on the engine workloop through the `runAction` gate (single-writer invariant on `fMode[+0x144]`, `[+0x2e0]`, `[+0x2e4]`, `[+0x228]`, `[+0x2d4]`).

---

## 5. Rate table, SYT interval and CIP constants

`SetupCIP(rateHz, SYTInterval, SYTIncrement, FDFindex) @0x27102` (caller `0x272f6`, switch on the sample rate in Hz; Q2, Q15e). `FDFValue` is the **enum 0..6** (`ChangeSampleRateToFDFValue @0x52e8`), not the hex rate (R5):

| Rate (Hz) | hex | FDF index | SYTInterval | SYTIncrement | `rateIdx` | round-trip frames† |
|---|---|---|---|---|---|---|
| 32000 | `0x7D00` | 0 | 8 | `0x1800` | 0 | 210 |
| 44100 | `0xAC44` | 1 (default) | 8 | `0x116A` | 1 | 210 |
| 48000 | `0xBB80` | 2 | 8 | `0x1000` | 2 | 225 |
| 88200 | `0x15888` | 3 | 16 | `0x116A` | 3 | 358 |
| 96000 | `0x17700` | 4 | 16 | `0x1000` | 4 | 378 |
| 176400 | `0x2B110` | 5 | 32 | `0x116A` | 5 | 642 |
| 192000 | `0x2EE00` | 6 | 32 | `0x1000` | 6 | 698 |

† A1-Q10 (static table). The earlier column "AM824 channels 8/8/8/16/16/32/32" was the SYT interval, not a channel count (`[+0x224]` = SYT interval, R5). **Real channel counts** depend on the digital mode and rate (kext `DeviceChannels` tables `0x363c0/0x364a0/0x36580/0x4ab80`; S/PDIF base in 10 / out 6; ADAT 44.1/48 in 16 / out 12; ADAT 88.2/96 in 12 / out 8 — R12); the full per-rate table is owned by C4.

Rates not in the table fall through to the 44.1 kHz path (`$0xac44` default at `0x2738b`). `+0x4a0` stores the **rate in Hz**, `+0x4dc/+0x4e0` = `0x3E80000/(rate·4) ∓ 2` (TX packets per bank `P`, Q-DCL).

**Per-FDF CIP accumulator constants** (jump table `0x272d8`, Q2/Q15e Correction): 32 kHz `{1,2,1,1}`; the 44.1 family (FDF 1/3/5) `{0xc7, 0x280, 0x153, 0x1b9}`; the 48 family (FDF 2/4/6) `{1,4,1,1}`. The divisor of the post-switch computation uses the SYT interval (`imul 0x4ac(%r14)` at `0x27290`).

**CIP header templates** (R5): header 1 = `((nodeID & 0x3f) << 24) | (([+0x64] + [+0x68]) << 16)` at `[+0x498]` (`0x27173..0x2718e`; DBS = audio + MIDI quadlets, no `0x3F0000` mask); header 2 = `0x90000000` at `[+0x49c]` (`0x27195`) is the CIP Q1 word (EOH/FMT), not "AM824 tag bits". Non-sync packets carry the NODATA SYT value (A1-Q2 `CalcIsochPacketHeaders`). CIP assembly belongs to C3/C4.

---

## 6. Cycle timer → CoreAudio counter

### 6.1 How the DCL rings feed sample time
TX = 20 segments (4 banks × (4 one-packet + 1 large segment of `P−4` packets that carries the callback) + 4 jumps; `BuildTransmittingDCLProgram @0x25ae4`); RX = 64 segments × 4 packets + 64 jumps (`BuildReceivingDCLProgram @0x25962`, packet count immediate `$0x4` at `0x25a99`) — Q-DCL-geometry-table. 4 packets = 4 isoch cycles ≈ 500 µs per RX segment (derive timing from this, not from "64 × ~1 ms").

Each segment is built with `AddTimestampedTransferPackets` (Q1a). **The end-of-segment interrupt comes from the CallProc node**: on 10.13.6 (`AppleFWOHCI` 5.5.9) `translateLegacyProgram @0x8d5e` folds a legacy `kDCLCallProcOp` into the preceding transfer DCL as its callback (`0x8f69`), and `SendDCL::compile @0xd53f..0xd60e` / `ReceiveDCL::compile @0xcce7..0xceac` OR the `0x300000` interrupt bits into the last descriptor only when a callback is present. `kDCLPtrTimeStampOp` only calls `setTimeStampPtr()` on the preceding DCL (`0x901d`); a timestamp pointer alone *suppresses* the interrupt bits (Q-OHCI-interrupt). So earlier statements that the timestamp node "makes OHCI raise the interrupt" or that "callbacks never fire without it" are **wrong**; the pre-Ghidra TX-silence root cause is therefore a hypothesis (e.g. missing CallProc).

**Open:** which function calls `IOAudioEngine::takeTimeStamp`, and from which callback. Check: xrefs of `takeTimeStamp` in the kext.

### 6.2 Rate-dependent offsets
B5 §6c (cap 50, Studio One 2 — `51-ps-logic-playback.txt:54` `studioapp`, despite the "logic" file name) first showed that `IOAudioEngineSampleOffset` moves with rate: 40→48 and `OutputSampleLatency` 216→231 on 44.1→48 kHz; `InputSampleLatency` 32. Capture 72 (SL v1.9, six rates at idle) extends this; the per-rate numbers and the formula status are owned by **C6 §1**. `FWBaseEngine::ResetLatency` republishes them. Per-rate formula: **open** (C6).

### 6.3 HSCI poll is not the cycle-timer source
The 50 ms HSCI poll (Q6) is the control-plane heartbeat for sync state; sample time comes from the DCL ring (§6.1). The two timebases are independent.

---

## 7. Stream start (what the old text called "stream kick")

The FW1814 stays silent after a bare CMP connect until the AV/C plug signal formats are written (third-party hardware finding, macfw `special-firmware-notes.md`; also snd-bebob `special_set_rate`, `bebob_maudio.c:312-337`). **The reference's order is not snd-bebob's** (R1; `ConnectAudioMIDIChannels @0x12156`, asm `0x121c9..0x122bb`, `[+0x348] = 1` set by `SetupAudioEngine @0xed73` for the FW1814):

1. start the TX DCL program (`vt[0x120]` on `[+0x350]`);
2. `IOSleep(500)`;
3. AV/C INPUT plug-signal-format `00 FF 19 00 90 <SFC> FF FF` (`AVCControlPlugSignalFormat`, `FWRev1AudioDevice @0x13734`, `vt[0xb50]`, `esi = 1`);
4. `IOSleep(500)`;
5. start the RX DCL program (`[+0x358]`);
6. AV/C OUTPUT plug-signal-format `00 FF 18 00 90 <SFC> FF FF` (`esi = 0`), no sleep after.

Plain 8-byte CONTROL commands (Rev1); the `0x2F` extended form is Rev2-only (Q3a). The only `IOSleep(100)` in the kext is the CMP lock retry in `updateP2PCount @0x24690`. **Runtime confirmation (capture 72, SL v1.9; call sites identical in 1.11.3):** at idle a rate change runs 3×`IOSleep(100)` → `IOSleep(500)` → `IOSleep(500)` → `IOSleep(100)` → ~0.9 s gap → `IOSleep(1000)` (`ResendCurrentMixerSettings`/`CompletePendingStateChangesAction`), ≈3.5 s; frame contents are not observable on that rig (`fbt` reaches only `mach_kernel`).

**Third-party hardware finding that does carry over:** the INPUT rate command fails while the host→device stream carries only NODATA packets and succeeds once timed data-bearing packets flow (macfw `special-firmware-notes.md` experiments 3–4, `first-duplex-capture-success.md`); the reference starts TX *before* sending INPUT `0x19`. **Unverified:** whether the 500 ms gaps are required by the device; why the device behaves this way (earlier "BeBoB CMP state machine" explanation and "empirical plug-state settle time" were unsourced). Check: timestamped FCP + isoch capture on the SL rig.

**Clone rule:** mirror the reference order and gaps (6 steps above). Do not use OUTPUT → 100 ms → INPUT.

---

## 8. Open questions

1. Consumer of `MAClockSource` **bit 2** (§2.3).
2. Direction of the bit-30 compare at the entry of `SetClockSourceInternal` (§2.2).
3. Bytes 10–15 of the 16-byte HSCI frame (§2.4) — SL FCP capture of a clock change.
4. `takeTimeStamp` call site (§6.1).
5. Per-rate `SampleOffset`/`OutputSampleLatency` formula (§6.2, C6).
6. Relationship between `FWEngine2::PerformSampleRateChangeCore @0x117e6` and `FWBaseEngine::PerformSampleRateChangeCore @0x479e`, and the slot-`0xb10` tail call (§4).
7. Rate change while streaming on 1.11.3 (§3.3).
8. Whether the 500 ms stream-start gaps are needed (§7).
9. Resolved and kept for traceability: HSCI byte 9 = S/MUX (`MAClockSource` bit 5), not `clk_lock` (`0xe41a..0xe425`); the `vt[0xce0]` byte layout question is moot (`vt[0xce0]` is not an AV/C commit); the 100 ms primitive question is answered by R1.

## 9. Clone-build implications (D2)

1. Clock-source control is `setProperties` on the device entry, not the user client (B5 §10).
2. Keep the full 32-bit `MAClockSource`; bit 30 is the entry gate; store the whole word in `fMode`.
3. Mirror the 16-byte HSCI frame with bytes 7/8/9 from bits 3/4/5; leave bytes 10–15 zero until the capture says otherwise (labelled unverified in code).
4. Mirror the fixed sleeps (2×300 ms in `SetClockSourceInternal`, 2500 ms in `SetBlankSlateClockSource`) and the stream-start order of §7. Treat their purpose as unverified; do not describe them as "device-measured".
5. Emit `MARateLocked` (sync-change ticks + once at init); never gate rate writes on it; the `cmpb 0x25a` change-detect guard stays.
6. Rate changes run on the engine workloop via `runAction`; gate them while streaming.
7. Implement the full 7-rate table of §5, the CIP templates and per-FDF accumulator constants bit-for-bit.
8. DCL rings: TX 20 / RX 64×4, callbacks raised by CallProc-derived callbacks; classic-DCL acceptance by 10.13.6 `AppleFWOHCI_DCLProgram::init` is unverified (BUILD-PLAN P1).
9. Republish `SampleOffset`/`OutputSampleLatency` per rate from the table in C6.
10. No register-poke path for the clock source; the `0xFFC7:0x00700000` window is the mixer's (C4).

## Cross-links
C1 (lifecycle; §3.7 start chain, §3.9 format change), C3 (HSCI envelope, FCP), C4 (DCL, CMP, channel tables), C5 (UC — not on the clock path), C6 (latency). A1-Q2/Q5/Q6/Q10/Q12/Q15b/Q15c/Q15d/Q15e, Q-DCL-geometry-table, Q-OHCI-interrupt, B1 §4, B2 §4.7, B2a, B5 §4/§6c/§9/§10, R1–R6, R8, R12, R21, R25; memory `project_maclocksource_is_composite_bitfield`.

## Change log (2026-10-14 corrections folded in)
Stream-start chain and sleep sites (R25) → §7; clock-change sleeps (R21) → §2.7; capture provenance (R16) → §6.2; channel counts per mode (R12) → §2.3/§5; stream kick, slot names, rate table/CIP, OHCI interrupt, ReconnectP2P (R1, R2, R5, R4, Q-OHCI-interrupt) → §2.3, §2.7, §4–§7; "device-measured PLL settle" and 100 ms claims removed (unsourced); DCL ring shape (Q-DCL) → §6.1; HSCI bytes 7/8/9 and 16-byte frame (R3, Q5) → §2.4; D1/D2/D3 wording removed from the clone section.
