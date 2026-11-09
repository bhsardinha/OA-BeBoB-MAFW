# Glue tools

All build with the same flags as `dsp/` (C++17 language, C++14 library, `-mmacosx-version-min=10.13`) and run on the dev Mac
and on the build PC (`fw1814-build`, 10.13.6).

## glue-bench — CPU share and calibrated values
```
make -C ../dsp bench                         # 16 channels, 48 kHz, 64-frame blocks
../dsp/build/glue-bench 18 44100 64          # channels, rate, frames
```
It prints the share of one core, the calibration time, the trims and wet norms per preset, and channel 0's gain reduction.

## glue-listen — hear the chain on a file, before the driver exists
```
make -C ../dsp listen
../dsp/build/glue-listen <file> [--device <name part>] [--out <first output>] [--tour]
```
- It plays the file in a loop through the Glue chain (left and right each through their own channel chain).
- Output goes to the device whose name contains `1814` by default, on outputs 1/2 = SW return 1/2. `--out 3` uses 3/4.
- The file is converted to the device's current rate; the device's rate and settings are never changed. Output starts at
  −6 dB.
- It's an ordinary Core Audio client: no kext, no plug-in.

Keys while it plays:

| Key | Action |
|---|---|
| `t` `u` `p` | Transformer / Tube / Tape: next level (Off → Soft → Medium → High → Full) |
| `d` | Desk: next option (Off → Neve Medium → Neve High → API Medium → API High → SSL Medium → SSL High) |
| `a` | Transformer, Tube and Tape to the next level together (Desk keeps its option) |
| space | A/B: Glue ↔ dry (the chain keeps running, so the switch is instant) |
| `,` `.` | compressor threshold −2 / +2 dB (0 = off) |
| `s` | compressor source: General → Drums (keeps transients) → Vocals → Heavy guitar → Bass → Synth |
| `e` | EQ curve: flat, smile, low-mid cut, presence, low-end cut, scoop, punch, air (table below) |
| `'` `¡` | output level −1 / +1 dB (up to 0 dB) |
| `q` | quit |

EQ curves (gain in dB at the band's frequency):

| Curve | Low shelf | Low-mid | High-mid | High shelf |
|---|---|---|---|---|
| flat | | | | |
| smile | 80 Hz +6 | | | 10 kHz +5 |
| low-mid cut | | 400 Hz −6 | | |
| presence | | | 3 kHz +6 | |
| low-end cut | 100 Hz −5 | 250 Hz −5 | | |
| scoop | 100 Hz +4 | 600 Hz −6 | | 8 kHz +4 |
| punch | 60 Hz +5 | 350 Hz −4 | 4 kHz +4 | |
| air | | 200 Hz +3 | | 12 kHz +6 |

`--tour` needs no keys, 8 s per step: dry; Transformer + Tube + Tape at Soft, Medium, High, Full (Desk Off); then Desk alone
through its six options.

## port-diff — the mono ports against the Airwindows originals
```
cd port-diff && ./fetch-ref.sh && make      # ref/ and build/ are git-ignored
```
Fails if any case's residual is above −120 dB. Results: `../docs/PROVENANCE_v1.md` G7.

## Build PC
```
rsync -a --delete --exclude build --exclude .DS_Store ./ fw1814-build:oa-glue/   # from openaudio-glue/
ssh fw1814-build 'cd ~/oa-glue && make -C dsp test && make -C tools/port-diff'
```
`~/oa-glue` is Glue's own folder on the build PC; it is separate from `~/oa-build`, so the main project's sync never wipes it.

## glue-probe — read-only Core Audio listing
```
cd glue-probe && cc -mmacosx-version-min=10.13 glue_probe.c -framework CoreAudio -framework CoreFoundation -o glue-probe && ./glue-probe
```
For every device: UID, transport, clock domain, rate, channels, buffer, latency, safety offset. For aggregates: sub-devices,
clock master, drift compensation. It changes nothing (G9).

## glue-clock and glue-lockcheck — two-unit clocking (read-only / silence)
```
cd glue-probe && cc -mmacosx-version-min=10.13 glue_clock.c -framework IOKit -framework CoreFoundation -o glue-clock && ./glue-clock
cc -mmacosx-version-min=10.13 glue_lockcheck.c -framework CoreAudio -framework CoreFoundation -o glue-lockcheck && ./glue-lockcheck 60
```
- `glue-clock` prints every FW1814's `MAClockSource` decoded, plus its lock state.
- `glue-lockcheck` streams silence on every FW1814 at once and compares their measured rates. It measures the bus-derived
  stream rate, not the converters' clocks (G10).

## glue-thd — harmonic distortion per preset
```
clang++ -std=c++17 -O2 -mmacosx-version-min=10.13 -I../dsp glue-thd/glue_thd.cpp ../dsp/build/libgluedsp.a -o glue-thd/glue-thd
glue-thd/glue-thd [rate]
```
THD (harmonics 2–10), H2 and H3 of a 1 kHz sine at −18/−12/−6/0 dBFS for the strongest presets. Results: `../docs/DSP_v1.md`.
