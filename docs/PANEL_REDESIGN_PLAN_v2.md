# Panel redesign — modes (plan v2, proposal)

Status: **proposal, waiting for the user's decision** (2026-10-27, branch `panel-redesign`). No panel code has changed. Supersedes the
single view of `PANEL_REDESIGN_PLAN_v1.md`, which treated every channel as a stereo pair (see the Correction block there).

Mockup: `docs/panel-redesign/PANEL_MODES_MOCKUP_v1.html` (FW1814 with ADAT in and out; switch modes in the window's header; meters are
simulated). Screenshots: `docs/panel-redesign/modes-*.png`. The v1 sketch (`PANEL_FLOW_MOCKUP_v1.html`) stays as history; its Flow layout
becomes the Monitor mode, made mono first.

## 1. What the user asked (2026-10-27)

- Studio I/O is mostly **mono** (a drum kit is 8 mono mics). Treating everything as stereo pairs and hard-panning to get mono is wrong;
  outputs and mixes must be choosable as mono.
- **One screen per way of working**, each with its own view: direct monitoring, everything in the DAW (the panel only for meters and
  levels), the 1814 as I/O for an external analog console (e.g. 16 in / 8 out), and tracking/mixing with outboard (DAW → processor → DAW).
  Versatile, with customisable views.

## 2. Hardware facts that shape the modes (FW1814)

| Fact | Source |
|---|---|
| Recording is taken **before** the interface mixer: no panel fader, pan or mute changes what is recorded | snd-firewire-ctl-services `protocols/bebob/src/maudio/special.rs:14-41` (signal-flow diagram); manual §10 (direct monitoring). **unverified on our unit**: move an input fader to −∞ while recording |
| Mix 1/2 and Mix 3/4: **one level and one pan per input channel**, shared by both | `MAInputLevels` (18), `MAInputPan2` (18); `special.rs` `analog_gains[8]`, `analog_balances[8]` |
| Inputs join Mix 1/2 / Mix 3/4 **per pair** (In 1 + In 2 together) | `MAInputRoutingV2` = 9 two-bit masks (A2 PN2 row 24); `special.rs:842` "pairs of analog inputs" |
| DAW returns 1–4: level per channel, routing per pair, **no pan** (odd → left side of the mix, even → right) | no pan key in `MaKeys.swift`; `special.rs` has no stream balance |
| Aux: one send per channel (22), stereo output. **unverified:** whether a channel's send reaches Aux L, Aux R or both | `MAAuxSendLevels`; `special.rs` aux ranges 0x64–0x90 |
| Line 1/2 plays Mix 1/2 or Aux; Line 3/4 plays Mix 3/4 or Aux; each phones output plays one of the three | `MAMonitorAuxOnOutput`; `special.rs:540-562` |
| Output and phones levels are per channel (mono outputs possible) | `MAOutputLevels` (4), `MAHeadphoneLevels` (4) |
| DAW 5–12 go straight to ADAT out, no mixer | `special.rs` diagram |
| Core Audio: ADAT 16 in / 12 out at 44.1/48 kHz, 12/8 at 88.2/96; S/PDIF 10/6; 192 kHz 2/4 | kext `models/FW1814/FW1814Tables.h` |

Consequences:

- The interface has **two real states**: a cue mixer (direct monitoring) or a **passthrough** (DAW 1–4 → Line 1–4 at 0 dB, inputs out of
  the mixes). DAW, Console and Hybrid modes all use the passthrough; they differ in what the panel shows and protects.
- At most **three independent cue mixes**: Mix 1/2, Mix 3/4, Aux. A stereo mix can be split into two mono mixes (members panned to one
  side each), but a phone on that mix then hears one mix per ear. If Mix 1/2 carries only the DAW playback, input levels belong to the
  Mix 3/4 cue alone.

## 3. What a mode is

- **Mode = view + interface scene + view options.** The scene is the mixer state (routing, levels, pans, sends, output sources, A/B
  function, knob target): the keys the presets already save (`DeviceModel.presetSnapshot` / `restore` / `fittingPreset`). View options:
  channel names, hidden channels, meter reference, which groups are open.
- **Switching** saves the scene of the mode you leave and applies the scene of the mode you enter (its template the first time). The cue
  mixes of Monitor survive a week in DAW mode.
- **Global, never changed by a mode**: rate, clock source, digital ports, stereo-pair engines. A mode says what it expects (Console: ADAT
  in/out, the converter's clock) and offers the change; it never makes it.
- **Safety:** a switch that would raise any output level asks first and lists the outputs (switching to Console: Line 1–4 to 0 dB).
- Changes made from the front panel (knob, A/B), the status menu or another app belong to the current mode's scene.
- **Custom modes:** ＋ duplicates any mode with its own name, scene and view options ("Drums tracking", "Mixdown with outboard"). The
  status menu lists the modes.
- Stored panel-side per unit GUID (Application Support, beside the presets). **No driver change.**

## 4. The four modes

| | Monitor | DAW | Console | Hybrid |
|---|---|---|---|---|
| Use | direct monitoring while tracking | monitoring in the DAW; mixing in the box | the 1814 as converter for an analog console | outboard inserts and reamp from the DAW |
| Scene | cue mixes on Mix 1/2, Mix 3/4, Aux | passthrough | passthrough, every interface level 0 dB | passthrough; loop sends 0 dB; loop returns out of the cue mixes |
| View | sources (mono) → cue mixes → outputs, with wires | input meter bridge (16, names, Core Audio channel, peak hold, clip latch), output meters, control room | channel table with console labels, linear calibrated meters with a 0 VU mark, clock and ADAT status | loop cards (DAW out → output → device → input → DAW in) with send/return meters and the level difference; other inputs; compact control room |
| Knob | user's choice (default Line 1/2) | monitor (Line 1/2) | phones only — **unverified** whether the FW1814 Level Controller accepts the phones group (its panel offers 4 groups); if not, the knob does nothing in Console and the phones keep their own front knobs | monitor |
| A/B button | Phones 1 source or direct monitoring | Phones 1: DAW 1/2 ⇄ DAW 3/4 (cue) | as set | as set |
| Protects | shared levels and the pair rule, explained | — | unity lock | sends at unity, no doubled returns |

Details:

- **Monitor (mono first).** Each cue mix is MONO (default) or STEREO. In a mono mix the panel centres the members' pans and shows no pan
  control; in a stereo mix each source has a pan. Faders are the sources' levels in the selected mix. Template: Monitors (stereo, DAW 1/2) on
  Mix 1/2 → Line 1/2; first cue on Mix 3/4 → Line 3/4 and/or a phone; second cue on the Aux → a phone. The panel shows the capacity ("3 of
  3"), marks a source whose level is shared by Mix 1/2 and Mix 3/4, joins inputs in pairs behind the scenes (a partner the user left out
  goes to −∞) and offers the split into two mono mixes with its warning. ADAT inputs fold into one column until needed.
- **DAW.** No mixer on screen. Control room: monitor level on the front knob, speakers A (Line 1/2) / B (Line 3/4), phones 1 and 2 with
  DAW 1/2 or a DAW cue on 3/4, mute, dim. Speakers B: the panel routes DAW 1/2 into Mix 3/4 and mutes Line 1/2; while B is on, DAW 3/4 is
  not on Line 3/4.
- **Console.** Unity lock keeps Line 1–4 at 0 dB (ADAT out is direct anyway); meters are linear dBFS with a selectable reference (0 VU =
  −20 / −18 / −14 / −12 dBFS); labels are the console's. The clock chip warns when a converter should be master.
- **Hybrid.** Each loop shows its send (the output meter) and return (the input meter) and the difference, for gain matching. "Send at
  0 dB" and "Hear return in cue" per loop. The DAW measures the loop latency (ping); the panel shows the bench round trip (≈ 9 ms at
  48 kHz / 64 frames, CLAUDE.md) as a reference only. A free-sends note says which outputs are left.

A **Classic** mode (today's tabs) stays available during the transition so nothing is lost at any step (§7).

## 5. Other models

The modes are generic; their views read the profile (`DeviceProfile`, `ChannelLayout`). Audiophile: 2 analog + S/PDIF in, four hardware
mixes (Mix 1/2, 3/4, S/PDIF, Aux). FW410: 2 in, 8 line out, five mixes + Aux, phones on several mixes at once: DAW and Console modes fit
it naturally. The hardware facts of §2 were checked for the FW1814 only; the other models' equivalents are to be read from their kext
models (`FW410MixerMap.h`, Audiophile profile) before their views are drawn.

## 6. Unverified (settle with meter readings, only when the user asks — no formal testing, R123)

1. Aux send distribution to L/R per channel (decides whether the Aux can be a mono mix).
2. Pan law at centre (−3 or −6 dB): decides whether one source can sit at 0 dB in two mono mixes split from one bus.
3. Recording is pre-mixer on our unit.
4. The FW1814 Level Controller with the phones group (`MARotaryMask.headphone`, bits 56–57).
5. Whether our driver's mixer works at 88.2/96 kHz (the manual says the mixer page is greyed there).
6. Window size on the 11-inch MacBook Air (1366 × 768 is Apple's spec; not measured, v1 §6).

## 7. Phases (each ends with `--demo --snapshot` PNGs on the build PC and the layout tests)

- **M0** this proposal + mockup — **user decision**.
- **M1** mode framework: per-GUID mode store (scene + options), switching with the rising-level check, mode switcher in the header and the
  status menu; today's tabs become the Classic mode.
- **M2** DAW view (meters + control room) — the simplest, uses only existing keys.
- **M3** Console view.
- **M4** Monitor view (mono-first Flow) + the allocation logic (cue mixes → Mix 1/2, Mix 3/4, Aux, pair rule, shared levels, split) as
  pure logic under `make test`.
- **M5** Hybrid view.
- **M6** custom modes; per-mode names and hidden channels.

Constraints from v1 §6 still apply: Swift 4.1 / AppKit / 10.13 SDK, GPU-first meters, no driver change, fits 1366 × 768.

## 8. Questions for the user

1. Are these four modes the right set? Anything missing (live / DJ with the A/B cue, rehearsal)?
2. Console unity lock: restore 0 dB actively when something changes it, or only warn?
3. Keep Classic permanently, or only during the transition?
4. Which mode on first launch (DAW is the safest: it touches nothing audible)?
