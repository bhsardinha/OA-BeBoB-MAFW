# Panel redesign — "Flow" layout (plan v1, proposal)

> **Correction (2026-10-27) — superseded by `PANEL_REDESIGN_PLAN_v2.md`.** The user found that this plan treats every channel as a stereo
> pair, while studio I/O is mostly mono (hard-panning to get mono is the problem, not the solution), and that one view cannot serve every
> way of working. v2 splits the panel into modes (Monitor, DAW, Console, Hybrid), each with its own view and interface scene; the Flow
> layout below survives as the Monitor mode, made mono first. The hardware facts in §2 are refined in v2 §2 (routing per input pair,
> recording before the mixer, returns without pan). The parity table (§5) still applies to the set of modes as a whole.

Status: **proposal, waiting for the user's decision** (2026-10-27, branch `panel-redesign`). No panel code has changed. The companion
mockup is `docs/panel-redesign/PANEL_FLOW_MOCKUP_v1.html` (open it in a browser; it has the three models, the Aux view, 192 kHz and every
popover). Screenshots of the mockup: `docs/panel-redesign/flow-*.png`.

Scope: the panel app only (`openaudio-mafw-bebob/control-panel`). The driver, the `MA*` keys, the DeviceProfile plans and the helper do not
change. Every capability of today's panel stays (memory "Clone = full feature parity + modern UI acceptable"); the table in §5 is the checklist.

## 1. Why

The current panel is the official 2014 panel's layout, redrawn: tabs by object type (Mixer = sources, Output = destinations, Hardware =
configuration) plus a fixed sidebar. It is faithful, and it hides the one thing users need to see: **what each output plays**.

- Routing lives in two places on two tabs: the small `1/2 · 3/4` buttons at the foot of each mixer strip, and the phones `monitor` buttons
  and the `main/aux` pill on the Output tab. Building a headphone mix means switching tabs.
- The aux mix is 2 × 16 small knobs (one per channel) on top of the strips, the hardest control to read and to set.
- Each linked pair spends two faders, two values and two meters (84 px); the bus meters at the bottom repeat what the strips show.
- The sidebar (presets, level-control knob with its radio list, mute/dim) takes 200 px on every tab, and the knob's assignment is split
  between the radios and the `ctrl` buttons on the strips (greyed unless the matching radio is on).
- Configuration sits far from its effect: the ADAT/S/PDIF choice (Hardware tab) changes the mixer's strips and the Output tab's digital
  strip; the A/B button's function (Hardware tab) changes Phones 1.
- At 176.4/192 kHz the FW1814's Mixer tab becomes a page of text.

## 2. What the hardware really offers (the model the layout must express)

Sources: `MA-…` / Q-MAMAP rows in `MaKeys.swift`; signal paths `docs/manual-reference.md` §2-§3; per-model plans in `Sources/Models/*`.

- **Mixes.** One mix bus per routing bit (`routeLabels`: FW1814 `1/2, 3/4`; Audiophile `1/2, 3/4, spd`; FW410 `1/2 … 7/8, spd`) plus the
  **aux** bus. A source is on or off in each mix (`MAInputRoutingV2`, `MASWReturnRoutingV2`) but has **one level and one pan for all of
  them** (`MAInputLevels`, `MAInputPan2`). The aux is the only independent mix: one send per channel (`MAAuxSendLevels`, 22 elements on the
  FW1814 = 18 inputs + 4 SW returns), no per-channel pan, one master (`MAAuxOutputLevels`, pan on the Audiophile/FW410 `MAAuxOutputPan2`).
- **Outputs.** Each line pair plays its own mix or the aux (`MAMonitorAuxOnOutput`), with level, balance, solo, link, mute. Phones play one
  mix (FW1814, Audiophile) or any set of them (FW410, `phonesMultiSelect`, bit 5 = aux).
- **Direct paths.** On the FW1814 the DAW returns 5–12 (ADAT) or the S/PDIF return go straight to the digital output, with no mixer; at
  176.4/192 kHz the mixer is off and DAW 1–4 go straight to Line 1–4 (hardware, 2026-10-17).
- **Front controls with software meaning.** The Level Controller knob (`MARotaryControlV2`: one group at a time, any pairs inside it), the A/B
  button (Phones 1/Phones source swap or direct monitoring), the phones encoders and their step law (`MAMonitorKnobStep`).
- **Global.** Rate and clock (`MAClockSource` composite, rate through CoreAudio), digital ports (FW1814), mute, dim, presets, stereo-pair
  engines, firmware check, about.

## 3. The Flow layout

One window, no tabs for everyday work, read **left to right as the signal flows**:

```
┌ header: unit ▾ │ ● 48 kHz · Internal · locked ▾ │ Scene ▾ │            knob −10 dB → Outputs ▾ │ DIM │ MUTE │ views ┐
├──────────────────────────────────────────────────────────┬────────────┬─────────────────────────────────────────┤
│ ▌Mix 1/2 — what the faders mean in this mix              │ MIXES      │ OUTPUTS                                 │
│ DAW        ANALOG IN                 ADAT IN ▾            │ [Mix 1/2]──┼──▶ Line Out 1/2  [Mix 1/2|Aux] ═══ M ⋯ │
│ ┌──┐┌──┐   ┌──┐┌──┐┌──┐┌──┐          ┌──┐┌──┐┌──┐┌──┐     │ [Mix 3/4]──┼──▶ Line Out 3/4  [Mix 3/4|Aux] ═══ M ⋯ │
│ │▮▮││▮▮│   │▮▮││▮▮││▮▮││▮▮│  strips  │▮▮││▮▮││▮▮││▮▮│     │ [Aux    ]──┼──▶ Phones 2      [1/2|3/4|Aux] ═══ M ⋯ │
│ └──┘└──┘   └──┘└──┘└──┘└──┘          └──┘└──┘└──┘└──┘     │ DIRECT     │     Phones 1 … A/B: A 1/2 · B Aux ●     │
│                                                          │ [DAW 5–12]─┼──▶ ADAT Out (8 meters)                 │
└──────────────────────────────────────────────────────────┴────────────┴─────────────────────────────────────────┘
```

The nine ideas (numbered as in the mockup's callouts):

1. **Signal flow, left to right.** Sources (DAW returns, analog, digital) → mixes (one tile per bus, with its sum meter) → outputs (one card
   per physical output). What each output plays is a **wire** from a mix tile to the output card, coloured as the mix; hovering a tile
   outlines the strips that feed it. Hard-wired paths (DAW 5–12 → ADAT out) come from a dashed "Direct" tile, so the hardware's limits are
   visible instead of being an empty strip.
2. **Work on one mix at a time.** Clicking a mix tile (or keys 1…n) makes it the context: its sources stay bright, the others dim (still
   usable). In a line mix the faders are the channel levels; in the **Aux** mix the same faders become the **aux sends** ("sends on
   faders"), which removes the 2 × 16 aux knobs. A context bar says what the faders mean, including the hardware rule that one level feeds
   every mix a channel is lit in.
3. **One colour per mix, everywhere.** Mix 1/2 blue (today's accent), 3/4 violet, 5/6 teal, 7/8 pink, S/PDIF orange, Aux green (today's aux
   colour). The strips' route chips, the tiles, the wires, the outputs' source buttons, the phones and later the status menu use them.
4. **Fader-meter strips.** One fader per linked pair, the stereo meter inside the fader track; a stereo-image bar with two handles replaces
   the two pan knobs (inputs only; DAW returns have no pan, as today). 70 px per pair instead of 84; unlinking opens two faders (100 px).
   Route chips at the top (one per mix, aux chip = send on/off), M · S · link at the bottom.
5. **Outputs as cards.** Name and source on the first line, level fader-meter and mute on the second; balance, solo, link, the phones knob
   law and the A/B setup behind `⋯`. Phones 1 shows the A/B button's job and its state (A · 1/2, B · Aux, LED).
6. **Settings where they act.** Rate, clock source, follow-external and lock in the header pill's popover; ADAT/S/PDIF on the digital input
   group's header and on the digital output card; the A/B function on the phones card; the phones knob law in the phones card. **The
   Hardware tab goes away**; one *Device settings* sheet (unit menu) still holds every hardware setting, plus stereo-pair engines.
7. **The front knob in the header**: its value, its target ("Knob → Outputs (2)") and a popover with the group and the pairs — the
   interface's rule, one group at a time. A blue ring on strips, cards and the aux tile shows what it drives. Replaces the radio list and
   the per-strip `ctrl` buttons.
8. **No sidebar.** Scenes (today's Load/Save/Reset/Delete, with the saved files listed) and mute/dim go to the header. The 200 px go to the
   channels. The FW1814 window is 1280 × 720 in ADAT mode and fits a 1366 × 768 screen (§6).
9. **One layout at every rate.** At 176.4/192 kHz the FW1814's sources area folds in place with the reason; the mixes column shows the
   Core Audio direct tile; outputs and phones stay where they are (phones choose Out 1/2 or 3/4, as today's `hideLastMon`).

Panel-only additions (no driver involvement): channel names and hidden strips per unit (panel preferences keyed by GUID), ⌥-click /
double-click to 0 dB, keys 1…n for the mixes, the hover outline. Later, if wanted: undo of panel changes.

Unchanged: the meter windows and compact view (header view switcher), the status menu (gets the mix colours later), the firmware check, the
demo and snapshot modes, presets' file format.

## 4. One design for every model

Nothing model-specific is added outside `Sources/Models/<Model>/` (memory "Generic first"). The Flow builds from the plans the profiles
already return:

| Flow element | Built from |
|---|---|
| Source groups and strips | `MixerPlan.returns` / `.inputs` (+ `ChannelLayout` for ADAT/S/PDIF and rate greying) |
| Mix tiles | `routeLabels` + aux; tile meters = `MixerPlan.bus` |
| Direct tile | `MixerPlan.returnBus` (FW1814 ADAT/S/PDIF return bus), the quad-rate note |
| Line output cards | `OutputPlan.main` (`monitorPair` → its mix or aux, `features`) + `digitalBus` |
| Aux master | the aux `OutputStripPlan` (`pan` when present) |
| Phones cards | phones `OutputStripPlan` + `phonesSources`, `phonesMultiSelect`, `hideLastMon` |
| Knob popover | `levelControllerTargets`, `MARotaryMask` |
| Header pill / sheet | `HardwarePlan` (`sync`, `abSwitch`, the model's `hardwareCard`) |

Model checks in the mockup: FW1814 (ADAT and S/PDIF modes, 88.2/96 S/MUX greying, 192 kHz fold), Audiophile (Mix S/PDIF, one Phones with
A/B), FW410 (six mixes, phones on several mixes at once, phones as a knob group). Open for the FW410: its phones also have a `main/aux`
toggle (`MAMonitorAuxOnOutput[6]`, capture 103) besides the aux source bit (bit 5, capture 106); the mockup shows only the source chips.
Decide in phase F2 from the captures before drawing it.

## 5. Parity checklist (today → Flow)

| Today | Key(s) | Flow |
|---|---|---|
| Mixer strip title | — | strip name (renamable, panel-side) |
| aux L / aux R knobs | `MAAuxSendLevels` | faders in the Aux mix; aux chip = send on/off (restores the last send) |
| fader pair + values | `MAInputLevels`, `MASWReturnLevels` | fader-meter (one linked, two unlinked) + value |
| strip LED meters | peak page input / SW return | meter inside the fader track |
| pan L / pan R knobs | `MAInputPan2` | stereo-image bar (two handles) |
| link · solo · mute | `MA*Link`, `MA*Solo`, `MA*Mute` | link icon · S · M |
| out `1/2` `3/4` (…) buttons | `MAInputRoutingV2`, `MASWReturnRoutingV2` | route chips in mix colours |
| `ctrl` buttons | `MARotaryControlV2` bits | knob popover pairs + ring badge |
| ADAT Bus / S/PDIF Bus strip | return peaks | Direct tile |
| Output bus / Aux bus meters | output / aux peaks | mix tiles' meters |
| 176.4/192 page note | — | in-place fold (idea 9) |
| Out 1/2, 3/4 (… 7/8, S/PDIF) strips | `MAOutputLevels/Mute/Solo/Link`, balance | Line Out cards: level, mute; ⋯ balance, solo, link |
| `main` / `aux` pill | `MAMonitorAuxOnOutput` | the card's source buttons [Mix x/y · Aux] + wire |
| ADAT Out / S/PDIF Out bus | output peaks, `mainAuxPair` | digital output card (meters, source when it has one) |
| Aux Out strip | `MAAuxOutputLevels/Mute/Link`, `MAAuxOutputPan2` | Aux tile master + ⋯ (balance, link, pan) |
| Phones strips + monitor buttons | `MAHeadphoneLevels/Mute/Link`, `MAHeadphoneSourceV2`, `MAHeadphoneSecondSource` | Phones cards: source buttons, level, mute; ⋯ balance, link |
| SW return bus / input bus meters | peaks | the strips' meters (same slots) |
| Sample rate display, stepper, lock | CoreAudio rate, `MARateLocked` | header pill + popover (and sheet) |
| Follow external clock rate | `MAExternalSyncRate` (R131) | clock popover (and sheet) |
| Sync source radios | `MAClockSource` bits 0-1 | clock popover (and sheet) |
| FW1814 digital ports card | `MAClockSource` bits 2-5 | digital group header ▾ and digital output card ⋯ (and sheet) |
| FW410 S/PDIF card | `MAClockSource` bit 2 | its S/PDIF input group header ▾ (and sheet) |
| A/B switch + Set Source sheet | `MAMomentarySwitchSource`, `MAMomentarySwitchRoutings` | phones card A/B row + ⋯ (and sheet) |
| Phones knob law | `MAMonitorKnobStep` | phones card ⋯ (and sheet) |
| CoreAudio stereo pairs | `MAEnableStereoPairEngines` | Device settings sheet |
| Sidebar Load/Save/Reset/Delete | presets, `MAMetaResetToFactorySettings` | Scene menu |
| Sidebar level-control knob + radios | `MAMetaVirtualRotateDataKnob`, `MARotaryControlV2` | header knob + popover |
| Sidebar main mute / dim | `MAGlobalMute`, `MAGlobalDim` | header buttons |
| About tab, firmware check, unit sidebar | `MAFirmwareInfo`, … | unit menu (About, Firmware check, other units) |

## 6. Constraints

- AppKit, Swift 4.1, Xcode 9.4.1 / 10.13 SDK for the Classic panel (no SwiftUI; `NSView` + layers as today). The Modern panel builds from
  the same source.
- GPU-first meters (memory "Panel: GPU-first"): meters and wires are layers (`CALayer` / `CAShapeLayer`), static parts drawn once; measure
  CPU before/after as for 0.2.x.
- Smallest screen: the MacBook Air (MacBookAir5,1, 11-inch). **unverified:** 1366 × 768 is Apple's spec for that model, not measured here
  (the Air was offline 2026-10-27; check with `system_profiler SPDisplaysDataType` on it). The FW1814 Flow window is 1280 × 720 of content.
  **unverified:** whether today's window adds a title-bar height to its content height (`styleWindow` makes the title bar transparent);
  measure the current window before fixing the Flow height.
- Wording that depends on unverified hardware behaviour: whether the aux send is pre- or post-fader, and whether a channel's mute also
  mutes its aux send (both affect only the context bar's text; settle on the FW1814 with a test tone when the user wants). Whether
  `MAMomentarySwitchLatent` updates live when the front A/B button is pressed (the Phones 1 LED): check with `ioreg` while pressing it.
- Both layouts live together until the user accepts Flow: `View ▸ Layout ▸ Flow / Classic`, Classic the default during F1-F4.

## 7. Phases (each ends with `--demo --snapshot` PNGs on the build PC and the layout tests)

- **F0** (this): proposal + mockup on `panel-redesign`. **User decision needed.**
- **F1** Flow sources + mix tiles (context, sends on faders, route chips, fader-meter strips), from the existing plans; `--layout flow`
  in demo/snapshot; pure-logic mapping (plans → flow model) under `make test`.
- **F2** Output cards + wires + Direct tile; FW410 phones `main/aux` decided from captures 103/106.
- **F3** Header: clock pill popover, Scene menu, knob popover and ring badges, mute/dim; Device settings sheet; sidebar and Hardware tab
  hidden in Flow.
- **F4** Names and hidden strips (per GUID), keyboard, hover outline; status menu mix colours; CPU measurement.
- **F5** User acceptance on the real units (only when the user asks, no formal testing — R123); then Flow becomes the default and Classic
  stays one release behind a menu item.

## 8. Questions for the user

1. Does the left-to-right flow with wires read right to you, or should the outputs sit on top (a "who listens" row) with the strips below?
2. Sends on faders for the aux: yes, or keep small send knobs on the strips as well?
3. Remove the Hardware tab (sheet + in-place controls), or keep it as a tab?
4. Rename "SW return 1/2" to "DAW 1/2" in the UI (the sub-line keeps "SW return 1–2")?
5. Keep Classic as an option permanently, or only during the transition?
