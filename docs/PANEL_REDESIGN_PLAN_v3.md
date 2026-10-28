# Panel redesign — the Advanced window (plan v3, proposal)

Status: **proposal, approved in direction by the user (2026-10-27), details below waiting for confirmation** (branch `panel-redesign`).
No panel code has changed. Builds on `PANEL_REDESIGN_PLAN_v2.md` (the four modes and their rules, §3-§4 there still apply) and replaces
its placement: the modes are **not** a replacement of the panel.

Mockup: `docs/panel-redesign/PANEL_MODES_MOCKUP_v2.html` (menu bar menu, one or two units, 44.1/48/96 kHz switch). Screenshots:
`docs/panel-redesign/adv-*.png`.

## 1. User decisions (2026-10-27)

1. The four modes (DAW, Monitor, Console, Hybrid) are approved as presented.
2. They live in an **extra "Advanced" window inside the current panel app**, opened from the menu bar (status menu). The standard panel
   stays exactly as it is; there is no "Classic" mode.
3. Advanced exists **only for the FireWire 1814 and only at 44.1 / 48 kHz**.
4. It **opens in DAW mode**.
5. **Monitor mode, pan on mono sources:** every mono strip has a pan. Feeding a mono mix, it is greyed at centre; feeding a stereo mix, it
   is live, centre by default, full left / full right possible.
6. **Dual:** a second, extra (not main) scope for **two FW1814s used as one integrated device** (32 in / 24 out) on a Mac Pro with a
   separate FireWire bus per unit. It **always** assumes a macOS aggregate device. **Only the DAW and Console modes** in Dual.

## 2. Placement and gating

- Status menu: **"Advanced Panel…" (⌥⌘A)** right after "Open Panel" (order of `StatusMenu.swift`, user 2026-10-21). Shown only when the
  selected unit is an FW1814; greyed with "(44.1/48 kHz)" at other rates.
- If the rate leaves 44.1/48 while the window is open, the window shows why and offers the standard panel; it never changes the rate
  itself ("Set 48 kHz…" goes through Core Audio and asks first; rate changes while a client streams stay a known risk, CLAUDE.md issue 1).
- Scope bar under the header when two FW1814s are present: **A · B · A + B (Dual)**. Single-unit views use each unit's own Core Audio
  channel numbers; Dual uses the aggregate's (A first: in 1–16 A, 17–32 B; out 1–12 A, 13–24 B).

## 3. Monitor mode: pan rule (refines v2 §4)

| Mix the strip feeds | Pan control |
|---|---|
| mono mix on Mix 1/2 or Mix 3/4 | greyed, at centre (the panel centres the source) |
| stereo mix on Mix 1/2 or Mix 3/4 | live, centre by default, L127 … R127; double-click = centre |
| the Aux (mono or stereo) | greyed: the Aux has no per-channel pan (no pan key for aux sends, `MaKeys.swift`) |

Hardware limit kept visible: Mix 1/2 and Mix 3/4 share **one pan per input channel** (`MAInputPan2`, 18). If a stereo mix pans a source
that is also in a mono mix, the mono mix's handle turns amber, shows the real pan and says which mix sets it. Switching a mix to MONO
centres its sources unless a stereo mix pans them. DAW returns have no pan (unchanged).

## 4. Dual (two FW1814s as one aggregate) — DAW and Console only

Facts:

- The driver exposes each unit as its own Core Audio device; the official driver does the same (capture 94, R93).
  `MAEnableAggregateEngine` answers "unsupported" (`0xE00002C7`) in the reference v1.11.2 (Q-MAMAP row 37) and in our clone
  (`FWAudioDevice.h:126`). So Dual **is a macOS aggregate device**, which the panel creates and checks (CoreAudio
  `AudioHardwareCreateAggregateDevice`, 10.9+ — **unverified** on the 10.13 build: check the API in the 10.13 SDK before M7).
- Clock: unit A internal, unit B on **word clock** from A's BNC out (the FW1814 has word clock in and out, manual §4). The aggregate's
  clock source is A; drift correction off on B. **unverified:** that two word-clock-locked units keep the aggregate free of drift
  correction artefacts with this driver.
- Bandwidth: one FireWire bus per unit on the Mac Pro (user); one FW1814 in ADAT mode uses 976 bytes per cycle (capture 94).
- **Two FW1814s together are not yet run with this driver** (CLAUDE.md: on demand). Two different units on one bus are (R117).

Views:

- **DAW Dual:** input bridge in two lanes (unit A = DAW in 1–16, unit B = 17–32), output bridge in two lanes (1–12 / 13–24), control room
  with unit A's monitor knob and speakers A/B, four phones rows (A1, A2, B1, B2) each with its DAW source pair. Both mixers in passthrough.
- **Console Dual:** three columns — console → DAW 1–16 (A), console → DAW 17–32 (B), DAW 1–24 → console — compact rows, the same unity
  lock, reference level and labels; clock chip shows A → word clock → B.
- A "Dual setup" checklist (rate on both, B on word clock and locked, the aggregate present and configured, bus per unit) with what is
  missing and the one-click fixes the panel may offer (create the aggregate; never rate or clock silently).
- Monitor and Hybrid are disabled in Dual (each unit still has them in its own scope A or B).

## 5. Phases (replaces v2 §7)

- **M0** this plan + mockup v2 — user confirmation of §2-§4.
- **M1** Advanced window shell: status-menu item with the FW1814 / 44.1-48 kHz gate, header, mode switcher, mode store per GUID (scene +
  options), rising-level check on switches.
- **M2** DAW mode (single). **M3** Console mode (single). **M4** Monitor mode with the allocation logic and the pan rule (pure logic under
  `make test`). **M5** Hybrid mode. **M6** custom modes.
- **M7** Dual: unit pairing, aggregate create/check, DAW and Console Dual views. Needs two FW1814s on the Mac Pro for the first real run
  (only when the user asks; no formal testing, R123).

Each phase ends with `--demo --snapshot` PNGs on the build PC and the layout tests. Constraints unchanged: Swift 4.1 / AppKit / 10.13 SDK,
GPU-first meters, no driver change, window 1280 × 720.

## 6. Still open (from v2 §6, unchanged)

Aux send L/R distribution; pan law at centre; recording pre-mixer on our unit; the FW1814 Level Controller with the phones group (Console
knob); the mixer at 88.2/96 kHz no longer matters for Advanced (gated to 44.1/48).
