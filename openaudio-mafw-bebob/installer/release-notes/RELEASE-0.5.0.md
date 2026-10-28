Driver and control panel for three M-Audio FireWire interfaces — the **FireWire 1814**, the **FireWire Audiophile** and the **FireWire 410** — in two editions: **Classic** for macOS 10.13 to 10.15 and **Modern** for macOS 11 to 15 on Intel Macs.

> **Homebrew and experimental.** The kernel extension is unsigned and stays unsigned. Only for owners of M-Audio FireWire hardware. Nothing is promised or supported.

## What is new in 0.5.0

**Panel 2.0: the Advanced window for the FireWire 1814** (at 44.1 and 48 kHz; status menu › *Advanced Panel…*, or *View › Advanced*, ⌥⌘A). Four modes, each with its own mixer scene per unit; switching asks first when an output would get louder:

- **DAW:** big input and output meters, monitor level on the front knob, speakers A / B, the two phones.
- **Monitor:** direct monitoring, mono first. Mix 1/2 and Mix 3/4 are each a stereo mix or two independent mono lines (Out 1 / 2, Out 3 / 4), the panel panning each source to the lines it is IN on; the Aux is a third mix. Solo, stereo link per pair, mono or joined DAW returns, one output card per mono line.
- **Console:** the 1814 as converter for an analog console: unity levels, linear meters with a 0 VU reference, your console's labels, **CUE** (PFL on Phones 1).
- **Hybrid:** outboard loops with send / return meters and their difference.
- Your own modes (＋), hidden channels per mode, the front knob's target in the header.

**The Dual window:** two FireWire 1814s as one aggregate device (32 in / 24 out): DAW and Console views for both units and a setup sheet (rates, word clock A → B, the aggregate). **Not yet tested with a real aggregate.**

Also: one panel window at a time; the Advanced and Dual meters are light on the CPU (one GPU image per meter row).

## Features

- **Interfaces:** FireWire 1814 (ADAT and S/PDIF modes, 44.1 to 192 kHz), FireWire Audiophile (44.1 to 96 kHz) and FireWire 410 (44.1 to 192 kHz, all eight analog outputs at 192 kHz), alone or together on one bus.
- **Audio:** every rate and channel mode, low latency (about 9 ms round trip at 48 kHz / 64 frames), MIDI 1 in / 1 out on each interface through M-Audio's own plug-in.
- **Plug and play:** the interface comes up by itself after power-up, with its saved settings.
- **Control panel:** each interface's own mixer, outputs, phones and hardware pages; presets; Level Controller; stereo pairs as separate CoreAudio devices; follow the external clock rate; status menu in the menu bar; big meter; read-only firmware check; for the FireWire 1814 the Advanced and Dual windows.
- **Installer:** updates any earlier version in place and keeps your settings; Uninstall app on the disk image.

Driver 0.3.1 (unchanged), **panel 2.0**, M-Audio MIDI plug-in 1.11.2 (unmodified).

## Pick your edition

| | **Classic** | **Modern** |
|---|---|---|
| macOS | **10.13 High Sierra to 10.15 Catalina** | **11 Big Sur to 15 Sequoia, Intel only** |
| File | `OA-BeBoB-MAFW-0.5.0-classic.dmg` | `OA-BeBoB-MAFW-0.5.0-modern.dmg` |
| Security | SIP disabled | SIP set so unsigned kexts load, **then Allow in System Settings › Privacy & Security**, then restart |

Each installer accepts only its own macOS range. Not supported: Apple Silicon, macOS 26 Tahoe, macOS 10.12 and earlier.

## Install

1. Quit audio and MIDI applications. Remove the official M-Audio (and Avid/DICE) FireWire drivers; they conflict. An earlier OpenAudio version does not need to be removed: the installer updates it in place and keeps your settings.
2. Set your security level: **Classic** needs SIP disabled (Recovery, `csrutil disable`). **Modern** needs SIP set so unsigned kexts load.
3. Open the `.dmg` of your edition and run **Install OA BeBoB MAFW.pkg**.
4. **Modern only:** when the installer says so, open System Settings › Privacy & Security and press **Allow** (the developer shows as *unidentified*, because the driver is unsigned).
5. Restart, power on the interface, and open **OA BeBoB MAFW** from `/Applications`.

To remove everything, run **Uninstall OA BeBoB MAFW** from the disk image and restart. The package contains M-Audio's CoreMIDI plug-in unmodified (see [`NOTICE.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/NOTICE.md)).

## Known issues

- Do not change the rate or the digital mode while audio is playing.
- Front-panel knobs can lose clicks on fast turns (as with the official driver).
- FireWire 1814: an S/PDIF ↔ ADAT switch interrupts audio for about 4 s.
- After a sleep in which the interface lost power, some routing may need to be set again in the panel.
- Advanced and Dual windows: most interactions checked only in the panel's demo mode; the Dual window not yet used with a real aggregate.

The full list is in the [README](https://github.com/bhsardinha/OA-BeBoB-MAFW#known-issues).

## Downloads

| File | SHA-256 |
|---|---|
| `OA-BeBoB-MAFW-0.5.0-classic.dmg` | `444694db4e6054e927c70e1508ba787fb04c49dc360cc41ce140ec67e2d39b3f` |
| `OA-BeBoB-MAFW-0.5.0-modern.dmg` | `b342f9fe3dcb52c1d358769b16ad30cd528fed501e941f2c3879b6b978bad218` |

License: PolyForm Noncommercial 1.0.0. M-Audio's software, firmware and icons belong to M-Audio / inMusic; this project is independent and not affiliated with them.
