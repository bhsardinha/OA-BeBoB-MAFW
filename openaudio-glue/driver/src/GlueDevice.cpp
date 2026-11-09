// OpenAudio Glue driver - one Glue device (see GlueDevice.h).
#include "GlueDevice.h"
#include <CoreAudio/AudioHardwareBase.h>
#include <mach/mach_time.h>
#include <cmath>
#include <map>
#include <cstdlib>

namespace gluedrv {

// ---- presets per rate: calibrated once off the IO thread, kept for the life of the plug-in --------------------------------
static const glue::Presets *PresetsForRate(uint32_t rate)
{
    static std::mutex lock;
    static std::map<uint32_t, glue::Presets *> cache;
    std::lock_guard<std::mutex> g(lock);
    auto it = cache.find(rate);
    if (it != cache.end()) return it->second;
    glue::Presets *p = new glue::Presets();
    p->calibrate((double)rate);
    cache[rate] = p;
    return p;
}

static double HostTicksPerSecond()
{
    mach_timebase_info_data_t tb; mach_timebase_info(&tb);
    return 1e9 * (double)tb.denom / (double)tb.numer;
}

static uint64_t NowNanos()
{
    static mach_timebase_info_data_t tb;
    if (tb.denom == 0) mach_timebase_info(&tb);
    return mach_absolute_time() * tb.numer / tb.denom;
}

// FWUnpackAM824 (openaudio-mafw-bebob/kext/src/base/FWSamples.h): one wire quadlet as the little-endian host reads it -> signed 24-bit
static inline int32_t UnpackAM824(uint32_t x)
{
    return (int32_t)(((x & 0xff00u) << 16) | (x & 0xff0000u) | ((x >> 24) << 8)) >> 8;
}

Device::Device(AudioObjectID deviceID, io_service_t service, AudioServerPlugInHostRef host, Workers *workers)
    : fDeviceID(deviceID), fService(service), fHost(host), fWorkers(workers)
{
    IOObjectRetain(fService);
    fScratch = static_cast<float *>(calloc((size_t)kGlueTapMaxChannels * kGlueTapRingFrames, sizeof(float)));
    for (uint32_t c = 0; c < kGlueTapMaxChannels; c++) { fGr[c].store(0); fHit[c].store(0); }
}

Device::~Device()
{
    TapClient *t = fTap.exchange(nullptr);
    if (t) { if (fIORefs.load() > 0) t->stop(); delete t; }
    for (auto &r : fRetiredTaps) delete r.tap;
    free(fScratch);
    IOObjectRelease(fService);
}

// ---- monitor (the plug-in's serial queue) ----------------------------------------------------------------------------------

void Device::reapTaps(bool all)
{
    const uint64_t now = NowNanos();
    for (size_t i = 0; i < fRetiredTaps.size();) {
        if (all || now - fRetiredTaps[i].at > 2000000000ull) { delete fRetiredTaps[i].tap; fRetiredTaps.erase(fRetiredTaps.begin() + (long)i); }
        else i++;
    }
}

bool Device::poll()
{
    bool request = false, published = false;
    {
        std::lock_guard<std::recursive_mutex> lock(fMonLock);
        published = pollLocked(request);
    }
    // Outside the lock: the host may call PerformDeviceConfigurationChange before RequestDeviceConfigurationChange returns.
    if (request) fHost->RequestDeviceConfigurationChange(fHost, fDeviceID, 0, nullptr);
    return published;
}

bool Device::pollLocked(bool &request)
{
    if (!fAlive.load()) return false;
    reapTaps(false);
    TapClient *tap = fTap.load();
    if (!tap || tap->page()->state == kGlueTapRetired) {
        // First open, or the engine behind the old tap is gone (re-published): open a new client, retire the old one after 2 s so
        // an IO thread that still holds it reads valid memory.
        TapClient *n = new TapClient();
        if (!n->open(fService)) { delete n; return false; }
        if (fIORefs.load() > 0) n->start();
        TapClient *old = fTap.exchange(n);
        if (old) fRetiredTaps.push_back(RetiredTap{ old, NowNanos() });   // its isoch reference goes with its connection
        fSeenGeneration = 0;
        tap = n;
    }
    tap->reapRetired(false);
    const GlueTapPage *g = tap->page();
    const uint32_t gen = g->generation;
    if (gen != fSeenGeneration) {                       // new stream or new geometry: start the DSP clean
        fSeenGeneration = gen;
        fResetDSP.store(true);
    }
    tap->remapRingIfNew();                              // only when the kext allocated a new ring (or the first map failed)
    if (!fPublished.load()) {
        if (g->rateHz == 0 || g->inChannels == 0 || !tap->ring()) return false;
        UnitInfo info;
        if (!TapClient::ReadInfo(fService, info)) return false;
        fInfo = info;
        takeGeometry();
        loadSettings();
        fPublished.store(true);
        return true;
    }
    if (!fConfigPending.load() && (g->rateHz != fPub.rate || g->inChannels != fPub.channels)) {
        fConfigPending.store(true);
        request = true;
    }
    return false;
}

void Device::takeGeometry()
{
    TapClient *tap = fTap.load();
    const GlueTapPage *g = tap ? tap->page() : nullptr;
    if (g) { fPub.rate = g->rateHz; fPub.channels = g->inChannels; }
    fPub.safety = fInfo.inputSafetyOffset;
    fPub.domain = fInfo.clockDomain;
    fPub.name = fInfo.name;
    fPub.uid = "com.openaudio.glue:" + fInfo.guidHex;
    fPub.channelNames = fInfo.channelNames;
    fHostTicksPerFrame = fPub.rate ? HostTicksPerSecond() / (double)fPub.rate : 0;
    fPresets = fPub.rate ? PresetsForRate(fPub.rate) : nullptr;     // calibration (~130 ms) here, never on the IO thread
    // The unit's latency plus Glue's fixed delay compensation (the same for every channel and setting at this rate), so the DAW
    // aligns Glue's inputs with the unit's own ones.
    fPub.latency = fInfo.inputLatency + (fPresets ? (UInt32)fPresets->maxLatency() : 0);
    fResetDSP.store(true);
}

void Device::performConfigurationChange()
{
    std::lock_guard<std::recursive_mutex> lock(fMonLock);
    UnitInfo info;
    if (TapClient::ReadInfo(fService, info)) fInfo = info;      // latency follows the rate
    takeGeometry();
    fConfigPending.store(false);
}

void Device::retire()
{
    fAlive.store(false);                                // IO returns silence; the plug-in deletes the device later
}

// ---- settings --------------------------------------------------------------------------------------------------------------

CFStringRef Device::storageKey() const
{
    return CFStringCreateWithFormat(kCFAllocatorDefault, nullptr, CFSTR("settings-%s"), fInfo.guidHex.c_str());
}

void Device::loadSettings()
{
    std::lock_guard<std::mutex> g(fSetLock);
    CFStringRef key = storageKey();
    CFPropertyListRef p = nullptr;
    if (fHost->CopyFromStorage(fHost, key, &p) == noErr && p) { SettingsFromPlist(p, fSettings); CFRelease(p); }
    CFRelease(key);
    fSettings.version++;
    fBox.write(fSettings);
    fSettingsVersion.store(fSettings.version, std::memory_order_release);
}

void Device::saveSettings()
{
    CFStringRef key = storageKey();
    CFDictionaryRef p = SettingsToPlist(fSettings, kGlueTapMaxChannels);
    fHost->WriteToStorage(fHost, key, p);
    CFRelease(p);
    CFRelease(key);
}

void Device::applySettingsOnIOThread(bool force)
{
    if (!force && fSettingsVersion.load(std::memory_order_acquire) == fAppliedVersion) return;
    UnitSettings s;
    if (!fBox.read(s)) return;                          // a write in progress: apply next cycle
    // Stereo link: the follower (c + 1) takes the leader's settings (EQ, compressor, on/off); the pair is processed together.
    for (uint32_t c = 0; c + 1 < kGlueTapMaxChannels; c += 2) {
        fLinked[c] = s.channel[c].linkNext && c + 1 < fPub.channels;
        fLinked[c + 1] = false;
        if (fLinked[c]) { s.channel[c + 1] = s.channel[c]; s.channel[c + 1].linkNext = false; }
    }
    fApplied = s;
    fAppliedVersion = s.version;
    for (uint32_t c = 0; c < kGlueTapMaxChannels; c++) {
        fChain[c].setBypassAll(s.bypass);                // bypass still passes through the compensation delay
        fChain[c].setGlobal(s.global);
        fChain[c].setChannel(s.channel[c]);
    }
}

// ---- properties ------------------------------------------------------------------------------------------------------------

AudioStreamBasicDescription Device::format() const
{
    AudioStreamBasicDescription f = {};
    f.mSampleRate = fPub.rate;
    f.mFormatID = kAudioFormatLinearPCM;
    f.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
    f.mChannelsPerFrame = fPub.channels;
    f.mBitsPerChannel = 32;
    f.mBytesPerFrame = 4 * fPub.channels;
    f.mFramesPerPacket = 1;
    f.mBytesPerPacket = f.mBytesPerFrame;
    return f;
}

static CFStringRef CF(const std::string &s) { return CFStringCreateWithCString(kCFAllocatorDefault, s.c_str(), kCFStringEncodingUTF8); }

bool Device::hasProperty(AudioObjectID obj, const AudioObjectPropertyAddress &a) const
{
    if (obj == fDeviceID) {
        switch (a.mSelector) {
        case kAudioObjectPropertyBaseClass: case kAudioObjectPropertyClass: case kAudioObjectPropertyOwner:
        case kAudioObjectPropertyName: case kAudioObjectPropertyManufacturer: case kAudioObjectPropertyOwnedObjects:
        case kAudioDevicePropertyDeviceUID: case kAudioDevicePropertyModelUID: case kAudioDevicePropertyTransportType:
        case kAudioDevicePropertyRelatedDevices: case kAudioDevicePropertyClockDomain: case kAudioDevicePropertyDeviceIsAlive:
        case kAudioDevicePropertyDeviceIsRunning: case kAudioDevicePropertyDeviceCanBeDefaultDevice:
        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice: case kAudioDevicePropertyLatency: case kAudioDevicePropertyStreams:
        case kAudioObjectPropertyControlList: case kAudioDevicePropertySafetyOffset: case kAudioDevicePropertyNominalSampleRate:
        case kAudioDevicePropertyAvailableNominalSampleRates: case kAudioDevicePropertyIsHidden:
        case kAudioDevicePropertyPreferredChannelsForStereo: case kAudioDevicePropertyZeroTimeStampPeriod:
        case kAudioObjectPropertyCustomPropertyInfoList: case kGluePropertySettings: case kGluePropertyMeters:
            return true;
        case kAudioObjectPropertyElementName:
            return a.mScope == kAudioObjectPropertyScopeInput && a.mElement >= 1 && a.mElement <= fPub.channels;
        default: return false;
        }
    }
    if (obj == streamID()) {
        switch (a.mSelector) {
        case kAudioObjectPropertyBaseClass: case kAudioObjectPropertyClass: case kAudioObjectPropertyOwner:
        case kAudioObjectPropertyOwnedObjects: case kAudioStreamPropertyIsActive: case kAudioStreamPropertyDirection:
        case kAudioStreamPropertyTerminalType: case kAudioStreamPropertyStartingChannel: case kAudioStreamPropertyLatency:
        case kAudioStreamPropertyVirtualFormat: case kAudioStreamPropertyPhysicalFormat:
        case kAudioStreamPropertyAvailableVirtualFormats: case kAudioStreamPropertyAvailablePhysicalFormats:
            return true;
        default: return false;
        }
    }
    return false;
}

OSStatus Device::isSettable(AudioObjectID obj, const AudioObjectPropertyAddress &a, Boolean &out) const
{
    if (!hasProperty(obj, a)) return kAudioHardwareUnknownPropertyError;
    out = (obj == fDeviceID && a.mSelector == kGluePropertySettings);
    return noErr;
}

OSStatus Device::getSize(AudioObjectID obj, const AudioObjectPropertyAddress &a, UInt32, const void *, UInt32 &out) const
{
    if (!hasProperty(obj, a)) return kAudioHardwareUnknownPropertyError;
    const bool input = a.mScope == kAudioObjectPropertyScopeInput || a.mScope == kAudioObjectPropertyScopeGlobal;
    switch (a.mSelector) {
    case kAudioObjectPropertyBaseClass: case kAudioObjectPropertyClass: out = sizeof(AudioClassID); return noErr;
    case kAudioObjectPropertyOwner: out = sizeof(AudioObjectID); return noErr;
    case kAudioObjectPropertyName: case kAudioObjectPropertyManufacturer: case kAudioDevicePropertyDeviceUID:
    case kAudioDevicePropertyModelUID: case kAudioObjectPropertyElementName: out = sizeof(CFStringRef); return noErr;
    case kAudioObjectPropertyOwnedObjects:
        out = (obj == fDeviceID) ? sizeof(AudioObjectID) : 0; return noErr;
    case kAudioDevicePropertyStreams: out = (obj == fDeviceID && input) ? sizeof(AudioObjectID) : 0; return noErr;
    case kAudioObjectPropertyControlList: out = 0; return noErr;
    case kAudioDevicePropertyRelatedDevices: out = sizeof(AudioObjectID); return noErr;
    case kAudioDevicePropertyAvailableNominalSampleRates: out = sizeof(AudioValueRange); return noErr;
    case kAudioDevicePropertyNominalSampleRate: out = sizeof(Float64); return noErr;
    case kAudioDevicePropertyPreferredChannelsForStereo: out = 2 * sizeof(UInt32); return noErr;
    case kAudioObjectPropertyCustomPropertyInfoList: out = 2 * sizeof(AudioServerPlugInCustomPropertyInfo); return noErr;
    case kGluePropertySettings: case kGluePropertyMeters: out = sizeof(CFPropertyListRef); return noErr;
    case kAudioStreamPropertyVirtualFormat: case kAudioStreamPropertyPhysicalFormat: out = sizeof(AudioStreamBasicDescription); return noErr;
    case kAudioStreamPropertyAvailableVirtualFormats: case kAudioStreamPropertyAvailablePhysicalFormats:
        out = sizeof(AudioStreamRangedDescription); return noErr;
    default: out = sizeof(UInt32); return noErr;
    }
}

OSStatus Device::get(AudioObjectID obj, const AudioObjectPropertyAddress &a, UInt32 qs, const void *q, UInt32 inSize, UInt32 &outSize,
                     void *out) const
{
    UInt32 need = 0;
    OSStatus e = getSize(obj, a, qs, q, need);
    if (e) return e;
    if (inSize < need) return kAudioHardwareBadPropertySizeError;
    outSize = need;
    const bool input = a.mScope == kAudioObjectPropertyScopeInput || a.mScope == kAudioObjectPropertyScopeGlobal;
    if (obj == fDeviceID) {
        switch (a.mSelector) {
        case kAudioObjectPropertyBaseClass: *(AudioClassID *)out = kAudioObjectClassID; return noErr;
        case kAudioObjectPropertyClass: *(AudioClassID *)out = kAudioDeviceClassID; return noErr;
        case kAudioObjectPropertyOwner: *(AudioObjectID *)out = kAudioObjectPlugInObject; return noErr;
        case kAudioObjectPropertyName: *(CFStringRef *)out = CF(fPub.name); return noErr;
        case kAudioObjectPropertyManufacturer: *(CFStringRef *)out = CFSTR("OpenAudio"); return noErr;
        case kAudioObjectPropertyOwnedObjects: case kAudioDevicePropertyStreams:
            if (need) *(AudioObjectID *)out = streamID();
            return noErr;
        case kAudioDevicePropertyDeviceUID: *(CFStringRef *)out = CF(fPub.uid); return noErr;
        case kAudioDevicePropertyModelUID: *(CFStringRef *)out = CFSTR("com.openaudio.glue.model"); return noErr;
        case kAudioDevicePropertyTransportType: *(UInt32 *)out = kAudioDeviceTransportTypeVirtual; return noErr;
        case kAudioDevicePropertyRelatedDevices: *(AudioObjectID *)out = fDeviceID; return noErr;
        case kAudioDevicePropertyClockDomain: *(UInt32 *)out = fPub.domain; return noErr;
        case kAudioDevicePropertyDeviceIsAlive: *(UInt32 *)out = fAlive.load() ? 1 : 0; return noErr;
        case kAudioDevicePropertyDeviceIsRunning: *(UInt32 *)out = fIORefs.load() > 0 ? 1 : 0; return noErr;
        case kAudioDevicePropertyDeviceCanBeDefaultDevice: *(UInt32 *)out = input ? 1 : 0; return noErr;
        case kAudioDevicePropertyDeviceCanBeDefaultSystemDevice: *(UInt32 *)out = 0; return noErr;
        case kAudioDevicePropertyLatency: *(UInt32 *)out = input ? fPub.latency : 0; return noErr;
        case kAudioDevicePropertySafetyOffset: *(UInt32 *)out = input ? fPub.safety : 0; return noErr;
        case kAudioObjectPropertyControlList: return noErr;
        case kAudioDevicePropertyNominalSampleRate: *(Float64 *)out = fPub.rate; return noErr;
        case kAudioDevicePropertyAvailableNominalSampleRates: {
            AudioValueRange r = { (Float64)fPub.rate, (Float64)fPub.rate }; *(AudioValueRange *)out = r; return noErr;
        }
        case kAudioDevicePropertyIsHidden: *(UInt32 *)out = 0; return noErr;
        case kAudioDevicePropertyPreferredChannelsForStereo: ((UInt32 *)out)[0] = 1; ((UInt32 *)out)[1] = 2; return noErr;
        case kAudioDevicePropertyZeroTimeStampPeriod: *(UInt32 *)out = kGlueTapRingFrames; return noErr;
        case kAudioObjectPropertyCustomPropertyInfoList: {
            AudioServerPlugInCustomPropertyInfo *i = (AudioServerPlugInCustomPropertyInfo *)out;
            i[0] = { kGluePropertySettings, kAudioServerPlugInCustomPropertyDataTypeCFPropertyList, kAudioServerPlugInCustomPropertyDataTypeNone };
            i[1] = { kGluePropertyMeters, kAudioServerPlugInCustomPropertyDataTypeCFPropertyList, kAudioServerPlugInCustomPropertyDataTypeNone };
            return noErr;
        }
        case kGluePropertySettings: {
            std::lock_guard<std::mutex> g(fSetLock);
            *(CFPropertyListRef *)out = SettingsToPlist(fSettings, fPub.channels);
            return noErr;
        }
        case kGluePropertyMeters: {
            CFMutableDictionaryRef d = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            CFMutableArrayRef gr = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
            CFMutableArrayRef hit = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
            for (UInt32 c = 0; c < fPub.channels; c++) {
                float x = fGr[c].load(), y = fHit[c].load();
                CFNumberRef nx = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &x), ny = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &y);
                CFArrayAppendValue(gr, nx); CFArrayAppendValue(hit, ny); CFRelease(nx); CFRelease(ny);
            }
            CFDictionarySetValue(d, CFSTR("gr"), gr); CFDictionarySetValue(d, CFSTR("hit"), hit); CFRelease(gr); CFRelease(hit);
            CFDictionarySetValue(d, CFSTR("running"), fIORefs.load() > 0 ? kCFBooleanTrue : kCFBooleanFalse);
            *(CFPropertyListRef *)out = d;
            return noErr;
        }
        case kAudioObjectPropertyElementName: {
            const UInt32 c = a.mElement - 1;
            const std::string n = (c < fPub.channelNames.size() && !fPub.channelNames[c].empty()) ? fPub.channelNames[c]
                                                                                                    : "Input " + std::to_string(a.mElement);
            *(CFStringRef *)out = CF(n);
            return noErr;
        }
        default: return kAudioHardwareUnknownPropertyError;
        }
    }
    switch (a.mSelector) {                               // the stream
    case kAudioObjectPropertyBaseClass: *(AudioClassID *)out = kAudioObjectClassID; return noErr;
    case kAudioObjectPropertyClass: *(AudioClassID *)out = kAudioStreamClassID; return noErr;
    case kAudioObjectPropertyOwner: *(AudioObjectID *)out = fDeviceID; return noErr;
    case kAudioObjectPropertyOwnedObjects: return noErr;
    case kAudioStreamPropertyIsActive: *(UInt32 *)out = 1; return noErr;
    case kAudioStreamPropertyDirection: *(UInt32 *)out = 1; return noErr;                  // input
    case kAudioStreamPropertyTerminalType: *(UInt32 *)out = kAudioStreamTerminalTypeLine; return noErr;
    case kAudioStreamPropertyStartingChannel: *(UInt32 *)out = 1; return noErr;
    case kAudioStreamPropertyLatency: *(UInt32 *)out = 0; return noErr;
    case kAudioStreamPropertyVirtualFormat: case kAudioStreamPropertyPhysicalFormat:
        *(AudioStreamBasicDescription *)out = format(); return noErr;
    case kAudioStreamPropertyAvailableVirtualFormats: case kAudioStreamPropertyAvailablePhysicalFormats: {
        AudioStreamRangedDescription r;
        r.mFormat = format();
        r.mSampleRateRange.mMinimum = r.mSampleRateRange.mMaximum = fPub.rate;
        *(AudioStreamRangedDescription *)out = r;
        return noErr;
    }
    default: return kAudioHardwareUnknownPropertyError;
    }
}

OSStatus Device::set(AudioObjectID obj, const AudioObjectPropertyAddress &a, UInt32, const void *, UInt32 inSize, const void *in)
{
    if (!hasProperty(obj, a)) return kAudioHardwareUnknownPropertyError;
    if (obj == fDeviceID && a.mSelector == kGluePropertySettings) {
        if (inSize < sizeof(CFPropertyListRef) || !in) return kAudioHardwareBadPropertySizeError;
        CFPropertyListRef p = *(const CFPropertyListRef *)in;
        {
            std::lock_guard<std::mutex> g(fSetLock);
            if (!SettingsFromPlist(p, fSettings)) return kAudioHardwareIllegalOperationError;
            fSettings.version++;
            fBox.write(fSettings);
            fSettingsVersion.store(fSettings.version, std::memory_order_release);
            saveSettings();
        }
        const AudioObjectPropertyAddress changed = { kGluePropertySettings, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        fHost->PropertiesChanged(fHost, fDeviceID, 1, &changed);
        return noErr;
    }
    if (obj == fDeviceID && a.mSelector == kAudioDevicePropertyNominalSampleRate && inSize >= sizeof(Float64) &&
        *(const Float64 *)in == (Float64)fPub.rate)
        return noErr;                                    // the rate follows the unit; asking for the current one is fine
    return kAudioHardwareUnsupportedOperationError;
}

// ---- IO ----------------------------------------------------------------------------------------------------------------------

OSStatus Device::startIO()
{
    if (!fAlive.load()) return kAudioHardwareBadDeviceError;
    if (fIORefs.fetch_add(1) == 0) {
        TapClient *t = fTap.load();
        if (t) t->start();                               // one isoch reference (selector 508): the streams run without the unit's device
        fRealTimeline = false;
        fSynthAnchor = mach_absolute_time();
        fSeed++;
        fResetDSP.store(true);
    }
    return noErr;
}

OSStatus Device::stopIO()
{
    const int was = fIORefs.fetch_sub(1);
    if (was <= 0) { fIORefs.store(0); return noErr; }
    if (was == 1) { TapClient *t = fTap.load(); if (t) t->stop(); }
    return noErr;
}

bool Device::readWrap(const GlueTapPage *g, uint64_t &count, uint64_t &host)
{
    for (int tries = 0; tries < 8; tries++) {
        const uint32_t a = g->seq;
        if (a & 1) continue;
        std::atomic_thread_fence(std::memory_order_acquire);
        count = g->wrapCount;
        host = g->wrapHostTime;
        std::atomic_thread_fence(std::memory_order_acquire);
        if (g->seq == a) return true;
    }
    return false;
}

OSStatus Device::zeroTimeStamp(Float64 &sampleTime, UInt64 &hostTime, UInt64 &seed)
{
    TapClient *t = fTap.load();
    const GlueTapPage *g = t ? t->page() : nullptr;
    uint64_t count = 0, host = 0;
    const bool real = fAlive.load() && g && g->state == kGlueTapRunning && readWrap(g, count, host) && count > 0;
    if (real) {
        if (!fRealTimeline || count < fLastWrap) { fRealTimeline = true; fSeed++; }   // new timeline: Core Audio resyncs on the seed
        fLastWrap = count;
        sampleTime = (Float64)count * kGlueTapRingFrames;
        hostTime = host;
    } else {
        // Not locked yet (the first two TX laps take no timestamp) or not running: the nominal rate on the host clock, silence.
        const UInt64 now = mach_absolute_time();
        if (fRealTimeline) { fRealTimeline = false; fSeed++; fSynthAnchor = now; }
        const double period = fHostTicksPerFrame * kGlueTapRingFrames;
        const double n = period > 0 ? std::floor((double)(now - fSynthAnchor) / period) : 0;
        sampleTime = n * kGlueTapRingFrames;
        hostTime = fSynthAnchor + (UInt64)(n * period);
    }
    seed = fSeed;
    return noErr;
}

// FWBaseEngine::ConvertInputSamplesCore's rule: frames [first, first + num) have arrived when the RX write position is past them,
// comparing modulo the 8192-frame ring.
bool Device::enoughInput(const GlueTapPage *g, UInt32 first, UInt32 num)
{
    const uint32_t frameBytes = g->frameQuads * 4;
    if (frameBytes == 0) return false;
    const uint32_t written = g->writeOff / frameBytes;
    const uint32_t last = first - 1 + num;
    const int32_t d = (int32_t)(written - last);
    int32_t alt, mag;
    if (last < written) { alt = d - 0x2000; mag = d; }
    else                { alt = d + 0x2000; mag = -d; }
    if (mag < 0x801) alt = d;
    return alt >= 0;
}

void Device::ChannelPart(void *ctx, int channel)
{
    const Job *j = static_cast<const Job *>(ctx);
    j->dev->processChannel(*j, channel);
}

void Device::unpack(const Job &j, int c)
{
    float *s = fScratch + (size_t)c * kGlueTapRingFrames;
    const GlueTapChannel m = j.page->map[c];
    const uint32_t fq = j.page->frameQuads;
    for (UInt32 i = 0; i < j.frames; i++) {
        const uint32_t frame = (j.first + i - m.delay) & (kGlueTapRingFrames - 1);
        // FWInt24ScaledToFloat32: the same multiply as the kext, so a bypassed channel is bit-identical to the unit's device
        s[i] = (float)UnpackAM824(j.ring[(size_t)frame * fq + m.slot]) * 1.1920928955078125e-07f;
    }
}

void Device::store(const Job &j, int c)
{
    const float *s = fScratch + (size_t)c * kGlueTapRingFrames;
    const UInt32 ch = j.channels;
    for (UInt32 i = 0; i < j.frames; i++) j.out[(size_t)(j.outOffset + i) * ch + (UInt32)c] = s[i];
    fGr[c].store(fChain[c].gainReductionDb(), std::memory_order_relaxed);
    fHit[c].store(fChain[c].hitLiftDb(), std::memory_order_relaxed);
}

void Device::processChannel(const Job &j, int c)
{
    if (c > 0 && fLinked[c - 1]) return;                 // the follower of a linked pair: its leader's part does it
    unpack(j, c);
    float *s = fScratch + (size_t)c * kGlueTapRingFrames;
    if (fLinked[c]) {
        unpack(j, c + 1);
        float *t = fScratch + (size_t)(c + 1) * kGlueTapRingFrames;
        glue::ChannelChain::processPair(fChain[c], fChain[c + 1], s, t, s, t, (int)j.frames);
        store(j, c);
        store(j, c + 1);
        return;
    }
    fChain[c].process(s, s, (int)j.frames);             // bypassed and switched-off channels: exact values, delay-compensated
    store(j, c);
}

void Device::readInput(Float64 sampleTime, UInt32 frames, float *out)
{
    const UInt32 ch = fPub.channels;
    TapClient *t = fTap.load();
    const GlueTapPage *g = t ? t->page() : nullptr;
    const RingView *ring = t ? t->ring() : nullptr;
    const bool ok = fAlive.load() && fRealTimeline && g && ring && fPresets && g->state == kGlueTapRunning && g->inChannels == ch &&
                    (uint64_t)g->frameQuads * 4 * kGlueTapRingFrames <= ring->bytes && ch > 0 && ch <= kGlueTapMaxChannels;
    if (!ok) { memset(out, 0, (size_t)frames * ch * sizeof(float)); return; }
    if (fResetDSP.exchange(false)) {
        for (UInt32 c = 0; c < kGlueTapMaxChannels; c++) fChain[c].reset(fPresets, c + 1);
        applySettingsOnIOThread(true);
    } else {
        applySettingsOnIOThread(false);
    }
    const int64_t t0 = (int64_t)llround(sampleTime);
    UInt32 done = 0;
    while (done < frames) {
        const UInt32 first = (UInt32)((uint64_t)(t0 + done) & (kGlueTapRingFrames - 1));
        UInt32 seg = frames - done;
        if (seg > kGlueTapRingFrames - first) seg = kGlueTapRingFrames - first;   // the kext converts up to the ring's end too
        if (!enoughInput(g, first, seg)) {
            memset(out + (size_t)done * ch, 0, (size_t)seg * ch * sizeof(float));
        } else {
            Job j = { this, first, seg, done, ch, out, ring->base, g };
            fWorkers->run((int)ch, &ChannelPart, &j);
        }
        done += seg;
    }
}

} // namespace gluedrv
