# Q-AP0c — Audiophile mixer/router: AV/C function blocks, mapping tables, settings wiring (AP-0.3)

Status: **resolved 2026-10-19** except where a line says **unverified**. Bundle v1.11.2 (`_extracted/kext`, x86_64 slice). Tracker: `PROVENANCE-MATRIX-AUDIOPHILE.xlsx` Itens `AP-0.3.*`. Resolution entry: `PROVENANCE-RESOLUTIONS.md` R79. Related: `Q-ME-mixer-property-framework.md` (the shared property/channel-group engine, already mirrored by the 1814 clone), `Q-AP0-status-packet-front-panel.md` (A/B switch, Level Controller).

Evidence: decompiles `FWAVCAudioDevice__{AVCControlFunctionBlock,AVCControlFeatureFunctionBlock,AVCControlProcessingFunctionBlock,AVCSelectorCommand,AVCMixerConnectPair,AVCMixerConnectSingle,FetchRoutingEntry,ResendCurrentMixerSettings,InitializeUserSettableProperties}.c`, `FWAudiophileDevice__{MapChannelAndFunctionBlock,MapRoutingParameters,SetHeadphoneSelector,ResendCurrentMixerSettings,ResetToFactorySettings,InitializeUserSettableProperties,$_667}.c`, `FWSettingsLevels__*.c`, `FWSettingsAuxSend__*.c`; tables read from the slice bytes (`0x2f750`, `0x2f7a0`, key strings `0x2f010/0x2f1ce/0x2f1de/0x2f27e/0x2f292`); asm `0x215b0-0x2168b` (`BaseChannelGroup::SetAll*`), `0x21bc4-0x21c59` (`FWSettingsLevels::ResetToFactorySettings`). Cross-check: `ctl-services` `protocols/bebob/src/maudio/normal.rs` Audiophile classes (~474-620).

> **Correction (2026-10-20, R104):** §5 says the factory reset leaves "mute/solo/link off on both unions (`BaseChannelGroup::SetAll*` ignore their argument and store 0)". Wrong for **link**: `SetAllLinked @0x216d6` stores **1** (`movq $0x1, 0x18(%rax)` at asm `0x21705`); levels/balance/mute/solo store 0. Matches capture 92 (every `MA*Link` = 1 after the official reset, panel screenshot `reset/reset-mixer.png` shows all link buttons lit). The clone always stored 1 (`FWChannelGroup.cpp` `SetAllLinked`).

> **Update (2026-10-19, R84):** §7 item 2 closed — the Direct-Monitoring bits `0x12` set bit 4, but `SetAVCRouting` only loops over `nDst` = 3 mixers (bits 0-2), so bit 4 is never sent on the Audiophile (the generic `FWSettingsABSwitch` code serves models with more mixers). §5 headphone selector: `SubmitHeadphoneSourceChange` is called only from `SetMomentarySwitchRoutingSource` and `SendMixerSettingsForCurrentMomentaryMode` (asm `0x1e360`, `0x1e39c`), and `SetHeadphoneSource` (setter of `MAHeadphoneSourceV2`) sends it directly: value = mask → `SourceMaskToHeadphoneSelector` (lowest set bit) → device functor `SetHeadphoneSelector`; it also sets latent = 1 (LEDs off). Nothing resends it on start — it reaches the device only when the key is written (panel, helper replay) or the A/B button is pressed. Mirror as is.

## 1. The four AV/C frames (all CONTROL to the audio subunit `0x08`, opcode `0xB8` FUNCTION BLOCK, attribute `0x10` current, sent through the async queue `[+0x268]->vt[0x8b0]`, response ignored)

| Builder | Frame | Notes |
|---|---|---|
| `AVCControlFeatureFunctionBlock(fb, sel, ch, v16)` | `00 08 B8 81 <fb> 10 02 <ch> <sel> 02 <v hi> <v lo>` (12 B) | feature FB; `sel` 1 mute, **2 volume**, **3 L/R balance** (AV/C audio subunit control selectors) |
| `AVCSelectorCommand(fb, in)` and `SetHeadphoneSelector` via `AVCControlFunctionBlock(0x80, fb, …)` | `00 08 B8 80 <fb> 10 02 <in> 01 00 00 00` (12 B) | selector FB, control selector `01` |
| `AVCMixerConnectSingle(on, fb, plug, inCh, outCh)` (processing FB) | `00 08 B8 82 <fb> 10 04 <plug> <inCh> <outCh> 03 02 <on ? 00 : 80> 00 00 00` (16 B) | mixer control `03`, value `0x0000` (0 dB, connected) / `0x8000` (−∞, disconnected) |
| `AVCMixerConnectPair(on, fb, plug, inPair, outPair)` | two singles: channels `(2·in−1, 2·out−1)` then `(2·in, 2·out)` | stops if the first fails |

Value encoding for feature controls: `v16 = (uint16)(value << 7)` — the property value is in **½ dB** steps (AV/C volume is 1/256 dB); balance/pan are sent as `−value << 7`. Property range (all groups): min `0xFFFFFF01` (−255 = −127.5 dB), max 0 (`FWSettingsLevels::Init`, `FWSettingsAuxSend::Init`).

The 1814 clone already contains these four builders byte-for-byte (`openaudio-fw1814/kext/src/FWAvc.cpp:185-210`, `FWAvcSelectorCommand`, `FWAvcFeatureFunctionBlock`, `FWAvcMixerConnectSingle/Pair`, async `vt[0x8b0]`), used by the 1814's routing (`FWMixer.cpp:154`).

## 2. Audiophile function-block map (`MapChannelAndFunctionBlock @0xb0e2`, `vt[0xb48]`)

Input: `{kind, fb, ch}`; output: function-block id and the channel inside that block (1/2).

| kind | Used by | Channel → function block | ctl-services name |
|---|---|---|---|
| 0 | `SetAuxSendLevel` (feature, sel 2) | table `@0x2f750`: ch 1-2 → `0x09`, 3-4 → `0x0A`, 5-6 → `0x06`, 7-8 → `0x07`, 9-10 → `0x08` (channel re-based to 1/2) | aux sources: analog in, digital in, stream 1/2, 3/4, 5/6 |
| 1 | `SetHeadphonesLevel` (feature, sel 2) | → `0x0F` | headphone |
| 2 | `SetOutputLevel` (feature, sel 2) | ch 1-2 → `0x0C`, 3-4 → `0x0D`, 5-6 → `0x0E` | analog out 1/2, 3/4, digital out |
| 3 | `SetSWReturnLevel` (feature, sel 2) | ch 1-2 → `0x01`, 3-4 → `0x02`, 5-6 → `0x03` | **not in ctl-services** (kext-only, **unverified on hardware**) |
| 4 | — | `0xE00002C7` (unsupported) | — |
| 5 | `SetAuxMonitor` (selector) | fb = pair (1, 2, 3) | output selectors: `0` = mixer, `1` = aux |
| 6 | `SetInputChannelFeature` (feature, sel 2 level / 3 balance) | ch 1-2 → `0x04`, 3-4 → `0x05` | analog in, digital in |
| 7 | `SetAuxOutChannelFeature` (feature, sel 2 / 3) | → `0x0B` | aux output |
| 8 | `SetHeadphoneSelector` (selector) | → `0x04` | headphone source: `0` mixer 1/2, `1` mixer 3/4, `2` mixer 5/6, `3` aux |
| 9 | — | `0xE00002C2` | — |

Note: function-block ids are per type (feature `0x04` ≠ selector `0x04`).

## 3. Routing (`MapRoutingParameters @0xba50`, `vt[0xb40]`; table `_kRouteTable.36633 @0x2f7a0`, 15 entries × 16 B)

Entry `{srcKind, srcPair, dstMixer, fb, plug}`; `srcKind` 1 = software return, 0 = physical input. The kext looks up the entry and then sets `srcPair = dstMixer = 1` (only the pair inside the block).

| srcKind | srcPair | dstMixer 1/2/3 → fb | plug |
|---|---|---|---|
| 1 (SW return) | 1 (1/2) | 1 / 2 / 3 | 0 |
| 1 | 2 (3/4) | 1 / 2 / 3 | 1 |
| 1 | 3 (5/6) | 1 / 2 / 3 | 2 |
| 0 (input) | 1 (analog) | 1 / 2 / 3 | 3 |
| 0 | 2 (S/PDIF) | 1 / 2 / 3 | 4 |

ctl-services `AudiophileMixerProtocol`: destinations fb `0x01/0x02/0x03` = mixer 1/2, 3/4, 5/6; sources analog in `0x03`, digital in `0x04`, stream 1/2 `0x00`, 3/4 `0x01`, 5/6 `0x02` (`normal.rs:609-622`) — **identical**.

`FWSettingsLevels::SetAVCRouting(kind, nDst, srcPair, mask)`: for each destination mixer `i` = 0..nDst−1 → `MapRoutingParameters({kind, srcPair, i+1})` → `AVCMixerConnectPair(mask bit i, fb, plug, 1, 1)`. `SetSWReturnRouting(pair, mask)` uses kind 1, `SetInputRouting(pair, mask)` kind 0, both with `nDst` = SW-return count / 2 = 3.

## 4. Keys, groups and senders (v1.11.2)

Groups (`$_667` at `[+0x1270]`, constructor + `InitializeUserSettableProperties @0xb7e6`): outputs `ConcreteChannelGroup<6>` `[+0x1270]`, inputs `<4>` `[+0x1378]`, SW returns `<6>` `[+0x1440]`, headphone `<2>` `[+0x1548]`, aux sends `<10>` `[+0x15d0]`, aux output `<2>` `[+0x1758]`. Settings objects: `FWSettingsABSwitch [+0x2d0]`, `FWSettingsRotaryControl [+0x468]`, `FWSettingsAuxSend [+0x530]` (sends, aux out, n = outputs/2 = 3), `FWSettingsLevels [+0x808]` (SW return, output, input, headphone, aux out).

| Key | Owner | Sent as |
|---|---|---|
| `MASWReturnLevels/Mute/Solo/Link` | Levels (SW-return group) | per channel: effective level (group engine) → `SetSWReturnLevel` kind 3 |
| `MAOutputLevels/Mute/Solo/Link` | Levels (output group) | → `SetOutputLevel` kind 2 |
| `MAInputLevels/Mute/Solo/Link` | Levels (input group) | → `SetInputLevel` → kind 6, sel 2 |
| `MAHeadphoneLevels/Mute/Link` (no Solo) | Levels (headphone group) | → `SetHeadphonesLevel` kind 1 |
| `MAInputPan2` (4) | Levels | `SetInputPan(ch, v)` → kind 6, sel 3, `−v<<7` |
| `MAInputRoutingV2` (2) | Levels | `SetInputRouting(pair, mask)` → mixer connects |
| `MASWReturnRoutingV2` (3) | Levels | `SetSWReturnRouting(pair, mask)` → mixer connects |
| `MADirectMonitoringActive` | Levels | stores 1, clears the input group's `+0x11` flag, resends the group |
| `MAGlobalMute` / `MAGlobalDim` | Levels | flags `+0x11` / `+0x10` on the **output** group, resend (the group engine applies them) |
| `MAAuxSendLevels` (10) | AuxSend | `SetAuxSendLevel` kind 0, sel 2 |
| `MAAuxOutputLevels/Mute/Link` | AuxSend | → `SetAuxOutputLevel` kind 7, sel 2 |
| `MAAuxOutputPan2` (2) | AuxSend | `SetAuxPan` kind 7, sel 3, `−v<<7` |
| `MAMonitorAuxOnOutput` (3) | AuxSend | `SetAuxMonitor(pair, on)` → selector fb = pair, `on ? 1 : 0` |
| `MAHeadphoneSourceV2`, `MAHeadphoneSecondSource`, `MAMomentarySwitchRoutings`, `MAMomentarySwitchSource`, `MAMomentarySwitchLatent` | ABSwitch | `SubmitHeadphoneSourceChange` → device functor `[+0x2f8]` = `SetHeadphoneSelector` kind 8 (see Q-AP0b §4) |
| `MARotaryControlV2`, `MAMetaVirtualRotateDataKnob` | RotaryControl | Q-AP0b §3 |
| `MAClockSource`, `MAFirmwareInfo` | `FWAVCAudioDevice::InitializeUserSettableProperties` | Q-AP0 §2.3 |

Mute/solo/link/dim/global-mute semantics come from the shared group engine (`Q-ME` §2; clone `FWChannelGroup.{h,cpp}`); the two unions are `[+0x8d0]` = SW returns + inputs and `[+0x930]` = outputs + aux out + headphone (`FWSettingsLevels::Init`).

## 5. Initial state and resend

* `ResendCurrentMixerSettings` (Audiophile `@0xb00e`): base `FWAVCAudioDevice::ResendCurrentMixerSettings` (if `vt[0xac8]`, rate-limited to once per second with `IOSleep(1000)`, then the `[+0x150]` functor with `[+0x144]`), then `SendToDevice` of ABSwitch (only normalises latent), RotaryControl, AuxSend (aux-send group; `MAMonitorAuxOnOutput` per pair; `MAAuxOutputPan2` ch 1-2), Levels (both unions' `SendLevelsToDevice`; headphone group; `MAInputPan2` ch 1-4; `MAInputRoutingV2` per pair; `MASWReturnRoutingV2` per pair).
* **Unverified:** what `vt[0xac8]` and the `[+0x150]/[+0x144]` functor are on the Audiophile (read `FWAVCAudioDevice` ctor/Init), and whether the headphone selector is resent (ABSwitch's `SendToDevice` does not send it).
* `ResetToFactorySettings` (Audiophile `@0xb078`): base, ABSwitch, Rotary, AuxSend, Levels. Values from asm/decompile:
  * all levels **0 (0 dB)**, balance 0, mute/solo/link off on both unions (`BaseChannelGroup::SetAll*` ignore their argument and store 0, asm `0x215b0-0x2168b`);
  * `MADirectMonitoringActive` = 1; `MAInputPan2` pairs = `−255, +255` (hard L / hard R); `MAInputRoutingV2` = 0 (inputs unrouted); `MASWReturnRoutingV2[i]` = `1 << i` (SW 1/2 → mixer 1/2, 3/4 → 3/4, 5/6 → 5/6);
  * aux sends all `−255` (−127.5 dB), `MAAuxOutputPan2` = `−255, +255`, `MAMonitorAuxOnOutput` = 0 (outputs on mixer);
  * A/B and Level Controller: Q-AP0b.
* **Discrepancy (open):** the manual (p.55, `audiophile-manual-reference.md` §5) says output faders −6 dB by default; the v1.11.2 kext factory reset sets 0 dB. The kext is the spec; check the panel's own defaults (AP-0.6) and the SL rig after a factory reset (AP-0.9 c).

## 6. Reuse map for the plan

| Piece | 1814 clone today | Audiophile needs |
|---|---|---|
| Four FB builders + async queue | present (`FWAvc.cpp`) | reuse unchanged |
| Property dict, array/number props, channel groups, unions, mute/solo/dim engine | present (`FWPropertyDict`, `FWChannelGroup`, `FWSettings`) | reuse; check the generic `FWSettingsLevels` setters (AV/C path) exist, not only the 1814 overrides |
| `MapChannelAndFunctionBlock`, `MapRoutingParameters` + route table | 1814 versions | new Audiophile tables (§2, §3) |
| `FWSettingsAuxSend`, `FWSettingsABSwitch`, `FWSettingsRotaryControl` | 1814 has its own levels/aux classes | port the generic classes (decompiles listed above) |
| LED vendor command, status packet | 1814 differs | new (Q-AP0b) |

## 7. Open items

1. SW-return feature blocks `0x01-0x03` exist only in the kext (not in ctl-services/FFADO) — hardware check (AP-7.6).
2. `MAInputRoutingV2` Direct-Monitoring bits `0x12` touch bit 4, but the Audiophile has 3 mixers (bits 0-2) — generic code shared with other models? Read who else uses `SendMixerSettingsForCurrentMomentaryMode`.
3. Factory output level 0 dB (kext) vs −6 dB (manual).
4. Base resend functor (§5).
