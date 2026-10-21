// FWAudioDevice module (base) -- reference m_audio_b_FWDiscreteDevice + m_audio_b_FWAudioDevice (A1 Q3b, Q-ME, Q-MD, Q-MF).
//
// What the reference does in this layer (model-independent):
//  * initHardware @0x342e: config-app property, IOAudioDevice::initHardware, names from the model (GetDescriptionString / GetShortDescription
//    String / GetIconName), transport '1394', MAIconName, MAMaxBytesPerCycle, connection manager (vt AllocateConnectionManager), main engine
//    (createAudioEngine @0x2fd8 = AllocateAudioEngine + SetupAudioEngine, both MODEL virtuals), RequestFormatUpdateForSampleRate,
//    InitializeUserSettableProperties (property dictionary; the model adds its mixer), the 4 KiB shared meter page (52 slots) + host peaks,
//    MARateLocked, ClearPeakMeters, ResetToFactorySettings, engine registration.
//  * Per-rate data comes from the MODEL: GetDeviceChannelsForSampleRate (channel counts, rate list, signal-type groups), latency
//    (GetRoundTripLatencyForFDF, per revision/model), meter layout (GetFirstSWReturnPeakMeterIndex per model; SW-return count = out channels).
//  * Property dictionary (setProperties / copyProperty / getProperty) is the panel's control plane (MA* keys, Q15b); external sync detection
//    (DetectExternalSyncChanges: FDF/lock from the model's status packet -> MASampleRate / MARateLocked); stereo-pair sub-engines; power
//    (performPowerStateChange -> SystemWillSleep / SystemWokeUp of the AV/C layer).
// A model never edits this module; it overrides the virtuals marked "per model".
#ifndef OA_FWAUDIODEVICE_H
#define OA_FWAUDIODEVICE_H

#include <IOKit/audio/IOAudioDevice.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <libkern/c++/OSDictionary.h>
#include "FWRates.h"

class com_openaudio_FWMetaNub;
class IOFireWireNub;
class com_openaudio_FWAvcUnit;
class com_openaudio_FWAvcConnectionManager;
class com_openaudio_FWBaseEngine;
class com_openaudio_FWPropertyDict;
class com_openaudio_FWSettingsLevels;
class com_openaudio_FWSettingsAuxSend;
class com_openaudio_FWSettingsRotaryControl;
class com_openaudio_FWSettingsABSwitch;
class com_openaudio_FWNumber32Prop;
class com_openaudio_FWHsci;

class com_openaudio_FWDiscreteDevice : public IOAudioDevice
{
    OSDeclareAbstractStructors(com_openaudio_FWDiscreteDevice)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    // ref FWDiscreteDevice::audioEngineStarting @0x1e88e / audioEngineStopped @0x1e8a0 (18-byte forwards to IOAudioDevice's);
    // public here because the sub-engine activation calls them from outside IOAudioDevice's friend list.
    virtual void audioEngineStarting() APPLE_KEXT_OVERRIDE { IOAudioDevice::audioEngineStarting(); }
    virtual void audioEngineStopped() APPLE_KEXT_OVERRIDE  { IOAudioDevice::audioEngineStopped(); }
};

class com_openaudio_FWAudioDevice : public com_openaudio_FWDiscreteDevice
{
    OSDeclareAbstractStructors(com_openaudio_FWAudioDevice)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;

    // ---- M-D: per-rate device parameters (Q10, Q-P4 Part A) ----
    // ref vt[0xa68] GetDeviceChannelsForSampleRate: per model only (FW1814 @0xc998, Audiophile @0xafb4); no base body in the reference
    virtual void GetDeviceChannelsForSampleRate(UInt32 rateHz, FWDeviceChannels &out) const = 0;
    // ref vt[0xab8] (FW1814Device @0xcf30) and vt[0xac0] (FWAudioDevice @0x1d58)
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const = 0;          // per revision / model (Rev1 @0x136da, Rev2 @0x12a54, FW1814 @0xcf30)
    virtual UInt32 GetSafetyOffsetForFDF(UInt32 fdf) const    { return FWSafetyOffsetForFDF(fdf); }
    // ref vt[0xb30] FW1814Device::GetMaxBandwidthBytesPerCycle @0xc364 = 0x928 (2344); the min with 0x186a is Maximum...Supported
    virtual UInt32 GetMaxBandwidthBytesPerCycle() const { return 0x928; }
    // ref GetBandwidthBytesAtSampleRate @0x1ee2
    UInt32 GetBandwidthBytesAtSampleRate(UInt32 rateHz) const;
    // ref RequestFormatUpdateForSampleRate @0x2a84 (vt[0xa78]): table lookup then FWBaseEngine::SetIOFormat
    IOReturn RequestFormatUpdateForSampleRate(UInt32 rateHz);
    // ref SetStaticPropertyAndNotifyUserClients @0x2928: property + user-client notification (M-F); until then only the log line.
    virtual void SetStaticPropertyAndNotifyUserClients(const char *key, UInt32 value);
    // Clone bench aid (2026-10-17): the kernel log of the build PC drops this kext's lines, so format-change events are kept as a ring of the last 16
    // strings in the registry property `MADebugTrace` (read with tools/maprop get MADebugTrace).
    void DebugTrace(const char *msg);
    // ref vt[0xb50] (Rev1: plain 8-byte 0x18/0x19). Implemented where the AV/C unit lives (FWAVCAudioDevice).
    virtual IOReturn AVCControlPlugSignalFormat(bool input, UInt8 plug, UInt8 fdf) { (void)input; (void)plug; (void)fdf; return kIOReturnUnsupported; }

    // ref createAudioEngine @0x2fd8 / AllocateAudioEngine (vt[0xa20]) / SetupAudioEngine (vt[0xa38]): build the main engine, let the
    // device configure it, activate it with IOAudioDevice, remember it as [+0xe8].
    virtual com_openaudio_FWBaseEngine *AllocateAudioEngine() = 0;
    virtual void SetupAudioEngine(com_openaudio_FWBaseEngine *engine) = 0;
    // ref vt[0xaf8] (FWAudioDevice @0x1c8c: empty): install the engine's rearrange / input-convert procs for its current channel counts
    virtual void ResetRearrangementProcs(com_openaudio_FWBaseEngine *engine) { (void)engine; }
    bool createAudioEngine();
    void SetChannelNamesForMainEngine(IOAudioEngine *engine);                  // ref @0x2d2a: IOAudioEngineChannel{,Category,Number}Names

    // ---- M-E: names, property dictionary, meters, factory settings (ref FWAudioDevice::initHardware @0x342e, Q3b §1) ----
    virtual const char *GetIconName() const { return ""; }                      // vt[0xa48]
    virtual const char *GetDescriptionString() const { return ""; }             // vt[0xa50]
    virtual const char *GetShortDescriptionString() const { return ""; }        // vt[0xa58]
    // AC3 / IEC 61937 passthrough capability (ref FWAudioDevice @0x1c40 / 0x1c4c / 0x1c9e / 0x1c2e; the FW1814 uses the base versions
    // except IndexForAC3OutputChannels @0xc3fe = {4, 5}, the S/PDIF output pair)
    virtual bool SupportsAC3() const { return true; }                           // vt[0xaa8]
    virtual bool WantsAC3MixerBypass() const { return false; }                  // vt[0xab0]
    virtual void AC3Activated(bool on) { (void)on; }                            // vt[0xb38]
    virtual void IndexForAC3OutputChannels(UInt32 &left, UInt32 &right) const { left = 0; right = 2; }   // vt[0xaa0]
    virtual UInt32 GetPeakLevelCount() const { return 0x34; }                   // vt[0xaf0] ref @0x1d30: 52 meters
    // vt[0xa80]: per model (FW1814 @0xc3ba 0x26, Audiophile @0xaa02 0xe, ...); no base body in the exports, the clone's base 0 is a default
    virtual UInt32 GetFirstSWReturnPeakMeterIndex() const { return 0; }
    // vt[0xa88] ref FWAudioDevice @0xa96c (asm 0xa96c-0xa97d): GetDeviceChannels()->out (+0x08); the FW1814 overrides it (@0xc3c6, 0xe)
    virtual UInt32 GetSWReturnPeakMeterCount() const { FWDeviceChannels ch; GetDeviceChannels(ch); return ch.out; }
    virtual IOReturn InitializeUserSettableProperties();                        // vt[0xb28] ref @0x...: GUID, property dictionary, device-level props
    virtual void ResendCurrentMixerSettings() {}                                // vt[0xb00]
    virtual void ResetToFactorySettings() { fMode = 0x40000000; }               // vt[0xb08] ref @0x1db4 (objdump): [+0x144] = internal clock
    virtual bool StartReceptionOfControlPackets() { return true; }              // vt[0xb78]
    virtual IOReturn SetBlankSlateClockSource() { return kIOReturnSuccess; }    // vt[0xb58]
    // power (ref FWAudioDevice::performPowerStateChange @0x1e96): sleep -> active marks "woke from sleep"; active -> sleep runs SystemWillSleep
    virtual IOReturn performPowerStateChange(IOAudioDevicePowerState oldState, IOAudioDevicePowerState newState, UInt32 *microsecondsUntilComplete) APPLE_KEXT_OVERRIDE;
    virtual void SystemWillSleep() {}                                           // vt[0xb18]
    virtual void SystemWokeUp() {}                                              // vt[0xb10]  (also WakeAfterSleep @0x1e7c)
    virtual void PauseReceptionOfControlPackets() {}                            // vt[0xb88]
    virtual void ResumeReceptionOfControlPackets() {}                           // vt[0xb90]
    virtual void BusReset() {}                                                  // vt[0xb20] (ref FWAudioDevice::BusReset @0x1d3c forwards to the connection manager)
    virtual IOReturn message(UInt32 type, IOService *provider, void *argument = 0) APPLE_KEXT_OVERRIDE;   // ref @0x3344
    virtual void SwitchDigitalSignals(UInt32 clk) { (void)clk; }                // vt[0xbe0]
    virtual IOReturn SetClockSourceInternal(UInt32 clk, bool blank) { (void)clk; (void)blank; return kIOReturnSuccess; }   // vt[0xb60]
    // ref GetDeviceChannels vt[0xa60] @0x1e52 / FetchIndexIntoSampleRateArray
    void GetDeviceChannels(FWDeviceChannels &out) const;
    static UInt32 FetchIndexIntoSampleRateArray(const FWSampleRateEntry *rates, UInt32 rate);
    virtual bool AllocateConnectionManager() { return true; }                   // vt[0xa28]
    virtual bool initHardware(IOService *provider) APPLE_KEXT_OVERRIDE;         // ref @0x342e
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;
    com_openaudio_FWPropertyDict *propDict() const { return fPropDict; }        // ref [+0xf8]
    void   ClearPeakMeters();                                                   // ref @0x...: zero both 52-entry arrays
    IOReturn ResetMixer(OSObject *);                                            // MAMetaResetToFactorySettings
    IOReturn SetDeviceUserLabel(OSObject *);                                    // MADeviceUserLabel
    IOReturn SetDeviceLabel(const char *label);
    IOReturn SetExportStereoPairEngines(UInt32 on);                            // ref @0x1dd6: off -> RemoveAllSubEngines, on -> CreateStereoPairSubEngines
    virtual void CreateStereoPairSubEngines();                                 // vt[0xa40] ref @0x30f2
    IOReturn SetExportAggregateEngine(UInt32) { return (IOReturn)0xE00002C7; }  // ref @0x1dca
    // ref DetectExternalSyncChanges: FDF/lock bits of the control packet -> MASampleRate / MARateLocked
    void   DetectExternalSyncChanges(bool locked, UInt32 fdf);
    // ref overrides: the property dictionary answers getProperty/copyProperty (after the LatencyProfiler handshake) and setProperties
    virtual OSObject *getProperty(const char *aKey) const APPLE_KEXT_OVERRIDE;
    virtual OSObject *copyProperty(const char *aKey) const APPLE_KEXT_OVERRIDE;
    virtual IOReturn setProperties(OSObject *properties) APPLE_KEXT_OVERRIDE;
    // ref @0x...: engine registration is deferred until the device is fully set up (FWBaseEngine::registerService is a no-op)
    UInt32 *meters() const { return fMeters; }                                  // ref [+0x108], the shared 52 x uint32 page
    UInt32 *softwarePeaks() const { return fSwPeaks; }                          // ref [+0x110]
    IOBufferMemoryDescriptor *meterMemory() const { return fMeterMem; }         // ref [+0x100]

    UInt32 deviceMode() const { return fMode; }
    void   setDeviceMode(UInt32 m) { fMode = m; }
    com_openaudio_FWBaseEngine *mainEngine() const { return fMainEngine; }
    void   setMainEngine(com_openaudio_FWBaseEngine *e) { fMainEngine = e; }
protected:
    UInt32 fMode;                           // ref [+0x144]: bit 3 = ADAT in, bit 4 = ADAT out (MAClockSource bits)
    com_openaudio_FWBaseEngine *fMainEngine;   // ref [+0xe8]
    com_openaudio_FWMetaNub *fMetaNub;         // the provider (ref [+0xe0] is the IOFireWireNub behind it)
    OSObject *nubProperty(const char *key) const;   // ref nub->getProperty(key) on [+0xe0]
    com_openaudio_FWPropertyDict *fPropDict;   // ref [+0xf8]
    IOBufferMemoryDescriptor *fMeterMem;       // ref [+0x100]
    UInt32 *fMeters;                           // ref [+0x108]
    UInt32 *fSwPeaks;                          // ref [+0x110]
    OSArray *fTrace;                           // clone: MADebugTrace ring
    IOLock  *fTraceLock;                       // clone (AP-3.9): DebugTrace is called from the engine, work-loop and HSCI threads
    OSDictionary *fGetCache;                   // clone: keeps the last getProperty() result per key (the reference leaks one reference per call)
    bool   fReady;                             // ref [+0x258]
    bool   fWokeFromSleep;                     // ref [+0x259]
    bool   fRateLocked;                        // ref [+0x25a]
    UInt32 fDetectedFdf;                       // ref [+0x25c]
};

#endif
