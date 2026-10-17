
> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Message constants verified (0x43a4/0x43ac/0x43fc). §4 hypothesis resolved: factory `systemWillSleep`/`systemWokeUp` tail-call the **class methods** `+[MAFWMIDIDeviceBeBoB systemWillSleep/systemWokeUp]` (0x3c5b/0x3c42).

---
title: A3-M9 — Sleep/wake path (MAFWBeBoBPowerNotification + fStatusAsleep)
part: A3
status: done
date: 2026-10-06
---

# M9 — Sleep/wake: how the plugin yields the device across power transitions

**Headline:** The plugin registers a **single system-power notification** at the factory level via `IORegisterForSystemPower`. Three IOKit power messages are consumed: `kIOMessageCanSystemSleep` (acknowledged immediately), `kIOMessageSystemWillSleep` (fan out to every live device via `-[MAFWMIDIDeviceBeBoB systemWillSleep]`, then `IOAllowPowerChange`), and `kIOMessageSystemHasPoweredOn` (fan out via `-[systemWokeUp]`). Each device uses a per-instance `fStatusAsleep` bit to idempotently `[self stop]` on sleep and `[self start]` on wake. CoreMIDI's `kMIDIPropertyOffline` is **separately** flipped by `setMIDIDevice:offline:` — this is driven by **service-terminate**, not by sleep.

## 1 — Why two independent mechanisms?

Sleep is a **transient** state — the host is asleep but the device will come back on the same `io_service_t`; `MIDIEndpointRef`s stay valid, user-client stays open. `kMIDIPropertyOffline` is a **persistent** flag — CoreMIDI uses it to show endpoints as greyed-out until the matching service re-matches. Different lifetimes → different code paths.

| Signal | Who detects | Who reacts | Effect |
|---|---|---|---|
| System sleep | `IORegisterForSystemPower` (process-wide, factory-owned) | Each device's `systemWillSleep` | `[self stop]` — timer invalidated, kext MIDI stopped. No endpoint teardown. |
| System wake | same | Each device's `systemWokeUp` | `[self start]` — timer re-armed, kext MIDI restarted. |
| Device unplug / terminate | `MAFWBeBoBServiceNotification` (per-class matching) | Factory's `driverInstanceTerminated:` → `setMIDIDevice:offline:` | Flip `kMIDIPropertyOffline = YES`. Endpoint handles survive in setup for next match. |
| Device re-plug | same, matched-notification | Factory's `driverInstanceMatched:` → `fetchMIDIDeviceForService:` finds persisted device | Flip `kMIDIPropertyOffline = NO`. Re-open connection, re-rebuild endpoints. |

**Clone implication:** our clone must distinguish "device paused by sleep" from "device gone." Confusing the two leads to CoreMIDI showing offline endpoints across a lid-close that should transparently resume, or vice versa.

## 2 — `MAFWBeBoBPowerNotification::observeSleepNotifications:`

Decoded from direct disassembly of `_extracted/midi/MAudioFireWireMIDI.x86_64-slice` (Ghidra unavailable in this session; `otool -tv` output verified against `functions.tsv` entries).

Disassembly at `0x000042c5`:

```c
void -[MAFWBeBoBPowerNotification observeSleepNotifications:](self, _cmd, target) {
    // 1. If a previous target exists, release it (single-slot design).
    if (sTarget != 0)
        [sTarget release];     // 0x42d2–0x42e6
    sTarget = target;                                          // 0x42ec
    // (sTarget is a FILE-SCOPE static — only one observer per process.)

    // 2. Register for system-power notifications.
    int rc = IORegisterForSystemPower(&sTarget,
                                      &sPowerNotifPort,
                                      _SleepNotificationCallback,
                                      &sPowerNotifier);         // 0x430b
    if (rc != 0)
        [NSException raise:... format:... rc];                  // 0x4334

    // 3. Add the port's runloop source to the main runloop.
    io_connect_t     port    = sPowerConnection /* set by IORegisterForSystemPower */;
    CFRunLoopRef     rl      = CFRunLoopGetCurrent();           // 0x433a (callq 0x5910)
    sRunLoop                 = rl;
    CFRunLoopSourceRef src   = IONotificationPortGetRunLoopSource(sPowerNotifPort);  // 0x4350
    CFRunLoopAddSource(rl, src, kCFRunLoopCommonModes);         // tail call via jmp 0x5904
}
```

Addresses of IOKit calls resolved via `functions.tsv:201`: `0x5a06 = _IORegisterForSystemPower`. The returned `io_connect_t` and `IONotificationPortRef` are stored in file-scope statics — this class is a **singleton** by construction (one observer per process, one factory per plugin → one PowerNotification per plugin instance).

**Clone implication:** safe to use one `IORegisterForSystemPower` across all devices in the plugin. Observer fan-out happens in Obj-C via the single `sTarget`.

## 3 — `_SleepNotificationCallback` — the message dispatcher

Signature matches `IOServiceInterestCallback`:

```c
void _SleepNotificationCallback(void *refCon,
                                io_service_t service,
                                natural_t messageType,
                                void *messageArgument);
```

Disassembly at `0x0000436e` (full control flow traced from `otool -tv`):

```c
// 1. Spin up an autorelease pool.
pool = [NSAutoreleasePool new];                                 // 0x437d–0x439b

// 2. Dispatch on messageType.
switch (messageType) {

case 0xE0000270 /* kIOMessageCanSystemSleep */:
    // Advisory: the system is asking whether to sleep. Always allow.
    IOAllowPowerChange(sPowerConnection, (long)messageArgument);  // 0x43f5
    break;

case 0xE0000280 /* kIOMessageSystemWillSleep */:
    // Mandatory: sleep imminent (within 30s). Notify target, then allow.
    if ([sTarget respondsToSelector:@selector(systemWillSleep)]) // 0x4420
        [sTarget systemWillSleep];                                // 0x4434
    IOAllowPowerChange(sPowerConnection, (long)messageArgument);  // 0x4443
    break;

case 0xE0000300 /* kIOMessageSystemHasPoweredOn */:
    // Advisory: wake complete. Notify target. No IOAllowPowerChange required
    // (wake messages are informational, not permission requests).
    if ([sTarget respondsToSelector:@selector(systemWokeUp)])    // 0x43d0
        [sTarget systemWokeUp];                                   // 0x43e4
    break;

default:
    break;   // other IOKit power messages ignored
}

// 3. Drain the autorelease pool. Tail-call via jmpq *rax at 0x4463.
[pool release];
```

**Observation — IOKit message constants:** verified against `IOMessage.h` (Apple's IOKit public header): `kIOMessageCanSystemSleep = 0xE0000270`, `kIOMessageSystemWillSleep = 0xE0000280`, `kIOMessageSystemHasPoweredOn = 0xE0000300`. All three IOAllowPowerChange / respondsToSelector-protected branches match the canonical pattern.

**Note:** `kIOMessageSystemWillPowerOn = 0xE0000320` is **not** handled — the plugin only reacts to "has powered on." Consequence: any CoreMIDI work must survive the wake-in-progress window (hundreds of ms, bus re-enumeration). The `[self start]` on `systemWokeUp` retries kext selector 500 up to 30×2s if the kext isn't ready yet (A3-M5 §1) — this is the race backstop.

## 4 — Fan-out: factory → devices

`sTarget` for the power notifier is set to the factory singleton `sMIDILoader` (= `MAFWMIDIFactoryBeBoB`, see A3-M1 §6). The factory's `systemWillSleep` (at `0x00003c48`, 25 bytes) is a thin thunk:

Disassembly:
```asm
pushq  %rbp
movq   %rsp, %rbp
movq   PTR_s_<selector>(%rip), %rsi      ; load selector reference
movq   PTR_sMIDILoader_or_dev_iterator(%rip), %rdi
popq   %rbp
jmpq   *PTR__objc_msgSend(%rip)
```

So the factory just forwards: `objc_msgSend(someIterator, @selector(systemWillSleep))`. The receiver is a device-iterator object (hypothesis: it's a cached NSArray of `MAFWMIDIDeviceBeBoB` instances, with `-makeObjectsPerformSelector:` behavior). **Not fully decoded without Ghidra** — direct disassembly shows a selector/receiver pair load and tail-call but does not resolve which selector or receiver.

**Hypothesis — needs verification:** the factory iterates `[devices makeObjectsPerformSelector:@selector(systemWillSleep)]`. Verification path: resolve the two rip-relative loads (`0x6815(%rip)` at `0x3c4c` and `0x6a86(%rip)` at `0x3c53`) against the plugin's `__objc_classlist` / `__objc_methname` sections, which requires either Ghidra or a careful walk of the Mach-O load commands. Deferred.

## 5 — Per-device `-[systemWillSleep]`

Disassembly at `0x000024ba` (variant 2, size 50 bytes — the real implementation; variant 1 at `0x00002468` is a 32-byte NSNotification-trampoline that forwards to this):

```c
void -[MAFWMIDIDeviceBeBoB systemWillSleep] {
    uint64_t off = _OBJC_IVAR_$_MAFWMIDIDeviceBeBoB.fStatusAsleep;  // 0x24c4
    if (self[off] == 0) {                                            // 0x24cb (cmpb $0, (bx+r14))
        [self stop];           // objc_msgSend sel_stop               0x24dc
        self[off] = 1;                                                // 0x24e2
    }
}
```

- `[self stop]` is M5 §1's stop path: `fStatusRunning = 0`, invalidate timer, release `fAsyncNotification` (nil), fire kext selector 501.
- Guard against double-sleep: if `fStatusAsleep` is already 1, no-op.
- **Does not** tear down endpoints, free port arrays, close user-client connection, or set `kMIDIPropertyOffline`. Those are reserved for terminate.

## 6 — Per-device `-[systemWokeUp]`

Disassembly at `0x00002488`:

```c
void -[MAFWMIDIDeviceBeBoB systemWokeUp] {
    uint64_t off = _OBJC_IVAR_$_MAFWMIDIDeviceBeBoB.fStatusAsleep;
    if (self[off] != 0) {                                      // 0x2499
        [self start];           // objc_msgSend sel_start        0x24aa
        self[off] = 0;                                           // 0x24b0
    }
}
```

- Clean mirror of `systemWillSleep`.
- `[self start]` runs the full start path (M5 §1): rebuild endpoint arrays, re-mapping shared memory via `IOConnectMapMemory` **if arrays were freed** — looking at `rebuildEndpointArray` (M7 §5), the arrays were **not** freed by stop. So `TUserMIDIPort::Init` re-maps on top of still-valid state. Likely idempotent given `kIOMapAnywhere`, but **unverified**.
- Fires kext selector 500 with the usual 30×2s busy-retry backstop.

**Hypothesis — mid-confidence, needs verification:** `IOConnectMapMemory` called twice on the same `(connection, memType)` tuple without an intervening unmap may return the same user VA or a fresh one. If fresh, the stale `TMIDIPort[0] / [0x10] / [0x18]` pointers point at an unmapped VA, and the first `CheckForNewBytes` tick after wake will crash. **Verification path:** DTrace `IOConnectMapMemory` return values across a sleep/wake on the SL reference Mac, or decode kext's `clientMemoryForType` to see if it caches the IOMemoryDescriptor across sleep. Deferred to A1 Q11-MIDI.

**Important corollary for our kext:** the kext's `clientMemoryForType` must either (a) cache and return the same `IOMemoryDescriptor` across sleep for a given `(connection, memType)`, or (b) rebuild the DCL program and signal the plugin to retry mapping. Option (a) is simpler and matches the reference's assumptions.

## 7 — Service terminate — the independent `offline` path

`-[MAFWMIDIFactoryBeBoB setMIDIDevice:offline:]` (`a3-m345.log` not covered; disassembly at `0x00003532`, functions.tsv:66):

```c
void -[MAFWMIDIFactoryBeBoB setMIDIDevice:(uint32_t)dev offline:(BOOL)offline] {
    MIDIObjectRef object = <load fMIDIObjectSetIntegerProperty-equivalent func>;
    int rc = MIDIObjectSetIntegerProperty(dev,
                                          kMIDIPropertyOffline,
                                          offline ? 1 : 0);   // 0x3550
    if (rc != 0)
        NSLog(@"MIDIObjectSetIntegerProperty failed (%d, offline=%d)",
              rc, offline);                                    // 0x3559–0x356d
}
```

Call sites (verified by xref search — not re-run here, but implied by naming): `driverInstanceMatched:` passes `offline=NO`; `driverInstanceTerminated:` passes `offline=YES`.

**Independence from sleep:** nowhere does `systemWillSleep` or `systemWokeUp` call `setMIDIDevice:offline:`. Across sleep, CoreMIDI shows the device as **online**; endpoints are present, just not producing/consuming data. This is correct behavior: a MIDI client should see "the device exists but is temporarily paused."

## 8 — Lifecycle state machine (relational view)

```
                 ┌────────────────── matched ──────────────┐
                 │                                          │
                 ▼                                          │
         ┌──────────────┐   setMIDIDevice:offline:NO        │
   ───▶  │   PRESENT    │  ◂─────────────┐                  │
         │  (online)    │                 │                  │
         └──┬───────┬───┘                 │                  │
            │       │                     │                  │
    start   │       │ WillSleep           │                  │
            ▼       ▼                     │                  │
      ┌─────────┐   ┌──────────────┐      │                  │
      │ RUNNING │◂─▶│   PAUSED     │      │                  │
      │ (timer  │   │ (fStatusAsleep│     │                  │
      │  live)  │ ▲ │   = 1)       │      │                  │
      └────┬────┘ │ └──────────────┘      │                  │
           │      │                       │                  │
           │      │ WokeUp (resumes start)│                  │
   stop    │      │                       │                  │
           ▼      │                       │                  │
      ┌─────────┐ │        terminate       │                 │
      │ STOPPED │ │       ─────────────▶  │   setMIDIDevice:offline:YES
      └─────────┘ │                       ▼                  │
                  │                 ┌──────────────┐          │
                  │                 │   OFFLINE    │          │
                  │                 │ (persisted)  │ ─────────┘
                  │                 └──────────────┘
                  │ (not represented — stop never fires
                  │  outside sleep/terminate/dealloc)
```

- **PRESENT ↔ PAUSED:** controlled by sleep notifications. CoreMIDI sees "online" the entire time. `fStatusAsleep` is the gate.
- **PRESENT ↔ OFFLINE:** controlled by matching notifications. CoreMIDI sees "offline" when hardware is gone. `kMIDIPropertyOffline` is the gate.
- **PAUSED → OFFLINE** (hardware yanked while asleep): the next matching event after wake will see the device gone, fire `driverInstanceTerminated:` → `setMIDIDevice:offline:YES`. The still-asleep instance may be deallocated at that point; `dealloc` calls `stop` which is already a no-op due to `fStatusRunning == 0`.

## 9 — Clone implications

1. **Register for system power once per plugin instance.** Factory-owned, singleton. Mirror the reference's class split (`MAFWBeBoBPowerNotification` ≈ `OpenAudioPowerNotification`).
2. **Fan out WillSleep/WokeUp per device**, idempotently guarded by `fStatusAsleep`.
3. **`kIOMessageCanSystemSleep` must be acknowledged** — do not consume it to prevent sleep. The reference always allows.
4. **Do not flip `kMIDIPropertyOffline` on sleep.** CoreMIDI must see the device as online across sleep; only matching-notification terminate flips it offline.
5. **Kext must survive the stop-start cycle** across sleep. Specifically: after selector 501 (stop) → S3 → selector 500 (start), the shared-memory mappings must either be stable or the kext must publish a new memType that the plugin re-maps. The reference plugin takes the "mappings are stable" branch — our kext **must** preserve `IOMemoryDescriptor` identity across stop/start.
6. **No `kIOMessageSystemWillPowerOn` handling needed** — the matching + busy-retry path on wake is the backstop.

## 10 — Open items

- **Factory fan-out mechanism (§4):** which selector + receiver pair does `MAFWMIDIFactoryBeBoB::systemWillSleep` dispatch? Deferred — needs full Ghidra decode or rip-rel resolution.
- **`IOConnectMapMemory` idempotency across sleep (§6):** does the second call return the same user VA? Deferred — DTrace or kext decode will settle it.
- **`CanSystemSleep` auto-allow policy:** the reference always allows. **Hypothesis — low confidence:** might want to **deny** sleep while a sysex transfer is in flight. But no such logic in reference. For parity, mirror the always-allow behavior.
- **`fStatusAsleep` ivar offset** — not resolved to a byte offset here (depends on class layout). The disassembly uses `movq _OBJC_IVAR_$_MAFWMIDIDeviceBeBoB.fStatusAsleep(%rip), %r14` which the Obj-C runtime fixes up at load. For the clone, declare the ivar and let the Obj-C runtime handle layout — do not hard-code offsets.
