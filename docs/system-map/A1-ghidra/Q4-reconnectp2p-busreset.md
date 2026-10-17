# Q4 — `ReconnectP2P` + `BusReset` — stream-kick vs real reconnect

> **Correction (2026-10-14) — power-cycle timeline** (evidence: [PROVENANCE-RESOLUTIONS.md §R25](../PROVENANCE-RESOLUTIONS.md), captures 72 and 73). **Cold-start timeline measured (capture 73, interface switched off/on):** off → two bus resets; on → bootloader found (`M-FW: Bootloader active: starting firmware`, kext `FirmwareStart`) → ~170 ms of 5 ms polling sleeps → bus reset → firmware active +3.3 s after power-on (`Firmware active: registering metanub`) → `Reregister: device node vanished` → clock sets (300×2, 2500) → one-second mixer resend (the log warns `Trying to resend mixer settings too often`) → the standard stream-start chain again; quiet after ~14.7 s. **`ReconnectP2P`'s `IOSleep(1100)` did not occur in this scenario**: the device node vanished and a fresh `FW1814Device` was instantiated, so the live-device BusReset→ReconnectP2P path (R4) remains unobserved. The 11 stale `FWUserClient`s created by my earlier process restarts disappeared with the old device instance (leak is bounded by device lifetime, not permanent).


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - **ReconnectP2P / BusReset** (R4): `vt[0xcb8]` = `ConnectAllPlugs`, `vt[0xcc0]` = `DisconnectAllPlugs`; the tail runs `SetClockSourceInternal(fMode, blankSlate=1)` when `[engine+0x2d0]==0` (`cmpb $0,0x2d0; je` @0xe22c-0xe233 — inverted in earlier text). `[+0x2d0]` is the **external-sync mode flag** (written only at `0x53f0`, which refuses with "Cannot change sync mode while audio/MIDI streams are running"), not `fRunning`. `BusReset` stores PMF `0xBF1` (@0xe5ab) = virtual `vt[0xbf0]` = `ReconnectP2P`, then `InvokeOnNewThread` — one path, not two. The tail is an HSCI clock-source resend, not an AV/C stream kick.
> - The '1100 ms PLL-lock recovery window' rationale has **no source** (**unverified**); only the `IOSleep(0x44c)` value is verified.


**Status:** done.

**Scope:** `0xe1ea` (`FW1814Device::ReconnectP2P`), `0xe53a` (`FW1814Device::BusReset`), `0x3037` → file offset `0x4c9a` (`FWBaseEngine::ReconnectAfterBusReset`), `0x1aba` (`FWAudioDevice::BusReset`).

## Answer in one line

**`ReconnectP2P` is a 1100 ms stream-kick sequence** (quiesce → disconnect-all → wait 0x44c=1100ms → reassert clock source via vtable 0xb60); **`BusReset` is a thread-dispatch** (spawn a new worker that calls the device's bus-reset handler indirectly). They are different code paths — `ReconnectP2P` is driven from user space (preference-pane re-sync); `BusReset` fires from the IOKit bus-reset notification.

## `FW1814Device::ReconnectP2P()` @ 0xe1ea

```c
void FW1814Device::ReconnectP2P() {
    FWEngine *e = this->fEngine;              // [this + 0xe8]
    e->vtable[0xcc0](e);                      // DisconnectAllPlugs
    e->vtable[0xcb8](e);                      // Quiesce / StopDCL
    IOSleep(0x44c);                           // 1100 ms

    if (!this->fEngine || !this->fEngine->fRunning /* [0x2d0] */)
        return;                               // was never running — nothing to re-arm

    // Tail-call this->vtable[0xb60](mode=fMode[0x144], flag=1)
    //   → SetClockSourceInternal(this, mode, /*blankSlate*/1) — the stream kick
    this->vtable[0xb60](this, this->fMode, /*reasserted*/ 1);
}
```

The 1100 ms sleep matches the FW1814's PLL-lock recovery window after disconnecting the isoch channels. The final tail-call is the AV/C stream-kick memory confirms the hardware needs (`project_fw1814_stream_kick`).

## `FW1814Device::BusReset()` @ 0xe53a

```c
void FW1814Device::BusReset() {
    void *context = this->vtable[0x688](this);     // getBusResetContext()
    // Allocate a m_audio_FWb::UKernelCallHelpers::CallCenter (0x20 bytes)
    CallCenter *center = new CallCenter();
    center->vtable      = &CallCenter_vtable;
    center->next        = NULL;
    center->target      = this;                     // [+0x18]
    center->context     = context;                  // [+0x10]
    if (this) {
        CallCenter *entry = new CallCenter();
        entry->vtable  = &CallCenter_entry_vtable;
        entry->target  = this;
        entry->selector= 0xBF1;                     // 3057 — bus-reset selector
        entry->next    = NULL;
        center->next   = entry;
    }
    UKernelCallHelpers::InvokeOnNewThread(center);  // spawn worker thread
}
```

Two-node linked list of `CallCenter` objects passed to `InvokeOnNewThread`. The worker thread will:
1. Invoke the device's bus-reset context callback.
2. Dispatch selector 0xBF1 (likely `FWEngine::BusResetWorker` or a message-dispatch index).

**No synchronous work happens in `BusReset()` itself** — it hands off to a dedicated thread so the FireWire bus-reset handler doesn't block the FireWire workloop.

## `FWBaseEngine::ReconnectAfterBusReset()` @ file offset 0x4c9a

Full decode deferred (large function), but structure visible:
1. Walk the DCL program collection via `TCollectionIterator`.
2. For each program, call `program->vtable[0x158]` (`ReconnectAfterBusReset` on the program).
3. Re-issue plug connects (`ConnectAudioMIDIChannels` subset).
4. Resume HSCI heartbeat.

## Delta summary

| Aspect | Reference | Clone | Impact |
|---|---|---|---|
| `ReconnectP2P` sequence | disconnect → stop DCL → 1100 ms → clock-reassert | missing or short | **Match required** — bus-reset race if too short |
| 1100 ms sleep | `IOSleep(0x44c)` | TBD | Hardware-timing constant; mirror exactly |
| `BusReset` thread model | `InvokeOnNewThread(CallCenter list)` | TBD | Needed to keep FireWire workloop non-blocking |
| Selector 0xBF1 | second entry in call-center chain | — | Identify what this maps to (Q4a) |

## Follow-up queue impact

- **Q4a:** identify selector 0xBF1 — probably a method-dispatch index on `FWEngine::BusResetWorker` or similar. Search the dispatch table at this device's vtable[0x688].
- Follow-up: full decode of `FWBaseEngine::ReconnectAfterBusReset` once Q7 (FWMetaNub lifecycle) lands — the two interact when a bus reset coincides with a bootloader↔runtime transition.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- 1100 ms = `0x44c` immediate at 0xe216 (`callq _IOSleep`).
- `fRunning` flag at `[fEngine + 0x2d0]` — same bit cleared in `DisconnectAllPlugs` (observed in Q3).
