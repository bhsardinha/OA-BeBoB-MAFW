# Q-AP0b — Audiophile status packet, front-panel controls and LEDs (AP-0.4)

Status: **resolved 2026-10-19** except where a line says **unverified**. Bundle v1.11.2 (`_extracted/kext`, x86_64 slice). Tracker: `PROVENANCE-MATRIX-AUDIOPHILE.xlsx` Itens `AP-0.4.*`. Resolution entry: `PROVENANCE-RESOLUTIONS.md` R78. Builds on `Q-AP0-nub-and-rev2.md` §2.5 and on `Q6-fwhsci.md` (the 1814's HSCI).

## 1. Polling (`m_audio_b_FWHSCI`, shared with the 1814)

* `Start(workloop)` (decomp `FWHSCI__Start.c`): allocates `size & ~3` bytes (`[+0xc0]` = `GetControlPacketSize()` = `0x3C` for the Audiophile), initialises a kernel timer on `HSCITimerFired`, arms it with **50 ms** (`vt[0x1d0](…, 0x32)`).
* `HSCITimerFired` (decomp): zeroes the buffer and reads **`size/4` quadlets from `0xFFC7:0x00600000`** with one `IOFWReadQuadCommand` (reused after the first allocation), callback `FWHSCIDeviceCallback`.
* `HSCIReadComplete(err)`: on error re-arms the timer with **1000 ms**; on success runs `HSCISafelyComplete` on a new thread, which calls the device functor (tag `0xBB9` → `ReadHSCIData`) and re-arms with **50 ms** (`+[+0xd4]`, reset to 0).
* `Pause`/`Resume` are counted (`[+0x1c]`); `Resume` re-arms with 50 ms.
* Byte order: `IOFWReadQuadCommand` copies the response quadlets without swapping (Apple `_refs/IOFireWireFamily/IOFireWireFamily.kmodproj/IOFWReadQuadCommand.cpp:67`, `*fQuads++ = *src++`), so the buffer holds **wire (big-endian) bytes**. (That source is a later IOFireWireFamily than 10.6/10.13; the code path is unchanged in practice — **unverified** for 10.13.6's binary, check `_refs/macos-10.13.6-fw/IOFireWireFamily.bin` if it ever matters.)

## 2. The 60-byte packet

Quadlets 0-13: peak meters; quadlet 14: status word. Kext: `FWAudiophileDevice::ReadHSCIData @0xba92` (asm `0xba92-0xbb99`, matches the decompile); `FWRev2AudioDevice::UpdatePeakMeters @0x12aae`.

| Quadlet | Meaning | Kext handling |
|---|---|---|
| 0-1 | analog in 1-2 | `peak[i] = upper 16 bits of the big-endian quadlet` → `[+0x108][i]` (shared meter page) |
| 2-3 | S/PDIF in 1-2 | same |
| 4-7 | analog out 1-4 | same |
| 8-9 | S/PDIF out 1-2 | same |
| 10-11 | headphone L/R | same |
| 12-13 | aux L/R | same |
| — | software returns 1-6 | page slots 14-19 ← `[+0x110][14..19]` (host-computed, `_SamplePeaks6`); `GetFirstSWReturnPeakMeterIndex` = 14 |
| 14 | status word (wire bytes `w0 w1 w2 w3`) | below |

Label order agrees with `snd-bebob` `bebob_maudio.c:651-655` (`audiophile_meter_labels`: ANA_IN, DIG_IN, ANA_OUT, ANA_OUT, DIG_OUT, HP_OUT, AUX_OUT, stereo pairs) and `ctl-services` `normal.rs` (`AudiophileMeterProtocol` 4 in / 6 out / headphone ×2 / aux ×2 / 1 status quadlet, `calculate_meter_frame_size`, `read_meter` ~835-955).

Status word (asm on the host-order value `s = w3<<24 | w2<<16 | w1<<8 | w0`):

| Wire byte | Kext | ctl-services |
|---|---|---|
| `w3` ≠ 0 | external sync locked (`s > 0xFFFFFF`, `seta`) → `DetectExternalSyncChanges(locked, rate)` | `sync_status = frame[len-1] > 0` |
| `w2` | rate code → `DetectExternalSyncChanges` | not read |
| `w1` low nibble | **encoder 1 = Level Controller**: `TRotaryControl[+0x1268]` → `FWSettingsRotaryControl::UserRotatedHardwareKnob` | rotary 0: `(frame[len-3] >> 0) & 0xF` |
| `w1` high nibble | **encoder 2 = headphone level**: `TRotaryControl[+0x126c]` → `SetLevelsFromKnob(…, "MAHeadphoneLevels", 0x38, group [+0x1548])` | rotary 1 |
| `w0` bits 4-7 ≠ 0 | A/B button pressed (`s<<24 >= 0x10000000`) → `FWSettingsABSwitch::MomentarySwitchPressed` (`[+0x2d0]`) | `frame[len-4] & 0xF0` |

Encoder nibble values (`TRotaryControl::RotaryValueChanged @0xc2ba`): `1` → `+step`, `2` → `−step`, anything else → no change; step `[+0x1268]` = `[+0x126c]` = **1** (ctor, asm `0xbee0`/`0xbeea`). ctl-services: `0x01` increments, `0x02` decrements. So the encoders report **detents**, not positions. Which physical knob is "encoder 1" is from the kext's use, not observed (**unverified** on hardware; AP-7.7).

## 3. Level Controller (`FWSettingsRotaryControl`, object at `[+0x468]`)

* `Init` (decomp): five channel groups (SW returns, outputs, inputs, headphones, aux outputs); properties **`MARotaryControlV2`** (64-bit, setter `NullSetProperty64`) and **`MAMetaVirtualRotateDataKnob`** (setter `VirtualRotateKnob` → `UserRotatedHardwareKnob(value)`, i.e. the panel can turn the knob virtually).
* `UserRotatedHardwareKnob(delta)`: mask `m = [+0x68]`; for each group calls `SetLevelsFromKnob(m, key, bitBase, group, delta)` with `MASWReturnLevels` base 0, `MAInputLevels` 0x10, `MAOutputLevels` 0x20, `MAAuxOutputLevels` 0x32, `MAHeadphoneLevels` 0x38. `SetLevelsFromKnob` walks the group's stereo pairs and, for each pair whose bit `base + pair` is set in `m`, adjusts both elements of the key's array by `delta` (`AdjustOSNumberElement`), then publishes the array with `SetPropertyAndNotifyUserClients`.
* `ResetToFactorySettings`: `m = 0xFFFF00000000` → bits 32-47 → **all output pairs** (manual: the Level Controller controls the outputs by default).
* **Step (closed 2026-10-19, R87)** — `AdjustOSNumberElement @0x21148` (asm `0x21148-0x211fe`), value unit ½ dB: if the element is above −13 (−6.5 dB) or `value + delta ≥ −12`, it moves `±1` (½ dB) per detent; otherwise `value + 12·delta` (6 dB per detent); result clamped to `[−128, 0]` (−64 dB … 0 dB). The changed array is published with `SetPropertyAndNotifyUserClients`, whose setter runs the normal group path (Q-AP0c §4) to the device.
* **`MARotaryControlV2` writes (closed, R87):** the panel's write goes through `FW64BitNumberProp::CallSetter`, which calls the functor (`NullSetProperty64`, returns 0) and then stores the value at `prop+0x30` = `[+0x4d0]` = the rotary object's `[+0x68]` mask. So the setter is "store only", by design.

## 4. A/B switch and LEDs (`FWSettingsABSwitch`, object at `[+0x2d0]`)

* Properties (`Init`): `MAMomentarySwitchRoutings` (array, 4 elements), `MAHeadphoneSourceV2`, `MAHeadphoneSecondSource`, `MAMomentarySwitchSource`, `MAMomentarySwitchLatent` (setter `NullSetProperty32`).
* `MomentarySwitchPressed`: latent `[+0x104]` = 0; state `[+0xd0]` toggles 0 (A) ↔ 1 (B); then `SendMixerSettingsForCurrentMomentaryMode` and `UpdateLEDs`.
* `SendMixerSettingsForCurrentMomentaryMode`: if `MAMomentarySwitchSource` (`[+0x5c]`) == 0 → headphone source `[+0x144]` = `MAMomentarySwitchRoutings[state]`, `SubmitHeadphoneSourceChange`; otherwise (**Direct Monitoring mode**) reads `MAInputRoutingV2[0..1]`, state A clears bit 0 of element 0 and bits `0x12` of element 1, state B sets them, and republishes `MAInputRoutingV2` (notifying the panel).
* `SetMomentarySwitchRoutingSource(index, value)`: applies immediately only when `index − 1` is the current state.
* `UpdateLEDs`: latent ≠ 0 → LED value 0; state 0 → 1; state 1 → 2; other → 3; sent through `[+0x18]` = the device's `AVCControlSetLEDStatus`.
* `ResetToFactorySettings`: state A, latent 0, routings `[1, 2, 3, 4]`, headphone source 1, second source 1.
* `SendToDevice` only normalises latent to 0/1 (the LEDs are not resent there).

**LED command** — `FWAudiophileDevice::AVCControlSetLEDStatus(v) @0xb7d0` → `AVCVendorCommand(0x02, 0x00, v)` → `AVCVendorCommand(a, b, n=1, &v) @0x23970` (asm): a **16-byte** CONTROL, zero-padded:

`00 FF 00 | 00 0D 6C | 02 00 01 <v> 00 00 00 00 00 00`

(`n > 7` → `0xE00002C2`; an overflow check panics with "array overrun in SetElements".) ctl-services `AudiophileLedSwitch`: vendor-dependent, OUI `00 0D 6C`, data `02 00 01 <state> FF FF` with Off/A/B = 0/1/2 — **same significant bytes and values**; it pads with `FF FF` (12 bytes) where the kext pads with zeros (16 bytes).

**Open:** capture 90 (v1.9) wrote `MAMomentarySwitchRoutings` with **3** elements (`[1, 4, 1]`), v1.11.2 declares 4 — version skew or a different panel encoding; check in AP-0.6 (panel decompile) and AP-0.9.
