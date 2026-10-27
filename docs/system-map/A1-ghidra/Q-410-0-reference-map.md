# Q-410-0 — FireWire 410 reference map, desk pass 1 (2026-10-26)

> **Update (2026-10-26, R135):** §4 corrected from the asm (the decompile merged `clk` and `blank`); the two sample procs decoded (§2); live confirmations from capture 103 (stereo pairs, engine UID, 2/8 channels at 176.4/192 kHz, every control's key) in §9. Panel side: `A2-panel/PN410-control-key-map.md`.

Bundle `_extracted/` v1.11.2 kext (x86_64 slice). Sources: existing decompiles `ghidra-exports/decomp/FW410Device__*.c`, `FWSettings410Levels__*.c` (42 files, already exported; nothing re-decompiled), `otool-exports/mafw.asm`, bytes read from the slice (`__TEXT,__const` addr 0x2f700 = file 194304; `__DATA,__data` addr 0x4aaa0 = file 305824). Hardware: SL rig capture 102 (baseline ioreg, v1.9). Everything below is the reference unless marked **unverified**.

## 1. Identity and attach
* Class `com_m_audio_FW410Device : FWRev2AudioDevice` (vtable `0x3c600`; metaclass `0x4bbd0`, super `FWRev2AudioDevice::gMetaClass`, vt slot 0xc00). 48 methods in the symbol table.
* Plist personality `com_m_audio_FW410Device`: `IOProviderClass com_m_audio_FWMetaNub`, `FireWire Product Name "FW 410"`, `IOUserClientClass com_m_audio_FWUserClient` (`_extracted/kext/…/Info.plist`). Matches by product name, like the 1814/Audiophile.
* Names: `GetDescriptionString` "FireWire 410", `GetShortDescriptionString` "FW410", `GetIconName` "FW 410 box.png".
* **Live (capture 102, SL v1.9, GUID `0xD6C010006B8C0`):** device `Vendor_ID 2037` (0x7F5, BridgeCo), unit `Vendor_ID 3436` (0xD6C), `Model_ID 65606` (0x10046), unit SW version 0x14001, spec 0xA02D, 400 Mb/s. Tree: `FWMetaNub → FW410Device → FWAVCUnit + FWEngine2 (2 FWStream) + FWUserClient`. Not a problem for the clone: `FWMetaNub::probe` (ref @0x1ce62, clone `FWMetaNub.cpp:114-119`) matches the vendor **name**, and the 410 reports `FireWire Vendor Name = "M-AUDIO"` (capture 102); its `kDeviceSpecs` row `{"FW 410", "FW Bootloader"}` already exists (`FWMetaNub.cpp:45`). Bootloader Model_ID 0x10058 (snd-bebob `bebob.c:446`) **unverified** on this unit.

## 2. Engine, channels, rates
* `SetupAudioEngine` (decomp): MIDI 1 in / 1 out; `[+0x2e4]` in 4, `[+0x2e0]` out 10, `[+0x2e8]` midi 1, `[+0x2f0]` inStream 4, `[+0x2ec]` outStream 10; procs `[+0x300] _RearrangeFrameLLRR10WithMIDI` (0x2b951), `[+0x308] _SamplePeaks10` (0x280d3), `[+0x310] _ConvertLLRR4SkipMIDI1` (+ `[+0x318]` = this); flags `[+0x2d6]` = 1, `[+0x34a]` = 1, `[+0x34b]` = 1 (asm 0x15526/0x1552d).
  * `[+0x34a]`: `FWBaseEngine::getGlobalUniqueID @0x5b84` uses the class string `"com_m_audio_FWEngine1"` instead of the real class name when set (asm 0x5b95-0x5ba8): the 410's engine UID keeps an older name. **Not in the clone base yet.**
  * `[+0x34b]`: written only; no byte, word or dword access at displacement 0x348-0x34b reads it in mafw.asm (2026-10-26: only `[+0x348]` and `[+0x34a]` are read). **No reader in v1.11.2** — the clone stores it and does nothing with it.
  * `[+0x2d6]` = clone `fRxRefreshAtHighRates` (exists).
  * `_RearrangeFrameLLRR10WithMIDI @0x2b951` (asm, mafw.asm lines 51942-52130): host stride 10, wire stride 11 (MIDI quadlet not written); wire[k] = `FWPackAM824(host[p[k]])`, p = 8 0 2 4 6 9 1 3 5 7 (wire = S/PDIF L, L1-L4, S/PDIF R, R1-R4); same `decl/incq` count as `_RearrangeSampleFrameLLRR6` (frames = 0 must be guarded). **Not in clone `FWSamples` yet.**
  * `_SamplePeaks10 @0x280d3` (decomp): 10 channels, identity map, groups of 8 frames = `FWPeaksMax(src, dst, frames, 10, identity)`. **Not in clone yet** (one line).
* `GetDeviceChannelsForSampleRate` (asm 0x14010/0x14017, lea targets computed from the bytes): rate > 96000 → `sDeviceChannels192 @0x4ad40`, else `sDeviceChannels @0x4ae20` (0x4ac60 is the FW1814's runtime copy, not the 410's).
  * `@0x4ae20`: in 4 / inStream 4 / out 10 / outStream 10 / midi 1, `+0x18..0x28` = 0, rates `0x2fa10` both; groups `{1,1,1,0}` in pair 1 analog, `{1,2,2,1}` in pair 2 S/PDIF, `{0,1,4,0}` out pairs 1-4 analog, `{0,5,5,1}` out pair 5 S/PDIF, `{0,0,0,3}`.
  * `@0x4ad40` (176.4/192): in 2 / 2, out 8 / 8, midi 1, `+0x18 = 0x2f9e0`, `+0x20 = 0x2f9f0`, `+0x28 = 0`; same group list as above. The `+0x18/+0x20` fields are **pointers** (clone names them `clipOffset`; `FWBaseEngine::ConvertInputSampleData` only ports the "0" branch and `FWEngine2::ClipPCMOutput` returns early when `[+0x328]` != 0). Bytes: `0x2f9e0` = `00 00 00 00 01 00 00 00 …`, `0x2f9f0` = `00 00 00 00 04 00 00 00 01 00 00 00 05 00 00 00 02 00 00 00 06 00 00 00 03 00 00 00 07 00 00 00` (read as UInt32: {0,4,1,5,2,6,3,7}). **Decoded 2026-10-26 (R136):** `+0x18` → engine `[+0x320]` = input map, `+0x20` → `[+0x328]` = output map, `+0x28` → `[+0x330]` = output copy list. `ConvertInputSampleData @0x4624`: with `[+0x320]` set it calls `ConvertSampleFramesFromInput(src, dst, in [+0x2e4], inStream [+0x2f0], frames, map, midi [+0x2e8] != 0) @0x27bb6`: per frame, wire slot k → host channel `map[k]` (skipped if ≥ inStream), wire stride in + midi; `0x2f9e0` = {0, 1} (identity for the 2 inputs). `ClipPCMOutput` (asm 0x11e41-0x11eb4): with `[+0x328]` set it calls `RemapSampleFramesForOutput(host, wire, out [+0x2e0], outStream [+0x2ec], frames, map [+0x328], copy [+0x330], midi) @0x27aad` instead of the `[+0x300]` rearrange proc: per frame host h → wire `map[h]` (skipped if ≥ out + midi), AM824 pack as `FWPackAM824`, then copy pairs (a → b, −1-terminated) inside the frame; host stride outStream, wire stride out + midi. `0x2f9f0` = {0,4,1,5,2,6,3,7} → wire = L1-L4, R1-R4, MIDI (9 quadlets); no copy list. Live (capture 105): at 192 kHz every analog output 1-8 streams. The FW1814's 192 kHz row (`0x4ab80`) has no maps (2/4 through its normal procs).
* Rates `kSupportedInputRatesArray @0x2fa10`: 44.1, 48, 88.2, 96, 176.4, 192 kHz (in = out).
* `GetRoundTripLatencyForFDF`: FDF 2 → 0xa0, 3 → 0xfe, 4 → 0xf5, 5 → 0x1b8, 6 → 0x1e0, else 0x96.
* AC-3: `IndexForAC3OutputChannels` = 0 / 5; `AC3Activated` empty.

## 3. Status packet
* `GetControlPacketSize` 0x4c (76 bytes); `NeedsControlPacketsForReadingHardwareControls` 1; `GetFirstSWReturnPeakMeterIndex` 4.
* `ReadHSCIData` (decomp): meter slots 0-3 ← packet q0-q3 (byte-swapped 16-bit peak, `(q & 0xff00)*0x100 + q*0x1000000 >> 16`); slots 4-13 ← host SW-return peaks (`[+0x110]`+0x10…0x34); slots 14-27 ← q4-q17; q18 = status: `DetectExternalSyncChanges(bit 24, (>>16) & 7)`, encoder byte `(>>8) & 0xff` → `TRotaryControl::RotaryValueChanged` (`+0x2d0`) → `FWSettingsRotaryControl::UserRotatedHardwareKnob` (`+0x2d8`). One encoder, no A/B button, no headphone encoder (unlike the Audiophile).

## 4. Clock
`SetClockSourceInternal @0x14eda` (asm 0x14eda-0x14f56; the decompile merged the two arguments): pause the main engine (vt 0xa50); `clk <= 0x3fffffff` (gate bit 30 clear) → 0xE00002C2; `clk & 3 == 0` → `AVCSetupSyncStream(0, 1)` (internal); `clk & 3 == 1` → `AVCSetupSyncStream(4, 1)` if `clk & 4` (coax) else `AVCSetupSyncStream(3, 1)` (optical); `clk & 3` = 2 or 3 → 0xE00002C2; resume (vt 0xa58). `blank` is not read. Live (capture 103): coax = bit 2 set (`0x40000004`), optical = bit 2 clear (`0x40000000`), external = `0x40000005`. The Audiophile has coax only and always uses type 4.

## 5. Mixer — a REGISTER mixer on a Rev2 device (corrects plan v9 Appendix A row 0)
Every 410 mixer setter writes through vt `0xba8` = `FWAVCAudioDevice::CustomMixerWriteQuads @0x22c7a` (vtable read 2026-10-26), i.e. an async block write to `0xFFC7:0x00700000 + off`, the FW1814's mechanism — not AV/C function blocks. Offsets and shadow words (device object):

| Setter (vt) | Offset, quads | Shadow | Encoding |
|---|---|---|---|
| group batch writers (InitializeUserSettableProperties) | 0x00, 0x14, 0x28 | channel groups `+0x13d8` (SW return), `+0x1188` (output), `+0x1310` (input) | `SetupBatchChangeMethod` (as the 1814's) |
| `SetInputLevel` (0xbe8) | 0x28 (ch 1-2) / 0x2c (ch 3-4), 1 | `+0x1100` 4×UInt16 | `(v>>1)&0xff + v<<15` |
| `SetInputPan` (0xbd0) | 0x38 + (ch-1)*2… , 1 | `+0x1110` 4×UInt16 | `(-v>>1)&0xff + v*-0x8000` |
| `SetInputRouting` (0xbc8) | 0x5c, 2 | `+0x1134`, `+0x1138` | bit masks per pair (decomp) |
| `SetSWReturnRouting` (0xbd8) + `BatchSetSWReturnRouting` (0xbe0) | 0x64, 1 | `+0x113c` | bits for pairs 1-5 (decomp); batch writes the word |
| `SetHeadphoneSource` | 0x68, 1 | `+0x1140` | bits 24-29 from source mask; key `MAHeadphoneSourceV2`, stored `+0x1174` (factory 1) |
| init tail | 0x28, 2 | `+0x1100` | input levels |

* Settings objects: Rotary `+0x2d8` (with `TRotaryControl +0x2d0`), AuxSend `+0x3a0` (aux group `+0x15e8`, aux out `+0x17f0`, monitor switches = output count/2 + 1), `FWSettings410Levels +0x678` (subclass of `FWSettingsLevels`; its SetInputPan/SetInputRouting/SetSWReturnRouting/BatchSetSWReturnRouting forward to the device's vt 0xbd0/0xbc8/0xbd8/0xbe0; `Init` installs a functor tag 0x79; `SendToDevice` = base + vt[0x78]). Groups: headphone `+0x1560`.
* `MapChannelAndFunctionBlock @0x14600` still exists (kinds 0-7; aux-send map `@0x2fa50` = `{1-2 → 7}, {3-4 → 8}, {5-12 → 5}, {13-14 → 6}`; outputs 1-10 → fb 0x0a…0x0e; SW return 1-8 → 1, 9-10 → 2; clock kind 4 by `bits & 7`; inputs 1-2 → 3, 3-4 → 4; aux out → 9; headphone 1 → 0xf). **Open:** which kinds still go out as AV/C on the 410 (aux sends? outputs?) and which only through the register writers — settle from the `FWSettings*` base paths with the 410's batch writers installed, then by a DTrace of `setProperties` + FCP/block writes on the SL rig.
* `MapRoutingParameters @0x1428a`: base (via Rev2 vtable slot) then 410 fix-ups (kind 1 pair 5 → `[+0xd]`=1; kind 0: `[+0xd]` = 1 for pair 1 else 2, pair := 1).
* `ResendCurrentMixerSettings`: base, Rotary, AuxSend, 410Levels, `SetHeadphoneSource(+0x1174)`. `ResetToFactorySettings`: base, Rotary, AuxSend, Levels, headphone source = 1.

## 6. Stereo-pair engines (user, 2026-10-26: the panel checkbox splits the 10 outputs into 5 stereo CoreAudio devices)
Generic, already in the clone base (`FWAudioDevice::CreateStereoPairSubEngines`, `FWPairEngine`, Q-MJ, R49), driven by the `DeviceChannels` group list → for the 410: Analog 1/2, 3/4, 5/6, 7/8, S/PDIF out; Analog 1/2 in, S/PDIF in. Never run on hardware (CLAUDE.md "on demand"). Panel: key `MAEnableStereoPairEngines` defined in `MaKeys.swift`, **no control yet**. Official panel: cookie `0x614` from the Hardware view (`CHardwareView`, P8); P8 labels that checkbox "ASIO-DM" — **likely a mislabel**, to be checked on the SL panel (screenshot) and corrected in P8.

## 7. Base gaps found (to plan, not code)
`[+0x34a]` UID name; `_RearrangeFrameLLRR10WithMIDI`, `_SamplePeaks10`; the 192 kHz remap branch (`+0x18/+0x20` tables); panel control for pair engines; `MAHeadphoneSourceV2`.

## 8. Panel: the 410 has its own UI classes (user, 2026-10-26: each interface has its own custom control panel)
Official panel `_extracted/panel/M-Audio FireWire.app/Contents/MacOS/Panel` (i386 Carbon), symbol counts: `CFW410DeviceSettings` 35, `CFW410HardwareView` 33, `CFW410MixerView` 19, `CFW410OutputView` 19, `CFW410MiniMetersView` 19, `CFW410GlobalSettingsView` 17 (incl. `LoadControllerAssignBtns` / `MapAssignButtonToSetting`: the front-panel encoder assignment), `CFW410HPOutChannelPairView` 16, `CFW410MixerChannelPairView` 16, `CFW410SpdifOutChannelPairView` 16, `CX410Device` 13; resources `FW410*Logo.pct`, `FW410Unit{On,Off}{,Hi}Btn.psd`, `410S.icns`. Next (same method as the Audiophile's `PNAP`): decompile these classes, build `PN410-control-key-map.md` (control → MA* key / cookie / range), screenshots of every tab on the SL panel, then design the clone's own 410 layout in our panel style.

## 9. Live confirmations (capture 103, SL v1.9, 2026-10-26)
* 4 in / 10 out at 44.1-96 kHz, **2 in / 8 out at 176.4/192 kHz** (CoreAudio); MABytesPerCycle 528 / 1040 / 1552; latencies in the capture README.
* Engine UID `com_m_audio_FWEngine1:<GUID>` (the `[+0x34a]` flag).
* Stereo pairs: 5 pair engines (Analog 1/2 in+out, S/PDIF 1/2 in+out, Analog 3/4 / 5/6 / 7/8 out), UIDs `m_audio_b_FWPairEngine:1..5` — matches the clone's `CreateStereoPairSubEngines` walk of the ≤96 kHz group list (in-pair 1 and out-pair 1 share "Analog 1/2"). **Check for the clone:** the reference UID suffix runs 1..5 (one per engine), while the clone's engine *bit* index counts every pair visited (Q-MJ §8) — the UID must not be taken from the bit index.
* Mixer register window not readable from user space (`0xE0008017`), status window readable.
