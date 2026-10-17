# A2-PN2 — Panel control → MA* key map (approved wireframe PN.1)

> **Correction (2026-10-16) — PN.2 open items 1–5 closed by decompiling the panel translator (`P13-translator-index-map.md`).** Changes to this map: (a) rotary mask layout and presets are known (bits 0–15 SW return, 16–31 input, 32–47 output, 50 aux output, 56–57 headphone; radios → presets 1/2/3/5); (b) pan scale is −255..+255 raw, display ×0.5; faders −129..0 half-dB in the panel; (c) `MAMonitorAuxOnOutput` is indexed per output **pair** (1..10); (d) phones 1 = output pair 8 → `MAHeadphoneSourceV2`, phones 2 = pair 9 → `MAHeadphoneSecondSource`, sheet cookie `0x609` writes a 3-entry `MAMomentarySwitchRoutings` array; (e) cookie `0x608` is the **a/b function** (`MAMomentarySwitchSource = value==3`), not Monitor-Aux; (f) peak slots: input n → `peak[n−1]`, output n → `peak[18+n−1]`, SW return k → `peak[38+k−1]`. Remaining open: see P13 §7. Rows below that still say "unverified" for these points are superseded.

**Status:** drafted 2026-10-16 for review (Build row PN.2 acceptance: "checklist 100% mapped and reviewed by you"). **Scope:** which `MA*` registry key every control of the approved wireframe (`PN1-wireframe.html`, `P12-panel-screens-v19.md`) reads or writes, with value encoding and the clone-kext source. Evidence base: `../A1-ghidra/Q-MAMAP-key-crosswalk.md` (reference decompile, rows cited as "row N"), `P7-fup-translator.md` (reference panel cookie → key), the clone kext sources (`openaudio-fw1814/kext/src/*`), `FWRates.h`. Code: `openaudio-fw1814/control-panel/Sources/MaKeys.swift` (typed catalog + `MAClockSource` codec; `swiftc -parse` OK, nothing run).

**Transport (all controls):** write = `IORegistryEntrySetCFProperties` on the device entry with a dictionary of changed keys; read = `IORegistryEntryCreateCFProperties` / `MAMetaAllProperties` seed; change notifications = type-0 notification port (P2). Meters are not keys (§5). (B5 §10, P3.)

**Key count.** The clone publishes **54** distinct keys (`MAKeys.all`): 19 per-group Levels/Mute/Solo/Link keys + 23 scalar/mixer/clock keys (rows 1–43 of Q-MAMAP, row 20 is the same key as row 21) = 42 panel-relevant, + 9 read-only descriptors + 3 firmware keys that belong to the helper daemon. The "46" used in earlier docs is **not reproduced** by any count I can make from Q-MAMAP; **Correction (2026-10-16):** treat 46 as a stale label.

## 1. Sidebar (every tab)

| Control | Key | Value / encoding | Source |
|---|---|---|---|
| settings → reset | `MAMetaResetToFactorySettings` | any value (function property) | row 38; panel cookie 0x5e3 (P7-fup §3.1) |
| settings → load / save / delete | none (panel-side files) | save = snapshot of `MAMetaAllProperties` (mixer keys only); load = write the keys back in one dictionary | **hypothesis**: manual §2 says the Mac file dialog; format not in `_extracted/`. Check: `Presets` format in the panel binary (PN.7) |
| level controller radios (sw rtn / output / input / aux send) | `MARotaryControlV2` (u64 mask) | the group choice selects which pair bits are set; default `0xFFFF00000000` (all) | row 29; panel cookie 0x5e5 indexes a 5-slot table → mask (P7-fup §3.1). **Unverified:** slot → group order and the bit layout (`ChooseRotaryPairPhaseAndBitIndex` not decoded) |
| strip `ctrl` buttons | `MARotaryControlV2` | single-bit read-modify-write of the same mask | P7-fup §3.2 cookies 0x5ed/0x5f7; same open item |
| level controller knob (on-screen) | `MAMetaVirtualRotateDataKnob` | u32 step count; kext adjusts levels of the masked pairs: SW return, Output, Input, Headphone, Aux output | row 30; cookie 0x5e4 |
| main output mute / dim | `MAGlobalMute` / `MAGlobalDim` | u32 bool | rows 28/27. Apply to the Output group only (`[+0x8b8]`); whether they reach aux/headphone is **unverified** (Q-MAMAP §4) |
| view switch (full / wide / narrow meters) | none | panel preference | — |

## 2. Mixer tab

Channel order of the 18-channel Input group: pairs 1–4 analog, pair 5 S/PDIF, pairs 6–9 ADAT, **inferred** from the routing bit layout in row 24 (pair 5 uses distinct bits 0x100/0x200). The SW-return group has 4 channels = `1/2 sw rtn`, `3/4 sw rtn`.

| Control | Key | Encoding | Source |
|---|---|---|---|
| input strip faders (L/R) | `MAInputLevels[ch]` | int, [-255, 0] half-dB; wire = value × 128 as big-endian int16, Reg 0x10 | row 9, Q-MAMAP §1.4 |
| sw rtn faders | `MASWReturnLevels[ch]` | same, Reg 0x00 | row 1 (capture 62 confirms 0 / −1 values) |
| input pan knobs (L/R) | `MAInputPan2[ch]` | int array; reference default per pair `{-255, +255}`; Reg 0x40 + 4·pair | row 23. Knob legend "L127…R127" suggests display = value/2: **guess — needs verification**. Wire half-word order **not derived** |
| solo / mute / link (inputs, sw rtn) | `MA<Input|SWReturn>Solo/Mute/Link` | bool arrays, one entry per channel; mute/solo act as level operations (no mute register) | rows 2–4, 10–12; Q-MAMAP §3. Solo of SW return and Input are one union (kext `soloSecond`, `FWSettings.cpp:51,61`) |
| aux L / aux R knobs | `MAAuxSendLevels[i]` | 22 ints, default −255; Reg 0x64, rotated writer | row 16; clone `FWSettingsAuxSend` (M-E). Index → strip order **unverified** |
| out 1/2, out 3/4 buttons per strip | `MAInputRoutingV2` (inputs, 9 two-bit masks, Reg 0x90) / `MASWReturnRoutingV2` (2 bitmasks, over AV/C mixer connect) | bit per destination, element i default `1<<i` for SW return | rows 24, 25 |
| bottom meters (output bus, aux bus) | shared peak page (§5) | — | P9 |
| `ctrl` | see §1 | | |

## 3. Output tab

| Control | Key | Encoding | Source |
|---|---|---|---|
| 1/2 out, 3/4 out faders, balance | `MAOutputLevels[ch]` (Reg 0x08); balance = per-channel `balance` field inside the group (level + balance clamped, Q-MAMAP §1.5) | the panel moves a fader pair: with **link off** a single fader changes the pair's balance value (screenshot `R35`) | rows 5–8; `AdjustedLevelForChannel @0x1ee66`. How the panel turns a one-fader drag into level/balance values is **unverified** (panel `SetChannelLevelProperty`, P7-fup §3.2) |
| solo / mute / link | `MAOutputSolo/Mute/Link` | bool arrays | rows 6–8 |
| `main` ↔ `aux` toggle under 1/2 out, 3/4 out, spdif out | `MAMonitorAuxOnOutput[n]` | **3 bools** (output pair 1, pair 2, S/PDIF pair; **Correction 2026-10-16**: earlier text said 10, the kext array has `(output.Count() >> 1) + 1` = 3), written inverted (`val != 1`, panel cookie 0x5fb); kext sets bit n−1 of Reg 0x9c | row 22; P7-fup §3.2; `scripts/conformance-check.sh` |
| adat bus / spdif out strip | none (scale and meters only) | hardwired buses, no level key | manual-reference §3, screenshots |
| aux out bus fader, balance, mute, link | `MAAuxOutputLevels[2]` (Reg 0x34), `MAAuxOutputPan2[2]` (AV/C feature selector 3; reference default `{-255,+255}`), `MAAuxOutputMute`, `MAAuxOutputLink` | | rows 17–21 |
| phones 1 / phones 2 faders, balance, mute, link | `MAHeadphoneLevels[4]` (Reg 0x38), `MAHeadphoneMute`, `MAHeadphoneLink` (no solo) | | rows 13–15 |
| phones 1 mon 1/2 – 3/4 – aux | `MAHeadphoneSourceV2` | source mask: 1 = out 1/2, 2 = out 3/4, 4 = aux; lowest set bit → selector 0..2 → Reg 0x98 bits 24..26 | clone `FWSettings.cpp:335-372`, `FWMixer.cpp:128-139`; row 32. Writing it sets the override flag (A/B LEDs off) |
| phones 2 mon 1/2 – 3/4 – aux | `MAHeadphoneSecondSource` | same mask encoding → Reg 0x98 bits 8..10 | `FWSettings.cpp:375-379`; row 33 |
| bottom meters (software return bus, input bus) | shared peak page (§5) | | P9 |

## 4. Hardware tab

| Control | Key | Encoding | Source |
|---|---|---|---|
| sample rate stepper | `MASampleRate` | u32 Hz, fast path; panel coalesces 1 s (`SendNewSampleRateTimer`). Disable while a client streams (v1.9 panics on rate change while streaming, memory `project_v19_rate_change_while_streaming_panics`) | row 41; P7-fup §3.1 |
| sample-rate display colour / lock icon | `MARateLocked` (read-only bool) | green + closed lock when 1 | row 42; P3 `ReceiveRateLocked`; user statement 2026-10-16 (P12 item 8) |
| rate readout value | `MASampleRate` read-back | **unverified** whether the reference reads it from `MASampleRate` or a separate cookie (P7-fup §3.4 says no reverse handler) | check: clone kext publishes `MASampleRate` (`FWAudioDeviceProps.cpp`) |
| sync source radios | `MAClockSource` bits [1:0]: internal unmuted = 0, external digital = 1, word clock = 2, internal muted = 3 | v1.9 capture; v1.5.2 values **unverified** | `P-ClockUI-encoding.md` §2; `MaKeys.swift` `MAClockSource.Sync` |
| spdif input port (optical / coaxial) | `MAClockSource` bit 2 (1 = coaxial) | greyed when input = ADAT (user) | P-ClockUI §2 (B5 §10). Consumer of bit 2 in the kext is **unverified** |
| digital input adat / spdif | bit 3 (1 = ADAT) | | P-ClockUI §2; cookie 0x60d |
| digital output adat / spdif | bit 4 (1 = ADAT) | | cookie 0x60e |
| adat S/MUX | bit 5 | clone rule (user, 2026-10-16): panel forces it on at 88.2/96 kHz, user-set at 44.1/48 kHz, greyed with output S/PDIF or at 176.4/192 kHz | P12; `FWRates.h` channel tables ignore bit 5 |
| (all writes) | `MAClockSource` read-modify-write, gate bit 30 always set, bit 31 cleared | `MAClockSource.wireValue` | `ChangeDigitalSourceAttribute @0x71fea`; default `0x40000003` (Q3b) |
| a/b switch function radios | `MAMomentarySwitchSource` | 0 = headphone-source mode, non-zero = direct-monitoring mode (the kext then rewrites `MAInputRoutingV2[0] \|= 1`, `[1] \|= 0x12`) | row 34; clone `SendMixerSettingsForCurrentMomentaryMode`, `FWSettings.cpp:388-417` |
| direct monitoring state | `MADirectMonitoringActive` | the kext **ignores the value** (row 26); the panel must not rely on it for state | row 26 |
| set source… sheet (A source, B source) | `MAMomentarySwitchRoutings` | 4 entries, default {1,2,3,4}; entry index == current mode → also becomes the headphone source (row 31). **Unverified:** that entries 0 and 1 are the A and B sheet sources (reference sheet writes cookie 0x609, P11 §3.1) | row 31; `FWSettings.cpp:361-365` |
| A/B position LED | `MAMomentarySwitchLatent` (read-only, toggled by the front-panel press) | | row 35 |
| rate-dependent greying | derived, no key | channel table by (rate, bits 3/4): 88.2/96 kHz ADAT 4 ch; 176.4/192 kHz 2 in / 4 out analog | `FWRates.h` (`kFWADAT88_96`, `kFW176_192`) |

## 5. About tab, meters, devices

| Item | Source | Note |
|---|---|---|
| panel / driver / build / bootloader / firmware / hardware / GUID | `MAFirmwareInfo` (80 B, A1-Q7a), registry GUID, bundle versions | field offsets of the 80 B descriptor: use A1-Q7a. Driver build number is the clone's own |
| web links (updates, support, FAQ) | none | URLs not in `_extracted/` yet — **open** |
| meters (all views and bottom panels) | `clientMemoryForType(1)` of the user client: one 4 KiB page, `uint32 peak[52]` = 38 hardware + 14 SW-return slots (P9 Correction; clone `FW1814UserClient`) | slot → strip map comes from `GetMixerPairChannelMap` / `GetOutputPairChannelMap` (P9 §7.3) — the 1814 values are **not decoded yet**. Refresh ≈ 30 Hz (P9) |
| device chooser (several interfaces) | registry matching by GUID (P1) | M-K needs a second FW1814 |
| user label | `MADeviceUserLabel` | not shown in the v1.9 screens; drop or put in About — **open** |
| `MAEnableStereoPairEngines`, `MAEnableAggregateEngine` | none in the panel | the v1.9 screens show no such control (P12). Kext defaults apply |

## 6. Open items from this map (all labelled above)
1. Rotary mask layout and slot → group order (§1).
2. Input pan scale and wire half-word order (§2).
3. Aux-send and monitor-aux element → strip order (§2, §3).
4. Peak slot → strip map for the FW1814 (§5).
5. Sheet A/B entries of `MAMomentarySwitchRoutings` (§4).
6. Preset file format (§1), URLs, `MADeviceUserLabel` placement.

Items 1–5 can be closed by decompiling the panel translator (`CXDevice::GetHW[Indexed]Setting` / `Choose*` helpers, P7-fup "deferred") or the clone kext's own handlers; none blocks writing the Swift model layer.
