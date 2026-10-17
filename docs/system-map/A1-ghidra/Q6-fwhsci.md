# Q6 — `m_audio_b_FWHSCI` full method set — 12-byte envelope + heartbeat

> **Correction (2026-10-15, M-C / R44):** `[dev+0x268]` is the `FWAVCUnit` (private `IOFireWireAVCUnit` copy), not an `FWHSCI`; the HSCI frame is sent with `AVCCommandWithErrorResponse` (retry loop + post-command pauses). See [`Q-MC-cmp-isoch-avc-start.md`](Q-MC-cmp-isoch-avc-start.md).

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - **HSCI clock-source frame is 16 bytes, not 12:** `bzero(…,0x10)` @0xe284/0xe2a2, length `0x10` passed to `[+0x268]->vt[0x8b8]` @0xe441 (R3). The 12-byte layout is snd-bebob `bebob_maudio.c:183-198`. Bytes 10-15 content is **unverified**.
> - **HSCI threading:** completion is dispatched via `InvokeOnNewThread` (@0x1d89e), re-arm = `50 + [+0xd4]` ms (@0x1d50d-0x1d513) (R6). "No async reads outside the HSCI window" is wrong: `updateP2PCount` reads PCRs (Q-CMP §3).
> - Method addresses: `HSCIReadComplete 0x1d80e`, `Release 0x1d8a4`, `Stop 0x1d8fe`, `Start 0x1d94e` (header table had 0x1d81e/8c4/8dc/8fb). `+0xd4` is zeroed on the error path and added to the 50 ms re-arm; the error path re-arms 1000 ms. `[+0xc0]` is a byte length, not a rate.


**Status:** done.

**Scope:** symbols `__ZN16m_audio_b_FWHSCI*` in `docs/system-map/A1-ghidra/otool-exports/mafw.asm` lines 34876–35572. Primary methods at offsets `0x1d3fc` (Pause), `0x1d424` (Resume), `0x1d4d6` (HSCISafelyComplete), `0x1d600` (HSCITimerFired), `0x1d78c` (FWHSCIDeviceCallback), `0x1d81e` (HSCIReadComplete), `0x1d8c4` (Release), `0x1d8dc` (Stop), `0x1d8fb` (Start).

## Answer in one line

**HSCI is a repeating-timer-driven FireWire read-quadlet polling object. Every ~50 ms it issues a read of `0xFFC7_00600000..` for N quadlets (N = `this->rate >> 2`), parses the response into a 12-byte status envelope `{ bus_hi(2), node_hi(2), 0x600000(4), ...payload... }`, invokes `HSCIReadComplete`, and re-arms.**

## Field layout (`m_audio_b_FWHSCI` instance)

| Offset | Field | Semantic |
|---|---|---|
| `+0x10` | `IOFireWireNub *fNub` | target device |
| `+0x18` | `uint32_t fPauseGuard` | 0 = running; non-zero = paused (set via Pause, cleared via Resume) |
| `+0x1c` | `int fPauseCount` | nesting counter (Pause increments, Resume decrements) |
| `+0x30` | `IOWorkLoop *fWorkLoop` | the kernel work-loop |
| `+0xa0` | `IOFWReadQuadCommand *fReadCmd` | reused per cycle |
| `+0xb8..0xc0` | callback target/context | post-read dispatch |
| `+0xc0` | `uint32_t fRate` | poll quadlet count (likely 4 or 8) |
| `+0xc8` | `uint8_t *fScratchBuf` | zeroed before each read |
| `+0xd4` | `uint32_t fLastErrCode` | written by `HSCISafelyComplete` |

## `Pause()` @ 0x1d3fc

```c
void FWHSCI::Pause() {
    this->fPauseCount++;
    if (fPauseCount != 1 && this->fPauseGuard == 0) return;
    // tail-call fWorkLoop->vtable[0x158] (removeEventSource / cancel timer)
    this->fWorkLoop->vtable[0x158](this->fWorkLoop);
}
```

Nesting is counted; the actual timer stop happens on the first increment only.

## `Resume()` @ 0x1d424

```c
void FWHSCI::Resume() {
    this->fPauseCount--;
    if (fPauseCount != 0 || this->fPauseGuard != 0) return;
    this->fWorkLoop->vtable[0x150](this->fWorkLoop);     // add timer back
    // Schedule next fire in 50 ms
    this->fWorkLoop->vtable[0x1d0](50);                  // setTimeoutMS
}
```

## `HSCITimerFired()` @ 0x1d600 — the heartbeat

Pseudocode:

```c
void FWHSCI::HSCITimerFired() {
    uint16_t nodeID, generation;
    uint32_t fwAddrHi = 0xFFC7;         // high 16 bits of FireWire address
    uint32_t fwAddrLo = 0x00600000;     // low 32 bits — HSCI register window
    this->fNub->getNodeIDGeneration(&generation, &nodeID);

    uint32_t numQuadlets = this->fRate >> 2;
    bzero(this->fScratchBuf, numQuadlets * 4);

    if (!this->fReadCmd) {
        this->fReadCmd = new IOFWReadQuadCommand();
        if (!this->fReadCmd) return;
        this->fReadCmd->vtable[0x1d0](
            this->fReadCmd,
            {.addressHi = fwAddrHi, .addressLo = fwAddrLo},   // 48-bit FireWire address
            this->fScratchBuf,
            numQuadlets,
            FWHSCIDeviceCallback,                              // completion callback
            this);
    } else {
        // Reinitialize existing command with current address.
        this->fReadCmd->vtable[0x1e0](...);
    }
    if (this->fReadCmd->submit() != 0) {
        this->fReadCmd->vtable[0x140](0);                      // reset
        this->fWorkLoop->setTimeoutMS(1000);                   // retry in 1 s
    }
}
```

The **FireWire address `0xFFC7:0x00600000`** is the HSCI status window. Any clone that polls this hardware must use these exact bytes.

## `FWHSCIDeviceCallback(void *ctx, int status, IOFireWireNub*, IOFWCommand*)` @ 0x1d78c

Thin stub: cast `ctx` → `FWHSCI*`, call `this->HSCIReadComplete(status)`.

## `HSCIReadComplete(int status)` @ 0x1d81e

Branches on `status`:
- `== 0`: parse the 12-byte reply (first 3 quadlets); dispatch registered listeners (vtable[0xb8]+callback); re-arm timer in 50 ms.
- `!= 0`: increment `fLastErrCode`, retry in 1 s (1000 ms).

**The 12-byte envelope confirmation:** matches `project_fw1814_hsci_envelope` memory. The payload is 3 big-endian quadlets after the FireWire bus/node prefix.

## `Start(IOWorkLoop *wl)` @ 0x1d8fb / `Stop()` @ 0x1d8dc / `Release()` @ 0x1d8c4

Standard IOKit lifecycle: `Start` installs a timer event source into `wl` with `HSCITimerFired` as fire handler, 50 ms initial delay. `Stop` removes the event source. `Release` tears down the read command and the scratch buffer.

## Delta summary

| Aspect | Reference | Clone (hypothesis) | Impact |
|---|---|---|---|
| Poll address | `0xFFC7:00600000` | required | Hardware-defined |
| Poll quadlet count | `fRate >> 2` | required | panel configures this |
| Timer period | 50 ms normal, 1000 ms after error | required | PLL tracking |
| 12-byte envelope | 3 quadlets payload | required | memory `project_fw1814_hsci_envelope` |
| Pause-counting | nesting via `fPauseCount` | should mirror | clock-source changes nest Pause/Resume |

## Follow-up queue impact

- **Q6a (new):** decode `HSCIReadComplete` payload parsing — identify which bytes of the 12-byte envelope map to {clock_lock, sample_rate_detect, digital_input_present, …}. Needed to drive the control-panel status LEDs.
- Follow-up: locate where listeners register (`vtable[0xb8]+callback`). Likely `AddHSCIListener(void(*)(void*,uint8_t*), void*)` on the HSCI object.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- `0xFFC7:0x00600000`: direct immediates at 0x1d63b (`$0xffc7`) and 0x1d641 (`$0x600000`).
- Timer periods: 0x32 = 50 ms (0x1d464), 0x3e8 = 1000 ms (0x1d776).
- Sentinel debug values `0xdead`/`0xcafebabe` written to the stack scratch slots before `getNodeIDGeneration` — classic uninit-detection pattern.
