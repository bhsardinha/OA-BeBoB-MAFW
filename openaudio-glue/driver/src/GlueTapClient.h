// OpenAudio Glue driver - the IOKit side: one unit's Glue tap (openaudio-mafw-bebob/kext/src/base/GlueTap.h, main R-log R143).
// Opens the unit's vendor user client (com_openaudio_FWUserClient, listed in Info.plist AudioServerPlugIn_IOKitUserClients as
// AudioServerPlugIn.h line 52 requires), maps the Glue page and the input ring read-only, and holds or releases one isoch reference
// (selectors 508/509). Also reads what the plug-in reports from the registry: GUID, name, clock domain, latency, channel names.
#pragma once
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <atomic>
#include <string>
#include <vector>
#include "GlueTap.h"

namespace gluedrv {

struct UnitInfo {                       // from the registry (the device and its main IOAudioEngine)
    uint64_t guid = 0;
    std::string guidHex;                // "0xD6C040012337F", as in the engine's Core Audio UID
    std::string name;                   // "FireWire 1814 Glue"
    uint32_t clockDomain = 0;           // IOAudioEngineClockDomain (= the HAL's kAudioDevicePropertyClockDomain, Glue G9)
    uint32_t inputLatency = 0;          // IOAudioEngineInputSampleLatency
    uint32_t inputSafetyOffset = 0;     // IOAudioEngineInputSampleOffset, else IOAudioEngineSampleOffset
    std::vector<std::string> channelNames;   // IOAudioEngineChannelNames "InputChannel<n>"
};

// One mapping of the ring: base and size published together, so an IO thread never pairs a new size with an old mapping.
struct RingView { const uint32_t *base; uint32_t bytes; mach_vm_address_t addr; };

class TapClient {
public:
    ~TapClient() { close(); }
    bool open(io_service_t device);     // user client + page + ring
    void close();
    bool isOpen() const { return fConn != IO_OBJECT_NULL && fPage != nullptr; }
    const GlueTapPage *page() const { return fPage; }

    // The ring as last mapped; replaced by remapRing() (old views and mappings are freed later by reapRetired, so an IO thread
    // that loaded the view just before keeps valid memory).
    const RingView *ring() const { return fRing.load(std::memory_order_acquire); }
    // Map the ring again only when the kext allocated a new one (page ringGeneration). Retired mappings stay mapped until close():
    // IOConnectUnmapMemory64 removes a mapping of the type's current descriptor, which can be the live one (Glue G11).
    bool remapRingIfNew();
    void reapRetired(bool all = false); // frees retired views older than 2 s once the connection is closed (or all)

    kern_return_t start();              // selector 508: one isoch reference
    kern_return_t stop();               // selector 509

    static bool ReadInfo(io_service_t device, UnitInfo &out);
    // Host tests (driver/tests): a page and ring in ordinary memory instead of the kext's mappings; start/stop then do nothing.
    void attachForTest(GlueTapPage *page, const uint32_t *ring, uint32_t bytes)
    {
        fPage = page;
        fRingGeneration = page->ringGeneration;
        fRing.store(new RingView{ ring, bytes, 0 }, std::memory_order_release);
    }

private:
    io_connect_t fConn = IO_OBJECT_NULL;
    GlueTapPage *fPage = nullptr;
    mach_vm_address_t fPageAddr = 0;
    mach_vm_size_t fPageSize = 0;
    std::atomic<const RingView *> fRing{ nullptr };
    uint32_t fRingGeneration = 0;
    bool mapRing();
    struct Retired { const RingView *view; uint64_t at; };
    std::vector<Retired> fRetired;
};

} // namespace gluedrv
