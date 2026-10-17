# A2-P1 — Device discovery + user-client open path

> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Type-1 `IOConnectMapMemory` (gated by the ctor's `createSharedMem` bool) maps the **4 KiB peak-meter page** (`FWAudioDevice[+0x100]`), not a MIDI ring. The ctor also seeds its cache from `MAMetaAllProperties` and reads 80 bytes of `MAFirmwareInfo`. Notification type strings are `IOServiceMatched`/`IOServiceTerminate` (no `IOServicePublish`); `CDeviceList::AddDevice` takes an io_service_t.


**Scope:** How the Carbon control panel finds the kext, opens a user client, maps shared memory, and hands the device object up to the UI framework.

**Decompile log:** `ghidra-exports/decomp/a2-batch1.log` (functions `CDeviceList::*`, `MAServiceNotification`, `ServiceDispatchCallback`, `CXDevice::CXDevice`, `CMiddleWareManager::AddDevice`, `CMiddleWareDevice::CMiddleWareDevice`).

**Panel source path (from strings):** `/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/` — this is M-Audio FireWire panel **v1.11.002**.

## Call chain

```
CControlPanelApp (app init)
  └─ CDeviceList::MakeDeviceList           @ 000648e0
       └─ CDeviceList::InitDeviceList      @ 00064c28
            └─ -[MAServiceNotification observe:dictionary:target:selector:]
                 └─ -[MAServiceNotification initWithDictionary:port:type:target:selector:]   @ 0008108d
                      └─ IOServiceAddMatchingNotification(port, "IOServicePublish", dict,
                                                          ServiceDispatchCallback, self)
                           ↓ (fires once per matching service, async on main run loop)
                      ServiceDispatchCallback(refCon, iterator)   @ 000811aa
                           ├─ IOIteratorNext → io_object
                           ├─ [[MAIOKitService alloc] initWithService:io_object]
                           └─ [target selector:maIOKitService]        ← CDeviceList::AddDevice
                                └─ CDeviceList::AddDevice           @ 00064d64
                                     ├─ CXDevice::FetchDeviceForService  @ 0006c23e   (dup-check)
                                     ├─ CXDevice::CXDevice(mas, true)   @ 0006c2b8
                                     └─ CMiddleWareManager::AddDevice   @ 00063df2
                                          └─ CMiddleWareDevice::CMiddleWareDevice  @ 000635f2
```

Two separate matching dictionaries are registered (one per device family — FireWire 1814 vs. FireWire 410/Solo/etc.), both pointing at the same `AddDevice` selector but with different `type:` tags so the panel can dispatch to the correct `CFW<N>DeviceSettings` subclass later.

## `CXDevice::CXDevice(MAIOKitService *mas, bool mapMIDI)` — @ 0006c2b8

The constructor is the panel's equivalent of our kext's `start()` — it is where the user client is opened and the kernel-side MAServiceClient object comes to life.

Observed sequence (decompile paraphrased):

1. Zero `this`, retain `mas` into `this[+0x40]`.
2. `[mas openConnection]` — wraps `IOServiceOpen(service, mach_task_self, kUserClient_FWKIPC, &conn)` and stashes `conn` inside `MAIOKitService`.
3. Allocate three Foundation containers:
   - `this[+0x6c]` = `[[NSMutableDictionary alloc] init]`  → **local property cache** (mirrors every value we ever wrote so queued writes can re-coalesce without a kernel round-trip).
   - `this[+0x70]` = `[[NSMutableSet alloc] init]`         → **dirty-key set** for queued writes.
   - `this[+0x74]` = `[[NSMutableDictionary alloc] init]`  → **NSString-key → C function pointer** change-handler table (populated by `InstallPropertyChangeHandlers`, see P3).
4. If `mapMIDI` is true (which is always true for FW1814):
   ```
   IOConnectMapMemory(conn,
                      1,                 /* kMAMemoryType_MIDI */
                      mach_task_self(),
                      &this[+0x68],      /* mapped vaddr */
                      &len,
                      0x1001)            /* kIOMapAnywhere | kIOMapDefaultCache */
   ```
   This **closes A1-Q11** from the receive side: the type-1 shared buffer is the MIDI packet ring the kext publishes at `[FWAudioDevice+0x100]`. The panel maps it so the UI can read incoming MIDI without an RPC round-trip (used by the MIDI-activity LEDs on the hardware tab).
5. Read `MAFirmwareInfo` IORegistry property (80 bytes, same payload we saw in A1-Q7a) and copy it into `this[+0x88]` — this is where the "Firmware: 1.11.002" label on the About tab is read from.
6. Spawn `pthread_create(TerminationWaitThread, this)` — a background thread that blocks on `IOServiceWaitQuiet` + the termination notification iterator and tears the device down when the kext goes away (hotplug).
7. `CXDevice::RegisterForNotifications` (see P2).
8. `CXDevice::InstallPropertyChangeHandlers` (see P3).

## Clone implication for OpenAudio user client

Our user client **must** support:

- **Type-1 memory mapping** (`clientMemoryForType(1)`) — the MIDI shared ring. Panel assumes it; our MIDI plugin will assume it. Kernel-side plan is already captured in A1-Q11 (buffer lives at `FWAudioDevice+0x100`).
- **`IOServiceAddMatchingNotification` compatible registration** — this is just normal IOKit, but it means our matching dict must publish both `kIOServicePublish` *and* `kIOServiceTerminate` so the panel's `TerminationWaitThread` fires correctly on hotplug.
- **`MAFirmwareInfo` 80-byte IORegistry property** — the panel reads it at ctor time; absence will crash the About tab. A1-Q7a confirmed the kext builds this from CSR `0xFFFF:0xC8020000`.
- **No externalMethod dispatch is used during discovery.** All device state transfer happens via IORegistry properties (confirmed in P3). Our user client does not need an `externalMethod` surface *for the panel* — but the MIDI plugin may still need it; A3 will confirm.

## Cross-references

- Closes A1-Q14 from the receive side (notification wiring in P2).
- Closes A1-Q11 from the receive side (MIDI shared memory map).
- Confirms A1-Q7a (`MAFirmwareInfo` property shape).
- Panel source lineage: `FireWireBeBoB_01.11.002`.
