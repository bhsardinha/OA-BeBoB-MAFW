# Q5 — `SetClockSourceInternal` + `SetBlankSlateClockSource` — clock-source enum + HSCI payload

> **Correction (2026-10-15, M-C / R44):** the §"`FWHSCI *hsci = this->fHSCI; // [+0x268]`" reading is wrong: `[+0x268]` is the `FWAVCUnit`. See [`Q-MC-cmp-isoch-avc-start.md`](Q-MC-cmp-isoch-avc-start.md).

> **Note (2026-10-14, P5/R35):** the pseudo-code below compares `mode > 0x3fffffff -> 0xE00002C2`, which contradicts the accepted values (`0x40000014`, R21; bit 30 must be set). The direction of the gate compare is **unverified in this sketch**; use C2 §2.2/§2.7.
> **Correction (2026-10-14) — clock-change sleeps (SL v1.9)** (evidence: [PROVENANCE-RESOLUTIONS.md §R21](../PROVENANCE-RESOLUTIONS.md)). Live on the SL rig (v1.9, device idle, `MAClockSource` 0x40000014 -> 0x40000017 -> 0x40000014 written with `IORegistryEntrySetCFProperties`, `fbt::IOSleep` traced): each change runs **two consecutive `IOSleep(300)`** (timestamps 410 and 710 ms; 734 and 034 ms for the second change) on the workloop thread, exactly the two `IOSleep(0x12c)` of `SetClockSourceInternal` (0xe482, 0xe450) — no 2500 ms sleep (that is only `SetBlankSlateClockSource`). The property read-back shows the new value ~0.324 s after the write (after the first 300 ms sleep, matching the `fMode` store at 0xe490). The sleeps are **fixed constants**, not a poll of the lock bit (`MARateLocked` stayed 1 throughout), so the phrase 'device-measured PLL settle time' is unsupported; that they exist *for* PLL/plug settling remains **unverified** (no capture ties them to a hardware event).


> **Correction (2026-10-14) — Part A completion batch 1** (evidence: [PROVENANCE-RESOLUTIONS.md §R8](../PROVENANCE-RESOLUTIONS.md)). `vt[0xbe0]` `SwitchDigitalSignals` re-runs `SetIOFormat` and picks the peak template by the engine's **output** channel count (6/8/12/else) — see R8. `SetBlankSlateClockSource` passes `(fMode & 0x1c)+0x40000003` (drops S/MUX bit 5).


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - **Slot names** (R2): `vt[0xce0]` = `FWEngine2::PerformSampleRateChangeCore @0x117e6` (not an AV/C commit); `vt[0xaf8]` = `FW1814Device::ResetRearrangementProcs @0xcad4` (not an HSCI emit); `vt[0xb00]` = `IOAudioEngine::beginConfigurationChange`; `vt[0xb10]` = `cancelConfigurationChange` (error path only, `0xe30b`); `vt[0xb58]` = `IOAudioEngine::setSampleRate`; `vt[0xbe0]` = `FW1814Device::SwitchDigitalSignals @0xe5dc` (not an AV/C stream kick); `vt[0xce8]` = `SetExternalSyncEnabled(bool) @0x11bc0`; `vt[0xa50]/[0xa58]` = `pauseAudioEngine/resumeAudioEngine`.
> - **HSCI clock-source frame is 16 bytes, not 12:** `bzero(…,0x10)` @0xe284/0xe2a2, length `0x10` passed to `[+0x268]->vt[0x8b8]` @0xe441 (R3). The 12-byte layout is snd-bebob `bebob_maudio.c:183-198`. Bytes 10-15 content is **unverified**.
> - Rev1 `vtable[0xb70]` super-call: `__ZTV17FWRev1AudioDevice`+0xb70 = slot 0xb60 = `FWAVCAudioDevice::SetClockSourceInternal @0x23148`.


**Status:** done + corrected 2026-10-09 + composite-bitfield addendum 2026-10-09 PM + dispatcher/mute decode 2026-10-12 + **full bijection locked 2026-10-12 PM via caps 68+69**.

## Final bijection (2026-10-12 PM — all four sync-source radios captured)

Captures `pulled/68-dtrace-digital-mute.txt` + `pulled/69-dtrace-syncradios-spdifport.txt` resolved **all four sync-source radios** (per manual §2.3 — four radio buttons on the Sync Source quadrant, not a dropdown):

| Sync Source radio | Panel 32-bit value | bits [1:0] | HSCI wire byte 6 |
|---|---|---|---|
| Internal — digital inputs **unmuted** | `0x4000001?` with low bits = `00` | **0** | **3** |
| External — digital | `0x4000001?` with low bits = `01` | **1** | **1** |
| Word Clock | `0x4000001?` with low bits = `10` | **2** | **2** |
| Internal — digital inputs **muted** | `0x4000001?` with low bits = `11` | **3** | **0** |

Matches `SetClockSource @ 0x23858`'s `SetDigitalInputActive` branch exactly:
- bits[1:0] ∈ {0, 1, 2} → `vtable[0xce8](true)` → digital input stream active (passes through)
- bits[1:0] = 3 → `vtable[0xce8](false)` → digital stream suppressed (true "muted" state)

**The earlier B5 §10 bijection table was wrong.** It claimed `wire 0 = Internal, wire 3 = S/PDIF` based on a mis-labeled capture-61 sweep. The actual mapping is `wire 0 = Internal-muted, wire 3 = Internal-unmuted` — those are **two different device states**, not variants of a single "Internal" selection with a mute flag. The confusion was Claude's: Claude read "Internal unmuted" as "Internal clock + unmute toggle" and treated the four Panel radios as if they were a dropdown with a separate mute flag. The manual clearly describes four radio buttons; cap 69 captured each one explicitly and settled the bijection.

**Legacy translator revisited** — the four cases in `SetClockSource @ 0x23858`:
```c
case 0: param_1 = 0x40000000;  // bits[1:0]=00 → Internal unmuted (wire 3)
case 1: param_1 = 0x40000004;  // bits[1:0]=00 + bit 2 (coaxial) → Internal unmuted + coax
case 2: param_1 = 0x40000001;  // bits[1:0]=01 → External digital (wire 1)
case 3: param_1 = 0x40000005;  // bits[1:0]=01 + bit 2 (coaxial) → External digital + coax
```
Legacy BeBoB-era API only exposed two of the four current radios (Internal-unmuted and External-digital), each with a S/PDIF-port variant. No access to Internal-muted or Word Clock from legacy callers.

## Complete MAClockSource bitfield (2026-10-12 PM — locked)

| Bit(s) | Field | Encoding |
|---|---|---|
| bit 30 (`0x40000000`) | required gate | always 1; `0xE00002C2` if clear |
| bits [1:0] | **Sync Source radio** | see table above |
| bit 2 (`0x4`) | S/PDIF input port | 0 = optical, 1 = coaxial |
| bit 3 (`0x8`) | Optical-IN mode | 0 = S/PDIF, 1 = ADAT |
| bit 4 (`0x10`) | Optical-OUT mode | 0 = S/PDIF, 1 = ADAT |
| bit 5 (`0x20`) | S/MUX / ADAT 4-channel | 0 = off, 1 = on |
| bits 6..29 | always 0 in all captures | — |

## Dispatcher + mute-bit decode (2026-10-12)

## Dispatcher + mute-bit decode (2026-10-12)

A second Ghidra pass (`docs/system-map/A1-ghidra/ghidra-exports/decomp/dispatcher-mute.log`, script `A1-ghidra/scripts/DecompDispatcherMute.java`) targeted the kext-side `MAClockSource` composite dispatcher and the mute-bit allocation inside the low nibble. Findings:

### Dispatcher: there is NO single `MAClockSource` bit-field dispatcher

The composite word is NOT demultiplexed by a dedicated route-by-bitmask function. Instead `MAClockSource` writes flow through the standard `m_audio_b_FWAVCAudioDevice::SetClockSource @ 0x23858` which calls into:

1. **`FW1814Device::SetClockSourceInternal @ 0xe25c`** — handles bits [1:0] via the wire-byte permutation (already decoded) and ships the HSCI envelope.
2. **Engine `vtable[0xce8]` (SetDigitalInputActive-equivalent)** — toggled based on `(mode & 0xf) != 0` or `(mode & 3) ∈ {0,1,2}` (per legacy branch).
3. **`FW1814Device::SwitchDigitalSignals @ 0xe5dc`** — the digital-mode channel-count adapter (not called directly from SetClockSource; invoked by the engine via a different entrypoint — likely the `SetIOFormat` path that reads the channel count from `[+0x2e0]`). This rebuilds the peak-meter dispatch:
   ```
   ch=6  → _Sample1814PeaksSPDIF6   (S/PDIF-mode output topology)
   ch=8  → _Sample1814PeaksADAT8    (ADAT 8-ch @ ≤48k)
   ch=12 → _Sample1814PeaksADAT12   (ADAT 8-ch + 4 extra, likely S/MUX active @ 88.2/96k)
   else  → _Sample1814Peaks4All     (fallback / 192k-era with 2-ch S/MUX)
   ```
   This is the function that reshapes IOAudioStreams/peak-meters when bit 4 (optical-out) or bit 5 (S/MUX) flip. Called via `(**(code **)(*(long *)this + 0xaf8))(this, engine)` from a format-change path.

**Implication for the clone**: bits 4 and 5 of `MAClockSource` are not routed via a dispatcher at property-write time; they are side-effects consumed later by `SwitchDigitalSignals` when `SetIOFormat` re-queries the channel count. The composite-bitfield design is really "one write updates `fMode` + a few sideband flags; downstream code (engine format change, peak-meter rebuild) reads the sideband flags on next format resync."

### `InitializeUserSettableProperties @ 0xe6ca` — property registration

Installs the `FWSettingsABSwitch` object at `this+0x2d8` with:
- **Cookie `0xbe9`** (opcode base for the A/B button handler)
- **Callback `SetHeadphoneSelector`** at `this+0x300`
- `UpdateLEDs()` call at init time

Also calls `BaseChannelGroup::SetupBatchChangeMethod` with opcodes `{0x00, 0x08, 0x10, 0x34, 0x38}` for the five channel-group writers. These are AV/C vendor-command opcode bases, not bit positions in `MAClockSource`.

### Mute-bit allocation — **still partially unverified**

User's "Internal muted" state throughout capture 66 was `0x40000027` (low nibble `0x7`). Decomposing:

| Bit | Value in capture | Meaning per decompile |
|---|---|---|
| bit 0 (`0x1`) | 1 | together with bit 1 → clock-source selector (bits[1:0]=3 → Internal, wire byte 0) |
| bit 1 (`0x2`) | 1 | " |
| bit 2 (`0x4`) | 1 | **hypothesis: mute indicator** — set in all captures where user had Internal-muted active |
| bit 3 (`0x8`) | 0 | **hypothesis: unused / unmuted-complement** |

The `SetClockSource @ 0x23858` legacy translator passes bit 2 through unchanged (legacy 1 → bit 2 set, legacy 3 → bit 2 set) but it does NOT affect the HSCI wire-byte or `SetDigitalInputActive`. That means **bit 2 is a Panel-/UI-level state that the kext accepts and stores but doesn't act on in the clock-source path**. Most likely it mirrors the "digital inputs muted" state the v1.5.2 Panel exposes as separate buttons (`PPob 5001 0x1452/0x1453` = "internal - digital inputs muted" vs "unmuted").

**Still open:** need a mute-toggle capture (user clicks between "Internal — digital inputs muted" and "Internal — digital inputs unmuted" on the Panel) to confirm bit 2 flips. Without that, the exact bit position for mute is a hypothesis — bit 3 is the alternative candidate.

**Clone-side action**: our dispatcher must preserve bit 2 (or bit 3, when resolved) when writing back any `MAClockSource` composite — do not mask it out. If the user hasn't exercised mute, store the whole low nibble as-is and treat bits [3:2] as opaque until a mute-toggle capture resolves them.

## Addendum (2026-10-09 PM) — `MAClockSource` is a composite bitfield

## Addendum (2026-10-09 PM) — `MAClockSource` is a composite bitfield

Live SL capture `pulled/66-dtrace-smux-toggle.txt` (session 2026-10-09 PM) proves the 32-bit `MAClockSource` property-write value packs **multiple orthogonal hardware controls**, not just the clock source:

| Bit range | Field | Values |
|---|---|---|
| bit 30 (`0x40000000`) | **required gate** | always 1 in valid writes |
| bit 5 (`0x20`) | **ADAT 4-channel / S/MUX** | 0 = normal ADAT, 1 = S/MUX |
| bit 4 (`0x10`) | **Optical-out mode** | 0 = S/PDIF, 1 = ADAT |
| bits 0..3 | **Clock source nibble** (per the Q5 bijection, `0x1C..0x1F`) + probable mute/lock-indicator bits still open | see table below |

Captured user sequence (optical-out and S/MUX toggles while clock stayed Internal):
```
pre:   0x40000027   (optical-out = S/PDIF, S/MUX ON)
step1: 0x40000037   (optical-out → ADAT)  [+ bit 4]
step2: 0x40000017   (S/MUX → OFF)         [- bit 5]
step3: 0x40000037   (S/MUX → ON)          [+ bit 5]
step4: 0x40000027   (optical-out → S/PDIF)[- bit 4]
```

**Implications for the clone:**
- The kext's `MAClockSource` property dispatcher must mask + route by bit-field, NOT dispatch on the whole value as a clock-source enum. The clock-source bijection documented below (via `SetClockSourceInternal @ 0xe25c`) handles ONLY bits 0..3; the digital-mode bits 4..5 flow through separate kext handlers that are not yet decoded. **Open task:** find the dispatcher that routes bit 4 → `SetOpticalOutMode()` and bit 5 → `SetADAT4Channel()` in `_extracted/kext/`.
- Earlier A2-P3 schema count treated `MAClockSource` as one scalar key — in reality it's ~5 controls multiplexed into one 32-bit word. No separate `MADigitalOutMode` or `MAADAT4Channel` keys exist on the Panel side.
- Reference: memory `project_maclocksource_is_composite_bitfield.md`.

## Correction (2026-10-09)

A re-decompile via Ghidra headless (`docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:49..345`, script `A1-ghidra/scripts/DecompClockBijection.java`) refutes three claims the original Q5 made from `docs/system-map/A1-ghidra/otool-exports/mafw.asm` immediates alone:

1. **HSCI wire byte 6 is NOT identity-mapped from `mode & 3`.** The actual switch at `com_m_audio_FW1814Device::SetClockSourceInternal @ 0xe25c:248..276` is a permutation `{mode&3 == 1 → 1, 2 → 2, 3 → 0, 0 → 3}`. See the corrected bijection table in §Clock-source enum below.
> **Correction (2026-10-14) — this item is itself wrong.** Raw disassembly of `SetClockSourceInternal` shows `r14d = esi` (the 32-bit `MAClockSource` argument, `0xe26f`), unmodified until `0xe3e2`. Then: byte 7 = `(r14 >> 3) & 1` (`0xe3e2..0xe3ed`), byte 8 = `(r14 >> 4) & 1` (`0xe3fe..0xe409`), byte 9 = `(r14 >> 5) & 1` (`0xe41a..0xe425`). These are optical-IN ADAT, optical-OUT ADAT and S/MUX (B5 §10 bitfield). The original 2026-10-06 bit-shift reading was right. The Ghidra decompile in `clock-bijection.log:278..291` shows `*puVar3 = 0` because Ghidra mis-typed the parameter (`CONCAT71(…, param_2)`, i.e. treated as 1 byte). Raw instructions win. Also matches snd-bebob bytes 7/8 `dig_in_fmt`/`dig_out_fmt` (`bebob_maudio.c:194-195`). Byte 9 is S/MUX here, not snd-bebob's `clk_lock`. Confirmed by Q15f and `Q-SDK-reserved-slots.md`-session re-read.

~~2. **HSCI payload bytes 7, 8, 9 are hardcoded 0**~~, not bit-shifts of `mode`. The decomp at `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:278..291` writes `*puVar3 = 0` three times for offsets 7/8/9. Mode bits 3/4/5 are NOT propagated into the HSCI envelope.
3. **The `0x40000000` bit is a required-high API gate, not a "force reconfigure" flag.** `if (local_18 < 0x40000000) return 0xE00002C2` at `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:218..220` (same path at `0xe25c`) rejects any mode word with the high bit clear. Legacy BeBoB-era callers that still use the raw 2-bit enum go through `m_audio_b_FWAVCAudioDevice::SetClockSource @ 0x23858` first, which runs a legacy-bitmap translator (see §Legacy translator) before dispatching.

The sections below retain the sequence/sleep/retain analysis — those parts were correct — but the bijection table, byte-7/8/9 semantics, and `0x40000000` meaning are rewritten.

**Scope:** `0xe25c` (`FW1814Device::SetClockSourceInternal`), `0xe4f0` (`FW1814Device::SetBlankSlateClockSource`), `0xbd7a` (`FWAudiophileDevice::SetClockSourceInternal`).

## Answer in one line

**Clock-source selection assembles a 16-byte HSCI payload and ships it via vtable[0x8b8] (likely `FWHSCI::SendCommand`), then sleeps 300 ms, then calls the Rev1 vtable[0xb70] (plug-rate-set), then another 300 ms, then AV/C rate re-assert via vtable[0xbe0], then re-sends `FWSettingsLevels`.** The 16-byte envelope is a vendor-dependent HSCI control block decoded here.

## HSCI 16-byte payload layout (SetClockSourceInternal @ 0xe25c)

Field offsets within the 16-byte output buffer (`-0x44(%rbp)` through `-0x44+16`):

| Byte offset | Value source | Semantic |
|---|---|---|
| 0 | 0x00 | Fixed header byte 0 |
| 1 | 0xFF | Fixed header byte 1 |
| 2 | 0x00 | — |
| 3 | 0x04 | — |
| 4 | 0x00 | — |
| 5 | 0x04 | — |
| 6 | permutation of `mode & 3` (see table below) | Clock-source wire byte |
| 7 | `(mode >> 3) & 1` — optical-IN ADAT | `0xe3e2..0xe3ed` (Correction 2026-10-14) |
| 8 | `(mode >> 4) & 1` — optical-OUT ADAT | `0xe3fe..0xe409` |
| 9 | `(mode >> 5) & 1` — S/MUX | `0xe41a..0xe425` |
| 10..15 | zero (bzero'd) | padding / reserved |

**The 32-bit `clockSourceSel` parameter is required to have the high bit set** (`>= 0x40000000`); otherwise the call errors out with `0xE00002C2`. The low 2 bits select the clock source (via the permutation below). Mode bits 2 and higher control other things — notably the engine's "digital-input active" flag computed in the parent `SetClockSource` at `0x23858` — but do NOT appear in the HSCI envelope.

The accepted mode word is also stored to `fMode[+0x144]` (`docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:296`) and reused on blank-slate rebuilds.

## Clock-source bijection (v1.11.2 kext — corrected)

The switch at `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:248..276`:

```c
switch (mode & 3) {
    case 1: *(buf+6) = 1; break;
    case 2: *(buf+6) = 2; break;
    case 3: *(buf+6) = 0; break;
    default /* case 0 */: *(buf+6) = 3; break;
}
```

**SUPERSEDED 2026-10-12 PM by cap 69** — see §Final bijection at top of this doc. The earlier B5 §10 interpretation (reproduced below for history) mislabeled the four radios because the capture-61 sweep didn't name the clicks explicitly. The real mapping (from cap 69 ground truth with named radio clicks):

| Panel 32-bit `MAClockSource` | `mode & 3` | HSCI wire byte 6 | Radio label (manual §2.3) |
|---|---|---|---|
| low bits `00` + other state | 0 | **3** | Internal — digital inputs unmuted |
| low bits `01` + other state | 1 | **1** | External — digital |
| low bits `10` + other state | 2 | **2** | Word Clock |
| low bits `11` + other state | 3 | **0** | Internal — digital inputs muted |

**Wire-byte → source assignment:** `0=Internal-muted, 1=External-digital, 2=Word Clock, 3=Internal-unmuted`.

**v1.5.2 Panel encoding is NOT revealed by the kext decompile.** The v1.5.2 UI (PPob 5001 buttons `0x1452..0x1455`) exposes labels `{"internal - digital inputs muted", "internal - digital inputs unmuted", "external - digital", "word clock"}`. Those four buttons must map to four distinct 32-bit mode words — but the specific low-byte values live in the Panel binary's clock-button handler, not the kext. The kext accepts any `>= 0x40000000` and routes by `mode & 3` only.

## Legacy translator (`m_audio_b_FWAVCAudioDevice::SetClockSource @ 0x23858`)

Callers that still use the pre-v1.11 2-bit enum (e.g. BeBoB clients, FFADO-era code) hit this entry first. The decomp (`docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:97..113`) runs:

```c
if (param_1 < 0x40000000) {
    _IOLog("M-FW: Converting old digital bitmap (%X)\n", param_1);
    switch (param_1) {
        case 0: param_1 = 0x40000000; break;  // → wire 3 (S/PDIF)
        case 1: param_1 = 0x40000004; break;  // → wire 3 (S/PDIF, +bit2)
        case 2: param_1 = 0x40000001; break;  // → wire 1 (ADAT)
        case 3: param_1 = 0x40000005; break;  // → wire 1 (ADAT, +bit2)
    }
}
```

Note the legacy enum maps only to {wire 1, wire 3} (ADAT and S/PDIF) — the legacy API never selected Internal or Word Clock. This is a historical artifact of BeBoB's `MA_CLOCKSOURCE_V1` which only exposed the two external digital sources plus an unused reserved pair.

After translation, `SetClockSource` also sets a boolean `digitalInputActive` for the engine (`docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:122..130`):

```c
// With high bit always set after translation:
digitalInputActive = ((mode & 0xf) != 0);
```

For legacy translated values: `{0x40000000 → false, others → true}`.
For Panel-sent `0x4000001X` values: always true (`0x1X & 0xf != 0`).

This boolean dispatches to engine `vtable[0xce8]` — the digital-input mute toggle.

## Removed from original Q5

- ~~The `bit 3 → digital-in`, `bit 4 → digital-out`, `bit 5 → extra flag` HSCI-payload bit map was wrong — those bytes are hardcoded 0.~~ **Correction (2026-10-14):** the bit map was right; see the Correction at item 2 above. If v1.5.2 encodes digital-in/digital-out selection in the mode word, it's consumed by the parent `SetClockSource` or the Rev1 `vtable[0xb60]` plug-rate call, not by the HSCI envelope.
- The `0x40000000 = "force reconfigure even if same mode"` claim — that bit is a required gate, not a flag.

## `SetClockSourceInternal(uint32_t mode, bool blankSlate)` @ 0xe25c — sequence

```c
IOReturn FW1814Device::SetClockSourceInternal(uint32_t mode, bool blankSlate) {
    // 1. Zero the 16-byte output + 16-byte response buffers.
    uint8_t out[16] = {0}, in[16] = {0};
    uint32_t outLen = 16, inLen = 16;

    FWEngine *e = this->fEngine;              // [+0xe8]
    if (e) e->vtable[0xa50](e);               // retain

    // 2. Early-out if we're already updating.
    if (this->fClockSourceUpdating /* [0x1b88] */) return 0;

    // 3. Begin update: tell the engine, set flag.
    e->vtable[0xb00](e);                      // beginClockSourceChange
    this->fClockSourceUpdating = 1;

    // 4. Validate mode (upper bound 0x3fffffff).
    if (mode > 0x3fffffff) {
        err = 0xE00002C2;                     // kIOReturnBadArgument
        goto rollback;
    }

    // 5. Build the HSCI payload bytes (table above).
    out[0]=0; out[1]=0xFF; out[2]=0; out[3]=4; out[4]=0; out[5]=4;
    switch (mode & 3) {              // clock-source permutation
        case 1: out[6] = 1; break;   // ADAT
        case 2: out[6] = 2; break;   // Word Clock
        case 3: out[6] = 0; break;   // Internal
        default: out[6] = 3; break;  // S/PDIF (mode&3 == 0)
    }
    out[7] = (mode >> 3) & 1;        // optical-IN ADAT  (0xe3e2, Correction 2026-10-14)
    out[8] = (mode >> 4) & 1;        // optical-OUT ADAT (0xe3fe)
    out[9] = (mode >> 5) & 1;        // S/MUX            (0xe41a)

    // 6. Send via vtable[0x8b8] on HSCI object (likely FWHSCI::IssueCommand).
    FWHSCI *hsci = this->fHSCI;     // [+0x268]
    hsci->vtable[0x8b8](hsci, out, 16, in, &inLen);

    // 7. Settle: 300 ms.
    IOSleep(0x12c);

    // 8. Remember mode, call parent Rev1 plug-rate-set.
    this->fMode = mode;             // [+0x144]
    err = FWRev1AudioDevice::vtable[0xb70](this, mode);
    if (err) goto rollback;

    // 9. Another 300 ms.
    IOSleep(0x12c);

    // 10. If not blankSlate, re-issue AV/C rate + mixer refresh.
    if (!blankSlate) {
        this->fMode = mode;
        this->vtable[0xbe0](this, mode);              // AV/C stream kick
        this->fSettingsLevels.SendToDevice();         // mixer resend
    }
    err = 0;
rollback:
    this->fClockSourceUpdating = 0;
    e->vtable[0xb08](e);            // endClockSourceChange
    if (e) e->vtable[0xa58](e);     // release
    return err;
}
```

## `SetBlankSlateClockSource()` @ 0xe4f0 — the cold-boot variant

```c
IOReturn FW1814Device::SetBlankSlateClockSource() {
    if (!this->fEngine) return 0;
    // Derive the "blank slate" source from current fMode:
    //   newMode = 0x40000003 | (fMode & 0x1c)
    //   = enum 3 (Word Clock) with digital-in/out bits preserved and the 0x40000000 "blank" flag set
    uint32_t newMode = (this->fMode & 0x1c) + 0x40000003;
    IOReturn err = this->vtable[0xb60](this, newMode, /*blankSlate*/1);
    IOSleep(0x9c4);                     // 2500 ms — long post-reset settle
    return err;
}
```

The `0x40000000` bit is **the required-API gate** (see Correction above), not a reconfigure flag. The post-reset settle is 2500 ms because the device PLL needs that long to re-lock after a full plug-config shuffle.

## Delta summary

| Aspect | Reference | Clone | Impact |
|---|---|---|---|
| 16-byte HSCI payload bytes 0..5 fixed header `00 FF 00 04 00 04` | yes | TBD | Required; exactly these bytes |
| Payload byte 6 = permutation of `mode & 3` → `{0:3, 1:1, 2:2, 3:0}` | yes | TBD | Required; wire-byte table above |
| Payload bytes 7/8/9 = `MAClockSource` bits 3/4/5 (Correction 2026-10-14) | yes | TBD | Required |
| Mode word must have `0x40000000` bit set | yes | TBD | Required gate; `0xE00002C2` otherwise |
| 300 ms settle after HSCI send | `IOSleep(0x12c)` | TBD | Required for PLL lock |
| 300 ms second settle after Rev1 rate-set | `IOSleep(0x12c)` | TBD | Required |
| 2500 ms "blank-slate" settle | `IOSleep(0x9c4)` | TBD | Only on cold-boot path |
| `fClockSourceUpdating` guard flag | `[0x1b88]` | TBD | Prevents concurrent re-entry |
| AV/C stream-kick (`vtable[0xbe0]`) after non-blank clock change | yes | **critical** | memory `project_fw1814_stream_kick` names this |

## Follow-up queue impact

- **Q5a (new):** identify vtable[0xb60]'s true target. The code at 0xe471 uses `FWRev1AudioDevice`'s vtable, not `FW1814Device`'s — so the plug-rate-set lives in the Rev1 parent. Decode Rev1's vtable[0xb70] to confirm what AV/C selector it issues.
- Follow-up: cross-check HSCI payload bytes 10..15 (currently zero-padded). FFADO's maudio.cpp may document additional bits.

## Provenance

- Original decode: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06 — bytes-7/8/9 bit-shift reading retracted; see Correction.
- Correction decode: `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log`, 2026-10-09, script `A1-ghidra/scripts/DecompClockBijection.java`.
- Fixed-header bytes `00 FF 00 04 00 04`: `com_m_audio_FW1814Device::SetClockSourceInternal @ 0xe25c` writes at `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:222..247`.
- Wire-byte permutation: `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:248..276`.
- Bytes 7/8/9: raw disassembly `otool-exports/mafw.asm` `0xe3e2..0xe425` (authoritative). The Ghidra decompile `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:278..291` shows `0` here, which is a decompiler artifact; see Correction 2026-10-14.
- `0x40000000` gate: `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:218..220`.
- Legacy translator: `m_audio_b_FWAVCAudioDevice::SetClockSource @ 0x23858` at `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:97..113`.
- B5 v1.9 Panel low-byte values: `B5-sl-dtrace-truth/README.md §10`.
- Sleep constants: `0x12c` = 300 ms, `0x9c4` = 2500 ms.
