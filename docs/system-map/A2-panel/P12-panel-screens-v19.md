# A2-P12 — v1.9 panel screens: layout and dependencies (user screenshots, PN.1)

**Source:** 11 screenshots of the v1.9 panel (panel 1.4, driver 1.9, build 01.09.003) on the Snow Leopard Mac, taken by the user on 2026-10-15/16, in `screenshots/`. Plus the user's written explanations of the same day. **Version skew:** these are v1.9 captures; `_extracted/` is v1.11.2 (memory `feedback_version_skew_v19_vs_v1112`). Anything below that disagrees with a decompile of the v1.11.2 or v1.5.2 panel is a skew candidate, not a proven error.

## Observations (screenshots)
- Window: tabs `mixer`, `output`, `hardware`, `about`; view switch top right (full, wide meters, narrow meters); right sidebar on every tab: `settings` (load, save, reset, delete), `level controller` (knob + radios sw rtn bus / output bus / input bus / aux send), `main output` (mute, dim).
- There is **no buffer-size control and no Global Settings tab** (user: buffer size is CoreAudio-only on the Mac, chosen in the DAW).
- Mixer, ADAT/ADAT: strips `1/2 sw rtn`, `3/4 sw rtn`, `adat rtn bus` (scale only), `analog 1/2 … 7/8 in`, `adat 1/2 … 7/8 in`. S/PDIF/S/PDIF: `spdif sw rtn` replaces the ADAT bus, one `spdif in` replaces the ADAT input strips. Input strips have aux L/R knobs, faders, pan knobs, ctrl/solo/link/mute, out 1/2 / 3/4. sw rtn strips have no pan knobs.
- Output: `1/2 out`, `3/4 out` (bal, faders, ctrl, solo, link, mute, main), `adat bus` or `spdif out` (with main), `aux` (ctrl disabled when the encoder is not on aux send), `phones 1`, `phones 2` (mon 1/2, 3/4, aux).
- The areas at the bottom of mixer and output are **live meters** (user), not labels: mixer = output bus + aux bus; output = software return bus + input bus (input labels follow optical in).
- Hardware: `sample settings`, `sync source` (4 radios), `digital port selection`, `monitoring settings` (radios `direct monitoring` and `headphone out 1 A/B` + `set source`).
- HP A/B sheet: A and B each choose one of Out 1/2, Out 3/4, Aux; the option held by the other side is disabled; Set is disabled until a change.
- Meter windows: wide (input 1–8 + spd, sw rtn 1–4 + spd, output 1–4 + spd) and narrow (same, rows labelled 1–8/sL/sR).
- Output with link off: `3/4 out` shows balance `R35` with the two faders at different positions.

## User-stated behaviour (not visible in a still)
1. Mixed optical modes: input strips follow optical in, return/output strip follows optical out.
2. `spdif input port` is greyed only when input = ADAT.
3. `adat S/MUX` is greyed when output = S/PDIF.
4. S/MUX is "S/MUX II", works up to 96 kHz; the ADAT input strips 5/6 and 7/8 are greyed in the mixer (user: "it appears so").
5. `ctrl` lets the front-panel rotary encoder control that strip. The encoder controls one group at a time (sw rtn, out, in, aux); strips of the other groups show ctrl greyed; strips of the selected group are on by default and can be toggled off.
6. `link` keeps the pair of faders together; with link off, pulling one fader moves the balance (user: effect similar to pan).

## Further user statements (2026-10-16)
7. The output tab's `aux` strip is the **aux out bus**: the sum of the mixer strips' aux sends. It reaches hardware outputs through the `main`/`aux` toggle under `1/2 out`, `3/4 out` and `spdif out`, or through the phones source `aux`. The encoder's "aux send" group addresses this strip (its ctrl button).
8. The lock icon and the green/red background of the sample-rate display are redundant sync indicators: green + closed lock = synced (internal or external), red + open lock = not synced.
9. `manual.pdf` describes an older version. Aux sends still work at higher sample rates. At 88.2/96 kHz the only greyed items are the last four ADAT channels: mixer input strips adat 5/6 and 7/8, the matching rtn/output bus channels 5-8 and the input-bus meters.
10. `adat S/MUX` applies only to ADAT. When it does not apply (output S/PDIF) it is greyed but keeps its on/off state.

## Rate behaviour: reference panel vs device (2026-10-16, screenshot `Screen shot 2026-10-16 at 12.36.09 AM.png`)
- **Observation (screenshot):** at 192000 Hz the panel (hardware tab) looks as at any rate, while Audio MIDI Setup shows "FireWire 1814 Multichannel", 2 in / 4 out, 4ch-24bit, outputs "Output Analog 1..4". The user states the panel does not change at 192 kHz and greys ADAT 5-8 only when the S/MUX box is on, whereas the Audio Device behaves correctly.
- **Cross-check (our clone's table, from the reference binary):** `kext/src/FWRates.h`: `kFW176_192` (@0x4ab80) = 2 in / 4 out, groups analog only; `kFWADAT88_96` (@0x36580) = 12 in / 8 out, i.e. 4 ADAT channels per direction. `FWComposeDeviceChannels` selects the table from rate and mode bits 3/4 only; S/MUX (bit 5) is not an input.
- **Design decision for our panel (not the reference's behaviour):** derive panel state from the same channel table the kext publishes: 88.2/96 kHz greys ADAT 5-8 whatever the S/MUX box says; 176.4/192 kHz removes all digital strips/buses and greys the digital port card and external-digital sync (the last two are **inferred**, no screenshot). S/MUX box keeps its state when greyed. **Decided by the user 2026-10-16:** S/MUX switches on automatically (box locked on) at 88.2/96 kHz and is user-set at 44.1/48 kHz; at 192 kHz every digital control is hidden or greyed (digital strips/buses removed, port card and external-digital sync greyed); phones 1/2 and the aux out bus stay.
- Items 4 and 9 above therefore describe the reference panel; ours is coherent with the device.

## Corrections to older docs (2026-10-16)
- **P5 `AdaptPortsToSettings`:** P5 says the ADAT-4ch toggle is enabled when optical-in OR optical-out is ADAT. The user observes it greyed when output = S/PDIF (item 3). **Unresolved conflict, version skew possible.** Check: re-read `CFW1814HardwareView::AdaptPortsToSettings @ 0x…` (a2-manual-crosscheck.log:51..89) and, if needed, a v1.9 DTrace/screenshot with input ADAT + output S/PDIF.
- **P5 / P8 / `BUILD-MANUAL`:** the Hardware tab has no buffer-size or ASIO/stereo-pair-engine controls in the v1.9 screens; there is no Global Settings tab (the "Virtual Rotary Controller Assignment" is the sidebar level controller). **Correction to P8 §4 labelling.**
- **P11 / wireframe revision 1:** direct monitoring is a radio of the hardware tab's `a/b switch settings`, not a separate toggle; matches `manual-reference.md` §1 and §2.3.
- **`manual-reference.md` §2 rate table and §6/§5 "aux send is 44.1/48 kHz only":** contradicted by the user (item 9). The manual is an older version; v1.9 behaviour wins. Mark those lines stale when `manual-reference.md` is next edited. **P6 (`AdaptToClockRate`, cookie 0x615):** the cookie read gates one element; whether it is the ADAT 5-8 greying is unverified.

## Open
Per-side S/MUX greying in mixed ADAT/S-PDIF modes (assumed: only the ADAT side in use).

## Sign-off
PN.1 wireframe approved by the user 2026-10-16 ("looks good"); final page saved as `PN1-wireframe.html` in this folder.
