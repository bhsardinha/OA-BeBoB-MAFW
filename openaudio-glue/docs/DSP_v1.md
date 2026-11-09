# Glue — DSP notes (v1)

Per channel, in this order: **Transformer → Tube → EQ → Compressor → Desk colour → Tape.** The design values below are starting
points to be tuned by ear with the user; each tuning pass is logged at the end with its date.

## Colour stages (global)

Transformer, Tube and Tape: Off / Soft / Medium / High / Full. **Desk (user, 2026-10-28): Off, Neve Medium, Neve High, API
Medium, API High, SSL Medium, SSL High** (six presets: Medium and High for each console type).

| Stage | Source (Airwindows, MIT) | What the preset sets |
|---|---|---|
| Transformer | Interstage (no parameters) | drive into the stage, dry/wet, output trim |
| Tube | Tube2 (Input, Tube) | Input, Tube, output trim |
| Desk colour | Channel9 (Console Type Neve/API/SSL, Drive, Output) | type and Drive (own option list, below) |
| Tape | ToTape9 (Input, Tilt, Shape, Flutter, FlutSpd, Bias, HeadBmp, HeadFrq, Output) | all nine, with head-bump frequency aimed at 15 ips character |

- Off is an exact bypass: the stage isn't run.
- Each preset has an output trim so that switching presets keeps a −18 dBFS sine at the same level (± 0.5 dB, checked by the
  host test).
- Mono ports (`dsp/AirwindowsMono.*`, from commit `dfa54018`): each Airwindows plug-in is stereo; the port keeps the left
  channel's state and logic. The only place L reads R state is ToTape9's flutter (it steers its random speed by the R sweep);
  that R state never depends on the audio, so the port carries a shadow of it (`sweepR`, `nextmaxR`, `fpdR`) and stays
  identical to the original's L.
  > **Correction (2026-10-28):** the first port used L's own sweep there; `tools/port-diff` showed it drifting from the original
  > at 96 kHz (flutter on), so the shadow state replaced it.
- Verified (G7): all four ports equal the originals' L output (fed L = R, same dither seed) to float rounding, worst residual
  −146.9 dB over 100 cases (every preset, flutter on/off, 44.1/48/96/192 kHz), on the dev Mac and on the High Sierra build PC.

## Compressor (per channel: threshold + source)

> **Correction (user, 2026-10-28):** the compressor gets a per-channel **Source** setting (General, Drums, Vocals, Heavy
> guitar, Bass, Synth) that picks a voicing; Drums keeps the transients ("an expander on the hit and a comp on the sustain").
> The threshold stays the only amount control. The list below is the General voicing (v1); the table after it gives every
> voicing (`dsp/GlueComp.cpp`, design values).
- feed-forward, RMS detector (10 ms);
- ratio 3:1, 6 dB soft knee;
- program-dependent attack/release: two envelopes follow the reduction, fast (attack 10 ms, release 80 ms) and slow (attack
  150 ms, release 600 ms), and the deeper one wins. So a short burst recovers in about 80 ms, while sustained compression
  releases slowly (tested: `TestCompDynamic`);
- threshold −40…0 dB, where 0 dB means off;
- makeup = half the static gain reduction a −18 dBFS signal gets at this threshold;
- gain reduction is published for the panel's meter.

| Source | Detector | Ratio / knee | Attack fast / slow | Release fast / slow | Extra |
|---|---|---|---|---|---|
| General | RMS 10 ms | 3:1 / 6 dB | 10 / 150 ms | 80 / 600 ms | |
| Drums | peak | 4:1 / 6 dB | 30 / 200 ms | 120 / 400 ms | hit lift: a peak detector feeds a 0.5 ms and a 20 ms follower (shared 60 ms release); 0.7 × their gap in dB, up to +6 dB, scaled by threshold depth (full at −20 dB) |
| Vocals | RMS 5 ms | 3:1 / 10 dB | 5 / 120 ms | 60 / 800 ms | smooth, opto-like two-stage release |
| Heavy guitar | RMS 20 ms | 2:1 / 10 dB | 20 / 300 ms | 200 / 900 ms | gentle on dense material |
| Bass | RMS 30 ms | 4:1 / 6 dB | 15 / 200 ms | 120 / 500 ms | the detector window spans a 30 Hz cycle |
| Synth | RMS 10 ms | 2.5:1 / 6 dB | 8 / 150 ms | 100 / 600 ms | |

- Measured (host test `TestTransientKeep`, 48 kHz): 1 kHz hits with a 150 ms decay at −6 dBFS, threshold −24 dB. Punch =
  peak of the first 5 ms over the RMS 100–150 ms after the onset: **dry 10.2 dB, General 15.0 dB, Drums 21.6 dB.** General's
  10 ms attack already lets the start of a hit through; Drums adds the slow attack and the lift.
- Every voicing's steady-state reduction follows its own curve (± 0.6 dB, `TestCompSources`). Drums adds no lift on a steady
  tone (< 0.3 dB).

## EQ (per channel, 4 bands, RBJ Audio-EQ-Cookbook biquads)

| Band | Type | Frequency range | Gain | Q |
|---|---|---|---|---|
| LO | low shelf | 30–600 Hz | ±12 dB | fixed slope S = 1 |
| LO-MID | peaking | 60 Hz–2 kHz | ±12 dB | 0.5 + 0.1·\|gain dB\| |
| HI-MID | peaking | 500 Hz–12 kHz | ±12 dB | 0.5 + 0.1·\|gain dB\| |
| HI | high shelf | 1.5–16 kHz | ±12 dB | fixed slope S = 1 |

- A band at 0 dB is skipped.
- Coefficients are recomputed once per IO block from smoothed targets, never per sample.

## Preset table v1 (starting points, `dsp/GluePresets.cpp`)

| Stage | Soft | Medium | High | Full |
|---|---|---|---|---|
| Transformer (Interstage) | drive 0 dB, wet 50 % | +3 dB, 75 % | +6 dB, 100 % | +9 dB, 100 % |
| Tube (Tube2) | A 0.35 B 0.25, wet 50 % | A 0.50 B 0.45, 75 % | A 0.70 B 0.65, 100 % | A 0.90 B 0.85, 100 % |
| Desk (Channel9) | see below | | | |
| Tape (ToTape9) v2 | A 0.56 (+1.9 dB), B 0.72, D 0.15, G 0.30 | A 0.63 (+4.0 dB), B 0.74, D 0.20, G 0.35 | A 0.71 (+6.0 dB), B 0.77, D 0.25, G 0.40 | A 0.80 (+8.2 dB), B 0.79, D 0.30, G 0.45 |

- Desk options: Neve / API / SSL type, each at Medium = Drive 0.4 and High = Drive 0.6. Type constants from Channel9's
  `setParameter` (Neve iir 0.005832, slew threshold 0.334, low-pass 28.8 kHz; API 0.004096, 0.600, 27.2 kHz; SSL 0.004913,
  0.849, 23.0 kHz).
  > **Correction (2026-10-28, user):** the first table was Neve only, at Drive 0.2 / 0.4 / 0.6 / 0.8 for Soft…Full.
- Tape, all levels: C (Shape) 0.5, E (flutter speed) 0.5, F (bias) 0.5 = neutral, I (output) 0.5 = unity; H 0.63 = head
  bump at about 94 Hz (30 ips character).
  > **Correction (2026-10-28, user: "a little more bright, maybe 30 ips but with more saturation"):** v1 was A 0.50/0.56/0.63/0.71,
  > B 0.5, D 0.2–0.5, G 0.3–0.6, H 0.414 (bump 55 Hz, 15 ips).
- Level matching: `Presets::calibrate` normalises each stage's wet path and then the blend, so a −18 dBFS 1 kHz sine leaves
  every preset at −18 dBFS (± 0.5 dB, tested at 6 rates). It takes 111–138 ms per rate.

## Delay compensation (user, 2026-10-28: "delay compensation in bypassed channels to avoid phase issues")
- `Presets::calibrate` measures each stage's phase delay at 1 kHz, −18 dBFS, per option.
- A chain's latency is the rounded sum over its active stages; it can be negative where a stage leads in phase (the tape at
  176.4/192 kHz).
- Every channel is padded with a plain delay up to the largest latency of any combination:

  | Rate | Fixed latency | Worst deviation (`TestDelayCompensation`) |
  |---|---|---|
  | 44.1 / 48 kHz | 1 sample | 0.49 sample |
  | 88.2 / 96 kHz | 3 samples | 0.50 |
  | 176.4 / 192 kHz | 4 samples | 0.50 |

- **Every channel arrives with the same delay:** processed channels with any stages, switched-off channels, and the unit's
  bypass. Off and bypass stay bit-exact in value, just delayed.
- The device reports the unit's latency plus the fixed one, a constant per rate, so stage changes need no Core Audio
  reconfiguration and the DAW also aligns Glue with the unit's own inputs.
- **What delay can't remove:** each filter's frequency-dependent phase, and up to half a sample of rounding.
- Per-stage delays measured at 192 kHz (samples, 1 kHz):

  | Stage | Delay |
  |---|---|
  | Tube | +0.75 to +1.50 |
  | Desk | +1.6 (Neve), +1.9 (API), +2.3 (SSL) |
  | Tape | −0.5 to −2.1 |
  | Transformer | 0 |

  At 44.1/48 kHz the tape is the only one that counts (+0.9).

## THD of the strongest presets (`tools/glue-thd`, 48 kHz, 1 kHz sine, compressor off, EQ flat, 2026-10-28)
THD = harmonics 2–10 over the fundamental, over 1000 whole cycles.

| Preset | −18 dBFS | −12 dBFS | −6 dBFS | 0 dBFS | Character |
|---|---|---|---|---|---|
| dry (all Off) | 0.000 % | 0.000 % | 0.000 % | 0.000 % | — |
| Transformer Full | 0.000 % | 0.000 % | **9.4 %** | 9.0 % | clean, then a hard slew-limit edge (+9 dB drive into a 0.38 slew threshold) |
| Tube Full | 1.38 % | 3.02 % | 6.87 % | 23.3 % | mostly odd (H3 −37.5 dB at −18), some H2 (−51.5) |
| Desk Neve / API / SSL High | 0.044 % | 0.197 % | 1.37 % | 16.8 % | odd, with H2 rising when hot (−19.5 dB at 0 dBFS) |
| Tape Full | 1.41 % | 3.31 % | 13.0 % | 24.8 % | odd only (H2 below −90 dB) |
| All Full (any desk type) | 2.95 % | 6.59 % | 4.24 % | 21.6 % | the transformer's slew limit at the front lowers what reaches the rest at −6 dBFS |

- The three desk types measure the same on a steady 1 kHz tone at 48 kHz. Channel9's types differ in their transient clamp
  (Neve 0.334, API 0.600, SSL 0.849) and in a high-frequency filter that only acts at 88.2 kHz and above, not in the saturation
  curve.

## Colour v2 (user, 2026-10-28: "the colour buttons need to be in the path of every channel")
- **Finding.** The stages were in every channel's path: each one alone changed all 16 channels through the driver code (driver
  test "colour: … changes every channel"). But Transformer and Tube were voiced for near full scale. On the user's drum loop at
  −12 dBFS peaks:
  - Transformer Full changed the signal by −31.7 dB and Tube Full by −28.9 dB (energy of the difference over the dry signal),
    almost inaudible;
  - Tube Soft by −54.9 dB, inaudible.
- **Re-voiced.** `tools/glue-voice` finds the drive into each preset that reaches a target at −12 dBFS peaks; the wet path stays
  normalised, so the level is still matched.
  - **Transformer** (on the drum loop): Soft +11.0, Medium +12.3, High +14.1, Full +17.4 dB, for a change of
    −36 / −30 / −24 / −18 dB.
  - **Tube:** +6.7 / +7.3 / +8.4 / +7.0 dB, for THD of 0.3 / 1 / 3 / 8 %.
  - **Desk** (every type): Medium +6.5, High +8.0 dB, for THD of 1 / 3 %.
  - **Tape** unchanged: its v2 presets already exceed the targets.
- **Global Drive** (user, 2026-10-28): 0…+18 dB more into every colour stage, taken back after it, so the level stays about the
  same (`TestDrive`: +12 dB on Tube Medium more than doubles the THD, level within 3 dB). Plist key `"drive"`; the app's slider
  is in the global card.
- **THD after v2** (1 kHz, 48 kHz, `tools/glue-thd`):

  | Preset | −18 dBFS | −12 dBFS | −6 dBFS | 0 dBFS |
  |---|---|---|---|---|
  | Transformer Full | 3.2 % | 10.0 % | 7.5 % | 4.3 % |
  | Tube Full | 3.5 % | 8.0 % | 25.8 % | 36.4 % |
  | Desk High (any type) | 0.35 % | 3.0 % | 24.5 % | 42.7 % |
  | Tape Full | 1.4 % | 3.3 % | 13.0 % | 24.8 % |
  | All Full | 3.5 % | 4.4 % | 22.7 % | 40.0 % |

## Tuning log
- **2026-10-28, tape v2** (user, after listening to `bateria.mp3` through `glue-listen`). Measured at 48 kHz, flutter off,
  response at −18 dBFS relative to 1 kHz, harmonics of a −6 dBFS 1 kHz sine:

  | Level | 10 kHz v1 → v2 | 5 kHz v2 | 80 Hz v1 → v2 | THD (H3 dominant) v1 → v2 |
  |---|---|---|---|---|
  | Soft | −1.3 → +0.5 dB | +0.9 | −1.1 → +1.9 | −38.8 → −31.2 dB |
  | Medium | −1.8 → +0.5 | +1.0 | −0.4 → +2.4 | −35.3 → −27.9 |
  | High | −2.7 → +0.5 | +1.3 | +0.6 → +2.7 | −31.6 → −24.2 |
  | Full | −4.0 → +0.5 | +1.5 | +1.4 → +2.8 | −27.8 → −17.7 |

  The Tilt per level came from a search for +0.5 dB at 10 kHz. ToTape9's saturation is symmetric, so the harmonics are odd
  (2nd below −90 dB).
