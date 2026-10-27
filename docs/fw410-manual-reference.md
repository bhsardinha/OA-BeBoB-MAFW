# FireWire 410 — Manual Reference (developer-oriented, Mac only)

Companion to `docs/manual-reference.md` (FW1814) and `docs/audiophile-manual-reference.md`. Sources:

* **Manual**: `docs/fw410_manual.pdf` (32 pp., pages 30-32 blank; PDF title `FireWire410_EN_Mnl_100703C-rev.qxd`): an **early revision** (Mac OS 9.2 / OS X 10.1.5, Windows 2000/XP, panel with an ASIO/WDM buffer field and a "Unit 1" picker). Transcribed in `docs/fw410-manual.md` (private). Cited as `man p.N`.
* **Driver**: v1.11.2 kext + panel (`_extracted/`; `A1-ghidra/Q-410-0-reference-map.md`, `A2-panel/PN410-control-key-map.md`).
* **Live**: SL rig v1.9 / panel 1.4, captures 102 and 103 (`B5-sl-dtrace-truth/captures/`).

Rule (as for the other models): the manual describes what the user sees; the driver (`_extracted/`) is the spec; where the two disagree the driver wins and the disagreement is listed in §7. Nothing here is verified on our clone yet.

## 1. Identity and enumeration
* Product "FireWire 410 Mobile Recording interface" (man p.2). Registry: `FireWire Product Name = "FW 410"`, vendor name `M-AUDIO`, device `Vendor_ID 0x7F5`, unit `Vendor_ID 0xD6C`, `Model_ID 0x10046`, unit SW version 0x14001, spec 0xA02D, 400 Mb/s (capture 102). Bootloader product name `FW Bootloader` (kext `kDeviceSpecs`).
* The unit boots as a bootloader and the driver starts the firmware (man p.9-10 describes Windows installing "FireWire 410 Bootloader files" then the driver — same two-stage enumeration as the other models).
* Powered off but cabled, the blue LED flashes ("low-power" mode, the FireWire repeater stays alive) (man p.7).
* Two FireWire ports (daisy chain) (man p.7).
* Never hot-plug (man p.3, p.29) — the project rule already.

## 2. I/O inventory (man p.3-7)
| Port | Count | Software-visible | Notes |
|---|---|---|---|
| Mic/inst inputs (XLR/TS combo, front) | 2 | analog in 1/2 | gain pots, 20 dB pad, phantom (both), signal/clip LEDs — **hardware only**, no software control |
| Line inputs (TS, rear) | 2 | same channels | selected by the front mic/line switch, never both |
| S/PDIF in (coax RCA, TOSLink) | 1 active | S/PDIF in L/R | **selected in the panel** (coax default); the in-LED flashes when a valid signal is on the *other* input (man p.7, p.26) |
| Line outputs 1-8 (TS, rear) | 8 | outputs 1-8 | up to 7.1 surround from the host |
| S/PDIF out (coax + TOSLink) | 1 pair | output 9/10 | **always on both connectors**; AC-3/DTS pass-through (man p.3) |
| Headphones (front) | 2 jacks | one phones bus | same signal on both, **analog level pots** per jack (man p.7, p.21) |
| MIDI in/out (DIN) | 1/1 | 1 port, 16 channels | front MIDI Thru switch = hardware bypass, works with the unit off (man p.7, p.17) |
| Level controller | 1 encoder | software | assignable bus (§4) |
| Output meter (front, 2 × 4 LEDs) | 1 | "outputs as selected in the control panel"; **dark at 192 kHz** (man p.6) | source of the selection: §8 item 3 |

## 3. Streams and rates
* Manual (man p.3-4): 4 in / 10 out to the computer; rates "from 32 kHz up to 192 kHz"; recording up to 96 kHz; **192 kHz playback on outputs 1 and 2 only**.
* Driver v1.11.2 (Q-410-0 §2) and live (capture 103): 44.1-192 kHz (**no 32 kHz**); 4 / 10 at 44.1-96 kHz; **2 / 8 at 176.4 and 192 kHz** (inputs too). See §7 items 1-2.
* Wire (output, ≤96 kHz): 11 quadlets per frame, MIDI in the last one; host channels 9/10 (S/PDIF) go to wire slots 0 and 5 (`_RearrangeFrameLLRR10WithMIDI`). AC-3 indices 0/5 (`IndexForAC3OutputChannels`) = exactly those S/PDIF slots — consistent with "AC-3/DTS on S/PDIF out" (man p.3).
* Input: 5 quadlets per frame (4 audio + MIDI), `_ConvertLLRR4SkipMIDI1`.
* Latencies / offsets / bytes per cycle per rate: capture 103 README.

## 4. Control plane (what the panel offers; man p.18-21 vs capture 103)
* **Mixer** (man p.19): 5 SW-return pairs (1/2, 3/4, 5/6, 7/8, spdif) + analog in + spdif in; per pair faders, link, mute, solo (multiple solos allowed), 2 aux sends, output routing to **any or all** of 1/2 · 3/4 · 5/6 · 7/8 · spd (summed; the manual warns about clipping); inputs also have pan. Keys: `MASWReturn*`, `MAInput*`, `MAAuxSendLevels`, `MASWReturnRoutingV2` / `MAInputRoutingV2` (bitmasks).
* **Output** (man p.20-21): 5 output pairs with faders, bal, link, mute, solo, **MAIN/AUX** (aux = the output carries the aux bus: `MAMonitorAuxOnOutput`); aux master (faders, link, pan, bal, mute); phones (faders, link, bal, mute, **monitor assigns: any or all of the five output buses**, MAIN/AUX = phones carries the aux bus).
* **Hardware** (man p.21): sample rate detected; sync internal / external (S/PDIF); S/PDIF input coax / optical; firmware info. The ASIO/WDM buffer field is Windows-only (out of scope). Panel 1.4 adds the stereo-pairs checkbox (capture 103).
* **Global** (man p.18): settings load / save / reset / delete; **level controller** assignment sw rtn / output / input / phones / aux send, with per-channel **CTRL** buttons to remove single faders from the group, remembered per group (man p.18, p.25) = `MARotaryControlV2`; main output **mute** and **dim (−20 dB)** = `MAGlobalMute` / `MAGlobalDim`; a "Unit" picker for future multi-unit support (man p.18).
* **Level controller targets** (man p.6-7, p.18): SW return (the 10 returns), output (8 analog + 2 digital), input (2 analog + 2 digital), phones (the bus; the jacks keep their own pots), aux send (the aux master).

## 5. Factory defaults (man p.28) — to check against `ResetToFactorySettings`
1. mixer faders 0 dB; 2. **output faders −6 dB**; 3. SW return n/n+1 → output n/n+1, spdif rtn → spdif, **phones → output 1/2**; 4. aux sends −∞; 5. analog in / spdif in: **no routing**; 6. pans hard left / right; 7. balance centre; 8. nothing muted, soloed or **linked**; 9. level controller assigned to every channel of each group; 10. level controller = output bus; 11. every output on MAIN.
Driver side known so far: phones source factory = 1 (out 1/2) (`ResetToFactorySettings`, Q-410-0 §5). The rest comes from the generic settings objects' factory values — to be compared item by item (§8 item 5). Capture 103's starting state (links on, outputs 0 dB) is the rig's saved state, not the factory one.

## 6. Usage patterns the manual documents (what our panel must make easy)
* Direct monitoring through the mixer inputs with zero latency (man p.22-23).
* **Effects while direct monitoring** (man p.24): S/PDIF out set to AUX → outboard effect → S/PDIF in routed to 1/2; mirror the settings on a SW return for playback.
* **Aux as a separate cue mix** (man p.24-25): 7/8 or S/PDIF out on AUX → headphone amp.
* Level controller on the inputs with one strip's CTRL off (man p.25).
* Surround up to 7.1 on outputs 1-8 (man p.7, p.16).

## 7. Where the manual and the driver disagree (driver wins; checks listed)
1. **32 kHz**: manual yes (man p.4); v1.11.2 rate table `0x2fa10` and the live sweep: no. Not offered.
2. **192 kHz**: manual "playback on outputs 1 and 2"; driver 2 in / 8 out at 176.4/192 (table `0x4ad40`, CoreAudio, capture 103). **Resolved for the outputs (capture 105, R136):** at 192 kHz all 8 analog outputs carry audio (one device meter per channel); the driver packs them through the 192 kHz remap table (Q-410-0 §2). The 2 inputs at 192 kHz are not tested yet.
3. **Buffer size** (ASIO/WDM field): Windows only, as for the other models.
4. **Firmware info on the hardware page**: panel 1.4 shows it on the about tab.
5. **Multiple units**: "presently only a single FireWire 410" (man p.18); the v1.11.2 driver is multi-device by design (per-GUID nubs); our clone too (on demand).

## 8. Unverified / open items (with the check that settles each)
1. Sixth phones source (`SetHeadphoneSource` bit 5, Q-410-0 §5): the manual lists five buses plus MAIN/AUX; hypothesis: bit 5 is the aux bus, i.e. the phones MAIN/AUX button lands in the same register word. Check: trace what `MAMonitorAuxOnOutput[5]` calls in the kext (desk); then the rig.
2. S/PDIF "signal on the other input" (in-LED flashing, man p.7): is it in the status packet? Check the bits of status quadlet 18 beyond lock (bit 24) and FDF (bits 16-18) with `slfwread` while a source is on the unselected input (needs an S/PDIF source on the rig).
3. Front output-meter source ("as selected in the control panel", man p.6): no panel key seen in capture 103; check whether it follows the level-controller assignment or the output pair 1/2 (rig, by eye) and whether a register write exists (decomp).
4. 192 kHz: outputs resolved (§7 item 2); inputs at 192 kHz: record a tone on the rig (loop a line out into a line in) — open.
5. Factory defaults: §5 vs `ResetToFactorySettings` + generic settings factory values (desk), then a factory reset on the rig with restore (as capture 92 did for the Audiophile).
6. MIDI 16 channels / thru: hardware; nothing for the driver beyond the 1×1 port.

## 9. Source index
Manual pages: identity p.2-3, features p.4, controls p.6-7, connections p.16-17, panel p.18-21, usage p.22-25, troubleshooting p.26, defaults p.28. Driver: `A1-ghidra/Q-410-0-reference-map.md`, `A2-panel/PN410-control-key-map.md`, `A2-panel/ghidra-exports/decomp/cfw410-2026-10-26.txt`. Live: captures 102, 103. Plan: `docs/BUILD_FW410_PLAN_v1.md`.
