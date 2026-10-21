# Capture 91 — official panel screens with the FireWire Audiophile (SL rig, 2026-10-19)

Rig `fw1814-sl` (Mac OS X 10.6.8 10K549), official M-Audio FireWire driver **1.9** (build 01.09.003), panel **1.4**, FireWire Audiophile GUID `0xD6C0300211D54`, bootloader 2003-10-06 10:00:41, firmware 2007-05-04 10:22:12, hardware "model 13 version 1" (see `04-about.png`). Engine at 48 kHz, internal clock; a client (Vox) was playing during the capture — **only tabs and the view switch were clicked, no control or rate was changed**.

Method: the panel (PID 169, owner "M-Audio Firewire") was driven over SSH with `openaudio-mafw-bebob/tools/slpanel.py` run inside the console session (`sudo launchctl bsexec 169 sudo -u <user> python slpanel.py …`): `click X Y` posts Quartz mouse events, `shot FILE` grabs only the panel window with `CGWindowListCreateImage` (10.6 `screencapture` has no `-l`). Full window = 694×489 at (608, 409); tab row at window y = 66 (mixer x 37, output 92, hardware 148, about 205); view switch at window (620/642/662, 60).

| File | Content |
|---|---|
| `00-desktop.png` | full screen (1920×1080) before any click: panel on the mixer tab |
| `01-mixer.png` | mixer tab: 1/2 sw rtn, 3/4 sw rtn, spdif sw rtn (aux L/R knobs, faders, ctrl/solo/link/mute, out 1/2·3/4·spd), analog in and spdif in (aux, faders, pan L127/R127, ctrl/solo/link/mute, out), output bus / aux bus meters; right column: settings (load/save/reset/delete), level controller (sw rtn bus / output bus / input bus / aux send), main output (mute/dim) |
| `02-output.png` | output tab: 1/2 out, 3/4 out, spdif out (bal, faders, ctrl/solo/link/mute, main↔aux button), aux (faders, ctrl/link/mute, pan), phones (bal, faders, link/mute, mon 1/2·3/4·spd·aux), software return bus / input bus meters |
| `03-hardware.png` | hardware tab: sample rate (48000 Hz, lock icon), sync source internal/external, headphone source setting (A/B rows × 1/2 out, 3/4 out, spdif out, aux), a/b switch settings (direct monitoring / headphone source A/B, with the LED legend) |
| `04-about.png` | about tab: versions, bootloader/firmware dates, hardware model, GUID |
| `05-view-mini-meters-horizontal.png` | view switch icon 1: mini meters 322×228 (input 1 2 spd / sw rtn 1 2 3 4 spd / output 1 2 3 4 spd) |
| `06-view-mini-meters-vertical.png` | view switch icon 2: vertical mini meters 159×470 (input, sw rtn, output; rows 1 2 3 4 sL sR) |
| `07-mixer-restored.png` | full view restored (icon 3), back on the mixer tab — rig left as found |

Use: **reference for which controls exist only** — the clone's Audiophile screens follow the layout of our new 1814 panel (user decision 2026-10-19).

Bundle note: these are **v1.9 / panel 1.4** screens (version-skew rule); the spec panel is v1.11.2's (`A2-panel/PNAP-control-key-map.md`).
