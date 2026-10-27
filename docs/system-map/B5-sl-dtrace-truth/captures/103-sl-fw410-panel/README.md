# Capture 103 — FireWire 410 with the official panel: screens, stereo pairs, every control, rate sweep (SL rig, 2026-10-26)

Rig `fw1814-sl` (Mac OS X 10.6.8 10K549), official driver **1.9** (build 01.09.003), panel **1.4**, FireWire 410 alone on the bus, GUID `0xD6C010006B8C0`, bootloader 2003-04-04 13:46:25, firmware 2007-05-04 10:26:56, hardware "model 2 version 1" (`about.png`). Found at **96 kHz**, internal clock, coax S/PDIF input, stereo pairs off, engine idle (`IOAudioEngineState = 0`), no audio client. Baseline registry: capture 102.

Tools (`openaudio-mafw-bebob/tools/`, copies in `fw1814-sl:~/fw1814-captures/tools/`): `slpanel.py` (clicks/drags/window shots in the console session), `slsetprops.d` (DTrace `is_io_registry_entry_set_properties`, Panel only), `slhal.py` (CoreAudio device list), `slprops.py` (restore; **2026-10-26: now finds the FW410/1814 entry too**), new `sl-fw410-controls.seq` (250 actions), `sl-fw410-rates.sh`, `sldecode.py` (pairs writes with actions; truncated records decoded up to the 768-byte `tracemem` limit and marked `...`).

| File / folder | Content |
|---|---|
| `start.png`, `mixer.png`, `output.png`, `about.png` | the tabs as found (`mini.png` / `full.png`: the view-switch clicks missed, both show the about tab) |
| `pairs-*` | stereo-pair checkbox on → off: `MAEnableStereoPairEngines` 1 / 0 (`pairs-setprops.txt`), CoreAudio list before / on / off (`pairs-hal-*.txt`), registry with the pair engines (`pairs-ioreg-on.txt`), shots |
| `controls/` | every mixer / output / hardware button clicked twice, knob and fader drags, level-controller radios, mute/dim: `actions.txt`, `setprops.txt` (263 writes), `decoded.txt`, start/end shots per tab, `h-ext.png` (external sync), `h-opt.png` (optical) |
| `rates/` | 96 → 176.4 → 192 → 96 → 88.2 → 48 → 44.1 → 96 via the hardware-tab arrows (one step too many at the end: 176.4, then back to 96): `ioreg-<rate>.txt`, `hal-<rate>.txt`, `hw-<rate>.png`, `setprops-rates.txt` |
| `settings-backup.plist` | the per-GUID settings file before the session; restore source |
| `fix*`, `restored-*.png` | the restore and two pan-knob corrections (see below) |

## Results

**Stereo pairs** (checkbox "show 410's stereo pairs as devices", hardware tab, "core audio device settings"): on adds 5 CoreAudio devices next to "FireWire 410 Multichannel" (which stays): `FireWire 410 Analog 1/2` (2 in / 2 out), `FireWire 410 S/PDIF 1/2` (2 in / 2 out), `FireWire 410 Analog 3/4 Out`, `… 5/6 Out`, `… 7/8 Out` (0 in / 2 out); classes `m_audio_b_FWPairEngine` + `m_audio_b_FWPairStream` under the device; UIDs `m_audio_b_FWPairEngine:1`…`:5`, 8192 frames, 96 kHz like the master. Off removes all five.

**Engine UID:** `com_m_audio_FWEngine1:0xD6C010006B8C0` (multichannel engine is class `com_m_audio_FWEngine2`) = the `[+0x34a]` flag of `SetupAudioEngine` (Q-410-0 §2).

**Per rate** (CoreAudio channels; registry):

| Rate | in / out | MABytesPerCycle | In latency | Sample offset | Out latency |
|---|---|---|---|---|---|
| 44.1 kHz | 4 / 10 | 528 | 32 | 40 | 226 |
| 48 kHz | 4 / 10 | 528 | 32 | 48 | 210 |
| 88.2 kHz | 4 / 10 | 1040 | 64 | 88 | 457 |
| 96 kHz | 4 / 10 | 1040 | 64 | 96 | 385 |
| 176.4 kHz | **2 / 8** | 1552 | 128 | 176 | 397 |
| 192 kHz | **2 / 8** | 1552 | 128 | 192 | 397 |

The panel's hardware tab keeps the S/PDIF input choice and the sync choice at 192 kHz (`rates/hw-192000.png`).

**Control → key** (decoded, `controls/decoded.txt`; v1.9 panel): see `A2-panel/PN410-control-key-map.md`. Highlights: `MAClockSource` internal+coax `0x40000004`, internal+optical `0x40000000`, external (S/PDIF) `0x40000005`; `MASWReturnRoutingV2` = 5 bitmasks (bit 0 out 1/2 … bit 3 out 7/8, bit 4 S/PDIF; several bits allowed), factory-looking `[1,2,4,8,16]`; `MAInputRoutingV2` = 2 bitmasks (analog in, S/PDIF in), same bits, `[0,0]` here; `MAHeadphoneSourceV2` = bitmask, multi-select (1/2 + S/PDIF = 17); `MAMonitorAuxOnOutput` 6 entries (5 output pairs + phones "main"); `MARotaryControlV2` 64-bit group mask (level-controller radios and the output `ctrl` buttons); the mixer strips' `ctrl` buttons are greyed with the level controller on "output bus" and wrote nothing.

**Mixer register window:** `0xFFC7:0x00700000…` answers reads with `0xE0008017` (address error); the status window `0x00600000` reads normally (`slfwread quad`). The wire side of the mixer is therefore taken from the kext decompiles, not from the rig.

## Rig left as found
96 kHz, internal clock, coax, stereo pairs off, panel on the mixer tab. Live state: `slprops.py restore settings-backup.plist D6C010006B8C0` (31 keys, every status 0); two pan knobs (analog in L, aux L) still showed L118 afterwards and were dragged back to L127 (`fix*`, `MAAuxOutputPan2 [-255,255]`, mixer shot `fix2-mixer.png` = `start.png`); settings file copied back from the backup, byte-identical 60 s later. `pmset noidle` was started on the rig to keep it awake (the rig dropped off the network once during the session).
