# A1 — Reference kext static disassembly

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - Line 55 'FW1814Device → likely FWRev2AudioDevice' is wrong: OSMetaClass superclass at `0xf473` = **FWRev1AudioDevice** (see Q16 correction).


**Target binary:** `_extracted/kext/M-AudioFireWireBeBoB.kext/Contents/MacOS/M-AudioFireWireBeBoB`
Also present at repo root for direct inspection: `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` (identical bytes; convenience copy).

**Bundle meta (`_extracted/kext/M-AudioFireWireBeBoB.kext/Contents/Info.plist`):**

- `CFBundleIdentifier` = `com.m-audio.driver.firewire`
- `CFBundleVersion` = `1.11.3`
- `DTSDKName` = `macosx10.7`, `DTXcode` = `0463` (Xcode 4.6.3)
- `OSBundleLibraries_x86_64`: `IOAudioFamily 1.1`, `IOFireWireAVC 1.0`, `IOFireWireFamily 1.0.0`, KPIs `kpi.iokit 8.0`, `kpi.libkern 8.0`, `kpi.mach 8.0` — KPI 8.0 was the first stable kernel KPI (10.4-era) and is still accepted on 10.13 for backward compatibility, so this is **not** the loader-blocker on High Sierra.
- Fat/arch: x86_64-only kext bundle (`file(1)` reports `Mach-O 64-bit kext bundle x86_64`); no i386 slice at all, which matches 10.13's x86_64-kernel-only constraint.

## Status / next step

**In-progress.** First-pass symbol-table survey complete (nm + c++filt). What's below is accurate for *existence and relationships*; **pseudocode-level content for each listed method still needs a Ghidra pass**. Resume anchor: pick a method out of the §Open questions / Ghidra queue list and decompile it.

## Finding 1 — Info.plist matching is NOT what we assumed

Our clone matches `IOFireWireUnit` on Vendor_ID + Model_ID per personality. **The reference kext does something fundamentally different.**

| Personality | IOProviderClass | Match keys | Role |
|---|---|---|---|
| `FWMetaNub` | `IOFireWireDevice` | *(none in plist — pure class match)* | Catch-all nub for every FireWire device on the bus |
| `com_m_audio_FW1814Device` | `com_m_audio_FWMetaNub` | *(none in plist)* | Publishes FW1814 device on top of nub |
| `com_m_audio_FW410Device` | `com_m_audio_FWMetaNub` | *(none)* | FW 410 |
| `com_m_audio_FWAudiophileDevice` | `com_m_audio_FWMetaNub` | *(none)* | FW Audiophile |
| `com_m_audio_FWLightbridgeDevice` | `com_m_audio_FWMetaNub` | *(none)* | ProFire Lightbridge |
| `com_m_audio_FWNRV10Device` | `com_m_audio_FWMetaNub` | `Model_ID = 65665` | NRV-10 (**only** per-model Model_ID match in plist) |
| `com_m_audio_FWOzonicDevice` | `com_m_audio_FWMetaNub` | *(none)* | Ozonic |
| `com_m_audio_FWProjectMixDevice` | `com_m_audio_FWMetaNub` | *(none)* | ProjectMix |
| `com_m_audio_FWSoloDevice` | `com_m_audio_FWMetaNub` | *(none)* | FW Solo |
| `FWSoloAppleGoAway1` | `IOFireWireAVCSubUnit` | `Vendor_ID 3436`, `SubUnit_Type 1` | AppleFWAudio detach (Audio) — **FWSolo only** |
| `FWSoloAppleGoAway12` | `IOFireWireAVCSubUnit` | `Vendor_ID 3436`, `SubUnit_Type 12` | AppleFWAudio detach (Music) — **FWSolo only** |

**Consequences:**
1. The nub matches **any** FireWire device, then `com_m_audio_FWMetaNub::probe()` filters by reading the device's config ROM (Vendor_ID / Model_ID) in C++. All per-model device classes then match the nub by *class alone* and discriminate again in their own `probe()`.
2. `MakeAppleFWAudioGoAway` is scoped to **FWSolo**. The reference does **not** detach `AppleFWAudio` for the FW1814. Either (a) `AppleFWAudio` ignores the FW1814 of its own accord, (b) the nub-first match wins and `AppleFWAudio` never gets a shot because the FWMetaNub holds the device open, or (c) FWSolo was the only product the detach was ever needed for. **This has to be resolved in Part C1 before the clone's `com_openaudio_MakeAppleFWAudioGoAway` for FW1814 can be considered correct.**
3. Our clone's `IOFireWireUnit`-based matching is a *different paradigm*. Matching at `IOFireWireUnit` means the kext never sees the parent `IOFireWireDevice` as its provider — which is exactly where some firmware/bootloader state lives. This is a strong candidate for why the clone sees TX silence: wrong provider in the service graph.

## Finding 2 — Class hierarchy as read from vtables

Class list (c++filt of all `__ZTV*` symbols in the binary):

**IOKit nub chain:**
- `com_m_audio_FWMetaNub` : `IOService` (likely — confirmed via Ghidra of its MetaClass CTOR; next step)

**Shared audio-device base (reused across all 8 products):**
- `m_audio_b_FWAudioDevice` → `IOAudioDevice` (vtable size + method set matches IOAudioDevice overrides)
- `m_audio_b_FWAVCAudioDevice` → `m_audio_b_FWAudioDevice` (adds AV/C: `AVCControlFunctionBlock`, `AVCMixerConnectPair`, `AVCStatusOutputPlugSignalFormat`, `AVCVendorCommand(...)`)
- `FWRev1AudioDevice` → `m_audio_b_FWAVCAudioDevice` (23 methods; probable "revision 1 BeBoB" split)
- `FWRev2AudioDevice` → `m_audio_b_FWAVCAudioDevice`

**Per-product device classes:**
- `com_m_audio_FW1814Device` → likely `FWRev2AudioDevice` (confirm via Ghidra of CTOR)
- `com_m_audio_FW410Device`, `com_m_audio_FWAudiophileDevice`, `com_m_audio_FWSoloDevice`, `com_m_audio_FWOzonicDevice`, `com_m_audio_FWProjectMixDevice`, `com_m_audio_FWLightbridgeDevice`, `com_m_audio_FWNRV10Device`

**Engine chain:**
- `m_audio_b_FWBaseEngine` → `IOAudioEngine` (66 methods — the heavy-lifting base: `clipOutputSamples`, `convertInputSamples`, `performAudioEngineStart/Stop`, DCL program add/remove, MIDI schedule, format change, sample-rate change)
- `m_audio_b_FWPairEngine` → `m_audio_b_FWBaseEngine` (stereo pair engine)
- `m_audio_b_FWDiscreteEngine` → `m_audio_b_FWPairEngine` (per-channel discrete engine)
- `com_m_audio_FWEngine2` → `m_audio_b_FWDiscreteEngine` **this is the FW1814's engine class**
- `m_audio_b_FWTimeSlaveEngine`, `m_audio_b_FWAggregateEngine` → specialized engines (used by other products / aggregate device)

**DCL / isoch stack:**
- `m_audio_b_FWDCLProgram` (27 methods — base: `BuildPacketDCLSegment`, `CreateIsochChannelAndPorts`, `SetupCIP`, `SetPacketSize`, `Start/Stop/ResetPort`, `IOSynchronize`)
- `m_audio_b_FWDCLInputProgram` → `m_audio_b_FWDCLProgram` (RX: `BuildReceivingDCLProgram`, `InputDCLCallbackCopyPacketGroupToSamples`, `InputDCLStaticOverrunCallback`, `ResyncInputBuffer`, `ProcessCompoundMIDIInputForPacket`)
- `m_audio_b_FWDCLOutputProgram` → `m_audio_b_FWDCLProgram` (TX: `BuildTransmittingDCLProgram`, `OutputDCLCallback`, `AdjustOutputDCL`, `ConvertTimestampNanosToAbsPacketIndex`, `ScheduleMidiPacket`)
- `m_audio_b_FWDCLBuilder` (15 methods — helper for layering DCLs into segments)
- `m_audio_b_FWIsochChannel`, `m_audio_b_FWRemoteIsochPort`, `m_audio_b_FWStream`, `m_audio_b_FWTPacketIndex`, `m_audio_b_FWKernelTimer`

**Comm:**
- `m_audio_b_FWHSCI` — standalone HSCI handler (`HSCIReadComplete`, `HSCITimerFired`, `Pause`, `Release`, `HSCISafelyComplete`)
- `m_audio_b_FWAsyncAVCCommand` — AV/C transport, FCP envelope
- `m_audio_b_FWAVCUnit`, `m_audio_b_FWConnectionManager`, `m_audio_b_FWAVCConnectionManager`, `FWP2PConnection`
- `m_audio_b_FWTTrio` — triple-buffer / tri-state helper (likely for queue coherence)

**User client:**
- `com_m_audio_FWUserClient` (34 methods — single class, shared across all 8 products — confirmation that the panel/prefpane/MIDI plugin open **one** userclient surface regardless of device)
- `m_audio_b_FWEngineUserClient` (per-engine audio userclient — IOAudioEngineUserClient subclass)
- `m_audio_b_FWKIPCUserClient` (24 methods — "KIPC" kernel-IPC userclient; MIDI plugin candidate)

**Property / settings tree:**
- `m_audio_b_FWBaseProp` → `m_audio_b_FWPropertyDict`, `m_audio_b_FWBaseArrayProp` / `m_audio_b_FWArrayProp`, `m_audio_b_FWNumberProp`, `m_audio_b_FW32BitNumberProp`, `m_audio_b_FW64BitNumberProp`, `m_audio_b_FWFunctionProp`, `m_audio_b_FWChannelGroupProperty`, `m_audio_FWb::BaseChannelGroup`, `m_audio_FWb::GroupChannelGroup`
- Per-settings leaves: `FWSettings`, `FWSettingsLevels`, `FWSettings1814Levels`, `FWSettings1814AuxSend`, `FWSettingsAuxSend`, `FWSettings410Levels`, `FWSettingsABSwitch`, `FWSettingsRotaryControl` — this is the on-device mixer-state model. The device class holds a `FWPropertyDict` of these.

**AppleFWAudio detach:**
- `com_m_audio_MakeAppleFWAudioGoAway` → ~`IOService` (minimal class; just matches to steal the match slot — same strategy the clone uses, confirmed appropriate for FWSolo only).

## Finding 3 — FWMetaNub owns the firmware upload path

`com_m_audio_FWMetaNub` has 43 methods including:
- `FirmwareStart`, `FirmwareHalt`, `FirmwareReset(uint)`, `FirmwareUpdate`
- `FirmwareUpdateBeBoB32`, `FirmwareUpdateBeBoB251` — two upload protocols; the FW1814 uses one of these.
- `DownloadImageStart`, `DownloadCNEStart`, `DownloadSectionStart(5 args)`, `DownloadSendNextBlock`, `DownloadStop`
- `FireWireReadCommand(FWAddressStruct&, uint, void*)`, `FireWireWriteCommand(...)`, `FirmwareWriteCommand(MAFireWireRequestHeader*, uint)` / overload `(uint, uint)`, `FirmwareReadResponse(MAFireWireResponseDevice*, uint)`
- `ReregisterDeviceAfterFirmwareReset`, `ReregisterDeviceTimerFired`, `RegregisterDeviceThread` *(sic)*, `ReplaceExistingMetaNub`
- `FirmwareCacheInfoRegisters(IOService*)`

**Consequences:**
- The reference kext uploads firmware **in-kext** from the nub; it does not require a separate userspace helper (our M2 `firmware-helper` LaunchDaemon). The 2014 driver's shipping topology was single-kext.
- `ReplaceExistingMetaNub` + the Rere/Regregister methods indicate the nub replaces itself across the bootloader→runtime Model_ID transition (0x10070 → 0x10071) by publishing a new nub rather than relying on IOKit re-matching. **This is the mechanism we need to understand before shipping firmware upload** — ours punts to a userspace daemon and the kext matches only the runtime Model_ID.
- Decision point for Part D: do we keep the clone's split-kext+daemon design, or mirror the single-kext firmware upload? The reference answer is clear; the deviation would need a "10.13.6 forced this" justification.

## Finding 4 — FW1814Device method inventory (48 methods)

Grouped by function:

**IOKit lifecycle / IOAudioDevice overrides:**
- CTOR `(OSMetaClass const*)` and default CTOR, DTOR, `stop(IOService*)`
- `GetIconName`, `GetDescriptionString`, `GetShortDescriptionString`, `GetControlPacketSize`
- `MetaClass::MetaClass()` and the usual metaclass boilerplate
- (No `start` / `probe` override seen in symbol table — inherited from `FWRev2AudioDevice` or `m_audio_b_FWAVCAudioDevice`.)

**Bus events:**
- `BusReset()`
- `ReconnectP2P()` — reconnect peer-to-peer plugs after bus reset. **This is the sequencing hook the clone's "stream kick" workaround is approximating.**

**Mixer / routing (control panel surface):**
- `SetInputPan(uint, uint)`
- `SetInputRouting(uint, uint)`
- `SetAuxMonitor(uint, uint)`
- `SetHeadphoneSelector(uint, uint)`
- `SourceMaskToHeadphoneSelector(uint)`
- `MapChannelAndFunctionBlock(FunctionBlockMapping&)`
- `MapRoutingParameters(Routing&)`
- `SelectFunctionBlock(uchar&, SelectMap1814*, uchar&)`
- `CustomMixerWriteWithRotatedAuxChannels(uint, uint, void*)`
- `ResendCurrentMixerSettings()`
- `ResetToFactorySettings()`

**Clock + digital-signal path:**
- `SetClockSourceInternal(uint, bool)` — per-device clock-source selection (Internal/S/PDIF/ADAT/Word-Clock lives here)
- `SetBlankSlateClockSource()` — called from `m_audio_b_FWAVCAudioDevice::SetBlankSlateClockSource` as a device-specific override; sets safe known clock state on startup
- `SwitchDigitalSignals(uint)` — the S/PDIF vs ADAT selector (correlates with FFADO byte 7+8 of the HSCI envelope)
- `AVCControlSetLEDStatus(uint)` — front-panel LED update

**HSCI / control packet:**
- `ReadHSCIData(uint*)`
- `ReceiveControlPacket(FWControlPacket*)` — generic inbound
- `ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*)` — **device-specific inbound struct**, named in the symbol itself. There is a `FW1814ControlPacketWithoutNodeID` struct (probably 11 bytes = HSCI envelope minus the FireWire NodeID). This resolves the ambiguity about which bytes of the HSCI envelope are payload vs addressing.
- `StartReceptionOfControlPackets()`, `StopReceptionOfControlPackets()`, `PauseReceptionOfControlPackets()`, `ResumeReceptionOfControlPackets()`

**Setup / engine wiring:**
- `SetupAudioEngine(m_audio_b_FWBaseEngine*)` — this is where the FW1814 configures its engine (channel counts, format advertisement, DCL program selection, latency table)
- `ResetRearrangementProcs(m_audio_b_FWBaseEngine*)` — channel-rearrangement function pointers; relates to AM824 ↔ CoreAudio layout
- `WorkaroundFor1814InputDelay(uint, int*, int*, void*)` — **named workaround** for input delay specific to FW1814. The exact behavior is Ghidra-queue priority.
- `InitializeUserSettableProperties()`
- `GetRoundTripLatencyForFDF(FDFValue)` — latency per rate (feeds the C6 budget)

**Static constant / vtable symbols:**
- `__ZN24com_m_audio_FW1814Device10gMetaClassE` at `0x4b990`
- `__ZN24com_m_audio_FW1814Device10superClassE` at `0x375d0` — read this pointer in Ghidra to confirm the direct parent
- `__ZTV24com_m_audio_FW1814Device` at `0x369b0`

## Finding 5 — Engine (`com_m_audio_FWEngine2`) override set

`com_m_audio_FWEngine2` overrides from `m_audio_b_FWBaseEngine`:

- `initHardware(IOService*)`, `stop(IOService*)`
- `StartAudio()`, `StopAudio()`, `StartPCMAudio()`, `StopIsochStreams()`, `StartMIDI()`, `StopMIDI()`
- `PerformFormatChange(IOAudioStream*, _IOAudioStreamFormat const*, _IOAudioSampleRate const*)`
- `PerformSampleRateChangeCore(_IOAudioSampleRate const*)`
- **AC3 path:** `ActivateAC3Output`, `DeactivateAC3Output`, `StartAC3Output`, `SetAC3OutputState(bool)`, `ClipAC3Output(float*, int*, uint, uint, _IOAudioStreamFormat const*)`
- **PCM clip path:** `ClipPCMOutput(float*, int*, uint, uint)`
- `ActivatePCMInput`, `ActivatePCMOutput`, `DeactivatePCMInput`, `DeactivatePCMOutput`
- `ConnectAllPlugs`, `DisconnectAllPlugs`, `ConnectAudioMIDIChannels`
- `LocalRearrangeSampleFrame16Bit2(ushort*, ushort*, int*, uint, uint, bool)` — AM824 rearrangement
- `UpdateDCLProgramsWithNewPacketSizes()`
- `RecalculateBandwidth()`
- `SetExternalSyncEnabled(bool)`

**Consequences for the clone:**
- Our clone merges the engine and clip paths into fewer methods. The reference's PCM/AC3 split (both activate/deactivate/start/stop paths exist independently) is the right paradigm — ClipAC3Output takes `_IOAudioStreamFormat const*` because AC3 and PCM share a stream.
- `ConnectAllPlugs` / `DisconnectAllPlugs` as single engine-level operations suggests the plug-connect ordering is centralized here, not scattered across device + engine. Our clone splits them.

## Finding 6 — DCL program interface is the authoritative shape

`m_audio_b_FWDCLProgram` + Input/Output subclasses give the shape every DCL ring must have:

- `Initialize(m_audio_b_FWAudioDevice*, FWDCLSharedSyncData*)` — construction requires both the device and a **shared sync struct**. There is a struct `FWDCLSharedSyncData` that TX and RX programs share — the central sync point we were missing.
- `SetupCIP(uint, SYTInterval, SYTIncrement, FDFValue)` — explicit CIP header setup with SYT interval/increment and FDF value as separate arguments.
- `BuildPacketDCLSegment(bool, m_audio_b_FWDCLBuilder*, uint, DCLLabelStruct*&, long, void(*)(DCLCommandStruct*), bool)` — segments are built by the `FWDCLBuilder` helper (not inline).
- `CreateIsochChannelAndPorts(bool, uint, uint)` — isoch-channel creation lives in the base class, not in the engine.
- `CalcIsochPacketHeaders(uint*, uint, uint, bool)` — packet-header precomputation.
- `RebuildDCLProgram()`, `RebuildDCLProgramCore()` (per subclass) — rebuild path for format/rate changes.
- Input callbacks: `InputDCLStaticCallback`, `InputDCLStaticOverrunCallback` (both static trampolines) → `InputDCLCallbackCopyPacketGroupToSamples`, `InputDCLCallbackOverrun` (member handlers).
- Output callbacks: `OutputDCLStaticCallback` → `OutputDCLCallback(long, bool)`.

**The clone's DCL callback plumbing diverges from this.** Our RX never calls back. Comparing our FW1814DCLInputProgram against `BuildReceivingDCLProgram` + `InputDCLCallbackCopyPacketGroupToSamples` in Ghidra is the single highest-value next step on the C2 sync path.

## Finding 7 — HSCI is a dedicated class, not a FW1814 inlined helper

`m_audio_b_FWHSCI` (19 methods — not all enumerated above):
- `HSCIReadComplete(int)`, `HSCITimerFired()`, `HSCISafelyComplete()`, `Pause()`, `Release()`
- Full method dump pending Ghidra pass (nm shows truncation at HSCI partway through output).

**Consequences:**
- HSCI is a reusable class across all BeBoB products. The byte envelope is the same; the per-device variation is in the inbound `ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*)` interpretation.
- Our clone embeds HSCI handling into `FW1814Device` directly. Mirroring the reference means pulling it out into a shared transport class — a Part D decision.

## Finding 8 — Settings/property model is the control-panel ABI

Settings leaves found in vtables (each is probably a serialized key in the device's `FWPropertyDict`):

- `FWSettings` (10-slot vtable — base)
- `FWSettingsLevels` (16 methods)
- `FWSettings1814Levels` — FW1814-specific levels
- `FWSettings1814AuxSend` — FW1814 aux send
- `FWSettingsAuxSend`, `FWSettings410Levels`, `FWSettingsABSwitch`, `FWSettingsRotaryControl`

**Consequence:** the control panel's wire format with the kext is defined by the shape of these settings classes — NOT by an ad-hoc userclient selector table. Part C5 (userclient surface) must enumerate these classes before claiming to have the userclient ABI.

## Open questions / Ghidra queue

In priority order (each contributes to a specific Part C section):

1. **(C2 / sync)** Decompile `m_audio_b_FWDCLInputProgram::BuildReceivingDCLProgram` + `InputDCLCallbackCopyPacketGroupToSamples` + `ResyncInputBuffer`. Compare segment layout against the authoritative decode in auto-memory (64 single-pkt segs × 64 jumps — **Correction 2026-10-14:** 64 segs × 4 pkts, per Q-DCL). Answer: why do reference RX callbacks fire where ours don't?
2. **(C2)** Decompile `m_audio_b_FWDCLProgram::SetupCIP` + `CalcIsochPacketHeaders`. Confirm SYT interval table and FDF-table ground truth.
3. **(C4 / transport)** Decompile `com_m_audio_FW1814Device::SetupAudioEngine` and `com_m_audio_FWEngine2::ConnectAllPlugs`. Answer: exact plug-connect sequence vs the current clone's PCR writes.
4. **(C4)** Decompile `com_m_audio_FW1814Device::ReconnectP2P` and `com_m_audio_FW1814Device::BusReset`. Answer: whether the "stream kick" FCP re-assert is actually what the reference does on bus reset.
5. **(C2 / clock)** Decompile `com_m_audio_FW1814Device::SetClockSourceInternal` + `SetBlankSlateClockSource`. Answer: canonical clock-source enum values and the HSCI payload layout.
6. **(C3 / HSCI)** Full method list of `m_audio_b_FWHSCI` + decompile `HSCIReadComplete`, `HSCITimerFired`. Answer: exact 12-byte envelope semantics (currently a hypothesis in auto-memory).
7. **(C1 / lifecycle)** Decompile `com_m_audio_FWMetaNub::probe` + `start` + `ReplaceExistingMetaNub`. Answer: the bootloader↔runtime transition — is it in-kext or does it rely on IOKit re-match?
8. **(C1 / AppleFWAudio)** Resolve why the reference's plist does **not** list `MakeAppleFWAudioGoAway` for FW1814. Is the FWMetaNub-level match sufficient to pre-empt `AppleFWAudio`? If yes, our clone's extra personality is unnecessary; if no, FWSolo was special.
9. **(C4)** Decompile `com_m_audio_FW1814Device::WorkaroundFor1814InputDelay`. Named workaround → almost certainly relevant to current silence/sync symptoms.
10. **(C6 / latency)** Decompile `com_m_audio_FW1814Device::GetRoundTripLatencyForFDF`. Produce the per-rate latency table that C6 will anchor on.
11. **(C5 / userclient)** Decompile `com_m_audio_FWUserClient::externalMethod` dispatch. Produce the selector table.
12. **(C4 / AC3)** Decompile `com_m_audio_FWEngine2::ActivateAC3Output` and the AC3 clip path. Confirm whether FW1814 ships AC3 pass-through (settings tree suggests yes — `FWSettingsABSwitch` is a candidate).
