# Changelog — OpenAudio BeBoB MAFW

## Version numbers (user, 2026-10-21)

| Number | Where it lives | Changes when |
|---|---|---|
| **Package** | `VERSION` | every release; names the `.pkg`, the `.dmg` and the entries below; also shown on the panel's About tab and the installer pages; the Uninstall app carries it |
| **Driver** (kext + helper) | `kext/Info.plist` (`CFBundleShortVersionString`, `CFBundleVersion`) and `kext/src/base/kmod.cpp` | only when the kext or the helper changes |
| **Panel** | `control-panel/Info.plist` | only when the panel changes (since 0.2.2 it includes the firmware check, formerly the separate Firmware Tool 0.1) |

The M-Audio CoreMIDI plug-in is shipped unmodified (1.11.2). Each entry lists the component versions it contains; evidence is in `docs/system-map/PROVENANCE-RESOLUTIONS.md` (R-numbers).

---

## Package 0.3.1 — 2026-10-22

| Driver | Panel | MIDI plug-in | Editions |
|---|---|---|---|
| **0.2.2** | **1.5** | 1.11.2 | Classic and Modern, as 0.3.0 |

**Follow the external clock rate** (R131, `docs/RATE_FOLLOW_PLAN_v1.md`)
- **Driver 0.2.2:** publishes `MAExternalSyncRate`, the rate of the external source as the interface reports it (lock bit + rate field of the status packet; 0 when nothing is seen). The interface reports the source's exact rate while it is in the current speed range (44.1/48 or 88.2/96 kHz; capture 98). `MASampleRate` and `MARateLocked` behave as before.
- **Panel 1.5:** with the sync source on **External – digital**, the panel sets the interface to the source's rate through CoreAudio, only while no application plays through it (otherwise it shows "Source at … kHz - follows when playback stops" and follows when playback stops). When it sees nothing it tries the other speed range once (48 ↔ 96 kHz) and goes back if nothing locks. Setting **Follow external clock rate** in the Hardware tab and **Follow External Rate** in the status menu, on by default. It also runs when the panel is hidden (launch at login). Word clock is not followed.
- **Tested** with the Modern package on macOS 14.8.9 with a FireWire 1814 and a POD X3 on coaxial S/PDIF (user, 2026-10-22, capture 99): out of range it switches speed range, in range it takes the exact rate and locks. The official driver detects the rate the same way but never follows it.

## Package 0.3.0 — 2026-10-21 (published 2026-10-22, release v0.3.0)

| Driver | Panel | MIDI plug-in | Editions |
|---|---|---|---|
| **0.2.1** | **1.4** | 1.11.2 | **Classic** (macOS 10.13; `OA-BeBoB-MAFW-0.3.0-classic.pkg`) and **Modern** (macOS 11–15, Intel; `OA-BeBoB-MAFW-0.3.0-modern.pkg`) |

**Driver 0.2.1** (knob law)
- The hardware knobs move **outputs and aux out with the official law** again (0.5 dB steps near the top, 6 dB further down). The 1.5 dB / 3 dB / official step setting now applies to the **phones** only (phones encoder, or the Level Controller on a phones pair). R127.

**Panel 1.4**
- **Output and aux-out faders are back on the official console scale** (0, 2, 5, 10, 20, 30, 50 dB); only the **phones** faders keep the dB-linear 72 dB scale with off at the bottom. The step setting is now called **Phones Knob**. R127.
- **About tab names the edition** (Classic / Modern) in the banner and in a new *Edition* row; the Modern build reads "Modern Edition · macOS 11 to 15 (Intel) Driver" instead of the High Sierra caption. The project link points to the renamed repository **github.com/bhsardinha/OA-BeBoB-MAFW**. R127.
- **Menu bar:** a click on the icon opens the panel; a **right click** (or control-click) opens the menu. New order: Open Panel, Outputs and Phones volumes (each on its fader's law), Mute, Dim, Presets | Big Meter, Audio MIDI Setup, Firmware Check | Sync Source, Sample Rate, Show Panel In, Launch at Login | rate and lock status, Quit. Headphones A/B and the knob step left the menu (they stay in the panel); the unit list appears only with several interfaces. R128.

**Installer**
- **One package per edition.** Classic installs on macOS 10.13 to 10.15 (allowed range "before 11"), as before. Modern installs on macOS 11 and later; its postinstall checks that every kernel symbol the driver needs exists on that Mac (else it removes the driver and fails), stages the driver with `kmutil`, and the closing page asks to **Allow** it in Privacy & Security and restart. R128.
- The Modern edition's kext is built with the macOS 15 SDK; first load and playback on macOS 14.8.9 with the FW Audiophile (R125, R126).

---

## Package 0.2.2 — 2026-10-21

| Driver | Panel | MIDI plug-in |
|---|---|---|
| 0.2.0 (unchanged) | **1.3** | 1.11.2 |

**Panel 1.3**
- **Renamed to `OA BeBoB MAFW`** (app, Dock and menu names); the bundle identifier, preferences and presets are unchanged, so settings carry over.
- **Firmware check inside the panel:** the separate *OpenAudio Firmware Tool* app is gone; the same read-only check opens from *View > Firmware Check…* and from the status menu. `--firmware-list` prints it as text (was the tool's `--list`).

**Installer**
- Names under icons use **OA**: `Install OA BeBoB MAFW.pkg`, `Uninstall OA BeBoB MAFW.app`, the disk image `OA BeBoB MAFW 0.2.2`, files `OA-BeBoB-MAFW-0.2.2.pkg/.dmg`. The disk-image background names both interfaces and the package version.
- The update removes the 0.2.0/0.2.1 apps (`OpenAudio BeBoB MAFW.app`, `OpenAudio Firmware Tool.app`) and points the launch-at-login item at the renamed panel.
- **Fix:** the uninstall step quit apps by a name that also matched the Uninstall app itself (`Uninstall … .app`), so the Uninstall app could quit while removing; apps are now matched by their full path in `/Applications`.

---

## Package 0.2.1 — 2026-10-21

| Driver | Panel | Firmware Tool | MIDI plug-in |
|---|---|---|---|
| 0.2.0 (unchanged) | **1.2** | 0.1 | 1.11.2 |

**Panel 1.2 — graphics fix**
- **CPU use:** with the panel or the big meter on screen the panel used 30-40 % CPU on a MacBook Air (95 % of a core on the iMac with moving demo meters): every meter tick redrew the whole window on the CPU. Meters are now GPU-composited Core Animation layers. Segment patterns are rendered once and cached; a tick only resizes a clipped layer and moves the hold LED, and only when a segment changes. This is the official panel's `CMAVUMeter` method: cached off-screen image, no redraw without a change, only the meter's own area. Static controls draw once; hidden or covered windows stop reading the meters; fonts are cached. Measured on an iMac11,2: panel 95 % -> 2 %, big meter 20 % -> 1.4 % (WindowServer 3-7 %). R119.
- **Big meter segmented** like an old stereo deck: 80 LED blocks over -60...0 dBFS (0.75 dB each), green / amber from -18 dB / red from -6 dB, unlit blocks faintly visible, peak hold as one bright block.
- **About tab** shows the package version next to the panel and driver versions.

**Installer**
- **Update in place:** installing over any earlier release (0.1.0 or 0.2.x) needs no manual uninstall. Before copying, the package runs its own `uninstall.sh --update`, which removes the files of every earlier release (the script keeps the paths of all releases, including renamed or dropped ones) and keeps the user's settings (per-unit settings, panel preferences, launch at login). Nothing is unloaded; the restart at the end replaces the driver in memory. The Uninstall app and `uninstall.sh` use the same list.
- **Version numbers:** package, driver, panel and Firmware Tool have separate numbers (see the top of this file); the welcome and conclusion pages name both interfaces and list them.

---

## Package 0.2.0 — 2026-10-21

| Driver | Panel | Firmware Tool | MIDI plug-in |
|---|---|---|---|
| 0.2.0 | 1.1 | 0.1 | 1.11.2 |

First release of **OpenAudio BeBoB MAFW**, successor of OpenAudio FW1814 0.1.0 (FW1814 only), which the installer removes.
- **One driver for the FireWire 1814 and the FireWire Audiophile**, built like the official driver's class family: a shared base (`FWAudioDevice` -> `FWAVCAudioDevice` -> `FWRev1/Rev2AudioDevice`) plus one module per model. R93-R98.
- **FireWire Audiophile:** 4 in / 6 out at 44.1-96 kHz, firmware start from its bootloader, mixer / router / aux / headphone A/B, front panel (Level Controller, headphone knob, A/B switch and LEDs), status packet with meters and lock, AC-3 formats, MIDI through the M-Audio plug-in; factory reset equal to the official driver's (34/34 keys). R99-R108, R115.
- **Two units on one bus:** both interfaces together; a FireWire resource leak on unload fixed in the driver (the official driver never unloads). R117.
- **Panel 1.1:** per-model layout for both interfaces, device menu for several units, monitor level law for output / phones / aux-out (dB-linear, knob 1.5 dB per click by default), headphone safety ceiling, menu bar mode with a status menu, big horizontal meter. R110-R111, R116, R118.
- **Helper:** settings per unit for both models; headphones lowered for a unit without saved settings. R109, R116.
- **OpenAudio Firmware Tool 0.1:** read-only check of each unit's firmware against the official 1.11.2 build of its model; nothing writes flash. R112-R113.
- **Installer:** never unloads a running driver; install and uninstall ask for a restart. R117-R118.
