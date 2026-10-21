# Capture 92 — Audiophile controls one by one, rate sweep, factory reset (SL rig, 2026-10-19)

Rig `fw1814-sl`, official driver 1.9 / panel 1.4, Audiophile GUID `D6C0300211D54`; no audio client running (user quit Vox). All actions automated over SSH with `openaudio-mafw-bebob/tools/`: `slpanel.py` (clicks/drags/window shots, run in the console session), `slsetprops.d` (DTrace `fbt:mach_kernel:is_io_registry_entry_set_properties:entry`, Panel only, wall-clock ms), `slprops.py` (restore), action lists `sl-audiophile-controls.seq`, `sl-audiophile-redo.seq`, `sl-rate-*.seq`, `sl-audiophile-rates.sh`, `sl-reset*.seq`, `sl-restore-shots.seq`.

| Folder | Content |
|---|---|
| `backup/` | the rig's per-GUID settings file before the session (used for the restore) |
| `controls/` | `actions.txt` + `setprops.txt` (139 writes): every button toggled on/off on the mixer/output/hardware tabs, Level-Controller radios, mute/dim, knob/fader drags; `*-redo.*` (12 writes): the SW-return fader and input pan drags redone after the first attempt missed (user report: the first fader drag hit between the two faders); start/end shots per tab |
| `rates/` | 48 → 44.1 → 48 → 88.2 → 96 → 88.2 → 48 kHz via the hardware-tab arrows: `ioreg-<rate>.txt`, `hw-<rate>.png`, `setprops-rates.txt` (the panel writes `IOAudioStreamFormat`, not an MA key) |
| `reset/` | factory reset: confirmation sheet, `setprops-reset.txt` (`MAMetaResetToFactorySettings = 1`), settings file after reset, tab shots, then the restore (`device-settings-after-restore.plist`, `restore-*.png`) |
| `decoded-control-key-map.txt` | every write paired with the last action before it (decoder: 32-bit ints signed, 64-bit hex, IDREF expanded) |

Registry per rate (v1.9): output latency 226 / 210 / 457 / 385, input 32 / 32 / 64 / 64, sample offset 40 / 48 / 88 / 96, `MABytesPerCycle` 400 / 400 / 784 / 784 at 44.1 / 48 / 88.2 / 96 kHz; streams 6 out / 4 in at every rate.

Rig left as found: live state restored with `slprops.py restore` (all writes status 0; output tab matches the original screen), settings file copied back from `backup/` (byte-identical, unchanged 60 s later), 48 kHz internal clock, engine idle, panel on the mixer tab.
