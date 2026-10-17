// The FW1814 device class chain (Q16 Correction 2026-10-14, Q-SDK5):
//   IOAudioDevice -> FWDiscreteDevice -> FWAudioDevice -> FWAVCAudioDevice -> FWRev1AudioDevice -> FW1814Device
// The reference names are m_audio_b_FW{Discrete,Audio,AVCAudio}Device / FWRev1AudioDevice / com_m_audio_FW1814Device.
// M-A: shells only. Every free() is NULL-safe from day one (10.13.6 operator delete(NULL) panics, Q-SDK5 obs. 8-10);
// each later milestone that adds a member also adds its guarded release here.
#ifndef OA_FWAUDIODEVICECHAIN_H
#define OA_FWAUDIODEVICECHAIN_H

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
class com_openaudio_FW1814Mixer;
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
    // ref vt[0xa68] GetDeviceChannelsForSampleRate (FW1814Device @0xc998). The base returns the S/PDIF 44.1-96 table.
    virtual void GetDeviceChannelsForSampleRate(UInt32 rateHz, FWDeviceChannels &out) const { FWComposeDeviceChannels(rateHz, fMode, out); }
    // ref vt[0xab8] (FW1814Device @0xcf30) and vt[0xac0] (FWAudioDevice @0x1d58)
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const { return FWRoundTripLatencyForFDF(fdf); }
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
    virtual UInt32 GetFirstSWReturnPeakMeterIndex() const { return 0x26; }      // vt[0xa80] ref FW1814 @0xc3ba
    virtual UInt32 GetSWReturnPeakMeterCount() const { return 0xe; }            // vt[0xa88] ref FW1814 @0xc3c6
    virtual IOReturn InitializeUserSettableProperties();                        // vt[0xb28] ref @0x...: GUID, property dictionary, device-level props
    virtual void ResendCurrentMixerSettings() {}                                // vt[0xb00]
    virtual void ResetToFactorySettings() {}                                    // vt[0xb08]
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
    OSDictionary *fGetCache;                   // clone: keeps the last getProperty() result per key (the reference leaks one reference per call)
    bool   fReady;                             // ref [+0x258]
    bool   fWokeFromSleep;                     // ref [+0x259]
    bool   fRateLocked;                        // ref [+0x25a]
    UInt32 fDetectedFdf;                       // ref [+0x25c]
};

class com_openaudio_FWAVCAudioDevice : public com_openaudio_FWAudioDevice
{
    OSDeclareAbstractStructors(com_openaudio_FWAVCAudioDevice)
public:
    // The only probe override in the device chain (Q-SDK5 obs. 5, ref @0x22f78).
    virtual IOService *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
    virtual bool initHardware(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    com_openaudio_FWMetaNub *metaNub() const { return fMetaNub; }
    com_openaudio_FWAvcUnit *avcUnit() const { return fAvcUnit; }
    com_openaudio_FWAvcConnectionManager *connectionManager() const { return fConnMgr; }
    virtual IOReturn AVCControlPlugSignalFormat(bool input, UInt8 plug, UInt8 fdf) APPLE_KEXT_OVERRIDE;   // Rev1 form (FWAvc.cpp)
    // ref CustomMixerWriteQuads @0x22c7a (vt[0xba8]): async block write of `quads` quadlets to 0xFFC7:0x00700000 + off (no-op without a nub)
    virtual IOReturn CustomMixerWriteQuads(UInt32 off, UInt32 quads, const void *data);
    virtual IOReturn InitializeUserSettableProperties() APPLE_KEXT_OVERRIDE;   // ref @0x...: base + MAFirmwareInfo + MAClockSource
    virtual void BusReset() APPLE_KEXT_OVERRIDE;                               // ref FWAudioDevice::BusReset @0x1d3c: connection manager replay
    virtual void SystemWillSleep() APPLE_KEXT_OVERRIDE;                        // ref @0x22b16
    virtual void SystemWokeUp() APPLE_KEXT_OVERRIDE;                           // ref @0x22d76
    virtual bool HasSoftwareControls() const { return true; }                  // vt[0xac8] (ref FW1814 @0xc32c)
    virtual void ResendCurrentMixerSettings() APPLE_KEXT_OVERRIDE;             // ref @0x23af8: throttle + clock source re-assert
    IOReturn SetClockSource(UInt32 clk);                                       // ref @0x23858 (MAClockSource setter)
    virtual IOReturn SetClockSourceInternal(UInt32 clk, bool blank) APPLE_KEXT_OVERRIDE;   // ref @0x23148: AV/C selector command on function block 4
protected:
    UInt64 fLastResend;                                     // ref [+0x270]
    com_openaudio_FWAvcUnit *fAvcUnit;                      // ref [+0x268]
    com_openaudio_FWAvcConnectionManager *fConnMgr;         // ref [+0xf0]
    virtual bool AllocateConnectionManager() APPLE_KEXT_OVERRIDE;     // ref @0x23c72
    void ReleaseConnectionManager();
};

// Rev1: plain 8-byte 0x18/0x19 AV/C stream-format command (Q3a). Nothing to override until M-C.
class com_openaudio_FWRev1AudioDevice : public com_openaudio_FWAVCAudioDevice
{
    OSDeclareAbstractStructors(com_openaudio_FWRev1AudioDevice)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
};

// Reference-name alias (A3-M2: the reference MIDI plug-in and panel find the device with IOServiceMatching("com_m_audio_FW1814Device"), which
// matches by class inheritance). An abstract class of that exact name sits between the chain and our concrete class so the unmodified
// reference user-space components still match the clone. It adds no behaviour. Conflicts with the reference kext (same class name) by design.
class com_m_audio_FW1814Device : public com_openaudio_FWRev1AudioDevice
{
    OSDeclareAbstractStructors(com_m_audio_FW1814Device)
};

class com_openaudio_FW1814Device : public com_m_audio_FW1814Device
{
    OSDeclareDefaultStructors(com_openaudio_FW1814Device)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    // ref FW1814Device::GetRoundTripLatencyForFDF @0xcf30 -- the FW1814 table (the base default is the same table, kept explicit)
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const APPLE_KEXT_OVERRIDE { return FWRoundTripLatencyForFDF(fdf); }
    virtual com_openaudio_FWBaseEngine *AllocateAudioEngine() APPLE_KEXT_OVERRIDE;      // ref @0xd78a -> FWEngine2
    virtual void SetupAudioEngine(com_openaudio_FWBaseEngine *engine) APPLE_KEXT_OVERRIDE;   // ref @0xed34
    virtual void ResetRearrangementProcs(com_openaudio_FWBaseEngine *engine) APPLE_KEXT_OVERRIDE;   // ref @0xcad4
    virtual void IndexForAC3OutputChannels(UInt32 &left, UInt32 &right) const APPLE_KEXT_OVERRIDE { left = 4; right = 5; }   // ref @0xc3fe
    virtual const char *GetIconName() const APPLE_KEXT_OVERRIDE { return "FW 1814 box.png"; }          // ref @0xc96e
    virtual const char *GetDescriptionString() const APPLE_KEXT_OVERRIDE { return "FireWire 1814"; }   // ref @0xc97c
    virtual const char *GetShortDescriptionString() const APPLE_KEXT_OVERRIDE { return "FW 1814"; }    // ref @0xc98a
    virtual IOReturn InitializeUserSettableProperties() APPLE_KEXT_OVERRIDE;                          // ref @0xe6ca
    virtual void ResendCurrentMixerSettings() APPLE_KEXT_OVERRIDE;                                    // ref @0xd014 (parent + the four settings)
    virtual void ResetToFactorySettings() APPLE_KEXT_OVERRIDE;                                        // ref @0xd190
    virtual IOReturn SetClockSourceInternal(UInt32 clk, bool blank) APPLE_KEXT_OVERRIDE;                // ref @0xe25c
    virtual IOReturn SetBlankSlateClockSource() APPLE_KEXT_OVERRIDE;                                    // ref @0xe4f0
    virtual void SwitchDigitalSignals(UInt32 clk) APPLE_KEXT_OVERRIDE;                                  // ref @0xe5dc
    void ReconnectP2P();                                                                                // ref @0xe1ea
    virtual void BusReset() APPLE_KEXT_OVERRIDE;                                                        // ref @0xe53a: ReconnectP2P on a new thread
    // hardware control poll (A1 Q-ME §5)
    virtual bool StartReceptionOfControlPackets() APPLE_KEXT_OVERRIDE;                                  // ref @0xe064
    void StopReceptionOfControlPackets();                                                               // ref @0xda7e
    virtual void PauseReceptionOfControlPackets() APPLE_KEXT_OVERRIDE;                                  // ref @0xdbbe
    virtual void ResumeReceptionOfControlPackets() APPLE_KEXT_OVERRIDE;                                 // ref @0xdba6
    UInt32 GetControlPacketSize() const { return 0x58; }                                                // ref @0xc962
    void ReadHSCIData(const UInt32 *packet);                                                            // ref @0xd952
    void ReceiveControlPacket(const UInt8 *packet);                                                     // ref @0xe9ea
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;                                         // ref @0xd9ce
    com_openaudio_FW1814Mixer *mixer() const { return fMixer; }
    // M-S deviation (user decision 2026-10-17): after an optical switch that changes the channel counts the main engine is re-published, because
    // the 10.13 Audio MIDI Setup builds its "N ins / M outs" sidebar label only when the device is added (Q-AVC-unit-and-O1 section 3, capture 82).
    // The reference changes the stream formats in place (SwitchDigitalSignals @0xe5dc) and never re-creates the engine.
    void RepublishAudioEngine();
    // A re-publish drops every CoreAudio client of the device (reAMP froze mid-playback, 2026-10-17), so it only runs while the engine is idle;
    // during playback the formats change in place (as the reference) and the re-publish is deferred until the engine has stopped.
    void RepublishIfIdle();                                                                             // runs on the device work loop
    void DeferredRepublish();                                                                           // thread body
    virtual void audioEngineStopped() APPLE_KEXT_OVERRIDE;
    // ref @0xcba4: input converter for the 1814 -- reorders the first 8 channels and delays 6 of them by 16 frames
    void WorkaroundFor1814InputDelay(UInt32 count, const UInt32 *src, SInt32 *dst);
private:
    com_openaudio_FW1814Mixer *fMixer;
    bool fClockBusy;                                // ref [+0x1b88]: SetClockSourceInternal in progress
    bool fRepublishPending;                         // clone (M-S): SwitchDigitalSignals changed the channel counts
    bool fRepublishDeferred;                        // clone (M-S): counts differ from the published engine's but a client was streaming
    bool fRepublishThread;                          // clone (M-S): a deferred-republish thread is alive
    UInt32 fPubIn, fPubOut;                         // clone (M-S): channel counts the HAL currently knows (0 = not recorded yet)
    com_openaudio_FWBaseEngine *fMidiDonor;         // clone (M-S): engine whose MIDI ports the next SetupAudioEngine adopts
    com_openaudio_FWHsci *fHsci;                    // ref [+0x1270]
    com_openaudio_FWNumber32Prop *fStereoPairProp;  // MAEnableStereoPairEngines (ref value cell [+0x184])
    // ref device [+0x1b8c] index table (identity * 6, set in the constructor @0xf04a), [+0x1bcc] 16 frames x 6 raw quadlets, [+0x1d4c] counter
    UInt32 fDelayHist[16 * 6];
    UInt32 fDelayCounter;
};

#endif
