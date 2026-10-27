# Capture 105 — FW410 at 192 kHz: every analog output streams (SL rig, official v1.9, 2026-10-26)

Question (user, 2026-10-26): does the 410 play audio on outputs other than 1/2 at 192 kHz? The early manual (`docs/fw410_manual.pdf` p.4) says "192 kHz playback on outputs 1 and 2"; the v1.11.2 kext's 192 kHz row has 8 outputs with a remap table (Q-410-0 §2).
Method: `openaudio-fw1814/tools/chtone` (new: a −40 dBFS 1 kHz sine on ONE device channel from a CoreAudio IOProc; built on the build PC for 10.6) on each channel of "FireWire 410 Multichannel", at 96 kHz (channels 1-10) and at 192 kHz (channels 1-8); the hardware output peaks are read from the status packet `0xFFC7:0x00600000` quadlets 4-17 with `slfwread quad` while the tone plays. Rate changed with the panel arrows with the engine idle; back to 96 kHz at the end. First attempt with `afplay` and 8-channel WAVs was invalid (afplay folds a multichannel file to channels 1/2). Script `sl-fw410-192-outputs.sh`, raw `peaks.txt`.

| Rate | Channel played → meter that moved |
|---|---|
| 96 kHz | 1→q4, 2→q5, 3→q6, 4→q7, 5→q8, 6→q9, 7→q10, 8→q11, 9→q12, 10→q13 (one-to-one, ≈0x0140) |
| 192 kHz | 1→q4, 2→q5, 3→q6, 4→q7, 5→q8, 6→q9, 7→q10, 8→q11 (one-to-one, ≈0x0125-0x0144) |

Result: at 192 kHz **all 8 analog outputs carry audio**, each on its own output (the device's own output meters); the status-packet meters also work at 192 kHz (only the front LED meter goes dark, manual p.6). Status quadlets 4-13 = outputs 1-10 (S/PDIF = 9/10). The manual's "outputs 1 and 2" is out of date for the 1.9/1.11.2 driver + 2007 firmware. Not checked by ear (meters only). Inputs at 192 kHz (2 channels) not tested here.
Comparison with the FW1814: its 176.4/192 kHz row (`0x4ab80`) is 2 in / 4 out with no remap tables (analog in 1/2, out 1/2 + 3/4 only; CLAUDE.md pitfall "2 in / 4 out analog only"); the 410's row (`0x4ad40`) is 2 in / 8 out with remap tables (`0x2f9e0` input identity {0,1}, `0x2f9f0` output {0,4,1,5,2,6,3,7}).
