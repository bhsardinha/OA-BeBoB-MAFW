# Gap analysis — official stack vs the clone (kext, helper daemon, panel app)

> **Correction (2026-10-17, later):** K6 closed by your decision (pass-through controls added, 18 + 1 verified in the registry); P-3 implemented; M-1 plug-in installed and loading; K7 has new evidence. See R58 and capture 82. The rows below are updated in place.

**Date:** 2026-10-17. **Method:** side-by-side registry/process/preference comparison of the SL rig (official v1.9, live) with the build PC (clone, 10.13.6), the v1.11.2 reference binary (`_extracted/`, the clone's spec) and the system-map documents. Evidence: captures `76`-`81` under `B5-sl-dtrace-truth/captures/`, `PROVENANCE-RESOLUTIONS.md` §R56/§R57, `A1-ghidra/Q-AVC-unit-and-O1.md`.
**Reading guide:** *Closed* = fixed and verified on hardware this session. *Open* = a difference remains. *Unverified* = implemented but never exercised on hardware (nothing is claimed). *Decision* = the two reference builds disagree, so the user chooses the target. Version skew is real: SL = v1.9, spec = v1.11.2 (shipping target).

## 1. Kext

| # | Official behaviour (source) | Clone state | Status | Next action |
|---|---|---|---|---|
| K1 | `m_audio_b_FWAVCUnit` is an `IOFireWireAVCUnit` subclass created by the device (SL `ioreg`; asm 0x23c72, 0x1594, 0x17fc) | rebuilt 2026-10-17: commands ACCEPTED in 6-22 ms (were 1.27 s timeouts), bring-up 23 s → 13 s, SW-return routing works | **Closed** (R56) | 5-minute stability, bus reset, unload with the async queue busy |
| K2 | `stopAudioEngine` calls `performAudioEngineStop` (asm 0x18dba) | fixed; optical switch under duplex load works, CoreAudio 16/12 ↔ 10/6 | **Closed** (O1/M-Q) | — |
| K3 | stream format: alignment 0, format extension (FormatFlags 0, FramesPerPacket 1, BytesPerPacket ch×4) (capture 67c; asm 0x4192) | mirrored | **Closed** | — |
| K4 | engine `IOAudioEngineGlobalUniqueID` = `com_m_audio_FWEngine2:0x<GUID>` (SL registry) | clone built `…:com_openaudio_FWEngine2:0x…` (wrong slot, `getName` vs `getLocation`); fixed | **Closed** (R57) | **Effect:** apps that stored the clone's old UID must re-select the device once; sessions saved with the official driver now match |
| K5 | `FWHsci` read in flight at teardown | no synchronous cancel; retain pins the object | **Closed** (S1) | — |
| K6 | CoreAudio-visible controls: v1.9 also exposes **19 pass-through controls** (18 level + 1 mute, usage `pass`); v1.11.2's `FWSettingsLevels::Init` does **not** (verified in both binaries, capture 81) | was v1.11.2 (46 vs 65). **M-U (your decision): the v1.9 calls were added** (`FWSettings.cpp` Init, asm 0x23cb8 / 0x23d8f) and 18 'pass' level + 1 'pass' mute are in the registry (capture 82 §3) | **Closed** (deviation from the v1.11.2 spec, by decision) | AMS shows them under the device's controls — look once |
| K7 | AMS device-list label "N ins / M outs" follows the optical switch (SL AMS 10.6) | stays 10/6 on 10.13 (pane updates, HAL reports 16/12, AMS re-reads 16/12) | **Open by decision (R60 correction)**: an engine re-publish fixes the label (screenshots 83a/83b) but kills live clients (reAMP froze), so it is OFF; the reference pause/change/resume path is used and the sidebar label may be stale. Earlier state — new evidence: you saw 16/12 when the device was added while already in ADAT (user observation, capture 82 §5), as the hypothesis predicted | the in-place change cannot refresh the label on 10.13; the remaining option is a HAL re-publish at an optical switch (a 10.13 deviation that would drop the stream during the switch) — **your decision**; the official driver's behaviour on 10.13 itself is unobservable here |
| K8 | sleep/wake (`SystemWillSleep/WokeUp`, `WakeAfterSleep`) (M-E2) | written | **Unverified** | integrated test with the stream open |
| K9 | bus reset → `ReconnectP2P` on a new thread (`FWAudioDevice::message` on `kIOMessageServiceIsResumed`, asm 0x3344) | wired in `FWAudioDeviceChain.cpp:488-516` | **Unverified** | power-cycle the interface with a stream open (no hot-plug) |
| K10 | MIDI in/out through the user client (selectors 500-507) | kext half written (M-H); the plug-in's start call (500) reaches it and brings the isoch streams up (capture 82 §2) | **Partly verified** | loopback with a cable |
| K11 | AC3 pass-through (`cac3`) | written (M-I) | **Unverified** | needs an S/PDIF AC3 source |
| K12 | 4-engine family / `MAEnableStereoPairEngines` (M-J) | written; the FW1814 default shows one engine on both machines (class counts equal, capture 81) | **Unverified** with the option on | toggle the option on both rigs and compare `ioreg` |
| K13 | multi-interface (M-K) | skeleton only | **Open** | needs a second FW1814 |
| K14 | latency per rate (`SampleOffset`/`OutputSampleLatency`, C6) | tables ported (M-D) | **Unverified** | loopback jig (M-L) |
| K15 | boot state: only `/Library/Extensions/OpenAudioFW1814.kext` is loaded at boot | the installed copy is the **old M-A build (Oct 15, 101 KB)**; the current kext only exists in `/tmp` | **Closed 2026-10-17 (night)** - installed by the new package (R59), current build loads from `/Library/Extensions` | reboot autoload still to be observed |

## 2. Helper daemon (official: two StartupItems — "M-Audio Firmware Loader" + "M-Audio FireWire Device Helper"; clone: one LaunchDaemon)

| # | Official behaviour (A4) | Clone state | Status | Next action |
|---|---|---|---|---|
| H-1 | firmware start for a bootloader/current nub (H2) | works for the FW1814 (`MAFirmwareStart`, logs) | **Closed** | other products are out of scope (flash writes out of scope, R42) |
| H-2 | **settings persistence owned by the helper**: per GUID, 4 s after the last change, under the console user; replay on attach/login with one `setProperties` (H11, capture 71) | the installed helper was the minimal H.1 build and persisted nothing; H.4 build installed 2026-10-17 and verified: file `com.m-audio.device-settings.firewire` (0600, user-owned, key `D6C040012337F`, the same 36 user-settable keys), replay after a kext reload with the panel closed | **Closed** (H.4) | first-ever attach seeds from `MAMetaAllProperties` (as the reference); see residual risk below |
| H-3 | the helper drops its device instance when the service goes away | **bug found today**: it pinned the device object, `kextunload` exit 3; fixed (interest notification on termination); scripts now stop the helper first and retry | **Closed** | the first `kextunload` can still race the helper by a few seconds (retry succeeds) |
| H-4 | gating by system sleep and console user (H.3) | written, **not exercised** (no sleep/logout test yet) | **Unverified** | sleep/wake and fast-user-switch test |
| H-5 | firmware status UI app (FirmwareStatusWindow) | cut from scope (H.5) | **Decision (recorded)** | none |
| H-6 | launch at boot, clean unload | `RunAtLoad`+`KeepAlive`; `launchctl unload` verified clean | **Closed** (H.6) | boot test with the final installer |
| — | **Residual risk:** the replay may re-select a headphone source with a saved level; if the saved phone level is 0 dB the phones are loud before the panel opens (the panel's -86 cap only runs when it opens) | the panel caps and the cap is persisted by the helper, so saved levels are the capped ones | **Open (low)** | optionally clamp phone levels in the helper replay (a clone addition, needs your approval) |

## 3. Panel app (official: `M-Audio FireWire.app` Carbon panel + prefpane)

| # | Official behaviour (A2) | Clone state | Status | Next action |
|---|---|---|---|---|
| P-1 | rate: pick shown at once, `setNominalSampleRate:` after 1 s, display follows CoreAudio, lock colour from `MARateLocked` (P8 §6, capture 80) | implemented (`HALRate` + `DeviceModel`), unit-tested; **user-confirmed after a manual test** | **Closed** (PN.8) | — |
| P-2 | the panel does not persist settings (A4 H11) | the panel's own save/restore removed; only the clone's headphone cap remains | **Closed** | — |
| P-3 | supported rates come from CoreAudio (`AvailableNominalSampleRates`, P8 §10) | the Hardware tab offers a fixed list of 6 rates | **Implemented** (`availableRates` from the HAL, six-rate fallback, 15 host tests); live behaviour across a digital-mode change **unverified** | watch the stepper while switching S/PDIF/ADAT (S/PDIF has no 176.4/192 kHz; B5 captures 67a-c) |
| P-4 | rate display update when Audio MIDI Setup changes it | the panel header followed within ~20 s in a background test; the "resync only on click" report was not reproduced | **Unverified** | watch it during your manual testing |
| P-5 | prefpane (`M-Audio FWPanel.prefPane`) | dropped by decision (Proposal.md) | **Decision (recorded)** | none |
| P-6 | the panel app is not installed anywhere (runs from the build tree) | — | **Open** (IN.5) | installer |

## 4. CoreMIDI and installer

| # | Official | Clone | Status | Next action |
|---|---|---|---|---|
| M-1 | `/Library/Audio/MIDI Drivers/MAudioFireWireMIDI.plugin` (64-bit slice); it finds the device with `IOServiceMatching("com_m_audio_FW1814Device")` (plug-in strings) | **installed 2026-10-17** (unmodified, `/Library/Audio/MIDI Drivers`); MIDIServer restarted; it loads and lists "FireWire 1814" 1 in / 1 out (capture 82 §1); TX sends accepted, no panic (§2) | **Partly verified** | bytes on the wire, RX, SysEx round trip: needs a DIN cable OUT→IN on the 1814 (M-H/M-H2/M-V) |
| I-1 | packaged installer (kext, panel, helper, plug-in) | `dist/OpenAudioFW1814-0.1.0.pkg` built and installed on the build PC (R59); Developer-ID signing not started | **Partly done** (IN.5 built; IN.6 open) | clean-machine/SIP-on test, signing |

## 5. Audio MIDI Setup crash on "Show MIDI Studio" (reported 2026-10-17 09:03)
The crash report (capture 82 §4) is inside Apple CoreUI/QuartzCore (`CAPackage` layer decode, `toggleMIDIWindow:`); no frame from the kext, panel or any M-Audio plug-in, and the plug-in was not installed at the time. Not attributed to the clone; one report, not reproduced. To re-check with the plug-in installed: open MIDI Studio again.

## 6. What this does not claim
No claim here is stronger than its citation. Items marked *Unverified* have code but no hardware evidence; the AMS label (K7) has a hypothesis only; the K15/H-4/K8/K9 tests need the interface power-cycled or the machine slept, which is on your side (no hot-plug, no flashing).
