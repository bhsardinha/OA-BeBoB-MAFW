# Q15c — Format-change chain: `performFormatChange` → `PerformFormatChange` → `RequestFormatUpdateForSampleRate` → `SetIOFormat` → `PerformSampleRateChangeCore`

> **Correction (2026-10-15, M-D / R45):** the input stream is `[+0x210]` and the output stream `[+0x218]` (`RecreateAudioStreams @0x4546`: direction 0 = output); the pending-state machine behind `CompletePendingStateChanges` and the `[+0x344]` bit meanings are decoded in [`Q-MD-engine-rate-chain.md`](Q-MD-engine-rate-chain.md) §1.6; `ResetLatency` also sets `setSampleOffset(GetSafetyOffsetForFDF)`.

> **Correction (2026-10-14) — Part A completion batch 1** (evidence: [PROVENANCE-RESOLUTIONS.md §R8](../PROVENANCE-RESOLUTIONS.md)). `[+0xa5]` is `IOAudioEngine::configurationChangeInProgress` (10.13.6 IOAudioFamily `beginConfigurationChange` writes it, `0xa3d8`), **not** an AC3 flag; AC3 is `[+0x2d4]`. Earlier 'fIsAC3 = 0xa5' text is wrong.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - **Slot names** (R2): `vt[0xce0]` = `FWEngine2::PerformSampleRateChangeCore @0x117e6` (not an AV/C commit); `vt[0xaf8]` = `FW1814Device::ResetRearrangementProcs @0xcad4` (not an HSCI emit); `vt[0xb00]` = `IOAudioEngine::beginConfigurationChange`; `vt[0xb10]` = `cancelConfigurationChange` (error path only, `0xe30b`); `vt[0xb58]` = `IOAudioEngine::setSampleRate`; `vt[0xbe0]` = `FW1814Device::SwitchDigitalSignals @0xe5dc` (not an AV/C stream kick); `vt[0xce8]` = `SetExternalSyncEnabled(bool) @0x11bc0`; `vt[0xa50]/[0xa58]` = `pauseAudioEngine/resumeAudioEngine`.
> - **AC3 flag** is `[+0x2d4]` (`EnableAC3Format @0x44ab`, `DisableAC3Format @0x59b7`). `[+0xa5]` read at `SetIOFormat 0x42c4` is an `IOAudioEngine`-base byte whose meaning is **unverified**.
> - **Rate table / CIP** (R5): the FDF argument is the enum 0..6 = 32/44.1/48/88.2/96/176.4/192 kHz (`ChangeSampleRateToFDFValue @0x52e8`), not the hex rate; `[+0x224]` = 8/16/32 is the SYT interval, not a channel count (real counts: kext `DeviceChannels` tables 0x363c0/0x364a0/0x36580/0x4ab80). CIP header 1 = `((nodeID&0x3f)<<24) | (([+0x64]+[+0x68])<<16)` @0x27173-0x2718e (DBS = audio+MIDI quadlets, no mask); header 2 = `0x90000000` @0x27195 is the CIP Q1 word (EOH/FMT), not 'AM824 tag bits'.


**Opened by:** B5 §13 (reclassified from B5 pending work on 2026-10-13).
**Scope:** decode what happens downstream of the `MASampleRate` fast-path siphon (B5 §13 / A1-Q15b) — AV/C writes, HSCI envelope, DCL reload, and latency/offset recompute. Firms up B5 §6c (per-rate latency table) and the Q15 batch-change claim that there is no separate `FWAVCAudioDevice::ChangeRate` symbol.

**Key correction (vs. Q15c prompt wording):** there is **no symbol named `FWAVCAudioDevice::ChangeRate`** in `_extracted/kext/`. The vtable slot the prompt referenced (`[+0xa78]`) resolves to `__ZN23m_audio_b_FWAudioDevice32RequestFormatUpdateForSampleRateEj` — i.e. `FWAudioDevice::RequestFormatUpdateForSampleRate(uint)`. All call sites in the kext use this slot to request a rate change on the device object.

## Chain overview

```
CoreAudio IOAudioEngine::performFormatChange(stream, format, rate)
  │
  ▼  (virtual, slot inherited from FWBaseEngine)
m_audio_b_FWBaseEngine::performFormatChange(stream, format, rate)      [asm 3434, 0x3e54]
  │  r12 ← this
  │  callq *0x688(this->vtable)                   → returns command gate / workloop object
  │  callq *0x1a0(gate->vtable)  (==runAction)    with trampoline = ActionPerformFormatChange
  │                                                arg1..4 = (this, stream, format, rate)
  │
  ▼  (on workloop thread, inside runAction)
ActionPerformFormatChange(engine, stream, format, rate)                [asm 3427, 0x3e43]
  │  jmpq *0xc88(engine->vtable)                  → subclass PerformFormatChange
  │
  ▼  (FWEngine2 example — matches FW1814)
com_m_audio_FWEngine2::PerformFormatChange(stream, format, rate)       [asm 21010, 0x124cc]
  │  callq *0xa50(this->vtable)                   → Acquire/Lock
  │  if (!format && !rate) return 0
  │  — AC3 fork decoded below —
  │  non-AC3 branch:
  │     rateVal = (rate ? rate->whole : this->field_0x220)
  │     callq *0xa78(fDevice->vtable)             → RequestFormatUpdateForSampleRate(rateVal)
  │  commit:
  │     callq *0xce0(this->vtable)                → engine commit (AV/C PLUG SIGNAL FORMAT write)
  │  callq FWBaseEngine::CompletePendingStateChanges()
  │  callq *0xa58(this->vtable)                   → Release/Unlock
  │
  ▼
m_audio_b_FWAudioDevice::RequestFormatUpdateForSampleRate(rateVal)     [asm 1981, 0x2a84]
  │  callq *0xa68(this->vtable, rateVal)          → GetDeviceChannelsForSampleRate(rateVal) → DeviceChannels*
  │  table = channels->field_0x38                  → per-rate code table
  │  loop: find rateIdx where table[rateIdx] == rateVal (else rateIdx=0)
  │  jmp FWBaseEngine::SetIOFormat(channels, rateIdx)
  │
  ▼
m_audio_b_FWBaseEngine::SetIOFormat(channels, rateIdx)                 [asm 3785, 0x42aa]
  │  fIsAC3 = this->field_0xa5
  │  if (!fIsAC3) callq *0xb00(this->vtable)      → BeginFormatChange / StopStreams
  │  stash: this->fRate@0x2e0 = channels.whole@0x8
  │         this->fFrac@0x2ec = channels.frac@0xc
  │         this->fChan@0x2e4 = channels.chan@0x0
  │         this->f???@0x2f0 = channels.?@0x4
  │  if (fDevice@0x138) callq *0xaf8(fDevice->vtable, this)   → device UpdateFormatCache / HSCI envelope
  │  if (newRate != cachedRate@0x8c) {
  │     callq *0xb58(this->vtable, &channels.table[rateIdx]) → ApplyRateToIsoch
  │     callq *0xce0(this->vtable, &channels.table[rateIdx]) → engine commit (AV/C PLUG SIGNAL FORMAT)
  │  }
  │  SetStreamFormat(outputStream@0x210, channels, rateIdx)
  │  SetStreamFormat(inputStream@0x218, channels, rateIdx)
  │  if (!fIsAC3) callq *0xb10(this->vtable)      → EndFormatChange / StartStreams
  │  propagate clip offsets: channels.{+0x18,+0x20,+0x28} → this->{+0x320,+0x328,+0x330}
```

The **DCL reload + latency recompute** happens one level further out, when CoreAudio also hands down a new sample-rate struct via `performSampleRateChange`:

```
m_audio_b_FWBaseEngine::PerformSampleRateChangeCore(const _IOAudioSampleRate*)   [asm 4138, 0x479e]
  │  this->field_0x220 = rate->whole
  │  iterate DCL programs at this->field_0x178 (OSCollection):
  │     per rate (switch on rate->whole):
  │        32000  (0x7D00) : fChannelsPerRate@0x224 = 8,  fRateIdx@0x228 = 0
  │        44100  (0xAC44) :                        8                    = 1  (default)
  │        48000  (0xBB80) :                        8                    = 2
  │        88200  (0x15888):                        16                   = 3
  │        96000  (0x17700):                        16                   = 4
  │        176400 (0x2B110):                        32                   = 5
  │        192000 (0x2EE00):                        32                   = 6
  │     for each program: callq *0x128(program->vtable)    → DCL ApplyRateChange()
  │  callq FWBaseEngine::ResetLatency()
  │  this->field_0x346 |= 0x1d                             → dirty flags (rate|chan|latency|?)
  │  return 1
```

## AC3 fork inside `FWEngine2::PerformFormatChange`

Decoded from asm lines 21037–21101 (`0x124e7..0x125ff`):

```c
uint32_t fmtTag = format ? format->fSampleFormat : 0;
bool enteringAC3 = (fmtTag == 'cac3') && (this->field_0x2d4 == 0);
bool leavingAC3  = (fmtTag != 'cac3') && (this->field_0x2d4 != 0);

if (!enteringAC3) {
    if (this->field_0x2d4 == 0) {                // currently PCM
        uint32_t r = rate ? rate->whole : this->field_0x220;
        fDevice->vtable[0xa78/8](fDevice, r);   // RequestFormatUpdateForSampleRate
    }
}

_IOAudioSampleRate ac3Rate = { .whole = 0xBB80 /* 48000 */, .fraction = 0 };
_IOAudioSampleRate *committed;

if (enteringAC3) {
    m_audio_b_FWBaseEngine::EnableAC3Format(this);
    committed = rate ? rate : &ac3Rate;
} else if (leavingAC3) {
    m_audio_b_FWBaseEngine::DisableAC3Format(this);
    committed = rate;
} else {
    committed = rate;
}

if (committed) {
    if (committed->whole == this->field_0x220 && !leavingAC3) {
        // no actual change; skip commit
    } else {
        _IOAudioSampleRate tmp = { .whole = this->field_0x220, .fraction = 0 };
        _IOAudioSampleRate *arg = committed ? committed : &tmp;
        this->vtable[0xce0/8](this, arg);        // commit — issues AV/C PLUG SIGNAL FORMAT
    }
}

m_audio_b_FWBaseEngine::CompletePendingStateChanges(this);
```

`'cac3'` is the 4CC `0x63 0x61 0x63 0x33` (little-endian memory layout → `cmpl $0x63616333`), matching `kIOAudioStreamSampleFormatAC3` from the IOKit `_IOAudioStreamFormat` struct — see A1-Q12 for the AC3 passthrough path.

## Latency recompute entry

`SetIOFormat` and `PerformSampleRateChangeCore` both call `FWBaseEngine::ResetLatency()`. The per-rate latency numbers themselves are the FW1814 table decoded in **A1-Q10**:

| rate (Hz) | 32000 | 44100 | 48000 | 88200 | 96000 | 176400 | 192000 |
|---|---|---|---|---|---|---|---|
| FDF | 0x7D00 | 0xAC44 | 0xBB80 | 0x15888 | 0x17700 | 0x2B110 | 0x2EE00 |
| round-trip (frames) | 210 | 210 | 225 | 358 | 378 | 642 | 698 |
| channels | 8 | 8 | 8 | 16 | 16 | 32 | 32 |
| rateIdx | 0 | 1 (default) | 2 | 3 | 4 | 5 | 6 |

The channel count and rateIdx columns are new facts from `PerformSampleRateChangeCore` (asm 4138–4194); the latency numbers come from `GetRoundTripLatencyForFDF` in Q10.

## Vtable slot inventory used by the chain

| slot (byte offset) | class whose vtable it indexes | what it does | provenance |
|---|---|---|---|
| `[+0x688]` | FWBaseEngine | returns command gate / workloop | asm 3448 |
| `[+0x1a0]` | IOCommandGate | `runAction` (standard IOKit) | asm 3457 |
| `[+0xc88]` | FWBaseEngine (overridden by subclasses) | subclass `PerformFormatChange` dispatch | asm 3431 |
| `[+0xa50]` | FWBaseEngine | Acquire/Lock | asm 21025, 5002 |
| `[+0xa58]` | FWBaseEngine | Release/Unlock | asm 21106, 5027 |
| `[+0xa68]` | FWAudioDevice (overridden by FW1814Device/Audiophile/…) | `GetDeviceChannelsForSampleRate(uint)` → `DeviceChannels*` | asm 1991 |
| `[+0xa78]` | FWAudioDevice | `RequestFormatUpdateForSampleRate(uint)` | asm 2008, 5010, 21064 |
| `[+0xaf8]` | FWAudioDevice | device-side format cache update + HSCI envelope emit | asm 3830 |
| `[+0xb00]` | FWBaseEngine | `BeginFormatChange` / `StopStreams` | asm 3804, 5005 |
| `[+0xb10]` | FWBaseEngine | `EndFormatChange` / `StartStreams` | asm 3863 |
| `[+0xb58]` | FWBaseEngine | apply rate to isoch stack | asm 3837, 5296 |
| `[+0xce0]` | FWBaseEngine (overridden by subclass) | **commit new rate — AV/C PLUG SIGNAL FORMAT write** | asm 3841, 21098, 5011-equiv |
| `[+0x128]` | FWDCLProgram | per-DCL `ApplyRateChange()` | asm 4197, 4232 |

## Observations and clone implications

1. **No separate `ChangeRate` symbol — rename this concept.** In the clone and in Part C synthesis, the rate-change method on the device object is `RequestFormatUpdateForSampleRate(uint)`. Clone code should match the name.
2. **Format-change always runs on the workloop.** The `IOCommandGate::runAction` dance in `FWBaseEngine::performFormatChange` means the real work happens on the engine workloop, not the CoreAudio caller. The clone must preserve this (same gate pattern) to keep the single-writer invariant on the format/rate caches at `[+0x220/+0x224/+0x228/+0x2e0..+0x2f0]`.
3. **Rate-cache fields on the engine.** `[+0x220]` = fRate (Hz), `[+0x224]` = per-rate channel count, `[+0x228]` = rate index 0..6, `[+0x2d4]` = AC3-mode bool, `[+0x2e0..0x2f0]` = incoming-format cache (whole/frac/chan). These are the state the Panel + Dice-shim mirror into Panel `MA*` properties.
4. **DCL reload is per-program, synchronous.** `PerformSampleRateChangeCore` iterates the `[+0x178]` DCL-program collection and calls `vt[0x128]` on each — this is the DCL rate-change notify. Our clone's DCL programs must expose a matching entry (TX, RX, and the TX-silence idle-lock pre-fill).
5. **Latency reset is unconditional on rate change.** `ResetLatency()` is called after every DCL walk — the clone must publish updated latency to CoreAudio (`setLatency(fFrames)` on the IOAudioEngine) via the same path.
6. **`vt[0xce0]` is where the AV/C sample-rate command goes out.** This is the subclass-overridden commit slot. The actual AV/C bytes (`OUTPUT PLUG SIGNAL FORMAT` on plug 0, FDF selector) are documented in A1-Q16 §SyncStream + A1-Q2 §FDF table.
7. **`vt[0xaf8]` is the HSCI envelope emitter.** Each format change pokes the FDF value into the device's own HSCI mailbox (A1-Q5 clock-source envelope / A1-Q6 50 ms polling) so the device's control plane sees the same rate the audio plane just adopted.

## Dependencies and follow-ups

- Confirms **B5 §6c latency numbers** by anchoring them to `PerformSampleRateChangeCore` (channel count derivation) + `GetRoundTripLatencyForFDF` (per-rate frame count). B5 §6c now has provenance; closable in Part C.
- Confirms **B5 §13** reclassification: the fast-path siphon in `FWAVCAudioDevice::setProperties("MASampleRate", …)` (A1-Q15b batch-1) tail-calls `RequestFormatUpdateForSampleRate` on the device, hitting exactly the chain above.
- **Deferred:** decoding `vt[0xce0]` subclass override on `FWEngine2` to extract the exact AV/C vendor-opcode bytes for the rate commit. A1-Q16 covered the AV/C `OUTPUT PLUG SIGNAL FORMAT` layout at the bus level; the engine-side commit buffer construction is the only remaining black box in the chain. Open as **Q15e** if Part C needs the exact byte layout.

## Cross-links

- A1-Q2 — FDF rate table (SYT interval + FDF values)
- A1-Q5 — HSCI envelope layout
- A1-Q6 — HSCI 50 ms poll (invokes `DetectExternalSyncChanges` which guards `MARateLocked`)
- A1-Q10 — `GetRoundTripLatencyForFDF` per-rate numbers
- A1-Q12 — AC3 passthrough path (counterpart to the `'cac3'` fork in `FWEngine2::PerformFormatChange`)
- A1-Q13 — engine family (FWBaseEngine → FWEngine2/Pair/Aggregate/TimeSlave override `vt[0xc88]`)
- A1-Q15 / Q15b — mixer typed-property model + `setProperties` dispatch (fast-paths `MASampleRate` to this chain)
- A1-Q15d — `MARateLocked` is write-only; the Panel-side rate-lock is not enforced here
- A1-Q16 — AV/C `OUTPUT PLUG SIGNAL FORMAT` sync-stream + plug map
- B5 §6c, §9, §13 — hardware observations that this chain explains
