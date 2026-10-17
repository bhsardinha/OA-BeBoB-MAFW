# A2-P3 — Property-key table and property-dict RPC (closes A1-Q15a)

> **Correction (2026-10-14) — MA* crosswalk** (evidence: [Q-MAMAP-key-crosswalk.md](../A1-ghidra/Q-MAMAP-key-crosswalk.md), [PROVENANCE-RESOLUTIONS.md §R20](../PROVENANCE-RESOLUTIONS.md)). For the FW1814 the mixer is **not** a single path: levels (SW return 0x00, output 0x08, input 0x10, headphone 0x38, aux output 0x34, aux send 0x64), input pan (0x40+), input routing (0x90), headphone selector (0x98) and aux monitor (0x9c) are vendor register writes via `CustomMixerWriteQuads` to `0xFFC7:0x00700000+offset`; **`MAAuxOutputPan2` (FWSettingsAuxSend::SetAuxPan → `AVCControlFeatureFunctionBlock`) and `MASWReturnRoutingV2` (base `FWSettingsLevels::SetSWReturnRouting` → `SetAVCRouting` → `AVCMixerConnectPair`) go over AV/C.** Mute/solo/dim are level operations (minimum / −10 dB). Level wire value = property level (−255..0, 0.5 dB) × 0x80, int16 big-endian, two channels per quadlet.


**Scope:** The complete set of NSString keys the panel uses to drive the kext via `IORegistryEntrySetCFProperties` / `IORegistryEntryCreateCFProperties`, and how writes are coalesced.

**Decompile log:** `ghidra-exports/decomp/a2-batch1.log` — `CXDevice::InstallPropertyChangeHandlers` @ 0006ca12, `CXDevice::Get32BitNumberProperty` @ 0006d660, `CXDevice::Set32BitNumberProperty` @ 0006d7c6, `CXDevice::HaveQueuedProperties` @ 0006dafa, `CXDevice::SendQueuedProperties` @ 0006db2c, `CMiddleWareDevice::{Get,Set}CP[Indexed]Setting` @ 0006378a / 000637de / 00063832 / 0006388a.

## Property key table

`InstallPropertyChangeHandlers` populates `this[+0x74]` (NSMutableDictionary) with a key → C-function mapping, one per property the panel listens for. All 21 keys below are literal `cf_...` NSString constants in `__cstring`:

| NSString key              | Receive callback                | UI binding                             | Encoding |
|---------------------------|---------------------------------|----------------------------------------|----------|
| `MASWReturnLevels`        | `ReceiveSWReturnLevelChange`    | Mixer SW-return faders                 | NSArray of NSNumber (per channel pair) |
| `MAInputLevels`           | `ReceiveInputLevelChange`       | Mixer input faders                     | NSArray |
| `MAOutputLevels`          | `ReceiveOutputLevelChange`      | Output tab faders                      | NSArray |
| `MAHeadphoneLevels`       | `ReceiveHeadphonesLevelChange`  | Headphone-out faders                   | NSArray |
| `MAAuxOutputLevels`       | `ReceiveAuxOutputLevelChange`   | Aux-out faders                         | NSArray |
| `MAAuxSendLevels`         | `ReceiveAuxSendLevelChange`     | Aux-send faders (per input strip)      | NSArray |
| `MASWReturnSolo`          | `ReceiveSWReturnSolo`           | SW-return solo buttons                 | NSNumber (bitmask) |
| `MASWReturnMute`          | `ReceiveSWReturnMute`           | SW-return mute buttons                 | NSNumber (bitmask) |
| `MAInputSolo`             | `ReceiveInputSolo`              | Input solo buttons                     | NSNumber (bitmask) |
| `MAInputMute`             | `ReceiveInputMute`              | Input mute buttons                     | NSNumber (bitmask) |
| `MAOutputSolo`            | `ReceiveOutputSolo`             | Output solo buttons                    | NSNumber (bitmask) |
| `MAOutputMute`            | `ReceiveOutputMute`             | Output mute buttons                    | NSNumber (bitmask) |
| `MAHeadphoneMute`         | `ReceiveHeadphoneMute`          | Headphone mute                         | NSNumber (bool) |
| `MAAuxOutputMute`         | `ReceiveAuxOutputMute`          | Aux-out mute                           | NSNumber (bool) |
| `MAMonitorAuxOnOutput`    | `ReceiveMonitorAuxOnOutput`     | "Monitor aux on output" checkbox       | NSNumber (bool) |
| `MASWReturnRoutingV2`     | `ReceiveSWReturnRouting`        | SW-return destination picker           | NSNumber (routing selector byte) |
| `MAInputRoutingV2`        | `ReceiveInputRouting`           | Input destination picker               | NSNumber |
| `MAHeadphoneSourceV2`     | `ReceiveHeadphoneSource`        | Headphone A/B source picker            | NSNumber |
| `MAClockSource`           | `ReceiveClockSource`            | Clock-source popup (Internal/SPDIF/ADAT/WordClock) | NSNumber (0..3) |
| `MARateLocked`            | `ReceiveRateLocked`             | "Rate locked" indicator LED            | NSNumber (bool) |
| `MARotaryControlV2`       | `ReceiveChannelMask`            | Hardware rotary → selected input mask  | NSNumber (bitmask) |

Three additional keys appear in supporting code paths but have **no** handler installed (they are read-only, consumed during ctor):

- `MAMetaAllProperties` — initial-state slurp. When the kext boots the panel-side, the kext publishes the whole dict under this synthetic umbrella key; the panel reads and seeds `this[+0x6c]` with it.
- `MAFirmwareInfo` — 80-byte firmware descriptor (A1-Q7a).
- `GUID` — IORegistry match key (also used by the matching dictionaries built in P1).

## Scalar accessors

`CXDevice::Get32BitNumberProperty(NSString *key)`:

```
cached = [this->cachedPropertyDict objectForKey:key]
if (cached) return [cached unsignedIntValue]
raw = IORegistryEntryCreateCFProperty(this->serviceEntry, key, NULL, 0)
val = [raw unsignedIntValue]
[this->cachedPropertyDict setObject:raw forKey:key]
CFRelease(raw)
return val
```

`CXDevice::Set32BitNumberProperty(NSString *key, uint32_t val)`:

```
num = [NSNumber numberWithUnsignedInt:val]
[this->cachedPropertyDict setObject:num forKey:key]        /* local cache */
[this->dirtyKeys addObject:key]                            /* queue */
```

**Writes are not pushed immediately.** They sit in `this->dirtyKeys` (NSMutableSet at `this[+0x70]`) until `SendQueuedProperties` is called at a UI commit boundary (slider mouse-up, popup selection).

## Queued batch flush (`SendQueuedProperties` @ 0006db2c)

```
if ([this->dirtyKeys count] == 0) return
batch = [NSMutableDictionary dictionary]
for key in this->dirtyKeys:
    batch[key] = [this->cachedPropertyDict objectForKey:key]
IORegistryEntrySetCFProperties(this->serviceEntry, batch)
[this->dirtyKeys removeAllObjects]
```

The call target is `IORegistryEntrySetCFProperties`, which lands in the kext's `IOService::setProperties(OSObject*)` override. **A1-Q11b showed that FWAVCAudioDevice::setProperties only dispatches `MASampleRate`.** Every other key must therefore be handled by a *different* IOService in the registry — most likely `MAServiceClient` itself (the user client), or an intermediate `FWAudioDevice::setProperties` override we have not yet decoded.

**Q15b resolved (see `../A1-ghidra/Q15b-setproperties-dispatch.md`):** `FWAVCAudioDevice::setProperties` siphons `MASampleRate` as a fast-path then **unconditionally tail-calls** `FWAudioDevice::setProperties` → `FWPropertyDict::SetProperties @ 0xa0e0` which iterates the dict and routes every key through `SetOneProperty` into typed-property wrappers (A1-Q15). **There is no `FWMixer::setProperties`** — hypothesis #3 above was wrong. All feature behaviour lives inside typed-property `Apply()` methods invoked from the workloop (`checkForWork` → `ApplyModifiedProperties`).

## PowerPlant plumbing (CMiddleWareDevice ↔ CXDevice)

The Carbon-side (PowerPlant) widgets talk to the Objective-C `CXDevice` bridge via two LBroadcaster message IDs:

| Opcode | Direction           | Dispatcher                                   |
|-------:|---------------------|----------------------------------------------|
| 0x3F2  | widget → kernel     | `CMiddleWareDevice::SetCPSetting` / `SetCPIndexedSetting` broadcasts; `CXDevice::Receive32BitValueChange` listens, routes to `Set32BitNumberProperty` |
| 0x3F3  | widget ← kernel     | `CMiddleWareDevice::GetCPSetting` / `GetCPIndexedSetting` broadcasts; `CXDevice` resolves via `Get32BitNumberProperty` and returns the current value |

Indexed variants pack `(selector, index)` into a 32-bit cookie — this is how a single key like `MAInputLevels` with an N-channel NSArray body is addressed per-channel from Carbon.

## Clone implication for OpenAudio kext property surface

To make our kext work with the existing panel (patch-the-reference strategy) **or** to make our new panel work with the existing kext (bootstrap-from-reference strategy), the kext must:

1. Publish all 21 `MA*` keys as IORegistry properties on the `FWAudioDevice` entry.
2. Honour `IORegistryEntrySetCFProperties` writes on every `MA*` key (not just `MASampleRate`) — this is where Q15b lands.
3. Serialize an `MAMetaAllProperties` umbrella NSDictionary at matching time so the panel can seed its cache in one shot.
4. Broadcast a `bodyType == 0` Mach-port notification (see P2) with the changed key's ID whenever any `MA*` property changes server-side.

## Status

Closes **A1-Q15a** (identifies all `setProperties` keys actually used). Opens **Q15b** (which IOService in the kext dispatches the non-sample-rate keys).
