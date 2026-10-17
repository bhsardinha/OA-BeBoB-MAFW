# Build-PC run 2026-10-16 — first integrated hardware run (kext + panel + FW1814)

> **Update (2026-10-17, second session)** — see the "Second session" section at the end of this file. Corrections to the table below: **F4 was reverted** (the reference leaves the phones silent until a source is chosen; the level starts at 0 dB); **F10 was incomplete** (the synchronous `cancel()` in `FWHsci::Release` hung `kextunload`; removed); **O1 is resolved** (the clone's `stopAudioEngine` called `pauseAudioEngine` instead of `performAudioEngineStop`, leaking the nesting counter); **O3** (23 s bring-up) was caused by AV/C commands timing out (now 13 s). Evidence: `A1-ghidra/Q-AVC-unit-and-O1.md`, `PROVENANCE-RESOLUTIONS.md` §R56.


**Host:** build PC (iMac11,2, macOS 10.13.6 17G14042), SIP disabled, **no `boot-args` (kext-dev-mode 0)**; unsigned kext loaded from `/tmp` with `sudo kextutil` (never copied to `/Library/Extensions`; the old M-FW build installed there autoloads at boot). User present; interface powered, no hot-plug, no flash. Tools: `tests/hw-smoke/integrated.sh --preflight`, `tools/maprop.c` (bench property / meter-page tool), panel `--snapshot` on the real device, `sudo dmesg` (the `log show` view of IOLog lines shows `<compose failure>` for kexts loaded from `/tmp` after a reload; dmesg is reliable).

## What worked (first time on hardware)
1. **Load:** nub found (`GUID:d6c040012337f product:FW 1814 bootloader:0`), "Firmware active: registering metanub", TX program 1492 DCLs (4 banks x (4 primers + 1 x 369)), RX 256 DCLs (64 x 4), `initHardware: up` after ~23 s. No panic on load.
2. **Property surface:** 36 keys in `MAMetaAllProperties` (the registry table holds only the 8 static keys); writes round-trip (`MAOutputLevels`, `MAGlobalDim`).
3. **CoreAudio:** after fix F1, "FireWire 1814 Multichannel" listed (10 in / 6 out at 44.1 kHz S/PDIF mode, transport FireWire).
4. **Audio out:** the user played music through ReAmp: clean sound on out 1/2.
5. **Panel on the real device:** About tab shows the real firmware info (bootloader 2004-03-30 02:59:09, firmware 2007-07-13 08:04:40, model 131 version 1, GUID 0xD6C040012337F); after fix F3 meters, front Level knob and faders work ("funcionou perfeito"); phones work once a monitor source is clicked.
6. **Mixer writes:** every block write to 0xFFC7:0x00700000 returned 0 (dmesg diagnostics); headphone selector writes to Reg 0x98 when a monitor source is clicked.
7. **Clean `kextunload`** twice (no HSCI read in flight at those moments).

## Fixes made during the run (all committed)
| # | Symptom | Root cause | Fix |
|---|---|---|---|
| F1 | Device absent from Audio MIDI Setup | `ReallyRegisterService` called `IOService::registerService`, skipping `IOAudioEngine::registerService`, so the streams were never registered (ioreg `FWStream !registered`; AppleHDA streams and the SL reference streams are registered). The reference jumps through FWDiscreteEngine's vtable slot 0x5c0 (mafw.asm 0x398c..0x3994) = the inherited IOAudioEngine::registerService. | `FWBaseEngine.h`: `IOAudioEngine::registerService(0)` |
| F2 | Panel would start with an empty cache | `IORegistryEntryCreateCFProperties` returns only the static table; the mixer keys come through the kext's copyProperty / `MAMetaAllProperties` | `IOKitBridge.copyProperties` merges `MAMetaAllProperties` |
| F3 | Meters dead, front knob / A/B switch ignored | `IOFWReadQuadCommand::reinit` returns an IOReturn (0 = success) but was tested as a bool: every HSCI poll after the first failed silently (1 s retry) | `FWHsci.cpp` |
| F4 | Phones silent until a monitor source is clicked | Nothing writes Reg 0x98 at start-up (the reference's `ABSwitch::SendToDevice` only normalises a flag) | clone addition `ABSwitch::SendHeadphoneSelectors()` called from `FW1814Mixer::ResendCurrentMixerSettings` (**not yet verified on hardware**) |
| F5 | Panel parameters jump only on mouse-up | panel committed at mouse-up (reference behaviour) | user request: `DeviceModel` flushes pending writes every 40 ms while dragging |
| F6 | Output balance moves the faders | reference writes balance into the two channel levels (cookie 0x5f6) | user request: balance kept as panel state per pair; device level = fader - balance attenuation of that side |
| F7 | A/B text "3/4, -" | panel read `MAMomentarySwitchRoutings` entries 1/2; the kext indexes entries by A/B mode (fMode 0 = A, 1 = B) and its default `{1,2,3,4}` = A out 1/2, B out 3/4 | panel reads/writes entries 0/1 (**verify on hardware**) |
| F8 | Rate locked after the ADAT hang | the M-O gate blocked on engine state alone; the engine was left "running" with 0 clients | gate = running **and** `numActiveUserClients > 0` |
| F9 | Second panel instance | — | single instance: a second launch activates the running one |
| F10 | **Kernel panic on `kextunload`** (`2026-10-16-panic-kextunload.txt`): page fault in IOFireWireFamily called from AppleFWOHCI, CR2 0x18, R8 = 0x600000, RAX = 0xffc7 = an async command to 0xFFC7:0x600000 = the HSCI read completing after its owner was freed | HSCI read in flight during teardown (it never happened before F3 because every read after the first failed) | `FWHsci`: retain while a read is outstanding, `Release()` cancels it, late completions ignored (**built and kextutil-validated, not yet loaded**) |

## Open, not solved
- **O1 — switching optical S/PDIF -> ADAT while audio plays hangs the stream.** dmesg: `IsochStreamRunning during SetPacketSize` twice; afterwards engine state 1 with 0 clients, no sound, rate change refused (F8 fixes the refusal). The DCL programs were still running when `UpdateDCLProgramsWithNewPacketSizes` ran; the reference panics in that case (`FWDCLProgram.cpp:1727` assert), so in the reference the isoch streams must be stopped before (pause -> `performAudioEngineStop` -> `DeactivateSubEngine` -> `StopIsochStreams` -> `StopAudio`). Diagnostics added: `FWEngine2::LogStreamState` before/after the pause in `SetClockSourceInternal` and in `UpdateDCLProgramsWithNewPacketSizes` (engine state, isoch refcount, active sub-engine mask, pause count, TX/RX running). Next: reproduce (with and without audio playing), read dmesg, find who keeps the isoch refcount / active mask non-zero; also check that `Disconnect()` (reference @0x250a2 clears [+0x8c] only) never leaves a program "running" without a channel.
- **O2 — CoreAudio did not show new channel counts after the switch** (expected 16 in / 12 out at 44.1 kHz ADAT): follows from O1 (format change aborted).
- **O3 — `initHardware` takes ~23 s** (start 20:18:56, up 20:19:19): includes the 2.5 s blank-slate sleep and AV/C/CMP steps; not analysed yet.
- **O4 — diagnostics to remove** once the mixer/HSCI/clock paths are verified: rate-limited `OA_LOG` in `CustomMixerWriteQuads`, `ReadHSCIData`, `FWHsci`, `LogStreamState`.
- Still to run from the ladder: input capture / loopback, rate sweep, ADAT and 192 kHz layouts, MIDI (reference plug-in), sleep/wake, the conformance of meters per channel.

## Next session — first steps
1. On the build PC the old M-FW kext is loaded after the reboot. Stage the current build (`scripts/stage-kext.sh`), unload the old one with **no panel or audio app open**, load from `/tmp`, open the panel.
2. Verify F4 (phones play at start-up), F7 (A/B text "A = out 1/2, B = out 3/4"), F10 (`kextunload` with the panel closed and then with the HSCI poll running: no panic).
3. Reproduce O1 with dmesg running; fix; then continue the ladder (`tests/hw-smoke/integrated.sh --from 6`).

## Second session (2026-10-17)
SL v1.9 reachable for ground truth; DTrace on the build PC (10.13, SIP off) for the AMS side. Tools added: `tools/aduplex.c` (duplex CoreAudio client, silence out), `tools/halwatch.c` (HAL notification logger), `tools/maprop.c` `MA_CLASS=` (same tool against the official driver).
| # | Symptom | Cause | Fix / status |
|---|---|---|---|
| S1 | `kextunload` hung (uninterruptible, no panic) | F10's `fCmd->cancel()` in `FWHsci::Release` on the termination thread | no cancel; command and buffer freed in `free()`; 3 load/unload cycles + unload with panel open clean |
| S2 | Phones audible at 0 dB at start-up (F4) | clone addition wrote Reg 0x98 at start-up | F4 reverted; panel caps phones at -86 on open and restores last settings per GUID (`DeviceModel.applyStartupSettings`) |
| S3 | O1: S/PDIF→ADAT with audio hangs | `stopAudioEngine` → `pauseAudioEngine` (leaked `[+0x234]`) | `performAudioEngineStop` (asm 0x18dba..0x18dc0); verified under duplex load |
| S4 | All AV/C commands time out (1.27 s); SW return 1/2 → out 3/4 not sent; bring-up 23 s | no `IOFireWireAVCUnit` object: no FCP response delivered | `FWAvcUnit` rebuilt as an `IOFireWireAVCUnit` subclass + async queue + `fCommand`; 22 ms ACCEPTED, bring-up 13 s, routing works (user) |
| S5 | Stream formats differ from the official | alignment HighByte(1) vs official 0; no format extension | alignment 0 + extension (`FormatFlags 0, FramesPerPacket 1, BytesPerPacket ch*4`) |
**Open:** AMS sidebar "N ins / M outs" stays 10/6 after a switch (pane updates; hypothesis in Q-AVC-unit-and-O1 §3); panel reverts a rate chosen in the panel (the official kext also keeps the old `MASampleRate`, its panel keeps the chosen rate in red - capture 80); remaining ladder items (input/loopback, rate sweep, MIDI, sleep/wake); diagnostics to remove (O4) incl. the per-command `AVC …` log in `FWAvc.cpp`.
