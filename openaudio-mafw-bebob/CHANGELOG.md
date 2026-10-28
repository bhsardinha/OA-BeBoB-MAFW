# Changelog — OpenAudio BeBoB MAFW

## Version numbers (user, 2026-10-21)

| Number | Where it lives | Changes when |
|---|---|---|
| **Package** | `VERSION` | every release; names the `.pkg`, the `.dmg` and the entries below; also shown on the panel's About tab and the installer pages; the Uninstall app carries it |
| **Driver** (kext + helper) | `kext/Info.plist` (`CFBundleShortVersionString`, `CFBundleVersion`) and `kext/src/base/kmod.cpp` | only when the kext or the helper changes |
| **Panel** | `control-panel/Info.plist` | only when the panel changes (since 0.2.2 it includes the firmware check, formerly the separate Firmware Tool 0.1) |

The M-Audio CoreMIDI plug-in is shipped unmodified (1.11.2). Each entry lists the component versions it contains; evidence is in `docs/system-map/PROVENANCE-RESOLUTIONS.md` (R-numbers).

---

## Package 0.5.0 — 2026-10-28 (release v0.5.0)

| Driver | Panel | MIDI plug-in | Editions |
|---|---|---|---|
| 0.3.1 (unchanged) | **2.0** | 1.11.2 | Classic and Modern |

**Panel 2.0 — the Advanced window (FireWire 1814, 44.1 / 48 kHz)**, from the status menu or *View › Advanced* (⌥⌘A). Four modes, each with its own interface scene per unit; switching saves the mode left and applies the next (asking first when an output would get louder):
- **DAW:** big input and output meters (the DAW does the monitoring), monitor level on the front knob, speakers A / B, the two phones.
- **Monitor:** direct monitoring, mono first. Mix 1/2 and Mix 3/4 are each a stereo mix or two independent mono lines (Out 1 / 2, Out 3 / 4; the panel pans each source to the lines it is IN on); the Aux is a third mix. Solo, a stereo link per input pair, mono or joined DAW returns, split output cards, the front A/B switch on the Phones 1 card. IN keeps a source's level; a new source joins at 0 dB.
- **Console:** the 1814 as converter for an analog console: unity at every interface level (with a warning and *Restore 0 dB*), linear meters with a 0 VU reference (−20 / −18 / −14 / −12 dBFS), your console's labels, **CUE** (PFL on Phones 1 through the Aux, the outputs untouched).
- **Hybrid:** outboard loops (DAW out → device → DAW in) with send / return meters and their difference.
- **Your own modes** (＋: new from the current one, rename, delete), listed in the status menu; **hidden channels** per mode; the **front knob's** target in the header.
- The interface's rules shown, not hidden: inputs and DAW returns reach Mix 1/2 and Mix 3/4 in pairs with one level for both (register 0x90 / 0x94), and the DAW returns have no pan (odd ones land left, even ones right).

**The Dual window** (two FireWire 1814s as one macOS aggregate device, 32 in / 24 out with ADAT): DAW view with both units' meters, each unit's monitor knob, speakers and phones; Console view for both; a setup sheet (rates, word clock from A to B, the aggregate). **Not tested with the aggregate on hardware yet.**

**Other panel changes:** one panel window at a time (standard, Advanced or Dual); the Advanced and Dual meters are drawn as one GPU image per row from the driver's peak snapshot (Dual DAW: about 6 % of a core plus 8 % in WindowServer on a 2010 iMac, from 31 % + 42 %); writes the driver refuses are shown.

**Tested:** every mode's scene written and read back on two real FireWire 1814s (capture 108). Most interactions inside the views are checked in the demo only; nothing of 0.5.0 has been tested on the Modern edition beyond building and opening it.

---

## Package 0.4.1 — 2026-10-27 (published, release v0.4.1)

| Driver | Panel | MIDI plug-in | Editions |
|---|---|---|---|
| **0.3.1** | **1.7** | 1.11.2 | Classic and Modern |

**Driver 0.3.1** (R141, capture 107)
- **Fixed: "Show stereo pairs as separate devices" gave devices without channels** at any rate but 44.1 kHz (seen on macOS 14 with the FireWire 1814 at 48 kHz). macOS refused the pair streams' format because the pair device's rate was set after it (the official driver's order; the 10.6 system coped, macOS 14 does not). The rate is now set first. Generic (every model). Checked with the FireWire 1814 at 48 kHz on Modern (user, 2026-10-27).

**Panel 1.7**
- **Launch at Login no longer shows macOS's "Background Items Added" at every start** (user, 2026-10-27): the panel rewrote its LaunchAgent at each launch to keep the app path current; it now rewrites it only when the path differs (a moved or renamed app). The installers' postinstall does the same check.

**Docs and installer**
- README, installer pages and disk-image text simplified: the three interfaces and every feature listed as supported in 0.4.1, without the per-version test notes (user, 2026-10-27).

## Package 0.4.0 — 2026-10-27 (published, release v0.4.0)

| Driver | Panel | MIDI plug-in | Editions |
|---|---|---|---|
| **0.3.0** | **1.6** | 1.11.2 | Classic and Modern |

**M-Audio FireWire 410** (plan `docs/BUILD_FW410_PLAN_v2.md`, R135-R138, capture 106)
- Driver: a new model module — 4 in / 10 out at 44.1-96 kHz, 2 in / 8 out at 176.4/192 kHz (all eight analog outputs, through the official 192 kHz channel map), MIDI 1/1; register mixer; coaxial or optical S/PDIF input; Level Controller; meters.
- Panel: the 410's own layout — five software returns and two inputs routed to one or several outputs at once, five outputs with main/aux, aux bus, phones from any combination of the output buses and from the aux bus (a source the official panel never offered), S/PDIF input card, Level Controller on five buses, About photo.
- Checked on Classic (iMac, 10.13.6, installed from the package): attach and firmware start, every output at 44.1 and 192 kHz, recording at every rate, reported latency within 0.5 ms of a measured loopback, mixer, Level Controller, settings per unit. Not tried: MIDI, S/PDIF / external sync, coaxial/optical switch, sleep/wake, Modern.
- Deviation (user): the phones aux source is cleared when switched off (the official driver never clears it; R138).

**Every model**
- "Show stereo pairs as separate devices" (Hardware page, CoreAudio Devices card): each stereo pair also appears as a 2-channel CoreAudio device; pair devices keep the official IDs (`m_audio_b_FWPairEngine:<n>`). Tried with the 410 only.
- **Fixed: a kernel panic at the next bus reset after the driver was unloaded** (R140): a FireWire plug stayed registered after `kextunload`, idle or playing (the R117 fix of 0.2.0 did not work). Freed now when the driver stops; checked with idle and playing unloads and a power-cycle.
- **Fixed: `kextunload` needed a second try** (R139): the status poller now finishes its last read before the driver stops.
- Includes driver 0.2.3's wake-from-sleep fix (R133) and the sleep / power-loss mixer recovery (R134), which 0.3.2 carried but never published.
- Shared code changed (channel maps, unload path, pair-device IDs): the FireWire 1814 and the Audiophile have not been re-tested on 0.4.0.

## Package 0.3.2 — 2026-10-22 (test build, not published)

| Driver | Panel | MIDI plug-in | Editions |
|---|---|---|---|
| **0.2.3** | 1.5 | 1.11.2 | Classic and Modern, as 0.3.0 |

**Driver 0.2.3** (R133)
- **Fixed a kernel panic on wake from sleep** (found on the Modern edition with a FireWire 1814, macOS 14.8.9). On wake the driver re-sends the mixer settings; when the interface was not back on the bus yet, a refused AV/C command was released twice and the Mac froze. A command refused at once is now released once only.

## Package 0.3.1 — 2026-10-22

| Driver | Panel | MIDI plug-in | Editions |
|---|---|---|---|
| **0.2.2** | **1.5** | 1.11.2 | Classic and Modern, as 0.3.0 |

**Follow the external clock rate** (R131, `docs/RATE_FOLLOW_PLAN_v1.md`)
- **Driver 0.2.2:** publishes `MAExternalSyncRate`, the rate of the external source as the interface reports it (lock bit + rate field of the status packet; 0 when nothing is seen). The interface reports the source's exact rate while it is in the current speed range (44.1/48 or 88.2/96 kHz; capture 98). `MASampleRate` and `MARateLocked` behave as before.
- **Panel 1.5:** with the sync source on **External – digital**, the panel sets the interface to the source's rate through CoreAudio, only while no application plays through it (otherwise it shows "Source at … kHz - follows when playback stops" and follows when playback stops). When it sees nothing it tries the other speed range once (48 ↔ 96 kHz) and goes back if nothing locks. Setting **Follow external clock rate** in the Hardware tab and **Follow External Rate** in the status menu, on by default. It also runs when the panel is hidden (launch at login). Word clock is not followed.
- **Tested** with the Modern package on macOS 14.8.9 with a FireWire 1814 and a POD X3 on coaxial S/PDIF (user, 2026-10-22, capture 99): out of range it switches speed range, in range it takes the exact rate and locks. The official driver detects the rate the same way but never follows it.

**Uninstall app, disk images replaced 2026-10-22** (same package number; the two `.pkg` files are unchanged, only the Uninstall app in both `.dmg`s is new; R132)
- **Progress:** a one-line status ("40 % — Removing the driver…") and a 0–100 % bar instead of a spinner. The uninstall script reports each step (helper, panel, driver, panel and MIDI driver, icons and receipts, MIDI server, launch-at-login item, kernel cache); the long kernel-cache rebuild moves the bar slowly from 60 to 95 %. If it fails, the alert names the step it stopped at.
- **Dry run:** `Uninstall OA BeBoB MAFW.app/Contents/MacOS/Uninstall OA BeBoB MAFW --dry-run` runs every step without the password and changes nothing (`--start` begins it without a click). `uninstall.sh --dry-run` and `--progress FILE` do the same from a shell; the installer's `--update` step is unchanged.

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
- **One package per edition.** Classic installs on macOS 10.13 to 10.15, as before. Modern installs on macOS 11 to 15; its postinstall checks that every kernel symbol the driver needs exists on that Mac (else it removes the driver and fails), stages the driver with `kmutil`, and the closing page asks to **Allow** it in Privacy & Security and restart. R128.
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
