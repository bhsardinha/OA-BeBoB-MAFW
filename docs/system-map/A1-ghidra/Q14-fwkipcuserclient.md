# Q14 — `FWKIPCUserClient` surface (Carbon panel back-channel)

**Status:** done.

**Scope:** `m_audio_b_FWKIPCUserClient` — the *second* UserClient on the kext, invisible to the Q11 CoreAudio-side selector table. Methods decompiled from Ghidra: `initWithTask`, `start`, `stop`, `registerNotificationPort`, `SendMachMessage`, `message`, `SendChangedPropertiesMessage`, `SerializeModifiedProperties`, `AddModifiedProperties`, `UserClientAdded`, `UserClientRemoved`, `clientClose`.

## Answer in one line

**`FWKIPCUserClient` is a Mach-port notification channel, not an RPC endpoint.** The kernel pushes serialized dirty-property OSDictionaries to the panel over Mach messages whenever anything in the kext's control model changes. The panel reads actual property values via standard `IORegistryEntryCreateCFProperty` on the parent FW1814Device. No `externalMethod` / selector-table surface — K-IPC is pure "poke the panel, come re-read."

## Lifecycle

```c
// 1. Panel opens IOServiceOpen() against the FW1814Device → kext creates
//    FWKIPCUserClient via its MetaClass::alloc.

bool FWKIPCUserClient::initWithTask(task_t owningTask, void *securityID, uint32_t type) {
    bzero(this + 0xe0, 0x2c);              // zero Mach message header body (44 bytes)
    this[+0x118] = 0;                      // dirty counter
    this[+0x110] = NULL;                   // modified-properties OSDictionary (lazy)
    this[+0x120] = NULL;                   // IORecursiveLock (lazy)
    this[+0x138] = owningTask;             // owning user-space task
    return IOUserClient::initWithTask(owningTask, securityID, type);
}

bool FWKIPCUserClient::start(IOService *provider) {
    if (!IOUserClient::start(provider)) return false;
    this[+0xd8] = provider;                // keep provider pointer
    UserClientAdded(this);                 // append to static sUserClients OSSet
    return true;
}

void FWKIPCUserClient::stop(IOService *provider) {
    this[+0xe8] = NULL;                    // drop notification port
    UserClientRemoved(this);               // remove from sUserClients
    if (this[+0x110]) { release OSDictionary; this[+0x110] = NULL; }
    if (this[+0x120]) { IORecursiveLockFree; this[+0x120] = NULL; }
    if (this[+0x128]) { release IOBufferMemoryDescriptor; this[+0x128] = NULL; }
    if (this[+0x130]) { release OSSerialize; this[+0x130] = NULL; }
    IOUserClient::stop(provider);
}

IOReturn FWKIPCUserClient::clientClose() {
    this[+0x138] = NULL;                   // just drops the task — stop() does the rest
    return 0;
}
```

## Notification-port registration — the only user-side setup

```c
IOReturn FWKIPCUserClient::registerNotificationPort(
        ipc_port_t port, uint32_t type, uint32_t refCon) {
    if (type != 0) {
        IOLog("M-FW: Someone registered for an unknown message type:%X\n", type);
        return kIOReturnBadArgument;       // 0xe00002c2
    }
    // Prime a Mach message header template:
    this[+0xe0] = 0x13;                    // msgh_bits = MACH_MSGH_BITS(COPY_SEND, 0)
    this[+0xe4] = 0x2c;                    // msgh_size = 44 bytes
    this[+0xe8] = port;                    // msgh_remote_port = user-provided
    this[+0xf0] = 0;                       // msgh_local_port
    this[+0xf8] = 0;                       // msgh_voucher_port
    this[+0xfc] = 0;                       // msgh_id
    this[+0x108] = this[+0x118];           // snapshot dirty counter
    this[+0x104] = refCon;                 // user-provided tag
    return 0;
}
```

- Only `type == 0` is accepted. The log string literal `"M-FW: Someone registered for an unknown message type:%X\n"` is the kext's user-facing identity.
- `msgh_bits = 0x13 = 0b10011` → MACH_MSGH_BITS(0x13 & 0x1F = 0x13 = MACH_MSG_TYPE_COPY_SEND, 0 local).
- `msgh_size = 0x2c = 44 bytes` → **the Mach message is exactly `sizeof(mach_msg_header_t)` (24) + 20 bytes of in-line payload** (header + 5 × uint32_t + 1 × uint64_t to be exact — see `SendChangedPropertiesMessage` below).

## Dirty-property collection — the kernel-side half

The kext's control model (level controls, selector controls, mode flags) writes into an OSDictionary and hands it off:

```c
void FWKIPCUserClient::AddModifiedProperties(OSDictionary *changed) {
    if (!this[+0x110]) {
        this[+0x110] = OSDictionary::withCapacity(changed->getCount());
        if (!this[+0x110]) return;
        this[+0x120] = IORecursiveLockAlloc();
        if (!this[+0x120]) return;
    }
    OSIncrementAtomic(this + 0x118);       // bump dirty counter
    IORecursiveLockLock(this[+0x120]);
    OSCollectionIterator *it = OSCollectionIterator::withCollection(changed);
    for (OSSymbol *key; (key = it->getNextObject()); ) {
        OSObject *val = changed->getObject(key);
        ((OSDictionary*)this[+0x110])->setObject(key, val);
    }
    IORecursiveLockUnlock(this[+0x120]);
    it->release();
}
```

The dict at `[+0x110]` is a cumulative "all properties changed since last flush." Keyed by property name. One instance per connected panel (every open panel accumulates its own dirty set).

## The flush — `SendChangedPropertiesMessage`

```c
IOReturn FWKIPCUserClient::SendChangedPropertiesMessage() {
    IOBufferMemoryDescriptor *bmd = SerializeModifiedProperties(this);
    if (!bmd) return kIOReturnError;
    uint8_t *dstBase = bmd->getBytesNoCopy();
    uint32_t serLen  = bmd->getLength();   // = serialized-XML length (not full BMD size)

    if (this[+0xe8]) {                     // notification port is live
        // Build the 44-byte Mach message inside the BMD header area:
        //   bytes  0..23 : mach_msg_header_t template from [+0xe0..+0xf7]
        //   bytes 24..31 : [+0xf8] (voucher) + [+0xfc] (id)       → 8 bytes
        //   bytes 32..39 : [+0x100] (last-known type/body)       → 8 bytes
        //   bytes 40..43 : [+0x108] (dirty-counter snapshot)      → 4 bytes
        // So the receiver's payload is {bodyType, dirtyGen, refCon} = 12 bytes.
        memcpy(dstBase, &this[+0xe0], 44);
        // Serialized XML is at dstBase + 0x2c (= 44) for serLen bytes.
        // (The receiver uses IOConnectMapMemory64 on the BMD to read the serialized dict.)
        return mach_msg_send_from_kernel_proper((mach_msg_header_t*)dstBase, 44)
                 ? kIOReturnError : 0;
    }
    return kIOReturnError;
}
```

## IOService::message override — opportunistic kick

```c
IOReturn FWKIPCUserClient::message(uint32_t type, IOService *provider, void *arg) {
    if (this[+0xe8]) {
        this[+0x100] = type;               // stash body-type field
        mach_msg_send_from_kernel_proper((mach_msg_header_t*)&this[+0xe0], 44);
    }
    return IOUserClient::message(type, provider, arg);
}
```

- On *every* `IOService::message` the KIPC client receives (bus resets, firmware events, child-device changes), it fires a Mach message. Panel wakes up and re-reads.
- This is the generic "something changed" ping — distinct from the property-specific `SendChangedPropertiesMessage`.

## Broadcast to all panels

```c
static OSSet *sUserClients;    // kext-global, one entry per live FWKIPCUserClient

static void FWKIPCUserClient::UserClientAdded(FWKIPCUserClient *uc) {
    if (!sUserClients) sUserClients = OSSet::withCapacity(4);
    if (sUserClients) sUserClients->setObject(uc);
}
static void FWKIPCUserClient::UserClientRemoved(FWKIPCUserClient *uc) {
    if (sUserClients) {
        sUserClients->removeObject(uc);
        if (sUserClients->getCount() == 0) {
            sUserClients->release();
            sUserClients = NULL;
        }
    }
}
```

Any kext code wanting to broadcast property changes iterates `sUserClients` and calls `AddModifiedProperties` + `SendChangedPropertiesMessage` on each. Multiple panels (unusual, but supported) all get notified.

## Field map

| Offset | Field | Type |
|---|---|---|
| `+0xd8` | `fProvider` | `IOService *` (the FW1814Device) |
| `+0xe0..+0xe4` | `msgh_bits` + `msgh_size` | `mach_msg_header_t` prefix (`0x13`, `44`) |
| `+0xe8` | `msgh_remote_port` (the panel's receive port) | `ipc_port_t` |
| `+0xf0..+0xff` | `msgh_local_port` + `msgh_voucher_port` + `msgh_id` | zero-initialized |
| `+0x100` | last `::message` type | `uint64_t` (used as body field in Mach msg) |
| `+0x104` | panel-provided `refCon` | `uint32_t` |
| `+0x108` | dirty-generation snapshot at `registerNotificationPort` time | `uint32_t` |
| `+0x110` | `fModifiedProperties` (lazy) | `OSDictionary *` |
| `+0x118` | atomic dirty counter | `uint32_t` |
| `+0x120` | lock protecting the dict | `IORecursiveLock *` |
| `+0x128` | serialized output buffer | `IOBufferMemoryDescriptor *` |
| `+0x130` | serializer | `OSSerialize *` |
| `+0x138` | owning task | `task_t` |

Class size: at least `0x140` bytes (plus IOUserClient base).

## Error codes

- `kIOReturnBadArgument = 0xe00002c2` — unknown notification type.
- `kIOReturnError = 0xe00002bc` — Mach send failed.

## What the panel does (reconstructed from the kernel half)

1. `IOServiceOpen` against the FW1814Device → gets a `FWKIPCUserClient` connection handle.
2. Allocate a Mach receive port, call `IOConnectSetNotificationPort(conn, type=0, port, refCon)` → the kext stashes it at `[+0xe8]`.
3. Register a CFMachPort source on the run loop. Each Mach message is:
   - `mach_msg_header_t` (24 bytes) with the panel's port in `msgh_remote_port` (actually destination from kernel POV).
   - 8 bytes voucher/id.
   - 8 bytes body-type (what triggered the ping — a `IOService::message` constant).
   - 4 bytes dirty-generation count.
4. On message receipt:
   - If body-type is a bus-reset / firmware event, re-read all properties.
   - Otherwise, call `IOConnectMapMemory64(conn, type=0)` to map the IOBMD at `[+0x128]`, parse the serialized OSDictionary at offset 44, apply the diff.
5. Panel writes back by calling standard IOKit property setters on the parent (e.g. `IORegistryEntrySetCFProperties`), which hit the kext's `IOService::setProperties` override.

**No RPC methods, no selector tables.** The whole control surface is just "panel writes properties, kernel notices and acts; kernel changes properties, panel is poked via Mach."

## Clone implications

### For a Carbon-panel-compatible clone
Mirror this exactly. The 2014 panel binary expects:
- `IOConnectSetNotificationPort(type=0, …)` + `IOConnectMapMemory64(type=0, …)` on the same UC class.
- A Mach message of exactly 44 bytes with the 3-field body.
- Serialized OSDictionary at BMD offset 44.
- `setProperties`-based reverse channel on the FW1814Device.

### For a from-scratch AppKit panel (our actual plan)
This UC is **optional** — we can skip `FWKIPCUserClient` entirely and build:
- A simple IOUserClient with `externalMethod` for property gets/sets (less code than the dirty-dict machinery).
- An async callback via `IOAsyncCallback` or XPC rather than Mach ports.

**Recommendation: skip `FWKIPCUserClient` in the clone.** It exists to serve the Carbon panel, and we're dropping Carbon anyway. The 2014 reference's own control model (OSDictionary properties on the FW1814Device, settable via `setProperties`) is enough for AppKit.

### What *is* worth copying

- **The property-dict model.** Our clone should expose controls as OSDictionary/OSNumber in the registry — not through `externalMethod`. Panel reads via `IORegistryEntryCreateCFProperty`, writes via `IORegistryEntrySetCFProperties`. This matches `m_audio_b_FWPropertyDict` + `FWBaseProp` + `FWChannelGroupProperty` — the typed-property framework surfaced in the overview.
- **The `sUserClients` broadcast pattern** — if we ever support multiple panel instances (headless agent + GUI), we need the same OSSet-based tracking.

## Follow-ups

- **Q14a:** confirm the panel's actual Mach message handling by disassembling `_extracted/panel/M-Audio FireWire.app/Contents/MacOS/M-Audio FireWire`. Not blocking for the clone.
- **Q14b:** does the FW1814Device override `setProperties` to apply control-dict writes? Grep `functions.tsv` for `com_m_audio_FW1814Device::setProperties` or `m_audio_b_FWAudioDevice::setProperties`.
- Observe on SL reference Mac: `log stream --predicate 'eventMessage CONTAINS "M-FW: Someone registered"'` while opening the panel to confirm the log line fires.

## Provenance

- All function offsets: `ghidra-exports/functions.tsv` (2026-10-06).
- Decompiled bodies: Ghidra 12.1.4, output captured in `docs/system-map/A1-ghidra/ghidra-exports/decomp/q14.c` (regenerable via `scripts/DecompQ14.java`).
- Mach message size `0x2c = 44` and bits `0x13`: immediates in `registerNotificationPort` @ `0x15f3a`.
- Log string `"M-FW: Someone registered for an unknown message type:%X\n"`: literal in same function.
- `sUserClients` static OSSet: referenced by `UserClientAdded` @ `0x1620c` and `UserClientRemoved` @ `0x15bea`.
- IOUserClient vtable offsets (`0x5d0` start, `0x5d8` stop, `0x740` message, `0x8f8` initWithTask): standard IOKit on 10.6-10.13.
