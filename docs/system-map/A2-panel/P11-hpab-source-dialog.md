# P11 — Headphone A/B Source Dialog (`CFW1814HPABSourceDialog`)

> **Correction (2026-10-16):** cookie `0x609` (the 6-byte tuple) is written as `MAMomentarySwitchRoutings`, an NSArray of three masks `1<<(v−1)` (v: 1→1, 2→2, 4→3 on the FW1814); the Output-tab phone source radios use cookie `0x5fd` (index 8 = phones 1 → `MAHeadphoneSourceV2`, 9 = phones 2 → `MAHeadphoneSecondSource`). See `P13-translator-index-map.md` §5.

> **Correction (2026-10-14) — audit** (evidence: [PROVENANCE-RESOLUTIONS.md §R19](../PROVENANCE-RESOLUTIONS.md)). Verified in the i386 asm (`CFW1814HPABSourceDialog`, 0x53969-0x547d1): resource IDs `0x170d/0x170e/0x170f/0x1711` (HP-A) and `0x1717/0x1718/0x1719/0x171b` (HP-B); `ListenToMessage` compares `0x384` (Set), `0x3f5` (ESC), `0x385` (Cancel), `0x601`, `0x609`, `0x170d`, `0x1717`; cookies `0x609`, `0x601`, `0x615` are read. §3.3 ('aux max rate hardcoded in the kext to 48000') is superseded by the cookie-0x615 correction above (192000 on the FW1814, from the panel's own `CX1814Device`).


> **Correction (2026-10-14) — cookie 0x615 = 192000 for FW1814** (evidence: [PROVENANCE-RESOLUTIONS.md §R18](../PROVENANCE-RESOLUTIONS.md)). **Cookie `0x615` for the FW1814 is 192000 Hz, not 48000.** `CXDevice::GetHWSetting` (0x6e2ca) answers cookie `0x615` with `this->vt[0x58]()` (asm 0x6eecd-0x6ef3a, after a 4-byte size check); slot `0x58` of `__ZTV12CX1814Device` is `CX1814Device::GetMaximumMixerSampleRate @0xe9e56` = `movl $0x2EE00` (192000); the base `CXDevice` version and the other devices return `0x17700` (96000), `CX1212csDevice` 96000 (vtable walk of all CX* classes). `CFW1814DeviceSettings::ReadSettingsFromHardware` (0x5bd98-0x5bdaf) stores that value in `[+0x388]`; `0xBB80` (48000) is used **only if the read fails** (`testw %ax,%ax; cmovel`). Consequence: `AdaptToClockRate` (`rate <= max`) is true for every rate the FW1814 supports (32-192 kHz), so the reference panel **never disables the aux-send section (nor the HP A/B Aux radios) by rate** on the FW1814. The earlier statements 'aux-send unavailable above 48 kHz', 'hardcoded in the kext to 48000' (P11 §3.3), 'HW-seeded default 48000' and the P-manual-crosscheck item 5 conclusion are **not supported** for the FW1814; whether the manual says otherwise was not re-read (unverified).


**Scope:** the modal picker launched from the Hardware tab's "set source" button (PPob 5001, strings table ID 7). Not to be confused with the hardware tab's "a/b switch settings" group which stays on the main window.

**Decompile log:** `docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log:172..838` — `ShowHPABSourceDialog, FinishCreateSelf, ListenToMessage, SaveSettings, LoadSettings, SavePrefs, RestorePrefs, MapHPSourceButtonToSetting, MapHPSourceSettingToButton, UpdateHPSourceButtons` @ `0x53124..0x54476`.

**Resource:** `'PPob' (5900) "1814 Headphone A/B Settings"` at `ghidra-exports/panel-rsrc.txt:3657..3725`.

## 1. Dialog layout (from PPob 5900)

| Widget type | Resource ID | Role |
|---|---|---|
| `HPab` | 0x170c | window root |
| `rgpv` | 0x170d | **HP-A** radio-group container |
| `rdbt` | 0x170e | HP-A → "Out 1/2" (value 1) |
| `rdbt` | 0x170f | HP-A → "Out 3/4" (value 2) |
| `rdbt` | 0x1711 | HP-A → "Aux"     (value 4) |
| `rgpv` | 0x1717 | **HP-B** radio-group container |
| `rdbt` | 0x1718 | HP-B → "Out 1/2" (value 1) |
| `rdbt` | 0x1719 | HP-B → "Out 3/4" (value 2) |
| `rdbt` | 0x171b | HP-B → "Aux"     (value 4) |
| `push` | 0x1721 | Cancel |
| `push` | 0x1722 | Set |
| `stxt` | — | "Select the source for the A and B positions of the Headphone Out 1 A/B switch." |

**Note:** this dialog has **no** Direct Monitoring toggle, **no** mode selector, and **no** front-panel A/B button mode binding. All three live on the Hardware tab's "a/b switch settings" subgroup (PPob 5001).

## 2. Negative findings — what is NOT here

### 2.1 No `MADirectMonitoringActive` write path

`SaveSettings` at `0x5486c` touches only cookie `0x609`. `FinishCreateSelf` at `0x53840` installs listeners on exactly 8 widgets (0x170e, 0x170f, 0x1711, 0x1718, 0x1719, 0x171b, 0x1721, 0x1722 — the three HP-A radios, three HP-B radios, Cancel, Set) and reads cookies `0x601` (sample rate), `0x609` (3-tuple), `0x615` (max-aux-rate). None of these is `MADirectMonitoringActive`. The DM toggle must be bound by `CFW1814HardwareView::FinishCreateSelf` (not in current decompile batch).

### 2.2 No `MAMomentarySwitch{Routings,Source}` write path

Same analysis — the HPAB dialog does not touch front-panel A/B switch state. The front-panel switch is dedicated hardware wiring: a 2-position toggle whose two possible routings are the A and B rows of cookie `0x609`. Changing which routing-pair the switch applies to is a **mode** selection on the Hardware tab, not something editable here. See item #8 in `P-manual-crosscheck.md`.

### 2.3 No HP2-at-high-rate auto-override

The decompile shows **disable**, not **rewrite**. When `current_rate > local_1c` (where `local_1c = GetHWSetting(0x615) → 48000`), `UpdateHPSourceButtons` at `0x54256` disables the Aux radios in **both** HP-A and HP-B rgpvs. If stored state had `hpB == 4` (Aux), the dialog shows that radio disabled; the user must pick a non-Aux value before pressing Set. The stored cookie `0x609` is NOT silently rewritten. (Prior guess that the manual's wording "HP2 forced to 1/2 at 176.4/192 kHz" meant an auto-rewrite is refuted.)

## 3. Positive findings — what IS wired

### 3.1 Cookie `0x609` — the (mainMix, hpA, hpB) 3-tuple

`SaveSettings` writes a 6-byte buffer `{local_16, local_14, ...}` to cookie `0x609` only if the dialog's radio-group selections differ from the current stored value (dirty-check). The buffer layout observed in `LoadSettings` and `UpdateHPSourceButtons`:

```
offset 0 (2 B): mainMix  — preserved, not read in HPAB dialog
offset 2 (2 B): hpA      — read/write
offset 4 (2 B): hpB      — read/write
```

Values: `1 = out 1/2`, `2 = out 3/4`, `4 = aux`.

### 3.2 Cookie `0x601` — current sample rate (RO)

Pulled in `ListenToMessage` (line 421) and in `UpdateHPSourceButtons` to compute whether to disable the Aux radios. Kext-published; the dialog never writes it.

### 3.3 Cookie `0x615` — aux max rate (RO)

Pulled in `UpdateHPSourceButtons` as the threshold for Aux-radio enable. Hardcoded in the kext to `48000`. If `current_rate > 48000`, both Aux radios are disabled.

### 3.4 Button→value and value→button maps

```c
MapHPSourceButtonToSetting(button_id):
    if (0x170e <= button_id < 0x171c):
        return value_from_table[button_id - 0x170e]
    else:
        return 1  // default to "out 1/2"

MapHPSourceSettingToButton(buf, which):
    // which == 0 → HP-A, which == 1 → HP-B
    v = buf[which]
    switch(v):
        case 4: return 0x1711 if HP-A else 0x171b  // Aux
        case 2: return 0x170f if HP-A else 0x1719  // Out 3/4
        case 1: return 0x170e if HP-A else 0x1718  // Out 1/2
        default: return 0  // indeterminate
```

The value-from-table is a 14-entry static array at `_LSOpenCFURLRef + 4` (symbol-named `LSOpenCFURLRef` is a stub mislabel; the actual table lives in `__const` at that offset in `__TEXT`). Entries indexed `[button_id - 0x170e]` for both HP-A (`0x170e..0x1711`) and HP-B (`0x1718..0x171b`) with gaps.

### 3.5 Window position persistence

`SavePrefs` and `RestorePrefs` serialize the dialog's window position (not its contents) to `kCFPreferencesCurrentApplication/CurrentUser/AnyHost` under the key `cf_HeadphoneABSourceDialogPosition`. This is pure UX nicety — the actual dialog content is held in cookie `0x609` on the kext side, not in CFPrefs.

### 3.6 `ListenToMessage` dispatch table

| Message | Source | Action |
|---|---|---|
| 900 (`0x384`) | Set button | `SaveSettings`; `this[0x150] = 0` (clear dirty flag); `DoThisDialog` close (vtable+0x27c) |
| 901 (`0x385`) | Cancel button | fall-through to `DoThisDialog` cancel (vtable+0x240) |
| 0x3f5 | ESC | same as Cancel |
| 0x601 (sample rate) | kext broadcast | re-run `UpdateHPSourceButtons` with new rate |
| 0x609 (3-tuple) | kext broadcast | re-pull the stored tuple; re-run `UpdateHPSourceButtons` |
| `0x170d`, `0x1717` (rgpv selection changed) | user click | `UpdateHPSourceButtons` with the new rgpv-local selection |

The dialog does not call `SaveSettings` on every radio click — only on the Set button. Mid-dialog state is held in the radio-group widgets themselves; Cancel discards them by not calling `SaveSettings`.

## 4. Clone implications

1. **Clone's HPAB view is a 3×2 radio matrix** (3 choices × 2 outputs) plus Set/Cancel. No DM, no mode toggle in this view.
2. **Dirty-check on Save:** compare in-dialog selection against the current `0x609` value; only write if changed.
3. **Rate-reactive Aux enable:** subscribe to `0x601` broadcasts; disable Aux radios when rate > `0x615` (fetch once on dialog open — the `0x615` value is a kext constant and does not change at runtime).
4. **Window position persistence** via NSUserDefaults is optional parity; not required for feature correctness.
5. **The cookie layout `(mainMix, hpA, hpB)` as 3× 2-byte values** must be preserved end-to-end: read-modify-write to avoid clobbering mainMix (which is set by a different view).

## Open — needs separate decompile pass

- `CFW1814HPABSourceDialog::ShowHPABSourceDialog @ 0x53124` sets up two class-static singletons and a modal event loop (`sModalEventLoopUPP`) — classic Carbon modal pattern. Not re-decoded in detail; parity for the clone is modal `NSWindowController` with `beginSheet:completionHandler:`.
- Resource IDs 0x170e..0x171b with gaps (0x1710, 0x1712..0x1716, 0x171a) are reserved slots that other M-Audio devices use for additional HP sources (e.g., FW410's "S/PDIF"); the FW1814 ignores them.
