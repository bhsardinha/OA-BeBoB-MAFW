# A2-P9 — Peak-meter dispatch + polling model

> **Correction (2026-10-16):** `GetMixerPairChannelMap` is a `CFW1814DeviceSettings` method (identity pair map), not `CMiddleWareDevice vt+0x1c` (that slot is `GetNumberOfMixerChannels`). Peak slots: input n → `peak[n−1]`, output n → `peak[18+n−1]`, SW return k → `peak[38+k−1]`. See `P13-translator-index-map.md` §6.

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - Kernel-side meter page is `uint32_t[52]` (38 hardware + 14 SW-return slots), not '≈18 inputs / ≈14 outputs' (`GetPeakLevelCount` = 0x34 @0x1d30; writer `ReceiveControlPacket @0xe9ea`).


**Scope:** How the panel acquires peak-meter data (mixer VU + output VU), the kext-side contract, and FW1814-specific overrides.

**Decompile log:** `ghidra-exports/decomp/a2-p9.log`. Script: `scripts/DecompA2P9.java`.

**Target address pivot:** P2 predicted that `CXDevice::HandleNotification` had a `bodyType==1` branch calling `ReceivedPeakMeterUpdate`. **That prediction is wrong.** The real `HandleNotification` only handles `bodyType==0` (property-dict change) — see `a2-batch1.log:411` — there is no case 1 or case 2, and `ReceivedPeakMeterUpdate` / `ReceivedRateLockedChange` do not exist as symbols. Peak meters come from a **polling pump**, not a Mach push.

> P2 §"Dispatch" claimed three bodyTypes 0/1/2 with case-1 for peaks and case-2 for rate-lock. Only case 0 exists in the shipping binary. P2 §"Clone implication" needs correction: our kext does **not** need to broadcast bodyTypes 1 or 2. See §6 of this doc.

> **Correction (2026-10-14, P5/R35):** §7.3's statement that selector `0x1fa` (506) is a vestigial no-op is superseded: 506 is `FWUserClient::WaitForTermination @0x761e`, a **blocking** Mach-semaphore wait released when the user client stops; the Panel's `TerminationWaitThread` relies on that blocking. Meter page = `uint32 peak[52]` written by `ReceiveControlPacket @0xe9ea` (see C5 §4).

## 1. Polling cookies and their state (`CMiddleWareDevice`)

`CMiddleWareDevice::AddMWPollingType` @ `000639ea` accepts exactly **three** cookies and rejects everything else with `-50` (kParamErr):

| cookie  | enable byte | ref-count | meaning (hypothesis) |
|---------|-------------|-----------|----------------------|
| `0x5e9` | `this[0x54]` | `this[0x56]` u16 | Mixer peak-meter stream |
| `0x5f3` | `this[0x50]` | `this[0x52]` u16 | Output peak-meter stream |
| `0x601` | `this[0x58]` | `this[0x5a]` u16 | Sample-rate change (also used by P8 writer) |

Ref-counted — multiple views can subscribe. When any of the three enable bytes is non-zero, `LPeriodical::StartRepeating(this + 0x30, 0.0333…)` is called (vtable slot `0x20`; period double `0x3fa11111_11111111` ≈ 30 ms = ~30 Hz). When all three go to zero, `StopRepeating` fires (slot `0x14`). The `LPeriodical` subobject is at `this + 0x30`.

`SuspendAllPolling` / `ResumeAllPolling` set `this[0x3c]` and start/stop the repeater without touching the per-cookie enables — this is how the panel freezes the pump when the window loses focus.

**0x601 is the oddball:** sample-rate "polling" is really a rate-change notifier, not a VU stream. Multiplexing it with VU on the same LPeriodical tick means the rate update is cheap but means the kext must expose a current-rate read on the same path as the peak arrays.

## 2. View-side subscription (`CDeviceSettings`)

`CDeviceSettings::AddPollingType` / `RemovePollingType` forward through vtable slots `0x38` / `0x3c` of the CMiddleWareDevice stored at `this + 0x68`. `CSetMgrSubView::StartViewPolling` / `StopViewPolling` just set a bool at `this + 0x120` — the subview opts in/out; the owner settings object is what actually ref-counts into the LPeriodical.

## 3. Message shape (`middleWareMessage*`)

The level consumers all take `middleWareMessage *param_1` and read `*(uint32_t*)(param_1 + 0xc)` as a pointer to a `uint32_t levels[]` array indexed from 1. Layout (reconstructed from four consumer sites):

```c
struct middleWareMessage {
    uint32_t cookie;        // 0x00 — which stream (0x5e9 / 0x5f3 / 0x601 / …)
    uint32_t count;         // 0x04 — redundant with per-view strip count
    uint32_t _pad;          // 0x08
    uint32_t *levels;       // 0x0c — points at levels[0], consumers index levels[i-1]
    // 16+ bytes total; higher offsets not referenced in peak path
};
```

The sample-rate path (cookie 0x601) likely reuses the same envelope with `levels[0] = newRate`. Confirming belongs in P9-fup.

## 4. Consumer dispatch

Four consumers decoded, all living on `CMetersView`/`CMixerView`/`COutputView` (not FW1814-specific):

- `CMetersView::UpdateMixerPeakLevels` @ `d9f8c` → walks `this->mixerPairs[1..n]` (n = `this[0x124]->count`), calls widget vtable slot `0x24` with `levels[i-1]`. Gated by enable bool `this[0x12c]`.
- `CMetersView::UpdateOutputPeakLevels` @ `d9ff0` → same shape, output pairs at `this[0x128]`, gate `this[0x12d]`.
- `CMixerView::UpdateMixerPairPeakLevels` @ `de4ec` → fetches the (left-ch, right-ch) index pair for each strip via `CMiddleWareDevice` vtable slot `0x1c (GetMixerPairChannelMap)`, then calls `CMixerChannelPairView::SetVUMeters(level_L, level_R, false)`. This is the stereo-aware path.
- `COutputView::UpdateOutputPairPeakLevels` @ `e1d32` → same but through vtable slot `0x24 (GetOutputPairChannelMap)` and `COutputChannelPairView::SetVUMeters`.

Widget vtable slot `0x24` is the "set VU" entry on `CMAVUMeter` (confirmed indirectly — `CMAVUMeter` is the only VU widget class and the only one these calls make sense against).

The scalar `UpdateMixerPeakLevels` / `UpdateOutputPeakLevels` paths are used when `GetSWReturnPeaksAreEmbedded==1` (base) — mixer peaks already include SW-return embed. The paired-map paths are used for the actual stereo pair rendering.

## 5. FW1814 override — SW-return peaks are **not** embedded

`CX1814Device::GetSWReturnPeaksAreEmbedded` @ `72e86` returns **0**. The base `CXDevice::GetSWReturnPeaksAreEmbedded` returns **1**. Every other decoded device would inherit the "1" default; FW1814 overrides to 0.

**Implication:** For FW1814 the kext sends **two** peak streams per tick — the hardware mixer peaks (cookie `0x5e9`) and a separate SW-return peak block. The panel then composites them in the widget. The audiophile override at `73236` also returns 0 — same deal. The 1814/Audiophile pair is the "embedded == 0" family.

This is a critical kext-side contract: our clone must produce two separate peak arrays for FW1814, not a single pre-mixed one. The ordering of channels in the mixer-peak array must therefore exclude SW-return channels, and the SW-return peaks arrive as a distinct message (likely a different cookie — a `P9-fup` candidate).

## 6. FW1814 port counts in meters

`CFW1814MiniMetersView::GetPortSettings` @ `9e236` reads **two new RO cookies** via `CXDevice::GetHWIndexedSetting` (vtable `0x0c`):

| cookie  | size | meaning |
|---------|------|---------|
| `0x60d` | 2 B  | Number of input ports visible in the mini-meter window |
| `0x60e` | 2 B  | Number of output ports visible in the mini-meter window |

Default-on-failure for both: `2`. Both reads use the usual `(cookie, index=0, cap=2, outptr)` call shape; the `cap=2` and 2-B size suggest `uint16_t` values. These are **kext-published** and must appear in `CFW1814DeviceSettings`'s cookie table too — add rows to P7's table on next edit.

## 7. The kext-side mechanism — resolved (fup1 + fup2)

**Decompile log:** `ghidra-exports/decomp/a2-p9fup{,2,3}.log`.

> **Correction (2026-10-06, fup3):** earlier §7 claimed shared memory is "mapped but not on the peak hot path". That was wrong — `CXDevice::GetHWSetting` for cookies `0x5e9` and `0x5f3` reads directly from `this[0x68]` (the shm). Decoded in §7.5 below. §7.1 is kept for the mapping call site; §7.3 is superseded.

### 7.1 Shared memory IS mapped — and IS used for peaks

`CXDevice::CXDevice` @ `0x6c2b8` calls:

```c
_IOConnectMapMemory(conn,
                    1,                       /* memory type = clientMemoryForType(1) */
                    mach_task_self,
                    &this[0x68],             /* out: mapped address — this becomes CXDevice::fSharedMem */
                    &dummySize,
                    0x1001);                 /* kIOMapAnywhere | ? */
```

- Only called when ctor's `createSharedMem` bool is `true` — **CX1814Device ctor passes `true`**, so FW1814 maps the type-1 shm.
- Unmapped in `CXDevice::~CXDevice` with the matching `_IOConnectUnmapMemory`.
- Pointer lives at `CXDevice[+0x68]`.

**The shm is the live peak-level buffer** (see §7.5). The kext owns it, updates it continuously, and the panel reads cookie-indexed slices of it on every ~30 Hz tick. All other cookies in `GetHWSetting` go through `IORegistryEntryCreateCFProperties` (slow); the two peak cookies are the only ones that take the shm fast-path.

### 7.2 The pump body — `CMiddleWareDevice::SpendTime` @ `0x638e2`

```c
void CMiddleWareDevice::SpendTime(EventRecord *this) {
    if (this->fPollSuspended /*[+0x3c]*/) return;

    if (this->fPollOutputEnabled /*[+0x50]*/) {              /* cookie 0x5f3 */
        if (this->fOutputBuf /*[+0x40]*/ == NULL) {
            uint16_t n  = this->GetNumberOfOutputChannels(); /* vtable[0x18] */
            this->fOutputBufSize /*[+0x44]*/ = n * sizeof(uint32_t);
            this->fOutputBuf                 = operator new[](n * 4);
        }
        if (this->GetHWSetting(0x5f3, size, buf) == 0) {    /* vtable[0x20] */
            this->SetCPSetting(0x5f3, size, buf);           /* vtable[0x4c] — broadcast */
        }
    }

    if (this->fPollMixerEnabled /*[+0x54]*/) {               /* cookie 0x5e9 */
        if (this->fMixerBuf /*[+0x48]*/ == NULL) {
            uint16_t n  = this->GetNumberOfMixerChannels(); /* vtable[0x1c] */
            this->fMixerBufSize /*[+0x4c]*/ = n * sizeof(uint32_t);
            this->fMixerBuf                 = operator new[](n * 4);
        }
        if (this->GetHWSetting(0x5e9, size, buf) == 0) {
            this->SetCPSetting(0x5e9, size, buf);
        }
    }
}
```

The pump:

1. Allocates a `uint32_t[channel-count]` buffer **once** per cookie (lazy on first tick).
2. Calls `vtable[0x20] = CXSelectorHPDevice::GetHWSetting(cookie, size, buf)` — this is the normal property-read path, same one used for every other cookie. For `0x5e9`/`0x5f3` the kext must publish an NSArray-of-NSNumbers (or equivalent) that fills `buf` with `n` 32-bit level values. **Shared memory is not involved on this path at the panel side.**
3. On success, calls `vtable[0x4c] = CMiddleWareDevice::SetCPSetting(cookie, size, buf)` — which `LBroadcaster::BroadcastMessage(0x3f2, {cookie, _, size, buf})`. The `0x3f2` payload struct lays out as `{cookie@0, flag@4, size@8, buf@c}` — this is the `middleWareMessage` struct §3 decoded. The `+0xc = buf` field matches `levels` exactly.
4. `CMetersView::ListenToMessage(0x3f2, msg)` filters by `msg->cookie` and dispatches to `UpdateMixerPeakLevels` (0x5e9) or `UpdateOutputPeakLevels` (0x5f3). Note: `UpdateMixerPeakLevels` consumes bodyType `0x5e9` and `UpdateOutputPeakLevels` consumes `0x5f3` — mapping confirmed.

**Cookie 0x601 (sample rate) is NOT polled by SpendTime.** Despite being accepted by `AddMWPollingType`, the sample-rate branch is absent from the pump body. The `0x601` subscription only causes `LPeriodical::StartRepeating` to run (so the pump exists), but the actual rate-change delivery for cookie `0x601` must be via the `0x3f2` broadcast from the writer side (P7-fup / P8). Correction to §1: cookie `0x601` is accepted by the pump accounting but has no `SpendTime` consumer — it is a shared-timer vehicle only.

### 7.3 Pull mechanism — resolved

**Peak-meters are pulled from `clientMemoryForType(1)` shared memory via a cookie-indexed read in `GetHWSetting`.** `IOConnectMethodScalarIScalarO` is used by only one call site in the whole panel: `CXDevice::TerminationWaitThread` @ `0x6d108` sends selector `0x1fa` (= `506`). **This is NOT a shutdown blocking ping — it's a vestigial ProTools entry.** A1-Q11 shows selector 506 is `EntryProToolsOpen` @ `0x7482`, a hard-coded `xor %eax, %eax; ret` no-op left over from a 2008-era ProTools LE integration. The panel's "TerminationWaitThread" calls the no-op, gets instant return, nulls `this->shm` (`this[0x68] = NULL`), and `pthread_exit`s immediately. The thread's name reflects an abandoned design intent — in both current kext and panel, this is dead code that neither blocks nor meaningfully terminates. **Our clone MUST NOT make selector 506 blocking** — doing so would permanently hang the panel's spawned thread. Treat it as a no-op return-zero stub to match the reference.

So the kext-side peak contract reduces to: **publish a live `uint32_t[]` peak-level buffer at `clientMemoryForType(1)`**, laid out in the specific order §7.5 decodes, and update it continuously (hardware DSP side rate — the panel samples at ~30 Hz but reads whatever is current).

### 7.4 CX1814Device vtable (fup2) — no `DoTask` override

Dumping the CX1814Device main vtable @ `001ccd98` (first 40 slots) confirms:

- `[+0x18] GetNumberOfOutputChannels = CX1814Device::GetNumberOfOutputChannels` → feeds pump buffer sizing for cookie `0x5f3`.
- `[+0x1c] GetNumberOfMixerChannels = CX1814Device::GetNumberOfMixerChannels` → feeds pump buffer sizing for cookie `0x5e9`.
- `[+0x20] GetHWSetting = CXSelectorHPDevice::GetHWSetting` → FW1814 inherits from `CXSelectorHPDevice`, not plain CXDevice. The selector-HP intermediate class handles the headphone-pair selector cookies.
- `[+0x34] SpendTime = CMiddleWareDevice::SpendTime` → **no FW1814 override**. The pump body is base-class only; everything FW1814-specific is driven by its overrides of `GetNumberOf*Channels` + `GetSWReturnPeaksAreEmbedded`.
- `[+0x6c] GetSWReturnPeaksAreEmbedded = CX1814Device → 0` → confirmed again.

**Implication for FW1814 SW-return peaks:** `GetSWReturnPeaksAreEmbedded==0` means the SW-return peaks live at a **further offset** inside the same shm after the regular mixer-channel region (see §7.5 for the exact math). SW-return peaks for FW1814 are **in the same shm but at a different index range** than mixer peaks — not a separate buffer, not a separate cookie, but a separate contiguous region.

### 7.5 Shm layout — decoded (fup3)

`CXDevice::GetHWSetting` has special-case branches for cookies `0x5e9` and `0x5f3` that read directly from `this[0x68]` (the shm) and convert u32 raw peak values to dBFS int (floored at `-0x40` = -64 dB) using the `log10` + IEEE-754 bias trick:

```c
int32_t dB_or_floor(uint32_t raw) {
    if (raw == 0) return -64;
    double v = ((double)(raw | 0x4330000000000000) - 4503599627370496.0);  // u32→double fast path
    return (int32_t)ceil(log10(v) * K);                                     // K = DAT_000f8b40 * DAT_000f8b48
}
```

**Cookie `0x5f3` — output peaks** (`param_3[0..outputChannels-1]`):

```c
uint32_t base;
if (GetSWReturnPeaksAreEmbedded())   // base=1 (FW410 etc.): shm offset = GetNumberOfMixerChannels()
    base = GetNumberOfMixerChannels();
else                                  // FW1814: shm offset = GetNumberOfInputChannels()
    base = GetNumberOfInputChannels();
for (i = 0; i < GetNumberOfOutputChannels(); i++)
    param_3[i] = dB_or_floor(shm[base + i]);
```

**Cookie `0x5e9` — mixer peaks** (`param_3[0..mixerChannels-1]`):

```c
if (GetSWReturnPeaksAreEmbedded()) {  // embedded=1 (base): simple linear read
    for (i = 0; i < GetNumberOfMixerChannels(); i++)
        param_3[i] = dB_or_floor(shm[i]);
} else {                              // FW1814 (embedded=0): two-region read
    uint32_t inCh  = GetNumberOfInputChannels();
    uint32_t outCh = GetNumberOfOutputChannels();
    // Region A: inputs (shm[0..inCh-1]) → param_3[0..inCh-1]
    for (i = 0; i < inCh; i++)
        param_3[i] = dB_or_floor(shm[i]);
    // Region B: SW-returns start AFTER inputs + outputs
    uint32_t extra = GetNumberOfMixerChannels() - inCh;   // = SW-return channel count
    for (i = 0; i < extra; i++)
        param_3[inCh + i] = dB_or_floor(shm[inCh + outCh + i]);
}
```

**Decoded shm layout** (per-device, u32 raw-peak slots):

```
offset 0                              shm[0..inCh-1]     mixer-input peaks
offset inCh                           shm[inCh..inCh+outCh-1]  output peaks (always this region)
offset inCh+outCh (FW1814 only)       shm[inCh+outCh..]  SW-return peaks
```

For FW1814 with the plug map we know (`GetNumberOfInputChannels()` ≈ 18 analog+digital inputs, `GetNumberOfOutputChannels()` ≈ 14 outputs, SW-returns = `MixerCh − InCh`), the shm total size is approximately `(InCh + OutCh + SWRetCh) * sizeof(uint32_t)` ≈ one page. The 0x1001 mapping flag passed to `_IOConnectMapMemory` is consistent with a page-sized or smaller mapping.

Only cookies `0x5e9` and `0x5f3` consume the shm. All other cookies in `GetHWSetting` fall through to `IORegistryEntryCreateCFProperties` + NSNumber extraction (slow path, suitable for setting changes that don't need 30 Hz freshness).

## 8. Clone implications (revised after fup1 + fup2)

- Kext does **not** send `bodyType 1` or `bodyType 2` Mach notifications. The 44-B `MAudioSettingsMessage` wire format carries only `bodyType==0` (property-dict changed). P2 is corrected accordingly.
- Peak-meter publishing is a **shared-memory contract**: the kext MUST implement `IOUserClient::clientMemoryForType(1, ...)` returning an `IOMemoryDescriptor` over a kext-owned `uint32_t[]` peak-level buffer, laid out as `[inCh | outCh | swRetCh]` per §7.5. The DSP side writes raw u32 peaks continuously; no per-tick synchronization — the panel accepts whatever snapshot it reads (no tearing-protection needed, levels are coarse). FW1814 must size the buffer `inCh + outCh + swRetCh` words; devices with `SupportsSWReturnPeaksEmbedded=1` (FW410 et al.) size it `mixerCh + outCh` words.
- No peak-related property keys exist — `GetHWSetting(0x5e9/0x5f3)` doesn't consult properties for the peak path; it only consults the shm. So no `MAMixerPeaks` or `MAOutputPeaks` NSString publication is needed.
- **Scalar selector `0x1fa` (`506`) must remain a return-zero no-op** on the user client (match A1-Q11's `EntryProToolsOpen` stub). The panel's "TerminationWaitThread" calls it, gets 0 back, nulls its shm pointer, and exits — all design-intent "wait for termination" semantics are dead code on both sides. **DO NOT** make this selector blocking — that would hang the panel thread forever.
- For FW1814 specifically, `GetNumberOfMixerChannels` must include SW-return channels in its count so the `0x5e9` array sizes correctly. `GetSWReturnPeaksAreEmbedded == 0` is consumed by per-strip rendering (not by the pump), so the kext does not need a separate SW-return peak publication.
- Publish RO cookies `0x60d` (u16 input-port count) and `0x60e` (u16 output-port count); default on failure is `2`.
- Cookie `0x601` must be published as the current nominal rate (u32 Hz) because `AddMWPollingType(0x601)` is a valid subscription; the pump accepts it but does not consume it inside `SpendTime` — the current rate is read by consumers via the normal `GetHWSetting(0x601)` path on-demand, not off the ~30 Hz tick.
- No "start/stop peak reporting" selector exists; the kext should sample peaks continuously (cheap on the DSP side) and let the panel pull them. Add this to the kext userclient selector table **only if** profiling later shows a measurable idle cost.

## 9. Status

- Closes P9 for pump mechanism, consumer dispatch, FW1814 SW-return-not-embedded override, port-count cookies `0x60d`/`0x60e`.
- Closes **P9-fup1** — pull mechanism confirmed as `GetHWSetting` via property dict; the mapped `clientMemoryForType(1)` region is not on the peak path.
- Closes **P9-fup2** — no FW1814 `DoTask` override; `CX1814Device` inherits from `CXSelectorHPDevice` and only overrides channel counts / SW-return-embedded-flag / HP-routing helpers.
- Applied the **P2 correction** inline in `P2-notification-receive.md` — bodyType 1/2 lines removed from the dispatch.
- New follow-ups opened:
  - **P9-fup3** — decode `CXDevice::GetHWSetting` @ `0x6e2ca` to see what `this[0x68]` shared-memory snapshot is used for (and whether it has any peak-meter role).
  - **A1-selector-0x1fa** — resolved: it's a no-op (A1-Q11 `EntryProToolsOpen` stub); nothing to register. The panel's "TerminationWaitThread" is dead code — mirror the stub exactly, do not implement blocking semantics.
