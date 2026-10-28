Driver and control panel for three M-Audio FireWire interfaces — the **FireWire 1814**, the **FireWire Audiophile** and the **FireWire 410** — in two editions: **Classic** for macOS 10.13 to 10.15 and **Modern** for macOS 11 to 15 on Intel Macs.

> **Homebrew and experimental.** The kernel extension is unsigned and stays unsigned. Only for owners of M-Audio FireWire hardware. Nothing is promised or supported.

## What is new in 0.5.1

**Panel 2.1: fixes to the Advanced window for the FireWire 1814.**

- **The Aux is a stereo pair** in the Monitor mode, as the hardware has it: no pan, odd channels on Aux L, even channels on Aux R. Its tile says STEREO and each strip says where it lands. Its default name is now **Aux** (was *Cue C*).
- **M and S in the Aux act in the Aux only:** with the Aux selected, M cuts that source's Aux send (the fader keeps its level) and S hears only the soloed sources in the Aux. Before, they muted or soloed the channel in every mix.
- **Console CUE:** the tooltip says where a cue is heard (an odd channel on the left of Phones 1, an even one on the right; the Aux has no pan).
- **View menu:** *Advanced Panel…* and *Advanced Dual (A + B)…* are hidden when the interfaces connected cannot open them.

The driver is unchanged (0.3.1). Checked with the panel's logic tests and demo snapshots; not yet tried by ear on a unit.

## Features

- **Interfaces:** FireWire 1814 (ADAT and S/PDIF modes, 44.1 to 192 kHz), FireWire Audiophile (44.1 to 96 kHz) and FireWire 410 (44.1 to 192 kHz, all eight analog outputs at 192 kHz), alone or together on one bus.
- **Audio:** every rate and channel mode, low latency (about 9 ms round trip at 48 kHz / 64 frames), MIDI 1 in / 1 out on each interface through M-Audio's own plug-in.
- **Plug and play:** the interface comes up by itself after power-up, with its saved settings.
- **Control panel:** each interface's own mixer, outputs, phones and hardware pages; presets; Level Controller; stereo pairs as separate CoreAudio devices; follow the external clock rate; status menu in the menu bar; big meter; read-only firmware check; for the FireWire 1814 the Advanced and Dual windows.
- **Installer:** updates any earlier version in place and keeps your settings; Uninstall app on the disk image.

Driver 0.3.1 (unchanged), **panel 2.1**, M-Audio MIDI plug-in 1.11.2 (unmodified).

## Pick your edition

| | **Classic** | **Modern** |
|---|---|---|
| macOS | **10.13 High Sierra to 10.15 Catalina** | **11 Big Sur to 15 Sequoia, Intel only** |
| File | `OA-BeBoB-MAFW-0.5.1-classic.dmg` | `OA-BeBoB-MAFW-0.5.1-modern.dmg` |
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
- Advanced and Dual windows: most interactions checked only in the panel's demo mode (the 0.5.1 Aux changes included); the Dual window not yet used with a real aggregate.

The full list is in the [README](https://github.com/bhsardinha/OA-BeBoB-MAFW#known-issues).

## Downloads

| File | SHA-256 |
|---|---|
| `OA-BeBoB-MAFW-0.5.1-classic.dmg` | `c9dc50ed988a51221d973cebc4503a57e9ed9dfbfe79b32074afd1bc78f114a8` |
| `OA-BeBoB-MAFW-0.5.1-modern.dmg` | `c15629ddda119be677dcce3b86746a5744d8001d4ca0eb4f3c5702dc36d0ef15` |

License: PolyForm Noncommercial 1.0.0. M-Audio's software, firmware and icons belong to M-Audio / inMusic; this project is independent and not affiliated with them.
