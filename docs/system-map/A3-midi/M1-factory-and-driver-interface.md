# A3-M1 — CoreMIDI factory + driver interface + glue bridge

> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Verified against the x86_64 slice (factory UUID, `new(0x20)` + 8, ctor layout, QueryInterface UUID and `[+0x14]=2`). Refcount at `+0x18` is an 8-byte store.


**Scope:** The call path from CFPlugIn registration (`_NewFW410MIDIDriver`) through the CFPlugIn COM shim, the C++ `MIDIDriver` base, the `MAFWMIDIPluginGlueBeBoB` override, and the `_MAFireWireMIDIDICE_*` dispatch, down to allocation of the `MAFWMIDIFactoryBeBoB` singleton and its IOKit observer installation.

**Decompile logs:** `ghidra-exports/decomp/a3-m1.log` (550 lines), `ghidra-exports/decomp/a3-m1b.log` (DICE thunks).
**Scripts:** `scripts/DecompA3M1.java`, `scripts/DecompA3M1b.java`.

## 1. Entry: `_NewFW410MIDIDriver` @ 0x1be6 (`a3-m1.log:1-20`)

```c
MAFWMIDIPluginGlueBeBoB *_NewFW410MIDIDriver(allocator, requestedUUID) {
    uuid = CFUUIDGetConstantUUIDWithBytes(0, ECDE9574-0FE4-11D4-BB1A-0050E4CEA526); // kMIDIDriverTypeID
    if (!CFEqual(requestedUUID, uuid)) return NULL;
    obj = operator new(0x20);
    MAFWMIDIPluginGlueBeBoB::MAFWMIDIPluginGlueBeBoB(obj);
    return obj + 8;   // offset to embedded MIDIDriverInterface** (ftbl)
}
```

Classic CFPlugIn-COM entry: gated on `kMIDIDriverTypeID`, allocates `0x20` (32-byte) object, returns pointer offset by `+8` so CFPlugIn can treat it as `MIDIDriverInterface**`. The outer object pointer is recoverable as `ftbl - 8` — which is how every `MIDIDriver*` C stub (below) walks back.

**Factory UUID (registered in `Info.plist`):** `29F28FC6-B0F8-11D7-B850-000393942016`.

## 2. CFPlugIn COM shim: `MIDIDriverQueryInterface` @ 0x19f3 (`a3-m1.log:24-67`)

```c
HRESULT QueryInterface(this, uuidBytes, outIface) {
    reqUUID = CFUUIDCreateFromUUIDBytes(uuidBytes);
    // Branch A: kMIDIDriverInterface2ID = 43C98C3C-306C-11D5-AF73-003065A8301E
    if (CFEqual(reqUUID, kMIDIDriverInterface2ID)) {
        this->Retain();                      // vtable slot 0x10
        *outIface = this;
        CFRelease(reqUUID);
        *(uint32_t*)(this + 0x14) = 2;       // record interface version
        return 0;
    }
    // Branch B: kIUnknownUUID = 00000000-0000-0000-C000-000000000046
    if (CFEqual(reqUUID, IUnknownUUID)) {
        this->Retain();
        *outIface = this;
        CFRelease(reqUUID);
        return 0;
    }
    *outIface = NULL;
    CFRelease(reqUUID);
    return E_NOINTERFACE;                    // 0x80000004
}
```

**Finding:** the driver advertises `kMIDIDriverInterface2ID` (the modern CoreMIDI driver iface, `MIDIDriver.h`), not `kMIDIDriverInterface1ID`. Writes `2` to `this[0x14]` so later dispatch can version-switch. Also handles bare `IUnknown` for completeness.

Reference count via `MIDIDriverAddRef` / `MIDIDriverRelease` (`a3-m1.log:71-102`) — standard IUnknown over `this[0x10]`, with `this - 8` yielding the C++ object whose vtable slot `[8]` is the C++ destructor called on refcount → 0.

## 3. Object layout — `MIDIDriver` base ctor @ 0x194a (`a3-m1.log:231-240`)

```c
void MIDIDriver::MIDIDriver(this, factoryUUID) {
    this[0x00] = &PTR__MIDIDriver_0000aa20;      // C++ vtable (overridden by subclass)
    this[0x08] = &MIDIDriverInterfaceFtbl;       // CFPlugIn C function table — returned to CoreMIDI
    this[0x10] = factoryUUID;
    CFPlugInAddInstanceForFactory(factoryUUID);
    this[0x18] = 1;                              // refcount
}
```

So the full 32-byte object is:

| offset | field | note |
|---|---|---|
| `0x00` | C++ vtable ptr | `MIDIDriver` (base) → `MAFWMIDIPluginGlueBeBoB` (subclass overrides it in its ctor) |
| `0x08` | `MIDIDriverInterface**` ftbl | the pointer CoreMIDI sees after the `+8` offset in `_NewFW410MIDIDriver` |
| `0x10` | factoryUUID | kept for `CFPlugInRemoveInstanceForFactory` in dtor |
| `0x14` | iface version | written by QueryInterface (2 for `kMIDIDriverInterface2ID`) |
| `0x18` | refcount | u32 initialized to 1 |
| `0x1c..0x1f` | pad | — |

## 4. C stubs → C++ virtuals (`a3-m1.log:106-223`)

Each `MIDIDriverFooBar` C stub does:

```c
void MIDIDriverFoo(MIDIDriverInterface **ftbl, args...) {
    MIDIDriver *obj = (MIDIDriver *)(ftbl - 1);   // walks back one slot from +8 to +0 (object start)
    (**(code **)(*(long*)obj + <slot>))(obj);
}
```

Vtable slot mapping (confirmed from decomp — `a3-m1.log:106-223`):

| C stub | C++ vtable slot | signature |
|---|---|---|
| `MIDIDriverFindDevices` | `0x10` | `FindDevices(uint)` |
| `MIDIDriverStart` | `0x18` | `Start(uint)` |
| `MIDIDriverStop` | `0x20` | `Stop()` |
| `MIDIDriverConfigure` | `0x28` | `Configure(uint)` |
| `MIDIDriverSend` | `0x30` | `Send(MIDIPacketList*, void*, void*)` |
| `MIDIDriverEnableSource` | `0x38` | `EnableSource(uint, uchar)` |
| `MIDIDriverFlush` | `0x40` | `Flush(uint, void*, void*)` |
| `MIDIDriverMonitor` | `0x48` | `Monitor(uint, MIDIPacketList*)` |

Base `MIDIDriver::{Start,Stop,Configure,Send,EnableSource,Flush,Monitor}` all `return 0` — the base class defines everything as no-ops; the subclass overrides.

Note the `WARNING: Could not recover jumptable` in ghidra on each stub — these are plain indirect calls, not jumptables; the decompiler misreads them. The ASM is still a straight `jmpq *offset(%rdi)` tailcall.

## 5. Subclass: `MAFWMIDIPluginGlueBeBoB` (`a3-m1.log:345-468`)

- **ctor** @ `0x1cb0`: calls `MIDIDriver::MIDIDriver(this, <factory UUID 29F28FC6-...>)`, then overrides `this[0]` with the subclass vtable `PTR__MAFWMIDIPluginGlueBeBoB_0000aa80`.
- **Subclass vtable slots** (from the decomp — only the overrides matter):

| slot | override | behavior |
|---|---|---|
| `0x10` FindDevices | `return 0` (no-op) | real discovery happens via IOKit observer |
| `0x18` Start | `_MAFireWireMIDIDICE_Start(this + 8)` | **lazily instantiates the factory singleton** |
| `0x20` Stop | `_MAFireWireMIDIDICE_Stop()` | terminates devices + releases singleton |
| `0x30` Send | `_MAFireWireMIDIDICE_Send(packetList, dest, refCon)` | per-send dispatch |

**Finding — refutes M10 hypothesis in README:** the `_MAFireWireMIDIDICE_*` exports are NOT dead code. They are the live BeBoB dispatch path. The name "DICE" is a historical misnomer — the SVN tree evidently shared Start/Stop/Send scaffolding between DICE and BeBoB families and the symbols kept the DICE prefix. M10 will be closed as "name misleading; path is live". **Correction landed in README on 2026-10-06.**

## 6. Factory singleton birth: `_MAFireWireMIDIDICE_Start` @ 0x1db7 (`a3-m1b.log:1-16`)

```c
undefined8 _MAFireWireMIDIDICE_Start(MIDIDriverInterface **ftbl) {
    pool = [[NSAutoreleasePool alloc] init];
    sMIDILoader = [[MAFWMIDIFactoryBeBoB alloc] initWithDriver:ftbl];
    [pool release];
    return 0;
}
```

- `sMIDILoader` is a **file-scope global** (data section at `PTR_sMIDILoader_*`), not an ivar on the glue.
- The glue object itself is almost stateless — it is a shim that owns the CFPlugIn ABI; all state lives on the factory singleton.

`_MAFireWireMIDIDICE_Stop` @ 0x1e35 (`a3-m1b.log:20-33`):

```c
undefined8 _MAFireWireMIDIDICE_Stop() {
    pool = [[NSAutoreleasePool alloc] init];
    [MAFWMIDIDeviceBeBoB terminateAllDevices];    // class method
    [sMIDILoader release];
    sMIDILoader = nil;
    [pool release];
    return 0;
}
```

`_MAFireWireMIDIDICE_Send` @ 0x1ead (`a3-m1b.log:37-55`):

```c
int _MAFireWireMIDIDICE_Send(MIDIPacketList *list, id endpoint, void *refCon) {
    pool = [[NSAutoreleasePool alloc] init];
    if (!endpoint) {
        NSLog("Bogus... no such endpoint %lX!", 0);
        rv = 0xFFFFD5AE;    // == -10834 (kMIDIUnknownEndpoint)
    } else {
        rv = [endpoint send:list viaPort:refCon];
    }
    [pool release];
    return rv;
}
```

**Finding — endpoint-centric send dispatch:** CoreMIDI passes the `MIDIEndpoint` ref as the `connRefCon` on each send; the glue trusts it as an Obj-C object and calls `-[endpoint send:viaPort:]`. The actual endpoint class (likely `MAFWMIDIDestinationBeBoB` or a port wrapper — **unverified; decode in M5**) implements the real wire-send.

**Note on arg mapping:** The decomp shows `_MAFireWireMIDIDICE_Send(param_1, param_2, param_3)` with `param_2` being the endpoint object. In CoreMIDI's `MIDIDriverInterface2::Send(ftbl, packetList, destRefCon1, destRefCon2)` the per-destination refcons are set up when the driver creates the endpoint. The glue has already discarded `destRefCon1` and only forwards two of the three — **unverified mapping; confirm in M5 by decoding how the factory wires `MIDIDestinationCreate` and what it passes as refCon**.

## 7. Factory `initWithDriver:` @ 0x337b (`a3-m1.log:472-525`)

```c
ID MAFWMIDIFactoryBeBoB::initWithDriver_(id self, SEL _cmd, MIDIDriverInterface **drv) {
    const char *svcClasses[] = {
        "com_m_audio_FW410Device",
        "com_m_audio_FWAudiophileDevice",
        "com_m_audio_FW1814Device",         // <-- FW1814 is in-scope
        "com_m_audio_FWOzonicDevice",
        "com_m_audio_FWProjectMixDevice",
        "com_m_audio_FWLightbridgeDevice",
        NULL
    };
    self = [super init];
    self->fMIDIDriver = drv;                  // ivar at MAFWMIDIFactoryBeBoB.fMIDIDriver
    [self setAllDevicesToOffline];
    for (int i = 0; svcClasses[i] != NULL; i++) {
        CFMutableDictionaryRef matching = IOServiceMatching(svcClasses[i]);
        id autoMatching = [(id)matching autorelease];
        [MAFWBeBoBServiceNotification observeServiceMatched:autoMatching
                                                      target:self
                                                    selector:@selector(driverInstanceMatched:)];
        [MAFWBeBoBServiceNotification observeServiceTerminated:autoMatching
                                                         target:self
                                                       selector:@selector(driverInstanceTerminated:)];
    }
    return self;
}
```

**Findings:**
- **Family coverage:** the plugin is the single CoreMIDI driver for every M-Audio FireWire BeBoB device — FW410, FWAudiophile (Audiophile FireWire), **FW1814**, FWOzonic (Ozonic), FWProjectMix (ProjectMix I/O), FWLightbridge (ProFire Lightbridge). All six get both match and terminate observers.
- **No FindDevices work needed:** on cold start, `[self setAllDevicesToOffline]` marks all pre-registered CoreMIDI devices offline, then as IOKit matches fire, `driverInstanceMatched:` brings them online. On shutdown, `driverInstanceTerminated:` fires per device.
- **ivar `fMIDIDriver`** = the CFPlugIn `MIDIDriverInterface**` ftbl — the factory keeps it so each per-device `MAFWMIDIDeviceBeBoB::instantiateDriver:withService:withMIDIDevice:` can pass it along.

`dealloc` @ 0x34d3 (`a3-m1.log:533-547`): stops both `MAFWBeBoBServiceNotification` and `MAFWBeBoBPowerNotification` (sleep/wake) observers before `[super dealloc]`.

## 8. Call-graph summary

```
CoreMIDI (MIDIServer process)
   │
   ▼
_NewFW410MIDIDriver                       ← CFPlugIn factory, Info.plist UUID 29F28FC6-...
   │   checks kMIDIDriverTypeID
   │   new MAFWMIDIPluginGlueBeBoB
   │   returns ftbl ptr (object + 8)
   ▼
MIDIDriverQueryInterface(kMIDIDriverInterface2ID) → object retained, this[0x14]=2
   │
MIDIDriverStart ─────────► glue::Start ─► _MAFireWireMIDIDICE_Start(ftbl)
                                              │
                                              ▼
                                     sMIDILoader = [MAFWMIDIFactoryBeBoB
                                                       alloc] initWithDriver:ftbl
                                              │
                                              ▼
                              per-service observers installed:
                              • com_m_audio_FW410Device
                              • com_m_audio_FWAudiophileDevice
                              • com_m_audio_FW1814Device  ◄── us
                              • com_m_audio_FWOzonicDevice
                              • com_m_audio_FWProjectMixDevice
                              • com_m_audio_FWLightbridgeDevice
                                              │
                              IOKit → driverInstanceMatched: → [M2 topic]
                                              │
MIDIDriverSend ─────────► glue::Send ─► _MAFireWireMIDIDICE_Send(list, endpoint, refCon)
                                              │
                                              ▼
                                     [endpoint send:list viaPort:refCon]   ← M5 topic

MIDIDriverStop ─────────► glue::Stop ─► _MAFireWireMIDIDICE_Stop()
                                              │
                                              ▼
                              [MAFWMIDIDeviceBeBoB terminateAllDevices];
                              [sMIDILoader release]; sMIDILoader = nil;
```

**Not handled (base `return 0` or no override):**
- `FindDevices` — intentional no-op; discovery is observer-driven.
- `Configure` — never overridden; configuration happens via per-device Obj-C selectors, not CoreMIDI's generic hook.
- `EnableSource` — never overridden; source enable/disable is tracked by the kext user-client, not CoreMIDI (**hypothesis — confirm in M6/M7**).
- `Flush` — never overridden; **hypothesis** — on hardware termination there is nothing to flush because packets are posted synchronously via `-[endpoint send:viaPort:]` and no queue is held in the plugin. **Verify by decoding `-send:viaPort:` in M5.**
- `Monitor` — never overridden; MMC/MTC monitoring not supported.

## 9. Clone implications

For a from-scratch AppKit panel + userspace MIDI plugin (per `NEW_PLAN.md` Part D):

1. **Keep the CFPlugIn + `MIDIDriverInterface2ID` shape as-is.** CoreMIDI is a frozen ABI; this layout is the only shape that works. Our bundle Info.plist mirrors the one at `_extracted/midi/MAudioFireWireMIDI.plugin/Contents/Info.plist` with a new factory UUID.

2. **Drop the DICE prefix.** We only ship FW1814, so our factory singleton can be plain `OpenAudioFW1814MIDIFactory`. The DICE name was a historical artifact.

3. **Shrink the matching list to only `com_openaudio_FW1814Device`** (our kext IOService class, per A1-Q11 / CLAUDE.md). The multi-family loop in `initWithDriver:` becomes a single `IOServiceMatching` call.

4. **Keep the observer-driven discovery pattern** (match + terminate selectors on each IOService arrival). It handles hotplug correctly and matches the panel's A2-P1 pattern.

5. **FindDevices stays no-op** — same reason as the reference: CoreMIDI calls it once, but the observer path is authoritative.

6. **Interface version:** advertise `kMIDIDriverInterface2ID` (not v1). Modern CoreMIDI expects it.

7. **No MIDIDriver base class needed in a modern port** — use CoreMIDI's current driver sample (`MIDIDriverRef`) scaffolding. The 2014 `MIDIDriver` base in `_extracted/midi/` is a MIDIDriverKit-era C++ pattern that CoreMIDI docs still support but the Obj-C factory pattern is cleaner.

## 10. Status

**Closes M1.** The CoreMIDI entry path is fully mapped: CFPlugIn UUID gating → object construction → QueryInterface IUnknown dance → C stub → C++ vtable → glue thunk → factory singleton allocation → IOKit observer install for six BeBoB device classes (FW1814 included).

**Opens / updates:**
- **M10 (DICE exports hypothesis) — refuted.** The DICE-prefixed exports are the live BeBoB path, not dead code. Will fold M10 into README with a correction note.
- **M5 (TX path):** inherit the question "how does `-[endpoint send:viaPort:]` reach the kext?" — must decode the destination endpoint class.
- **M6 (RX path), M7 (endpoint model):** both unblocked by `driverInstanceMatched:` (M2 topic) which creates the endpoints.
- **Unverified claims to retire in M5/M6:** (a) `Flush`/`EnableSource` are not needed because packets are synchronous (§8); (b) `destRefCon1` vs `destRefCon2` arg mapping on Send (§6).
