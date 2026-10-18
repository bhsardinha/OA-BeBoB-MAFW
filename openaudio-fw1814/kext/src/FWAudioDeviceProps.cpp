// FWAudioDevice: initHardware flow, property dictionary glue, meters, rate-lock detection, LatencyProfiler handshake.
// Mirrors m_audio_b_FWAudioDevice::{initHardware @0x342e, stop, free, InitializeUserSettableProperties, ResetMixer @0x2660,
// SetDeviceUserLabel @0x269e, SetDeviceLabel @0x28aa, SetStaticPropertyAndNotifyUserClients @0x2928, ClearPeakMeters,
// DetectExternalSyncChanges, getProperty/copyProperty/setProperties} (A1 Q-ME §1/§5/§6/§7, Q3b §1).
#include "FWAudioDeviceChain.h"
#include "FWPropertyDict.h"
#include "FWBaseEngine.h"
#include "FWMetaNub.h"
#include "Trace.h"
#include <libkern/c++/OSString.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSData.h>
#include <libkern/libkern.h>

static const IOReturn kErrBadArg = (IOReturn)0xE00002C2;
static const IOReturn kErrNoMem  = (IOReturn)0xE00002BD;

// ---- LatencyProfiler handshake (ref DigiHandshake::Do + HandleDigiHandshakeProperty, decomp DigiHandshake__*.c) --------------------------
namespace {
struct DigiHandshake {
    static int sStage; static int s1, s2, s3;
    static int Do(int p)
    {
        if (p != 0) {
            switch (sStage) {
                case 0: break;
                case 1: {
                    sStage = 2;
                    s2 = p / 0x1f31d;
                    int v = (p % 0x1f31d) * 0x41a7;
                    s1 = v ^ 0x41a7;
                    return v;
                }
                case 2:
                    sStage = 3;
                    s3 = p + 0x496143 + (p / 0x2cf535) * -0x2cf535;
                    s2 = s2 * 0xb14;
                    return s2 % 0xb14;
                case 3: {
                    sStage = 4;
                    int v = p - s2;
                    if (v < 0) v += 0x7fffffff;
                    s2 ^= v;
                    s3 = v % 0x1f31d;
                    return v;
                }
                case 4:
                    sStage = 0;
                    s2 = s1 % 0x41a7;
                    s3 = (s1 % 0x41a7) % 0x1f31d;
                    return p + 0x496143 + (p / 0x2cf535) * -0x2cf535;
                default: return 0;
            }
        }
        s3 = 0; sStage = 1; s2 = 0; s1 = 0;
        return 0;
    }
};
int DigiHandshake::sStage = 0; int DigiHandshake::s1 = 0; int DigiHandshake::s2 = 0; int DigiHandshake::s3 = 0;
}

// ref HandleDigiHandshakeProperty: key "LatencyProfiler<decimal>" -> OSNumber(Do(n)); returns +1 or NULL
static OSObject *HandleDigiHandshakeProperty(const char *key)
{
    static const char kPrefix[] = "LatencyProfiler";
    const size_t n = sizeof(kPrefix) - 1;
    if (!key || strncmp(key, kPrefix, n) != 0) return 0;
    UInt64 result = 0;
    if (key[n] != '\0') {
        long v = 0; bool neg = false; const char *p = key + n;
        if (*p == '-') { neg = true; p++; }
        while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); p++; }
        result = (UInt64)(UInt32)DigiHandshake::Do((int)(neg ? -v : v));
    }
    return OSNumber::withNumber(result, 32);
}

OSObject *com_openaudio_FWAudioDevice::nubProperty(const char *key) const
{
    IOFireWireNub *n = fMetaNub ? fMetaNub->fireWireNub() : 0;
    return n ? n->getProperty(key) : 0;
}

// ---- property surface -------------------------------------------------------------------------------------------------------------

OSObject *com_openaudio_FWAudioDevice::getProperty(const char *aKey) const
{
    OSObject *o = HandleDigiHandshakeProperty(aKey);
    if (!o && fPropDict) o = fPropDict->CopyProperty(aKey);
    if (!o) return IOAudioDevice::getProperty(aKey);
    // the reference returns the object without keeping a reference (and leaks one); the clone parks it, replacing the previous one per key
    com_openaudio_FWAudioDevice *self = const_cast<com_openaudio_FWAudioDevice *>(this);
    if (!self->fGetCache) self->fGetCache = OSDictionary::withCapacity(8);
    if (self->fGetCache) self->fGetCache->setObject(aKey, o);
    OSObject *keep = self->fGetCache ? self->fGetCache->getObject(aKey) : 0;
    o->release();
    return keep;
}

OSObject *com_openaudio_FWAudioDevice::copyProperty(const char *aKey) const
{
    if (aKey && !strcmp(aKey, "MADebugTxStats")) {                              // bench aid, built on demand
        char buf[900] = "no engine";
        if (fMainEngine && fMainEngine->txProgram()) {
            fMainEngine->txProgram()->TxStatsString(buf, 400);
            size_t l = strlen(buf); if (l + 2 < sizeof(buf)) { buf[l++] = '\n'; fMainEngine->ClipStatsString(buf + l, sizeof(buf) - l); }
        }
        return OSString::withCString(buf);
    }
    OSObject *o = HandleDigiHandshakeProperty(aKey);
    if (!o && fPropDict) o = fPropDict->CopyProperty(aKey);
    if (o) return o;
    return IOAudioDevice::copyProperty(aKey);
}

// ref FWAudioDevice::setProperties / FWAVCAudioDevice::setProperties: a "MASampleRate" different from the engine rate is applied first and
// synchronously (vt[0xa78] RequestFormatUpdateForSampleRate), then the dictionary dispatches the rest.
IOReturn com_openaudio_FWAudioDevice::setProperties(OSObject *properties)
{
    if (fPropDict && fMainEngine) {
        OSDictionary *d = OSDynamicCast(OSDictionary, properties);
        if (!d) return kIOReturnSuccess;                                       // ref FWAVCAudioDevice: non-dictionary -> 0
        OSNumber *rate = OSDynamicCast(OSNumber, d->getObject("MASampleRate"));
        if (rate && fMainEngine->rateHz() != rate->unsigned32BitValue()) {
            // Clone addition (M-O): the reference applies the rate whatever the engine is doing, and the v1.9 reference panics when the panel
            // changes the rate while a client streams (IOAudioFamily buffer reallocation, memory project_v19_rate_change_while_streaming_panics).
            // A running or paused engine keeps its rate; the current rate is published back so the panel reverts its control. CoreAudio's own
            // format changes go through IOAudioEngine::performFormatChange (the client stops first) and are not affected. Only a running engine with
            // active I/O clients blocks (an engine left "running" with no client must not lock the rate, seen on the 2026-10-16 run).
            if (fMainEngine->getState() != kIOAudioEngineStopped && fMainEngine->ActiveUserClients() != 0) {
                OA_LOG("MASampleRate %u ignored while the audio engine is running (rate stays %u)",
                       (unsigned)rate->unsigned32BitValue(), (unsigned)fMainEngine->rateHz());
                SetStaticPropertyAndNotifyUserClients("MASampleRate", fMainEngine->rateHz());
            } else {
                RequestFormatUpdateForSampleRate(rate->unsigned32BitValue());
            }
        }
    }
    if (fPropDict) return fPropDict->SetProperties(properties);
    return IOAudioDevice::setProperties(properties);
}

// ref @0x2928: registry property + queued notification
void com_openaudio_FWAudioDevice::SetStaticPropertyAndNotifyUserClients(const char *key, UInt32 value)
{
    if (!fPropDict) return;
    OSNumber *n = OSNumber::withNumber((UInt64)value, 32);
    if (!n) return;
    setProperty(key, n);
    fPropDict->SetPropertyAndNotifyUserClients(key, n);
    n->release();
}

// ---- device-level properties ------------------------------------------------------------------------------------------------------

IOReturn com_openaudio_FWAudioDevice::ResetMixer(OSObject *)
{
    ResetToFactorySettings();                                                 // ref vt[0xb08]
    ResendCurrentMixerSettings();                                             // ref vt[0xb00]
    if (fPropDict) fPropDict->SendAllPropertiesToUserClients();
    return kIOReturnSuccess;
}

// ref SetDeviceLabel @0x28aa: "<description> (<label>)" handed to the engine as its description
IOReturn com_openaudio_FWAudioDevice::SetDeviceLabel(const char *label)
{
    char buf[0x100];
    snprintf(buf, sizeof(buf), "%s (%s)", GetDescriptionString(), label);
    if (fMainEngine) fMainEngine->setDescription(buf);                         // ref FWBaseEngine::SetLabel -> IOAudioEngine::setDescription (vt[0x9f8])
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWAudioDevice::SetDeviceUserLabel(OSObject *v)
{
    OSString *s = OSDynamicCast(OSString, v);
    if (!s) return kErrBadArg;
    IOReturn r = SetDeviceLabel(s->getCStringNoCopy());
    if (r == kIOReturnSuccess && fPropDict) fPropDict->SetPropertyAndNotifyUserClients("MADeviceUserLabel", s);
    return r;
}

// ref @0x...: GUID, the property dictionary and the generic device properties
IOReturn com_openaudio_FWAudioDevice::InitializeUserSettableProperties()
{
    OSObject *guid = nubProperty("GUID");
    setProperty("GUID", guid);
    fPropDict = OSTypeAlloc(com_openaudio_FWPropertyDict);
    if (!fPropDict || !fPropDict->InitWithDevice(this)) { if (fPropDict) { fPropDict->release(); fPropDict = 0; } return kErrNoMem; }
    typedef com_openaudio_FWAudioDevice D;
    if (!fPropDict->AddNumber32("MAEnableStereoPairEngines", FWBind1<D, &D::SetExportStereoPairEngines>(this))) return kErrNoMem;
    if (!fPropDict->AddNumber32("MAEnableAggregateEngine", FWBind1<D, &D::SetExportAggregateEngine>(this))) return kErrNoMem;
    if (!fPropDict->AddFunction("MAMetaResetToFactorySettings", FWBindObj<D, &D::ResetMixer>(this))) return kErrNoMem;
    if (!fPropDict->AddFunction("MADeviceUserLabel", FWBindObj<D, &D::SetDeviceUserLabel>(this))) return kErrNoMem;
    return kIOReturnSuccess;
}

// ---- meters -----------------------------------------------------------------------------------------------------------------------

void com_openaudio_FWAudioDevice::ClearPeakMeters()
{
    if (!fMeters) return;
    bzero(fMeters, (size_t)GetPeakLevelCount() * 4);
    if (fSwPeaks) bzero(fSwPeaks, (size_t)GetPeakLevelCount() * 4);
}

// ref DetectExternalSyncChanges (decomp FWAudioDevice__DetectExternalSyncChanges.c)
void com_openaudio_FWAudioDevice::DetectExternalSyncChanges(bool locked, UInt32 fdf)
{
    if (fDetectedFdf != fdf) {
        fDetectedFdf = fdf;
        UInt32 rate;
        switch (fdf) {
            case 0: rate = 32000; break;
            case 1: rate = 44100; break;
            case 2: rate = 48000; break;
            case 3: rate = 88200; break;
            case 4: rate = 96000; break;
            case 5: rate = 176400; break;
            case 6: rate = 192000; break;
            default:
                IOLog("M-FW: invalid sample rate detected (FDF:%X)%s\n", (unsigned)fdf, locked ? " mismatch" : "");
                rate = fMainEngine ? fMainEngine->rateHz() : 0;
                fDetectedFdf = 0xff;
                break;
        }
        SetStaticPropertyAndNotifyUserClients("MASampleRate", rate);
    }
    if (locked && fMainEngine && fDetectedFdf != fMainEngine->fdfIndex()) locked = false;
    if (locked != fRateLocked) {
        fRateLocked = locked;
        SetStaticPropertyAndNotifyUserClients("MARateLocked", locked ? 1 : 0);
    }
}

void com_openaudio_FWAudioDevice::DebugTrace(const char *msg)
{
    if (!fTrace) { fTrace = OSArray::withCapacity(16); if (!fTrace) return; }
    if (fTrace->getCount() >= 16) fTrace->removeObject(0);
    if (OSString *s = OSString::withCString(msg)) { fTrace->setObject(s); s->release(); }
    // A collection that is published as a registry property is frozen (changing it panics: "Trying to change a collection in the registry",
    // caused by the first version of this function, 2026-10-17 12:12), so the ring stays private and a fresh copy is published.
    if (OSArray *pub = OSArray::withArray(fTrace)) { setProperty("MADebugTrace", pub); pub->release(); }
}

// ---- initHardware / stop / free ---------------------------------------------------------------------------------------------------

// ref FWAudioDevice::initHardware @0x342e (Q3b §1)
bool com_openaudio_FWAudioDevice::initHardware(IOService *provider)
{
    fReady = false;
    setProperty("IOAudioDeviceConfigurationApplication", "com.m-audio.appc.MAfw");
    if (!com_openaudio_FWDiscreteDevice::initHardware(provider)) return false;
    setDeviceName(GetDescriptionString());
    setDeviceShortName(GetShortDescriptionString());
    setManufacturerName("M-Audio");
    setDeviceTransportType(kIOAudioDeviceTransportTypeFireWire);              // ref vt[0x850](0x31333934 = '1394')
    if (OSObject *prod = nubProperty("FireWire Product Name")) setProperty("MAIconName", prod);
    setProperty("MAMaxBytesPerCycle", GetMaxBandwidthBytesPerCycle() != 0);

    bool ok = AllocateConnectionManager() && createAudioEngine();             // ref [+0xf0] = vt[0xa28]; vt[0xa30]
    if (ok) {
        RequestFormatUpdateForSampleRate(fMainEngine->rateHz());              // ref vt[0xa78]
        ok = (fMainEngine->CompletePendingStateChanges() == kIOReturnSuccess)
          && (InitializeUserSettableProperties() == kIOReturnSuccess);
    }
    if (ok && !fMeterMem) {
        fMeterMem = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared | kIODirectionInOut, 0x1000, 0x1000);
        if (fMeterMem && fMeterMem->prepare(kIODirectionIn) == kIOReturnSuccess) {
            fMeters = static_cast<UInt32 *>(fMeterMem->getBytesNoCopy());
            fSwPeaks = static_cast<UInt32 *>(IOMalloc((size_t)GetPeakLevelCount() * 4));
            ok = (fMeters && fSwPeaks);
            if (ok) bzero(fSwPeaks, (size_t)GetPeakLevelCount() * 4);
        } else {
            ok = false;
        }
    }
    if (ok) {
        fRateLocked = true;                                                   // ref [+0x25a] = 1
        SetStaticPropertyAndNotifyUserClients("MARateLocked", 1);
        ClearPeakMeters();
        ResetToFactorySettings();                                             // ref vt[0xb08]
        fMainEngine->ReallyRegisterService();                                 // ref engine vt[0xc70]
        fReady = true;                                                        // ref [+0x258] = 1
        return true;
    }
    OA_LOG("FWAudioDevice::initHardware failed");
    return false;
}

void com_openaudio_FWAudioDevice::stop(IOService *provider)
{
    if (fPropDict) { fPropDict->Detach(); fPropDict->release(); fPropDict = 0; }
    if (fReady) IOAudioDevice::stop(provider);
    fReady = false;
}

void com_openaudio_FWAudioDevice::free()
{
    if (fMeterMem) { fMeterMem->complete(kIODirectionIn); fMeterMem->release(); fMeterMem = 0; fMeters = 0; }
    if (fSwPeaks)  { IOFree(fSwPeaks, (size_t)GetPeakLevelCount() * 4); fSwPeaks = 0; }
    if (fGetCache) { fGetCache->release(); fGetCache = 0; }
    if (fTrace)    { fTrace->release(); fTrace = 0; }
    if (fPropDict) { fPropDict->release(); fPropDict = 0; }
    com_openaudio_FWDiscreteDevice::free();
}

// ---- channel names ---------------------------------------------------------------------------------------------------------------

// ref AddChannelToDictionaries (decomp AddChannelToDictionaries.c): key "<Dir>Channel<n>"; names "<Dir> <Type> <k>", "<Dir> <Type>", "<k>"
static void AddChannelToDictionaries(OSDictionary *names, OSDictionary *categories, OSDictionary *numbers, UInt32 channel, UInt32 k,
                                     const char *dir, const char *type)
{
    char key[32], cat[64], num[32], full[96];
    snprintf(key, sizeof(key), "%sChannel%u", dir, (unsigned)channel);
    snprintf(cat, sizeof(cat), "%s %s", dir, type);
    snprintf(num, sizeof(num), "%d", (int)k);
    snprintf(full, sizeof(full), "%s %s", cat, num);
    if (OSString *s = OSString::withCString(full)) { names->setObject(key, s); s->release(); }
    if (OSString *s = OSString::withCString(cat))  { categories->setObject(key, s); s->release(); }
    if (OSString *s = OSString::withCString(num))  { numbers->setObject(key, s); s->release(); }
}

// ref SetChannelNamesForMainEngine @0x2d2a: names every channel of the current DeviceChannels row ("Input Analog 1" ... "Output ADAT 8")
void com_openaudio_FWAudioDevice::SetChannelNamesForMainEngine(IOAudioEngine *engine)
{
    FWDeviceChannels row;
    GetDeviceChannelsForSampleRate(fMainEngine ? fMainEngine->rateHz() : 44100, row);
    const UInt32 cap = row.outStream + row.inStream;
    const bool smux = fMainEngine && fMainEngine->rateHz() >= 88200;           // ref: [engine+0x8c] >= 0x15888
    OSDictionary *names = OSDictionary::withCapacity(cap), *cats = OSDictionary::withCapacity(cap), *nums = OSDictionary::withCapacity(cap);
    if (names && cats && nums) {
        UInt32 counters[6] = { 0, 0, 0, 0, 0, 0 };                              // [analog in, analog out, ADAT in, ADAT out, S/PDIF in, S/PDIF out]
        for (UInt32 g = 0; g < 5 && row.group[g][3] != 3; g++) {
            const bool input = row.group[g][0] != 0;
            const UInt32 type = row.group[g][3];
            const char *dir = input ? "Input" : "Output";
            const char *tname = (type == 1) ? "S/PDIF" : (type == 2) ? (smux ? "ADAT SMUX" : "ADAT") : "Analog";
            UInt32 *ctr = &counters[(input ? 0 : 1) + (type == 1 ? 4 : type == 2 ? 2 : 0)];
            for (UInt32 pair = row.group[g][1]; pair <= row.group[g][2]; pair++) {
                AddChannelToDictionaries(names, cats, nums, pair * 2 - 1, ++*ctr, dir, tname);
                AddChannelToDictionaries(names, cats, nums, pair * 2,     ++*ctr, dir, tname);
            }
        }
        engine->setProperty("IOAudioEngineChannelNames", names);
        engine->setProperty("IOAudioEngineChannelCategoryNames", cats);
        engine->setProperty("IOAudioEngineChannelNumberNames", nums);
    }
    if (names) names->release();
    if (cats)  cats->release();
    if (nums)  nums->release();
}
