# C6 — Latency budget: published latencies, round-trip stages, and the clone's pass/fail bar

**Status:** consolidated 2026-10-14 (task P5 of `docs/BUILD-PLAN.md`). The 2026-10-14 Correction blocks that sat on top of this file are applied to the body (above all the **measured latencies at every rate, capture 72**); estimates that had no source are now labelled as estimates; derivations built on the retracted "64 TX segments" and "OUTPUT → 100 ms → INPUT" are removed. The 2026-10-13 original is in git history (`git show a9cb63c:docs/system-map/C6-latency-budget.md`).

**Sources:** A1-Q10 (`GetRoundTripLatencyForFDF`), A1-Q9 (`WorkaroundFor1814InputDelay`), A1-Q15c (`PerformSampleRateChangeCore` → `ResetLatency`), A1-Q17 (`TPacketPool`), Q-DCL-geometry-table, Q-CMP-connect §2, B5 §4/§6a/§6c, **captures 72 and 73** (SL, R25), capture 74 (TX descriptors, R28). **All measured numbers are from the SL rig running driver v1.9, not 1.11.3** (version-skew rule); the call sites that matter were verified identical in both binaries (R25), but no latency measurement exists for 1.11.3.

**Scope:** CoreAudio-visible latency properties and the round-trip stage chain. Latency only — lifecycle C1, sync C2, comm C3, transport C4, UC C5.

## 0. Headline

The kext publishes three per-rate properties to CoreAudio: `IOAudioEngineSampleOffset`, `IOAudioEngineOutputSampleLatency`, `IOAudioEngineInputSampleLatency`. Measured on the SL rig at idle for the six Panel rates (§1): they **change with the rate** and are recomputed by `ResetLatency` after every format change. A separate static table (`GetRoundTripLatencyForFDF`, §1.2) gives 210/210/225/358/378/642/698 frames; how it feeds the published properties is not decoded. The clone's pass/fail bar is "≤ the reference's published values at the same rate" (§7); nothing else in this document is a requirement.

## 1. Published latencies and the round-trip table

### 1.1 Measured at every rate (capture 72; SL v1.9; device idle; internal clock; `ioreg -r -c com_m_audio_FWEngine2`)

| Rate (Hz) | `SampleOffset` (frames) | `OutputSampleLatency` | `InputSampleLatency` | Stream channels (out / in) |
|---|---|---|---|---|
| 44100 | 40 | 216 | 32 | 12 / 10 |
| 48000 | 48 | 231 | 32 | 12 / 10 |
| 88200 | 88 | 352 | 64 | 8 / 10 |
| 96000 | 96 | 387 | 64 | 8 / 10 |
| 176400 | 176 | 387 | 128 | 4 / 2 |
| 192000 | 192 | 387 | 128 | 4 / 2 |

The 44.1/48 kHz rows reproduce the earlier B5 values (afplay 44.1 kHz §4/§6a; Studio One 2 at 48 kHz §6c — the file named `50-ioreg-logic-playback.txt` was in fact taken with Studio One 2, `51-ps-logic-playback.txt:54` `studioapp`). The channel counts are those of that run's clock bits (`MAClockSource = 0x40000014` → bit 4 set: 12 outputs; bit 3 clear: 10 inputs); they follow `SetupAudioEngine @0xed34` (C2 §2.3, R12). The six rows are the rates the hardware offers (memory `project_fw1814_hardware_capabilities`); 32 kHz (in the Q10 table) was **not measured**, and whether the Panel offers it is **unverified**.

**Empirical regularities (observations, not decoded formulas):** `SampleOffset` equals the rate in kHz rounded down to a multiple of 8 (44→40, 48→48, 88→88, …). `InputSampleLatency` equals 4 × the SYT interval (8/16/32 → 32/64/128) at all six rates — consistent with, but **not proven to derive from**, the SYT interval (**hypothesis**). `OutputSampleLatency` has no derived formula (216, 231, 352, 387, 387, 387).

### 1.2 `GetRoundTripLatencyForFDF` (A1-Q10, static table)
210 / 210 / 225 / 358 / 378 / 642 / 698 frames at 32 / 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz. This is **not** the published `SampleOffset` (40/48 at 44.1/48 kHz). The published `OutputSampleLatency` exceeds the table by 6 frames at 44.1/48 kHz, is 6 lower at 88.2 kHz and 9 higher at 96 kHz; no formula was derived. Which CoreAudio setter (`setSampleOffset`, `setSampleLatency`, `setInputSampleLatency`) each value feeds is **open** (decompile `FWEngine2::initHardware` + `FWBaseEngine::ResetLatency`).

### 1.3 Observations
1. `InputSampleLatency` is rate-independent only up to 48 kHz (32); it steps to 64 at 88.2/96 kHz and 128 at 176.4/192 kHz. The earlier statement "constant 32 at all rates" and the prediction "still 32 above 48 kHz" are **refuted** by capture 72.
2. `WorkaroundFor1814InputDelay` (Q9) holds one host frame on channels 0..6 and bypasses channels 7..11; it is installed whenever the input stream has other than 2 channels. At 176.4/192 kHz the input stream is 2-channel, so `[+0x2e4] == 2` **skips** it (Q9:26) — the earlier "always on for the FW1814" is wrong at those rates. The published input latency therefore cannot be explained by the one-frame ring; its other contributors (device ADC pipeline, AR-DMA hold) are **unverified** (check: DTrace `WorkaroundFor1814InputDelay:entry`, compare with an Audiophile that has no Q9).
3. Clone: keep the per-channel asymmetry inside the stream (do not delay channels 7..11).

## 2. Round-trip stage chain (48 kHz, 512-frame buffer)

Wall-clock arithmetic (exact): one isoch cycle = 125 µs; one 48 kHz sample = 20.833 µs; a 512-frame buffer = 10.667 ms (host policy, additive and independent of the driver's published latency). Microsecond figures marked *estimate* have **no source** — they were computed on comparable hardware by feel in the 2026-10-13 draft and must be replaced by measurements; they are listed only to show where time can hide.

| # | Stage | Cost |
|---|---|---|
| 1 | CoreAudio client write → engine mix buffer | 0 (same memory, IOAudioFamily contract) |
| 2 | mix buffer → AM824 pack (`clipOutputSamples`, 512 frames) | *estimate* tens of µs (unmeasured) |
| 3 | pack buffer → DCL TX packet pool | 0 (the converter writes into the pool memory, Q17) |
| 4 | DCL TX → OHCI AT DMA | up to one cycle (125 µs) — arithmetic from the 8 kHz cycle |
| 5 | AT serialisation → PHY → wire | *estimate* ≲ 1 cycle |
| 6 | device ingress → PLL retime → DAC | device-internal — **open (measure)** |
| 7–8 | external analog loop → ADC → device packet buffer | device-internal — **open (measure)** |
| 9 | device → wire → OHCI receive | *estimate* ≲ 1 cycle |
| 10 | OHCI → RX packet pool (payload at +8 bytes) | up to one cycle |
| 11 | RX segment completes → callback | RX segment = 4 packets = 4 cycles = 500 µs (derived from the fixed `packetCount` 4, Q-DCL; assumes one packet per cycle); the earlier "≤ 125 µs / ~1 ms per segment" figures were based on the retracted 64-segment TX model |
| 12 | OHCI interrupt → isoch workloop → segment callback | *estimate* ≲ tens of µs |
| 13 | callback → `WorkaroundFor1814InputDelay` (1 frame hold = 20.833 µs at 48 kHz + byte-swap pass) | 20.833 µs + *estimate* |
| 14–15 | unpack → `takeTimeStamp` → CoreAudio read | 0 (zero-copy) |

The older sums ("≈ 616 µs per direction", "≈ 1.23 ms host round trip", "3.5 ms remaining in the device") were built from the estimates above and are **withdrawn**. What is solid: the published `OutputSampleLatency`/`InputSampleLatency` of §1.1 and the static table of §1.2 (225 frames = 4.69 ms at 48 kHz).

## 3. Buffer ladder (pure arithmetic)

The ladder `{64, 128, 256, 512, 1024, 2048, 4096}` frames (default 512; memory `project_fw1814_buffer_sizes`) is set by CoreAudio through `IOBufferFrameSize`; the kext has no per-rate buffer cookie (Windows-only cookies, memory `project_mac_vs_windows_scope`). Period = frames / rate:

| Buffer | 44.1 kHz | 48 kHz | 96 kHz | 192 kHz |
|---|---|---|---|---|
| 64 | 1.451 ms | 1.333 | 0.667 | 0.333 |
| 128 | 2.902 | 2.667 | 1.333 | 0.667 |
| 256 | 5.805 | 5.333 | 2.667 | 1.333 |
| 512 | 11.610 | 10.667 | 5.333 | 2.667 |
| 1024 | 23.220 | 21.333 | 10.667 | 5.333 |
| 2048 | 46.440 | 42.667 | 21.333 | 10.667 |
| 4096 | 92.880 | 85.333 | 42.667 | 21.333 |

The DCL ring geometry does not depend on the CoreAudio buffer size (C4 §3: TX 20 segments, RX 64 × 4); the published latency properties do not either — they change only with the rate (§1.1). (macfw's "low-latency 256-frame capture" target, B4 `README.md:22`, is for the FW410, not the FW1814.)

## 4. Format-change transient

Chain: C2 §4 (slot names per R2). **Measured at idle (capture 72):** a rate change runs 3×`IOSleep(100)` (CMP `updateP2PCount` retries during the TX start) → `IOSleep(500)` → `IOSleep(500)` (`ConnectAudioMIDIChannels`) → `IOSleep(100)` → ≈0.9 s gap → `IOSleep(1000)` (`ResendCurrentMixerSettings`/`CompletePendingStateChangesAction`), **≈ 3.5 s in total**. A clock-source change adds two `IOSleep(300)` (`SetClockSourceInternal`), and the blank-slate variant `IOSleep(2500)`; `ReconnectP2P` sleeps 1100 ms (call sites identical in v1.9 and 1.11.3; R21, R25). The earlier "worst-case audio suspension ≈ 180 ms" (built on a 100 ms stream kick, 20 ms AV/C and a 10 ms DCL rebuild) is **withdrawn**: the stream-kick figure was wrong and the others were estimates. Per-step costs of `ApplyRateChange` and the AV/C frames remain unmeasured.

**Hazard:** v1.9 kernel-panics when a rate change arrives mid-stream (B5 cap 65; memory `project_v19_rate_change_while_streaming_panics`); 1.11.3 behaviour **unverified**. Clone: gate the change on the engine state and run the full pause/begin/…/resume sequence (C2 §4).

## 5. Reference peers
FFADO, snd-bebob and the Rust crate publish no per-rate CoreAudio-style latency table (Linux sizes queues from the ALSA period at run time; B2); macfw targets the FW410 and gives only the "256-frame capture" target; the Sierra hack bundle contains the same binary as v1.10.5 (B3) so §1 applies to it unchanged. None is a latency cross-check for the FW1814.

## 6. Interrupt cadence

**RX:** one callback per 4-packet segment (`InputDCLStaticCallback`, every segment) — 4 cycles = 500 µs, i.e. 2000 callbacks/s at every rate (derived from the fixed `$0x4` packet count, `0x25a99`, assuming one packet per cycle; **unverified** against a live trace). **TX:** the callback is attached only to the *big* segment of each of the four banks (`P−4` packets, `P = ⌊65,536,000/(rate·4)⌋ + 2`: 514/373/343/187/172/94/87 at 32/44.1/48/88.2/96/176.4/192 kHz, C4 §3), so the TX callback cadence is `(P−4)` packet-times per callback; whether a TX packet is sent every cycle (its duration per callback = `(P−4) × 125 µs`) is **unverified** (capture 74 shows a repeating subset of descriptors with the extra interrupt bits, as expected for these big segments). The earlier "constant ~1 ms interrupt per segment at every rate" table was derived from the retracted 64-segment-per-direction model and is **withdrawn**. `takeTimeStamp`'s call site is **unverified** (C2 §6.1). Timing consequences for the clone must be derived from the real geometry, not from this section until measured.

## 7. Clone pass/fail bar

For every supported rate the clone's published `SampleOffset`, `OutputSampleLatency` and `InputSampleLatency` must be **≤ the reference's values at that rate** (reference-parity rule). Using the only measured ground truth (SL v1.9, §1.1):

| Rate | SampleOffset | OutputSampleLatency | InputSampleLatency |
|---|---|---|---|
| 44100 | ≤ 40 | ≤ 216 | ≤ 32 |
| 48000 | ≤ 48 | ≤ 231 | ≤ 32 |
| 88200 | ≤ 88 | ≤ 352 | ≤ 64 |
| 96000 | ≤ 96 | ≤ 387 | ≤ 64 |
| 176400 | ≤ 176 | ≤ 387 | ≤ 128 |
| 192000 | ≤ 192 | ≤ 387 | ≤ 128 |
| 32000 | not measured — measure or accept the Panel's range | | |

"Match" is the expected outcome for a parity clone. These are v1.9 numbers; re-measure against 1.11.x if a 1.11.x rig ever exists. **Fail modes to screen:** publishing zero latency (CoreAudio over-pulls); hard-coding the 48 kHz values at all rates; not calling `ResetLatency` after `performFormatChange` (A1-Q15c: mandatory). **Secondary bar:** the static round trip at 48 kHz is 225 frames = 4.69 ms (Q10); a loopback measurement (Main-L OUT → Analog-In-1, tone capture) should not exceed the reference's; the reference's own hardware round trip has **not** been measured (open, §8).

**Bring-up gate:** the first build that streams must publish these numbers before any mixer, routing or MIDI parity work is judged (wrong latency makes the monitoring loop click).

## 8. Open measurements

| # | Open | How |
|---|---|---|
| 1 | 32 kHz published latencies (if reachable) | same `ioreg` command as capture 72 |
| 2 | Which setter each Q10 value feeds; the formulas behind §1.1 | decompile `FWEngine2::initHardware`, `FWBaseEngine::ResetLatency` |
| 3 | Contribution of the Q9 ring / ADC / AR-DMA to `InputSampleLatency` | DTrace `WorkaroundFor1814InputDelay`, compare with an Audiophile |
| 4 | Real hardware round trip at 48 kHz/512 and other rates | loopback cable + timestamped tone capture |
| 5 | OHCI cycle-timer jitter, `takeTimeStamp` jitter and cadence | DTrace `processCycleTime` / `takeTimeStamp` on SL (60 s during afplay) |
| 6 | TX/RX callback cadence per rate; packets per cycle | trace or OHCI descriptor count (BUILD-PLAN M-B) |
| 7 | Cost of AV/C frames, `ApplyRateChange`, pack/convert | timestamped DTrace of one idle rate change |
| 8 | 1.11.x vs 1.9 differences, if any | needs a 1.11.x rig |

## Cross-links
C1 (format change gate), C2 (§4 chain, §5 rate table, §7 stream start), C3 (AV/C frames), C4 (DCL geometry, packet sizes), C5 (UC does not affect latency); A1-Q9/Q10/Q15c/Q17, Q-DCL-geometry-table, Q-CMP-connect; B5 §4/§6a/§6c; captures 72, 73, 74; R1, R21, R25, R28.

## Change log (2026-10-14 corrections folded in)
Latency at all rates (R25, capture 72) → §1.1 (the per-rate table is now measured, six rates); audit items: stream-kick order, slot names, rate-table/CIP, bundle naming, "estimates not measurements", 176.4/192 kHz workaround skip (R1, R2, R5, Q9) → §1.2–§4; DCL ring shape (Q-DCL) → §2, §6 (the "64 segments × ~1 ms" derivations removed); "breaker #3 / vtable drift" and D1/D2/D3 recommendation sections removed (Q-SDK, R34); old §10 replaced by the §7 bring-up gate.
