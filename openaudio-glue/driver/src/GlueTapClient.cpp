// OpenAudio Glue driver - one unit's Glue tap (see GlueTapClient.h).
#include "GlueTapClient.h"
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <cstring>

namespace gluedrv {

static uint64_t NowNanos()
{
    static mach_timebase_info_data_t tb;
    if (tb.denom == 0) mach_timebase_info(&tb);
    return mach_absolute_time() * tb.numer / tb.denom;
}

bool TapClient::open(io_service_t device)
{
    close();
    if (IOServiceOpen(device, mach_task_self(), 0, &fConn) != KERN_SUCCESS) { fConn = IO_OBJECT_NULL; return false; }
    if (IOConnectMapMemory64(fConn, kGlueTapPageType, mach_task_self(), &fPageAddr, &fPageSize, kIOMapAnywhere) != KERN_SUCCESS ||
        fPageSize < sizeof(GlueTapPage)) {
        close();
        return false;
    }
    fPage = reinterpret_cast<GlueTapPage *>(fPageAddr);
    if (fPage->magic != kGlueTapMagic || fPage->version != kGlueTapVersion) { close(); return false; }
    fRingGeneration = fPage->ringGeneration;
    mapRing();                                         // may fail until the engine has its buffers; remapRingIfNew retries
    return true;
}

void TapClient::close()
{
    if (fConn != IO_OBJECT_NULL) {
        const RingView *r = fRing.exchange(nullptr);
        if (r) fRetired.push_back(Retired{ r, 0 });
        IOServiceClose(fConn);                         // releases every mapping of this connection (page and rings)
    }
    for (auto &x : fRetired) delete x.view;
    fRetired.clear();
    fConn = IO_OBJECT_NULL;
    fPage = nullptr; fPageAddr = 0; fPageSize = 0;
}

bool TapClient::remapRingIfNew()
{
    if (fConn == IO_OBJECT_NULL || !fPage) return false;
    if (fRing.load() && fPage->ringGeneration == fRingGeneration) return true;     // same ring: keep the mapping
    fRingGeneration = fPage->ringGeneration;
    return mapRing();
}

bool TapClient::mapRing()
{
    if (fConn == IO_OBJECT_NULL) return false;
    mach_vm_address_t addr = 0; mach_vm_size_t size = 0;
    if (IOConnectMapMemory64(fConn, kGlueTapRingType, mach_task_self(), &addr, &size, kIOMapAnywhere) != KERN_SUCCESS) return false;
    const RingView *view = new RingView{ reinterpret_cast<const uint32_t *>(addr), (uint32_t)size, addr };
    const RingView *old = fRing.exchange(view, std::memory_order_acq_rel);
    if (old) fRetired.push_back(Retired{ old, NowNanos() });
    return true;
}

void TapClient::reapRetired(bool all)
{
    // The retired mappings themselves stay until close(): only rings the kext replaced are retired, and that is rare (format changes).
    if (!all || fConn != IO_OBJECT_NULL) return;
    for (auto &x : fRetired) delete x.view;
    fRetired.clear();
}

kern_return_t TapClient::start()
{
    if (fConn == IO_OBJECT_NULL) return kIOReturnNotOpen;
    return IOConnectCallScalarMethod(fConn, kGlueTapSelectorStart, nullptr, 0, nullptr, nullptr);
}

kern_return_t TapClient::stop()
{
    if (fConn == IO_OBJECT_NULL) return kIOReturnNotOpen;
    return IOConnectCallScalarMethod(fConn, kGlueTapSelectorStop, nullptr, 0, nullptr, nullptr);
}

// ---- registry ----

static bool Num(CFTypeRef v, uint64_t &out)
{
    if (!v || CFGetTypeID(v) != CFNumberGetTypeID()) return false;
    long long n = 0;
    CFNumberGetValue((CFNumberRef)v, kCFNumberLongLongType, &n);
    out = (uint64_t)n;
    return true;
}

static std::string Str(CFTypeRef v)
{
    if (!v || CFGetTypeID(v) != CFStringGetTypeID()) return std::string();
    char buf[512];
    if (!CFStringGetCString((CFStringRef)v, buf, sizeof buf, kCFStringEncodingUTF8)) return std::string();
    return std::string(buf);
}

// The unit's main engine: an IOAudioEngine child whose IOAudioEngineGlobalUniqueID has exactly one ':' ("<class>:<GUID hex>",
// FWBaseEngine::getGlobalUniqueID with no local part, as glue-probe shows); stereo-pair engines, when enabled, are skipped.
static io_registry_entry_t MainEngine(io_service_t device)
{
    io_iterator_t it = IO_OBJECT_NULL;
    if (IORegistryEntryCreateIterator(device, kIOServicePlane, kIORegistryIterateRecursively, &it) != KERN_SUCCESS) return IO_OBJECT_NULL;
    io_registry_entry_t e, found = IO_OBJECT_NULL;
    while ((e = IOIteratorNext(it))) {
        if (found == IO_OBJECT_NULL && IOObjectConformsTo(e, "IOAudioEngine")) {
            CFTypeRef uid = IORegistryEntryCreateCFProperty(e, CFSTR("IOAudioEngineGlobalUniqueID"), kCFAllocatorDefault, 0);
            const std::string s = Str(uid);
            if (uid) CFRelease(uid);
            if (!s.empty() && s.find(':') == s.rfind(':') && s.find(':') != std::string::npos) { found = e; continue; }
        }
        IOObjectRelease(e);
    }
    IOObjectRelease(it);
    return found;
}

bool TapClient::ReadInfo(io_service_t device, UnitInfo &out)
{
    CFTypeRef g = IORegistryEntryCreateCFProperty(device, CFSTR("GUID"), kCFAllocatorDefault, 0);
    const bool haveGuid = Num(g, out.guid);
    if (g) CFRelease(g);
    if (!haveGuid) return false;
    io_registry_entry_t eng = MainEngine(device);
    if (eng == IO_OBJECT_NULL) return false;
    CFMutableDictionaryRef p = nullptr;
    if (IORegistryEntryCreateCFProperties(eng, &p, kCFAllocatorDefault, 0) != KERN_SUCCESS || !p) { IOObjectRelease(eng); return false; }
    const std::string uid = Str(CFDictionaryGetValue(p, CFSTR("IOAudioEngineGlobalUniqueID")));
    out.guidHex = uid.substr(uid.find(':') + 1);
    std::string desc = Str(CFDictionaryGetValue(p, CFSTR("IOAudioEngineDescription")));
    const std::string tail = " Multichannel";                          // FWBaseEngine::initHardware: "<model> Multichannel"
    if (desc.size() > tail.size() && desc.compare(desc.size() - tail.size(), tail.size(), tail) == 0) desc.resize(desc.size() - tail.size());
    if (desc.empty()) desc = "FireWire";
    out.name = desc + " Glue";
    uint64_t v = 0;
    out.clockDomain = Num(CFDictionaryGetValue(p, CFSTR("IOAudioEngineClockDomain")), v) ? (uint32_t)v : 0;
    out.inputLatency = Num(CFDictionaryGetValue(p, CFSTR("IOAudioEngineInputSampleLatency")), v) ? (uint32_t)v : 0;
    if (Num(CFDictionaryGetValue(p, CFSTR("IOAudioEngineInputSampleOffset")), v)) out.inputSafetyOffset = (uint32_t)v;
    else out.inputSafetyOffset = Num(CFDictionaryGetValue(p, CFSTR("IOAudioEngineSampleOffset")), v) ? (uint32_t)v : 0;
    out.channelNames.clear();
    CFTypeRef names = CFDictionaryGetValue(p, CFSTR("IOAudioEngineChannelNames"));
    if (names && CFGetTypeID(names) == CFDictionaryGetTypeID()) {
        for (uint32_t c = 1; c <= kGlueTapMaxChannels; c++) {
            CFStringRef key = CFStringCreateWithFormat(kCFAllocatorDefault, nullptr, CFSTR("InputChannel%u"), (unsigned)c);
            const std::string n = Str(CFDictionaryGetValue((CFDictionaryRef)names, key));
            CFRelease(key);
            out.channelNames.push_back(n);
        }
    }
    CFRelease(p);
    IOObjectRelease(eng);
    return !out.guidHex.empty();
}

} // namespace gluedrv
