# OpenAudio Glue

A spin-off of OpenAudio BeBoB MAFW. Glue gives each M-Audio FireWire interface a second Core Audio **input** device ("FireWire
1814 Glue"). That device carries the same inputs as the interface, already processed by an analog-style chain, so the colour is
printed while you record. It is aimed at the control panel's Console mode: analog console → FW1814 at unity → DAW.

**Status: 1.0.0, published 2026-11-09 with OpenAudio BeBoB MAFW 0.6.0.** For the **FireWire 1814 only**. It runs on macOS 10.13.6
High Sierra with two FW1814s on the build iMac: both Glue devices are live; with every stage off the output is bit-identical to
the interface's own inputs; the app's settings reach the device (the user's report of 2026-11-02, and use since). The **Modern
edition (macOS 11 to 15, Intel) is built from the same binaries but has not been run.** Details: [`docs/STATE.md`](docs/STATE.md).

**Download:** `OpenAudio-Glue-1.0.0-classic.dmg` (macOS 10.13 to 10.15) or `OpenAudio-Glue-1.0.0-modern.dmg` (macOS 11 to 15,
Intel) from the project's [release v0.6.0](https://github.com/bhsardinha/OA-BeBoB-MAFW/releases/tag/v0.6.0). Install the main
package 0.6.0 first and restart; the Glue installer stops if the driver (0.4.0 or later) is missing.

## The chain (per input channel)

```
Transformer → Tube → EQ (4 bands) → Compressor → Desk colour → Tape
```

| Stage | Control | Scope |
|---|---|---|
| Transformer, Tube, Tape | Off / Soft / Medium / High / Full | global (all channels of a unit) |
| Desk colour | Off / Neve Medium / Neve High / API Medium / API High / SSL Medium / SSL High | global |
| Drive | 0…+18 dB more into every colour stage, taken back after it (the level stays about the same) | global |
| Compressor | threshold (−40…0 dB, 0 = off) and a Source (General, Drums, Vocals, Heavy guitar, Bass, Synth); fixed ratio and knee, automatic attack and release | per channel |
| EQ | low shelf, low-mid, high-mid, high shelf; each band is a dot on an XY graph (frequency, gain); shelves have a fixed slope, the mids proportional Q | per channel |
| ON | off = bit-exact pass-through, delay-compensated like the processed channels | per channel |
| Stereo link | the even channel follows the odd one; the compressors are linked | per pair |

The four colour stages are ports of [Airwindows](https://github.com/airwindows/airwindows) plug-ins (MIT): Interstage, Tube2,
Channel9 and ToTape9. The compressor and EQ are this project's own code. Credits are in [`NOTICE.md`](NOTICE.md). Measured
values (presets, THD, delay) are in [`docs/DSP_v1.md`](docs/DSP_v1.md).

## How it fits

- The Glue device is an AudioServerPlugIn (`/Library/Audio/Plug-Ins/HAL/OpenAudioGlue.driver`) that runs inside Core Audio.
- It reads the interface's input samples straight from the OpenAudio kext through a read-only shared-memory tap, and runs on the
  interface's own clock and timestamps. It needs an OpenAudio driver with the tap (driver 0.4.0 or later); the installer checks.
- The plain interface device is unchanged.
- In the DAW, use an aggregate device: Glue for inputs, the interface for outputs, with the interface as clock source.
- **Two FW1814s (Dual):** the app creates "Dual FireWire 1814 Glue" = Glue A, Glue B, FW1814 A, FW1814 B. Inputs 1–32 are the
  processed A + B inputs, 33–64 the raw ones (a safety copy), outputs 1–24 A then B.

## The app

`OpenAudio Glue.app` is its own application, in the style of the OpenAudio control panel. It has a tab per Glue device, a
bypass, the global stage card and a matrix of channel cards, six per row. Cards rename with a double-click or right-click; the
**Names** menu applies band templates (16 inputs, or 32 for Dual), which can also set each channel's Source, ON and stereo links.

## Install and remove

The disk image holds **Install OpenAudio Glue.pkg** and an **Uninstall** app. The package installs the plug-in, the app in
`/Applications` and an uninstaller in `/Library/Application Support/OpenAudio Glue/`, then restarts Core Audio (audio drops for a
moment). It never touches the main OpenAudio package's files.

## Layout

| Folder | Contents |
|---|---|
| `dsp/` | the processing chain (plain C++, no dependencies) and its host tests |
| `driver/` | the AudioServerPlugIn and its host tests (against a simulated tap) |
| `panel/` | the OpenAudio Glue app (Swift 4.1) and its tests |
| `tools/` | bench, port comparison, THD, voicing, listening and hardware-probe tools (recipes in `tools/README.md`) |
| `installer/` | its own `.pkg` and uninstaller |
| `docs/` | plan, architecture, DSP notes, provenance (G-numbers), state |

The only hook outside this folder is the kext tap in `openaudio-mafw-bebob/kext/` (main R-log R143). The main control panel (2.2) has a menu item that
opens the app. See [`docs/ARCHITECTURE_v1.md`](docs/ARCHITECTURE_v1.md).
