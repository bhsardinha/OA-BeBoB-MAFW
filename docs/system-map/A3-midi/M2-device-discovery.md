# A3-M2 — device discovery + user-client open

> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Constants 0xC35/0xFFFF/0xFFFF/0x3A98 verified in `createDeviceProperties`. **§8.2 is wrong:** SL v1.9 ioreg shows `MAMIDIInputPorts = ("1")` and `MAMIDIOutputPorts = ("1")` — one in, one out, each named `1` (`captures/12-ioreg-full.txt:3941,3951`). `MAIconName = "FW 1814"`, `IOAudioDeviceName = "FireWire 1814"`.


**Scope:** From the IOKit "service matched" notification installed in M1-§7 down to a live `MAFWMIDIDeviceBeBoB` instance with its CoreMIDI `MIDIDeviceRef` registered and its kext user client opened.

**Decompile logs:** `ghidra-exports/decomp/a3-m2.log` (453 lines), `ghidra-exports/decomp/a3-m2b.log` (device-side init).
**Scripts:** `scripts/DecompA3M2.java`, `scripts/DecompA3M2b.java`, `scripts/DumpStrings.java`.

## 1. `driverInstanceMatched:` @ 0x38b6 (`a3-m2.log:1-119`)

```c
void MAFWMIDIFactoryBeBoB::driverInstanceMatched_(id self, SEL _cmd, io_service_t svc) {
    svcWrapper = [MAFWBeBoBIOKitService deviceWithService:svc];   // wraps svc in Obj-C
    midiDev    = [self fetchMIDIDeviceForService:svcWrapper];     // scans existing CoreMIDI devs by GUID

    if (midiDev == 0) {                                           // first-time registration
        name   = [svcWrapper propertyForKey:@"IOAudioDeviceName"];
        mfr    = [svcWrapper propertyForKey:@"IOAudioDeviceManufacturerName"];
        inArr  = [svcWrapper propertyForKey:@"MAMIDIInputPorts"];   // NSArray<NSString*>
        outArr = [svcWrapper propertyForKey:@"MAMIDIOutputPorts"];  // NSArray<NSString*>

        if (MIDIDeviceCreate(self->fMIDIDriver, name, mfr, name, &midiDev)) return;
        //   signature: MIDIDeviceCreate(owner, name, manufacturer, model, outDevice)
        //   note: `model` is passed the same NSString as `name` (IVar7 reused — a3-m2.log:46)

        guid    = [[svcWrapper propertyForKey:@"GUID"] unsignedLongLongValue];
        guidStr = [NSString stringWithFormat:@"%llX", guid];
        if (MIDIObjectSetStringProperty(midiDev, @"com.m-audio.FW.GUID", guidStr)) return;

        // One MIDIEntity per device; sources = inArr.count, destinations = outArr.count
        if (MIDIDeviceAddEntity(midiDev, name, 0, inArr.count, outArr.count, &entity)) return;

        for (i in 0..entity.numSources - 1)       // skips the broadcast/anyone source (iterator starts at 1)
            MIDIObjectSetStringProperty(source[i], kMIDIPropertyName, inArr[i-1]);
        for (i in 0..entity.numDestinations - 1)
            MIDIObjectSetStringProperty(dest[i],   kMIDIPropertyName, outArr[i-1]);

        if (MIDISetupAddDevice(midiDev)) return;
        [self createDeviceProperties:midiDev withService:svcWrapper];
    }

    instance = [MAFWMIDIDeviceBeBoB fetchInstanceForDevice:midiDev];   // scans sDrivers set
    if (instance == nil)
        instance = [MAFWMIDIDeviceBeBoB instantiateDriver:self->fMIDIDriver
                                              withService:svcWrapper
                                           withMIDIDevice:midiDev];

    [self setMIDIDevice:midiDev offline:(instance == nil)];
}
```

**Findings:**

- **Kext → plugin contract keys** (CFString constants, resolved via `DumpStrings.java`):
  - `GUID` — passed through `unsignedLongLongValue`, serialized as `%llX`. Set by the kext on the `IOService` registry entry. Used as the stable identity between runs so CoreMIDI's `MIDISetup` can re-attach a previously-created `MIDIDevice`.
  - `IOAudioDeviceName` — human-readable name (e.g. "FireWire 1814"). Reused as both `name` and `model` in `MIDIDeviceCreate`.
  - `IOAudioDeviceManufacturerName` — manufacturer string (e.g. "M-Audio").
  - `MAMIDIInputPorts` — **NSArray of NSStrings**, one per source endpoint name. `inArr.count` = number of sources on the single MIDIEntity.
  - `MAMIDIOutputPorts` — same shape for destinations.
  - `MAIconName` — plain NSString (not an array), filename to search in `/Library/Application Support/M-Audio/DeviceIcons/`.

- **One MIDIEntity per device.** The whole FW1814 is modelled as a single entity with N sources + M dests; the plugin never calls `MIDIDeviceAddEntity` more than once per device. This matches the Mac convention for aggregated USB/FW audio+MIDI gear.

- **Persistence via GUID.** `fetchMIDIDeviceForService:` (below) looks up the already-known `MIDIDevice` across restarts by matching `com.m-audio.FW.GUID` to the current service's `GUID`. If found, reuse (keep the user's MIDI port routings in GarageBand/Logic/etc.); if not, create.

- **Note on iterator start.** The loops use `TIterator<...>::Next` starting at index `1` (iterator state `local_50 = 0` / `local_70 = 0`, pre-incremented in `Next`). The condition `(1 < count)` plus `local_50 - 1` on `objectAtIndex:` means index 0 of the NSArray is paired with iterator state 1 — i.e. CoreMIDI entity's first source/dest (which has index 0 in its parent entity's array) gets the array's element 0. **Hypothesis — the start-at-1 pattern is a `TIterator` quirk that bias-corrects to zero on first `Next`. Verify by decoding `TIterator::Next` directly if cookie ordering becomes a question.**

- **Offline-flag handshake.** `setMIDIDevice:offline:` writes `kMIDIPropertyOffline = 0/1` on the device. The initial state (from `setAllDevicesToOffline` in `initWithDriver:`) is online→offline for every pre-existing CoreMIDI device; as `driverInstanceMatched:` succeeds, the device goes back online. On `driverInstanceTerminated:` it goes back offline (device stays visible to clients but marked offline).

## 2. `fetchMIDIDeviceForService:` @ 0x35ff (`a3-m2.log:149-203`)

```c
unsigned_int fetchMIDIDeviceForService_(id self, SEL _cmd, id svcWrapper) {
    TDriverMIDIDeviceList list(self->fMIDIDriver);   // CoreMIDI's list of devices owned by this driver
    guidNumber = [svcWrapper propertyForKey:@"GUID"];
    if (guidNumber == 0) return 0;
    guidStr = [NSString stringWithFormat:@"%llX", [guidNumber unsignedLongLongValue]];

    for (midiDev : list) {
        if (MIDIObjectGetStringProperty(midiDev, @"com.m-audio.FW.GUID", &existing) == 0) {
            if ([guidStr isEqualToString:existing]) found = midiDev;
            [existing release];
        }
        else if (err + 0x2A54U < 2) {       // err == -10834 (kMIDIUnknownProperty) OR -10835
            // migrate: stamp our GUID key onto a device that lacks it (one-time upgrade path)
            if (MIDIObjectSetStringProperty(midiDev, @"com.m-audio.FW.GUID", guidStr) == 0)
                found = midiDev;
        }
    }
    MIDIDeviceListDispose(list);
    return found;
}
```

**Finding:** the migration branch (`err + 0x2A54U < 2`) stamps the GUID property onto any pre-existing `MIDIDevice` that lacks it — this is how 1.x → 1.11 updates kept user routings. The constant `0x2A54 = 10836` and `MIDIUnknownPropertyError = -10835`, `MIDINotPermitted = -10833` — so the check accepts either "property doesn't exist yet" (first migration) or an adjacent recoverable error. **Hypothesis — the `+ 2 < range` is a C idiom for "err in {-10835, -10834}". Verify by cross-checking Apple's `MIDIServices.h` for the exact OSStatus codes.**

## 3. `createDeviceProperties:withService:` @ 0x3743 (`a3-m2.log:207-264`)

```c
void createDeviceProperties_withService_(id self, SEL _cmd, MIDIDeviceRef dev, id svcWrapper) {
    MIDIObjectSetIntegerProperty(dev, kMIDIPropertyMaxSysExSpeed,       0xC35);   // 3125 bytes/sec = 1x MIDI rate
    MIDIObjectSetIntegerProperty(dev, kMIDIPropertyTransmitChannels,    0xFFFF);  // all 16 channels
    MIDIObjectSetIntegerProperty(dev, kMIDIPropertyReceiveChannels,     0xFFFF);  // all 16 channels
    MIDIObjectSetIntegerProperty(dev, kMIDIPropertyAdvanceScheduleTimeMuSec, 15000);  // 15 ms

    icon = [svcWrapper propertyForKey:@"MAIconName"];
    for (ext in {"png", "tif", "tiff", "jpg", NULL}) {
        path = [NSString stringWithFormat:@"%@%@.%@",
                        @"/Library/Application Support/M-Audio/DeviceIcons/",
                        icon, ext];
        if ([NSFileManager fileExistsAtPath:path]) {
            MIDIObjectSetStringProperty(dev, kMIDIPropertyImage, path);
            return;
        }
    }
}
```

**Findings:**

- **MaxSysExSpeed = 0xC35 (3125 B/s)** — the baseline MIDI 1.0 serial rate. The FW1814 could in principle do much faster SysEx (it's FireWire), but the driver advertises the slow 1x rate so hosts don't flood the bus. **Observation (cited); clone implication is to match 3125 unless we have a reason to deviate.**
- **Advance schedule 15 ms** — CoreMIDI hint for how early scheduled packets arrive at the driver. 15 ms is the standard CoreMIDI example in `MIDIDriver.h` sample code.
- **Transmit/Receive channels = 0xFFFF** — bitmask with all 16 channels set. Standard "no channel restriction" value.
- **Icon lookup** tries 4 extensions in order `png → tif → tiff → jpg`. If none found, no icon is set (CoreMIDI falls back to generic).

## 4. `instantiateDriver:withService:withMIDIDevice:` @ 0x2363 (`a3-m2.log:272-288`)

```c
+ (id) instantiateDriver:(MIDIDriverInterface **)drv
             withService:(id)svcWrapper
          withMIDIDevice:(MIDIDeviceRef)dev
{
    if (sDrivers == nil) sDrivers = [[NSMutableSet alloc] init];
    inst = [[[MAFWMIDIDeviceBeBoB alloc] initWithDevice:dev
                                                 driver:drv
                                                service:svcWrapper] autorelease];
    return inst;
}
```

Lazy-creates the file-scope `sDrivers` NSMutableSet, then alloc/init/autorelease the per-device instance. The autoreleased reference is returned to `driverInstanceMatched:` which checks it against nil for the offline flag; the retained copy that keeps it alive is added inside `initWithDevice:…` via `[sDrivers addObject:self]` (see §5).

## 5. `initWithDevice:driver:service:` @ 0x2588 (`a3-m2b.log:1-40`)

```c
- (id) initWithDevice:(MIDIDeviceRef)dev
               driver:(MIDIDriverInterface **)drv
              service:(MAFWBeBoBIOKitService *)svcWrapper
{
    self = [super init];
    if (!self) return nil;
    self->fDevice    = dev;
    self->fDriver    = drv;
    self->fKitDevice = [svcWrapper retain];

    int err = [self->fKitDevice openConnection];       // *** IOServiceOpen type=0 ***
    if (err != 0) {
        NSLog(@"%lX while opening connection", err);
        [self release];
        return nil;
    }
    if ([self start] != 0) {                           // M6/M7 topic: endpoint/port/notification setup
        [self release];
        return nil;
    }
    [sDrivers addObject:self];                         // retain into the file-scope set
    return self;
}
```

**Finding — this is where the kext user-client is opened.** `openConnection` resolves to:

```c
int MAFWBeBoBIOKitService::openConnection(id self, SEL _cmd) {
    return IOServiceOpen(self->fEntry, mach_task_self(), /*type*/ 0, &self->fConnection);
}
```

(from `a3-m2.log:296-304`.)

**Hypothesis — type=0:** on the kext side this routes to the kext's default `newUserClient` factory, which per A1-Q11 instantiates `com_m_audio_FWKIPCUserClient` — the same user-client class the panel opens in A2-P1. **Verify by (a) decoding the kext's `newUserClient` to confirm type=0 lands on `FWKIPCUserClient` or a dedicated MIDI subclass, and (b) checking in M3/M5 whether the MIDI plugin issues selectors that the panel doesn't.** If a dedicated MIDI user client exists, type would be nonzero — type=0 means they share the one user client.

**Teardown chain** (`a3-m2.log:381-400`):

```c
- (void) dealloc {                           // MAFWBeBoBIOKitService
    if (self->fConnection) IOServiceClose(self->fConnection);
    if (self->fEntry)      IOObjectRelease(self->fEntry);
    [super dealloc];
}
```

Device teardown is **refcount-driven**, not an explicit `closeConnection` from `driverInstanceTerminated:`. The removal from `sDrivers` drops the retain → chain of releases → `dealloc` runs → `IOServiceClose(fConnection)` → `IOObjectRelease(fEntry)`. Clean.

## 6. `fetchInstanceForDevice:` + `terminateInstanceOfDevice:` + `terminateAllDevices` (`a3-m2b.log:46-98`)

```c
+ (id) fetchInstanceForDevice:(MIDIDeviceRef)dev {
    for (inst in sDrivers)
        if ([inst isForDevice:dev]) return inst;
    return nil;
}

+ (void) terminateInstanceOfDevice:(MIDIDeviceRef)dev {
    inst = [self fetchInstanceForDevice:dev];
    if (inst) [sDrivers removeObject:inst];      // drops retain → dealloc → IOServiceClose
}

+ (void) terminateAllDevices {
    [sDrivers release];                           // nukes all per-device state
    sDrivers = nil;
}
```

## 7. Compare with A2 panel path

| step | panel (A2-P1) | MIDI plugin (A3-M2) | notes |
|---|---|---|---|
| matching class | `com_m_audio_FW1814Device` | same (and 5 siblings) | plugin covers whole BeBoB family |
| notification mechanism | `IOServiceAddMatchingNotification` | `MAFWBeBoBServiceNotification`-wrapped version of same API | same primitive, Obj-C façade |
| user client type | 0 (default) | 0 (default) | **both share one user client** per hypothesis — verify at kext |
| retention of `io_connect_t` | `CMiddleWareDevice::fConnection` | `MAFWBeBoBIOKitService::fConnection` | per-instance; both plugins have independent handles |
| teardown | `CDeviceList::DeviceRemoved` | `terminateInstanceOfDevice:` (via refcount) | plugin style, same net effect |

The two plugins are architecturally symmetrical and open independent user-client handles to the same kext IOService — the FW1814 kext must therefore accept at least **2 concurrent user clients per device** (panel + MIDI). **A1-Q11 and A1-Q14 both confirm `com_m_audio_FWKIPCUserClient` has no single-client restriction.**

## 8. Clone implications

1. **Keep the GUID-based reattach.** Our kext must publish `GUID` as `kCFNumber` on the `IOService` so CoreMIDI's `MIDISetup` can keep per-device routings across reboots. The 64-bit value is the FireWire GUID of the device (from the `IOFireWireUnit` → config ROM).
2. **Publish `MAMIDIInputPorts` / `MAMIDIOutputPorts`** as NSArrays of NSStrings. For FW1814 that is:
   - Inputs (what the plugin exposes as `MIDISource`): `["In 1", "In 2"]` — 2 physical MIDI inputs. **Hypothesis — exact strings should match what the panel UI displays. Verify by reading `MAMIDIInputPorts` from a live FW1814 kext via `ioreg -l`.**
   - Outputs: `["Out 1", "Out 2"]` — 2 physical MIDI outputs.
3. **Keep `IOAudioDeviceName` / `IOAudioDeviceManufacturerName`** on the service — these are already published by the audio side of the kext for CoreAudio; MIDI piggybacks on them.
4. **Keep `MAIconName`** + install an icon at `/Library/Application Support/M-Audio/DeviceIcons/<name>.png`. For an "OpenAudio" rebrand we would mirror the convention under our own namespace.
5. **One MIDIEntity per device.** No sub-entities; the kext does not need to publish entity grouping metadata.
6. **MaxSysExSpeed = 3125, AdvanceSchedule = 15000, All-channel bitmasks** — just mirror the values.
7. **User-client type = 0.** Our kext's `newUserClient` already returns the single UC class on type=0 (per CLAUDE.md compliance); nothing to add on our side.
8. **Teardown via refcount** is clean; no explicit teardown selector needed on the kext side.

## 9. Status

**Closes M2.** The IOKit → CoreMIDI bridge is fully mapped: match notification → service-wrapper → GUID-based device lookup/creation → device + manufacturer + ports property read → `MIDIDeviceCreate` → entity + source/dest naming → `MIDISetupAddDevice` → extra properties (icon, speed, channels) → per-device Obj-C instance → `IOServiceOpen` (type 0) → `start` (next topic).

**Opens:**
- **M3 (async notifications):** `MAFWMIDIDeviceBeBoB start` method (unseen so far) is where `MAFWBeBoBAsyncNotification` + `MAFWBeBoBPowerNotification` get installed. Needs its own decomp.
- **M4 (wire packet):** `start` also likely allocates ports and shm/pipe for `DigiFWMIDIPacket` flow.
- **Verification debt:**
  - User-client type=0 lands on `FWKIPCUserClient` (not a dedicated MIDI UC). Verify by decoding kext `newUserClient` or via `ioreg` on hardware.
  - `MAMIDIInputPorts` / `MAMIDIOutputPorts` exact strings and counts for FW1814. Verify from a kext property dump.
  - `TIterator` start-at-1 convention — paper decode of `TIterator::Next` to confirm it doesn't skip index 0.
