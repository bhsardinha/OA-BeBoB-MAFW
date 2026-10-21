# Q-AP0d — Audiophile sample converters and peak proc (AP-0.7)

Status: **resolved 2026-10-19** (machine code), except the physical port ↔ wire-slot mapping (**unverified**, hardware). Bundle v1.11.2. Tracker Itens `AP-0.7.*`; resolution R80. Installed by `FWAudiophileDevice::SetupAudioEngine @0xbb9a` (`Q-AP0-nub-and-rev2.md`; engine fields `[+0x310]` input proc, `[+0x300]` output rearrange, `[+0x308]` peak proc; the engine path that calls them is `Q-MD2-sample-path.md` §1, shared with the 1814).

| Routine | Machine code | Behaviour |
|---|---|---|
| `_ConvertLLRR4SkipMIDI1(n, wire, host)` `@0x28eeb` | asm `0x28eff-0x28fa6` | per frame: 5 wire quadlets (stride `0x14`), slot 4 (MIDI) skipped; each audio quadlet `[label b2 b1 b0]` → sign-extended 24-bit `int32` (`roll/and/shl/sar 8`); **`host = [wire1, wire3, wire0, wire2]`** (stores at `0x28f87-0x28f91`); `n` counts samples (`−4` per frame) |
| `_RearrangeSampleFrameLLRR6(host, wire, frames)` `@0x2c0e5` | asm `0x2c100-0x2c2af` | per frame: clamp each `int32` to `[−0x800000, 0x7FFFFF]`, write `[0x40, b2, b1, b0]` (AM824 MBLA label `0x40` + 24-bit big-endian); **`wire[k] = host[p[k]]`, `p = 0, 2, 4, 1, 3, 5`**; wire stride 7 quadlets, slot 6 (MIDI) not touched |
| `_SamplePeaks6(host, peaks, n)` `@0x27e17` | decompile | every 8th frame (`n >> 3` iterations, stride `0x30` quadlets = 8 frames × 6), per channel `peak = max(peak, |(int16)(s >> 8)|)` — 16-bit magnitude of the top of the 24-bit sample, running max (cleared by `ClearPeakMeters`, R76) |

Wire order this implies (same as `docs/audiophile-manual-reference.md` §3.3, which derived it from the same code): output `[A.L, B.L, C.L, A.R, B.R, C.R]` with host `[out1, out2, out3, out4, S/PDIF L, S/PDIF R]`; input wire `[X.L, Y.L, X.R, Y.R]` → host 1/2 = pair Y, 3/4 = pair X (with the manual's host order: wire `[S/PDIF L, Analog L, S/PDIF R, Analog R]`). `IndexForAC3OutputChannels = (2, 5)` = the C pair's wire slots. **Conflict** with FFADO `fwap.xml` stays; settle by loop test (AP-7.3) or by the device's own plug cluster info (`EXTENDED PLUG INFO` STATUS, read-only — candidate for the SL rig, needs the user's OK).

Clone: `openaudio-fw1814/kext/src/FWSamples.cpp` has the 1814's LRLR procs only (Q-MD2 §"Not ported"); these three are new, straight ports from the table above.
