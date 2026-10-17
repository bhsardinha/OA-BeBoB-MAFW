# P7 — `CFW1814DeviceSettings` state model (cookie ↔ field table)

> **Correction (2026-10-14) — cookie 0x615 = 192000 for FW1814** (evidence: [PROVENANCE-RESOLUTIONS.md §R18](../PROVENANCE-RESOLUTIONS.md)). **Cookie `0x615` for the FW1814 is 192000 Hz, not 48000.** `CXDevice::GetHWSetting` (0x6e2ca) answers cookie `0x615` with `this->vt[0x58]()` (asm 0x6eecd-0x6ef3a, after a 4-byte size check); slot `0x58` of `__ZTV12CX1814Device` is `CX1814Device::GetMaximumMixerSampleRate @0xe9e56` = `movl $0x2EE00` (192000); the base `CXDevice` version and the other devices return `0x17700` (96000), `CX1212csDevice` 96000 (vtable walk of all CX* classes). `CFW1814DeviceSettings::ReadSettingsFromHardware` (0x5bd98-0x5bdaf) stores that value in `[+0x388]`; `0xBB80` (48000) is used **only if the read fails** (`testw %ax,%ax; cmovel`). Consequence: `AdaptToClockRate` (`rate <= max`) is true for every rate the FW1814 supports (32-192 kHz), so the reference panel **never disables the aux-send section (nor the HP A/B Aux radios) by rate** on the FW1814. The earlier statements 'aux-send unavailable above 48 kHz', 'hardcoded in the kext to 48000' (P11 §3.3), 'HW-seeded default 48000' and the P-manual-crosscheck item 5 conclusion are **not supported** for the FW1814; whether the manual says otherwise was not re-read (unverified).


**Status:** done.
**Scope:** Decode how `CDeviceSettings` + `CFW1814DeviceSettings` turn panel widget updates into (a) a local cache write and (b) an `LBroadcaster` cookie push toward the kext-interface layer. Closes P6-fup1 and P6-fup2.

**Binary refs:** `_extracted/panel/M-Audio FireWire.app/Contents/MacOS/Panel`; Ghidra project `panel-app`.

Source log: `ghidra-exports/decomp/a2-p7.log` (decompiles of 8 functions below).

## 1. Dispatcher chain (`CDeviceSettings`)

The base class (@0x659…) is a four-method state engine. The FW1814 subclass only overrides the two polymorphic worker functions (`SetSettingData` at vtable+0x14, `GetSettingData` at vtable+0xc on the subclass, plus `ReceiveSetting` at vtable+0xa8 for inbound notifications).

| Method | Addr | Behaviour |
|---|---|---|
| `GetSetting(cookie, idx, size, outBuf)` | `0x65970` | Thin null-guard around `vtable[+0xac]` (= `GetSettingData` dispatch). |
| `UpdateSetting(cookie, idx, size, data)` | `0x657f0` | Calls `vtable[+0xa8]` (`ReceiveSetting`) — accepts a kext-pushed cookie into the local cache. On success + `dirty` out-param set, flips `this[+0x5d]=1` and broadcasts msg `0x3fd` (dirty signal, consumed by `OK/Apply` button). |
| `UpdateSettingAndHW(cookie, idx, size, data, broadcast)` | `0x658a2` | Panel-widget entry point. (a) `vtable[+0x14]` = `SetSettingData` (local cache + validation); (b) if size==0 path uses `this[+0x68]->vtable[+0x28]` = `CMiddleWareDevice::SetCPSetting`, else `vtable[+0x2c]` = `SetIndexedCPSetting` (both broadcast cookie as LBroadcaster msg `0x3F2` — see P3); (c) if `broadcast` arg true, broadcasts cookie on `this+0x30` so sibling views repaint. |
| `ListenToMessage(msg, body)` | `0x65750` | Handles the inverse direction. msg `0x3F2` → re-enters `SetSettingData` (cookie came from another listener). msg `0x3F3` → re-enters `GetSettingData` to answer. |

`this+0x68` = owning `CMiddleWareDevice`. `this+0x30` = embedded `LBroadcaster` subobject.

## 2. `CFW1814DeviceSettings::SetSettingData` @ `0x59836`

Shape: `switch (cookie - 0x5e2)` with range gate `cookie-0x5e2 ≤ 0x33` (so cookies `0x5e2…0x615` are the entire FW1814 setting space). Writes the matching field, sets `*out_dirty = true` on validation failure; returns 0/1/3 (ok / cookie-unknown / size-mismatch).

## 3. `CFW1814DeviceSettings::GetSettingData` @ `0x5a19c`

Same shape, range gate `cookie-0x5de ≤ 0x37`. Four extra read-only cookies (`0x5de…0x5e1`) delegate to `CMiddleWareDevice::GetCPSetting` (`this[+0x68]->vtable[+0x20]`) — kext-sourced globals (likely GUID / firmware-info / device-caps strings; cross-check with P3 keys `MAFirmwareInfo`, GUID).

## 4. Cookie table (writable + readable)

Column key:
- **Size** = `param_3` bytes.
- **Idx** = index semantics (`0` = whole / `n` = per-channel/pair/output).
- **Field** = offset in `CFW1814DeviceSettings` (local cache).
- **P3 key** = corresponding `MA*` NSString from `P3-property-key-strings.md` where known. Cookie→NSString translation itself lives downstream (see §6 — not inside `CMiddleWareDevice`).

### 4.1 Globals / device info
| Cookie | Size | Idx | Field | Direction | Semantics | P3 key |
|---|---|---|---|---|---|---|
| `0x5de`–`0x5e1` | var | 0 | kext-sourced | R | Device info quartet (passes through MW) | `MAFirmwareInfo` / `MAMetaAllProperties` candidates |
| `0x5e2` | 1 | — | (none) | validate | Size-check only (expects 1) | — |
| `0x5e3` | — | — | — | fall-through | Default no-op path | — |
| `0x5e4` | 8 | 0 | +0x84 (4B zeroed, +4B tail unused on write) | RW | 8-byte opaque; writes clear only `+0x84`, Get returns `+0x84` and 0 pad | — |
| `0x5e5` | 2 | 0 | +0x88 | RW | u16 global (default 2 at read) | — |
| `0x5e6` | 1 | 0 | +0x8a | RW | u8 | — |
| `0x5e7` | 1 | 0 | +0x8b | RW | u8 | — |
| `0x5e9` | 0x80 | 0 | +0x3a8 (block) | validate | 128-byte block seeded in `ReadSettingsFromHardware` (rate/clock profile) | — |

### 4.2 Level arrays (indexed, 4B each)
| Cookie | Idx range | Field base | Direction | Likely P3 key |
|---|---|---|---|---|
| `0x5ea` | 1..22 | +0x88 + idx·4 | RW | `MAInputLevels` |
| `0x5eb` | 1..18 | +0x108 + idx·4 | RW | `MAOutputLevels` |
| `0x5ec` | 1..22 | +0x188 + idx·4 | RW | `MASWReturnLevels` |

Seeded in `ReadSettingsFromHardware` loop iVar8=99..120 (addresses `+0x88+idx·4`, `+0x108+idx·4`, `+0x188+idx·4`); default `0xffffffc0` on error for 0x5ea/0x5ec and `0` for 0x5eb → confirms 0x5eb is SW-return (unity default), 0x5ea/0x5ec are gain-like (min default).

### 4.3 Mute / solo arrays (indexed, 1B each; range 1..11)
| Cookie | Field base | Likely P3 key |
|---|---|---|
| `0x5ed` | +0x20b + idx | `MAInputMute` |
| `0x5ee` | +0x21b + idx | `MAInputSolo` |
| `0x5ef` | +0x22b + idx | `MASWReturnMute` |
| `0x5f0` | +0x23b + idx | `MASWReturnSolo` |

### 4.4 Pair solo state (indexed, 2B pair-encoded)
| Cookie | Idx range | Field | Note |
|---|---|---|---|
| `0x5f1` | 1..11 | +0x24a + idx·2 (2 bytes) | **Solo pair** — the one FW1814-only override observed in P6 mixer view. |

### 4.5 Pan / balance (indexed, 4B)
| Cookie | Idx domain | Field | Direction | Semantics |
|---|---|---|---|---|
| `0x5f3` | 0 | +0x3d8 (0x50 bytes) | validate | 80-byte pan-table initialiser (seeded from HW) |
| `0x5f4` | 1..4 ∨ 15..20 | +0x268 + idx·4 (and mirror at +0x2b8 + idx·4) | RW | Per-input/pair level — triggers `CalculateOutputPairBalanceLevel` and broadcasts `0x5f6` to companion. P3 key: `MAInputLevels` / `MAHeadphoneLevels` mix. |
| `0x5f6` | 1..2 ∨ 8..10 | +0x308 + idx·4 | RW | Output-pair balance; symmetrically recomputes `+0x268+idx·4` for the pair and emits `0x5f4` back out. Uses bounds `+0x3e4/+0x3f0/+0x3f4` loaded by HW read. |

Pan/balance uses two writeable field columns because the pair-balance (`0x5f6`) is derived from, and must stay coherent with, two independent per-leg levels (`0x5f4` × 2). The `0x5f4`-case code snapshots the current pair-index via `vtable[+0x28]` (`GetPairIndex`?) and recomputes the companion balance; the `0x5f6`-case does the inverse, re-emitting two `0x5f4` events to the broadcaster so companion widgets repaint.

### 4.6 Per-pair state (1B, output-pair indexed)
Index domain `(1..2) ∨ (8..10)` (= output pairs 1/2 + "pair-8/9/10"; i.e. the two main out pairs + 3 aux/hdphn pairs).

| Cookie | Field base | Likely P3 key |
|---|---|---|
| `0x5f7` | +0x333 + idx | `MAOutputMute` (idx-domain `1,2,10` only — different gate) |
| `0x5f8` | +0x33d + idx | `MAOutputSolo` or `MAHeadphoneMute` |
| `0x5f9` | +0x347 + idx | 2-pair only (idx 1..2) — likely `MAHeadphoneMute` |
| `0x5fa` | +0x351 + idx | `MAAuxOutputMute` |

### 4.7 Routing / topology (2B, few-slot)
| Cookie | Idx | Field | Likely P3 key / role |
|---|---|---|---|
| `0x5fb` | 1..3 | +0x35a + idx·2 | `MASWReturnRoutingV2` (3 slot targets) |
| `0x5fd` | 8/9 | +0x370 / +0x372 | `MAHeadphoneSourceV2` (idx 8=HP-A, 9=HP-B) |
| `0x5fe` | 0 | +0x374 (1B) | `MAMonitorAuxOnOutput` (bool) |
| `0x5ff` | 0 | +0x384 (2B) | `MAAuxSendLevels` globals (count/flags) |

### 4.8 Clock + rate (sticky device-wide values)
| Cookie | Size | Field | Direction | P3 key |
|---|---|---|---|---|
| `0x601` | 4 | +0x378 | **RW** (settable from panel; was wrongly flagged read-only in P6-fup2) | `MARateLocked` or `MASampleRate` echo — HW seeded to 0 |
| `0x602` | 2 | +0x37c | RW | — |
| `0x605` | 4 | +0x380 | RW | — |
| `0x608` | 2 | +0x392 | RW | MOM group (confirmed P5) |
| `0x609` | 6 | +0x394 (3×u16) | RW | HP-source triple (confirmed P5 — key `MAHeadphoneSourceV2`?) |
| `0x60a` | 2 | +0x37e | RW | — |
| `0x60d` | 2 | +0x38c | RW | Optical-in (1=SPDIF/2=ADAT) — confirmed P5 |
| `0x60e` | 2 | +0x38e | RW | Optical-out — confirmed P5 |
| `0x60f` | 2 | +0x390 | RW | Active-input — confirmed P5 |
| `0x610` | 1 | +0x39a | RW | — (unused in P5 panel widgets) |
| `0x611` | 1 | +0x39b | RW | ADAT-4ch (confirmed P5) |
| `0x615` | 4 | +0x388 | RW (writeable in `SetSettingData`!) | **Max rate.** HW-seeded default 48000. — Correct P6-fup2: NOT read-only in cookie layer; however no panel widget calls `SetCPSetting(0x615)` — written only by `WriteSettingsToHardware` echo back. See §5. |

## 5. Hardware sync (`ReadSettingsFromHardware` @ `0x5b5f8` / `WriteSettingsToHardware` @ `0x5bf10`)

Both are pure for-each over the cookie set, going through the owner `CMiddleWareDevice` (`this[+0x68]`):

- Reads use `vtable[+0x20]` (whole) / `vtable[+0x30]` (indexed) / `vtable[+0x24]` (block).
- Writes use `vtable[+0x28]` (whole) / `vtable[+0x2c]` (indexed).

Observations worth noting for the kext-side clone work:

1. **HW-write set is strictly smaller than SW-set.** `WriteSettingsToHardware` writes 0x5e5, 0x5e6, 0x5e7, 0x5ea, 0x5eb, 0x5ec, 0x5ed..0x5f1, 0x5f4, 0x5f7..0x5fb, 0x5fd, 0x5fe (via a readback oddity — see below), 0x5ff, 0x601, 0x60d, 0x60e, 0x60f, 0x611, 0x608, 0x609. **Not written:** 0x5e2, 0x5e3, 0x5e4, 0x5e9 (block), 0x5f3, 0x5f6 (derived), 0x5f8/0x5fa with same restrictions as 0x5f7 fan-out, 0x602, 0x605, 0x60a, 0x610, **0x615**. Confirms `0x615` is read-only in practice (seeded by `ReadSettingsFromHardware`, mirrored to `+0x388`, never pushed back).
2. **`0x5fe` oddity** — `WriteSettingsToHardware` *reads* 0x5fe (not writes) and sticks the result into `this[0x374]`. This is a sync-back-from-HW convention rather than panel→HW.
3. **Default-on-error values** are informative: `0x5ea`/`0x5ec` default to `0xffffffc0` (= -64 dB in Q); `0x5eb` defaults to `0`; `0x5e5` defaults to `2`; `0x5fd` idx-9 defaults to `2`; `0x615` defaults to `48000`; `0x608` defaults to `1`. These are the panel's "no answer from kext" fallbacks and must match the kext's post-reset state to avoid cold-boot churn.

## 6. Where cookie→NSString translation lives (open: P7-fup)

**Not** inside `CMiddleWareDevice::{Get,Set}CPSetting` — those methods broadcast LBroadcaster msg `0x3F2`/`0x3F3` with the raw cookie and do no key lookup. **Not** inside `CDeviceSettings::ListenToMessage` either — it simply re-enters `GetSettingData`/`SetSettingData`.

The P3-key `MA*` NSString table is installed by `CXDevice::InstallPropertyChangeHandlers` @ `0x6ca12` and consumed by `CXDevice::Receive32BitValueChange` @ `0x6c1e6` (which takes `NSString*`, not a cookie). So there must be a listener registered on the same `LBroadcaster` chain that:

1. Receives `0x3F2`/`0x3F3`.
2. Maps cookie → NSString key (per-device table).
3. Calls `IORegistryEntrySetCFProperties(...)` with a 1-key dict, or triggers one via the P1/P2 path.

This translator is **the kext RPC seam** — it is where the panel-side setting model turns into the on-wire kext property update. **Opened as P7-fup** in README; target symbols likely in `CFW1814DeviceSettings`'s vtable (one of `+0xa8` ReceiveSetting, or a secondary listener subobject at `this+0x30`) or in a separate CFW1814-only `LListener`. Not resolved here to keep P7 scope-bounded.

## 7. Updates to prior docs

- **P6-fup1 — closed.** The mixer-view-visible per-strip level/pan/mute/aux-send cookies are confirmed as 0x5ea / 0x5eb / 0x5ec / 0x5ed / 0x5ee / 0x5ef / 0x5f0 / 0x5f1 / 0x5f4 / 0x5f6 / 0x5f7 / 0x5f8 / 0x5f9 / 0x5fa / 0x5fb / 0x5fd / 0x5fe / 0x5ff — i.e. the entire `0x5ea..0x5ff` band. Base `CMixerView::ListenToMessage` almost certainly routes these to the same cookie-indexed subviews; a dedicated decode is unnecessary now that the field+size+idx domain is pinned.
- **P6-fup2 — closed.** `0x601` (current rate) IS writable from the panel (`SetSettingData` case 0x601 accepts a 4B write and `WriteSettingsToHardware` echoes it). `0x615` (max rate) is accepted by `SetSettingData` but **never written** by `WriteSettingsToHardware` — effectively read-only, published by the kext at HW-read time. Update to `P6-mixer-view.md`: 0x615 read-only, 0x601 bidirectional (RW).

## 8. Clone implication (kext side)

The kext must publish, as either `IORegistryEntry` properties or by a cookie-table RPC:

- A frozen `0x615`-value "max rate" that the panel reads once per device open.
- A bidirectional `0x601` echo of the engine's current sample rate — accepted writes should re-enter the engine's rate-change path (M3/M4).
- All level/mute/solo/pair arrays with the index domains captured in §4.2–§4.4 (not inferred — these come from `(param_2 != 0 && param_2 < N)` gates in the switch).
- Default-on-missing values from §5(3) so the panel's "no answer" fallbacks match a cold device.

The **cookie→NSString translator** must land on the kext seam (P7-fup) before we can commit to a kext property-key schema. Expect it to use the P3 `MA*` names 1:1 so the kext matches the 2014 driver's `IORegistryEntrySetCFProperties` payload byte-for-byte.
