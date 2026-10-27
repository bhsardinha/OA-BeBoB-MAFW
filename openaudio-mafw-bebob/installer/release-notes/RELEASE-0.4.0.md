Driver and control panel for three M-Audio FireWire interfaces — the **FireWire 1814**, the **FireWire Audiophile** and, new in this release, the **FireWire 410** — in two editions: **Classic** for macOS 10.13 to 10.15 and **Modern** for macOS 11 to 15 on Intel.

> **Homebrew and experimental.** The kernel extension is unsigned and stays unsigned. Only for owners of M-Audio FireWire hardware. Nothing is promised or supported. Read *Known issues* before installing.

## What is new

### The FireWire 410

- **4 in / 10 out at 44.1 to 96 kHz, 2 in / 8 out at 176.4 / 192 kHz** (all eight analog outputs stream at 192 kHz), MIDI 1 in / 1 out.
- **Its own panel:** five software-return strips and the analog / S/PDIF inputs with routing to **one or several outputs at once**, five outputs with main/aux, the aux bus, phones from **any combination** of the output buses — and from the aux bus, which the official panel never offered — the S/PDIF input (coaxial or optical) and the Level Controller on five buses.
- **Checked on the Classic edition** (iMac, macOS 10.13.6, the 410 alone on the bus): it comes up by itself, every output channel reaches its own output, recording works at every rate, the reported latency is within 0.5 ms of a measured loopback (8.5 ms round trip at 48 kHz / 64 frames), the mixer and the Level Controller work, settings are kept per unit.
- **Not tried yet:** MIDI, S/PDIF and external sync, the coaxial/optical switch, sleep/wake, the Modern edition.

### Stereo pairs as separate CoreAudio devices (every model)

**Show stereo pairs as separate devices** (Hardware page, *CoreAudio Devices*): each stereo pair also appears as its own 2-channel device, next to the multichannel one. With the FireWire 410 the devices and their IDs are the same as with the official driver. Untried on the FireWire 1814 and the Audiophile.

### Fixes (every model)

- **Unloading the driver no longer leaves a FireWire plug registered.** Before, a later bus reset — switching an interface off — after an unload could make the Mac panic. The installer and uninstaller never unload, so a normal install was not affected; manual `kextunload` was.
- **`kextunload` succeeds on the first try** (it could need a second try).
- **Wake from sleep:** the freeze (kernel panic) on wake when the interface had dropped off the bus, found with 0.3.1, is fixed; after a sleep with power loss the mixer comes back (some routing may need to be set again).

**Driver 0.3.0**, **panel 1.6**, M-Audio MIDI plug-in 1.11.2 (unmodified).

## Pick your edition

| | **Classic** | **Modern** |
|---|---|---|
| macOS | **10.13 High Sierra to 10.15 Catalina** | **11 Big Sur to 15 Sequoia, Intel only** |
| File | `OA-BeBoB-MAFW-0.4.0-classic.dmg` | `OA-BeBoB-MAFW-0.4.0-modern.dmg` |
| Security | SIP disabled | SIP set so unsigned kexts load, **then Allow in System Settings › Privacy & Security**, then restart |
| Tested | iMac11,2 (FireWire 800), macOS 10.13.6: **FireWire 410** with this package; FW1814 and Audiophile, alone and together, up to 0.2.2 (not yet re-tested on 0.4.0) | **This package not yet tried.** 0.3.1 ran on a MacBook Air 7,2 + Thunderbolt-to-FireWire adapter, macOS 14.8.9, with the Audiophile (plays) and the FW1814 (panel, external-rate follow) |

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
- **Not tried:** this release's Modern package; the FireWire 410 on Modern, with MIDI, S/PDIF / external sync and sleep/wake; the FireWire 1814 and the Audiophile on 0.4.0; stereo pairs on the 1814 and the Audiophile.
- FireWire 410: its power LED lights or blinks while it is switched off but cabled to a running Mac — the interface's own low-power mode, not something a driver can change.
- Front-panel knobs can lose clicks on fast turns.
- No firmware update: the panel's firmware check only reads. Nothing writes the interface's flash.

Full list: the README. Changes in detail: [`CHANGELOG.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/openaudio-mafw-bebob/CHANGELOG.md). Evidence: [`PROVENANCE-RESOLUTIONS.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/docs/system-map/PROVENANCE-RESOLUTIONS.md) (R135-R140).

## Downloads

| File | SHA-256 |
|---|---|
| `OA-BeBoB-MAFW-0.4.0-classic.dmg` | `965518e254231690dededd51fdfe9d1a09f08398f7dc4d7fd7ad66bdcb91f2eb` |
| `OA-BeBoB-MAFW-0.4.0-modern.dmg` | `20d2f07ef9525bb94e06369ac72ec13a39a7d0862cb0e45d5b141d95068e3681` |

License: PolyForm Noncommercial 1.0.0. M-Audio's software, firmware and icons belong to M-Audio / inMusic; this project is independent and not affiliated with them.
