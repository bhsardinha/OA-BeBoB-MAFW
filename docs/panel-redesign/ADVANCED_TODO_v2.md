# Advanced and Dual windows — to-do v2 (2026-10-28)

Supersedes the open items of `ADVANCED_TODO.md` (v1 keeps the record of M0–M7). Plan: `docs/PANEL_REDESIGN_PLAN_v3.md` (+ v2 for the
mode rules). Branch `panel-redesign`, 12 commits ahead of `main`, not merged, not packaged.

Status keys: `[ ]` open · `[~]` partly done (says what is missing). Tags: **(user)** needs the user or a decision · **(hw)** needs the
hardware, only when the user asks (no formal testing, R123) · **(driver)** a kext change (load/unload only with the user's go-ahead).

## Where it stands
- Built and tested on the build PC (10.13.6, Xcode 9.4.1): Advanced window with DAW, Monitor, Console, Hybrid (M1–M5) and the Dual window
  with DAW and Console (M7). 50+ host tests of the pure logic pass.
- On the hardware: every mode's scene written and read back from the driver on both FW1814s (capture 108); the empty knob mask accepted
  (Console); A/B button on the second unit working after the FireWire contact fix (user).
- Not on the hardware yet: almost every interaction inside the views, anything audible, the whole Dual aggregate.

## 0. Done since this list was written (2026-10-28, demo snapshots on the build PC, 10.13.6)
- [x] **One window at a time** (user): the standard panel, a unit's Advanced window or the Dual window; opening one hides the others, and
      choosing the menu item of a window that is already open brings it forward instead of opening a second copy.
- [x] **Full HD sizing**: Advanced and Dual open at up to 1880 × 1000 of content (at least 1280 × 720, fitted to the screen) and are
      resizable.
- [x] **Dual DAW rebuilt around big meters** (user: too much text, too little meter): one full-width row of inputs and one of outputs,
      both units side by side, one dB scale per row, the phones as a group of the outputs, the setup reduced to a "Dual setup" chip, the
      mode bar reduced to "DAW · Dual". Two label lines above the meters, both centred over what they name with a rule each side: first
      "――― A · INPUTS ―――" / "――― A · OUTPUTS ―――" over the unit's whole block, then "――― ANALOG ―――", "――― ADAT ―――", "――― LINE ―――",
      "――― PHONES ―――" over each group's meters (snapshot `m7-snapshots-10.13/48000-adat-dual-daw.png`).
- [x] **Dual DAW: rename** by double-click on a meter's name.
- [x] **Single Advanced: broader meters** (user; Monitor left as it was): the DAW and Hybrid columns fill the width (up to 120 px) and
      the bars take about half a column (one bar up to 40 px, a stereo pair 2 × 20 px); the Console rows grow with the free height (up to
      48 px) and their meters thicken with them (up to 20 px) (snapshots `fullhd-snapshots-10.13/`).
- [x] **Single DAW and Hybrid take the Dual's labels** (user): "――― INPUTS → DAW ―――" / "――― DAW → OUTPUTS ―――" centred over all the
      columns, then "――― ANALOG · MIC/LINE 1–2 GAIN ON THE FRONT ―――", "――― ADAT ―――", "――― LINE ―――", "――― PHONES ―――" over each group;
      the cards' right-hand notes became tooltips. One drawing function (`drawRuleLabel`) serves both windows.
- [x] **CPU (user: "the panel must be a tool, not the reason of the session")**, measured with `tools/panel-cpubench.sh`-style runs on
      the build PC (iMac11,2, Radeon HD 4670, 10.13.6, full HD), demo with realistic peaks, panel % + WindowServer %:

      | view | before | after |
      |---|---|---|
      | Dual DAW | 30.8 + 42.4 | 6.1 + 7.7 |
      | single DAW | — | 3.8 + 6.4 |
      | single Monitor / Console / Hybrid | — | 1.6 + 8.4 / 2.9 + 8.0 / 3.0 + 4.9 |
      | Dual Console | — | 4.5 + 13.6 |
      | standard panel (reference) | 2.2 + 5.2 | 2.2 + 5.2 |

      What did it, in order of effect: (1) the static views draw into the root's one layer (`canDrawSubviewsIntoLayer`; one layer per view
      made Core Animation walk ~1000 layers a commit); (2) **MeterField**: each meter row (DAW, Dual DAW, Hybrid "other inputs") is one
      framebuffer filled from the driver's peak snapshot — one texel row per LED segment — scaled by the GPU (nearest), a static overlay for
      the segment gaps; (3) page flipping on two IOSurfaces: a tick writes only the bars that changed into the back buffer and flips (a new
      CGImage per tick cost ~2 % + ~5 %); (4) the framebuffer is in the window's colour space (device RGB made Core Animation colour-match
      every frame on the CPU, ~20 % in single DAW); (5) the meter columns have no subviews: they draw their texts and latch themselves and
      redraw only the readout / latch rectangle; the readout holds the peak 1.2 s. Meter behaviour = the standard panel's MeterBar (user:
      the ADAT bus of the standard mixer is "the way to go"): the bar follows the peak, the hold segment waits 1.2 s then drops 2 dB a tick.
      What stays: WindowServer's cost is per frame (30 Hz) and grows with the window; it did not depend on the bar size or the texture
      size. Snapshots of the Advanced / Dual windows are now WindowServer captures (`cacheDisplay` renders neither the flattened root nor
      the IOSurfaces).

## 1. Implement — built 2026-10-28 (type-checked, host tests pass; seen on screen only in the demo snapshots)
- [x] **M6 custom modes**: the header lists the four modes, the user's modes, then ＋. ＋ opens a menu: "New mode from <current>…"
      (today's mixer becomes the new mode's scene; its hidden channels are copied; nothing is written to the interface), "Rename…",
      "Delete" (custom modes only; the window goes to the base mode, asking first if an output gets louder). Scenes, the current mode and
      the view options are stored by mode key (built-in keys unchanged, so older files read as before). The status menu lists the unit's
      modes under "Advanced Panel…" (current one checked); choosing one opens the window in that mode (plan v2 §3).
- [~] **Front knob in the header**: a chip "Knob · Line 1/2 · −12.0 dB ▾" (the target and, for output pairs, the level it moves); its menu
      sets the target by the interface's rule (one group at a time, any pairs of it; choosing in another group replaces): Line 1/2, 3/4,
      In 1/2…7/8, DAW 1/2, 3/4, Aux out, Phones, Nothing. **Missing:** ring badges on the controls it drives.
- [x] **Monitor: solo** (S next to M on every strip; scenes already carry the solo keys and the passthrough clears them).
- [x] **Monitor: mono lines** (user, 2026-10-28, replaces "mono = centred" and the L/R split): Mix 1/2 and Mix 3/4 are each STEREO (one mix,
      a pan per source) or MONO = two independent lines, one tile each ("MONO · OUT 3", "MONO · OUT 4", own names). On a mono line a source
      has no pan control: IN on Out 3 only = hard left, Out 4 only = hard right, both = centre (the strip says "Out 3", "Out 4", "Out 3 + 4").
      The badge (or ＋ New mix) switches a bus between stereo and two mono lines; up to 5 mixes (two buses × 2 lines + the Aux). Defaults:
      Monitors stereo on Mix 1/2, Cue A / Cue B mono lines on Out 3 / Out 4, **Aux** on the Aux (named "Cue C" until 2026-10-28; R142).
- [x] **Monitor: IN acts on the clicked channel only** (user: the pair turned on together). The logic always did this when the partner is
      used nowhere else (tests); what the user saw is the hardware rule: the FW1814 routes inputs to Mix 1/2 and Mix 3/4 **per pair**
      (register 0x90, FFADO `special_mixer_model.h:163-174`, `special.rs:883-911`) with **one level per input for both** (analog_gains), so
      a partner that is in the other bus at a level comes along. The panel now marks that partner (amber IN, tooltip with the reason) and
      the hint says how to avoid it (link the pair, or take the partner out of the other bus). **Not removable in the panel.**
- [x] **Monitor: stereo link** (user: "the stereo link must make them work together"): right-click an odd input → "Join … as one stereo
      source" (the partner takes the level, Aux send and pan; a stereo mix spreads the pair hard L / R; a mono line puts both on it); "Split
      into two mono sources" undoes it. **Panel-side, per unit**, not the interface's `MAInputLink`: that flag is on for every input pair on
      a real FW1814 (SL rig capture 23, all 18 = 1), so tying the two would make every input a pair by default (mono first is the rule).
- [x] **Monitor: mono DAW returns** (user, 2026-10-28): DAW 1–4 are mono strips by default; right-click joins a pair as one stereo return or
      splits it again (panel-side, independent of `MASWReturnLink`). The FW1814 has **no pan for the DAW returns** (`FWSettingsLevels::Init`,
      Q-ME §30: levels, mute, solo, link and pair routing only; register 0x94 routes per stream pair), so DAW 1 / 3 always land on the left
      (Out 1 / Out 3) and DAW 2 / 4 on the right: IN works on the lines they reach, the others are greyed with the reason (play it on DAW 2
      or 4 in the DAW to reach Out 2 / 4). Pair rule and forced partner as for the inputs; the Aux takes any return on its own.
- [x] **Monitor: IN keeps the level** (user: "it always comes back as -12 dB"): when OFF has to silence a source (the pair stays routed
      for the partner, or the Aux send), the level is kept aside and IN gives it back; a source with no level of its own joins at **0 dB**
      (user, was -12 dB).
- [x] **Monitor: split output cards** (user, 2026-10-28): a bus of two mono lines gets one card per output (Line 3 = Cue A, Line 4 = Cue B)
      with its own level and mute (the pair's output link off; joining back links it and gives the even output the odd one's level); the
      Aux choice stays per output pair (hardware: MAMonitorAuxOnOutput) and the card says so. The phones cards stay stereo (a phone hears
      the odd line left, the even right). Wires go from each line's tile to its own card.
- [x] **Console scene clears the Aux sends** (so no CUE starts lit; the Aux is not used by Console otherwise).
- [x] **Monitor: the A/B button** on the Phones 1 card: "A/B ▾" menu (Phones 1 A / B sources, or direct monitoring; "A and B sources…"
      opens the standard panel's sheet); the card notes "front A/B: A = …, B = …" or "direct monitoring".
- [x] **Hidden channels** (a view option per mode, plan v2 §3): right-click a channel → "Hide … in this mode" / "Show the n hidden
      channels", in DAW, Monitor (inputs), Console, Hybrid (other inputs), Dual DAW and Dual Console (per unit, keys "dual-daw" /
      "dual-console"). The mode bar says how many are hidden. A hidden Monitor input keeps its levels and routing (it only leaves the view).
- [x] **Dual: speakers A/B and unit B's monitor knob** (one knob, value and A 1/2 / B 3/4 switch per unit; the speakers logic is shared
      with the single DAW view); **Dual Console: phones rows** A1, A2, B1, B2.
- [x] **Console: Phones 2** (the phones rows show each one's source: Line 1/2 · check / talkback, Line 3/4 · cue).
- [x] **Hybrid: edit a loop** (Edit on the card: the same sheet, prefilled; the loop keeps its place).
- [ ] **Status menu: mix colours** (plan v1 idea 3) — optional, not built.
- [x] **(user, 2026-10-28)** "Hear return in cue" for Hybrid: **no**.
- [x] **Console CUE (PFL, user 2026-10-28)**: a CUE button on every row that goes through the mixer (inputs; Line 1–4 = DAW 1–4) sends it
      at 0 dB to the Aux and Phones 1 listens to the Aux while any cue is on (its source comes back after the last); the outputs to the
      console do not change (user's choice: Aux to the phones, not Mix 3/4 which feeds Line 3/4). Single and Dual Console.
      Cue in the centre of the phones: **not possible** (user, 2026-10-28): the Aux has no pan, a cued odd channel is on the left only.
- [x] **(R142, user 2026-10-28) The Aux is a stereo pair** (odd channels Aux L, even Aux R, no pan — like the DAW returns), default name
      "Aux"; **M and S in the Aux** act on the Aux sends only (kept, given back), not the channel's mute / solo in every mix. View menu:
      "Advanced Panel…" and "Advanced Dual…" hidden when the front unit / the connected units cannot open them.

## 2. Fix
- [x] **Write failures**: `DeviceModel.commit` keeps a refused batch (`writeFailure`: kern_return_t + keys) until the next scene apply;
      the mode bar shows "⚠ the driver refused n settings (0x…)" in red instead of "✓ applied" (Dual: per unit). Host test with a refusing
      transport. **Not done:** reading the scene back after a switch (the refusal is what the driver reports; a write it accepts but does
      not apply is still not seen).
- [ ] Hybrid loop cards: their SEND / RETURN meters stay thin (fixed card height); widen if wanted.
- [x] **Monitor with the ADAT group**: no fold any more (user, 2026-10-28: the fold left strips with stale IN / greyed states, and every
      fader should be visible): analog, ADAT (or S/PDIF) and the DAW returns are all on screen; the strips fit the width without a scroll
      bar (about 60 px each at full HD, 34 px at the 1280 pt minimum, where names are cut).
- [~] Long texts: the mode bar and the Monitor context bar show their whole text as a tooltip; the Dual header subtitle no longer runs under
      the mode switch. The Dual rate pill at narrow widths is not handled.
- [x] **Demo GUIDs** are plainly fake (`0xD6C04000DE001`…`DE004`).
- [x] Bench aids listed in `tools/README.md` (`OA_LOG_WRITES`, the real-unit `--snapshot --advanced` pass with `printScene`,
      `--open-dual [mode]`, `--open-advanced [mode]`, the demo's peaks, `--fixed-meters`).
- [x] **Per-unit Advanced window vs Dual window** — settled by the one-window rule.
- [ ] Console (single and Dual) and the Monitor strips still use one MeterBar / LinearMeter layer tree per meter (Dual Console 4.5 + 13.6 %).
      Move them to the MeterField approach (a horizontal variant for the Console rows) if real use shows it matters.
- [ ] Snapshot runs wait forever on a locked screen (WindowServer captures); a timeout would be kinder.

## 3. Test
New since 2026-10-28 (hw or real clicks)
- [ ] Custom modes: ＋ new / rename / delete, switching between two modes of the same kind, the status-menu list; the stored file after a
      restart.
- [ ] Hidden channels in each view; the knob chip and its menu (does the front knob follow?); Monitor solo, stereo source, split mix (listen:
      L / R on Line 3 / 4 and in the phones), the A/B menu (front switch behaviour); Dual monitor knob + speakers per unit; Console phones 2;
      Hybrid loop edit; a refused write showing in the mode bar.
Single unit (hw)
- [ ] DAW: monitor knob (drag, scroll, the front knob moves it), speakers A/B both ways, phones source / level / mute, rename, meters vs
      the standard panel's meters.
- [ ] Monitor: IN / OFF with the pair rule (partner silent, the notes), shared level and pan, mono/stereo badge (centring), amber pan
      conflict, mix rename, output source buttons and wires, faders and pans written live.
- [ ] Console: off-unity warning and "Restore 0 dB", label edits, the reference kept per unit, the front knob doing nothing.
- [ ] Hybrid: New loop sheet (mono and stereo), "Set send to 0 dB", Remove, the return-vs-send difference with a real loop.
- [ ] Mode switches on a unit: the louder-outputs question; first opening (mixer kept as Monitor, DAW applied); the lock-out at 88.2+ kHz
      and coming back at 44.1/48; the status-menu items (shown for FW1814 only, greyed outside 44.1/48 kHz) — never seen on screen.
- [ ] **(hw, user) Audible checks** that settle the open hardware questions (plan v2 §6): ~~aux send L/R distribution~~ (user, R142: odd L, even R); pan law at centre;
      recording is pre-mixer; then the Monitor wording/defaults follow.
- [ ] One-window rule with real clicks (status menu, View menu, ⌥⌘A, Dock icon, the standard panel's own button) and resizing the
      Advanced and Dual windows on a Full HD screen — so far only demo snapshots.
Two units (hw)
- [ ] Bus stability after the contact fix: count bus resets over a session (should be 0 without anyone touching cables); capture 108 saw
      ~20 resets at 23:26 on 2026-10-27.
- [ ] Both units streaming at once (DAW on each device), then the meters of both Advanced windows.
Dual (hw, user: two breakout cables)
- [ ] Word clock BNC A → B; "Set B to Word clock"; B locked.
- [ ] "Create aggregate": Audio MIDI Setup shows "Dual FireWire 1814", A then B, A master, 32 in / 24 out (ADAT); drift off on B;
      **unverified**: whether a non-private aggregate made by the API persists across reboots.
- [ ] Recreate / Remove aggregate; Swap A and B (offsets and labels follow).
- [ ] Stream through the aggregate in a DAW (playback and record on both units), listen for drift artefacts.
- [ ] Dual DAW / Console on the pair; a mode switch applies to both units (read back as in capture 108).
Platforms
- [ ] **Modern edition** (macOS 11–15): `make app BAND=modern` on the build PC and run it on the dev Mac (14.8.9) — the Advanced code has
      only been built for Classic.
- [ ] CPU of the Advanced and Dual windows with meters running (GPU-first rule: measure, compare with the standard panel).
- [ ] Window fits the 11-inch MacBook Air (1366 × 768, not measured; the minimum is 1280 × 720 of content, fitted to the screen).

## 4. Docs and release (when the user wants a release)
- [ ] `/close-item` for the Advanced work (not run for M1–M7): system-map entry, `PROVENANCE-RESOLUTIONS.md` (next free R142),
      `PROVENANCE-MATRIX-AUDIOPHILE.xlsx` / Painel, `docs/STATE.md`, `NEXT_STEPS.md`, CLAUDE.md (Advanced window, branch, Dual).
- [ ] Plan v3: mark M1–M5 and M7 built; the decisions taken on the way (unity lock = warn + Restore; Console knob mask empty; Hybrid
      without "hear return").
- [x] **(user)** Merge `panel-redesign` into `main` (2026-10-28, for release 0.5.0).
- [x] Version (2026-10-28, user: release): package 0.5.0, panel 2.0 (driver 0.3.1 unchanged); CHANGELOG, README (Advanced and Dual, what is
      not tested), installer welcome pages, release notes; both editions' `.pkg` / `.dmg`; the Modern panel 2.0 opened in demo on the dev Mac
      (macOS 14.8.9); `sync-to-public.sh --tag v0.5.0 --push`; GitHub release with `gh`.

## 5. Driver follow-ups found on the way (driver, user)
- [ ] Put the unit's GUID in LOCK-DIAG / CMP / HSCI / AVC log lines (with two units the log cannot say which one).
- [ ] Both units report "host plug 0, slot 0" in their CMP traces (channels 0 and 1): check against the reference's multi-unit handling
      whether that is one shared host PCR (memory: check the reference for multi-device handling; R117 history).
- [ ] After the 2026-10-27 reboot 12337F came back with S/PDIF optical in (`0x40000013`) instead of ADAT in: confirm whether the user changed
      it or the helper's replay did.
