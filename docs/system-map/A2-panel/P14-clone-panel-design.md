# A2-P14 — Clone panel: architecture and rules (Build rows PN.3–PN.7)

> **Update (2026-10-16, redesign):** the first AppKit look was rejected by the user (gray boxes, bad proportions, windows opening bottom-left, dead level knob, hard-wired A/B text). The view layer was redesigned (dark minimalist look inspired by the Audient EVO series, user direction) and the functional bugs fixed; the user reviewed it running in demo mode on the build PC and approved the look ("looks great"; not yet wired to hardware). See §Design system and §Behaviour below; `clone-snapshots/` holds the new renders.

**Status 2026-10-16:** complete as source and built into an unsigned app on the build PC (Swift 4.1.2, 10.13 SDK, `make app`); the pure-logic layer passes `make test`; the app launches and quits cleanly with no kext/device ("No FireWire 1814 found" window) and renders every tab, both mini-meter windows and the rate/mode variants in demo mode (`clone-snapshots/`). **Nothing has run against hardware** (airliner rule); the IOKit paths (discovery, notification, meter page) are untested. Approved layout: `PN1-wireframe.html`; control→key map: `PN2-control-key-map.md`; index/range/cookie evidence: `P13-translator-index-map.md`; screens and user statements: `P12-panel-screens-v19.md`.

## Files (`openaudio-fw1814/control-panel/Sources/`)
| File | Role | Build row |
|---|---|---|
| `MaKeys.swift` | 54-key catalog, `MAClockSource` codec, `MAIndex`, `MARotaryMask`, `MARange`, `MAHeadphoneSource` | PN.2 |
| `ChannelLayout.swift` | strips/greying derived from (rate, optical in/out, S/MUX); pure logic | PN.3/PN.5 |
| `DeviceModel.swift` | property cache, batched writes, level/balance/link/clock/rate/headphone/rotary rules, presets snapshot | PN.3/PN.5/PN.6 |
| `IOKitBridge.swift` | discovery + hot-plug, registry get/set, notification port, meter page, `peakToDB`, `PeakSlot` | PN.6 |
| `FirmwareInfo.swift` | `MAFirmwareInfo` decoder | PN.5 |
| `Theme.swift` | colour/type/metric tokens and drawing helpers (dark minimal look) | PN.3 |
| `Widgets.swift`, `StripView.swift` | fader (non-linear dB scale, approximated; relative drag), ring knob, LED meters with peak hold and clip, pill buttons, segmented bar, icon buttons, strips | PN.3 |
| `MixerView.swift`, `OutputView.swift`, `HardwareTab.swift`, `AboutTab.swift`, `SidebarView.swift`, `MetersView.swift`, `PanelContext.swift` | tabs, sidebar, bus meter panels | PN.3–PN.5 |
| `Presets.swift` | save/load/delete of mixer settings (plist) | PN.7 |
| `PanelWindowController.swift`, `AppDelegate.swift`, `main.swift`, `Info.plist` | window per interface (hot-plug), view switch, 30 Hz meter poll, mini-meter windows, menu, waiting window | PN.1/PN.6 |
| `DemoBackend.swift` | debug aid: `--demo [--spdif] [--rate N] [--devices N] [--knob N] [--ab A,B] [--snapshot DIR]` runs the UI on in-memory devices (simulating the kext's level-knob effect), and writes PNGs of every tab, both meter windows, the A/B sheet and the launch state | — |
| `Resources/AppIcon.icns` | app icon (all sizes 16–1024 px built with `iconutil` from the user's `docs/images/app-icon-source.png`; `CFBundleIconFile` in `Info.plist`) | PN.1 |
| `Resources/fw1814-front.png` | front-panel photo for the about tab (cut-out of `docs/images/maudiofront1814.jpg`, user-supplied) | PN.5 |
| `Tests/LayoutTests.swift` | host tests (`make test` on the build PC) | — |

## Rules the code follows
1. **State follows the device, not the reference panel's widgets** (user decision 2026-10-16): 88.2/96 kHz greys ADAT 5–8 (mixer strips, ADAT bus, input-bus meters) whatever the S/MUX box says, and S/MUX is set automatically; 176.4/192 kHz is 2 in / 4 out analog with every digital control hidden or greyed; S/MUX is greyed (state kept) with output S/PDIF. Source: `FWRates.h` tables, P12.
2. Every control write is a registry property write through `DeviceModel` (batched per commit, rate deferred 1 s); never `IOConnectCallMethod` (B5 §10).
3. ctrl buttons edit single bits of `MARotaryControlV2`; the sidebar radios write the group preset (P13 §2).
4. Faders: panel domain −129…0 (0.5 dB steps), pan −255…255 shown ×0.5 (P13 §3); balance is derived from the two levels (P13 §6a).
5. Peak meters read `uint32 peak[52]` (map P13 §6, dB formula P13 §6a), ~30 Hz.
6. No buffer-size control (CoreAudio sets it in the host).

## Unverified / open (labelled in code and docs)
- Fader scale positions (approximation read off screenshots); input pair ↔ S/PDIF/ADAT strip assignment; aux-send index for the SW-return half; aux strip `ctrl` bit; `MAInputPan2` wire half-word order; which kernel mode the three `MAMomentarySwitchRoutings` entries address; which a/b function radio carries cookie value 3; byte order of the firmware info block; web-link button ↔ URL; default of `MAMomentarySwitchRoutings` ({1,2,3,4} in the kext) gives no valid B source under the 3-entry reading, so the A/B text shows "-" and the sheet needs both sides chosen (P13 §5 open item 4); rate-change gating while a client streams (kext side).
- Shared: 14 SW-return and 14 output channels are addressed by the panel while the clone kext's `MAOutput`/`MASWReturn` groups are `<4>` (clamped to channel 0); visible controls only touch pairs 1–2.

## How to build, test and look at it (build PC, 10.13.6)
`openaudio-fw1814/scripts/sync-to-buildpc.sh`, then on the build PC `cd ~/oa-build/control-panel`: `make typecheck`, `make test`, `make app` (output `build/OpenAudio FW1814.app`, unsigned; signing is IN.6). Visual check without hardware: `"build/OpenAudio FW1814.app/Contents/MacOS/OpenAudio FW1814" --demo --snapshot /tmp/pn` (add `--rate 96000|192000`, `--spdif`). `screencapture` does not work over SSH, hence the in-process snapshot. The dev Mac's Swift toolchain cannot build for 10.13 (CLT module clash).

## Layout fixes found by the snapshots
`NSBox` reports no intrinsic size and collapsed inside stack views, so cards are `CardStack`/`ColorLabel` views; scroll-view documents use a flipped stack so they hang from the top; the sidebar keeps its natural card heights with a flexible spacer.

## Design system (2026-10-16 redesign)
- **Look:** near-black surfaces separated by tonal steps and spacing (no borders), white/grey type with a small hierarchy, one blue accent; semantic colours only where a state matters (mute red, solo/dim amber, aux green, meter green/yellow/red). Everything is code-drawn with the system font except the about-tab photo. Tokens live in `Theme.swift`.
- **Strips:** fixed row layout so knobs, faders and buttons line up across a tab; faders between slim LED meters with peak hold and a clip LED; flat knobs with a thin value ring; value readouts under each control (level dB, aux "-∞", "L127/R127", "<C>/L35/R35"); labelled groups (Software Returns / Inputs; Main Outputs / Aux & Phones / Bus Meters).
- **Window:** header with device name (a device menu when several interfaces are present), a read-only rate/lock chip, the tab bar and icon buttons for full / wide / narrow; fixed sidebar with settings, the level-controller knob drawn as a big knob with an LED dot ring, and main output. The window size follows the selected tab (keeps its top edge and centre), minimum about 600 pt tall because of the sidebar.

## Behaviour (2026-10-16)
1. **Level-controller knob works:** drag (4 px per detent) or scroll wheel (reference acceleration: count × (13 − interval_ms/6) when ticks are < 78 ms apart, `SetHWSetting` cookie 0x5e4) sends signed detents to `MAMetaVirtualRotateDataKnob` at once; the kext moves the masked levels and publishes them back.
2. **Headphone A/B text is live:** reads entries 1 (A) and 2 (B) of `MAMomentarySwitchRoutings` on every refresh.
3. **Windows centred:** the main window opens centred; switching full/wide/narrow puts the target window on the same centre point; closing a meter window returns to the full panel.
4. **Mini meters:** channels grouped under Analog / ADAT / S/PDIF headings with plain numbers (L/R for S/PDIF); ADAT 5–8 greyed at 88.2/96 kHz like the other views.
5. **Several interfaces:** one panel per interface, each with its own settings, meters and knob; a device menu in the header and a Device menu in the menu bar (check on the current one, as the reference panel's device list) plus a View menu (⌘1–⌘4 tabs, meter views); a newly plugged interface is added to the menus without opening a window; closing a device's window hides it (it comes back from the Device menu or the Dock). Tested only with two simulated devices (`--devices 2`).
6. **About tab:** front-panel photo on a dark banner, versions table, web links.

## Changes after the first hardware run (2026-10-16)
- `copyProperties` merges `MAMetaAllProperties` (registry table = static keys only).
- Live writes: pending property writes flush every 40 ms while a control is dragged (user request; the reference batched at mouse-up).
- Output balance (out 1/2, 3/4, aux, phones) is panel state kept apart from the faders (user request): device level = fader - balance attenuation of that side; the reference wrote balance into the two levels.
- A/B sources: `MAMomentarySwitchRoutings` entries 0 = A, 1 = B (index = kext A/B mode); default reads "A = out 1/2, B = out 3/4". Unverified on hardware.
- Single instance (`main.swift`); `--snapshot DIR` also works on a real interface; demo/snapshot modes skip the single-instance check.
- Verified on hardware: device discovery, property reads/writes, meters, front knob, faders, phones after a source click, About tab data.
