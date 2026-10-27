# OpenAudio BeBoB MAFW

**A native driver, control panel and installer for the M-Audio FireWire 1814, FireWire Audiophile and FireWire 410 — on macOS 10.13 to 10.15 (Classic edition) and macOS 11 to 15 on Intel Macs (Modern edition).**

These are BridgeCo BeBoB FireWire interfaces that M-Audio stopped supporting years ago. The last official driver (1.11.x, 2014) kernel-panics on macOS 10.13.6, and its Carbon control panel is 32-bit only. OpenAudio BeBoB MAFW is a from-scratch replacement — a 64-bit kernel extension, a helper daemon, a modern AppKit control panel (**OA BeBoB MAFW**) and an installer — built by reverse engineering the official driver and checking its behaviour against the real hardware. It succeeds **OpenAudio FW1814 0.1.0** (FireWire 1814 only), which its installer replaces.

<p align="center"><img src="docs/images/panel-mixer.png" width="820" alt="Mixer page of the control panel with a FireWire 1814"></p>

> **Who this is for.** Only people who own an **M-Audio FireWire 1814, FireWire Audiophile or FireWire 410**. It does nothing without the interface. The original M-Audio software belongs to M-Audio / inMusic — see [`NOTICE.md`](NOTICE.md).

> **Homebrew and experimental.** A hobby project, not a product: the kernel extension is **unsigned and will stay unsigned**, so it runs only with **System Integrity Protection disabled**, and nothing is promised or supported.

> **Current version: 0.4.1** (driver 0.3.1, panel 1.7) — [download](https://github.com/bhsardinha/OA-BeBoB-MAFW/releases/tag/v0.4.1), [changelog](openaudio-mafw-bebob/CHANGELOG.md).

## Interfaces

| | **FireWire 1814** | **FireWire Audiophile** | **FireWire 410** |
|---|---|---|---|
| Channels | 18 in / 14 out (ADAT mode), 10 in / 6 out (S/PDIF mode) | 4 in / 6 out | 4 in / 10 out |
| Sample rates | 44.1 to 192 kHz | 44.1 to 96 kHz | 44.1 to 192 kHz |
| At 176.4 / 192 kHz | 2 in / 4 out, analog only | — | 2 in / 8 out, all eight analog outputs |
| Digital I/O | ADAT or S/PDIF (optical), coaxial S/PDIF, word clock | coaxial S/PDIF | S/PDIF, coaxial or optical input (output on both) |
| Mixer | aux bus, two phones outputs with A/B | aux bus, phones with A/B switch | aux bus, phones from any combination of the output buses and the aux bus |
| Front panel | Level Controller, phones knobs, A/B | Level Controller, phones knob, A/B switch and LEDs | Level Controller (assignable to five buses) |
| MIDI | 1 in / 1 out | 1 in / 1 out | 1 in / 1 out |

Each interface has its own panel layout, drawn from the official panel's controls. Several units can be used together on one FireWire bus. The driver is one kernel extension: a shared base plus one small module per model, structured like the official driver's class family ([`docs/ARCHITECTURE_MAFW_v1.md`](docs/ARCHITECTURE_MAFW_v1.md)).

## Screenshots

<table>
<tr>
<td align="center"><img src="docs/images/panel-fw410-mixer.png" width="400" alt="FW410 mixer page"><br><sub>FireWire 410: mixer</sub></td>
<td align="center"><img src="docs/images/panel-fw410-output.png" width="400" alt="FW410 output page"><br><sub>FireWire 410: outputs and phones</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/images/panel-fw410-hardware.png" width="400" alt="FW410 hardware page"><br><sub>FireWire 410: hardware page (S/PDIF input, CoreAudio devices)</sub></td>
<td align="center"><img src="docs/images/panel-fw410-about.png" width="400" alt="FW410 about page"><br><sub>FireWire 410: about page</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/images/panel-audiophile-mixer.png" width="400" alt="Audiophile mixer page"><br><sub>FireWire Audiophile: mixer</sub></td>
<td align="center"><img src="docs/images/panel-audiophile-output.png" width="400" alt="Audiophile output page"><br><sub>FireWire Audiophile: outputs</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/images/panel-output.png" width="400" alt="FW1814 output page"><br><sub>FireWire 1814: outputs</sub></td>
<td align="center"><img src="docs/images/panel-hardware-0.3.1.png" width="400" alt="FW1814 hardware page"><br><sub>FireWire 1814: hardware page</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/images/status-menu-0.3.1.png" width="220" alt="Status menu of OA BeBoB MAFW"><br><sub>Status menu</sub></td>
<td align="center"><img src="docs/images/big-meter.png" width="400" alt="Big meter"><br><sub>Big meter</sub></td>
</tr>
</table>

---

## Compatibility

Two editions, one source tree. Download the `.dmg` of your edition; each installer accepts only its own macOS range.

| | **Classic** | **Modern** |
|---|---|---|
| **Download** | `…-classic.dmg` | `…-modern.dmg` |
| **macOS** | 10.13 High Sierra to 10.15 Catalina | 11 Big Sur to 15 Sequoia, Intel Macs only |
| **Mac** | An Intel Mac with FireWire, or with Thunderbolt and Apple's Thunderbolt-to-FireWire adapter | the same |
| **Interfaces** | FireWire 1814, FireWire Audiophile and FireWire 410, alone or together | the same |
| **Security** | System Integrity Protection disabled (Recovery, `csrutil disable`) | SIP set so unsigned kexts load, then **Allow** in System Settings › Privacy & Security and a restart |

Not supported: macOS 26 Tahoe, Apple Silicon, and the other models of the official driver (FireWire Solo, Ozonic, ProFire Lightbridge, ProjectMix I/O, NRV10). The differences between the editions are in [`docs/EDITIONS.md`](docs/EDITIONS.md).

## What is in the package

- **Kernel extension** (`com.openaudio.OpenAudioBeBoBMAFW`): finds the interfaces on the FireWire bus, brings them up and publishes a CoreAudio device per unit, with the hardware mixer, peak meters and MIDI ports.
- **Helper daemon** (LaunchDaemon): starts the interface firmware after power-up and remembers your settings per unit, replaying them when the driver attaches. It never writes the interface's flash.
- **Control panel** (`OA BeBoB MAFW.app`): mixer, output, hardware and meter views for each model, presets, a device menu for several units, a menu bar mode, a big meter and a read-only firmware check.
- **MIDI:** the original M-Audio CoreMIDI plug-in, unmodified.
- **Installer:** `Install OA BeBoB MAFW.pkg`, an **Uninstall OA BeBoB MAFW** app, and a `.dmg` holding both. It updates any earlier version in place (settings kept) and asks for a restart.

## Features

### Audio and MIDI

- **Every rate and channel mode** of each interface (see [Interfaces](#interfaces)), including the FireWire 1814's ADAT and S/PDIF modes and 176.4 / 192 kHz.
- **Low latency:** about 9 ms round trip at 48 kHz with a 64-frame buffer; the latency reported to CoreAudio matches the measured one.
- **MIDI** in and out on every interface, through M-Audio's own CoreMIDI plug-in.
- **Plug and play:** the interface comes up by itself after power-up and after a restart, with its saved settings.
- **Several units at once** on one FireWire bus; the panel switches between them.
- **Factory reset** gives the official driver's factory state.

### Stereo pairs as separate CoreAudio devices

Besides the multichannel device, each stereo pair of the interface can also appear as its own 2-channel CoreAudio device — for applications that use only the first two channels of a device, or to send different programs to different outputs. Turn on **Show stereo pairs as separate devices** on the Hardware page (card *CoreAudio Devices*). The names and device IDs are the official driver's, so applications see the same devices (FireWire 410: *Analog 1/2*, *S/PDIF 1/2*, *Analog 3/4 Out*, *5/6 Out*, *7/8 Out*).

### Follow the external clock rate

With the sync source on **External – digital** (S/PDIF), the panel sets the interface to the source's sample rate by itself, whenever nothing is playing through the interface. Setting: **Follow external clock rate** on the Hardware page (and in the status menu), on by default; it also works with the panel hidden or launched at login.

### Mixer and front panel

- **Mixer:** software returns and inputs routed to one or several outputs, levels, pan, mute, solo, aux sends and aux bus, output levels, phones source and level, with each model's own layout.
- **Front panel:** the Level Controller follows its assignment in the panel; phones knobs and A/B switches work as on the official driver.
- **Phones level law** (this project's choice): phones faders dB-linear over 72 dB; phones knob 1.5 dB per click by default, 3 dB or the official law selectable per unit. Outputs, aux out and the mixer use the official console law.

### Status menu

- The panel can live in the **menu bar**, the **Dock** or both, and **launch at login**.
- The icon shows the selected interface: **online**, **muted** or **not locked**, dimmed when nothing is connected.
- Click opens the panel; right-click opens the menu: volume sliders, mute, dim, presets, the big meter, Audio MIDI Setup, the firmware check, sync source, sample rate and follow-external-rate.

### Big meter

- A horizontal peak meter about 80 % of the screen wide, segmented like an old stereo deck: 80 LED blocks over −60 … 0 dBFS, green / amber / red, peak hold and large peak readouts.
- Source: SW 1/2, Out 1/2, Phones or In 1/2 of the selected unit.
- Always on top, pinned to the top or bottom of the screen or free, transparent, click-through. *View › Big Meter* (⌘B).

### Light on the CPU

Meters are GPU-composited layers, redrawn only when a segment changes (the official panel's method): about 2 % of a core on a 2010 iMac with moving meters.

---

## Install

1. Download the `.dmg` of your edition from the [Releases](https://github.com/bhsardinha/OA-BeBoB-MAFW/releases) page: `…-classic.dmg` for macOS 10.13 to 10.15, `…-modern.dmg` for macOS 11 to 15 on Intel.
2. Quit audio and MIDI applications. Remove the official M-Audio (and Avid/DICE) FireWire drivers first; they conflict. An earlier OpenAudio version does **not** need to be removed: the installer replaces it and keeps your settings.
3. Set the security level: **Classic** needs System Integrity Protection disabled (Recovery mode, `csrutil disable`); **Modern** needs SIP set so unsigned kexts load.
4. Open the `.dmg`, run **Install OA BeBoB MAFW.pkg**. (The package contains M-Audio's CoreMIDI plug-in unmodified, so that MIDI works; see [`NOTICE.md`](NOTICE.md).)
5. **Modern only:** when the installer says so, open System Settings › Privacy & Security and press **Allow** (the developer shows as *unidentified*, because the driver is unsigned).
6. **Restart**, power on the interface. It appears in Audio MIDI Setup; the panel **OA BeBoB MAFW** is in `/Applications`.

To remove everything, run **Uninstall OA BeBoB MAFW** from the disk image and restart.

## Limitations

- **macOS 26 Tahoe and later:** no FireWire stack to attach to.
- **Apple Silicon:** not developed (it would also need a new MIDI plug-in).
- **SIP enabled, or a signed or notarised driver:** the driver is unsigned and will stay unsigned.
- **A Mac with no FireWire port and no adapter:** the interfaces are FireWire only. Newer Macs need Apple's Thunderbolt-to-FireWire adapter (and a Thunderbolt-3-to-2 adapter on the newest ones).
- **FireWire 1814 at 176.4 / 192 kHz:** the hardware switches the digital ports and the DSP mixer off; 2 in / 4 out analog only.
- **FireWire 410: its power LED lights or blinks while the unit is switched off** but cabled to a running Mac. That is the interface's low-power mode, in which it keeps passing FireWire through to its second port; no driver can change it.
- **ASIO and a driver-side buffer size:** Windows-only features of the official driver; on macOS the buffer size belongs to the application.
- **Firmware update:** the panel's firmware check only reads. Nothing writes the interface's flash.
- **Word clock:** the rate of a word-clock source is not followed; set it yourself.
- **OS X 10.9 to macOS 10.12:** not done (the Classic installer starts at 10.13). On Mac OS X 10.6.8 to 10.8.5 the official M-Audio drivers work.
- **Other M-Audio BeBoB interfaces and other makers' interfaces:** not implemented.

## Known issues

1. **Do not change the rate or the digital mode while audio is playing.** Stop playback first; the panel's own rate change waits for that, other applications' changes do not.
2. **Front-panel knobs can lose clicks on fast turns:** the interface reports only a direction per read (about every 50 ms), as with the official driver.
3. **FireWire 1814: an S/PDIF ↔ ADAT switch interrupts audio for about 4 s**, and the Audio MIDI Setup sidebar label can stay stale (the real channel counts are correct).
4. **After a sleep in which the interface lost power**, some routing (the phones source on the 1814, the S/PDIF input pans) may need to be set again in the panel.
5. **Rough edges:** diagnostic logging is still compiled in; English only.

## Roadmap

- **Other M-Audio BeBoB interfaces.** Each is a model module (tables, mixer map, status decode) on the shared base — [`docs/ARCHITECTURE_MAFW_v1.md`](docs/ARCHITECTURE_MAFW_v1.md) has the checklist, and the FireWire 410 ([`docs/BUILD_FW410_PLAN_v2.md`](docs/BUILD_FW410_PLAN_v2.md)) is the latest worked example. It needs people who own the hardware: captures, issues and pull requests are welcome.
- **A guide for building the panel on other systems.**

---

## Build from source

You need **macOS 10.13.6 with Xcode 9.4.1** (the last Xcode with the 10.13 SDK).

```sh
cd openaudio-mafw-bebob
make -C kext                 # kext/build/OpenAudioBeBoBMAFW.kext
make -C firmware-helper      # firmware-helper/build/oa-fwhelper
make -C control-panel app    # control-panel/build/OA BeBoB MAFW.app   (make -C control-panel test: logic tests)
scripts/stage-kext.sh        # copy to /tmp and run kextutil -t -n (static check; loads nothing)
tests/host/run.sh            # host tests of the kernel-free parts (frames, tables, mixer maps, converters)
```

- **Never copy an untested kext into `/Library/Extensions`** (a panicking kext there boot-loops).
- **Modern edition:** the kext is built on a macOS 14/15 Mac with the 15.2 SDK (`make -C kext BAND=modern`); see [`docs/EDITIONS.md`](docs/EDITIONS.md).
- **Installer:** it also needs M-Audio's `MAudioFireWireMIDI.plugin` from your own copy of the official 1.11.2 installer, placed in `openaudio-mafw-bebob/installer/assets/` (see the README there); `installer/make-dmg.sh` builds the package and the disk image.
- **Version numbers:** the package has its own number (`VERSION`), the driver and the panel keep theirs (see the top of [`CHANGELOG.md`](openaudio-mafw-bebob/CHANGELOG.md)).
- **Bench tools:** described in [`openaudio-mafw-bebob/tools/README.md`](openaudio-mafw-bebob/tools/README.md).

## Repository map

| Path | What |
|---|---|
| [`openaudio-mafw-bebob/`](openaudio-mafw-bebob/) | **Current code (0.4.x):** `kext/` (`src/base/` + `src/models/FW1814`, `FWAudiophile`, `FW410`), `firmware-helper/`, `control-panel/` (`Sources/Models/<Model>/`), `installer/`, `tools/`, `tests/`, `scripts/`, `CHANGELOG.md`, `VERSION` |
| [`openaudio-fw1814/`](openaudio-fw1814/) | The 0.1.0 tree (FireWire 1814 only), kept as released (tag `v0.1.0`); its `tools/` hold the bench tools |
| [`docs/system-map/`](docs/system-map/README.md) | Reverse-engineering notes: class chain, DCL rings, clocking, AV/C and HSCI commands, mixer model, panel, MIDI plug-in, helper, captures |
| [`docs/system-map/PROVENANCE-RESOLUTIONS.md`](docs/system-map/PROVENANCE-RESOLUTIONS.md) | Dated log of every finding, correction and fix (R1 … R141) |
| [`docs/ARCHITECTURE_MAFW_v1.md`](docs/ARCHITECTURE_MAFW_v1.md) | Driver layers and how a model plugs in |
| [`docs/BUILD_AUDIOPHILE_PLAN_v9.md`](docs/BUILD_AUDIOPHILE_PLAN_v9.md), [`docs/BUILD_FW410_PLAN_v2.md`](docs/BUILD_FW410_PLAN_v2.md) | The plans for the Audiophile and the FireWire 410 |
| [`docs/fw410-manual-reference.md`](docs/fw410-manual-reference.md), [`docs/audiophile-manual-reference.md`](docs/audiophile-manual-reference.md), [`docs/manual-reference.md`](docs/manual-reference.md) | Developer notes on each interface's manual, against the driver |
| [`docs/EDITIONS.md`](docs/EDITIONS.md), [compatibility plan](docs/compatibility/BROAD_COMPATIBILITY_PLAN_v4.md) | Classic vs Modern edition; the plan for macOS 10.9 to 15 |
| [`docs/STATE.md`](docs/STATE.md), [`NEXT_STEPS.md`](docs/project/NEXT_STEPS.md) | Current state and what is next |
| `PROVENANCE-MATRIX*.xlsx` | Project trackers (tasks, per-claim evidence, dashboards; in Portuguese) |

## How it was made

Everything the driver does is traced to evidence: the official 2014 driver (kext, panel, MIDI plug-in, helper, firmware loader) analysed with **Ghidra** and `otool`, plus live **DTrace**, `ioreg` and FCP captures from a Snow Leopard Mac running the official driver against all three interfaces. The wire protocol was cross-checked with [FFADO](https://github.com/ojsl1/libffado), the Linux [snd-bebob](https://github.com/torvalds/linux/tree/master/sound/firewire/bebob) driver, [snd-firewire-ctl-services](https://github.com/alsa-project/snd-firewire-ctl-services) and [macfw](https://github.com/mbprado/macfw). Where the original and the current macOS kernel disagree, the original is mirrored and the difference is commented in the code with what forced it.

Created by Bernardo Sardinha, with Claude (Anthropic).

## Acknowledgements, ownership and license

The original M-Audio FireWire software and firmware, the device icons and product photographs are the property of **M-Audio / inMusic Brands, Inc.** and their licensors (firmware: BridgeCo); this project claims no rights in them and acknowledges them gratefully. It is an independent, **non-commercial** effort for owners of M-Audio FireWire hardware only, and it is not affiliated with or endorsed by M-Audio, inMusic, Avid, Apple or BridgeCo. Details of what M-Audio material appears here, and what does not, are in [`NOTICE.md`](NOTICE.md).

Thanks to the open projects that documented the protocol: [FFADO](https://github.com/ojsl1/libffado), the Linux [snd-bebob](https://github.com/torvalds/linux/tree/master/sound/firewire/bebob) driver, [snd-firewire-ctl-services](https://github.com/alsa-project/snd-firewire-ctl-services) and [macfw](https://github.com/mbprado/macfw).

**License:** [PolyForm Noncommercial 1.0.0](LICENSE) — free to use, change and share for personal and non-profit purposes; commercial use is not permitted. The license covers this project's own work, not M-Audio's material.
