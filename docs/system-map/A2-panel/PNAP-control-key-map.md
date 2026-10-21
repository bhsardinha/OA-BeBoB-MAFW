# PNAP — official panel ↔ kext key map for the FireWire Audiophile (AP-0.6)

Status: **resolved 2026-10-19** for the panel → key layer; screen layout/visuals still to capture (AP-0.9, screenshots). Bundles: panel `_extracted/panel/M-Audio FireWire.app` (v1.11.2 installer, i386 Carbon/PowerPlant), kext v1.11.2; live check SL v1.9 capture 90. Tracker Itens `AP-0.6.*`; resolution R82. Companion of the 1814's `PN2-control-key-map.md` and `P13-translator-index-map.md` (the shared translator `CXDevice`).

Evidence: decompiles `ghidra-exports/decomp/cfwdj-audiophile-2026-10-19.txt` (all 164 functions of `CFWDJ*`, `CXAudiophileDevice`, `FWAudiophileConstructor`), `CFWUniDeviceList::CreateNewDevice @0x6a9d6`, `CXDevice::Choose{Mixer,Output}Bus{AndChannelIndex,AndChannelPair} @0x6de56/0x6ddbe/0x6dcee/0x6dc08`, all `CX*Device::GetDeviceType` (Ghidra 12.1.4 headless on a copy of `ghidra-project/panel-app`, 2026-10-19); panel `__TEXT,__const` control-group tables, entry p (1..5) at `0xf8924 + 2p` (mixer) and `0xf892e + 2p` (output), u16 little-endian; P7-fup / P13 cookie → key tables.

> **Update (2026-10-19, R84) — open items closed statically:** (1) `CXAudiophileDevice` vtable `@0x164c08` read from the bytes: `0x18` = outputs (10), `0x1c` = mixer (10), `0x5c` = inputs (4), `0x60` = headphone (2), `0x68` = `CXDevice::GetNumberOfAuxMonitorSwitchChannels` (= outputs); the §2 split is byte-level. (2) `MAMonitorAuxOnOutput` index 4: the panel's guard is `idx ≤ outputs/2 = 5`, but `CXDevice::SetChannelLevelProperty @0x6d9c2` does `replaceObjectAtIndex:idx−1` on the kext's 3-element array inside `@try`; index 4 raises `NSRangeException`, which is caught — **the write is dropped in the panel, the kext never sees it**. (3) Cookie `0x5fe`: not handled by `CXDevice::GetHWSetting @0x6e2ca` (default returns 1), and `CXAudiophileDevice` does not override it, so `WriteSettingsToHardware` always stores **1** in `[+0x1d4]`.

## 1. Which panel code is the Audiophile

`CXAudiophileDevice::GetDeviceType` returns **`'FWDJ'`** (`0x4657444A`), and `CFWUniDeviceList::CreateNewDevice` builds `CFWDJDeviceSettings` (0x29C bytes) for it. So the panel's **FWDJ** classes are the Audiophile UI: `CFWDJDeviceSettings`, `CFWDJMixerView`, `CFWDJMixerChannelPairView`, `CFWDJOutputView`, `CFWDJHPOutChannelPairView`, `CFWDJHardwareView`, `CFWDJGlobalSettingsView`, `CFWDJMiniMetersView` (thin subclasses of the generic `CMixerView`, `COutputView`, `CHardwareView`, `CGlobalSettingsView`, `CMetersView`). Tabs: `GetTabInfo` → 4 tabs, base resource `0x7D1`.

Panel type codes for every model (for Appendix A): `1814` FW 1814, `F410` FW 410, **`FWDJ` Audiophile**, `Mini` FW Solo, `FWOz` Ozonic, `NbLa` NRV-10 (Nebula), `1212` ProjectMix (`CX1212csDevice`), `FAdt` ProFire Lightbridge; `FWM4` is handled by `CreateNewDevice` but has no constructor (dead).

`CXAudiophileDevice` counts: outputs **10**, mixer channels **10**, inputs **4**, headphone channels **2**, SW-return peaks embedded **0**.

## 2. Channel ids → kext keys (shared translator, boundaries from the counts)

`CXDevice::ChooseMixerBus*` uses `vt[0x5c]` (inputs = 4) and `vt[0x1c]` (mixer = 10); `ChooseOutputBus*` uses `vt[0x18]` (outputs = 10) and `vt[0x60]` (headphone = 2). **Unverified detail:** the slot → getter assignment is by name order of the `CXAudiophileDevice` getters (vtable not dumped); the resulting split matches every kext group size, which is strong support.

| Panel | Ids | Kext key / element |
|---|---|---|
| Mixer channel | 1-4 | `MAInput*[1..4]` (analog L/R, S/PDIF L/R) |
| | 5-10 | `MASWReturn*[1..6]` |
| Mixer pair | 1-2 | input pairs 1 (analog), 2 (S/PDIF) |
| | 3-5 | SW-return pairs 1/2, 3/4, 5/6 |
| Output channel | 1-6 | `MAOutput*[1..6]` (out 1/2, 3/4, S/PDIF) |
| | 7-8 | `MAHeadphone*[1..2]` |
| | 9-10 | `MAAuxOutput*[1..2]` |
| Output pair | 1-3 / 4 / 5 | output pairs / headphone / aux output |

`MapMixerChannelPairToChannelIDs` and `MapOutputChannelPairToChannelIDs` are identity (pair p → ids 2p−1, 2p). Control groups (rotary `ctrl` grouping): mixer pairs `[3, 3, 1, 1, 1]` (3 = input, 1 = SW return — same numbering as the rotary presets in P13), output pairs `[2, 2, 2, 2, 5]` (pairs 1-4 → group 2, aux → 5). **Open:** why the headphone pair is in group 2 (outputs) and not 4.

## 3. What `CFWDJDeviceSettings::WriteSettingsToHardware @0x62b94` sends (cookie → key from P7-fup §3 / P13)

| Cookie | Loop | Key (kext) |
|---|---|---|
| `0x5e5` (2 B) | — | `MARotaryControlV2` (preset 1..5 → mask) |
| `0x5e6` / `0x5e7` | — | `MAGlobalMute` / `MAGlobalDim` |
| `0x5ea` | mixer ch 1-10 | `MAAuxSendLevels[1..10]` |
| `0x5eb` | mixer ch 1-4 | `MAInputPan2[1..4]` |
| `0x5ec` | mixer ch 1-10 | `MAInputLevels` / `MASWReturnLevels` |
| `0x5ed` | mixer pairs 1-5 | `MARotaryControlV2` bit (strip `ctrl`) |
| `0x5ee` / `0x5ef` / `0x5f0` | mixer pairs 1-5 | `MA<bus>Link` / `Solo` / `Mute` |
| `0x5f1` (3 B) | mixer pairs 1-5 | `MAInputRoutingV2` / `MASWReturnRoutingV2` (3 destination flags → mask) |
| `0x5f4` | output ch 1-10 | `MAOutputLevels` / `MAHeadphoneLevels` / `MAAuxOutputLevels` |
| `0x5f5` | output ch 9-10 | `MAAuxOutputPan2[1..2]` |
| `0x5f7` | output pairs 1-5 | `MARotaryControlV2` bit |
| `0x5f8` / `0x5fa` | output pairs 1-5 | `MA<bus>Link` / `MA<bus>Mute` |
| `0x5f9` | output pairs 1-3 (not 4, 5) | `MAOutputSolo` — matches the kext: headphone and aux out have no Solo |
| `0x5fb` (2 B) | output pairs 1-4 | `MAMonitorAuxOnOutput` (`value != 1`); the kext array has **3** elements, index 4 falls outside (P13: guard `idx ≤ outputs/2`) — **check** what the kext does with it |
| `0x5fd` (2 B) | — | `MAHeadphoneSourceV2` |
| `0x5ff` (2 B) | — | clock-source radio → `MAClockSource` via `ChangeDigitalSourceAttribute` (P7-fup) |
| `0x601` (4 B) | — | `MASampleRate` (1 s deferred) |
| `0x608` (2 B) | — | `MAMomentarySwitchSource = (value == 3)` |
| `0x609` (6 B) | — | `MAMomentarySwitchRoutings` = **3** masks (P13). **Closes AP-0.4.8:** capture 90's 3-element array is the panel's encoding, not v1.9 skew |
| `0x5fe` (read, 1 B) | — | stored in `[+0x1d4]` (lock/status flag; **unverified** meaning) |

All 14 keys of capture 90 appear in this list. Keys the kext has but this panel never writes: `MAHeadphoneSecondSource` (single headphone on the Audiophile), `MADirectMonitoringActive`, `MAMetaVirtualRotateDataKnob`.

## 4. Panel-side model and defaults

`CFWDJDeviceSettings` ctor `@0x604f6`: 10 mixer levels, 10 aux sends and 10 output levels start at `−64` (`0xFFFFFFC0`, the panel's dB floor) or 0; per-pair flags 0; `[+0x88]` (rotary preset) = 1; momentary routings `1, 2, 3`, source 0; ten `{1.0f, −64, 0/0x40}` triplets (graphics/fader scaling); then the ctor calls `vt[0x94](this, device)` — **unverified** which method (expected: the hardware read, `ReadSettingsFromHardware @0x623a4`; check the CFWDJDeviceSettings vtable). If so, **defaults come from the kext's factory state** (Q-AP0c §5: outputs 0 dB) unless a settings file is loaded; the manual's −6 dB is not in the panel constructor either. **Open:** check `CreateNewSettingsFile` / factory-reset path and the SL rig (AP-0.9 c).

`CalculateOutputPairBalanceLevel @0x61118`: output pairs have no balance key; the panel derives a balance from the two channel levels (same rule as the 1814, P13 "Balance rule").

## 5. For the clone panel (AP-5.2)

**Layout rule (user decision 2026-10-19):** the Audiophile screens follow the layout and style of our new 1814 panel (`openaudio-fw1814/control-panel/`); the official FWDJ screens (capture 91) are only the reference for which controls and labels exist.

* Audiophile tabs = 4 (mixer, output, hardware, global/about per `GetTabInfo`); strip sets: mixer = 2 input pairs + 3 SW-return pairs (pan only on the inputs, routing to 3 mixer pairs, aux send per channel, ctrl/link/solo/mute); output = 3 output pairs (main/aux toggle, solo, link, mute, ctrl) + headphone pair (source, level, link, mute) + aux master (level, pan, link, mute).
* The 1814 clone's translator layer already implements the shared cookie → key logic; the Audiophile needs a descriptor with the four counts and the two control-group tables.
* Visual layout of the FWDJ screens: **captured 2026-10-19** (capture 91, `B5-sl-dtrace-truth/captures/91-sl-audiophile-panel-screens/`: mixer, output, hardware, about, two mini-meter views; v1.9/panel 1.4). Layout matches §2-§3: mixer = 3 SW-return strips + analog in + spdif in; output = 1/2, 3/4, spdif out, aux, phones (mon 1/2·3/4·spd·aux); hardware = rate, sync internal/external, headphone A/B source matrix, a/b switch mode.

## 6. Peak meters (added 2026-10-19, R89)

`CFWDJMiniMetersView::LoadUIElements` builds 10 mixer meters (`[+0x124]`) and 6 output meters (`[+0x128]`). The values come from the shared page through `CXDevice::GetHWSetting` cookies `0x5e9` (mixer) and `0x5f3` (output) (P9; decompile `@0x6e2ca`). With `GetSWReturnPeaksAreEmbedded` = 0 (Audiophile, like the 1814):

| Panel meter | Page slots read | Kext writes there |
|---|---|---|
| mixer ids 1-4 (inputs) | `page[0..3]` | analog in 1-2, S/PDIF in 1-2 (status packet quadlets 0-3) |
| mixer ids 5-10 (SW returns) | `page[inputs + outputs + k]` = `page[14..19]` | host SW-return peaks (`_SamplePeaks6`, `GetFirstSWReturnPeakMeterIndex = 14`) |
| output ids 1-10 | `page[inputs + j]` = `page[4..13]` | analog out 1-4, S/PDIF out 1-2, headphone L/R, aux L/R (quadlets 4-13) |

Raw → dB: 0 → −64, else `ceil(20·log10(raw·2⁻¹⁵))` (P13 §"Peak → dB"). Panel and kext layouts agree with no gap.

## 7. Every Audiophile control: what it writes and what reaches the hardware (added 2026-10-19, R91)

Purpose (user, 2026-10-19): know what each control does and how it talks to the device, so that **our new 1814 panel is extended** with the Audiophile's items (its look and behaviour stay). Column "panel writes" = measured on the SL rig (capture 92, `decoded-control-key-map.txt`, v1.9 / panel 1.4); column "driver → device" = the v1.11.2 kext path (Q-AP0c §1-§4, Q-AP0b, Q-AP0 §2-§5). Levels/pans are ½-dB / ±255 integers; arrays are written whole; a linked pair writes both channels.

| Panel control | Panel writes (key = value) | Driver → device |
|---|---|---|
| **Mixer tab** | | |
| SW-return strip fader (1/2, 3/4, spdif) | `MASWReturnLevels[6]`, ½ dB, 0 = 0 dB, −255 floor | group engine → feature FB `0x01/0x02/0x03`, sel 2, `v<<7` (kext-only block, see AP-0.3.6) |
| SW-return aux knob L/R | `MAAuxSendLevels[10]` elements 5-10 | feature FB `0x06/0x07/0x08` (aux sources), sel 2 |
| SW-return solo / mute / link | `MASWReturnSolo` / `Mute` / `Link` `[6]` (pair → both elements) | group engine recomputes effective levels → same level commands |
| SW-return out `1/2` `3/4` `spd` | `MASWReturnRoutingV2[3]`, per pair a mask: bit0 = mixer 1/2 (out 1/2), bit1 = mixer 3/4, bit2 = mixer 5/6 (spdif); default `[1, 2, 4]` | per bit: mixer cross-point pair `00 08 B8 82 <fb 1/2/3> 10 04 <plug 0/1/2> …` connect `00` / disconnect `80` |
| Input strip fader (analog in, spdif in) | `MAInputLevels[4]` | feature FB `0x04` (analog) / `0x05` (spdif), sel 2 |
| Input aux knob L/R | `MAAuxSendLevels[10]` elements 1-4 | feature FB `0x09` / `0x0A`, sel 2 |
| Input pan L/R | `MAInputPan2[4]`, −255 (L) … +255 (R) | feature FB `0x04/0x05`, sel 3 (balance), `−v<<7` |
| Input solo / mute / link | `MAInputSolo` / `Mute` / `Link` `[4]` | group engine → level commands |
| Input out `1/2` `3/4` `spd` | `MAInputRoutingV2[2]` (analog, spdif), same mask bits; default 0 (unrouted) | mixer cross-point pair, plug 3 (analog) / 4 (spdif) |
| strip `ctrl` | `MARotaryControlV2` (64-bit) bit of that pair: SW pairs bits 0-15, input pairs 16-31, output pairs 32-47, aux 50 | stored only; the Level Controller knob uses it (Q-AP0b §3) |
| Level controller radios | `MARotaryControlV2` preset: sw rtn `0xFFFF`, input `0xFFFF0000`, output `0xFFFF00000000` (default), aux send `1<<50` | stored only |
| main output mute / dim | `MAGlobalMute` / `MAGlobalDim` = 0/1 | output-group flags → output level commands (group engine) |
| **Output tab** | | |
| out 1/2, 3/4, spdif fader | `MAOutputLevels[6]` | feature FB `0x0C/0x0D/0x0E`, sel 2 |
| out balance knob | no key — the panel lowers one channel of `MAOutputLevels` (L moved, R kept) | as above |
| out solo / mute / link | `MAOutputSolo` / `Mute` / `Link` `[6]` | group engine → level commands |
| out `main` ↔ `aux` | `MAMonitorAuxOnOutput[3]` = 0 main / 1 aux | selector FB `1/2/3`: `00 08 B8 80 <fb> 10 02 <0|1> 01 00 00 00` |
| aux master fader / pan | `MAAuxOutputLevels[2]` / `MAAuxOutputPan2[2]` | feature FB `0x0B`, sel 2 / sel 3 |
| aux mute / link | `MAAuxOutputMute` / `Link` `[2]` | group engine |
| phones fader / mute / link | `MAHeadphoneLevels` / `Mute` / `Link` `[2]` (no solo) | feature FB `0x0F`, sel 2 |
| phones `mon` 1/2, 3/4, spd, aux | `MAHeadphoneSourceV2` = mask 1 / 2 / 4 / 8 | lowest bit → selector FB `0x04` input 0-3; also turns the A/B LEDs off (latent) |
| **Hardware tab** | | |
| sample rate arrows | no `MA*` key: the panel sets `IOAudioStreamFormat` through CoreAudio (rate change through the HAL — same as our 1814 panel) | engine `SetIOFormat` → blank-slate clock (internal + 2.5 s) → `0x2F` stream format per plug (Q-AP0 §2.1, §5) |
| sync source internal / external | `MAClockSource` = `0x40000000` / `0x40000001` | SIGNAL SOURCE `0x1A`: internal `60 01`, S/PDIF `FF 82` (Q-AP0 §2.2-2.3) |
| headphone source A / B matrix | `MAMomentarySwitchRoutings` = `[A mask, B mask, 1]` (masks 1 = 1/2, 2 = 3/4, 4 = spdif, 8 = aux; default `[1, 2, 1]`) | applied when the A/B state equals that entry → selector FB `0x04` |
| a/b switch: direct monitoring / headphone source A/B | `MAMomentarySwitchSource` = 1 / 0 | button press (status packet) → either headphone selector or `MAInputRoutingV2` DM bits; LED vendor command `00 FF 00 00 0D 6C 02 00 01 <0|1|2>` |
| **Settings column** | | |
| reset (after the confirmation sheet) | `MAMetaResetToFactorySettings` = 1 | kext factory state (Q-AP0c §5) and full resend |
| load / save / delete | panel-side presets (files), then normal key writes | — |
| **Front panel (no panel write)** | | |
| Level Controller knob, headphone knob, A/B button | status packet every 50 ms → kext adjusts the keys and notifies the panel | as the corresponding keys above |

**Factory reset measured on the device (capture 92):** outputs 0 dB (`[0 ×6]`), headphones 0 dB, aux sends −255, `MASWReturnRoutingV2 [1, 2, 4]`, inputs unrouted, pans ±255, `MAMomentarySwitchRoutings [1, 2, 3, 4]`, `MARotaryControlV2 0xFFFF00000000` — identical to the v1.11.2 decode; the manual's −6 dB is **not** what the driver does (closes AP-0.3.5).
