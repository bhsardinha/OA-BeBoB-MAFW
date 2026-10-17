# FireWire 1814 — Manual Reference (Mac only)

Distilled from `docs/manual.pdf` (M-Audio FireWire 1814 Users Manual, 2004,
corresponds to v1.0.0.0 driver / panel+driver `5.10.0.5517x22`, firmware
`2004/04/20 14:59:30`, bootloader `2004/02/25 09:15:03`, hardware
`model: 131 version: 1`, GUID prefix `000D-6C04`). Windows-only features are
intentionally omitted. The purpose is to understand what the interface is,
what the control panel exposes, and where the hardware ends and software
begins — not to re-document the manual.

Provenance: every section cites the manual page it comes from
(`manual.pdf §N`). Nothing here is invented. Where the manual is silent or
ambiguous for the Mac path, it is labeled **unverified** with the check that
would resolve it.

---

## 1. What the box is

An 18-in / 14-out 24-bit FireWire audio+MIDI interface built on BridgeCo
BeBoB (DM1000). Half-rack, bus-powerable over 6-pin FireWire. Marketed as a
portable multi-channel I/O for a laptop studio with mic preamps, headphone
cueing, and digital I/O that can be ADAT **or** S/PDIF (TOSLink shares both
roles, mode-switched in software). `manual.pdf §1, §3`.

### I/O count (where "18/14" comes from)

**Inputs (18 total):**
- 8× analog (ch 1–2 are Neutrik combo mic/inst **or** rear 1/4" line, toggled
  by the front Mic/Line switch; ch 3–8 are rear 1/4" TS line only).
- 8× ADAT (optical TOSLink) — reduced to 4× when S/MUX is enabled for
  88.2/96 kHz.
- 2× S/PDIF — selectable between coaxial RCA and optical TOSLink; the two
  share one logical port (`spdif input port` selector, hardware page).

**Outputs (14 total):**
- 4× analog 1/4" TRS balanced/unbalanced (Line Out 1–4).
- 8× ADAT (optical) — again reduced to 4× in S/MUX mode.
- 2× S/PDIF — **the signal is mirrored on BOTH coaxial and optical outputs
  simultaneously**; there is no way to send different streams to the two
  S/PDIF jacks. `manual.pdf §6 note after S/PDIF LEDs`.

**MIDI:** 1× in, 1× out on 5-pin DIN via the DE15F breakout cable (16
channels each). `manual.pdf §4, §6 rear description 19`.

**Sync:** word clock in/out on BNC (also via breakout). `manual.pdf §4`.

### Hardware controls (purely analog or hardware-latched, not driver-visible)

- **Mic/Inst input gain pots (ch 1 & 2):** analog preamp gain, >40 dB range.
  Not reflected in software; the DAW sees whatever level the pot produced.
  `manual.pdf §6 front 4`.
- **Mic/Line switch (ch 1 & 2):** per-channel toggle between front Neutrik
  jack and rear line input. No software mirror. `manual.pdf §6 front 2`.
- **20 dB Pad switch (ch 1 & 2):** per-channel attenuator. No software mirror.
  `manual.pdf §6 front 3`.
- **Phantom power switch:** single switch, feeds both XLR mic inputs
  simultaneously. No per-channel enable; red Phantom LED. `manual.pdf §6
  front 16, 17`.
- **Power switch + Power LED (blue):** push-on/push-off. `manual.pdf §6
  front 14, 15`.

Signal/clip LEDs (per mic channel), S/PDIF in/out LEDs, ADAT in/out LEDs,
and line in/out signal LEDs are purely hardware indicators (threshold
driven: Signal at −30 dB, Clip at 1 dB below full scale, Line Out LEDs at
−60 dB, Line In LEDs at −30 dB). `manual.pdf §6 front 5, 8, 9, 10`.

### Hybrid controls (hardware encoder, software-defined behavior)

All three front-panel knobs that look like "volume pots" (Headphone 1,
Headphone 2, Level Controller) are **infinite-turn rotary encoders**, not
analog potentiometers. There is no analog attenuator in the signal path for
these — the encoder increments/decrements a software value that the kext
holds in the device property dictionary, and the DAC/mixer applies that
value in the digital domain. Session recall is therefore possible for all
three (unlike the Mic/Inst gain pots, which are true analog). `manual.pdf
§6 front 7, 13`.

- **Headphone 1 & 2 level encoders (front, #7):** rotary encoders dedicated
  to each headphone output. "Independent of output levels" in the manual
  means independent of the Line 1/2 and 3/4 output faders (so cueing
  doesn't disturb the monitor mix) — **not** independent of the digital
  realm. They drive the `phones 1` / `phones 2` fader value visible on the
  Output page, and the panel fader + hardware encoder are two views of the
  same stored value. `manual.pdf §6 front 7`.
- **Level Controller (front rotary encoder, #13):** physically an encoder.
  Its *assignment* is set in the control panel's right-side pane
  ("Level Controller Assignment"). It can address one of four fader groups:
    - `SW Return` (sw rtn 1–4)
    - `Output` (analog 1/2 out, 3/4 out, and the 8× ADAT or 2× S/PDIF
      depending on digital port selection) — **default**
    - `Input` (the 8 analog + 8 ADAT or 2 S/PDIF monitor levels)
    - `Aux Send` (L and R virtual aux buses)
  Users can further deselect individual channels within a group via each
  strip's `ctrl` button; the panel remembers these per-group modifications.
  `manual.pdf §6 front 13` + `§9 Global Menus Level Controller Assignment` +
  `§10 Using the Level Controller Assignments`.
- **A/B momentary switch (front, #12) + A/B status LED (#11):** physical
  push button, but its meaning is one of two modes set in the control panel
  (hardware page → `a/b switch function`):
    1. **Direct Monitoring on/off** (default). A = DM off (inputs muted in
       the mixer), B = DM on. Also controllable from ASIO Direct Monitoring
       on Windows — on Mac the switch is purely manual. `manual.pdf §6
       front 11, 12` + `§9 Hardware A/B Switch Function` + `§10 Using the
       A/B Selector Switch`.
    2. **Headphone 1 source select.** The switch toggles Headphone Output 1
       between two of three possible monitor sources (any two of
       `1/2 out`, `3/4 out`, `aux`), configured via the `Set Source` dialog
       on the hardware page.
  **Headphone 2 is NEVER affected by the A/B switch** — it always follows
  the Output page's `phones 2` source assignment. `manual.pdf §6 front 6` +
  `§10 Monitoring From Your DJ Program note`.

---

## 2. The Software Control Panel (Mac)

On Mac OS X the control panel is a PrefPane under Apple Menu → System
Preferences. (`_extracted/prefpane/` + `_extracted/panel/` are the live
artifacts — see `docs/system-map/A2-panel-ghidra/` and
`docs/system-map/A6-prefpane/` for implementation; this doc is UX-level.)
`manual.pdf §9 opening the Control Panel on the Mac`.

### Mode-dependence on sample rate (critical constraint)

The panel's capabilities collapse as the sample rate rises:

| Rate                | Mixer page                | Output page            | Aux send |
|---------------------|---------------------------|------------------------|----------|
| 44.1 / 48 kHz       | fully functional          | fully functional       | yes      |
| 88.2 / 96 kHz       | **grayed out** (meters live) | aux bus **removed** | **no**   |
| 176.4 / 192 kHz     | grayed out                | aux bus removed        | no       |

Headphone 2 is also forced at 192 kHz: HP1 monitors outs 1/2, HP2 monitors
outs 3/4 (the normal `phones 2` source selector doesn't apply).
`manual.pdf §9 opening paragraph (three modes)` + `§6 front 6`.

### Right-side global pane (visible on every tab)

- **Mixer Settings:** `reset`, `save`, `load`, `delete` — load/save/delete
  go through a Mac file dialog. Reset restores the defaults in §14 of the
  manual (summarised below).
- **Level Controller Assignment:** radio group (sw return / output / input
  / aux send) — mirror of the front encoder's binding, with per-channel
  `ctrl` override.
- **Main Output:** `mute` (red when active, kills ALL analog + ADAT + S/PDIF
  outputs) and `dim` (green when active, −20 dB). `manual.pdf §9 Global
  Menus Main Output`.

### 2.1 Mixer page

Purpose: routing and software-mixing of hardware inputs + DAW returns to
the physical output pairs. Scrollable; "up to 12" virtual channels
depending on which digital port mode is active.

Columns present (dependent on digital port selection):
- `sw rtn 1/2`, `sw rtn 3/4` — software returns from DAW output pairs
  1 and 2 (bus 1/2, bus 3/4). Always visible at 44.1/48 kHz.
- `adat rtn bus` — a single bus-level return/meter for the 8-channel ADAT
  return (when optical = ADAT). **Hardwired to the ADAT output bus** — not
  routable in software. `manual.pdf §14 defaults`.
- `spdif sw rtn` — present only when optical = S/PDIF. Hardwired to S/PDIF
  out.
- Hardware input strips: `ana 1/2 in`, `ana 3/4 in`, `ana 5/6 in`,
  `ana 7/8 in`, plus `spdif in` or `adat 1/2 in … adat 7/8 in` depending on
  the digital port selection.

Per-strip controls:
- `aux L`, `aux R` knobs (feed the L/R aux send master on the output page).
- Volume fader (per channel; `link` ties the pair).
- `pan L`, `pan R` pots.
- `link`, `solo`, `mute` buttons.
- Output routing buttons `1/2` and `3/4` (toggles whether the strip's
  signal is summed into the analog Out 1/2 or Out 3/4 bus; multiple strips
  can target the same bus — clipping is possible, not auto-attenuated).
  `manual.pdf §9 Mixer page Output Routing note`.
- `ctrl` button (appears when its matching Level-Controller-Assignment
  group is selected).

Meters at the bottom show the `output bus` and `aux` summed levels.

Default: hardware input strips are **unassigned** (no output routing
selected). Nothing is monitored unless the user clicks `1/2` or `3/4` on
each input strip. `manual.pdf §9 Mixer page Hardware Inputs NOTE`.

### 2.2 Output page

Purpose: control of each physical output and the two headphone buses.

Columns:
- `1/2 out` — fader pair, balance, link, mute, solo, `main/aux` toggle.
- `3/4 out` — same.
- `adat bus` or `spdif out` (depending on digital port selection) —
  fader(s), balance, mute, solo, `main/aux` toggle. ADAT shows as an
  8-channel bus (no individual channel faders at the kext level — the panel
  only exposes bus trim).
- `aux` — the aux-send master; `bal`, fader(s), `link`, `mute`.
- `phones 1`, `phones 2` — faders, `bal`, `link`, `mute`, and the
  **monitor source select** (`1/2`, `3/4`, or `aux`).

The `main/aux` toggle on an output strip is a bus swap: `main` (blue) =
mixer-page routed signal; `aux` (green) = aux-send master signal. This is
how outboard-FX returns get patched to outputs. `manual.pdf §9 Output page
Main/Aux Button`.

Headphone physical level is **not** on this page — the front-panel
headphone pots are analog (see §1). The panel only sets the digital level
feeding the headphone bus and chooses its source.

Bottom meters: `software return bus` (sw rtn pairs) + `input bus` (all
hardware inputs including ADAT 1/2 … 7/8 where applicable).

### 2.3 Hardware page

Four quadrants (on Mac — the ASIO/WDM buffer-size field is absent because
Core Audio drives buffer size from the host app, and the ASIO Direct
Monitoring checkbox is absent for the same reason):

1. **Sample Settings** — `sample rate detected` readout. No buffer-size
   field on Mac OS X; the Mac OS 9 bit-depth selector is also irrelevant to
   10.13.
2. **Sync Source** — four radios:
   - `internal - digital inputs muted` (default) — device is clock master,
     ignores digital inputs for sync.
   - `internal - digital inputs unmuted` — device still master but digital
     inputs are unmuted (use when an outboard device is slaving to the
     1814's S/PDIF out and returning its own signal, so you still hear it).
   - `external - digital` — slave to whichever digital input is selected
     (ADAT or S/PDIF per `digital port selection`).
   - `word clock` — slave to BNC Word Clock In.
3. **Digital Port Selection** — three selectors:
   - `input: adat | spdif`
   - `output: adat | spdif`
   - `spdif input port: optical | coaxial` (only when input = spdif)
   - `adat S/MUX` checkbox (needed to drop ADAT to 4 channels at 88.2/96).
4. **Monitoring Settings** — the `a/b switch function` radio group
   (`direct monitoring` / `headphone out 1 A/B`) and the `set source`
   button that pops the three-source-by-two-row (A/B × 1/2/3/4/aux) matrix.

### 2.4 About page

Shows `panel`, `driver`, `bootloader`, `firmware`, `hardware model/version`,
and `GUID`. The driver version string in the shipping bundle is
`5.10.0.5517x22`. `manual.pdf §9 About page`.

---

## 3. Routing — the actual signal paths

Three independent planes converge at the output strips:

```
                                ┌─ output buttons (1/2, 3/4)
Hardware inputs ──► mixer strip ┤                              ┌─► output bus 1/2 ─► 1/2 out strip ─► Line Out 1/2
                                └─ aux L / aux R knobs ─► aux  │
                                                         master ├─► output bus 3/4 ─► 3/4 out strip ─► Line Out 3/4
DAW sw returns ──► sw rtn strip ─► output buttons (1/2, 3/4) ──┤
                                                                └─► aux master ─► (any output pair, via main/aux switch
                                                                                  on the destination output strip)

ADAT return (bus-level, from DAW) ──────────────────── hardwired ──► ADAT output bus ─► ADAT optical out
S/PDIF sw rtn (from DAW)          ──────────────────── hardwired ──► S/PDIF output bus ─► S/PDIF (BOTH coax + optical)

Any output bus (1/2, 3/4, aux) ──► phones 1 source select ──► Phones 1 digital fader (set by HP1 encoder OR Output-page fader) ─► HP1 jack
                                   phones 2 source select ──► Phones 2 digital fader (set by HP2 encoder OR Output-page fader) ─► HP2 jack

Front A/B switch ─► (hardware-page mode) ─► toggles Phones 1 source between the two Set-Source selections
                                           OR  toggles global Direct-Monitor on/off (mutes all mixer-page input strips)
```

Notable routing properties:
- **S/PDIF out is mirrored to coax + optical.** No independent routing.
  `manual.pdf §6 front 8 NOTE`.
- **ADAT return bus and S/PDIF sw rtn bypass the mixer page** — they are
  hardwired to their respective digital output buses. The mixer page only
  exposes the first four software returns (sw rtn 1/2 and 3/4) as
  routable; sw rtn 5/6 onwards fall into the ADAT bus channels and have no
  mixer-page strip. `manual.pdf §14 defaults` (`spdif sw rtn hardwired to
  spdif out`, `adat rtn bus hardwired to adat output bus`).
- **Direct monitoring is "hardware" direct monitoring at the kext level.**
  Setting an input strip's output button to 1/2 or 3/4 sums that input
  into the hardware output bus with near-zero latency. There is no DSP in
  the path — it is a bus mix inside the DM1000. `manual.pdf §9 Mixer page
  intro, §10 Direct Monitoring`.
- **Aux send is the FX insert path.** The intended pattern (per `manual.pdf
  §10 Adding Effects While Direct Monitoring`): aux send feeds the aux
  master; the user flips an output strip's `main/aux` toggle to `aux`,
  routes that physical output to an outboard effects unit, feeds the FX
  return back into one of the hardware input strips (typically S/PDIF in),
  and routes that strip to 1/2 (the actual listening bus). This is why aux
  send disappears above 48 kHz — DSP capacity on the DM1000 can't support
  it at higher rates.

### Defaults (`manual.pdf §14`)

| Item                                | Default                                             |
|-------------------------------------|-----------------------------------------------------|
| mixer faders                        | 0 dB                                                |
| output faders                       | 0 dB                                                |
| output channels assignments         | `main`                                              |
| phones 1 & 2 source                 | `1/2`                                               |
| sw rtn 1/2 & 3/4 routing            | to buses 1/2 & 3/4                                  |
| `spdif sw rtn`                      | hardwired to S/PDIF out                             |
| `adat rtn bus`                      | hardwired to ADAT output bus                        |
| aux send knobs                      | all the way down                                    |
| analog & digital input channels     | **unassigned** (nothing monitored out of the box)   |
| pan knobs                           | hard L / R                                          |
| balance knobs                       | centered `<C>`                                      |
| mute / solo / link                  | all off                                             |
| main level controller assignment    | `output` (all output channels selected)             |

---

## 4. Hardware ↔ Control-Panel relationship (quick reference)

| Front/rear control                  | Nature                                     | Software mirror?                                                            |
|-------------------------------------|--------------------------------------------|-----------------------------------------------------------------------------|
| Mic/Inst gain pot (ch 1, 2)         | Analog preamp gain                         | **No** — invisible to driver                                                |
| Mic/Line switch (ch 1, 2)           | Analog mux                                 | **No**                                                                      |
| Pad switch (ch 1, 2)                | Analog attenuator                          | **No**                                                                      |
| Phantom switch (global)             | Analog +48 V                               | **No** (there is a Phantom LED only)                                        |
| Signal / Clip / S/PDIF / ADAT LEDs  | Hardware indicator                         | n/a                                                                         |
| Headphone 1, 2 level encoders       | Rotary encoder, software value             | **Yes** — same stored value as the Output-page `phones N` fader             |
| Power switch                        | Analog                                     | n/a                                                                         |
| **Level Controller encoder**        | Hardware encoder, software binding         | Yes — right-side `Level Controller Assignment` radios + per-strip `ctrl`    |
| **A/B momentary + LED**             | Hardware button, software mode             | Yes — hardware page `a/b switch function` + `set source`                    |
| Line Outputs 1–4                    | Analog TRS                                 | `1/2 out`, `3/4 out` strips (fader, mute, bal, main/aux)                    |
| Line Inputs 1–8                     | Analog TS                                  | `ana N/N in` mixer strips (fader, pan, aux, routing)                        |
| ADAT optical in/out                 | Digital                                    | `adat 1/2 in … 7/8 in` mixer strips when ADAT = input; `adat bus` on output |
| S/PDIF (coax & optical)             | Digital, shared mode                       | `spdif in` mixer strip when S/PDIF = input; `spdif out` on output           |
| MIDI DIN                            | Via breakout                               | Core MIDI (CoreMIDI CFPlugIn in `_extracted/midi/`)                         |
| Word Clock BNC                      | Via breakout                               | `word clock` sync-source radio on hardware page                             |
| FireWire 6-pin ×2                   | Transport + bus power                      | n/a                                                                         |

---

## 5. What the interface does well (perks)

- **Near-zero-latency hardware direct monitoring at the bus level** — no
  DSP, no software round-trip; latency is OHCI jitter + a cycle. The
  mixer-page input strips are the direct-monitor UI.
- **Aux send → outboard FX during direct monitoring** at 44.1/48 kHz.
  Lets you add hardware reverb to a dry monitor path without routing
  through the DAW.
- **Dedicated headphone level encoders** on the front panel — digital
  faders driven by infinite-turn encoders, mirrored to the Output-page
  `phones 1 / 2` sliders. Independent of the Line 1/2 and 3/4 output
  faders, so cueing doesn't disturb the monitor mix.
- **A/B switch for cueing / DJ workflow** — momentary hardware button,
  software-defined meaning. Headphone 1 can A/B between any two of three
  sources (1/2, 3/4, aux).
- **Assignable hardware Level Controller** — one physical encoder that can
  drive outputs, inputs, sw returns, aux sends, or any custom subset.
- **Phantom + 20 dB pad on both XLR mic inputs**, Neutrik combo jacks,
  signal/clip LEDs per channel.
- **Mode-switchable optical port** (ADAT 8ch / ADAT S/MUX 4ch / S/PDIF 2ch)
  — one TOSLink serves three roles.
- **Simultaneous S/PDIF out on coax and optical** — convenient for
  patching two destinations.
- **Save/load/delete mixer presets** via the right-side pane.
- **Core Audio integration with mute + dim** master buttons on every page.
- **Bus-powerable** over 6-pin FireWire (no PSU needed on properly powered
  hosts).

## 6. What it doesn't do (shortcomings / limits)

- **Mixer page is grayed out above 48 kHz.** All software routing /
  monitoring must be done inside the DAW at 88.2 kHz and up. The hardware
  bus still mixes, but you can't change it from the panel. `§9 opening`.
- **Aux send is 44.1 / 48 kHz only.** No outboard FX send at high rates.
  `§10 Adding Effects While Direct Monitoring opening`.
- **Only inputs 1 / 2 support 192 kHz.** Inputs 3–8 cap at 96 kHz. `§3
  NOTE after features paragraph`.
- **Optical S/PDIF caps at 48 kHz;** coaxial S/PDIF caps at 96 kHz. 192
  kHz digital is not supported at all. `§4, §13`.
- **S/PDIF out is always mirrored to both jacks.** You can't send a
  separate stream to coax vs optical. `§6 front 8 NOTE`.
- **Headphone 2 never participates in A/B source selection** — it always
  takes the Output-page `phones 2` source. Only Headphone 1 can be
  toggled by the hardware A/B switch. `§10 note under Monitoring From
  Your DJ Program`.
- **At 192 kHz the headphone source is forced** (HP1=1/2, HP2=3/4); the
  software source selector is bypassed. `§6 front 6`.
- **Mic preamp gain, pad, mic/line, phantom are all analog switches/pots**
  with no software recall. A session reload can't restore the mic/inst
  input gain structure — the user has to re-set the pots. (Note: the
  Headphone 1/2 and Level Controller knobs *are* software-recall-able
  because they are rotary encoders, not analog pots — only the Mic/Inst
  gain pots are truly analog.)
- **Default input routing is "no monitoring"** — fresh install hears
  nothing until the user routes each input strip to 1/2 or 3/4.
  `§9 Mixer page Hardware Inputs NOTE`.
- **Summing multiple mixer strips to the same output bus can clip** —
  there is no automatic headroom protection. `§9 Mixer page Output
  Routing note`.
- **Hot-plugging is explicitly discouraged** (manual warns of permanent
  damage to the port on some hosts). On Mac you must shut down before
  connecting/disconnecting. `§3 IMPORTANT IEEE 1394 Users` + `§8
  ATTENTION`.
- **The ADAT return bus and S/PDIF sw rtn are hardwired to their
  respective digital outputs** — no mixer-page routing. If you want to
  mix a DAW track into the analog mix instead of out ADAT, you have to
  send it on sw rtn 1/2 or 3/4. `§14 defaults`.
- **AC-3 / DTS pass-through on S/PDIF out is Windows + Mac OS X 10.3
  only.** Not supported on 10.4+ (so irrelevant to the 10.13.6 target).
  `§3 paragraph "The S/PDIF digital output also supports pass-through…"`.
- **Only sw rtn 1/2 and sw rtn 3/4 appear as routable strips in the
  mixer.** sw rtn 5–12 feed the ADAT bus with no mixer-level control.

---

## 7. Mac-specific notes (10.13.6 context)

The manual predates 10.13 by nine years, but the panel+kext architecture
hasn't changed in the shipping `_extracted/` bundle. For 10.13 work:

- The control panel runs as a PrefPane (`_extracted/prefpane/`) backed by a
  user-space panel app (`_extracted/panel/`). On modern 10.13 we drop the
  PrefPane and keep an AppKit panel (see `docs/Proposal.md` §4). The
  surface documented above is what the AppKit replacement must preserve.
- There is no ASIO/WDM buffer field on the Mac hardware page. Core Audio
  drives buffer size through `IOBufferFrameSize` (see auto-memory
  `project_fw1814_buffer_sizes.md`).
- No ASIO Direct Monitoring. The front-panel A/B switch in Direct
  Monitoring mode is manual — pressing it mutes/unmutes the mixer-page
  input strips directly. `§10 Direct Monitoring NOTE on front panel A/B`.
- Core MIDI integration is via a CFPlugIn (`_extracted/midi/`), not OMS
  (OMS is the OS 9 path only).
- Mute/dim behavior at the hardware level: `mute` kills all analog + ADAT
  + S/PDIF outputs; `dim` lowers hardware output by 20 dB. These are
  bus-level actions handled by the kext, not panel-local gain changes.
  `§9 Global Menus Main Output`.

### Cross-references to the system map

- Front-panel control decoding (A/B, Level Controller, phantom, pad): see
  the kext property dictionary in `docs/system-map/A1-ghidra/` Q15/Q15b
  (`FWPropertyDict`, `MA*` keys).
- Panel ↔ kext channel (why these knobs don't go through the vendor user
  client): see `project_panel_setproperties_channel.md` (auto-memory) +
  `docs/system-map/B5-sl-dtrace-truth/README.md §10`.
- Clock-source bijection (Internal muted / Internal unmuted / External
  digital / Word Clock → `MAClockSource` values): B5 §10.
- Hardware vs software gain locus (ch 1/2 preamp gain is a hardware pot,
  all other levels are software): auto-memory
  `project_fw1814_hardware_capabilities.md`.
- Buffer-size ladder (CoreAudio-driven; the panel has no size selector):
  auto-memory `project_fw1814_buffer_sizes.md`.
