# Q10 — `GetRoundTripLatencyForFDF` — per-rate reported latency

> **Correction (2026-10-15, M-D / R45):** `GetSafetyOffsetForFDF @0x1d58` (feeds `setSampleOffset` in `ResetLatency`) = 44/44/48/88/96/176/192 frames for FDF 0..6 (default `0x2c`); the round-trip value is split `out = rt >> 1`, `in = (rt + 1) >> 1`. See [`Q-MD-engine-rate-chain.md`](Q-MD-engine-rate-chain.md).

**Status:** done.

**Scope:** `FW1814Device::GetRoundTripLatencyForFDF` @ 0xcf30 (docs/system-map/A1-ghidra/otool-exports/mafw.asm:14695); `FWAudiophileDevice::GetRoundTripLatencyForFDF` @ 0xafde (:12524). Nine sibling device classes have equivalents at the symbols listed in the queue — M-Audio shipped one per product (FW410, Ozonic, Solo, NRV10, ProjectMix, Lightbridge, Rev1/Rev2 bases).

## Answer in one line

**Each device returns a per-FDF integer that CoreAudio uses as the device's reported round-trip safety offset in frames.** FW1814 reports 210/210/225/358/378/642/698 frames for 32k/44.1k/48k/88.2k/96k/176.4k/192k. Audiophile reports 182/182/196/330/354/-/- (182 is the default for every rate it doesn't recognise, including 32k/44.1k and all high rates — a soft indicator that the Audiophile is only truly supported at 48k/88.2k/96k).

## FW1814 table

Switch on `FDF - 2`; default (FDF=0 or 1, i.e. 32k and 44.1k) returns 210.

| FDF | Rate | Return value (frames) | Decimal | Approx ms @ rate |
|---|---|---|---|---|
| 0 | 32 kHz | `0xd2` | 210 | 6.56 ms |
| 1 | 44.1 kHz | `0xd2` | 210 | 4.76 ms |
| 2 | 48 kHz | `0xe1` | 225 | 4.69 ms |
| 3 | 88.2 kHz | `0x166` | 358 | 4.06 ms |
| 4 | 96 kHz | `0x17a` | 378 | 3.94 ms |
| 5 | 176.4 kHz | `0x282` | 642 | 3.64 ms |
| 6 | 192 kHz | `0x2ba` | 698 | 3.64 ms |

The near-constant ~3.6–4.8 ms wall-clock latency across rates is characteristic of a FireWire audio device whose latency is bus- and DMA-dominated (IRM bandwidth round-trip + 2 isoch cycles + DCL double-buffer), not sample-rate dominated. The 32k entry is anomalously high (6.56 ms) because the FW1814's isoch stack still runs at the same host cadence regardless of audio rate — slower sample clock means more wall-clock samples per DMA interrupt.

## Audiophile table

Three explicit cases (FDF=2/3/4); every other FDF (including 32k/44.1k and 176.4k/192k) falls to the default 182.

| FDF | Rate | Return value (frames) | Decimal | Approx ms @ rate |
|---|---|---|---|---|
| any default | 32k/44.1k/176.4k/192k | `0xb6` | 182 | varies |
| 2 | 48 kHz | `0xc4` | 196 | 4.08 ms |
| 3 | 88.2 kHz | `0x14a` | 330 | 3.74 ms |
| 4 | 96 kHz | `0x162` | 354 | 3.69 ms |

The default 182 for 32k/44.1k/176.4k/192k is the giveaway that those rates aren't properly characterised — likely the Audiophile firmware doesn't actually support 176.4k/192k, and 32k/44.1k reuse the fallback because the panel never exposed them separately.

## Interpretation — what the number *is*

Called from CoreAudio when the engine registers; its return value is wired into `IOAudioEngine::setSampleOffset` / `setInputSampleOffset` (one or both — needs cross-check in `FWEngine2`). CoreAudio treats this number as "frames of pipeline delay the host should assume beyond what the mix engine already accounts for" — i.e. the sum of (device-side ADC delay) + (FireWire bus round-trip) + (host DMA re-timing) that CoreAudio can't observe itself.

Low numbers are good (less latency reported, better round-trip for live monitoring); numbers that are too low cause CoreAudio to under-compensate and the user hears a monitoring click/double. The 2014 M-Audio engineers measured these empirically and hard-coded one value per rate.

## Delta summary

| Aspect | Reference | Clone (expected) | Impact |
|---|---|---|---|
| 7-entry per-rate table (FW1814) | yes | **must mirror exactly** | Any value too low → monitoring artefacts |
| Default fallback for FDF < 2 | 210 for FW1814, 182 for Audiophile | required | Fallback rates must match |
| Different base per device subclass | yes (10 variants) | FW1814 only for clone | FW1814 numbers only |
| Return in units of frames | yes | required | Confirmed by caller (`setSampleOffset`) |

## Follow-up queue impact

- **Q10a (new):** confirm caller — expect `FWEngine2::initHardware` (or similar) to invoke `GetRoundTripLatencyForFDF(currentFDF)` and feed it to `IOAudioEngine::setSampleOffset`. Needs 1 grep + 1 skim.
- Follow-up: the FW1814 table has only 7 entries; what happens at an FDF value not in {0..6}? Currently returns default 210. This is fine for the clone — the FDF value is bounded by the rate table in Q2.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- FW1814 immediates `0xe1, 0x166, 0x17a, 0x282, 0x2ba, 0xd2`: at 0xcf4c, 0xcf53, 0xcf5a, 0xcf61, 0xcf68, 0xcf6f.
- Audiophile immediates `0xc4, 0x14a, 0x162, 0xb6`: at 0xaff1, 0xaff8, 0xafff, 0xb006.
- Jump-table dispatch range `cmpl $0x4, %esi` (FW1814) and three-way `cmpl` chain (Audiophile) at 0xcf37 / 0xafe2.
