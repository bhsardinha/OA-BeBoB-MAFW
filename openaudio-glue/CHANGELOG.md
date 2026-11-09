# Changelog — OpenAudio Glue

## 1.0.0 — 2026-11-09 (published with OpenAudio BeBoB MAFW v0.6.0)
Disk images `OpenAudio-Glue-1.0.0-classic.dmg` (macOS 10.13 to 10.15) and `-modern.dmg` (macOS 11 to 15, Intel). FireWire 1814 only. Needs the main package 0.6.0 (driver 0.4.0). The Modern edition has not been run. History of the development:
- 2026-10-28: project started on branch `glue`; plan v1 approved (`docs/GLUE_PLAN_v1.md`).
- 2026-10-28: phase 0 (G1–G6); phase 1: DSP chain in `dsp/` (Airwindows Interstage/Tube2/Channel9/ToTape9 mono ports, verified equal to the originals; RBJ 4-band EQ with proportional Q; threshold-only compressor; Off/Soft/Medium/High/Full presets with level matching), host tests, `tools/glue-bench`, `tools/port-diff`.
- 2026-10-28: Desk options are now Off + Neve/API/SSL × Medium/High (user); `glue-listen` (plays a file through the chain, live keys; threshold `,` `.`, volume `'` `¡` for a Spanish keyboard).
- 2026-10-28: tape presets v2 (user): brighter (+0.5 dB at 10 kHz), 30 ips head bump (~94 Hz), one step more drive per level, less flutter.
- 2026-10-28: glue-listen EQ curves v2 (user): smile/low-mid cut/presence about twice as strong; new low-end cut (−5 dB at 100 Hz shelf and 250 Hz), scoop, punch, air.
- 2026-10-28: compressor Source per channel (user): General, Drums (transient-keeping: slow attack + hit lift), Vocals, Heavy guitar, Bass, Synth voicings; `glue-listen` key `s`.
- 2026-10-28: per-channel on/off (user): off = bit-exact pass-through with no DSP cost, even with the colour stages on.
- 2026-10-28: phase 2 kext tap (R143) and phase 3 driver (AudioServerPlugIn); delay compensation (user): one fixed latency per rate for every channel; THD table of the strongest presets (`tools/glue-thd`).
- 2026-10-28: phase 4, the OpenAudio Glue app (standalone, OA MAFW panel style; matrix of 6 channel cards per row; demo + snapshots).
- 2026-10-28: Names menu (band 16 / Dual 32 templates, also sources, on/off and links), right-click rename; stereo link per pair, linked compression in the DSP and driver (user).
- 2026-10-28: rename fixed (selectText alone; makeFirstResponder first started an edit that selectText ended at once); the name lets clicks through to the card.
- 2026-10-28: phase 5, Glue's own package (`installer/`); main test package 0.6.0-glue1 with driver 0.4.0 (user's number).
- 2026-10-28: first hardware run (G11): fixed a coreaudiod crash (the ring was remapped and the live mapping unmapped); kext `ringGeneration`; bypass bit-identical on both units; tools glue-tapread, glue-bypasscheck.
- 2026-10-28: Colour v2 (user): the stages re-voiced to be heard at tracking level (glue-voice); global Drive 0…+18 dB (DSP, driver, app); colour-path driver test.
- 2026-10-28: app icon from docs/images/glue_logo.jpg (transparent background, panel/Resources/make-icon.py).
- 2026-11-02: the user reports the tests on the High Sierra iMac pass, everything works as it should (G12); README, STATE and ARCHITECTURE updated (the app is standalone; the main panel only gets a menu item).
- 2026-11-06: release candidate: FireWire 1814 only (the plug-in matched every model); every bundle stamped with the version; the Airwindows MIT notice in the app and the plug-in; Classic and Modern packages with their macOS ranges; a disk image with an Uninstall app; the main panel (2.2) has a menu item that opens the app.
- 2026-11-09: published. The main driver's bus-reset fix (main R144) is in package 0.6.0; Glue itself is unchanged since 2026-11-06. New tool `tools/glue-probe/glue_settings.c` (prints a Glue device's settings, sets only its bypass).
