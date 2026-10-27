# P8 — Sample-rate + buffer-size change path (and the FW1814 "Global Settings" tab)

> **Correction (2026-10-26, R135):** cookie `0x614` is `MAEnableStereoPairEngines`; on the FireWire 410's panel its checkbox reads **"show 410's stereo pairs as devices"** (capture 103) and the 410's `LoadAsioDmEnableControl` is empty. The "ASIO-DM checkbox" name below comes from the code's `AsioDm` naming; the label on the FW1814's own panel is **unverified** (screenshot the 1814 hardware tab on the SL rig to settle it).

> **Correction (2026-10-17) — the clone's panel now follows §6** (evidence: `PROVENANCE-RESOLUTIONS.md` §R57, captures 79-81). The clone's panel had written `MASampleRate` through `setProperties` (the "P7-fup §3.1" reading this doc already corrected), which applies the rate to the engine without completing the kext's pending state (`MARateLocked` stays 0, `MASampleRate` stale; the same happens on the official kext, capture 80). Fixed: `HALRate`/`DeviceModel` set the rate with CoreAudio `setNominalSampleRate:` 1 s after the last pick and display CoreAudio's rate; the lock colour is `MARateLocked`. Confirmed by the user in a manual test.

> **Correction (2026-10-16):** the v1.9 screenshots (`P12-panel-screens-v19.md`) show no Global Settings tab; the virtual rotary assignment is the sidebar `level controller` (sw rtn / output / input / aux send) with per-strip `ctrl` buttons, and the Hardware tab has no buffer-size control (CoreAudio sets it in the DAW).

> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Message codes, cookies 0x603/0x604 and the CoreAudio `setNominalSampleRate:` path verified against the i386 asm/strings.


**Status:** done.
**Scope:** Decode the sample-rate and buffer-size change paths, as the seed required. Along the way, correctly re-identify the FW1814 "Global Settings" tab's contents (it's the **Virtual Rotary Controller Assignment** picker, not rate/buffer).

**Decompile log:** `ghidra-exports/decomp/a2-p8.log` — 17 functions from `CHardwareView`, `CDeviceSettings`, `CXDevice`, `CFW1814GlobalSettingsView`.

## 1. Scope correction — the P8 seed was mis-targeted

The seed said: *"P8 — `CFW1814GlobalSettingsView` — sample-rate / buffer-size change path (MASampleRate fast-path + buffer-size cookie)."*

The code says: sample-rate + buffer-size widgets live on the **Hardware tab** (base `CHardwareView` @ `/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/Source/Shared/ControlPanel/CHardwareView.cp`), **not** in `CFW1814GlobalSettingsView`. The FW1814 subclass of `CHardwareView` (P5) only overrides clock-source / optical-source / MOM / ADAT bits — it inherits rate + buffer handling verbatim from the base.

`CFW1814GlobalSettingsView` is a **completely different tab** — the Virtual Rotary Controller Assignment picker (§4 below).

The P5 doc already touched the Hardware tab's clock-source side; this P8 fills in the rate/buffer side on the same tab.

## 2. UI load — `CHardwareView::Load{SampleRate,BufferSize}{Control,Display}`

| Widget | ResID | this-offset | Type |
|---|---|---|---|
| Sample-rate display (text) | `0x13ed` | `+0x124` | text pane (RTTI `001630f4`) |
| Sample-rate control (radio-group) | `0x140a` | `+0x128` | radio-group (RTTI `00163184`) |
| Bit-depth control | `0x13fe`-ish | `+0x12c`-ish | per `LoadBitDepthControl @ 0xd6a6c` |
| Buffer-size control (radio-group) | `0x13f7` | `+0x134` | radio-group (RTTI `00163180`) |
| Buffer-size secondary (text?) | `0x13f8` | `+0x138` | aux pane (RTTI `00163188`) |
| Clock-rate text header | `0x13f6` | — | repainted via `vtable[+0xbc]` (PaneDrawn) |
| Clock-source radio-group | — | `+0x13c` | handled in P5 |
| ASIO-DM checkbox | — | `+0x14c` | §3 — msg `0x1466` → cookie `0x614` |
| Stereo-pair-engines checkbox | — | `+0x150` | §3 — msg `0x1465` → cookie `0x613` |
| CDeviceSettings backlink | — | `+0x11c` | SetCPSetting target |
| Suppress-echo flag | — | `+0xb4` | §3 — guards against UI↔cookie feedback loop |

Dynamic-cast failures `UDebugging::DebugThrow(0x6e696c50 /* 'Pnil' */, ..., 0x86)` fall through silently — the control is nulled and later `Adapt*` becomes a no-op.

## 3. `CHardwareView::ListenToMessage` @ `0xd7108` — the 968 B cookie switch

This is the hardware tab's central dispatcher for **both** inbound cookie broadcasts (kext → UI) and outbound widget clicks (UI → kext). The `this[+0xb4]` field is a one-shot "suppress echo" guard: set to 1 by the UI side before writing a cookie so the matching inbound broadcast doesn't bounce back and re-set the widget.

### 3.1 Inbound — cookie broadcast from `CDeviceSettings` LBroadcaster

| Msg | Cookie | Payload format | Side-effect |
|---|---|---|---|
| `0x5fe` | `0x5fe` (MOM) | byte at `*(char**)(body+0xc)` | `(this+0x13c)->vtable[+0xcc]` if 0 else `+0xc8` — toggles clock-source radio enable/disable |
| `0x5ff` | `0x5ff` (buffer size — 2 B) | `*(ushort**)(body+0xc)` | Calls `(buffer-size-widget)->vtable[+0x1ec](idx)` via `MapClockSourceButtonToSetting` equivalent at `vtable[+0x264]` — selects radio-group button by setting value |
| `0x601` | `0x601` (sample rate — 4 B) | `*(uint**)(body+0xc)` | Reads cookie `0x602` (bit-depth), calls `vtable[+0x268](rate, bitDepth)` = `CHardwareView::AdjustClockRateText @ 0xd78e6` (repaints the "48.000 kHz @ 24-bit" text) |
| `0x602` | `0x602` (bit depth — 2 B) | `*(ushort**)(body+0xc)` | Mirror of 0x601 — reads cookie 0x601 current rate, calls same `vtable[+0x268]` |
| `0x613` | ASIO-DM enable (1 B) | `*(char**)(body+0xc)` | `(this+0x150)->vtable[+0x24](byte)` — SetValue on checkbox |
| `0x614` | `MAEnableStereoPairEngines` (1 B) | `*(char**)(body+0xc)` | `(this+0x14c)->vtable[+0x24](byte)` — SetValue on checkbox |

After each inbound handler, `this[+0xb4]` is flipped to 1 to arm the suppress for the next *outbound* write.

### 3.2 Outbound — widget-click messages
Message IDs ≥ `0x1451` are PowerPlant control-click events (resource-ID-derived message codes):

| Msg | Widget | Cookie | Flow |
|---|---|---|---|
| `0x1451` | Clock-source radio (P5) | `0x5ff` (2 B) | `MapClockSourceButtonToSetting` → `GetCPSetting(0x5ff)` → if differs → `SetCPSetting(0x5ff, idx, 2, &newVal, 0)` |
| `0x1465` | Stereo-pair-engines checkbox | `0x613` (1 B) | GetValue widget → read current `0x613` → if differs → `SetCPSetting(0x613, 0, 1, &bool, 0)` |
| `0x1466` | ASIO-DM checkbox | `0x614` (1 B) | GetValue widget → read current `0x614` → if differs → `SetCPSetting(0x614, 0, 1, &bool, 0)` |

**Note the opcode-field swap.** `0x1465` writes cookie `0x613`, `0x1466` writes cookie `0x614`. The *outbound* mapping is the inverse of the widget-field assignments in §3.1 (where `0x613` was shown on `this+0x150` = stereo-pair checkbox). i.e. **the stereo-pair-engines widget actually publishes cookie `0x614`, and the ASIO-DM widget publishes cookie `0x613`** — the P7-fup §3.1 label `0x614 → MAEnableStereoPairEngines` is correct; `0x613` is the **ASIO Direct Monitoring** enable (no distinct NSString found yet; probably rides on `MASPDIFMixerBypass` or a sibling of it).

### 3.3 Sample-rate + buffer-size widget → kext — where is it?

**Not in `ListenToMessage`.** The buffer-size radio-group click fires message `0x1451` only for clock-source (not buffer-size). Rate and buffer widgets must dispatch via a different hook — most likely `CHardwareView::ListenCoreAudioDeviceControls @ 0xd70c6` (66 B, not decoded) or the PowerPlant control-command vtable override on the radio-group's own listener subobject. The *inbound* handling in §3.1 is present and correct — the panel re-paints its radio-group whenever the kext broadcasts a new rate/buffer-size, confirming the typed-property wire format.

**Rate-write path (confirmed from P7-fup + §5 here):** when the user picks a rate in the radio-group, the widget's `LCommander` subobject calls `CXDevice::SetHWSetting(0x601, 4, &rateHz)` directly (not via CDeviceSettings) — see P7-fup §3.1 "0x601" case: it caches `*(uint*)(this+0x84) = rateHz` and installs the 1-second deferred `LTimerTask` at `this+0xf8` whose callback is `CXDevice::SendNewSampleRateTimer`.

**Buffer-size write path:** uses cookie `0x5ff` (2 B index into a buffer-size table that the device publishes). Not plumbed through to the CoreAudio side in `SetHWSetting` — reaches the kext only via the normal setProperties path.

## 4. `CDeviceSettings::LoadSupportedSampleRates` @ `0x664b0` — rate-table RPC

Called once per Hardware-tab open. Flow:

```
count = SetCPSetting(0x603, 4, &u32)         // kext tells panel how many rates
if ok and count > 0:
    buf = new u32[count]
    SetCPSetting(0x604, count*4, buf)        // kext fills array
    this+0x7c = count
    this+0x80 = buf
```

**Two new read-only cookies surface here that are NOT in P7 or P7-fup:**
- `0x603` (4 B u32) — count of supported sample rates
- `0x604` (count·4 B u32[]) — the rate list in Hz

Followed by `GetSampleRateByIndex @ 0x665a2` (bounds-checked array read) and `GetSampleRateIndex @ 0x665d8` (reverse lookup rate→index). The panel's rate radio-group buttons are generated from this list at tab-build time.

## 5. `CDeviceSettings::LoadSupportedBufferSizes` + `GetBufferSizeByIndex` — stubs

Base implementations are degenerate: `LoadSupportedBufferSizes` just sets `this+0x6c = 0` and returns 1; `GetBufferSizeByIndex` returns 0.

**`CFW1814DeviceSettings` does NOT override either method** (verified by absence in `functions.tsv`). This means the FW1814 panel's buffer-size radio-group buttons are **hard-coded in the UI resource (nib/rsrc)**, not fetched from the kext. The kext only sees the user's choice as cookie `0x5ff` (2 B index into the panel's own list).

**Clone implication:** our 2026 panel must ship the exact buffer-size ladder `64/128/256/512/1024/2048/4096` from auto-memory → `CFW1814BufferSizeChoices.md` as *UI resource* data, and interpret cookie `0x5ff` as an index into that fixed list, not as a byte count.

## 6. `CXDevice::SendNewSampleRateTimer` @ `0x6c8c8` — the deferred rate push

Fires 1 second after the last `SetHWSetting(0x601)` call:

```c
void SendNewSampleRateTimer(LTimerTask *param_1) {
    [param_1[+0x60] setNominalSampleRate: (double)param_1[+0x84]];
    LTimerTask::Remove(param_1 + 0xf8);
}
```

Critical finding — **the rate change is NOT sent via `IORegistryEntrySetCFProperties(MASampleRate=X)`** as P7-fup §3.1 incorrectly stated. It goes through **CoreAudio** directly: the retained object at `CXDevice+0x60` is an `AudioObject`-wrapping NSObject (allocated by `CXDevice::FetchCoreAudioDevices @ 0x6c920`), and `setNominalSampleRate:` is a CoreAudio HAL message that eventually lands in the kext's `IOAudioDevice::setSampleRate` → `IOAudioEngine::performFormatChange` path.

This matters for the clone because:
1. The panel does **not** publish `MASampleRate` as a property; it drives the audio HAL.
2. The kext's rate-change hook must be wired through `IOAudioEngine::performFormatChange`, not through a setProperties dispatcher.
3. The 1-second deferral is a UX debounce — multiple radio clicks within 1 s collapse to one rate change.

The `.ulonglong|DAT_000f7d38) - DAT_000f7d38` idiom in the decompile is the classic u32→double conversion using IEEE-754 bias.

**P7-fup §3.1 correction:** replace `"0x601 | MASampleRate (deferred via LTimerTask) | 1s-delayed MASampleRate property push"` with `"0x601 | CoreAudio setNominalSampleRate: on CXDevice+0x60 | 1s-delayed; NOT a property write"`. Updated below.

## 7. FW1814 "Global Settings" tab — Virtual Rotary Controller Assignment

Now that rate/buffer is out of this tab's scope, the actual FW1814-only Global Settings content:

### 7.1 Widget layout (`LoadControllerAssignBtns @ 0x84694`)

Five controls:

| ResID | this-offset | Role |
|---|---|---|
| `0x9c5` | `+0x13c` | Master radio-group (contains the four buttons) |
| `0x9c6` | `+0x158` | Button → setting `1` (default) |
| `0x9c7` | `+0x15c` | Button → setting `2` |
| `0x9c8` | `+0x160` | Button → setting `3` |
| `0x9c9` | `+0x164` | Button → setting `5` |

Each loaded via dynamic-cast with `UDebugging::DebugThrow('Pnil')` fall-through if the dynamic_cast misses.

### 7.2 Setting ↔ button mapping (`MapAssignSettingToButton` / `MapAssignButtonToSetting`)

| Setting | ResID |
|---|---|
| 1 (default) | `0x9c6` |
| 2 | `0x9c7` |
| 3 | `0x9c8` |
| 5 | `0x9c9` |

**No setting value of 4** — this is the FW1814-specific gap (other devices like FW410 probably fill it; FW1814's rotary knob has only 4 assignable targets).

### 7.3 Listener wiring (`ListenToControllerAssignBtns @ 0x848fa`)

Adds `this+0x88` (embedded LListener) to each of the four button-broadcasters at `+0x158/+0x15c/+0x160/+0x164`. The master radio-group at `+0x13c` is not listened to directly — the per-button broadcasters fire first, and the radio-group's own `vtable[+0x1e8]` (seen in P5/P6) reads back the selected button.

### 7.4 Translation to the kext

**Cross-reference with P7-fup §3.1:** the setting value `{1,2,3,5}` is written via `SetHWSetting(0x5e4, 8, &data)` as `MAMetaVirtualRotateDataKnob`. The 8-byte payload is `(rotateDirection, stepsInput)` — see P7-fup line "steps = 13 − hi/6 if hi<78 else 1", where `hi = payload[1]` is the number of *detents* the knob is being rotated. The "setting" (1/2/3/5) selects *which parameter the rotary controls*, not the amount.

So the FW1814 panel's Global Settings tab lets the user pick one of {Volume, Pan, Aux1, Aux2} (or similar — exact labels live in the .rsrc string table) as the target of the device's single hardware "data knob", and the kext receives the choice as an `MAMetaVirtualRotateDataKnob` property write with setting as the "direction" byte.

## 7a. macOS-only scope — ASIO + driver buffer-size are Windows-only vestigial

The shared Win/Mac M-Audio panel codebase surfaces widgets that only matter on Windows. On our macOS/CoreAudio target:

- Cookie `0x5ff` (buffer-size, 2 B index) — **Windows-only.** On Mac, buffer size lives in the DAW via CoreAudio `IOBufferFrameSize` (per-client, negotiated through the AU/HAL). Our panel should NOT expose a buffer-size radio-group; our kext should NOT publish a buffer-size list and may ignore cookie `0x5ff` writes.
- Cookie `0x613` (ASIO Direct Monitoring enable, 1 B) — **Windows-only.** ASIO is a Windows audio API; macOS equivalents (direct-monitor via aggregate/pass-through) are CoreAudio-side, not driver-toggled. Drop the widget; don't honour the cookie.
- Cookie `0x614` (`MAEnableStereoPairEngines`, 1 B) — **Keep.** Affects CoreAudio channel grouping (stereo-pair vs independent mono); still relevant on Mac.

The buffer-size ladder from auto-memory (`64/128/256/512/1024/2048/4096`, default 512) applies only to the kext's `IOAudioStream` safe-sample-sizes list — it's a *CoreAudio capability* publication, not a panel widget.

## 8. Clone implications (kext side)

1. **Publish rate-table properties.** Two new read-only cookies / properties required:
   - `0x603` → u32 count of supported rates
   - `0x604` → u32[count] rate list in Hz
   These are read once per Hardware-tab-open by `CDeviceSettings::LoadSupportedSampleRates`. The kext's P7-fup NSString map does not include these — they must bind to a different NSString pair (candidates: `MASupportedRates`, `MARatesCount` — not yet confirmed by panel scan). **Open as P8-fup if we need the exact NSStrings.**

2. **Rate change is NOT a property write.** The kext must honour `IOAudioEngine::performFormatChange` (CoreAudio path) as the authoritative sample-rate-change hook. The 2014 driver's property-write path for `MASampleRate` is used by `FWAVCAudioDevice::setProperties` as a *siphon-and-forward* mechanism — see A1-Q15 — but the panel itself never sends `MASampleRate`.

3. **Buffer size is UI-local — AND Windows-only on this platform.** See §7a. The 2014 panel's buffer-size radio-group is Windows-only; on macOS the DAW controls buffer size via CoreAudio. Our kext publishes the ladder as `IOAudioStream` safe-sample sizes (CoreAudio capability), not as a cookie.

4. **ASIO-DM cookie `0x613` — drop entirely.** Windows-only (see §7a).

5. **`MAEnableStereoPairEngines`** is bidirectional: panel reads via `GetHWSetting(0x614)` and writes via `SetHWSetting(0x614)` from the Hardware-tab checkbox at `+0x14c`.

6. **`MAMetaVirtualRotateDataKnob`** has two independent inputs on the panel: the hardware data-knob emits step events (cookie `0x5e4` 8 B); the Global Settings tab picks the target parameter (also cookie `0x5e4` 8 B, but with a different byte layout — the "setting" value in `{1,2,3,5}`). The kext must disambiguate by inspecting the payload shape.

## 9. Updates to prior docs

- **P7-fup §3.1 — correction:** cookie `0x601` deferred write goes to **CoreAudio `setNominalSampleRate:`**, not `MASampleRate` property. Documented inline above (§6).
- **P5 — extend:** the Hardware-tab also hosts ASIO-DM checkbox (`0x1465` → cookie `0x613`) and stereo-pair-engines checkbox (`0x1466` → cookie `0x614`), both missing from P5's widget inventory. Add on next P5 touch; low priority.
- **README — close P8; promote P8-fup (rate-table NSString pair) and P9 next.**

## 10. P8-fup closed — rate cookies 0x603/0x604 are CoreAudio-synthesized, not kext properties

Decoded from `CXDevice::GetHWSetting` @ `0x6e2ca` (log `a2-tail.log:347-380`):

```c
case 0x603:  // rate count
    *(uint32_t*)param_3 = [this->fAudioObject.supportedSampleRates count];
    break;

case 0x604:  // rate list u32[count]
    NSArray *rates = [this->fAudioObject supportedSampleRates];
    for (i = 0; i < rates.count; i++)
        param_3[i] = (uint32_t)[[rates objectAtIndex:i] doubleValue];
    break;
```

**There is no NSString key for these cookies.** The panel reads them directly from the `MXDevice`/`AudioObject` wrapper (`this + 0x60`) via `-supportedSampleRates`, which proxies to CoreAudio HAL's `kAudioDevicePropertyAvailableNominalSampleRates`.

**Clone implication (correction to §8.1):** our kext does **not** need to publish `0x603`/`0x604` as properties. Once our `IOAudioStream` exposes supported sample rates to CoreAudio (which it must anyway), HAL surfaces them, the panel reads them from HAL via `-supportedSampleRates`, and the cookies "work" automatically. This closes P8-fup.

The cookie IDs `0x603`/`0x604` still exist as a dispatcher-internal address space inside `GetHWSetting`, but they are never NSString-translated — their handlers are implemented in the panel-side function body and read HAL-side state. Same pattern applies to cookie `0x601` (current rate) which reads cached `this[+0x84]` set at ctor from `[audioObject nominalSampleRate]`.
