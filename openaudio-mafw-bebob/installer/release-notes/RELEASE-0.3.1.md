Driver and control panel for the **M-Audio FireWire 1814** and **FireWire Audiophile**, in two editions: **Classic** for macOS 10.13 and **Modern** for macOS 11 to 15 on Intel.

> **Homebrew and experimental.** The kernel extension is unsigned and stays unsigned. Only for owners of M-Audio FireWire hardware. Nothing is promised or supported. Read *Known issues* before installing.

## What is new: follow the external clock rate

With the sync source on **External – digital** (S/PDIF), the panel now sets the interface to the **source's sample rate by itself**.

- The interface reports the source's exact rate while the source is in its current speed range (44.1 / 48 or 88.2 / 96 kHz), and the panel switches to it.
- When it sees nothing, the panel tries the other speed range once (48 ↔ 96 kHz), and goes back if nothing locks there.
- It changes the rate only while no application plays through the interface. During playback it shows *Source at 44.1 kHz - follows when playback stops* and follows as soon as playback stops.
- Setting: **Follow external clock rate** on the Hardware page, and **Follow External Rate** in the status menu. On by default. It also works with the panel hidden or launched at login.
- Tested on the **Modern** edition (macOS 14.8.9) with a **FireWire 1814** and a Line 6 POD X3 on coaxial S/PDIF.
- Word clock is not followed. Optical S/PDIF, ADAT, the Audiophile and the Classic edition are untried.

The official M-Audio driver detects the source's rate the same way but never follows it.

**Driver 0.2.2** publishes the rate the interface reports (`MAExternalSyncRate`); everything else behaves as in 0.2.1. **Panel 1.5** adds the follower and its setting.

Also new for the Modern edition: the **FireWire 1814** now installs, loads and runs the panel on macOS 14.8.9 (playback, recording and MIDI on Modern are still untried with the 1814).

## Pick your edition

| | **Classic** | **Modern** |
|---|---|---|
| macOS | **10.13 High Sierra** | **11 Big Sur to 15 Sequoia, Intel only** |
| File | `OA-BeBoB-MAFW-0.3.1-classic.dmg` | `OA-BeBoB-MAFW-0.3.1-modern.dmg` |
| Security | SIP disabled | SIP set so unsigned kexts load, **then Allow in System Settings › Privacy & Security**, then restart |
| Tested | iMac11,2 (FireWire 800) and MacBook Air 5,1 + Thunderbolt-to-FireWire adapter, macOS 10.13.6; FW1814 and Audiophile, alone and together (up to 0.2.2; the 0.3.x Classic package itself not yet installed on a 10.13 Mac) | MacBook Air 7,2 + Thunderbolt-to-FireWire adapter, **macOS 14.8.9**; Audiophile (plays) and FW1814 (panel, external-rate follow) |

Each installer accepts only its own macOS range. Not supported: Apple Silicon, macOS 26 Tahoe, macOS 10.12 and earlier.

## Install

1. Quit audio and MIDI applications. Remove the official M-Audio (and Avid/DICE) FireWire drivers; they conflict. An earlier OpenAudio version does not need to be removed: the installer updates it in place and keeps your settings.
2. Set your security level: **Classic** needs SIP disabled (Recovery, `csrutil disable`). **Modern** needs SIP set so unsigned kexts load.
3. Open the `.dmg` of your edition and run **Install OA BeBoB MAFW.pkg**.
4. **Modern only:** when the installer says so, open System Settings › Privacy & Security and press **Allow** (the developer shows as *unidentified*, because the driver is unsigned).
5. Restart, power on the interface, and open **OA BeBoB MAFW** from `/Applications`.

To remove everything, run **Uninstall OA BeBoB MAFW** from the disk image and restart. The package contains M-Audio's CoreMIDI plug-in unmodified (see [`NOTICE.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/NOTICE.md)).

## Known issues

- Do not change the rate or the digital mode while audio is playing (the external-rate follow waits for that reason).
- **Modern, not tried:** MIDI on the wire, sleep/wake, macOS 11, 12, 13 and 15, the uninstaller on macOS 11+, playback and recording with the FW1814.
- Front-panel knobs can lose clicks on fast turns.
- No firmware update: the panel's firmware check only reads. Nothing writes the interface's flash.
- Not tested: MIDI SysEx, AC-3 on the wire.

Full list: the README. Changes in detail: [`CHANGELOG.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/openaudio-mafw-bebob/CHANGELOG.md). Evidence: [`PROVENANCE-RESOLUTIONS.md`](https://github.com/bhsardinha/OA-BeBoB-MAFW/blob/main/docs/system-map/PROVENANCE-RESOLUTIONS.md) (R131).

## Downloads

| File | SHA-256 |
|---|---|
| `OA-BeBoB-MAFW-0.3.1-classic.dmg` | `23dec4bb1965369f24c8e56feea19bcdb14aaafa60db1572e48a36750b8a2f24` |
| `OA-BeBoB-MAFW-0.3.1-modern.dmg` | `97db4d6f660ac5b76c72fd3eeb8aefc5399cf055f800b9f3306cd9d4d4608bf2` |

License: PolyForm Noncommercial 1.0.0. M-Audio's software, firmware and icons belong to M-Audio / inMusic; this project is independent and not affiliated with them.
