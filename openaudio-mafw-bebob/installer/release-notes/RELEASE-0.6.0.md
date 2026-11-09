Driver and control panel for three M-Audio FireWire interfaces — the **FireWire 1814**, the **FireWire Audiophile** and the **FireWire 410** — in two editions: **Classic** for macOS 10.13 to 10.15 and **Modern** for macOS 11 to 15 on Intel Macs. This release also brings **OpenAudio Glue 1.0.0**, an optional add-on for the FireWire 1814.

> **Homebrew and experimental.** The kernel extension is unsigned and stays unsigned. Only for owners of M-Audio FireWire hardware. Nothing is promised or supported.

## What is new in 0.6.0

**Driver 0.4.0**

- **Fixed: a kernel panic when a second FireWire 1814 was switched on while the first was running.** After every FireWire bus reset the driver re-makes its connections; two of those reconnects could run at once and leave the transmit stream pointing at memory the system had already freed. The official 2014 driver is built the same way. Now one reconnect runs at a time.
- **Audio comes back by itself.** Switching an interface on or off pauses the audio of the one in use for a few seconds, then it continues. Before, the stream stayed dead until the application restarted it.
- **A read-only tap for OpenAudio Glue.** Nothing changes for other applications.

**Panel 2.2**

- A menu item that opens the OpenAudio Glue app, shown when it is installed and a FireWire 1814 is connected.

**Tested** on macOS 10.13.6 with two FireWire 1814s: bus resets issued from software in bursts, idle and while streaming, and the second unit switched on and off by hand. No panic; the stream resumed each time. **The Modern edition of this release was built but not run.**

## New: OpenAudio Glue 1.0.0 (FireWire 1814 only, optional)

Glue gives each FireWire 1814 a second input device, "FireWire 1814 Glue", with the same inputs already processed by an analog-style chain, so the colour is recorded:

`Transformer → Tube → EQ (4 bands) → Compressor → Desk colour → Tape`

- Per unit: transformer, tube and tape in five steps, a desk colour (Neve, API or SSL flavour), and a Drive.
- Per channel: on/off (off is bit-exact), a one-knob compressor with a Source voicing, a 4-band EQ on an XY graph, stereo link.
- Its own app, in the style of the control panel. With two FireWire 1814s it creates one aggregate: processed inputs 1–32, raw inputs 33–64, outputs 1–24.

**Install order:** first `OA-BeBoB-MAFW-0.6.0` and a restart, then `OpenAudio-Glue-1.0.0` of the same edition. The Glue installer stops if the driver is missing; during its install Core Audio restarts and audio drops for a moment. To remove Glue, run its Uninstall app from the Glue disk image.

The colour stages are ports of [Airwindows](https://github.com/airwindows/airwindows) plug-ins by Chris Johnson (MIT). More in [`openaudio-glue/README.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/openaudio-glue/README.md). Used on macOS 10.13.6 with two FireWire 1814s; the Modern edition has not been run.

## Features

- **Interfaces:** FireWire 1814 (ADAT and S/PDIF modes, 44.1 to 192 kHz), FireWire Audiophile (44.1 to 96 kHz) and FireWire 410 (44.1 to 192 kHz, all eight analog outputs at 192 kHz), alone or together on one bus.
- **Audio:** every rate and channel mode, low latency (about 9 ms round trip at 48 kHz / 64 frames), MIDI 1 in / 1 out on each interface through M-Audio's own plug-in.
- **Plug and play:** the interface comes up by itself after power-up, with its saved settings.
- **Control panel:** each interface's own mixer, outputs, phones and hardware pages; presets; Level Controller; stereo pairs as separate CoreAudio devices; follow the external clock rate; status menu in the menu bar; big meter; read-only firmware check; for the FireWire 1814 the Advanced and Dual windows.
- **Installer:** updates any earlier version in place and keeps your settings; Uninstall app on the disk image.

**Driver 0.4.0**, **panel 2.2**, M-Audio MIDI plug-in 1.11.2 (unmodified).

## Pick your edition

| | **Classic** | **Modern** |
|---|---|---|
| macOS | **10.13 High Sierra to 10.15 Catalina** | **11 Big Sur to 15 Sequoia, Intel only** |
| Driver and panel | `OA-BeBoB-MAFW-0.6.0-classic.dmg` | `OA-BeBoB-MAFW-0.6.0-modern.dmg` |
| Glue (optional) | `OpenAudio-Glue-1.0.0-classic.dmg` | `OpenAudio-Glue-1.0.0-modern.dmg` |
| Security | SIP disabled | SIP set so unsigned kexts load, **then Allow in System Settings › Privacy & Security**, then restart |

Each installer accepts only its own macOS range. Not supported: Apple Silicon, macOS 26 Tahoe, macOS 10.12 and earlier.

## Install

1. Quit audio and MIDI applications. Remove the official M-Audio (and Avid/DICE) FireWire drivers; they conflict. An earlier OpenAudio version does not need to be removed: the installer updates it in place and keeps your settings.
2. Set your security level: **Classic** needs SIP disabled (Recovery, `csrutil disable`). **Modern** needs SIP set so unsigned kexts load.
3. Open the `.dmg` of your edition and run **Install OA BeBoB MAFW.pkg**.
4. **Modern only:** when the installer says so, open System Settings › Privacy & Security and press **Allow** (the developer shows as *unidentified*, because the driver is unsigned).
5. Restart, power on the interface, and open **OA BeBoB MAFW** from `/Applications`.
6. **Optional, FireWire 1814:** open the Glue `.dmg` and run **Install OpenAudio Glue.pkg**.

To remove everything, run **Uninstall OA BeBoB MAFW** from the disk image and restart. The package contains M-Audio's CoreMIDI plug-in unmodified (see [`NOTICE.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/NOTICE.md)).

## Known issues

- Do not change the rate or the digital mode while audio is playing.
- Switching an interface on or off interrupts the audio of another one in use for a few seconds.
- Front-panel knobs can lose clicks on fast turns (as with the official driver).
- FireWire 1814: an S/PDIF ↔ ADAT switch interrupts audio for about 4 s.
- After a sleep in which the interface lost power, some routing may need to be set again in the panel.
- Advanced and Dual windows: most interactions checked only in the panel's demo mode.
- Modern edition: this release's driver and Glue were built for macOS 11 to 15 but not run there.

The full list is in the [README](https://github.com/bhsardinha/OA-BeBoB-MAFW#known-issues).

## Downloads

| File | SHA-256 |
|---|---|
| `OA-BeBoB-MAFW-0.6.0-classic.dmg` | `c17ebb3e8ab9c6952ec21a078c31701eae824e7976f5cde45784a94e02079d91` |
| `OA-BeBoB-MAFW-0.6.0-modern.dmg` | `ddb5995239700e8c23fa512c8dfb5555386e67e99ceceaa1680ec3cdb0e119f1` |
| `OpenAudio-Glue-1.0.0-classic.dmg` | `081babed50b692fad1379ffe0fe3e02953fa81af9d2f0c84a3ecee54a89351ca` |
| `OpenAudio-Glue-1.0.0-modern.dmg` | `251146cdc5c9df894ef5d384baa0240dd78352d7be5682db6c12f18fd1108188` |

License: PolyForm Noncommercial 1.0.0. M-Audio's software, firmware and icons belong to M-Audio / inMusic; this project is independent and not affiliated with them. The Airwindows ports in OpenAudio Glue are under the MIT license.
