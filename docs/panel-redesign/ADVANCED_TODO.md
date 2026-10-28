# Advanced window — to-do (plan: `docs/PANEL_REDESIGN_PLAN_v3.md`)

> **Superseded 2026-10-28 by `ADVANCED_TODO_v2.md`** for everything still open; this file keeps the record of M0–M7.

Status keys: `[x]` done and verified · `[~]` done, not yet verified (says what is missing) · `[ ]` open.
Verification for every panel step: `make -C control-panel typecheck test app` on the build PC (Xcode 9.4.1 / 10.13 SDK) and
`--demo --advanced --snapshot` PNGs. No hardware tests unless the user asks (R123).

## M0 — proposal
- [x] Plans v1–v3, mockups, screenshots; committed `68f6f84` (2026-10-27).

## M1 — window shell (done 2026-10-27)
- [x] M1.1 Pure logic `AdvancedModes.swift`: the four modes (titles, texts, Dual set), the gate (FW1814 profile flag + 44.1/48 kHz),
      scene keys (preset keys without mute/dim), passthrough and unity templates, rising-output check. Host tests in `Tests/LayoutTests.swift`.
- [x] M1.2 `AdvancedStore.swift`: per-GUID file (Application Support/OpenAudio BeBoB MAFW/Advanced/<GUID>.plist): current mode + one
      scene per mode. First open: today's mixer is kept as the Monitor scene, then DAW is applied (outputs never raised).
- [x] M1.3 `AdvancedWindow.swift`: 1280 × 720 window — header (name, mode switcher, rate/lock pill, DIM, MUTE), mode bar (title,
      description, interface-state chip), body (per-mode placeholder until M2–M5; lock-out view outside 44.1/48 kHz with
      "Open the standard panel"). Switching: save the mode left, apply the target, ask first when an output would get louder.
- [~] M1.4 Hooks: one Advanced window per unit (owned by `PanelWindowController`, refreshed with it, meters polled when visible);
      status menu "Advanced Panel…" after "Open Panel" (shown for FW1814 only, greyed "(44.1/48 kHz)"); View menu item ⌥⌘A.
      Built and type-checked; the menu items themselves not yet seen on screen (no menu capture taken).
- [x] M1.5 Demo: `--advanced` with `--snapshot DIR` writes one PNG per mode + the 96 kHz lock-out.
- [x] M1.6 Build PC (10.13.6, Xcode 9.4.1): `make typecheck` clean, `make test` ALL PASS (17 new Advanced checks), `make app` built;
      `--demo --advanced --snapshot` PNGs in `m1-snapshots-10.13/` (first opening in DAW, the four modes, the louder-outputs question,
      the 96 kHz lock-out); standard panel snapshots unchanged (8 PNGs, mixer checked by eye).

## M2 — DAW mode (single unit) (done 2026-10-27)
- [x] Input meter bridge (16 ADAT / 10 S/PDIF, names, jack → Core Audio channel, peak hold, clip latch), output bridge (Line, ADAT or
      S/PDIF out, phones). Channel map `Models/FW1814/FW1814AdvancedChannels.swift` + 5 host tests (Core Audio order, peak slots).
- [~] Control room: monitor knob (Line 1/2, linked; the DAW scene points the front knob at Line 1/2), speakers A/B, phones 1/2 with
      DAW 1/2 or DAW 3/4 and level/mute; rename by double-click (names per GUID). Drawn and wired; the interactions (knob drag, B,
      rename, phones writes) are not exercised by the snapshots and not run on a unit.
- [x] Build PC: typecheck clean, tests ALL PASS, app built; snapshots `m2-snapshots-10.13/` (ADAT and S/PDIF). A window opened inside the
      snapshot run is not reported visible yet, so the snapshot path sets `ignoresOcclusion` (bench only).

## M3 — Console mode (single unit) (done 2026-10-27)
- [x] Channel tables (Console → DAW, DAW → console) with editable labels (the same per-unit names as DAW mode), jack, Core Audio
      channel, linear dBFS meter (−60…0, layer-backed) with the 0 VU mark, peak hold, clip latch; reference −20/−18/−14/−12 dBFS per unit.
- [x] Unity lock as a watch (user decision pending → default chosen 2026-10-27: warn + "Restore 0 dB", never rewrites by itself):
      `AdvancedScene.offUnity` (Line 1–4 and DAW returns 1–4, levels and mutes) + host tests.
- [~] Console scene points the front knob at nothing (empty MARotaryControlV2 mask) — **unverified** that the kext accepts it.
- [x] Clock chip (warns when ADAT in runs on the internal clock), optical-port chip, phones 1 row.
- [x] Build PC: typecheck, tests ALL PASS, app; snapshots `m3-snapshots-10.13/` (ADAT, S/PDIF). Not run on a unit; the off-unity
      warning and the label editing are not in the snapshots.

## M4 — Monitor mode (done 2026-10-27)
- [x] Allocation logic `AdvancedMonitor.swift` (MonitorMixer): three cue mixes on Mix 1/2, Mix 3/4, Aux; membership from pair routing +
      level (or the aux send); the pair rule (partner kept silent, or a note when it must join/leave); shared level and pan of Mix 1/2 and
      3/4; mono centring that leaves sources a stereo mix pans; Aux sends per channel (DAW returns = elements 19–22). 14 host tests.
- [x] View `AdvancedMonitorView.swift`: mono input strips (IN, fader, meter, pan per the user's rule: greyed at centre in a mono mix,
      live in a stereo one, amber on a shared-pan conflict, none on the Aux), DAW return strips, ADAT fold, cue-mix tiles (select, rename by
      double-click, MONO/STEREO badge), output cards (Line 1/2, Line 3/4, phones: source, level, mute) and wires. Mix names / mono per GUID.
- [~] Snapshots `m4-snapshots-10.13/` (ADAT, S/PDIF: Cue A mono selected). Not in the snapshots: a stereo mix with live pans, the amber
      conflict, the notes of the pair rule, any write; not run on a unit. The Swift 4.1 type checker needed one expression split (PanBar).
- [ ] Split a stereo mix into two mono mixes (the mockup's "Split Mix 3/4…") — not built; the tiles show three mixes only.

## M5 — Hybrid mode (done 2026-10-27)
- [x] Loops per GUID (`HybridLoop`: name, device, send outputs, return inputs; mono or stereo) + 4 host tests; "New loop" sheet (free
      sends exclude Line 1/2 and used outputs), Remove.
- [x] Loop card: path chips, send and return meters with held peaks, return-vs-send difference (green within 1 dB), Line-out send unity
      check with "Set send to 0 dB" (ADAT / S/PDIF sends are direct), other inputs bridge, monitor knob, free sends and latency note.
- [~] Build PC: typecheck, tests ALL PASS, app; snapshots `m5-snapshots-10.13/` with three demo loops. Not run on a unit; the New loop
      sheet and the unity button are not in the snapshots. "Hear return in cue" (mockup) not built: the passthrough monitors no input.

## M6 — custom modes
- [ ] ＋ duplicates a mode (name, scene, labels, view options); status menu lists modes.

## M7 — Dual (two FW1814s as one aggregate; DAW and Console only) (built 2026-10-27, not yet run on the pair)
- [x] `AudioHardwareCreateAggregateDevice` / `Destroy` and the uid / name / subdevices / master / private / drift keys are in the 10.13 SDK
      (AudioHardware.h, 10.9+), checked on the build PC.
- [x] Pure logic `AdvancedDual.swift` (gate: same rate, 44.1/48; checklist: rate, A's clock, B on word clock and locked, aggregate A then B
      with A master and no drift on B, buses; aggregate description; aggregate channel offsets) + 6 host tests.
- [x] `DualAggregate.swift`: find / read / create / remove the aggregate, each unit's HAL UID, the FireWire bus of each unit.
- [x] `AdvancedDualWindow.swift`: Dual window (menu bar "Advanced Dual (A + B)…" with two FW1814s; View menu), DAW Dual (four meter bridges
      with aggregate channel numbers, unit A's monitor knob, four phones), Console Dual (three tables, unity watch over both, reference,
      clock chip), mode switch applying the scene to both units with one question, Dual setup sheet (Set B to Word clock, Create /
      Recreate / Remove aggregate, Swap A and B). Unit A = lower GUID unless swapped.
- [x] Build PC: typecheck, tests ALL PASS, app; demo snapshots `m7-snapshots-10.13/` (`--demo --devices 2 --dual`).
- [ ] On the pair: word clock BNC from A to B, Set B to Word clock, Create aggregate, check the aggregate's composition and channel counts in
      Audio MIDI Setup, stream through it. Needs the user (cable, and both actions change the system).
