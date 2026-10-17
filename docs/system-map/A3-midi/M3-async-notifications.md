---
title: A3-M3 — Async notification scaffold (and why MIDI doesn't actually use it)
part: A3
status: done
date: 2026-10-06
---

# M3 — Async notification scaffold (unused in the MIDI data path)

**Headline:** `MAFWBeBoBAsyncNotification` is a complete `IOConnectSetNotificationPort` + CFMachPort → CFRunLoop plumbing class, identical in shape to the A2 panel's receive path. **But the MIDI plugin never instantiates it for MIDI data** — the ivar `fAsyncNotification` has no WRITE references, and the production RX path is a pure 1500 Hz `CFRunLoopTimer`. Device add/remove is handled by the sibling class `MAFWBeBoBServiceNotification` (IOKit matching notifications), not by this one.

## 1 — Class plumbing

**`-[MAFWBeBoBAsyncNotification initWithType:connection:runLoop:target:selector:]`** (`a3-m345.log:329–392`, `0x00001f37`):

1. Store `fTarget`/`fAction`/`fRunLoop` from args.
2. One-shot init of file-scope `sMasterPort` via `IOMasterPort(0, &sMasterPort)` (`log:358`). Shared across all instances.
3. `IONotificationPortCreate(sMasterPort)` → `fNotificationIOKitPort` (`log:363`).
4. `IONotificationPortGetMachPort()` → raw mach port (`log:366`).
5. `CFMachPortCreateWithPort(NULL, machPort, StaticHandleMachMessage, &ctx, NULL)` → `fNotificationCFMachPort` (`log:372`). Context is the self pointer.
6. `CFMachPortCreateRunLoopSource` → `fMachPortRunLoopSource`; added to `fRunLoop` under `kCFRunLoopDefaultMode` (`log:376–380`).
7. `IOConnectSetNotificationPort(connection=param_4, type=param_3, port=machPort, refCon=0)` (`log:382`). **No `bodyType` here** — compare to A2-P2 which handles `bodyType==0`.

Failures raise `NSException` (`BogusErrorFroMIOMasterPort`, `FailedMostHeinously`, `SeriousFailure` strings at `log:361/389`).

**`+[MAFWBeBoBAsyncNotification userClientNotificationOfType:...]`** (`log:396–413`, `0x000021d4`): class-method convenience — `alloc` → `initWith...` → `autorelease`. Caller is expected to retain if it wants to keep it.

**`-[MAFWBeBoBAsyncNotification dealloc]`** (`log:417–448`, `0x0000228e`): removes runloop source, invalidates + releases CFMachPort, destroys IONotificationPort, releases `fTarget`.

**`StaticHandleMachMessage(CFMachPort*, msg, size, info)`** (`a3-m345b.log:186–209`, `0x000020fe`): wraps the delivered selector in an `NSAutoreleasePool`, extracts `[info target]`/`[info action]`, pulls `CFMachPortGetPort()`, then `objc_msgSend(target, action, port, msg, size)`. Standard CFMachPort dispatcher.

## 2 — The MIDI plugin **does not use this class for MIDI data**

Cross-reference on symbol `_OBJC_IVAR_$_MAFWMIDIDeviceBeBoB.fAsyncNotification` (`0x0000b1c8`):

```
0000b1c8  _OBJC_IVAR_$_MAFWMIDIDeviceBeBoB.fAsyncNotification
    ref @ 000099d8  type=DATA  (ivar-table entry)
    ref @ 00002f1a  type=READ  (stop:  [fAsyncNotification release])
    ref @ 00002e6d  type=READ  (start: [fAsyncNotification retain])
```

Only READ refs. **The ivar is never written.** `start` retains nil, `stop` releases nil (no-ops on nil in Obj-C) — leftover scaffolding, likely from an older code path that was superseded by polling. **Verified by xref** (not a hypothesis).

Instead, `-[MAFWMIDIDeviceBeBoB start]` (`a3-m345.log:5–66`, `0x00002d47`) creates a `CFRunLoopTimer` at `DAT_00007470` interval (1500 Hz, see M4 §4) with callback `HandleMIDIInputCallout`, and adds it to `MIDIGetDriverIORunLoop()`. The timer calls `-checkForMIDIInput` (M6), which polls the shared-memory rings.

## 3 — What async notifications *are* used for

**Device add/remove** is handled by `MAFWBeBoBServiceNotification` (per A3-M1 §7 and A3-M2 §1), via `IOServiceAddMatchingNotification` + `IOServiceAddInterestNotification`. That path **does** deliver callbacks (`driverInstanceMatched:` / `driverInstanceTerminated:`). But that is a *registry* notification, not a *user-client* notification — it uses the IOKit matching port, not `IOConnectSetNotificationPort`.

So **no `bodyType`-dispatched user-client messages are consumed by the MIDI plugin in production.** The `MAFWBeBoBAsyncNotification` class is live code only in the sense that it compiles and the ivar exists; the plugin's MIDI RX pipeline is purely polled.

## 4 — Comparison with A2-P2 (panel)

| | A2 panel (`com_m_audio_FWKIPCUserClient`) | A3 MIDI plugin |
|---|---|---|
| Uses `IOConnectSetNotificationPort`? | Yes — delivers `bodyType==0` meter frames | **No** — only polling |
| Timer-driven? | Partially (polls for level meters at low rate) | **Yes, exclusively** — 1500 Hz |
| AsyncNotification class present? | Yes, actively instantiated | Yes, dead ivar |

**Implication for the kext user client we're building:** the MIDI side **does not need an async callback path**. The kext must expose a shared-memory ring that userspace maps via `IOConnectMapMemory` (M4) and polls at 1500 Hz. No `IOUserClient::sendAsyncResult64` needed for MIDI data — only for the panel's meter frames.

## 5 — Clone implications

- The user client's MIDI selector surface needs **start** (= 500) and **stop** (= 501) scalar methods only — no async port registration (see M5 §1).
- Memory-mapped MIDI rings are the only data channel. Kext populates on RX, polls to drain on TX.
- We **can** omit the AsyncNotification class in the clone's plugin — it is dead weight that mirrors the panel's shape without serving the MIDI path.

## 6 — Open items / not yet checked

- **Hypothesis:** `MAFWBeBoBPowerNotification` (sleep/wake) may use `IOConnectSetNotificationPort` with a different type. Deferred to M9.
- **Hypothesis:** `fAsyncNotification` was likely wired to a sysex-length or firmware-event callback in an older rev. No evidence of a current use. Verification path: `strings` + `otool -I` for every selector that references the ivar — all three refs are inside `start`/`stop`, so the hypothesis holds.
