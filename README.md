# OpenAudio BeBoB MAFW

**A native driver, control panel and installer for the M-Audio FireWire 1814 and FireWire Audiophile on macOS 10.13 High Sierra (Classic edition) and macOS 11 to 15 on Intel (Modern edition).**

The FireWire 1814 (18 in / 14 out) and the FireWire Audiophile (4 in / 6 out) are BridgeCo BeBoB FireWire interfaces that M-Audio stopped supporting years ago. The last official driver (1.11.x, 2014) kernel-panics on macOS 10.13.6, and its Carbon control panel is 32-bit only. OpenAudio BeBoB MAFW is a from-scratch replacement — a 64-bit kernel extension, a helper daemon, a modern AppKit control panel (**OA BeBoB MAFW**) and an installer — built by reverse engineering the official driver and checking every behaviour against the real hardware. It succeeds **OpenAudio FW1814 0.1.0** (FireWire 1814 only), which its installer replaces.

<p align="center"><img src="docs/images/panel-mixer.png" width="820" alt="Mixer page of the control panel with a FireWire 1814"></p>

> **Who this is for.** Only people who own an **M-Audio FireWire 1814 or FireWire Audiophile**. It does nothing without the interface. The original M-Audio software belongs to M-Audio / inMusic — see [`NOTICE.md`](NOTICE.md).

> **Homebrew and experimental.** A hobby project, not a product: the kernel extension is **unsigned and will stay unsigned**, so it runs only with **System Integrity Protection disabled**, and nothing is promised or supported.

> **Status: package 0.3.1** (driver 0.2.2, panel 1.5; see [`CHANGELOG.md`](openaudio-mafw-bebob/CHANGELOG.md)), in two editions (see [Compatibility](#compatibility)). **Classic** (macOS 10.13): tested with **one FireWire 1814 and one FireWire Audiophile, alone and together on the same bus, on an iMac (iMac11,2, native FireWire 800)**, and installed from the disk image on a **MacBook Air (MacBookAir5,1) through Apple's Thunderbolt-to-FireWire adapter**, both on macOS 10.13.6 with SIP disabled; that testing was done up to package 0.2.2, and the 0.3.x Classic packages have not yet been installed on a 10.13 Mac. **Modern** (macOS 11 to 15, Intel): tested on a **MacBook Air (MacBookAir7,2), macOS 14.8.9**, with the Audiophile (plays) and a FireWire 1814 (panel and external-rate follow). Nothing else has been tested. Read [Known issues](#known-issues) before installing.

---

## What it is

| Component | What it does |
|---|---|
| **Kernel extension** (`com.openaudio.OpenAudioBeBoBMAFW`) | Matches the interfaces on the FireWire bus, brings them up (AV/C, CMP, isochronous streams with classic DCL programs) and publishes a CoreAudio device per unit with the hardware mixer properties, peak meters and MIDI ports. One driver for both models, structured like the official driver's class family: a shared base plus one module per model. |
| **Helper daemon** (LaunchDaemon) | Starts the interface firmware after power-up (the Audiophile waits in its bootloader until told to start) and remembers your settings per unit, replaying them when the driver attaches. A unit with no saved settings gets its headphone level lowered to −43 dB. It never writes the interface's flash. |
| **Control panel** (`OA BeBoB MAFW.app`) | Mixer, output, hardware and meter views for each model, presets, level knob, a device menu for several units, an optional **menu bar mode** with a status menu, a segmented **big meter**, and a read-only **firmware check** (is each unit running the official 1.11.2 firmware of its model?). Talks to the driver through registry properties, like the original panel. |
| **MIDI** | The original M-Audio CoreMIDI plug-in is reused **unmodified**. |
| **Installer** | `Install OA BeBoB MAFW.pkg`, a one-window **Uninstall OA BeBoB MAFW** app and a `.dmg` holding both. It **updates any earlier version in place** (0.1.0 or 0.2.x, no manual uninstall; settings kept) and asks for a restart; nothing is unloaded while it runs. |

### What works (checked on the interfaces)

**FireWire 1814:** 16 in / 12 out (ADAT), 10 in / 6 out (S/PDIF), 2 in / 4 out analog at 176.4 / 192 kHz (the digital ports are off at those rates — a hardware limit); all six rates; panel, saved settings, MIDI plug-in; real round-trip latency 9.3 ms at 48 kHz / 64 frames (analog loopback).

**FireWire Audiophile**
- **4 in / 6 out at 44.1 / 48 / 88.2 / 96 kHz**; channel order confirmed with a tone per channel and the device's own meters.
- **Round-trip latency 9.1 ms at 48 kHz / 64 frames** (analog loopback), matching what the driver reports.
- **Panel**: three software-return strips, two input strips, aux bus, outputs, phones, routing, sync, headphone A/B source matrix; a factory reset gives exactly the official driver's factory state.
- **Front panel**: Level Controller, headphone knob, A/B switch and its LEDs.
- **MIDI**: the M-Audio plug-in lists the Audiophile, 1 in / 1 out. Settings survive a driver reload.

**Both units on one bus:** both come up, both stream, both recover after the interfaces are power-cycled; the panel switches between them; the firmware check reports both as current.

**Phones level law (this project's choice, not the official one).** The phones faders are dB-linear over 72 dB (centre = −36 dB, bottom = off), and the phones knob moves them 1.5 dB per click by default (3 dB or the official law selectable per unit). Outputs, aux out and the mixer use the official console law, and the knobs move the outputs with the official law.

### Follow the external clock rate (new in 0.3.1)

With the sync source on **External – digital** (S/PDIF), the panel sets the interface to the **source's sample rate by itself**. The interface reports the source's exact rate only while the source is in its current speed range (44.1 / 48 or 88.2 / 96 kHz); when it sees nothing, the panel tries the other range once and goes back if nothing locks there. It changes the rate only while no application plays through the interface; otherwise it shows *Source at 44.1 kHz - follows when playback stops* and waits. Setting: **Follow external clock rate** on the Hardware page (and *Follow External Rate* in the status menu), on by default; it also works with the panel hidden or launched at login. Tested on the Modern edition with a FireWire 1814 and a Line 6 POD X3 on coaxial S/PDIF. Word clock is not followed; optical S/PDIF, ADAT, the Audiophile and the Classic edition are untried. The official driver detects the source's rate the same way but never follows it.

**Light on the CPU:** the meters are GPU-composited layers redrawn only when a segment changes (the official panel's method); with moving meters the panel uses about 2 % of a core on a 2010 iMac.

### Menu bar and big meter (new in 0.2)

<table>
<tr>
<td valign="top" width="300"><img src="docs/images/status-menu.png" width="290" alt="Status menu of OA BeBoB MAFW"></td>
<td valign="top">

**Status menu.** The panel can live in the **menu bar**, the **Dock**, or both, and can **launch at login** (choose under *Show Panel In*). The icon shows the selected interface at a glance — **online**, **muted** or **not locked** (no sync), dimmed when no interface is connected. A click opens the panel; a right click (or control-click) opens the menu, which acts on the unit checked at the top (the list appears only with several units):

- **Open Panel**, outputs and phones volume sliders (each on its fader's law), **mute**, **dim**, **presets** that fit that interface;
- the **big meter**, Audio MIDI Setup and the **firmware check**;
- **sync source**, **Follow External Rate**, **sample rate** (greyed while an application streams), *Show Panel In*, *Launch at Login*;
- the rate and lock state (and the external source's rate when the panel is waiting to follow it).

</td>
</tr>
</table>

<p align="center"><img src="docs/images/big-meter.png" width="820" alt="Big meter"></p>

**Big meter.** A horizontal peak meter about 80 % of the screen wide, segmented like an old stereo deck: **80 LED blocks over −60 … 0 dBFS** (0.75 dB each), green up to −18, amber up to −6, red above, unlit blocks faintly visible, a **peak-hold** block (about 2 s) and large peak readouts. Source: **SW 1/2** (default), **Out 1/2**, **Phones** or **In 1/2** of the selected unit. Right-click it (or use the status menu) for: **always on top**, **pin** to the top or bottom of the screen or move it freely, **transparent**, **click-through**. *View › Big Meter* (⌘B) toggles it. Peak only, from the driver's meter page — no RMS or loudness. Both pictures above are the panel's demo mode on a MacBook Air.

### Panel pages

<table>
<tr>
<td align="center"><img src="docs/images/panel-audiophile-mixer.png" width="400" alt="Audiophile mixer page"><br><sub>FireWire Audiophile: mixer (demo mode render)</sub></td>
<td align="center"><img src="docs/images/panel-audiophile-output.png" width="400" alt="Audiophile output page"><br><sub>FireWire Audiophile: output page</sub></td>
</tr>
<tr>
<td align="center"><img src="docs/images/panel-output.png" width="400" alt="FW1814 output page"><br><sub>FireWire 1814: output page</sub></td>
<td align="center"><img src="docs/images/panel-hardware.png" width="400" alt="FW1814 hardware page"><br><sub>FireWire 1814: hardware page (rate, sync, digital ports, A/B switch)</sub></td>
</tr>
</table>

---

## Compatibility

Two editions, one source tree, both published since 0.3.0 (current: 0.3.1). Download the `.dmg` of your edition; each installer accepts only its own macOS range. What differs, and what is tested on each, is in [`docs/EDITIONS.md`](docs/EDITIONS.md).

| | **Classic** | **Modern** |
|---|---|---|
| **macOS** | **10.13 High Sierra** (tested on 10.13.6; the package also allows 10.14 / 10.15, untried; 64-bit kext, 10.13 SDK, Xcode 9.4.1) | **11 Big Sur to 15 Sequoia, Intel only.** Works on **14.8.9**; the other versions of the range are allowed by the package but untried. No Apple Silicon, no macOS 26 |
| **Mac** | iMac11,2 with native FireWire 800; MacBook Air (MacBookAir5,1) through Apple's **Thunderbolt-to-FireWire adapter**. Other Macs, adapters and FireWire 400 are untested | MacBook Air (MacBookAir7,2) through the same kind of adapter. Nothing else tested |
| **Interfaces** | **M-Audio FireWire 1814 and FireWire Audiophile only**, alone and together | The **FireWire Audiophile** plays; the **FireWire 1814** has been tried for the panel and the external-rate follow only |
| **Security** | **Unsigned, always**; **System Integrity Protection must be disabled** (Recovery, `csrutil disable`) | Unsigned, always; SIP set so unsigned kexts load, then **Allow** in System Settings › Privacy & Security (shown as *unidentified developer*) and a restart |

The other models of the official driver (FireWire 410, FireWire Solo, Ozonic, ProFire Lightbridge, ProjectMix I/O, NRV10) are not implemented; the structure for them exists (see [Roadmap](#roadmap)). Apple Silicon is an idea, not started. Mac OS X 10.6.8 to 10.8.5 is out of scope: the official M-Audio drivers work there. See the [compatibility plan](docs/compatibility/BROAD_COMPATIBILITY_PLAN_v4.md).

## What does not work

Three different reasons, kept apart on purpose.

### Impossible (by hardware, by the OS, or by design)

| What | Why |
|---|---|
| **macOS 26 Tahoe and later** | No FireWire stack to attach to. This is another project's finding ([macfw](https://github.com/mbprado/macfw) states Apple removed it); it has not been checked here. |
| **Apple Silicon with the kernel driver, today** | Not buildable as it stands: the driver has no arm64e build, and M-Audio's MIDI plug-in has no arm64 code. The FireWire and IOAudio kernel families do ship for arm64e (read from Apple's debug kit), but FireWire through the adapter chain has never been tried on such a Mac, so this is *not developed* rather than proven impossible. |
| **Macs with SIP enabled, or a signed or notarised driver** | The driver is unsigned and will stay unsigned (the author's decision), so it only loads with SIP set to allow unsigned kexts. |
| **A Mac with no FireWire port and no adapter** | The interfaces are FireWire only. Newer Macs need Apple's Thunderbolt-to-FireWire adapter (and a Thunderbolt-3-to-2 adapter on the newest ones); only the Thunderbolt 2 adapter chain has been tried. |
| **FireWire 1814 at 176.4 / 192 kHz: digital ports and the DSP mixer** | Switched off by the hardware at those rates: 2 in / 4 out analog only. |
| **ASIO and a driver-side buffer size** | Windows-only features of the official driver; on macOS the buffer size belongs to the application. |

### Not developed (yet)

| What | State |
|---|---|
| **OS X 10.9 to macOS 10.12** | In scope but not done: the Classic installer starts at 10.13. |
| **Apple Silicon** | See above; it would also need a new MIDI plug-in. Needs the hardware to start. |
| **Other M-Audio BeBoB interfaces** (FireWire 410, FireWire Solo, Ozonic, ProFire Lightbridge, ProjectMix I/O, NRV10) | The code is structured for one module per model, but nothing is implemented. It needs people who own the hardware. |
| **Interfaces from other makers** (including DICE-based ones) | Out of scope. |
| **Firmware update from the panel** | The panel's firmware check only reads. Nothing writes the interface's flash, and there is no custom firmware. |
| **Following the rate of a word-clock source** | Only *External – digital* is followed (0.3.1); with word clock you set the rate yourself. |
| **Mac OS X 10.6.8 to 10.8.5** | Out of scope (decided 2026-10-22): the official M-Audio drivers work well there, so this project starts at 10.9. |
| **Windows and Linux** | Not this project (Linux has FFADO and snd-bebob). |

### Written but not tried

These exist in the driver, but nobody has run them, so do not count on them: macOS 10.14 and 10.15 (allowed by the Classic installer); macOS 11, 12, 13 and 15 (allowed by the Modern installer); the **FireWire 1814 on the Modern edition** beyond the panel and the external-rate follow (playback, recording, MIDI); the external-rate follow with optical S/PDIF, ADAT, the Audiophile or on the Classic edition; MIDI SysEx on the wire; AC-3 pass-through on the wire; sleep/wake; two FireWire 1814s on one bus; FireWire 400 and Macs other than those listed under Compatibility.

## Install

1. Download the `.dmg` of your edition from the [Releases](https://github.com/bhsardinha/OA-BeBoB-MAFW/releases) page: `…-classic.dmg` for macOS 10.13, `…-modern.dmg` for macOS 11 to 15 on Intel.
2. Quit audio and MIDI applications. Remove the official M-Audio (and Avid/DICE) FireWire drivers first; they conflict. An earlier OpenAudio version does **not** need to be removed: the installer replaces it and keeps your settings.
3. Set the security level: **Classic** needs System Integrity Protection disabled (Recovery mode, `csrutil disable`); **Modern** needs SIP set so unsigned kexts load.
4. Open the `.dmg`, run **Install OA BeBoB MAFW.pkg**. (The package contains M-Audio's CoreMIDI plug-in unmodified, so that MIDI works; see [`NOTICE.md`](NOTICE.md).)
5. **Modern only:** when the installer says so, open System Settings › Privacy & Security and press **Allow** (the developer shows as *unidentified*, because the driver is unsigned).
6. **Restart**, power on the interface. It appears in Audio MIDI Setup; the panel **OA BeBoB MAFW** is in `/Applications`.

Each installer refuses a macOS outside its range; the Modern one also removes the driver, failing the install, if this Mac lacks a kernel symbol it needs.

To remove everything run **Uninstall OA BeBoB MAFW** from the disk image and restart.

## Build from source

You need **macOS 10.13.6 with Xcode 9.4.1** (the last Xcode with the 10.13 SDK).

```sh
cd openaudio-mafw-bebob
make -C kext                 # kext/build/OpenAudioBeBoBMAFW.kext
make -C firmware-helper      # firmware-helper/build/oa-fwhelper
make -C control-panel app    # control-panel/build/OA BeBoB MAFW.app   (make -C control-panel test: logic tests)
scripts/stage-kext.sh        # copy to /tmp and run kextutil -t -n (static check; loads nothing)
```

Never copy an untested kext into `/Library/Extensions` (a panicking kext there boot-loops). The installer also needs M-Audio's `MAudioFireWireMIDI.plugin` from your own copy of the official 1.11.2 installer, placed in `openaudio-mafw-bebob/installer/assets/` (see the README there); `installer/make-dmg.sh` builds the package and the disk image. Version numbers: the package has its own number (`VERSION`), the driver and the panel keep theirs (see the top of [`CHANGELOG.md`](openaudio-mafw-bebob/CHANGELOG.md)). Bench tools are described in [`openaudio-mafw-bebob/tools/README.md`](openaudio-mafw-bebob/tools/README.md).

---

## Known issues

1. **Do not change the rate or the digital mode while audio is playing.** The original driver panics there; this driver gates only the panel's own rate change. Stop playback first.
2. **Never unload the driver while a unit is attached** (`kextunload`): the official design does not free a FireWire resource on unload, and a later bus reset can panic. This driver frees it, but the installer and uninstaller still never unload — they ask for a restart.
3. **Front-panel knobs can lose clicks on fast turns** — the interface reports only a direction per read (about every 50 ms), as with the official driver.
4. **FW1814: the Audio MIDI Setup sidebar label can be stale** after an S/PDIF ↔ ADAT switch (the real channel counts are correct), and the switch interrupts audio for about 4 s.
5. **The external-rate follow waits while audio plays** (by design: see item 1). A source outside the interface's speed range takes several seconds to be found (one try of the other range). Word clock is not followed. With *Follow external clock rate* off, set the source's rate yourself, as before 0.3.1.
6. **Not tested** (no formal test plan; these get looked at if someone hits a problem): MIDI bytes on the wire and SysEx, AC-3 pass-through on the wire, sleep/wake, streaming for hours, two FW1814s on one bus, the full 0.1.0 test ladder of the FW1814 on the new driver. Updating in place, the Uninstall app and the status menu have been used on a real install.
7. **No firmware update.** The driver starts the firmware already in the interface; the panel's firmware check only reads it. Nothing writes flash.
8. **FW1814 at 176.4 / 192 kHz:** digital ports and the DSP mixer are off (hardware); a loopback of analog inputs at 192 kHz read silence once, unexplained.
9. **Rough edges:** diagnostic logging is still compiled in; English only.

## Roadmap

- **Other M-Audio BeBoB interfaces.** Each is a model module (tables, mixer map, status decode) on the shared base — [`docs/ARCHITECTURE_MAFW_v1.md`](docs/ARCHITECTURE_MAFW_v1.md) has the checklist. It needs people who own the hardware: captures, issues and pull requests are welcome.
- A guide for building the panel on other systems.

---

## Repository map

| Path | What |
|---|---|
| [`openaudio-mafw-bebob/`](openaudio-mafw-bebob/) | **Current code (0.3.x):** `kext/` (`src/base/` + `src/models/<Model>/`), `firmware-helper/`, `control-panel/`, `installer/`, `tools/`, `tests/`, `scripts/`, `CHANGELOG.md`, `VERSION` |
| [`openaudio-fw1814/`](openaudio-fw1814/) | The 0.1.0 tree (FireWire 1814 only), kept as released (tag `v0.1.0`) |
| [`docs/system-map/`](docs/system-map/README.md) | Reverse-engineering notes: class chain, DCL rings, clocking, AV/C and HSCI commands, mixer model, panel, MIDI plug-in, helper, captures |
| [`docs/system-map/PROVENANCE-RESOLUTIONS.md`](docs/system-map/PROVENANCE-RESOLUTIONS.md) | Dated log of every finding, correction and test result (R1 … R131) |
| [`docs/ARCHITECTURE_MAFW_v1.md`](docs/ARCHITECTURE_MAFW_v1.md), [`docs/BUILD_AUDIOPHILE_PLAN_v9.md`](docs/BUILD_AUDIOPHILE_PLAN_v9.md) | Driver layers and how a model plugs in; the 0.2 plan |
| [`docs/EDITIONS.md`](docs/EDITIONS.md), [compatibility plan](docs/compatibility/BROAD_COMPATIBILITY_PLAN_v4.md) | Classic vs Modern edition: what differs and what is tested; the plan for macOS 10.6.8 to 15 |
| [`docs/STATE.md`](docs/STATE.md), [`NEXT_STEPS.md`](docs/project/NEXT_STEPS.md) | Current state and what is next |
| `PROVENANCE-MATRIX*.xlsx` | Project trackers (tasks, per-claim evidence, dashboards; in Portuguese) |

## How it was made

Everything the driver does is traced to evidence: the official 2014 driver (kext, panel, MIDI plug-in, helper, firmware loader) analysed with **Ghidra** and `otool`, plus live **DTrace**, `ioreg` and FCP captures from a Snow Leopard Mac running the official driver against both interfaces. The wire protocol was cross-checked with [FFADO](https://github.com/ojsl1/libffado), the Linux [snd-bebob](https://github.com/torvalds/linux/tree/master/sound/firewire/bebob) driver, [snd-firewire-ctl-services](https://github.com/alsa-project/snd-firewire-ctl-services) and [macfw](https://github.com/mbprado/macfw). Where the original and the 10.13 kernel disagree, the original is mirrored and the difference is commented in the code with what forced it.

Created by Bernardo Sardinha, with Claude (Anthropic).

## Acknowledgements, ownership and license

The original M-Audio FireWire software and firmware, the device icons and product photographs are the property of **M-Audio / inMusic Brands, Inc.** and their licensors (firmware: BridgeCo); this project claims no rights in them and acknowledges them gratefully. It is an independent, **non-commercial** effort for owners of M-Audio FireWire hardware only, and it is not affiliated with or endorsed by M-Audio, inMusic, Avid, Apple or BridgeCo. Details of what M-Audio material appears here, and what does not, are in [`NOTICE.md`](NOTICE.md).

Thanks to the open projects that documented the protocol: [FFADO](https://github.com/ojsl1/libffado), the Linux [snd-bebob](https://github.com/torvalds/linux/tree/master/sound/firewire/bebob) driver, [snd-firewire-ctl-services](https://github.com/alsa-project/snd-firewire-ctl-services) and [macfw](https://github.com/mbprado/macfw).

**License:** [PolyForm Noncommercial 1.0.0](LICENSE) — free to use, change and share for personal and non-profit purposes; commercial use is not permitted. The license covers this project's own work, not M-Audio's material.
