// OpenAudio Glue driver - the AudioServerPlugIn (10.13 SDK AudioServerPlugIn.h; Glue G1). One Glue device per FireWire 1814, found
// through IOKit notifications on com_openaudio_FW1814Device (Glue is FW1814-only on purpose: user, 2026-11-06). Everything that may take time (opening
// the tap, calibrating presets, following changes) runs on one serial queue; the IO entry points never lock or allocate.
#include <CoreAudio/AudioServerPlugIn.h>
#include <dispatch/dispatch.h>
#include <os/log.h>
#include <sys/sysctl.h>
#include <atomic>
#include "GlueDevice.h"

using namespace gluedrv;

namespace {

const int kMaxDevices = 8;
const AudioObjectID kFirstDeviceID = 16;                 // device = 16 + 4 * slot, its stream = device + 1

AudioServerPlugInHostRef gHost = nullptr;
std::atomic<ULONG> gRefCount{ 0 };
Workers gWorkers;
std::atomic<Device *> gSlot[kMaxDevices];
dispatch_queue_t gQueue = nullptr;
dispatch_source_t gTimer = nullptr;
IONotificationPortRef gPort = nullptr;
io_iterator_t gMatchIt = IO_OBJECT_NULL, gTermIt = IO_OBJECT_NULL;
os_log_t gLog = OS_LOG_DEFAULT;

Device *DeviceFor(AudioObjectID obj)
{
    if (obj < kFirstDeviceID) return nullptr;
    const int slot = (int)((obj - kFirstDeviceID) / 4);
    if (slot >= kMaxDevices) return nullptr;
    Device *d = gSlot[slot].load(std::memory_order_acquire);
    if (!d || (obj != d->deviceID() && obj != d->streamID())) return nullptr;
    return d;
}

void AnnounceDeviceList()
{
    const AudioObjectPropertyAddress a[2] = {
        { kAudioPlugInPropertyDeviceList, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster },
        { kAudioObjectPropertyOwnedObjects, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster } };
    gHost->PropertiesChanged(gHost, kAudioObjectPlugInObject, 2, a);
}

UInt32 ListedDevices(AudioObjectID *out, UInt32 max)
{
    UInt32 n = 0;
    for (int i = 0; i < kMaxDevices && n < max; i++) {
        Device *d = gSlot[i].load(std::memory_order_acquire);
        if (d && d->listed()) { if (out) out[n] = d->deviceID(); n++; }
    }
    return n;
}

// ---- unit discovery (gQueue) ----

void Matched(void *, io_iterator_t it)
{
    io_service_t s;
    while ((s = IOIteratorNext(it))) {
        int free = -1;
        for (int i = 0; i < kMaxDevices; i++) if (!gSlot[i].load()) { free = i; break; }
        if (free >= 0) {
            gSlot[free].store(new Device(kFirstDeviceID + 4 * (AudioObjectID)free, s, gHost, &gWorkers), std::memory_order_release);
            os_log(gLog, "OpenAudio Glue: unit found, slot %d", free);
        }
        IOObjectRelease(s);
    }
}

void Terminated(void *, io_iterator_t it)
{
    io_service_t s;
    bool changed = false;
    while ((s = IOIteratorNext(it))) {
        for (int i = 0; i < kMaxDevices; i++) {
            Device *d = gSlot[i].load();
            if (!d || !IOObjectIsEqualTo(d->service(), s) || !d->alive()) continue;
            const bool wasListed = d->listed();
            d->retire();
            changed |= wasListed;
            os_log(gLog, "OpenAudio Glue: unit gone, slot %d", i);
            // Free the slot later: an IO or property thread that looked the device up just before keeps a valid object.
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), gQueue, ^{
                Device *old = gSlot[i].exchange(nullptr);
                delete old;
            });
        }
        IOObjectRelease(s);
    }
    if (changed) AnnounceDeviceList();
}

void Monitor()
{
    bool changed = false;
    for (int i = 0; i < kMaxDevices; i++) {
        Device *d = gSlot[i].load();
        if (d && d->poll()) { changed = true; os_log(gLog, "OpenAudio Glue: device published, slot %d", i); }
    }
    if (changed) AnnounceDeviceList();
}

// ---- the driver interface ----

HRESULT QueryInterface(void *driver, REFIID iid, LPVOID *out);
ULONG AddRef(void *) { return ++gRefCount; }
ULONG Release(void *) { ULONG n = gRefCount.load(); if (n > 0) n = --gRefCount; return n; }

OSStatus Initialize(AudioServerPlugInDriverRef, AudioServerPlugInHostRef host)
{
    gHost = host;
    gLog = os_log_create("com.openaudio.glue", "driver");
    int logical = 1; size_t len = sizeof logical;
    sysctlbyname("hw.logicalcpu", &logical, &len, nullptr, 0);
    int workers = logical - 1;                           // the IO thread is the other one
    if (workers > 3) workers = 3;
    gWorkers.start(workers);
    gQueue = dispatch_queue_create("com.openaudio.glue.monitor", DISPATCH_QUEUE_SERIAL);
    gPort = IONotificationPortCreate(kIOMasterPortDefault);
    IONotificationPortSetDispatchQueue(gPort, gQueue);
    dispatch_sync(gQueue, ^{
        IOServiceAddMatchingNotification(gPort, kIOFirstMatchNotification, IOServiceMatching("com_openaudio_FW1814Device"), &Matched, nullptr, &gMatchIt);
        Matched(nullptr, gMatchIt);                      // arm, and take the units already there
        IOServiceAddMatchingNotification(gPort, kIOTerminatedNotification, IOServiceMatching("com_openaudio_FW1814Device"), &Terminated, nullptr, &gTermIt);
        Terminated(nullptr, gTermIt);
    });
    gTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, gQueue);
    dispatch_source_set_timer(gTimer, dispatch_time(DISPATCH_TIME_NOW, 0), 50 * NSEC_PER_MSEC, 10 * NSEC_PER_MSEC);
    dispatch_source_set_event_handler(gTimer, ^{ Monitor(); });
    dispatch_resume(gTimer);
    os_log(gLog, "OpenAudio Glue: initialised, %d DSP worker thread(s)", gWorkers.threads());
    return noErr;
}

OSStatus CreateDevice(AudioServerPlugInDriverRef, CFDictionaryRef, const AudioServerPlugInClientInfo *, AudioObjectID *)
{
    return kAudioHardwareUnsupportedOperationError;
}
OSStatus DestroyDevice(AudioServerPlugInDriverRef, AudioObjectID) { return kAudioHardwareUnsupportedOperationError; }
OSStatus AddDeviceClient(AudioServerPlugInDriverRef, AudioObjectID, const AudioServerPlugInClientInfo *) { return noErr; }
OSStatus RemoveDeviceClient(AudioServerPlugInDriverRef, AudioObjectID, const AudioServerPlugInClientInfo *) { return noErr; }

OSStatus PerformConfigChange(AudioServerPlugInDriverRef, AudioObjectID device, UInt64, void *)
{
    Device *d = DeviceFor(device);
    if (!d) return kAudioHardwareBadObjectError;
    d->performConfigurationChange();                     // serialised with the monitor by the device's own lock
    return noErr;
}
OSStatus AbortConfigChange(AudioServerPlugInDriverRef, AudioObjectID, UInt64, void *) { return noErr; }

// plug-in object properties
bool PlugInHas(const AudioObjectPropertyAddress &a)
{
    switch (a.mSelector) {
    case kAudioObjectPropertyBaseClass: case kAudioObjectPropertyClass: case kAudioObjectPropertyOwner:
    case kAudioObjectPropertyManufacturer: case kAudioObjectPropertyOwnedObjects: case kAudioPlugInPropertyDeviceList:
    case kAudioPlugInPropertyTranslateUIDToDevice: case kAudioPlugInPropertyResourceBundle:
        return true;
    default: return false;
    }
}

Boolean HasProperty(AudioServerPlugInDriverRef, AudioObjectID obj, pid_t, const AudioObjectPropertyAddress *a)
{
    if (obj == kAudioObjectPlugInObject) return PlugInHas(*a);
    Device *d = DeviceFor(obj);
    return d && d->hasProperty(obj, *a);
}

OSStatus IsPropertySettable(AudioServerPlugInDriverRef, AudioObjectID obj, pid_t, const AudioObjectPropertyAddress *a, Boolean *out)
{
    if (obj == kAudioObjectPlugInObject) { if (!PlugInHas(*a)) return kAudioHardwareUnknownPropertyError; *out = false; return noErr; }
    Device *d = DeviceFor(obj);
    return d ? d->isSettable(obj, *a, *out) : kAudioHardwareBadObjectError;
}

OSStatus GetPropertyDataSize(AudioServerPlugInDriverRef, AudioObjectID obj, pid_t, const AudioObjectPropertyAddress *a, UInt32 qs,
                             const void *q, UInt32 *out)
{
    if (obj == kAudioObjectPlugInObject) {
        switch (a->mSelector) {
        case kAudioObjectPropertyBaseClass: case kAudioObjectPropertyClass: *out = sizeof(AudioClassID); return noErr;
        case kAudioObjectPropertyOwner: case kAudioPlugInPropertyTranslateUIDToDevice: *out = sizeof(AudioObjectID); return noErr;
        case kAudioObjectPropertyManufacturer: case kAudioPlugInPropertyResourceBundle: *out = sizeof(CFStringRef); return noErr;
        case kAudioObjectPropertyOwnedObjects: case kAudioPlugInPropertyDeviceList:
            *out = ListedDevices(nullptr, kMaxDevices) * sizeof(AudioObjectID); return noErr;
        default: return kAudioHardwareUnknownPropertyError;
        }
    }
    Device *d = DeviceFor(obj);
    return d ? d->getSize(obj, *a, qs, q, *out) : kAudioHardwareBadObjectError;
}

OSStatus GetPropertyData(AudioServerPlugInDriverRef, AudioObjectID obj, pid_t, const AudioObjectPropertyAddress *a, UInt32 qs,
                         const void *q, UInt32 inSize, UInt32 *outSize, void *out)
{
    if (obj == kAudioObjectPlugInObject) {
        switch (a->mSelector) {
        case kAudioObjectPropertyBaseClass:
            if (inSize < sizeof(AudioClassID)) return kAudioHardwareBadPropertySizeError;
            *(AudioClassID *)out = kAudioObjectClassID; *outSize = sizeof(AudioClassID); return noErr;
        case kAudioObjectPropertyClass:
            if (inSize < sizeof(AudioClassID)) return kAudioHardwareBadPropertySizeError;
            *(AudioClassID *)out = kAudioPlugInClassID; *outSize = sizeof(AudioClassID); return noErr;
        case kAudioObjectPropertyOwner:
            if (inSize < sizeof(AudioObjectID)) return kAudioHardwareBadPropertySizeError;
            *(AudioObjectID *)out = kAudioObjectUnknown; *outSize = sizeof(AudioObjectID); return noErr;
        case kAudioObjectPropertyManufacturer:
            if (inSize < sizeof(CFStringRef)) return kAudioHardwareBadPropertySizeError;
            *(CFStringRef *)out = CFSTR("OpenAudio"); *outSize = sizeof(CFStringRef); return noErr;
        case kAudioPlugInPropertyResourceBundle:
            if (inSize < sizeof(CFStringRef)) return kAudioHardwareBadPropertySizeError;
            *(CFStringRef *)out = CFSTR(""); *outSize = sizeof(CFStringRef); return noErr;
        case kAudioObjectPropertyOwnedObjects: case kAudioPlugInPropertyDeviceList:
            *outSize = ListedDevices((AudioObjectID *)out, inSize / sizeof(AudioObjectID)) * sizeof(AudioObjectID); return noErr;
        case kAudioPlugInPropertyTranslateUIDToDevice: {
            if (inSize < sizeof(AudioObjectID) || qs < sizeof(CFStringRef) || !q) return kAudioHardwareBadPropertySizeError;
            CFStringRef want = *(const CFStringRef *)q;
            AudioObjectID found = kAudioObjectUnknown;
            for (int i = 0; i < kMaxDevices; i++) {
                Device *d = gSlot[i].load();
                if (!d || !d->listed()) continue;
                CFStringRef u = CFStringCreateWithCString(kCFAllocatorDefault, d->uid().c_str(), kCFStringEncodingUTF8);
                if (u && want && CFStringCompare(u, want, 0) == kCFCompareEqualTo) found = d->deviceID();
                if (u) CFRelease(u);
            }
            *(AudioObjectID *)out = found; *outSize = sizeof(AudioObjectID);
            return noErr;
        }
        default: return kAudioHardwareUnknownPropertyError;
        }
    }
    Device *d = DeviceFor(obj);
    return d ? d->get(obj, *a, qs, q, inSize, *outSize, out) : kAudioHardwareBadObjectError;
}

OSStatus SetPropertyData(AudioServerPlugInDriverRef, AudioObjectID obj, pid_t, const AudioObjectPropertyAddress *a, UInt32 qs,
                         const void *q, UInt32 inSize, const void *in)
{
    if (obj == kAudioObjectPlugInObject) return PlugInHas(*a) ? kAudioHardwareUnsupportedOperationError : kAudioHardwareUnknownPropertyError;
    Device *d = DeviceFor(obj);
    return d ? d->set(obj, *a, qs, q, inSize, in) : kAudioHardwareBadObjectError;
}

OSStatus StartIO(AudioServerPlugInDriverRef, AudioObjectID dev, UInt32)
{
    Device *d = DeviceFor(dev);
    return d ? d->startIO() : kAudioHardwareBadObjectError;
}

OSStatus StopIO(AudioServerPlugInDriverRef, AudioObjectID dev, UInt32)
{
    Device *d = DeviceFor(dev);
    return d ? d->stopIO() : kAudioHardwareBadObjectError;
}

OSStatus GetZeroTimeStamp(AudioServerPlugInDriverRef, AudioObjectID dev, UInt32, Float64 *sample, UInt64 *host, UInt64 *seed)
{
    Device *d = DeviceFor(dev);
    return d ? d->zeroTimeStamp(*sample, *host, *seed) : kAudioHardwareBadObjectError;
}

OSStatus WillDoIOOperation(AudioServerPlugInDriverRef, AudioObjectID, UInt32, UInt32 op, Boolean *willDo, Boolean *inPlace)
{
    *willDo = (op == kAudioServerPlugInIOOperationReadInput);
    *inPlace = true;
    return noErr;
}

OSStatus BeginIOOperation(AudioServerPlugInDriverRef, AudioObjectID, UInt32, UInt32, UInt32, const AudioServerPlugInIOCycleInfo *)
{
    return noErr;
}

OSStatus DoIOOperation(AudioServerPlugInDriverRef, AudioObjectID dev, AudioObjectID, UInt32, UInt32 op, UInt32 frames,
                       const AudioServerPlugInIOCycleInfo *cycle, void *main, void *)
{
    if (op != kAudioServerPlugInIOOperationReadInput) return noErr;
    Device *d = DeviceFor(dev);
    if (!d) return kAudioHardwareBadObjectError;
    d->readInput(cycle->mInputTime.mSampleTime, frames, static_cast<float *>(main));
    return noErr;
}

OSStatus EndIOOperation(AudioServerPlugInDriverRef, AudioObjectID, UInt32, UInt32, UInt32, const AudioServerPlugInIOCycleInfo *)
{
    return noErr;
}

AudioServerPlugInDriverInterface gInterface = {
    nullptr, QueryInterface, AddRef, Release, Initialize, CreateDevice, DestroyDevice, AddDeviceClient, RemoveDeviceClient,
    PerformConfigChange, AbortConfigChange, HasProperty, IsPropertySettable, GetPropertyDataSize, GetPropertyData, SetPropertyData,
    StartIO, StopIO, GetZeroTimeStamp, WillDoIOOperation, BeginIOOperation, DoIOOperation, EndIOOperation };
AudioServerPlugInDriverInterface *gInterfacePtr = &gInterface;
AudioServerPlugInDriverRef gDriverRef = &gInterfacePtr;

HRESULT QueryInterface(void *driver, REFIID iid, LPVOID *out)
{
    if (driver != gDriverRef || !out) return kAudioHardwareBadObjectError;
    CFUUIDRef want = CFUUIDCreateFromUUIDBytes(nullptr, iid);
    HRESULT r = E_NOINTERFACE;
    if (CFEqual(want, IUnknownUUID) || CFEqual(want, kAudioServerPlugInDriverInterfaceUUID)) {
        ++gRefCount;
        *out = gDriverRef;
        r = S_OK;
    }
    CFRelease(want);
    return r;
}

} // namespace

// The factory named in Info.plist (CFPlugInFactories).
extern "C" __attribute__((visibility("default"))) void *GlueDriverFactory(CFAllocatorRef, CFUUIDRef typeUUID)
{
    if (!CFEqual(typeUUID, kAudioServerPlugInTypeUUID)) return nullptr;
    return gDriverRef;
}
