# Capture 106 — first run of the clone with the FireWire 410 (iMac build PC, Classic, 2026-10-26)

Machine: iMac11,2, macOS 10.13.6, nothing of OpenAudio installed (no kext in /Library/Extensions, no helper, no panel); the FW410 alone on the bus (user). Driver **0.3.0** (branch `fw410`, commit after `483dc5f` + chtone/ratecycle edits), built on the build PC, staged in `/tmp` (`scripts/stage-kext.sh`, `kextutil -t -n` OK) and loaded by hand with `sudo kextutil /tmp/OpenAudioBeBoBMAFW.kext` (UUID `C72A48B4-ADDB-38D8-8695-C9FE75F279DB`) — **nothing installed**. Firmware start sent with `openaudio-fw1814/tools/mafw-start.c` (stand-in for the helper). Tools in `~/fw410-run` on the build PC: `mafw-start`, `chtone` (new device-name argument), `slfwread` (status packet reads), `ratecycle` (now matches `com_m_audio_FWEngine*`), `maprop` (`MA_CLASS=com_openaudio_FW410Device`). Signal checks read the 410's own hardware meters from the status packet `0xFFC7:0x00600000` while `chtone` plays a −40 dBFS 1 kHz tone on one channel of a CoreAudio device; not checked by ear.

## Attach
* At boot the 410 is the bootloader: **"FW Bootloader", Model_ID 0x10058, GUID 0xD6C011006B8C0** (confirms snd-bebob `bebob.c:446`; runtime GUID 0xD6C010006B8C0). The nub attached; `MAActualProductName` = "FW 410", `MAFirmwareInfo` firmware 2007-05-04 10:26:56.
* `MAFirmwareStart` → the unit re-enumerated as **"FW 410", Model_ID 0x10046**; a second `MAFirmwareStart` on the runtime nub (the reference's two-round attach, ARCHITECTURE §4.1) → `com_openaudio_FW410Device` + `FWAvcUnit` + `FWEngine2` + 2 streams within ~2 s; registered ~2 s later. No panic.
* Engine `IOAudioEngineGlobalUniqueID` = **`com_m_audio_FWEngine1:0xD6C010006B8C0`** = the official driver's (capture 103). CoreAudio: "FireWire 410 Multichannel", **4 in / 10 out, 44.1 kHz, locked**. Registry `ioreg-fw410device.txt`, `MADebugTrace` `madebugtrace.txt`.
* Latency properties at 44.1 kHz: input 75, output 75, offset 44 (official v1.9 on the SL rig: 32 / 226 / 40) — **open**: compare with the v1.11.2 latency path.

## Signal (meters)
* `imac-44k.txt`: at 44.1 kHz channels 1-10 each reach exactly their own output (q4-q13, 0x0147 = the official driver's level for the same tone, capture 105); the phones (q14/q15) follow out 1/2 (factory phones source).
* `imac-192k.txt`: rate set to 192 kHz through CoreAudio (`ratecycle 192000`, status 0, 2 in / 8 out); channels 1-8 each reach their own output (q4-q11) through the decoded 192 kHz output map (R136). Back to 44.1 kHz.
* Stereo pairs: `MAEnableStereoPairEngines = 1` → five pair engines, names and UIDs identical to the official driver ("FireWire 410 Analog 1/2" `m_audio_b_FWPairEngine:1` … "Analog 7/8 Out" `:5`), CoreAudio 2/2, 2/2, 0/2, 0/2, 0/2; channel 1/2 of each pair device reaches out 1/2, 9/10 (S/PDIF), 3/4, 5/6, 7/8. Option set back to 0 (engines removed).

## Mixer (meters, `imac-mixer.txt`)
| Write | Result |
|---|---|
| `MASWReturnRoutingV2` [2,…] / [3,…] | SW 1/2 → out 3/4 only / → out 1/2 + 3/4 (register 0x64 word) |
| `MAHeadphoneSourceV2` 2 / 1 | phones carry bus 3/4 / bus 1/2 (register 0x68) |
| `MAHeadphoneSourceV2` 32 (bit 5) | phones carry none of the five output buses; with SW 1/2's aux sends at 0 dB the phones carry the **aux bus** (q16 = aux, q14 = phones) — **bit 5 = aux bus as a phones source**. Because the reference keeps 0xc4ffffff, **bit 26 is never cleared** afterwards: masks 0 / 1 still give aux on the phones (latched until the driver reloads) |
| `MAAuxSendLevels` SW 1/2 at 0 dB | aux bus (q16) carries the tone (AV/C aux-send path) |
| `MAOutputLevels` out 1/2 −∞, `MAOutputMute` out 1/2 | the out 1/2 meter (q4) keeps the tone (it is taken before the output fader), the phones (bus 1/2 after the output fader) go silent → the output level / mute writes arrive |
| `MASWReturnLevels` SW 1/2 −∞ | out 1/2 silent (register 0x00 batch writer) |

State left: driver loaded by hand (not installed), stereo pairs off, levels / routing / aux sends restored, **phones word bit 26 still set** (aux bus on the phones; silent because every aux send is −∞) until the driver is reloaded. Panel 1.6 (`control-panel/build`) open on the iMac against the 410; snapshots in `panel/`.

## Not checked
By ear; inputs / recording (needs a source); MIDI; the front Level Controller encoder; S/PDIF in/out and external sync; coaxial/optical switch; sleep/wake; the helper (not installed); install through the package.

## Latency (loop cable out 3 -> in 1, user 2026-10-26; `tools/looplat.c`, new `LOOPLAT_OUT=3`)
Round trip in the IOProc stream at a 64-frame buffer, against the driver's reported `IOAudioEngine{Input,Output}SampleLatency` + 2 x `IOAudioEngineSampleOffset`:

| Rate | Measured | Minus 2 buffers | Reported in + out + 2 x offset | Reported short by |
|---|---|---|---|---|
| 44.1 kHz | 378 fr (8.6 ms) | 250 | 75 + 75 + 88 = 238 | 12 fr |
| 48 kHz | 408 fr (8.5 ms) | 280 | 80 + 80 + 96 = 256 | 24 fr |
| 88.2 kHz | 593 fr (6.7 ms) | 465 | 127 + 127 + 176 = 430 | 35 fr |
| 96 kHz | 604 fr (6.3 ms) | 476 | 123 + 122 + 192 = 437 | 39 fr |
| 176.4 kHz | 924 fr (5.2 ms) | 796 | 220 + 220 + 352 = 792 | 4 fr |
| 192 kHz | 1056 fr (5.5 ms) | 928 | 240 + 240 + 384 = 864 | 64 fr |

The reported latency is within 0.5 ms of the measured round trip at every rate (always a little short); the official v1.9 figures at 44.1 kHz (32 / 226 / 40 -> 338) would overstate it by 88 frames (2 ms). At 44.1 kHz / 512 frames: 1282 fr (29.1 ms). **Input 1 records at every rate, 192 kHz included** (closes the 192 kHz input question).

## Driver unload
`kextunload` after closing the panel (engine idle): the devices terminated, but the kext stayed loaded — **one `com_openaudio_FWHsci` instance left** ("classes have instances"). Open bug (R138): the Rev2 status-packet poller outlives its device; suspected an in-flight read whose completion never runs after termination. Affects unloading / in-place updates only. The fixed kext (phones aux bit, R138) is staged in `~/fw410-run/stage` on the build PC for loading after a restart.

## After the restart (fixed kext UUID F15E45EF, phones aux bit R138)
* The 410 kept its firmware through the iMac restart (enumerated as "FW 410" at boot); one `MAFirmwareStart` attached the device.
* Aux fix on the meters: phones = aux -> tone; back to 1/2 (empty bus) -> silent; 1/2 + aux -> tone; 1/2 -> silent.
* By ear (user): tone heard on the **right** phones channel only — the unit's phones jacks/amp pass one side only, the same under the official v1.9 (user) -> a hardware fault of this unit, not the driver. Phones fader to -20 dB mid-tone: the level dropped (user) -> the AV/C headphone volume (kind 1, fb 0x0F) works.
* Front Level Controller (user, with panel 1.6 open): **works, the faders follow the assignment** (encoder-log.txt: MA*Levels changes while turning). With the panel open the phones level is set to -86 (-43 dB) at launch.
* Panel: "src 6" relabelled "aux"; sidebar height now follows the number of level-controller radios (the 410's five radios cropped the mute/dim row on the output tab; user).

## Installed through the package (OA-BeBoB-MAFW-0.4.0-classic.pkg, user OK), restart
* `installer -pkg` successful; kext 0.3.0 in /Library/Extensions (same UUID F15E45EF as the hand-loaded build), helper LaunchDaemon, panel 1.6 in /Applications.
* After the restart, without any hand step: kext autoloaded, helper running, **device registered 34 s after boot**; helper log (`helper-log-after-install.txt`): nub "FW 410" state=running firmware 20070504 102656 -> `MAFirmwareStart` 0x0; first run for this GUID -> "no saved settings: phones limited to -86" (the -43 dB phones the panel showed).
* Repeat checks: outputs 1-10 one-to-one at 44.1 kHz (meters); loop out 3 -> in 1 at 48 kHz / 64 frames = 400 frames (8.3 ms). The device was left at 48 kHz by the loop test; panel opened from /Applications.

## Unload fixed (R139)
* Diagnosis: with the installed 0.4.0 the first `kextunload` failed ("com_openaudio_FWHsci has 1 instance") and a retry 3 s later succeeded -> not a leak: the status read in flight at stop() keeps the poller (and the kext) alive until its completion, and kextunload counts the instances before it arrives. The installed kext was then reloaded without a restart (helper: firmware start + "replayed saved settings for D6C010006B8C0").
* Fix: `FWAVCAudioDevice::stop()` waits for that read (`FWHsci::WaitForReadDone`, 5 ms steps, max 2 s) before dropping its reference. Hand-loaded build (UUID 8EF32836-…): **7 of 7 load / attach / kextunload cycles unloaded on the first try** (one cycle logged a read still outstanding at 2 s; it was gone by the instance check). Package 0.4.0 Classic rebuilt with it and installed as an update; the installed kext (same UUID) loaded by hand, unloaded on the first try, loaded again; helper replayed the saved settings. No restart.

## Phones level (user report "ultra low", 2026-10-26)
With the user's music playing on out 1/2 and `MAHeadphoneLevels` 0/0 (0 dB, not muted, no dim, source out 1/2), the 410's phones meters (status q14/q15) read full scale (0x7fff) -> the full digital level reaches the phones bus. The user hears it low, distorted and on the right side only, the same under the official v1.9 -> **the unit's headphone amplifier is faulty (hardware)**; no driver change. Workaround: monitor from a line output pair (same buses in the panel).

## Two panics on power-off, and the host-plug fix (R140)
* **23:43** (user switched the 410 off while music played) and **23:51** (again, music playing): both `IOFireWirePCRSpace` -> `HandlePCRLockStub` in an **unloaded** image (backtrace `AppleFWOHCI` -> `IOFireWireFamily` -> `IOFireWireAVC +0x425e`, RCX/R8 = 0xC1003C7A/0xC0003C7A = the host oPCR payload) — the panic of capture 97 / R117. The faulting address minus the stub's offset gives a page-aligned base = the load address of the copy unloaded earlier: 23:43 -> build F15E45EF (stub @0x9600, base 0x...84044000 = its boot load address + slide), unloaded at 23:16; 23:51 -> build 8EF32836 (stub @0x94e0, base 0x...9ca44000 = "last unloaded" in the report), unloaded idle to swap in a test build. Reports `2026-10-26-panic-imac-fw410-power-off-*.txt`.
* **Why idle too:** the plugs are connected at attach / rate change (`ConnectAudioMIDIChannels`) and kept while idle, so an idle driver holds host plug 0; the old builds' only free on unload was `FreeHostPlugs()` from the connection manager's `free()`, and it did not reach the plug in practice (cause not isolated: free order / timing during termination — hypothesis).
* **Fix:** `FWAvcConnectionManager::Close()` from `FWAVCAudioDevice::stop()` (frees every allocated host plug while the nub is valid, refuses later `Connect_iPCR`), `FWRemoteIsochPort` retains the manager; plug allocate/free traced to `MADebugTrace` (`CMP: ...`).
* **Check without a bus reset:** `IOFireWirePCRSpace::allocatePlug` hands out the lowest free client slot (asm 0x44f6), so the slot the next driver gets shows whether the previous one freed its plug (0 = freed, 1 = leaked). Build ED7E32BD: after an idle unload, after an unload **while streaming**, after a 410 **power-cycle with the driver loaded** (user: iMac survived) and after the swap to the installed package: every time `CMP: host plug 0 allocated (slot 0)`. Package 0.4.0 Classic rebuilt and installed with ED7E32BD.
* Earlier attempt `hostpcr` (read the Mac's own oPCRs through the local node): every read `0xE0008017` (local PCR space not served) — dropped.
