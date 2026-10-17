# Q-MAMAP — MA* property key crosswalk for the FW1814 (key → registration → setter → wire → default)

> **Correction (2026-10-14) — SL capture files** Corroboration from rig capture 62 (`pulled/62-dtrace-mixer-slide.txt`): a Panel fader move writes `MASWReturnLevels` as an array of signed 32-bit integers `[0x0, 0x0, 0xffffffff, 0xffffffff]` (0 and -1), i.e. the property level domain is <= 0 as in the group range [-255, 0]; capture 63 shows `MAMomentarySwitchSource` written as `0x1` / `0x0`.


> **Correction (2026-10-16, conformance check):** row 22 `MAMonitorAuxOnOutput` is an array of **3** elements, not 10: `FW1814Device::InitializeUserSettableProperties` calls `FWSettingsAuxSend::Init(..., (Output.Count() >> 1) + 1)` with the `<4>` Output group (decomp `FW1814Device__InitializeUserSettableProperties.c:31-35`), i.e. output pair 1, pair 2 and the S/PDIF pair. The clone already used 3; the panel catalog and notes that said 10 were wrong (the panel translator's bound of 10 pairs is panel-side).

**Status:** done 2026-10-14 (closes matrix item `A1-Q3b.2`; replaces the 'Q-MAMAP (46 rows)' placeholder in NEXT_STEPS).
**Bundle:** v1.11.2 `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` (class `com_m_audio_FW1814Device`, superclass `FWRev1AudioDevice`). **Evidence:** Ghidra headless decompile `ghidra-exports/decomp/qmamap.log` + `qmamap2.log` (script `scripts/DecompQMamap.java`, addresses in the headers below), `mafw.asm` for PMF/vtable resolution, vtables resolved with the R2 method (`otool -rv` + `__DATA,__const`). Third-party (FFADO `src/bebob/maudio/special_mixer.cpp:220-236`, register map) is used **only** to corroborate register offsets, never as the source of a claim.

## 1. Mechanism (all observed in the decompile)

1. `FW1814Device::InitializeUserSettableProperties @0xe6ca` calls the base (`FWAudioDevice @0x252a`, `FWAVCAudioDevice`) then builds six channel groups and four settings objects:

   | Group (device offset) | Template size | Used as | Batch sub-index = register offset |
   |---|---|---|---|
   | `[+0x1668]` | `<4>` | SW return (`SWReturn`) | `0x00` |
   | `[+0x1318]` | `<4>` | Output (`Output`) | `0x08` |
   | `[+0x13e0]` | `<18>` | Input (`Input`) | `0x10` |
   | `[+0x1730]` | `<4>` | Headphone (`Headphone`) | `0x38` |
   | `[+0x1b00]` | `<2>` | Aux output (`AuxOutput`) | `0x34` |
   | `[+0x17f8]` | `<22>` | Aux send (`AuxSend`) | `0x64` (`[+0x1838]=100`, via `CustomMixerWriteWithRotatedAuxChannels`) |

   Group template sizes are read from the embedded-struct constructor `FW1814Device::$_667::$_667 @0xd44b` (struct at device `+0x1318`): `ConcreteChannelGroup<4>` constructed explicitly at struct `+0x0`, `+0x350`, `+0x418`; vtable stores at `+0xc8` = `<18>`, `+0x4e0` = `<22>`, `+0x7e8` = `<2>` (0xd48f, 0xd539, 0xd5d0).

   Objects: `FWSettingsRotaryControl::Init`(this+0x470), `FWSettingsAuxSend::Init`(+0xf98), `FWSettingsLevels::Init`(+0x538), `FWSettingsABSwitch::Init`(+0x2d8). The sub-indices are passed to `BaseChannelGroup::SetupBatchChangeMethod @0xe184`, which stores `[group+0x40]=subIndex` and `[group+0x44]=0x80` (multiplier) and installs the batch functor at `[group+0x38]`. The sub-indices equal the byte offsets of FFADO's register blocks (`Stream in 0x00/0x04`, `Analog out 0x08/0x0c`, `Inputs 0x10..0x30`, `AUX out 0x34`, `HP 0x38/0x3c`, `Aux inputs 0x64..0x8c`) — an independent corroboration.
2. Property registration: `FWSettingsLevels::InitGroupProp @0x21444` registers `MA<P>Levels` (kind 0), `MA<P>Mute` (kind 8), `MA<P>Solo` (kind 0x10, only when the group has a solo partner) and `MA<P>Link` (kind 0x18) through `FWChannelGroupProperty::init @0x93a4` (stores group `[+0x60]`, kind `[+0x68]`, partner `[+0x70]`). Each group's valid level range is set to **min `0xffffff01` (-255), max 0** (`[group+0x14]`, `[group+0x18]`).
3. Write path for a property write: `IORegistryEntrySetCFProperties` → `FWPropertyDict::SetOneProperty @0x9f74` → `ModifyProperty` (queues) → `checkForWork` → `ApplyModifiedProperties @0x9b40` → `FWChannelGroupProperty::CallSetter @0x940e` (`OSArrayToSeriatimValues`, then `group->vt[0x28]` = `SendLevelsToDevice`) or the property's own setter.
4. **Level encoding on the wire** (`BaseChannelGroup::SendLevelsToDevice @0x1ef64` with a batch functor): per channel `v = AdjustedLevelForChannel(...)` (clamped to [-255, 0]) → `int16 = v * [group+0x44] (=0x80)` (`AdjustedAndMultipliedLevels @0x1f246`) → **byte-swapped to big-endian** (`<<8 | >>8`) → `nQuads = nChannels/2`, two channels per quadlet in channel order → `functor(subIndex, nQuads, buf)` → `CustomMixerWriteQuads @0x22c7a`: async block write to node address `0xFFC7:(offset + 0x00700000)`, `nQuads*4` bytes (`local_36 = 0xffc7; local_34 = param_1 + 0x700000`, then controller `vt[0x890]` createWrite). Unit: property level 1 = 0.5 dB; `-255*0x80 = -32640` (`0x8080`), `0` = 0 dB (AV/C 1/256-dB units).
5. `AdjustedLevelForChannel @0x1ee66`: returns the group minimum (-255) if (global mute flag `[group+0x11]`) **or** (some channel in the group is soloed and this channel is not); otherwise `clamp(ch.level[+0] + ch.balance[+4], min, max)` unless the channel's mute `[+8]` is set (then minimum); if the dim flag `[group+0x10]` is set the result is reduced by **20 units (−10 dB)** (floored at min). **Mute and solo have no register of their own**: they are realised as level = minimum on the wire.

## 2. Crosswalk (FW1814)

Legend — *Reg*: vendor register window `0xFFC7:0x00700000+offset`; *AV/C*: AV/C command to the unit; *RAM*: stored/echoed only. "Default" = factory reset (`FW1814Device::ResetToFactorySettings @0xd190` → `FWSettingsLevels/AuxSend/RotaryControl/ABSwitch::ResetToFactorySettings`), it also sets `fMode [+0x144] = 0x40000003`.

| # | Key | Registered by (fn @addr) | Value | Setter / effect | Wire | Default |
|---|---|---|---|---|---|---|
| 1-4 | `MASWReturnLevels/Mute/Solo/Link` | `InitGroupProp("SWReturn")` @0x21444 | int array (4 ch) / bool arrays | group send | Reg 0x00 (2 quads, rule §1.4-5) | level 0, mute/solo/link off (`SetAll*` @0x215b0..0x216d6 on the group) |
| 5-8 | `MAOutputLevels/Mute/Solo/Link` | `InitGroupProp("Output")` | (4 ch) | group send | Reg 0x08 (2 quads) | as above |
| 9-12 | `MAInputLevels/Mute/Solo/Link` | `InitGroupProp("Input")` | (18 ch) | group send | Reg 0x10 (9 quads) | as above |
| 13-15 | `MAHeadphoneLevels/Mute/Link` (no Solo) | `InitGroupProp("Headphone")` | (4 ch) | group send | Reg 0x38 (2 quads) | as above |
| 16 | `MAAuxSendLevels` | `FWSettingsAuxSend::Init @0x20854` | int array (22) | batch rotated functor `CustomMixerWriteWithRotatedAuxChannels @0xe982` | Reg 0x64 (11 quads; 44 B `{in[9],in[10],in[0..8]}`, R14) | **-255** each (`AuxSend::ResetToFactorySettings @0x20b9c`) |
| 17-20 | `MAAuxOutputLevels/Mute/Link` (+ `MAAuxOutputPan2` below) | `AuxSend::Init` | (2 ch) | group send | Reg 0x34 (1 quad) | level 0 (group), mute/link off |
| 21 | `MAAuxOutputPan2` | `AuxSend::Init` (PMF `SetAuxPan @0x20d78`, non-virtual) | int array (2) | `SetAuxOutChannelFeature(ch, selector 3, -value)` → `AVCControlFeatureFunctionBlock(..., value<<7)` | **AV/C** Feature function block (selector 3) | `{-255, +255}` (`[0]=0xffffff01,[1]=0xff`) |
| 22 | `MAMonitorAuxOnOutput` | `AuxSend::Init` (virtual PMF vt+0x30 → `FWSettings1814AuxSend::SetAuxMonitor @0xd0b4` → device `vt[0xbc8]` = `FW1814Device::SetAuxMonitor @0xcfc4`) | array (10) of 0/1 | sets bit `(n-1)` (byte-swapped) in image `[+0x1314]` | **Reg 0x9c** (1 quad) | 0 (`Fill(0)`) |
| 23 | `MAInputPan2` | `FWSettingsLevels::Init` (PMF vt+0x58 → `FWSettings1814Levels::SetInputPan @0xd128` → device `vt[0xbc0]` = `FW1814Device::SetInputPan @0xdd3e`) | int array (18) | 16-bit per channel: `((−p>>1)&0xff) + p·(−0x8000)` into image at `[+0x12b6/0x12b8 + 2(ch−1)]` | **Reg 0x40 + 4·((ch−1)/2)** (1 quad per channel pair; half-word byte order on the wire **not derived**) | pairs `{-255, +255}` |
| 24 | `MAInputRoutingV2` | `Levels::Init` (PMF vt+0x68 → `FWSettings1814Levels::SetInputRouting @0xd15c` → device `vt[0xbb8]` = `FW1814Device::SetInputRouting @0xdbea`) | array (9) of 2-bit masks | bit-set/clear in big-endian image `[+0x1308]`: pairs 1-4 → bits `n-1` and `n+3`; pair 5 → `0x100`/`0x200`; pairs 6-9 → bits `n+2` and `n+6` | **Reg 0x90** (1 quad) | 0 |
| 25 | `MASWReturnRoutingV2` | `Levels::Init` (PMF vt+0x70 → base `FWSettingsLevels::SetSWReturnRouting @0x218a6` → `vt[0x60]` = `SetAVCRouting @0x225b4`) | array (2) of bitmasks | per bit: `AVCMixerConnectPair(connect?, ...)` | **AV/C** mixer connect (FB select via device `vt[0xb40]`) | element i = `1<<i` |
| 26 | `MADirectMonitoringActive` | `Levels::Init` (`SetDirectMonitoringActive @0x217ec`) | u32 | **ignores the value**: sets `[+0x11c]=1`, clears the Input group's mute flag `[+0x11]`, resends input levels | Reg 0x10 (via group) | `[+0x11c]=1` |
| 27 | `MAGlobalDim` | `Levels::Init` (`SetGlobalDim @0x21848`) | u32 bool | sets Output group dim flag `[+0x10]`, resends | Reg 0x08 | 0 |
| 28 | `MAGlobalMute` | `Levels::Init` (`SetGlobalMute @0x21818`) | u32 bool | sets Output group mute flag `[+0x11]`, resends | Reg 0x08 | 0 |
| 29 | `MARotaryControlV2` | `RotaryControl::Init @0x20ed2` (setter `NullSetProperty64`) | u64 mask | stored only; masked pairs follow the knob | RAM | `0xFFFF00000000` (`ResetToFactorySettings @0x21060`) |
| 30 | `MAMetaVirtualRotateDataKnob` | `RotaryControl::Init` (`VirtualRotateKnob @0x213f8`) | u32 steps | `UserRotatedHardwareKnob(steps)` → adjusts `MASWReturnLevels` (from 0), `MAOutputLevels` (0x20), `MAInputLevels` (0x10), `MAHeadphoneLevels` (0x38), `MAAuxOutputLevels` (0x32) for masked pairs (`SetLevelsFromKnob @0x21284`) | Reg (via levels) | — |
| 31 | `MAMomentarySwitchRoutings` | `ABSwitch::Init @0x1e00a` (`SetMomentarySwitchRoutingSource @0x1e34c`) | array (4) | if index == current mode: `fHeadphoneSource := value`, submit | RAM → Reg 0x98 | `{1,2,3,4}` |
| 32 | `MAHeadphoneSourceV2` | `ABSwitch::Init` (`SetHeadphoneSource @0x1de2c`; 1814 functor `[+0x300]` = `FW1814Device::SetHeadphoneSelector @0xd1f8`) | u32 | sets hp-switch flag `[+0x104]=1`, LEDs, calls selector | **Reg 0x98** (image `[+0x1310]`: bits 8/9/10 or 24/25/26 by A/B position and phone index) | 1 (`[+0x144]`) |
| 33 | `MAHeadphoneSecondSource` | `ABSwitch::Init` (`SetHeadphoneSecondSource @0x1ddfe`) | u32 | selector for position 1 | Reg 0x98 | 1 (`[+0x184]`) |
| 34 | `MAMomentarySwitchSource` | `ABSwitch::Init` (`SetMomentarySwitchSource @0x1e53e` → `SendMixerSettingsForCurrentMomentaryMode @0x1e36a`) | u32 | mode 0: HP source := routings[mode]; else rewrites `MAInputRoutingV2[0] &= ~1 / |= 1` and `[1] &= ~0x12 / |= 0x12` | Reg 0x90/0x98 | — |
| 35 | `MAMomentarySwitchLatent` | `ABSwitch::Init` (`NullSetProperty32`, read-mostly) | u32 | current momentary mode `[+0xd0]` (toggled by the front-panel A/B press, `MomentarySwitchPressed @0x1e54c`) | RAM | 0 |
| 36 | `MAEnableStereoPairEngines` | `FWAudioDevice::InitializeUserSettableProperties @0x252a` (`SetExportStereoPairEngines`) | u32 | engine export | — | 0 (SL prefs) |
| 37 | `MAEnableAggregateEngine` | same (`SetExportAggregateEngine`) | u32 | engine export | — | — |
| 38 | `MAMetaResetToFactorySettings` | same (function prop `ResetMixer @0x2660`) | any | `ResetToFactorySettings` of all objects + `SendToDevice` | Reg (all blocks) + AV/C | — |
| 39 | `MADeviceUserLabel` | same (`SetDeviceUserLabel`) | string | label | — | — |
| 40 | `MAClockSource` | `FWAVCAudioDevice::InitializeUserSettableProperties` (`EstablishClockSourceControl`) | u32 composite | `SetClockSourceInternal @0xe25c` | HSCI 16-byte frame (Q5) | `0x40000003` |
| 41 | `MASampleRate` | fast-path in `FWAVCAudioDevice::setProperties` (Q15b) | u32 Hz | `PerformFormatChange` chain (Q15e) | none (engine-side) | — |
| 42 | `MARateLocked` | published by `DetectExternalSyncChanges` / `initHardware` | u32 bool (RO) | — | RAM | — |
| 43 | `MAMetaAllProperties` | `FWPropertyDict::CopyProperty @0xa1e0` special case → `CopyAllProperties` | dict (RO) | seeds Panel cache and helper seed | — | — |

Published read-only descriptors (not settable): `MAActualProductName` (`FWMetaNub::start`), `MAIconName`, `MAMainEngineUniqueID` (`createAudioEngine`), `MAMaxBytesPerCycle` (`initHardware`), `MABytesPerCycle` (`FWEngine2::RecalculateBandwidth`), `MAMIDIInputPorts/OutputPorts` (`SetMIDIPortProperties`, default names `"1"`,`"2"`…), `MADeviceInitialized` (`FWUserClient::SetInitializedProperty`), `MAFirmwareInfo` (80 B). Firmware write keys (`FWMetaNub`): `MAFirmwareHalt`, `MAFirmwareStart`, `MAFirmwareUpgradeImage`.

**Not registered by the FW1814 kext** (registered only by other products): `MASPDIFMixerBypass` (FW Solo init), `MASPDIFMirrorToAnalog12` and `MAChannelOrderSPDIFFirst` (Lightbridge init). The panel's shared translator knows `MASPDIFMixerBypass` (cookie `0x612`) but the FW1814 kext does not register it, so a write would only be stored/ignored by the generic dictionary (**unverified which**).

## 3. What this changes

* **'One mixer path' (C-docs) is wrong for the FW1814**: levels, input pan, input routing, headphone selector, aux monitor use the vendor register window; but **`MAAuxOutputPan2` and `MASWReturnRoutingV2` go over AV/C** (Feature function block / mixer connect) via the base `FWSettingsLevels`/`FWSettingsAuxSend` setters the 1814 does not override. The clone needs both channels.
* Mute/solo/dim are *level* operations (minimum / −10 dB) — there is no mute register.
* Level range is [-255, 0] half-dB steps; wire value is that ×128 as signed 16-bit big-endian.
* `MADirectMonitoringActive` is write-ignoring.

## 4. Not derived / unverified (labelled)

* Meaning of each routing bit in registers `0x90`/`0x98`/`0x9c` beyond the bit positions above (**needs** FFADO/Rust cross-read or a live write capture).
* Wire byte order of the `MAInputPan2` half-words (memory image is sent raw; host is little-endian).
* AV/C selector semantics for `AVCControlFeatureFunctionBlock` (selector 2 = volume, 3 = balance assumed from call sites) and the exact AV/C `AVCMixerConnectPair` frame.
* `Link` semantics beyond storage (no use in `AdjustedLevelForChannel`).
* That the aux-send rotation's `in[9..10]` are the stream returns (R14, inferred).
* `MAGlobalMute/Dim` flags apply only to the Output group (`[+0x8b8]`); whether the combined `GroupChannelGroup` (0x930) propagates them to aux/headphone was not decoded.
* Defaults for `MAMomentarySwitchSource` and `MAEnableAggregateEngine`/`MADeviceUserLabel` were not located.
