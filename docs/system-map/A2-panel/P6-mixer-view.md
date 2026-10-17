# A2-P6 — `CFW1814MixerView` + `CFW1814MixerChannelPairView`

> **Correction (2026-10-14) — cookie 0x615 = 192000 for FW1814** (evidence: [PROVENANCE-RESOLUTIONS.md §R18](../PROVENANCE-RESOLUTIONS.md)). **Cookie `0x615` for the FW1814 is 192000 Hz, not 48000.** `CXDevice::GetHWSetting` (0x6e2ca) answers cookie `0x615` with `this->vt[0x58]()` (asm 0x6eecd-0x6ef3a, after a 4-byte size check); slot `0x58` of `__ZTV12CX1814Device` is `CX1814Device::GetMaximumMixerSampleRate @0xe9e56` = `movl $0x2EE00` (192000); the base `CXDevice` version and the other devices return `0x17700` (96000), `CX1212csDevice` 96000 (vtable walk of all CX* classes). `CFW1814DeviceSettings::ReadSettingsFromHardware` (0x5bd98-0x5bdaf) stores that value in `[+0x388]`; `0xBB80` (48000) is used **only if the read fails** (`testw %ax,%ax; cmovel`). Consequence: `AdaptToClockRate` (`rate <= max`) is true for every rate the FW1814 supports (32-192 kHz), so the reference panel **never disables the aux-send section (nor the HP A/B Aux radios) by rate** on the FW1814. The earlier statements 'aux-send unavailable above 48 kHz', 'hardcoded in the kext to 48000' (P11 §3.3), 'HW-seeded default 48000' and the P-manual-crosscheck item 5 conclusion are **not supported** for the FW1814; whether the manual says otherwise was not re-read (unverified).


> **Correction (2026-10-14) — aux-send gate** (evidence: [PROVENANCE-RESOLUTIONS.md §R15](../PROVENANCE-RESOLUTIONS.md)). The aux-send-only rate gate is **not unsourced**: it is confirmed in `P-manual-crosscheck.md` item 5 / lines 82-88 (decomp `a2-manual-crosscheck.log:92-141`: `AdaptToClockRate @0x57012` reads cookie `0x615` (default 48000, `GetCPSetting`) and enables/disables the widget at `*(this[+0x12c]+0xbc)`; the manual agrees as recorded there — the PDF was not re-read in this pass). The gate itself is verified in the asm/decomp (`AdaptToClockRate`: `rate <= max` → `vt[200]` enable, else `vt[0xcc]` disable). The phrase 'standard BeBoB limitation' is an **explanation with no source** — **unverified**.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - 'Aux-send unavailable above 48 kHz (standard BeBoB limitation)' has no source — **unverified**; check = manual (`docs/manual.pdf`) or panel disable-logic decompile.


**Scope:** The Mixer tab. Topology adaptation (which strips exist as a function of optical in/out + ADAT S/MUX mode), per-rate strip visibility, and the one FW1814-specific per-pair handler (Solo). Level, pan, and Mute are handled by the base `CMixerView` / `CMixerChannelPairView` and are not FW1814-overridden.

**Decompile log:** `ghidra-exports/decomp/a2-p6.log` — `CFW1814MixerView::{GetPortSettings, MapSettingsToChannelViewID, ListenToMessage, AdaptChannelsToPortSettings, AdaptAdatChannelsToSMUX, AdaptToClockRate, AdaptUIElementsToSettings}` @ `0x56a7e..0x570b4` and `CFW1814MixerChannelPairView::{LoadUIElements, ListenToUIElements, ListenToMessage, MapMessageToHWPairID, AdaptUIElementsToSettings}` @ `0x55414..0x556e8`.

## Up-front correction

The P6 seed asked for "level/pan/mute/solo + aux-send bindings". The actual code tells a tighter story: **only Solo is FW1814-overridden on the channel-pair strip.** Level, pan, mute, and aux-send are handled by the generic `CMixerChannelPairView` / `CMixerView` base classes — the FW1814 subclass does **not** override them. This is consistent with the kext typed-property model (A1-Q15) where level/mute/aux-send properties are device-agnostic and only Solo and the topology-switch bits need FW1814-specific wiring.

## Mixer topology — four strip layouts

The mixer-tab's channel-strip layout depends on **three** cookies from P5: optical-in (`0x60d`), optical-out (`0x60e`), and ADAT 4-ch S/MUX mode (`0x611`). The strip-view resource ID is picked by `MapSettingsToChannelViewID(opticalIn, opticalOut)`:

| opticalIn | opticalOut | ResID (PowerPlant `.rsrc`) | Interpretation |
|-----------|------------|---------------------------|----------------|
| 1 (SPDIF) | 1 (SPDIF)  | **0xbec**                 | 2 analog pairs + 1 SPDIF pair in / 2 analog pairs + 1 SPDIF pair out |
| 1 (SPDIF) | 2 (ADAT)   | **0xbee**                 | 2 analog pairs + 1 SPDIF pair in / 2 analog pairs + 8-ch ADAT out |
| 2 (ADAT)  | 1 (SPDIF)  | **0xbed**                 | 2 analog pairs + 8-ch ADAT in / 2 analog pairs + 1 SPDIF pair out |
| 2 (ADAT)  | 2 (ADAT)   | **0xbeb**                 | 2 analog pairs + 8-ch ADAT in / 2 analog pairs + 8-ch ADAT out |

`AdaptChannelsToPortSettings(opticalIn, opticalOut)` compares the computed ResID against `this[+0x130]` (the ID currently in use). On mismatch it does a **full pane rebuild**:

```
vtable[+0x1e8] - get radio-group current (used elsewhere; here read as a side-effect)
vtable[+0x204] - broadcast "layout changing" to listeners (if +0x120 non-null)
vtable[+0xbc]  - FinishCreateSelf  (if +0x20 == 2, i.e. tab is already live)
vtable[+0x14c] - LoadFromStream / layout the new ResID
vtable[+0xb8]  - PaneDrawn
vtable[+0x200] - broadcast "layout changed"
```

Returns 1 on rebuild. This is why **the mixer tab visibly flickers when you flip optical in/out on the Hardware tab** — all its strip subviews are destroyed and re-instantiated.

## ADAT S/MUX adaptation (`AdaptAdatChannelsToSMUX`)

When any side is ADAT (optical-in = 2 **or** optical-out = 2), the ADAT 4-ch toggle (cookie `0x611`) decides whether channels 5–8 of that ADAT 8-ch group are visible. The code enables/disables **two child panes per ADAT side**:

- `this[+0x124][+0x14][+0x1c]` and `this[+0x124][+0x14][+0x20]` — the 5-6 and 7-8 pair strips on the **input** side (only touched when opticalIn == ADAT).
- `this[+0x124][+0x14][+0x38]` and `this[+0x124][+0x14][+0x3c]` — the 5-6 and 7-8 pair strips on the **output** side (only touched when opticalOut == ADAT).

If S/MUX is **on** → Disable (vtable+0xcc) both pairs — because at 88.2/96 kHz ADAT S/MUX only carries 4 channels per direction.
If S/MUX is **off** → Enable (vtable+0xc8/0xc200) both pairs — all 8 channels available at 44.1/48 kHz.

**Clone implication:** our panel must mirror this topology → either we ship 4 nib layouts (one per optical in/out combo) or we build the strip grid programmatically. Programmatic is easier on AppKit.

## Per-rate feature gating (`AdaptToClockRate(rate)`)

Reads cookie `0x615` (4 B, default 48000) via `GetCPSetting` — this is a **per-element max-rate limit** that the kext publishes as a property. If `rate <= maxRate` → the element at `this[+0x12c][+0xbc]` is enabled; otherwise disabled.

Only one element gates on rate in the mixer tab (`this[+0x12c][+0xbc]`), likely the aux-send section — aux-send is unavailable above 48 kHz on the FW1814 (standard BeBoB limitation).

**New property observation (not in P3 table):** cookie `0x615` ≈ "maximum rate at which this element is usable". **Effectively read-only** — no `SetCPSetting(0x615)` anywhere in the panel, and `CFW1814DeviceSettings::WriteSettingsToHardware` doesn't push it either, even though the cookie-switch accepts a 4 B write (see P7 §4.8). The kext publishes it; the panel caches it. **(P6-fup2 — closed by P7.)**

## `ListenToMessage` dispatch (parent class)

The FW1814 mixer-view override only handles **four** message IDs; everything else goes to `CMixerView::ListenToMessage`:

| Message ID | Trigger | Action |
|-----------:|---------|--------|
| `0x60d` | optical-in changed (broadcast from Hardware tab)  | `GetPortSettings` + `AdaptChannelsToPortSettings` (may rebuild strips) |
| `0x60e` | optical-out changed                                | same path |
| `0x611` | ADAT 4-ch toggle changed                           | `GetPortSettings` + `AdaptAdatChannelsToSMUX(…, smuxOn)` |
| `0x601` | clock rate changed                                 | `AdaptToClockRate(newRate)` only — no strip rebuild |

After every handler, the global `sUpdateCommandStatus = 1` flag is set (menu-item enable-state rebuild on next idle). Note: `0x601` is a new cookie (clock rate, 4 B). It's **the current operating rate**, distinct from the per-element max `0x615`.

**Level, pan, mute, aux-send, meter messages are all handled by the base `CMixerView`.** The FW1814 subclass exists purely to react to topology / rate changes.

## Per-pair Solo (`CFW1814MixerChannelPairView::ListenToMessage`)

The one FW1814-override on the per-pair strip:

```c
if (param_1 == 0x5f1) {                /* kext → UI: solo state changed */
    char newSolo[2] = {payload[0], payload[1]};
    for (i = 0; i < 2; i++) {
        this->soloBtn[i]->SetValue(vtable+0x24, newSolo[i]);
    }
}
else if (param_1 - 0x83U < 2) {        /* UI → kext: user clicked ch1 (0x83) or ch2 (0x84) solo */
    uint ch = (param_1 == 0x84);                        /* 0 or 1 */
    int newState = this->soloBtn[ch]->GetValue();
    char cur[2];
    sVar = GetCPSetting(0x5f1, this->hwPairID /* +0xb6 */, 2, cur);
    if (sVar == 0 && cur[ch] != (newState == 1)) {
        cur[ch] = (newState == 1);
        SetCPSetting(0x5f1, this->hwPairID, 2, cur, 0);
    }
}
else {
    CMixerChannelPairView::ListenToMessage(this, param_1, param_2);  /* level/pan/mute */
}
```

**Cookie `0x5f1`** is the Solo state for one hardware pair, indexed by `this[+0xb6]` (`hwPairID`, 2 B). The payload is a 2-byte pair of booleans — one per channel in the pair. P3's property-key table maps the panel-visible NSString `MAInputSolo` (and friends `MASWReturnSolo`, `MAOutputSolo`) to kext properties, so cookie `0x5f1` is the per-strip-type Solo property; the pair-view decides which solo property to hit based on its subclass (there is a parallel `CFW1814HPOutChannelPairView` etc. that will use a different cookie — out of scope for P6).

`MapMessageToHWPairID(buttonID)` returns `(buttonID == 0x84) + 1` — i.e. ch1 → 1, ch2 → 2. This is used by the base-class helpers (level/pan/mute) to disambiguate which channel of the pair the user is manipulating.

## Full one-shot sync (`AdaptUIElementsToSettings`)

Called on tab-switch and after a Mach-port `bodyType==0` notification:

```c
GetPortSettings(this, &opticalIn, &opticalOut);
if (!AdaptChannelsToPortSettings(this, opticalIn, opticalOut)) {   /* if no rebuild… */
    GetCPSetting(0x611, 0, 1, &smuxOn);
    AdaptAdatChannelsToSMUX(this, opticalIn, opticalOut, smuxOn);
    GetCPSetting(0x601, 0, 4, &rate);
    AdaptToClockRate(this, rate);
    CMixerView::AdaptUIElementsToSettings(this);   /* level/pan/mute/aux-send sync */
}
```

**Early-return if `AdaptChannelsToPortSettings` rebuilt the panes** — the rebuild will trigger a cascade of child-view `AdaptUIElementsToSettings` calls so re-syncing here would be redundant.

## Cookies seen in P6 (deltas from P5)

| Cookie | Size | Direction | Meaning |
|-------:|-----:|-----------|---------|
| `0x5f1` | 2 B (bool[2]) | r/w | **Solo per hw pair** — indexed by HWPairID; one of P3's `MA*Solo` keys at the typed-property layer |
| `0x601` | 4 B | r | **Current operating clock rate** (Hz) — one of P3's `MARateLocked` / rate-related keys |
| `0x611` | 1 B | r/w | ADAT 4-ch S/MUX mode (same as P5) |
| `0x615` | 4 B | r | **Per-element max rate** (Hz) — gates feature visibility; no P3 NSString equivalent (read-only, likely a sub-field of `MARotaryControlV2`/engine info) |
| `0x60d` | 2 B | r/w | Optical-in type (same as P5) |
| `0x60e` | 2 B | r/w | Optical-out type (same as P5) |

Both `0x601` and `0x615` are new; they do not have an obvious NSString equivalent in the P3 table. Likely candidates:

- `0x601` → engine's current rate. **Correction (closed by P7):** the panel *can* write `0x601` — `CFW1814DeviceSettings::SetSettingData` accepts a 4 B write and `WriteSettingsToHardware` pushes it. However, no mixer-view widget writes it; the write-side user is `CFW1814GlobalSettingsView` (P8). `MASampleRate` is a *separate* parallel fast-path on `FWAVCAudioDevice::setProperties` for latency reasons.
- `0x615` → per-element rate ceiling; effectively read-only (never written by panel code, only cached from kext).

## Clone implication for OpenAudio control panel

1. **Mixer tab must react to four cookies**, not just mixer values: `0x60d`, `0x60e`, `0x611`, `0x601`. First two trigger a strip-grid rebuild; third toggles ADAT S/MUX visibility; fourth gates per-rate features.
2. **Solo is the only per-pair FW1814-specific handler.** Level/pan/mute live in the generic strip view; we can reuse one strip widget for all our supported devices.
3. **The kext must publish three read-only informational properties** that are not in the P3 writable-key list: `0x601` (current rate), `0x615` (per-element max rate), and the pair-indexed Solo cookie `0x5f1` (bidirectional). All three ride on the typed-property framework (Q15).
4. **Four PowerPlant strip-view ResIDs (0xbeb..0xbee) encode four optical topology combos.** AppKit port can use one view-model-driven NSStackView; no need for four XIBs.
5. **Aux-send is read/written via the base `CMixerView`** — its binding is a *generic* per-pair cookie decoded in a future P (likely P-followup when we decode `CMixerView` base). Not in the FW1814 override, so **not** in P6 scope.

## New follow-up questions

- **P6-fup1:** Decode `CMixerView::ListenToMessage` + `CMixerChannelPairView::ListenToMessage` to pin down level/pan/mute/aux-send cookies. Currently unknown. (Likely in the `0x5e0..0x5fd` range by numeric proximity to Solo at 0x5f1.)
- **P6-fup2 — closed by P7.** `0x615` is kext-published-only (no `SetCPSetting`, no `WriteSettingsToHardware` push). `0x601` is bidirectional (panel can push rate via `WriteSettingsToHardware`); the mixer view uses it read-only but P8's `CFW1814GlobalSettingsView` is the write-side user.

## Status

Done. P7 (CFW1814DeviceSettings — the cookie ↔ NSString translator) is the next natural step and will likely close P6-fup1/fup2 as a side-effect since it enumerates the full cookie table.
