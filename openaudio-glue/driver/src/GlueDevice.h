// OpenAudio Glue driver - one Glue device (one unit): its Core Audio properties, its timeline and its IO.
// The device carries the unit's inputs only. Timeline: sample time = wrapCount * 8192 + ring frame, at the wrap times the kext
// stamps (GlueTap.h), so a frame has the same sample time here as in the unit's own Core Audio device; ReadInput reads the ring at
// those frames with the kext's underrun rule (FWBaseEngine::ConvertInputSamplesCore), the model's map and the same int->float
// multiply (FWInt24ScaledToFloat32), then runs each channel through glue::ChannelChain.
#pragma once
#include <CoreAudio/AudioServerPlugIn.h>
#include <atomic>
#include <mutex>
#include <string>
#include "GlueSettings.h"
#include "GlueTapClient.h"
#include "GlueWorkers.h"

namespace gluedrv {

// Custom properties (CFPropertyList): settings (settable) and meters (read-only).
enum : AudioObjectPropertySelector { kGluePropertySettings = 'glst', kGluePropertyMeters = 'glmt' };

class Device {
public:
    Device(AudioObjectID deviceID, io_service_t service, AudioServerPlugInHostRef host, Workers *workers);
    ~Device();

    AudioObjectID deviceID() const { return fDeviceID; }
    AudioObjectID streamID() const { return fDeviceID + 1; }
    io_service_t service() const { return fService; }
    bool published() const { return fPublished.load(); }
    bool alive() const { return fAlive.load(); }
    bool listed() const { return fPublished.load() && fAlive.load(); }   // in the plug-in's device list
    const std::string &uid() const { return fPub.uid; }

    // Monitor (the plug-in's serial queue, every 50 ms): open the tap, publish once geometry is valid, follow changes.
    // Returns true when the device became visible (the plug-in then announces its device list).
    bool poll();
    void retire();                                      // the unit went away
    void performConfigurationChange();                  // host-approved: take the new rate / channel count

    // Properties (HAL threads)
    bool hasProperty(AudioObjectID obj, const AudioObjectPropertyAddress &a) const;
    OSStatus isSettable(AudioObjectID obj, const AudioObjectPropertyAddress &a, Boolean &out) const;
    OSStatus getSize(AudioObjectID obj, const AudioObjectPropertyAddress &a, UInt32 qs, const void *q, UInt32 &out) const;
    OSStatus get(AudioObjectID obj, const AudioObjectPropertyAddress &a, UInt32 qs, const void *q, UInt32 inSize, UInt32 &outSize, void *out) const;
    OSStatus set(AudioObjectID obj, const AudioObjectPropertyAddress &a, UInt32 qs, const void *q, UInt32 inSize, const void *in);

    // Host tests (driver/tests): take a test tap and registry info instead of the unit's, and publish.
    void attachForTest(TapClient *tap, const UnitInfo &info)
    {
        fTap.store(tap);
        fInfo = info;
        fSeenGeneration = tap->page()->generation;
        takeGeometry();
        loadSettings();
        fPublished.store(true);
    }

    // IO (the IO thread)
    OSStatus startIO();
    OSStatus stopIO();
    OSStatus zeroTimeStamp(Float64 &sampleTime, UInt64 &hostTime, UInt64 &seed);
    void readInput(Float64 sampleTime, UInt32 frames, float *out);

private:
    struct Published {                                   // what Core Audio was told; changed only at publish / configuration change
        UInt32 rate = 0, channels = 0, latency = 0, safety = 0, domain = 0;
        std::string name, uid;
        std::vector<std::string> channelNames;
    };
    struct Job { Device *dev; UInt32 first, frames, outOffset, channels; float *out; const uint32_t *ring; const GlueTapPage *page; };
    struct RetiredTap { TapClient *tap; uint64_t at; };
    static void ChannelPart(void *ctx, int channel);
    void processChannel(const Job &j, int c);
    void unpack(const Job &j, int c);
    void store(const Job &j, int c);
    static bool readWrap(const GlueTapPage *g, uint64_t &count, uint64_t &host);
    static bool enoughInput(const GlueTapPage *g, UInt32 first, UInt32 num);
    bool pollLocked(bool &request);
    void takeGeometry();                                 // page + registry -> fPub, presets for the rate (monitor queue)
    void reapTaps(bool all);
    void loadSettings();                                 // host storage -> fSettings (publish time)
    void saveSettings();                                 // fSettings -> host storage (fSetLock held)
    CFStringRef storageKey() const;
    void applySettingsOnIOThread(bool force);
    AudioStreamBasicDescription format() const;

    const AudioObjectID fDeviceID;
    io_service_t fService;
    AudioServerPlugInHostRef fHost;
    Workers *fWorkers;
    std::atomic<TapClient *> fTap{ nullptr };            // replaced when the engine is re-published; old ones kept 2 s
    std::vector<RetiredTap> fRetiredTaps;                // monitor queue only
    UnitInfo fInfo;
    Published fPub;
    std::atomic<bool> fPublished{ false };
    std::atomic<bool> fAlive{ true };
    std::atomic<bool> fConfigPending{ false };
    uint32_t fSeenGeneration = 0;

    // IO state (IO thread only, except where atomic)
    std::atomic<int> fIORefs{ 0 };
    std::atomic<bool> fResetDSP{ true };
    bool fRealTimeline = false;
    uint64_t fLastWrap = 0;
    UInt64 fSeed = 1;
    UInt64 fSynthAnchor = 0;
    double fHostTicksPerFrame = 0;
    glue::ChannelChain fChain[kGlueTapMaxChannels];
    const glue::Presets *fPresets = nullptr;
    float *fScratch = nullptr;                           // kGlueTapMaxChannels x kGlueTapRingFrames
    UnitSettings fApplied;                               // what the chains run with
    bool fLinked[kGlueTapMaxChannels] = {};               // channel c (even) and c + 1 run as a stereo-linked pair
    uint32_t fAppliedVersion = 0xffffffff;
    std::atomic<float> fGr[kGlueTapMaxChannels];
    std::atomic<float> fHit[kGlueTapMaxChannels];

    // settings (property threads)
    std::recursive_mutex fMonLock;                       // poll() vs performConfigurationChange() (the host may call it from anywhere)
    mutable std::mutex fSetLock;
    UnitSettings fSettings;
    SettingsBox fBox;
    std::atomic<uint32_t> fSettingsVersion{ 0 };
};

} // namespace gluedrv
