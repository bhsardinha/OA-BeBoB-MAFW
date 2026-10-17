# A2-P5 — `CFW1814HardwareView` → kext RPC mapping

> **Correction (2026-10-16):** the "MOM group" row (cookie `0x608`, widgets `0x14b5..0x14b7`) maps to `MAMomentarySwitchSource` (`CXSelectorHPDevice::SetHWSetting @0x73906`: value `==3` → 1), the a/b switch function, **not** `MAMonitorAuxOnOutput` and not "Mix-Out-to-Monitor". See `P13-translator-index-map.md` §5.

> **Correction (2026-10-16):** user screenshots of the v1.9 panel (see `P12-panel-screens-v19.md`) show `adat S/MUX` greyed when output = S/PDIF, which contradicts the "enabled when optical-in OR optical-out is ADAT" rule below. Unresolved (version skew possible); check the `AdaptPortsToSettings` decompile against v1.9. Also: the v1.9 hardware tab has no buffer-size control.

**Scope:** The Hardware tab: clock source, sync source, optical in/out type, active-input toggle, MOM (Mix-Out-to-Monitor), ADAT 4-ch mode toggle, headphone A/B source. Maps UI button IDs to `CDeviceSettings` cookies (which in turn become the `MA*` NSString property keys from P3).

**Decompile log:** `ghidra-exports/decomp/a2-p5.log` — `CFW1814HardwareView::{LoadUIElements, LoadSyncSourceBtns, ListenToUIElements, ListenToSyncSourceBtns, ListenToMessage, AdaptPortsToSettings, AdaptUIElementsToSettings, MapClockSourceButtonToSetting, Map{MOM,OpticalInput,OpticalOutput,ActiveInput}SettingToButton, UpdateHPSourceButtonLabels}` @ `0x51250..0x5307c`.

**Up-front correction:** the P5 seed question listed "phantom power" and "nickname" as hardware-view features. **Neither exists in the panel binary** — `grep -i phantom|nickname` returns zero hits. The FW1814 hardware tab does not expose phantom power (hardware switch on the device) and has no software nickname field.

## UI button → setting-cookie table

Mouse-click on a radio button broadcasts the button's CommandID (`param_1` to `ListenToMessage`); the handler looks up the current button in the containing radio group (vtable+0x1e8 = `LRadioGroup::GetCurrentRadioID`), maps to a setting value, and writes it via `CDeviceSettings::SetCPSetting(cookie, index=0, size, inBuf, broadcast)`.

| CommandID (group) | Member buttons | Setting cookie | Enum values | Clone kext key (P3) |
|---|---|---|---|---|
| **0x14b5** MOM group     | 0x14b6, 0x14b7                 | **0x608** (2 B) | 1 = off / off-ish, 3 = on                      | `MAMonitorAuxOnOutput` |
| **0x1452..0x1455** clock | 4 buttons via base `CHardwareView` | base `CHardwareView` cookie (base class RPC) | 0..3 → see table below | `MAClockSource` |
| **0x157d** optical-in    | 0x157e (S/PDIF), 0x157f (ADAT) | **0x60d** (2 B) | 1 = S/PDIF, 2 = ADAT                           | (optical-in sub-key of routing dict) |
| **0x1587** optical-out   | 0x1588 (S/PDIF), 0x1589 (ADAT) | **0x60e** (2 B) | 1 = S/PDIF, 2 = ADAT                           | (optical-out sub-key) |
| **0x1591** active-input  | 0x1592, 0x1593                 | **0x60f** (2 B) | 1 / 2 (channel-pair selector)                  | (per-input routing) |
| **0x159b** ADAT-4ch mode | single checkbox (vtable+0x20 is `IsChecked`) | **0x611** (1 B) | bool                                        | (ADAT bit of routing) |
| **0x15ae** HP A/B picker | single push-button              | opens `CFW1814HPABSourceDialog` modal          | *dialog writes* `MAHeadphoneSourceV2` |
| **0x5fe** (bus signal)   | *not a button* — bus-signal from a sibling view when `[+0x5fe]` changes | 1 B bool  | re-triggers `AdaptPortsToSettings`             | *coalesce signal* |
| **0x609** (bus signal)   | HP-source changed             | ushort[2] (A, B channel) | drives `UpdateHPSourceButtonLabels`            | mirrors `MAHeadphoneSourceV2` |

Button-ID → setting-value is implemented with trivial lookup tables (`MapOpticalInputSettingToButton`, `MapOpticalOutputSettingToButton`, `MapActiveInputSettingToButton`, `MapMOMSettingToButton`) — all are 2-entry maps `{1 → low button-ID, 2 → high button-ID, else → 0 (indeterminate)}`. The clock-source map at `0x53056`/`0x5307c` uses a 4-entry data table at `DAT_000f5d94` / `DAT_000f8640` — the panel's canonical button-ID↔enum mapping for clock source.

## `CDeviceSettings` RPC vtable (as called from this view)

- **vtable+0x0c** `GetCPSetting(cookie, index, size, outBuf)` — read current value (returns 0 on success, writes value into `outBuf`).
- **vtable+0x18** `SetCPSetting(cookie, index, size, inBuf, broadcast)` — write value and (when `broadcast != 0`) kick an `LBroadcaster` broadcast to other views.

Pattern repeated for every UI write (verbatim from `ListenToMessage` case 0x14b5):

```c
local_14 = 0;  local_12 = <new value>;
if (GetCPSetting(0x608, 0, 2, &local_14) != 0) return;      /* RPC failure */
if (<new value> == local_14) return;                        /* no-op */
SetCPSetting(0x608, 0, 2, &local_12, 0);                    /* no broadcast; the kext notify fires anyway */
```

The optical-in/out and active-input writes additionally **cross-pull the other two settings and the `0x5fe` bool**, then call `AdaptPortsToSettings(opticalIn, opticalOut, pairActive)` to reconcile the UI widgets that depend on all three. This cross-dependency is the panel's "optical topology" state machine — changing optical-in alone can force the UI to re-enable/disable the optical-out radio.

## `AdaptPortsToSettings(opticalIn, opticalOut, bothPairActive)`

Pure UI widget state — no RPC. For each of four radio-group pointers (`+0x174`, `+0x180`, `+0x190` [i.e. this+400], `+0x18c`):

- If `bothPairActive` is **false** → disable all four (vtable+0xcc = `LPane::Disable`).
- If `bothPairActive` is **true**:
  - `+0x174` (optical-in group) and `+0x180` (optical-out group) always enabled (vtable+0xc8 = `Enable`).
  - `+0x190` (active-input group) is **disabled** when `opticalIn == 2 (ADAT)` — makes sense; ADAT 8-ch locks the input pair.
  - `+0x18c` (ADAT-4ch toggle) is **enabled** when `opticalIn == 2` OR `opticalOut == 2`; **disabled** only when neither port is ADAT. S/MUX applies when any ADAT port is in use — matches manual §16 wording "ADAT S/MUX = ADAT at 88.2/96 kHz".

**Correction (2026-10-09):** earlier revision of this file said `[+0x18c]` was disabled when `opticalOut == 2`. That was inverted — see `docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log:51..89` for the re-decompile. The TRUE branch at `param_1 == 2` disables `[+0x190]` and falls through to **enable** `[+0x18c]`; the FALSE branch enables `[+0x190]` and only disables `[+0x18c]` when `param_2 != 2`. Also confirmed by `P6-mixer-view.md` which observes ch 5–8 are gated off when ADAT is **active** (consistent with S/MUX being an ADAT sub-mode, not a S/PDIF-mode companion).

## `UpdateHPSourceButtonLabels(ushort hpSource[2])`

Composes "A = <n>" / "B = <n>" labels on the two HP source summary buttons.

**Resolved from `Panel.rsrc` STR# 1020 "kHWHeadPhoneSourceStrings" (`ghidra-exports/panel-rsrc.txt:525`):**

| Entry | String   |
|------:|----------|
| 1     | "A = "    |
| 2     | "B = "    |
| 3     | "out 1/2" |
| 4     | "out 3/4" |
| 5     | "out spdif" |
| 6     | "aux"     |
| 7     | ", LED off" |
| 8     | ", LED on"  |

Hard-wired decision in the decompile:

```c
if      (hpSource[i] == 2) idx = 4    /* "out 3/4" */
else if (hpSource[i] == 4) idx = 6    /* "aux"     */
else                        idx = 3    /* "out 1/2" */
```

Values on wire: `1 → out 1/2`, `2 → out 3/4`, `4 → aux` — see also `P11-hpab-source-dialog.md`. No "ADAT" / "S/PDIF" label is actually used by the FW1814 HP picker; those entries exist in the resource for sibling devices.

## `AdaptUIElementsToSettings` (one-shot sync, inverse of `ListenToMessage`)

Called at tab-switch and after a bus-wide kext notification. Pulls all cookies via `GetCPSetting` and drives widget state via `LRadioGroup::SetCurrentRadioID` (vtable+0x1ec) and `LControl::SetValue` (vtable+0x24):

- `0x608` → MOM group `+0x154` (via `MapMOMSettingToButton`)
- `0x60d` → optical-in group `+0x174` (via `MapOpticalInputSettingToButton`)
- `0x60e` → optical-out group `+0x180` (via `MapOpticalOutputSettingToButton`)
- `0x60f` → active-input group `+0x190` (via `MapActiveInputSettingToButton`)
- `0x611` → ADAT-4ch checkbox `+0x18c`
- `0x5fe` → `bothPairActive` passed to `AdaptPortsToSettings`
- `0x609` → `UpdateHPSourceButtonLabels` labels

## Setting-cookie → kext property-key map (crossing P3)

`CMiddleWareDevice::GetCPSetting(cookie)` was decoded in P3 as a `0x3F3`-opcode `LBroadcaster` broadcast that bubbles up to `CXDevice::Get32BitNumberProperty(NSString *key)`. The cookie-to-NSString translation layer lives in `CFW1814DeviceSettings` (not yet decoded — scheduled for **P7**). Likely mapping based on key semantics (P3 table) and context:

| Cookie | Likely `MA*` key                                 |
|-------:|--------------------------------------------------|
| 0x608  | `MAMonitorAuxOnOutput`                           |
| 0x60d  | *sub-selector* of `MAInputRoutingV2` (optical-in choice) |
| 0x60e  | *sub-selector* of `MASWReturnRoutingV2` (optical-out choice) |
| 0x60f  | *sub-selector* of `MAInputRoutingV2` (active-input toggle) |
| 0x611  | (new) ADAT 4-ch mode — not in the P3 key list; may ride on `MAInputRoutingV2` as a flag bit |
| 0x5fe  | ADAT optical topology enabled — likely a `MARotaryControlV2` bit |
| 0x609  | `MAHeadphoneSourceV2`                            |

**P7 will confirm these mappings** by decoding `CFW1814DeviceSettings::{GetCPSetting, SetCPSetting}` which is the actual Cookie → NSString translator.

## Clone implication for OpenAudio

- Our Hardware tab must expose exactly **4 buttons** for clock source. In v1.5.2 the labels are:

  | Button ID | v1.5.2 label (from `Panel.rsrc` PPob 5001 `ghidra-exports/panel-rsrc.txt:14645..`) | v1.4 label (SL capture, B5 §10) |
  |---|---|---|
  | 0x1452 | "internal - digital inputs muted"   | "Internal"   |
  | 0x1453 | "internal - digital inputs unmuted" | *(absent — only one Internal entry)* |
  | 0x1454 | "external - digital"                | "S/PDIF" / "ADAT" (selected via a secondary optical-input picker) |
  | 0x1455 | "word clock"                        | "Word Clock" |

  **Version skew — the v1.5.2 panel re-architected the clock-source UI** from "pick wire-source directly" to "pick clock mode (internal mute/unmute, external, wordclock)". The underlying byte protocol on the wire is still a 2-bit selector (see A1-Q5), but the UI collapses S/PDIF and ADAT into a single "external - digital" button because the optical-input radio-group `0x60d` already names which digital port is active. In v1.4 the clock-source button-group and the optical-input picker could disagree; v1.5.2 removes that duality. Our clone should follow v1.5.2 (the shipping-target bundle).
- The MOM feature (Mix-Out-to-Monitor) is a 2-state toggle backed by setting 0x608 — document in `docs/14-UNIFIED-DEV-PLAN.md` under control-panel scope.
- The optical input/output type (S/PDIF vs ADAT) is a **per-direction** choice (two independent radio groups), *not* one global optical-mode selector. This is important: the kext must accept independent values for `0x60d` and `0x60e` on the AV/C routing side.
- The "ADAT 4-channel mode" toggle (0x611) only applies when optical-out is in S/PDIF mode (see `AdaptPortsToSettings`). This is likely the FW1814's "ADAT 4-channel at 88.2/96 kHz" S/MUX mode and may change the plug map at higher rates.
- Headphone A/B source is a **complex picker** (`CFW1814HPABSourceDialog`) — scope creep for the MVP; defer until mixer views (P6) are done.

## Status

Done. P6 (CFW1814MixerView) is next. P7 (CFW1814DeviceSettings) will close the cookie → `MA*` key translation layer and resolve Q15b.
