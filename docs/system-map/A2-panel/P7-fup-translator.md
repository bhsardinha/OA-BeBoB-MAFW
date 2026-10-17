# P7-fup — Cookie ↔ NSString translator (`CXDevice::{Get,Set}HW[Indexed]Setting`)

**Status:** done.
**Scope:** Resolve the open question from P7 §6 — where does the cookie space (`0x5e2..0x615`, defined on `CFW1814DeviceSettings`) get mapped to the P3 `MA*` NSString keyspace, and how do the two broadcast chains meet.

**Decompile log:** `ghidra-exports/decomp/a2-p7fup.log` — `CXDevice::{SetHWSetting, SetHWIndexedSetting, InstallPropertyChangeHandlers, ReceiveSampleRate, ReceiveClockSource, ReceiveOnePropertyChange}`. Original binary path: `/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm` (Objective-C++, `.mm` — matches the mixed CF/Obj-C runtime observed in P1/P2/P3).

## 1. The seam: `CXDevice` is the device-agnostic translator

The panel binary has **one** translator class, `CXDevice` (per-device instance, allocated in P1 chain). It owns the IOKit user-client handle, the property-change-handler NSMutableDictionary (`this+0x74`), the queued-write NSMutableDictionary (`this+0x70`), the cached-property NSMutableDictionary (`this+0x6c`), and the deferred-rate `LTimerTask` (`this+0xf8`). All four translator entry points live here:

| Addr | Method | Direction | Behaviour |
|---|---|---|---|
| `0x6e2ca` | `CXDevice::GetHWSetting(cookie, size, outBuf)` | UI → kext → UI (synchronous read) | 3804 B switch on `cookie`; calls `Get32BitNumberProperty` / `Get64BitNumberProperty` / `GetChannelLevelProperty` / `GetPairBooleanProperty` with the matching NSString. |
| `0x6f436` | `CXDevice::SetHWSetting(cookie, size, data)` | UI → kext (write) | 1851 B switch; dispatches to `Set32BitNumberProperty` / `Set64BitNumberProperty` / `ChangeDigitalSourceAttribute` / direct-field-cache + `LTimerTask::Install`. |
| `0x6fce2` | `CXDevice::GetHWIndexedSetting(cookie, idx, size, outBuf)` | UI → kext → UI (indexed read) | 1899 B switch; routes idx through `ChooseMixerBusAndChannelIndex` / `ChooseOutputBusAndChannelPair` to pick the per-bus NSString. |
| `0x704bc` | `CXDevice::SetHWIndexedSetting(cookie, idx, size, data)` | UI → kext (indexed write) | 2450 B switch; same channel-bus translation helpers; calls `SetChannelLevelProperty` / `SetPairBooleanProperty` / `SetMixerPairBooleanProperty`. |

These are **vtable overrides on `CMiddleWareDevice`** — `CMiddleWareDevice::GetCPSetting @ 0x6378a` and friends broadcast message `0x3F2`/`0x3F3`, but when the owner is a concrete `CXDevice` the derived vtable entries (`vtable[+0x20]` / `+0x24` / `+0x28` / `+0x2c` / `+0x30`) land in the `GetHW*`/`SetHW*` quartet. The `CFW1814DeviceSettings` HW-sync loop (P7 §5) calls those slots directly, so writes go **CFW1814DeviceSettings → CXDevice → `objc_msgSend(IORegistryEntrySetCFProperties-wrapper, …)`** with no LBroadcaster hop.

## 2. The reverse path (kext → UI)

Installed by `CXDevice::InstallPropertyChangeHandlers @ 0x6ca12` (re-decoded here — this is a 20-entry registration dictionary). The handler dict is at `this+0x74`:

```
MASWReturnLevels      → ReceiveSWReturnLevelChange
MAInputLevels         → ReceiveInputLevelChange
MAOutputLevels        → ReceiveOutputLevelChange
MAHeadphoneLevels     → ReceiveHeadphonesLevelChange
MAAuxOutputLevels     → ReceiveAuxOutputLevelChange
MAAuxSendLevels       → ReceiveAuxSendLevelChange
MASWReturnSolo        → ReceiveSWReturnSolo
MASWReturnMute        → ReceiveSWReturnMute
MAInputSolo           → ReceiveInputSolo
MAInputMute           → ReceiveInputMute
MAOutputSolo          → ReceiveOutputSolo
MAOutputMute          → ReceiveOutputMute
MAHeadphoneMute       → ReceiveHeadphoneMute
MAAuxOutputMute       → ReceiveAuxOutputMute
MAMonitorAuxOnOutput  → ReceiveMonitorAuxOnOutput
MASWReturnRoutingV2   → ReceiveSWReturnRouting
MAInputRoutingV2      → ReceiveInputRouting
MAHeadphoneSourceV2   → ReceiveHeadphoneSource
MAClockSource         → ReceiveClockSource
MARateLocked          → ReceiveRateLocked
MARotaryControlV2     → ReceiveChannelMask        ← 1:1 with the 21st P3 key
```

(No handlers for `MAMetaAllProperties` / `MAFirmwareInfo` / GUID — read-only; and no handler for `MASampleRate` — see §4.)

`CXDevice::ReceiveOnePropertyChange(key, value) @ 0x71b6c` is the one-key dispatcher: looks up `this[+0x74][key]`, pointer-unwraps the NSValue, calls the handler. On the way out it writes-through to the cached-dict `this[+0x6c][key] = value` so subsequent `GetHWSetting` reads can serve stale-but-coherent data without a round-trip.

The handlers themselves fan out into cookies via `vtable[+0x4c]` (= "update one setting" on the CXDevice vtable, equivalent in signature to `CDeviceSettings::UpdateSetting`): e.g.

```c
ReceiveSampleRate(dev, key, obj) {
    u32 rate = [obj unsignedLongValue];
    dev->vtable[+0x4c](dev, 0x601, 4, &rate);     // cookie 0x601 (current rate)
}
```

Confirms the P7-§4.8 claim that `0x601` is bidirectional, and resolves the P6-fup2 confusion once and for all: `0x601` is written **from the kext side via `MASampleRate` property change → ReceiveSampleRate → UpdateSetting(0x601)**, and **from the UI side via widget → SetSettingData(0x601) → WriteSettingsToHardware → SetHWSetting(0x601) → defer timer → `MASampleRate` property write.**

## 3. Cookie → NSString table (write path, from `SetHWSetting` + `SetHWIndexedSetting`)

### 3.1 Simple scalar writes (`SetHWSetting`)
| Cookie | NSString | Helper | Note |
|---|---|---|---|
| `0x5e2` | — | direct `this[+0x7c] = byte` | **Batch-begin flag.** When set, SendQueuedProperties stops auto-flushing. Panel transactions. |
| `0x5e3` | `MAMetaResetToFactorySettings` | `Set32BitNumberProperty(…, 1)` | Empties the cache dict (`this[+0x6c]`) first. |
| `0x5e4` | `MAMetaVirtualRotateDataKnob` | `Set32BitNumberProperty` | 8-byte input, decoded as "rotate N steps" (steps = `13 − hi/6` if `hi<78`, else 1). |
| `0x5e5` | `MARotaryControlV2` | `Set64BitNumberProperty` | Index into table at `s_8CXDevice+0x8` (5 slots) → 64-bit mask. |
| `0x5e6` | `MAGlobalMute` | `Set32BitNumberProperty` | bool |
| `0x5e7` | `MAGlobalDim` | `Set32BitNumberProperty` | bool |
| `0x5ff` | — | `ChangeDigitalSourceAttribute(mask=—,bits=—,bool,0x40000000)` | Writable through the digital-source register, not an NSString. |
| `0x600` | — | `ChangeDigitalSourceAttribute(4, 4, bool, 0x40000000)` | ditto |
| `0x601` | `MASampleRate` *(deferred via `LTimerTask::Install(this+0xf8, mainLoop, 1.0, 1.0)` → `SendNewSampleRateTimer @ 0x6c8c8`)* | — | Direct cache at `this+0x84`, then 1s-delayed `MASampleRate` property push. Rationale: coalesces rapid rate flips. |
| `0x60d` | — | `ChangeDigitalSourceAttribute(8, 8, in==2, 0x40000000)` | Optical-in SPDIF/ADAT bit. |
| `0x60e` | — | `ChangeDigitalSourceAttribute(0x10, 0x10, out==2, 0x40000000)` | Optical-out bit. |
| `0x60f` | — | `ChangeDigitalSourceAttribute(4, 4, in==1, 0x40000000)` | Active-input bit. |
| `0x611` | — | `ChangeDigitalSourceAttribute(0x20, 0x20, bool, 0x40000000)` | ADAT-4ch S/MUX bit. |
| `0x612` | `MASPDIFMixerBypass` | `Set32BitNumberProperty(…, val==0)` | Inverted sense. |
| `0x614` | `MAEnableStereoPairEngines` | `Set32BitNumberProperty` | bool |

### 3.2 Indexed writes (`SetHWIndexedSetting`)
Several cookies use **bus-dependent** NSString suffix selection via helpers `ChooseMixerBusAndChannelIndex(key_suffix, &outKey, &outIdx)` / `ChooseOutputBusAndChannelPair` / `ChooseMixerBusAndChannelPair` / `ChooseOutputBusAndChannelIndex`. These rewrite the (cookie,idx) into a (bus-specific-NSString, bus-local-idx) pair, so the kext's bus-sharded key naming is kept hidden from the panel's channel-strip model.

| Cookie | NSString | Helper | Note |
|---|---|---|---|
| `0x5ea` | `MAAuxSendLevels` | `SetChannelLevelProperty(key, data, 1, idx)` | Direct — not bus-rewritten. |
| `0x5eb` | `MAInputPan2` | `SetChannelLevelProperty` | Per-input pan; range-checked against `vtable[+0x5c]`. |
| `0x5ec` | `MA<bus>Levels` | `ChooseMixerBusAndChannelIndex("Levels")` → key | Mixer-bus-dependent (SW-return, Input, …). |
| `0x5ed`, `0x5f7` | `MARotaryControlV2` | `ChooseRotaryPairPhaseAndBitIndex`; read-modify-write 64-bit mask | Single-bit toggle; shared "virtual rotary" state. |
| `0x5ee` | — | `SetMixerPairBooleanProperty(key="Link", …)` | — |
| `0x5ef` | — | `SetMixerPairBooleanProperty("Solo")` | **P5-confirmed Solo of mixer pair.** |
| `0x5f0` | — | `SetMixerPairBooleanProperty("Mute")` | — |
| `0x5f1` | `MA<bus>RoutingV2` | `ChooseMixerBusAndChannelPair("RoutingV2")`; byte-array → bitmask | Encodes an N-byte routing selection into a bitmask. |
| `0x5f4` | `MA<bus>Levels` | `ChooseOutputBusAndChannelIndex("Levels")` | Output-pair level. |
| `0x5f5` | `MAAuxOutputPan2` | `SetChannelLevelProperty` with idx offset by `(vtable[+0x18] − 2)` | Aux-output pan. |
| `0x5f8` | `MA<bus>Link` | `ChooseOutputBusAndChannelPair("Link")` | Output pair link. |
| `0x5f9` | `MAOutputSolo` | `SetPairBooleanProperty` | Hard-coded `MAOutputSolo` (not bus-rewritten). Range: `idx ≤ (vtable[+0x18] − 2) − vtable[+0x60]`. |
| `0x5fa` | `MA<bus>Mute` | `ChooseOutputBusAndChannelPair("Mute")` | — |
| `0x5fb` | `MAMonitorAuxOnOutput` | `SetChannelLevelProperty(…, idx, bool)` | Written as `val != 1` (inverted). |

### 3.3 New NSString keys surfaced (not in P3's 21-entry table)
Found inline in `SetHWSetting` cookie switch:

- `MAMetaResetToFactorySettings`
- `MAMetaVirtualRotateDataKnob`
- `MAGlobalMute`
- `MAGlobalDim`
- `MAInputPan2`
- `MAAuxOutputPan2`
- `MASPDIFMixerBypass`
- `MAEnableStereoPairEngines`

Plus suffix-tokens consumed by `Choose*BusAndChannel*` helpers to synthesize bus-specific keys at runtime: `Levels`, `Link`, `Solo`, `Mute`, `RoutingV2`. The full P3 names (e.g. `MASWReturnLevels`) are produced by concatenating `MA<busName>` + suffix.

### 3.4 `MASampleRate` is write-only; no reverse handler
Note `MASampleRate` is **not** in `InstallPropertyChangeHandlers` (§2). That matches A1-Q15's "setProperties fast-path" finding: `FWAVCAudioDevice::setProperties` siphons `MASampleRate` to the engine's rate change *without* re-publishing it. The panel's view of the current rate instead reads cookie `0x601` via the next property-change notification carrying `MARateLocked` (through `ReceiveRateLocked`), or on-startup via `GetHWSetting(0x601)`. **Important:** `ReceiveSampleRate` is **named** but is **not** key-registered — it's dead code or invoked only by a direct call path we haven't yet found; the panel doesn't react to kext-side `MASampleRate` changes.

## 4. Clone implication (kext side)

The kext must accept the **union of P3's 21 writable keys + §3.3's 8 new scalar keys + §3.2's bus-expanded suffix forms (`MA<bus>Link`, `MA<bus>Mute`, `MA<bus>RoutingV2`, `MA<bus>Levels` where `<bus>` ∈ {`Input`, `SWReturn`, `Output`, `AuxOutput`, `Headphone`, …})** on `IORegistryEntrySetCFProperties`. The authoritative union has to be lifted from a cross-reference pass over all `Choose*BusAndChannel*` helpers (bodies are small — ~130 B each; defer to P7-fup2 if we end up needing byte-for-byte parity for arbitrary M-Audio devices; **not required for FW1814-only clone** because the FW1814 bus set is already enumerated in P3 and P5).

Reverse-direction behaviour the kext must produce:
- Property-change notifications (A2-P2 `bodyType=0`) carrying any of the 20 registered keys from §2 are consumed by `CXDevice`.
- Rate-lock updates must flow as `MARateLocked` (not `MASampleRate`) to make it to the panel's rate indicators.
- Rotary control uses `MARotaryControlV2` as a 64-bit mask; the kext owns authoritative bit layout.

**Deferred to future scope (NOT part of P7-fup):**
- `CXDevice::GetHWSetting` (3804 B) — read-side cookie→NSString table. Mechanically a mirror of §3 using the `Get*Property` variants; decode on-demand if a specific widget read-path becomes a question.
- `CXDevice::GetHWIndexedSetting` (1899 B) — ditto for indexed reads.
- `ChooseMixerBusAndChannelIndex` / `ChooseOutputBusAndChannelPair` / `ChooseMixerBusAndChannelPair` / `ChooseOutputBusAndChannelIndex` (130 B each) — bus-NSString assembly. Decode when we need the exact bus-name alphabet (likely at kext property-schema-finalization time).

## 5. Updates

- **P7 §6 — resolved.** The translator is `CXDevice::{Get,Set}HW[Indexed]Setting`; it's a *CMiddleWareDevice vtable override*, not a listener. The LBroadcaster `0x3F2`/`0x3F3` chain exists for *panel-view ↔ panel-view* coherence inside the Carbon window, not for the kext seam.
- **P6-fup2 — tightened.** `0x601` is bidirectional; kext → UI arrives as `MARateLocked` (via `ReceiveRateLocked`) or `MASampleRate` (via dead-code `ReceiveSampleRate`); UI → kext arrives as `MASampleRate` (via `SendNewSampleRateTimer` on the deferred `LTimerTask`).
- **P3 — extend.** Writable NSString universe grows from 18 to **26 scalar keys** + bus-expanded suffix family (see §3.3). The P3 table in `P3-property-key-strings.md` remains correct for *the registered reverse-direction handlers*; forward-direction keys are a strict superset.
