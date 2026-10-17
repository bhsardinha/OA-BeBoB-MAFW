# Q9 — `FW1814Device::WorkaroundFor1814InputDelay` — the named workaround

> **Correction (2026-10-15, Q-MD2 `Q-MD2-sample-path.md` §1 items 8–9):** the pseudocode below is wrong in these points, all re-read from `FW1814Device__WorkaroundFor1814InputDelay.c`, `FW1814Device__ResetRearrangementProcs.c` and the constructor @0xf04a: (a) `[+0x318]` for the workaround is the **device pointer**, not the stream channel count; (b) the install tests `[+0x2e4]` (input channels) `== 2`; (c) the ring is **16 frames × 6 raw quadlets** (`[+0x1bcc]`, identity index table `i*6` written by the constructor, counter `[+0x1d4c]`), and only **six** of the first eight channels are delayed (by 16 frames), channels 0 and 4 pass through, and the output order is `0,4, old[0,3,1,4,2,5]` — it is not a one-frame FIFO; (d) there is no "advance 15 ints to skip the CIP header" (the source already points at the data blocks; source stride is `channels+1`). The "per-channel one-frame-late" explanation in the summary line is **unverified**.


**Status:** done.

**Scope:** `__ZN24com_m_audio_FW1814Device27WorkaroundFor1814InputDelayEjPiS0_Pv` @ 0xcba4 (docs/system-map/A1-ghidra/otool-exports/mafw.asm:14471); installer in `ResetRearrangementProcs` @ 0xcad4.

## Answer in one line

**The FW1814 ships each RX sample one host-frame late relative to its SYT timestamp.** The workaround is a per-channel 16-entry delay ring inside the device object (`[+0x1b8c..0x1be0]`) that reads the current frame from the DCL buffer, stores it in the ring, and emits the *previous* frame's samples to the CoreAudio buffer — a one-frame-of-host-latency FIFO that re-aligns the stream. **In 10/12-channel modes this function replaces the stock `ConvertInputFramesOneStereoPairSkipMIDI` at engine `[+0x310]`.**

## Install site — `ResetRearrangementProcs(FWBaseEngine *e)` @ 0xcad4

```c
void FW1814Device::ResetRearrangementProcs(FWBaseEngine *e) {
    // Pick a per-rate "rearrange" proc for TX based on fOutStreamChannels [+0x2e0]
    switch (e->fOutStreamChannels - 4) {
        case 0:  e->[+0x300] = RearrangeSampleFrameLRLR4;          break;
        case 2:  e->[+0x300] = RearrangeSampleFrameLRLR6SkipMIDI;  break;
        case 4:  e->[+0x300] = RearrangeSampleFrameLRLR8SkipMIDI;  break;
        case 6:  e->[+0x300] = RearrangeSampleFrameLRLR10SkipMIDI; break;
        case 8:  e->[+0x300] = RearrangeSampleFrameLRLR12SkipMIDI; break;
        case 10: e->[+0x300] = RearrangeSampleFrameLRLR14SkipMIDI; break;
        default: e->[+0x300] = NULL;
    }
    // Pick RX convert proc:
    if (e->fInStreamChannels /* [+0x2e4] */ == 2) {
        e->[+0x310] = ConvertInputFramesOneStereoPairSkipMIDI;
        e->[+0x318] = 2;                                      // stride marker
    } else {
        e->[+0x310] = &WorkaroundFor1814InputDelay;           // the workaround
        e->[+0x318] = e->fInStreamChannels;                   // real stride
    }
}
```

So the workaround is applied **whenever the device has more than 2 input stream channels** — i.e. in every normal FW1814 operating mode (6-in / 10-in / 12-in). Only a hypothetical "stereo-only" degraded mode would skip it. The Audiophile sibling (6-in minimum) always inherits the stock `ConvertLLRR4SkipMIDI1` because its `SetupAudioEngine` doesn't call this override.

## `WorkaroundFor1814InputDelay(uint32_t numFrames, int *src, int *dst, FWBaseEngine *e)` — pseudocode

```c
void FW1814Device::WorkaroundFor1814InputDelay(
    uint32_t numFrames, int *src, int *dst, FWBaseEngine *e)
{
    if (numFrames == 0) return;
    FW1814Device *dev = e->fDevice /* [+0xe8] */;
    uint32_t numChan = e->fInStreamChannels /* [+0x2e4] */;
    uint32_t extra   = numChan - 8;                     // 0 for 8ch, 4 for 12ch
    src += 0x3c / 4;   // advance 15 ints into the AM824 packet (skip CIP + MIDI)
    dst += 0x3c / 4;

    for (uint32_t f = 0; f < numFrames; f++) {
        // Advance the ring cursor (modulo 16) stored at [+0x1d4c].
        uint32_t cursor = (++dev->fRingCursor /* [+0x1d4c] */) & 0xf;
        uint32_t prevSlot = dev->fRingLookup[cursor]        /* [+0x1b8c + cursor*4] */;

        // Load 7 previously-stored samples (one per channel) from the delay ring:
        int prev_ch0 = dev->fDelayRing[prevSlot][0]  /* [+0x1bcc + prevSlot*4] */;
        int prev_ch1 = dev->fDelayRing[prevSlot][1]  /* +0x1bd0 */;
        int prev_ch2 = dev->fDelayRing[prevSlot][2]  /* +0x1bd4 */;
        int prev_ch3 = dev->fDelayRing[prevSlot][3]  /* +0x1bd8 */;
        int prev_ch4 = dev->fDelayRing[prevSlot][4]  /* +0x1bdc */;
        int prev_ch5 = dev->fDelayRing[prevSlot][5]  /* +0x1be0 */;
        int prev_ch6 = dev->fDelayRing[prevSlot][6]  /* +0x1be4 — writes via rax index */;

        // Save current frame's channels 0..6 into the ring slot (big-endian in buffer).
        dev->fDelayRing[prevSlot][0..6] = src[-11..-5];   // bytes offsets -0x2c..-0x1c

        // Byte-swap previous samples (24-bit AM824 BE → signed int little-endian) and
        // emit to CoreAudio buffer at dst[-15..-9] (offsets -0x3c..-0x24):
        dst[-15] = swap24(prev_ch6);  // etc. for each channel using the roll+and+shl pattern
        dst[-14] = swap24(prev_ch5);
        dst[-13] = swap24(prev_ch4);
        dst[-12] = swap24(prev_ch3);
        dst[-11] = swap24(prev_ch2);
        dst[-10] = swap24(prev_ch1);
        dst[-9]  = swap24(prev_ch0);

        // Current-frame pass-through for the back half: ch7, ch8 always; ch9/10 if extra>=3;
        // ch11/12 if extra>=5.
        dst[-8]  = swap24(src[-7]);
        dst[-7]  = swap24(src[-6]);
        if (extra >= 3) {
            dst[-6]  = swap24(src[-5]);
            dst[-5]  = swap24(src[-4]);
        }
        if (extra >= 5) {
            dst[-4]  = swap24(src[-3]);
            dst[-3]  = swap24(src[-2]);
            dst[-2]  = swap24(src[-1]);
            dst[-1]  = swap24(src[0]);
        }

        src += numChan;        // advance one AM824 frame
        dst += numChan + 1;    // CoreAudio stride is +1 (room for MIDI quadlet skip)
    }
}
```

Byte-swap pattern used throughout (3 instructions):
```asm
andl    $0xff0000, %eax     ; keep middle byte
roll    $8,       %r13d
andl    $0xff00ff, %r13d    ; keep ends
shll    $8,       %r13d
addl    %eax,     %r13d
sarl    $8,       %r13d     ; sign-extend 24→32
```
This is the standard AM824 "unpack signed 24-bit big-endian to int32" sequence. Same math as `ConvertInputFramesOneStereoPairSkipMIDI` but applied per-channel inside the delay loop.

## Delay-ring state — `FW1814Device` instance fields

| Offset | Field | Semantic |
|---|---|---|
| `+0x1b8c..0x1bcb` | `uint32_t fRingLookup[16]` | cursor → slot-index indirection table |
| `+0x1bcc..0x1be3` | `int32_t fDelayRing[N][7]` | 7-channels-wide delay slots (array stride encoded via `fRingLookup`) |
| `+0x1d4c` | `uint32_t fRingCursor` | monotonic frame counter |

The exact size of the ring and the initialisation of `fRingLookup` is not visible from the workaround body alone — needs cross-check at construction. **Follow-up Q9a** will decode `FW1814Device::init` to confirm the ring is allocated/zeroed; if not, the first 16 host frames of audio after start-up are garbage (previous process memory). This matches a long-standing complaint about the 2014 driver: brief RX noise burst immediately after device startup.

## Why "input delay" is the symptom

The FW1814 generates its SYT timestamps against a hardware clock that is slightly ahead of the actual sample it ships in that packet. CoreAudio reads `(packet_timestamp + offset)` and expects the sample at that time to be the "current" sample. Without the workaround, CoreAudio pulls each sample one frame early, which it interprets as a monotonic lead — i.e. the input stream appears to run slightly ahead, producing cumulative drift and (because the host re-syncs periodically) click-at-each-resync. The workaround defers each sample by exactly one host frame, pulling it back into alignment with the SYT math.

## Delta summary

| Aspect | Reference | Clone (expected) | Impact |
|---|---|---|---|
| 16-entry per-channel delay ring at `[+0x1b8c..0x1be3]` | yes | **must mirror** | Without it: RX drift + click-at-resync |
| 7-channels-per-slot layout | yes | TBD | Channel map must match |
| Byte-swap-24 unpack per sample | standard AM824 | required | Any format change invalidates |
| Install at `[+0x310]` only when `fInStreamChannels != 2` | yes | required | Degraded 2ch mode skips workaround |
| Pass-through for channels 8..12 (no delay) | yes | **critical** | Only ch0..6 are delayed; ch7+ bypass |

**Open question (Q9b):** why do only channels 0..6 get the one-frame delay? Hypothesis: the FW1814 hardware block that generates the first 7 analog/digital-in channels (analog pair, S/PDIF pair, HSCI heartbeat, ...) is clocked by a different domain than the ADAT expander. The ADAT expander's samples arrive correctly aligned, so they bypass the FIFO. Needs confirmation against FFADO's maudio.cpp channel-map.

## Follow-up queue impact

- **Q9a:** confirm `fRingLookup[]` initialisation in `FW1814Device::init`. Expect zero-init via `OSObject::operator new` but worth verifying so clone can skip explicit init.
- **Q9b:** cross-reference FFADO + snd-bebob for FW1814 channel map (which hardware channel maps to which AM824 slot). Current hypothesis above about why only 0..6 are delayed depends on this.
- Follow-up: no equivalent exists for TX (`[+0x300]` just picks one of the stock `RearrangeSampleFrameLRLR*` rearrangers). Verify no TX-side workaround is needed — likely confirmed by Q10 latency table.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06; nm-verified symbol `__ZN24com_m_audio_FW1814Device27WorkaroundFor1814InputDelayEjPiS0_Pv` at `0xcba4`.
- Install site `movq %rax, 0x310(%rsi)` at 0xcb68 (RX convert slot) with the function-pointer `leaq` at 0xcb61.
- Guard `cmpq $0x2, %rax` with branch to the install site at 0xcb44 — distinguishes stereo-only path from workaround path.
- Ring-cursor increment `incl` of `0x1d4c(%rcx)` at 0xcc00; mask `andl $0xf, %eax` at 0xcc08 → 16-entry ring.
- Field offsets `0x1bcc / 0x1bd0 / 0x1bd4 / 0x1bd8 / 0x1bdc / 0x1be0` at 0xcc39..0xcc12.
- Per-channel byte-swap: `roll $8` + `andl $0xff00ff` + `shll $8` + `sarl $8` idiom at 0xccb8..0xccd1 and 15 clones.
- 3-way branch on `extra` (= `fInStreamChannels - 8`): `cmpl $0x3` at 0xcdef, `cmpl $0x5` at 0xce41.
