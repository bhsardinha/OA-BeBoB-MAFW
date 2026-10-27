# PN410 — official panel ↔ kext key map for the FireWire 410 (desk + live, 2026-10-26)

Companion of `PN2-control-key-map.md` (FW1814) and `PNAP-control-key-map.md` (Audiophile). Sources: panel decompiles `ghidra-exports/decomp/cfw410-2026-10-26.txt` (all 204 functions of `CFW410*`, `CX410Device`, `FW410Constructor`; v1.11.2 panel, Ghidra 12.1.4 headless on a copy of `ghidra-project/panel-app`); live control sweep capture 103 (SL v1.9 driver / panel 1.4, `B5-sl-dtrace-truth/captures/103-sl-fw410-panel/controls/decoded.txt`); kext side `A1-ghidra/Q-410-0-reference-map.md`. Version skew: the sweep is v1.9/1.4, the spec is v1.11.2; the key names and values agree with the v1.11.2 kext setters (Q-410-0 §5).

## 1. Which panel code is the 410
`CX410Device::GetDeviceType` = `'F410'` (0x46343130). Counts (`CX410Device`): outputs 0xe (10 + phones 2 + aux 2), mixer 0xe, inputs 4, headphone 2, aux-monitor switches = outputs − headphone (vt 0x18 − vt 0x60). UI classes: `CFW410DeviceSettings`, `CFW410MixerView` / `CFW410MixerChannelPairView`, `CFW410OutputView` / `CFW410SpdifOutChannelPairView` / `CFW410HPOutChannelPairView`, `CFW410HardwareView`, `CFW410GlobalSettingsView` (level-controller assign buttons), `CFW410MiniMetersView`. `CX410Device::{Get,Set}HWSetting` handle cookie `0x5fc` themselves: `MAHeadphoneSourceV2` ↔ bool array via `SourceMaskToBools` / `SourceMaskFromBools` (size = (outputs − inputs)/2), every other cookie goes to `CXDevice`. `CFW410HardwareView::{Load,Listen,Adapt}AsioDmEnableControl` are empty (no ASIO control on the Mac 410 panel); `MapSPDIFSourceButtonToSetting` = `button != 0x14b7`.

## 2. Screens (capture 103)
* **mixer:** strips 1/2, 3/4, 5/6, 7/8, spdif **sw rtn** (aux L/R knobs, two faders, ctrl / solo / link / mute, out 1/2 · spd · 3/4 · 5/6 · 7/8); **analog in** and **spdif in** (aux L/R, faders, pan L/R, ctrl / solo / link / mute, out buttons); output-bus and aux-bus meter rows.
* **output:** 1/2, 3/4, 5/6, 7/8, spdif **out** (bal, faders, ctrl / solo / link / mute, main); **aux** (bal, faders, ctrl / link / mute, pan L/R); **phones** (bal, faders, ctrl / main / link / mute, mon 1/2 · spd · 3/4 · 5/6 · 7/8); software-return-bus and input-bus meter rows.
* **hardware:** sample rate (arrows + lock), sync source internal / external, **active spdif input coaxial / optical**, **core audio device settings: "show 410's stereo pairs as devices"**.
* **right column (every tab):** settings load / save / reset / delete, level controller knob + sw rtn bus / output bus / input bus / phones / aux send, main output mute / dim, unit picture.
* **about:** panel / driver / build, bootloader / firmware dates, hardware model, GUID.

## 3. Control → key (live, capture 103)

| Control | Key | Value |
|---|---|---|
| sw rtn n solo / mute / link | `MASWReturnSolo` / `MASWReturnMute` / `MASWReturnLink` | 10 entries, pair = two entries |
| sw rtn n out buttons | `MASWReturnRoutingV2` | 5 entries (one per sw-rtn pair), bitmask: bit 0 out 1/2, 1 out 3/4, 2 out 5/6, 3 out 7/8, 4 S/PDIF out; several bits allowed (e.g. 17 = 1/2 + spdif); as found `[1,2,4,8,16]` |
| sw rtn faders | `MASWReturnLevels` | 10 entries, 0 = 0 dB, negative steps |
| aux knobs (all 7 strips) | `MAAuxSendLevels` | 14 entries: analog in L/R, spdif in L/R, sw rtn 1…10; −255 = −∞ |
| analog / spdif in solo / mute / link | `MAInputSolo` / `MAInputMute` / `MAInputLink` | 4 entries |
| input out buttons | `MAInputRoutingV2` | 2 entries (analog in, spdif in), same bits as above; as found `[0,0]` |
| input faders | `MAInputLevels` | 4 entries |
| input pan | `MAInputPan2` | 4 entries, L = −255…, R = 255 (as found `[-255,255,-255,255]`) |
| out n ctrl, level-controller radios | `MARotaryControlV2` | 64-bit group mask (sw rtn bus `0xffff`, input bus `0xffff0000`, output bus `0xfffe00000000`-style per-pair bits, phones / aux send higher bits) |
| out n solo / mute / link | `MAOutputSolo` / `MAOutputMute` / `MAOutputLink` | 10 entries |
| out n main, phones main | `MAMonitorAuxOnOutput` | 6 entries: out pairs 1-5, phones |
| out faders, bal | `MAOutputLevels` | 10 entries; bal moves one side |
| aux link / mute, faders, pan | `MAAuxOutputLink` / `MAAuxOutputMute` / `MAAuxOutputLevels` / `MAAuxOutputPan2` | 2 entries |
| phones link / mute / faders | `MAHeadphoneLink` / `MAHeadphoneMute` / `MAHeadphoneLevels` | 2 entries |
| phones mon buttons | `MAHeadphoneSourceV2` | bitmask, **multi-select**: 1 = 1/2, 2 = 3/4, 4 = 5/6, 8 = 7/8, 16 = spdif (3 = 1/2 + 3/4 seen) |
| sync internal / external, spdif coax / optical | `MAClockSource` | internal+coax `0x40000004`, internal+optical `0x40000000`, external `0x40000005` (bit 0 = S/PDIF sync, bit 2 = coax, bit 30 = gate) |
| stereo pairs checkbox | `MAEnableStereoPairEngines` | 1 / 0 |
| main mute / dim | `MAGlobalMute` / `MAGlobalDim` | 1 / 0 |
| sample rate arrows | (CoreAudio stream format, as on the Audiophile, capture 92) | — |

Mixer-strip `ctrl` buttons are greyed while the level controller is on "output bus" (no write). Not exercised: settings load / save / reset / delete, the view switch (mini meters), the level-controller knob itself.

## 4. Key → kext (v1.11.2; Q-410-0 §5)
Levels of sw rtn / output / input groups → batch register writers (offsets 0x00 / 0x14 / 0x28); input pan → `SetInputPan` (vt 0xbd0, register 0x38+); input routing → `SetInputRouting` (vt 0xbc8, 0x5c); sw-rtn routing → `SetSWReturnRouting` + `BatchSetSWReturnRouting` (vt 0xbd8/0xbe0, 0x64); phones source → `SetHeadphoneSource` (0x68); clock → `SetClockSourceInternal` (sync stream 0 internal, 4 coax, 3 optical; asm 0x14eda); aux sends, aux out, phones levels and aux-monitor switches → the generic AV/C paths through `MapChannelAndFunctionBlock` (**hypothesis**: no batch writer is installed on those groups; settle when the clone runs, by FCP log).

## 5. Driver features the official panel does not expose (user, 2026-10-26: watch for these)

> **Update (2026-10-26, R137, capture 106 — clone on the iMac):** item 1 is **identified: mask bit 5 (word bit 26) = the aux bus on the phones** (phones carry the aux bus with bit 26 set, whatever the other bits). The reference never clears bit 26 (keep mask 0xc4ffffff), so after one write of bit 5 the phones stay on aux until the driver reloads; the official panel never sends bit 5 (it uses MAIN/AUX, `MAMonitorAuxOnOutput[5]`), so the latch never shows with it.
1. **`SetHeadphoneSource` bit 5 (`0x20` → word bit 26)**: the kext accepts six phones sources, the panel offers five (1/2, 3/4, 5/6, 7/8, spdif) plus "main" (= aux bus through `MAMonitorAuxOnOutput[5]`). What source 6 is: **unverified** (hypothesis: the aux bus or the inputs); check by writing `MAHeadphoneSourceV2 = 32` on the SL rig and listening.
2. **`SetInputLevel` (vt 0xbe8, registers 0x28/0x2c)**: no virtual caller found in mafw.asm (only the vtable entry); the input faders go through the input group's batch writer at 0x28. Probably unused; **unverified**.
3. **Mixer strip `ctrl`** (level-controller membership for single strips) exists in `MARotaryControlV2`; the panel enables it only for some bus selections.
4. **AC-3** (`IndexForAC3OutputChannels` 0/5): format-driven from CoreAudio, no panel control (same as the other models).
5. Routing masks allow fan-out (one sw rtn or input to several outputs at once): the official panel already allows it; our panel must keep it (multi-select out buttons, multi-select phones sources).
