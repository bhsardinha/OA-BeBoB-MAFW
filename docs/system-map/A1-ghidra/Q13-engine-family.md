# Q13 — Engine family inheritance

> **Correction (2026-10-15, Q-MD2):** the `_RearrangeSampleFrameLLRR6` / `ConvertLLRR*` set shown around lines 195-217 belongs to the Audiophile-class devices. The FW1814 uses the LRLR rearrange procs and the stereo-pair / 1814-delay input converters (`FW1814Device::ResetRearrangementProcs` @0xcad4). The clip/convert entry chain (`FWDiscreteEngine::clipOutputSamples` → `ClipSubEngineOutput` → `vt[0xc38]` → `vt[0xc90]` → `vt[0xca0]`) is decoded in `Q-MD2-sample-path.md`.


> **Correction (2026-10-15, M-D / R45):** `FWBaseEngine` derives from `FWDiscreteEngine` (ctor @0x633a), `FWEngine2` from `FWBaseEngine`; in `SetupAudioEngine` the outputs are 6/12 by mode bit 4 and the inputs 10/16 by bit 3 (the inline comments swap them); engine members `[+0x2e0]`=out, `[+0x2e4]`=in, `[+0x2ec]`=out-stream, `[+0x2f0]`=in-stream. See [`Q-MD-engine-rate-chain.md`](Q-MD-engine-rate-chain.md).

> **Correction (2026-10-14) — live engine census** (evidence: [PROVENANCE-RESOLUTIONS.md §R17](../PROVENANCE-RESOLUTIONS.md)). Live SL v1.9: with `MAEnableStereoPairEngines = 0` in the saved prefs (`defaults read com.m-audio.device-settings.firewire`) the only M-Audio engine in `ioreg` is `com_m_audio_FWEngine2` (`IOAudioEngineDescription = "FireWire 1814 Multichannel"`, flavor 1); the `DevShortName*` engines in the same dump belong to AppleHDA. How many `FWEngine1` pair engines appear when the option is on is **still unverified** (needs the option toggled on the rig while idle).


**Status:** done.

**Scope:** which base class does `com_m_audio_FWEngine2` inherit from, and what are `FWDiscreteEngine` / `FWPairEngine` / `FWAggregateEngine` / `FWTimeSlaveEngine` for?

## Answer in one line

**`FWEngine2` inherits DIRECTLY from `FWBaseEngine`.** The Discrete / Pair / Aggregate / TimeSlave engines are **sibling classes** — secondary IOAudioEngines that CoreAudio sees as independent audio devices, delegating per-channel-pair clip/convert back into the primary FWEngine2 via a pointer at `[+0x118]`. The FW1814 publishes one primary engine (FWEngine2) plus N secondary pair/discrete engines so applications can grab a single stereo pair as its own CoreAudio device.

## Evidence

### FWEngine2 constructor @ 0x12762

```c
void com_m_audio_FWEngine2::com_m_audio_FWEngine2(FWEngine2 *this, OSMetaClass *mc) {
    m_audio_b_FWBaseEngine::m_audio_b_FWBaseEngine((FWBaseEngine*)this, mc);
    *(void***)this = &PTR__com_m_audio_FWEngine2_000391d0;   // vtable
}
```

Direct call to `FWBaseEngine::FWBaseEngine` — no intermediate class. Vtable at `0x391d0`.

### FWEngine2::MetaClass::alloc @ 0x11a18

```c
FWBaseEngine *com_m_audio_FWEngine2::MetaClass::alloc(void) {
    FWBaseEngine *p = OSObject::operator_new(size=0x330);
    if (p) {
        FWBaseEngine::FWBaseEngine(p, &FWEngine2::gMetaClass);
        *(void***)p = &PTR__com_m_audio_FWEngine2_000391d0;
        OSMetaClass::instanceConstructed();
    }
    return p;
}
```

Return type is `FWBaseEngine*` — single inheritance confirmed. Allocation size `0x330` ≥ the field offsets we see used (`[+0x350]`, `[+0x358]` for DCL out/in programs). Checked: initHardware writes `[+0x350] = outputDCL`, `[+0x358] = inputDCL`, so the real FWEngine2 size is **~0x360 bytes**. Alloc size `0x330` and the +0x350/+0x358 writes don't reconcile unless Ghidra misread the immediate — worth re-checking at the raw bytes, but the inheritance edge is still crystal clear.

### FWPairEngine::performAudioEngineStart @ 0x1ae44

```c
void FWPairEngine::performAudioEngineStart(FWPairEngine *this) {
    FWDiscreteEngine::ActivateSubEngine(
        *(FWDiscreteEngine **)(this + 0x118),
        (uint)(1L << (this[300 /*=0x12c*/] & 0x3f))
    );
}
```

A `FWPairEngine` holds a pointer to a **`FWDiscreteEngine` at `[+0x118]`** and a channel-pair bitmask index at `[+0x12c]`. Starting a pair engine = calling `ActivateSubEngine` with a single-bit mask on the shared discrete engine.

### FWPairEngine::clipOutputSamples @ 0x1af04

```c
void FWPairEngine::clipOutputSamples(FWPairEngine *this, void *src, void *dst,
                                      uint firstFrame, uint numFrames,
                                      IOAudioStreamFormat *fmt, IOAudioStream *stream) {
    FWDiscreteEngine::ClipSubEngineOutput(
        *(FWDiscreteEngine **)(this + 0x118),
        (uint)(1L << (this[300] & 0x3f)),
        firstFrame, numFrames, fmt, stream);
}
```

Same pattern: a pair engine's `clipOutputSamples` is a 1-line forward to the shared discrete engine with its own channel-pair bitmask.

### FWDiscreteEngine::clipOutputSamples @ 0x188c6

```c
void FWDiscreteEngine::clipOutputSamples(FWDiscreteEngine *this, void *src, void *dst,
                                          uint firstFrame, uint numFrames,
                                          IOAudioStreamFormat *fmt, IOAudioStream *stream) {
    ClipSubEngineOutput(this, 1 /*mask*/, firstFrame, numFrames, fmt, stream);
}
```

The Discrete engine's own clip is `ClipSubEngineOutput(this, 1, ...)` — mask=1 means "just my first channel pair." The free-function `ClipSubEngineOutput` is the shared implementation.

## Architecture summary — revised view

```
m_audio_b_FWBaseEngine (abstract, 72 methods — IOAudioEngine base)
├── com_m_audio_FWEngine2         ← primary engine (DCL, format change, clock, MIDI, AC3)
│                                   FW1814 and Audiophile both use this directly.
├── m_audio_b_FWDiscreteEngine    ← secondary — "all discrete pairs" style engine
├── m_audio_b_FWPairEngine        ← secondary — a single-pair proxy that forwards to
│                                   a shared FWDiscreteEngine via [+0x118]
├── m_audio_b_FWAggregateEngine   ← secondary — aggregate-device membership
└── m_audio_b_FWTimeSlaveEngine   ← secondary — slave-clock member of an aggregate
```

**Sub-engine pattern:**
- `FWPairEngine` is a lightweight proxy, 21 methods — all operations delegate to a shared `FWDiscreteEngine` at `[+0x118]` plus a per-pair bitmask index at `[+0x12c]`.
- CoreAudio sees each pair as a separate IOAudioEngine → separate AMS device, independently selectable.
- The real audio work (clip, convert) happens in the shared `FWDiscreteEngine::ClipSubEngineOutput` / `ConvertSubEngineInput` (free functions nearby).
- Previous overview claim that "FWEngine2 inherits from Discrete/Pair/TimeSlave" is **wrong** — corrected here.

## What CoreAudio actually sees for the FW1814

Three published audio engines (hypothesis, needs runtime confirmation on the SL Mac):

1. **Primary FWEngine2** — "M-Audio FireWire 1814" multichannel device (10/16 in × 6/12 out, up to 192 kHz, MIDI 1×1). The DCL/isoch/AV/C transport is on this one.
2. **Pair/discrete sub-engines** — one per analog-pair, maybe one per S/PDIF, one per ADAT pair-group. These show up in AMS as "FW1814 Pair 1", etc. Each is a FWPairEngine → shared FWDiscreteEngine → shared transport.
3. **Aggregate engine** — only if the panel is configured to join multiple devices (unlikely for a single FW1814; this is for Audiophile+FW1814 scenarios).

Needs confirmation: run `ioreg -l -c IOAudioEngine` on the SL reference Mac with FW1814 attached and count instances.

## FW1814 `SetupAudioEngine` @ 0xed34 — confirms Q3 at the Ghidra level

```c
void FW1814Device::SetupAudioEngine(FW1814Device *this, FWBaseEngine *eng) {
    FWBaseEngine::SetMIDIInputPorts(eng,  (FWAudioDevice*)this, /*n*/1, NULL);
    FWBaseEngine::SetMIDIOutputPorts(eng, (FWAudioDevice*)this, /*n*/1, NULL);
    eng[0x2d5] = 1;    // enable flag A
    eng[0x2d6] = 1;    // enable flag B
    eng[0x348] = 1;    // enable flag C (FW1814-only)
    *(u32*)(eng + 0x2e8) = 1;              // MIDI channel count

    if ((this[0x144] & 0x10) == 0) {       // ADAT-in disabled → 6-in
        *(u32*)(eng + 0x2ec) = 6;          //   stream in channels
        *(u32*)(eng + 0x2e0) = 6;          //   active in channels
    } else {                               // ADAT-in enabled → 12-in
        *(u32*)(eng + 0x2ec) = 12;
        *(u32*)(eng + 0x2e0) = 12;
    }
    if ((this[0x144] & 0x08) == 0) {       // ADAT-out disabled → 10-out
        *(u32*)(eng + 0x2f0) = 10;
        *(u32*)(eng + 0x2e4) = 10;
    } else {                               // ADAT-out enabled → 16-out
        *(u32*)(eng + 0x2f0) = 0x10;
        *(u32*)(eng + 0x2e4) = 0x10;
    }
    // Jumptable @ 0xee00 — Ghidra couldn't recover.
    // Last instruction: (*this->vtable[0xaf8])(this, eng);
    //   — virtual dispatch to SetupAudioEngine on the FWRev2AudioDevice/FWAVCAudioDevice base
    //     to install the convert/clip procs and sample-peak proc.
}
```

**Field map for `com_m_audio_FWEngine2` (so far):**

| Offset | Field | Set by |
|---|---|---|
| `+0x118` | sub-engine → primary pointer (on Pair/Aggregate/TimeSlave engines, not primary) | engine ctor |
| `+0x138` | `fAVCDevice` (FWAVCAudioDevice *) | engine init |
| `+0x12c` (300) | pair-engine channel-pair index (0-based) | SetupAudioEngine |
| `+0x224` | fRate (FDF rate index 0..6) | PerformSampleRateChange |
| `+0x240` | FWDCLSharedSyncData (passed to DCLProgram::Initialize) | initHardware |
| `+0x2d5` | enable flag A | SetupAudioEngine |
| `+0x2d6` | enable flag B | SetupAudioEngine |
| `+0x2d8` | fOutBufferSize (bytes) | PerformFormatChange |
| `+0x2e0` | fInputChannels (active, 6 or 12) | SetupAudioEngine |
| `+0x2e4` | fOutputChannels (active, 10 or 16) | SetupAudioEngine |
| `+0x2e8` | fNumMIDIChannels (1) | SetupAudioEngine |
| `+0x2ec` | fInStreamChannels | SetupAudioEngine |
| `+0x2f0` | fOutStreamChannels | SetupAudioEngine |
| `+0x300` | rearrange-sample-frame proc (output interleave) | SetupAudioEngine (base) |
| `+0x308` | sample-peaks proc | SetupAudioEngine (base) |
| `+0x310` | convert-input proc | SetupAudioEngine (base), replaced by Workaround (Q9) |
| `+0x318` | fAudioDevice backref | SetupAudioEngine (base) |
| `+0x348` | FW1814-only enable flag | SetupAudioEngine |
| `+0x350` | fOutputDCLProgram | initHardware |
| `+0x358` | fInputDCLProgram | initHardware |

### FW1814 device-side bit map

`com_m_audio_FW1814Device::this[0x144]` holds the panel-driven mode flags:

| Bit | Mask | Meaning |
|---|---|---|
| 3 | `0x08` | ADAT output enabled → 16 output channels; else 10 |
| 4 | `0x10` | ADAT input enabled → 12 input channels; else 6 |

Clone implication: the FW1814Device must expose two bool properties ("ADAT In", "ADAT Out") that write bits 3 and 4 of a single mode byte at `[+0x144]`; `SetupAudioEngine` is called before engine start to pick up the counts. Matches 2014-M-Audio-panel UX.

## Audiophile SetupAudioEngine @ 0xbb9a — reference contrast

```c
void FWAudiophileDevice::SetupAudioEngine(FWAudiophileDevice *this, FWBaseEngine *eng) {
    *(u32*)(eng + 0x2e8) = 1;              // MIDI channels = 1
    FWBaseEngine::SetMIDIInputPorts(eng,  this, 1, NULL);
    FWBaseEngine::SetMIDIOutputPorts(eng, this, 1, NULL);
    *(u32*)(eng + 0x2e4) = 4;              // input channels = 4
    *(u32*)(eng + 0x2e0) = 6;              // output channels = 6
    *(u32*)(eng + 0x2f0) = 4;              // in stream channels
    *(u32*)(eng + 0x2ec) = 6;              // out stream channels
    *(void**)(eng + 0x310) = _ConvertLLRR4SkipMIDI1;        // L/L/R/R + skip 1 MIDI
    *(void**)(eng + 0x318) = this;
    *(void**)(eng + 0x300) = _RearrangeSampleFrameLLRR6;    // 6-channel LLRR layout
    *(void**)(eng + 0x308) = _SamplePeaks6;
}
```

Audiophile is static: always 4-in / 6-out, with specific LLRR rearrange procs. No mode bits. Confirms Q3's "Audiophile has no ADAT config" assumption.

The named procs are important:
- `_ConvertLLRR4SkipMIDI1` — 4-channel LLRR float→int32 convert that skips 1 MIDI quadlet. Confirms the AM824 MIDI-quadlet layout at tail of each frame.
- `_RearrangeSampleFrameLLRR6` — 6-channel LLRR interleave.
- `_SamplePeaks6` — 6-channel peak computation (feeds the panel's level meters).

The FW1814 doesn't set these in its own `SetupAudioEngine` — it calls `this->vtable[0xaf8](this, eng)` as the last line (truncated jumptable at 0xee00), which is the base-class / FWAVCAudioDevice virtual that picks the proper convert/rearrange proc for the current channel count. Then `Q9::WorkaroundFor1814InputDelay` replaces `[+0x310]` when `fInStreamChannels != 2` (confirms Q9).

## Clone implications

1. **Engine class split is optional but desirable:**
   - For M3 (one engine, PCM only), the clone's single `FW1814Engine` is fine — it maps to the reference's `FWEngine2`.
   - For feature parity with AMS "pair devices" view, M5+ should add a `FW1814PairEngine` secondary that forwards to the primary. Not blocking for first-light.
2. **The `[+0x144]` mode byte on the device is the UI contract.** Mirror it exactly — our `FW1814Device` should hold a single `uint8_t fMode` and the panel toggles bits 3/4 via a UserClient method. No separate "AdatInEnabled" / "AdatOutEnabled" booleans.
3. **The convert/rearrange procs are the vocabulary for AM824 packing.** Our clone's `FW1814ChannelMap.h` must enumerate at least:
   - `ConvertLLRR<N>SkipMIDI<M>` for N ∈ {4, 10, 16}
   - `RearrangeSampleFrameLLRR<N>` for N ∈ {6, 10, 16}
   - `SamplePeaks<N>` for N ∈ {6, 10, 16}
   Even if initial implementations are stubs, the switch at base-class SetupAudioEngine is where they get installed.
4. **The field offsets `[+0x2d5..0x358]` are the engine's state surface.** If we're ever going to patch the reference binary to make it 10.13-safe, we must know this field map intimately — it's where the panel's UserClient reads and writes.

## Follow-ups

- **Q13a:** decode the `(this->vtable[0xaf8])(this, eng)` tail in FW1814::SetupAudioEngine — exactly which vtable slot is this on FWAVCAudioDevice/FWRev2AudioDevice? Determines which convert/rearrange procs are installed per channel count.
- **Q13b:** `FWDiscreteEngine::ClipSubEngineOutput` and `ConvertSubEngineInput` — the shared sub-engine clip/convert. Decoding gives us the pair-selection + stride logic.
- Observed on SL reference Mac: `ioreg -l -c IOAudioEngine | grep -c IOAudioEngine` to confirm the primary + N sub-engine count hypothesis.

## Provenance

- `com_m_audio_FWEngine2::com_m_audio_FWEngine2` @ `0x12762` — direct call to `FWBaseEngine::FWBaseEngine`. Decompiler output in `docs/system-map/A1-ghidra/ghidra-exports/decomp/q13.regenerated-2026-10-14.out`.
- `com_m_audio_FWEngine2::MetaClass::alloc` @ `0x11a18` — allocates `FWBaseEngine *`.
- `com_m_audio_FWPairEngine::performAudioEngineStart` @ `0x1ae44` and `::clipOutputSamples` @ `0x1af04` — show the `[+0x118]` sub-engine pointer + `[+0x12c]` bitmask pattern.
- `com_m_audio_FW1814Device::SetupAudioEngine` @ `0xed34` — `[+0x144]` bits 3/4 mode-byte decoded.
- `com_m_audio_FWAudiophileDevice::SetupAudioEngine` @ `0xbb9a` — static 4/6/LLRR procs.
- Vtable address `PTR__com_m_audio_FWEngine2_000391d0`.
- Date: 2026-10-06, Ghidra 12.1.4 headless decompile output.
