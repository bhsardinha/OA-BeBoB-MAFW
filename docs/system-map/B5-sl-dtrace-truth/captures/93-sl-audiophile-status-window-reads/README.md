# Capture 93 — Audiophile status window read from user space (SL rig, 2026-10-19)

Rig `fw1814-sl` (10.6.8, official driver 1.9 running), Audiophile GUID `D6C0300211D54`, no audio client. Tool `openaudio-mafw-bebob/tools/slfwread.c` (IOFireWireLib, read-only; built on the build PC with the 10.13 SDK, `-mmacosx-version-min=10.6`, sha256 `9b0ec0fbcd0f9bc2…`): opens the `IOFireWireUnit` node (`Open` → `0x0`, coexists with the official driver) and block-reads 60 bytes at `0xFFC7:0x00600000`. Rate and clock were changed with the official panel (hardware tab, driven by `slpanel.py`; panel brought to the front first — an earlier attempt with the window inactive changed nothing and is not used).

`status-reads.txt`: two reads per state.

| State | `MASampleRate` / `MARateLocked` | status word bytes w0 w1 w2 w3 |
|---|---|---|
| 44.1 kHz internal | 44100 / 1 | `00 00 01 01` |
| 48 kHz internal | 48000 / 1 | `00 00 02 01` |
| 88.2 kHz internal | 88200 / 1 | `00 00 03 01` |
| 96 kHz internal | 96000 / 1 | `00 00 04 01` |
| 96 kHz external (no S/PDIF source) | 96000 / 0 | `00 00 ff 00` |
| 48 kHz external (no S/PDIF source) | 48000 / 0 | `00 00 ff 00` |

Result: **w2 = FDF index** (1 = 44.1, 2 = 48, 3 = 88.2, 4 = 96; `0xFF` when nothing is locked), **w3 = lock** (1/0), matching the kext's `DetectExternalSyncChanges(w3 ≠ 0, w2)` and the driver's `MARateLocked`. Peaks were 0-1 (no signal). Rig left at 48 kHz internal, engine idle, panel on the mixer tab (`restore-mixer.png`).

Not reachable here: `EXTENDED PLUG INFO` (channel cluster order). The official driver's AV/C unit offers only the generic FireWire user client, so a user tool can write an FCP command but cannot receive the response. `FWAP.bcd`'s attribute namespace has no `[ClusterInfo]` (unlike the 1814's); its BCO directory confirms the status window address (`DataAddressHigh 0xffffc7`, `DataAddressLow 0x600000`).
