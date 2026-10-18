// FWBaseEngine -- the IOAudioEngine the reference calls m_audio_b_FWBaseEngine (A1 Q13, Q15c, Q-MD; decomp FWBaseEngine__*.c).
// M-D scope (this file): lifecycle, the rate/format change chain, latency, pending-state machine, DCL-program ownership and
// isoch start/stop refcounting. NOT ported yet (declared, documented, safe no-ops): MIDI (M-H), AC3 (M-I), the sample converters /
// clip procs and sample peaks (M-D2: the _Convert*/_Rearrange*/_SamplePeaks* free functions), pair engines (M-J).
//
// State surface, reference offset -> member (offsets read in the decompiles; the IOAudioEngine part is verified against the 10.13
// header layout: numSampleFrames 0x88, sampleRate 0x8c, cfgChange 0xa5, state 0xa8, status 0xb0, device 0xb8, active clients 0x100).
#ifndef OA_FWBASEENGINE_H
#define OA_FWBASEENGINE_H

#include "FWDiscreteEngine.h"
#include "FWSharedSync.h"
#include "FWStream.h"
#include "FWDCLProgram.h"
#include "FWStreamControl.h"
#include "FWRates.h"
#include "FWSamples.h"
#include "FWMidi.h"
#include "FWAudioDeviceChain.h"

// Pending-state bits (engine [+0x344], a UInt32; the reference ORs the BYTES [+0x345] (bit 15), [+0x346] (bits 16-22) and
// [+0x347] (bit 24)). Names describe what CompletePendingStateChangesAction @0x3b2c does when the bit is set; bits 16 and 17
// are set (|= 0x1d / 0x6f / 7 on [+0x346]) but no consumer was found in the decompiles -- left unnamed on purpose.
enum {
    kFWPendingBusy             = 0x00000001,   // Action returns at once
    kFWPendingReconnectPlugs   = 0x00008000,   // ConnectAllPlugs after the plugs were dropped
    kFWPendingBit16            = 0x00010000,   // set only
    kFWPendingBit17            = 0x00020000,   // set only
    kFWPendingPacketSizes      = 0x00040000,   // vt[0xc78] UpdateDCLProgramsWithNewPacketSizes (also forces a plug reconnect)
    kFWPendingBandwidth        = 0x00080000,   // vt[0xcc8] RecalculateBandwidth (also forces a plug reconnect)
    kFWPendingRateNotify       = 0x00100000,   // SetStaticPropertyAndNotifyUserClients("MASampleRate")
    kFWPendingResetBuffers     = 0x00200000,   // ResetSampleBuffers
    kFWPendingResetPairFormats = 0x00400000,   // ResetStereoPairIOFormats
    kFWPendingRecreateStreams  = 0x00800000,   // RecreateAudioStreams(true, true)
    kFWPendingSettle           = 0x01000000    // IOSleep(1000)
};

class com_openaudio_FWBaseEngine : public com_openaudio_FWDiscreteEngine
{
    OSDeclareAbstractStructors(com_openaudio_FWBaseEngine)
public:
    // ---- lifecycle ----
    virtual bool init(OSDictionary *properties) APPLE_KEXT_OVERRIDE;                 // ref @0x5dd4
    virtual bool initHardware(IOService *provider) APPLE_KEXT_OVERRIDE;              // ref @0x56b0
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;                      // ref @0x5cd6
    virtual void free() APPLE_KEXT_OVERRIDE;

    // ---- IOAudioEngine ----
    virtual UInt32 getCurrentSampleFrame() APPLE_KEXT_OVERRIDE;                      // ref @0x3e0c
    virtual IOReturn performAudioEngineStart() APPLE_KEXT_OVERRIDE;                  // ref @0x556e
    virtual IOReturn performAudioEngineStop() APPLE_KEXT_OVERRIDE;                   // ref @0x543c
    virtual IOReturn pauseAudioEngine() APPLE_KEXT_OVERRIDE;                         // ref @0x3d54
    virtual IOReturn resumeAudioEngine() APPLE_KEXT_OVERRIDE;                        // ref @0x3dae
    virtual void performErase() APPLE_KEXT_OVERRIDE;                                 // ref @0x4e08
    // ref performFormatChange @0x3e54: runs ActionPerformFormatChange on the engine work loop -> PerformFormatChange (subclass).
    virtual IOReturn performFormatChange(IOAudioStream *audioStream, const IOAudioStreamFormat *newFormat,
                                         const IOAudioSampleRate *newSampleRate) APPLE_KEXT_OVERRIDE;
    // ---- sample path (M-D2, Q-MD2) ----
    virtual IOReturn MainEngineClipOutputSamples(UInt32 first, UInt32 num, const IOAudioStreamFormat *fmt, IOAudioStream *stream) APPLE_KEXT_OVERRIDE;   // ref @0x3ed4
    virtual void     ConvertInputSamplesCore(UInt32 first, UInt32 num) APPLE_KEXT_OVERRIDE;                                                              // ref @0x496a
    virtual void    *GetMultichannelInputBuffer() APPLE_KEXT_OVERRIDE { return fInRing180.data; }                                                       // ref @0x1178a: [+0x1a8]
    // vt[0xc90] ref @0x40aa: AC3 (only while the engine is in AC3 mode and the request fits 0x600 frames) or PCM clip of the scratch
    IOReturn ClipOutputSamplesWithMixPtrAdjusted(const float *mix, SInt32 *scratch, UInt32 first, UInt32 num,
                                                 const IOAudioStreamFormat *fmt, IOAudioStream *stream);
    // vt[0xca0] / vt[0xca8]: FWEngine2 implements them (ref @0x11d72 / @0x11c44)
    virtual void ClipPCMOutput(const float *mix, SInt32 *scratch, UInt32 first, UInt32 num) { (void)mix; (void)scratch; (void)first; (void)num; }
    virtual void ClipAC3Output(const float *mix, SInt32 *scratch, UInt32 first, UInt32 num) { (void)mix; (void)scratch; (void)first; (void)num; }
    // vt[0xc98] ref @0x4624
    void ConvertInputSampleData(UInt32 num, const UInt32 *src, SInt32 *scratch, float *dst);
    // installed by the device's ResetRearrangementProcs (vt[0xaf8]) -- ref engine [+0x300] / [+0x310] / [+0x318]
    void SetRearrangeProc(FWRearrangeProc p)               { fRearrangeProc = p; }
    void SetPeakProc(FWPeakProc p)                         { fPeakProc = p; }
    void SetInputConvertProc(FWInputConvertProc p, void *arg) { fInputProc = p; fInputProcArg = arg; }
    IOAudioStream *mainOutputStream() const { return fOutStream; }                  // ref vt[0xbf8] GetOutputAudioStream @0x11798
    IOAudioStream *mainInputStream() const  { return fInStream; }                   // ref vt[0xc00] GetInputAudioStream @0x117a6
    UInt32 outChannels() const  { return fOutChannels; }                             // [+0x2e0]
    UInt32 inChannels() const   { return fInChannels; }                              // [+0x2e4]
    UInt32 midiChannels() const { return fMidiChannels; }                            // [+0x2e8]

    // ---- format / rate chain (Q15c) ----
    // vt[0xc88] subclass hook
    virtual IOReturn PerformFormatChange(IOAudioStream *stream, const IOAudioStreamFormat *fmt, const IOAudioSampleRate *rate) = 0;
    // vt[0xce0] subclass hook; base = ref PerformSampleRateChangeCore @0x479e (returns 1 = success)
    virtual bool PerformSampleRateChangeCore(const IOAudioSampleRate *rate);
    // ref SetIOFormat @0x42aa: apply a DeviceChannels row + rate index
    IOReturn SetIOFormat(const FWDeviceChannels &ch, UInt32 rateIndex);
    // Clone fix (2026-10-17, stale output channel count after a CoreAudio rate change; the SL's official driver does not show it, capture 85):
    // on 10.13 IOAudioStream::setFormat re-applies the format CoreAudio REQUESTED to the stream it asked through, after performFormatChange has
    // returned, undoing the channel count SetIOFormat just set on that stream. VerifyStreamFormats (device work loop, scheduled from
    // PerformFormatChange so it runs after the gate is released) re-applies the formats of the current row to any stream that was overwritten.
    void ScheduleVerifyStreamFormats();
    void VerifyStreamFormats();
    // ref SetStreamFormat @0x4192
    IOReturn SetStreamFormat(IOAudioStream *stream, const FWDeviceChannels &ch, UInt32 rateIndex);
    // ref ChangeSampleRateToFDFValue @0x52e8: external-sync / HSCI driven rate change
    void ChangeSampleRateToFDFValue(UInt32 fdf);
    // ref ResetLatency @0x3f7a: input/output latency = round-trip/2, sample offset = safety offset
    void ResetLatency();
    // ref CompletePendingStateChanges @0x3ae6 (+Action @0x3b2c): run on the work loop
    IOReturn CompletePendingStateChanges();
    IOReturn CompletePendingStateChangesAction();                                    // ref @0x3b2c (called on the work loop)
    void     SetPending(UInt32 bits) { fPending |= bits; }

    // ---- DCL programs (Q-MB, Q-MC) ----
    IOReturn AddDCLProgram(com_openaudio_FWDCLProgram *p);                           // ref @0x4fcc
    IOReturn RemoveDCLProgram(com_openaudio_FWDCLProgram *p);                        // ref @0x44ec
    void     IOSynchronizeDCLPrograms();                                            // ref @0x4902
    // ref StartIsochStreams @0x54b0 / StopIsochStreams @0x416c: reference counted ([+0x22c])
    virtual IOReturn StartIsochStreams() APPLE_KEXT_OVERRIDE;
    virtual void     StopIsochStreams() APPLE_KEXT_OVERRIDE;
    // vt[0xcd0] / vt[0xcd8]: base = ref FWBaseEngine::StartAudio @0x4bca / StopAudio @0x4738
    virtual IOReturn StartAudio();
    virtual void     StopAudio();
    // vt[0xcb8] / vt[0xcc0] / vt[0xcc8] / vt[0xc78]: FWEngine2 implements (FWBaseEngine's slots are empty in the reference)
    virtual IOReturn ConnectAllPlugs() { return kIOReturnUnsupported; }
    virtual void     DisconnectAllPlugs() {}
    virtual IOReturn RecalculateBandwidth() { return kIOReturnSuccess; }
    virtual bool     UpdateDCLProgramsWithNewPacketSizes() { return true; }

    // ---- streams / buffers ----
    // ref RecreateAudioStreams @0x4546, createNewAudioStream @0x55b6, ResetSampleBuffers @0x57f8
    bool RecreateAudioStreams(bool createInput, bool createOutput);
    com_openaudio_FWStream *createNewAudioStream(bool attachToEngine, IOAudioStreamDirection dir, void *buffer, UInt32 bytes);
    bool ResetSampleBuffers();

    // ---- external sync (ref @0x53ec) ----
    virtual IOReturn SetExternalSyncEnabled(bool on);
    void TellClientsSampleRateChanged(UInt32 rate);                                  // ref @0x5422

    // ---- AC3 / MIDI hooks (M-I / M-H): present so the chain compiles and behaves as "feature off" ----
    virtual void EnableAC3Format();                                                  // ref @0x445e
    virtual void DisableAC3Format();                                                 // ref @0x596a
    void AddAC3Format(IOAudioStream *stream);                                        // ref @0x39a0
    virtual IOReturn StartMIDI();                                                    // vt[0xcf0] ref @0x4118 (FWEngine2 @0x120d4)
    virtual IOReturn StopMIDI();                                                     // vt[0xcf8] ref @0x40f6 (FWEngine2 @0x11b86)
    virtual IOReturn StopMIDIAndClearBuffers();                                      // vt[0xd00] ref @0x4d78
    // M-S deviation (user decision 2026-10-17, 10.13 AMS builds its "N ins / M outs" label only when the device is added): the device re-publishes
    // its main engine after an optical switch. Quiesce = stop the old engine completely (state, MIDI, isoch references, plugs) so that its
    // asynchronous termination (stop()) has nothing left to undo; AdoptMIDIPortsFrom hands the MIDI port rings (user/kernel shared memory the
    // CoreMIDI plug-in mapped once) to the new engine so MIDI keeps working across the re-publish.
    void Quiesce();
    void AdoptMIDIPortsFrom(com_openaudio_FWBaseEngine *donor);
    // ref SetMIDIInputPorts @0x5226 / SetMIDIOutputPorts @0x5160 / SetMIDIPortProperties @0x5028: "input" = MIDI sources (device -> host)
    void SetMIDIInputPorts(com_openaudio_FWAudioDevice *dev, UInt32 count, const char **names);
    void SetMIDIOutputPorts(com_openaudio_FWAudioDevice *dev, UInt32 count, const char **names);
    bool midiRunning() const { return fShared.midiInEnabled; }                      // ref [+0x298]
    // user-client entry points (ref FWUserClient::CoreStartMIDI @0x717c / CoreStopMIDI @0x714a, touching the engine's MIDI state)
    IOReturn UserStartMIDI();
    void     UserStopMIDI();
    virtual IOReturn SendMIDI(void *packet, UInt32 *result) { (void)packet; (void)result; return (IOReturn)0xE00002C7; }   // ref FWBaseEngine::SendMIDI (M-H)
    virtual IOMemoryDescriptor *MidiPortMemory(bool output, UInt32 port);           // ref [+0x2a8] (destinations) / [+0x2b0] (sources) descriptors
    virtual void     ReconnectAfterBusReset() {}                                     // ref @0x394a (empty in the base)

    // ref getGlobalUniqueID @0x5b84 / getLocalUniqueID @0x5adc: "<class>:<registry name>:<GUID hex>". The class-name part keeps the REFERENCE's
    // string ("com_m_audio_FWEngine2" / "...FWEngine1" for pair engines) so that a CoreAudio device UID stays the same as with the 2014 driver.
    virtual OSString *getGlobalUniqueID() APPLE_KEXT_OVERRIDE;
    virtual OSString *getLocalUniqueID() APPLE_KEXT_OVERRIDE;
    virtual const char *UniqueIDClassName() const { return "com_m_audio_FWEngine2"; }
    // ref FWBaseEngine::registerService @0x399a is an empty function: registration of the engine is deferred until the device has finished
    // initHardware, which calls ReallyRegisterService (vt[0xc70] @0x3986). That function jumps through FWDiscreteEngine's vtable slot 0x5c0
    // (mafw.asm 0x398c..0x3994), i.e. the inherited IOAudioEngine::registerService, not IOService::registerService.
    // **Correction (2026-10-16, first hardware run):** the clone called IOService::registerService(0), which skips IOAudioEngine's work
    // (registering the streams); coreaudiod then never listed the device. IOAudioEngine::registerService is the reference behaviour.
    virtual void registerService(IOOptionBits options = 0) APPLE_KEXT_OVERRIDE { (void)options; }
    void ReallyRegisterService() { IOAudioEngine::registerService(0); }
    // Bench aid: continuity of the clip requests IOAudioFamily makes (position `first`, `num` frames); read through MADebugTxStats.
    void ClipStatsString(char *out, size_t n) const;
    com_openaudio_FWDCLOutputProgram *txProgram() const { return fTx; }              // bench aid: TX callback timing (MADebugTxStats)
    bool externalSync() const { return fExternalSync; }                              // [+0x2d0]
    // vt[0xb00] / [0xb08] / [0xb10] of IOAudioEngine, public for the device (ref calls them through the engine's vtable)
    void BeginConfigChange()    { beginConfigurationChange(); }
    void CompleteConfigChange() { completeConfigurationChange(); }
    void CancelConfigChange()   { cancelConfigurationChange(); }

    // ---- state accessors ----
    UInt32 rateHz() const        { return fRate; }                                   // [+0x220]
    UInt32 sytInterval() const   { return fSytInterval; }                            // [+0x224]
    UInt32 fdfIndex() const      { return fFdf; }                                    // [+0x228]
    bool   isConnected() const   { return fConnected; }
    UInt32 ActiveUserClients() const { return numActiveUserClients; }          // IOAudioEngine's count of clients doing I/O                              // [+0x2d3]
    bool   isAC3() const         { return fAC3; }                                    // [+0x2d4]
    FWDCLSharedSyncData &sharedSync() { return fShared; }

protected:
    // ref engine fields -- keep the offsets in the comments; they are the contract with the decompiles
    com_openaudio_FWAudioDevice *fDevice;         // [+0x138]
    IOFireWireNub               *fNub;            // [+0x140] = device[0xe0]
    OSArray                     *fPrograms;       // [+0x178] FWDCLProgram collection
    com_openaudio_FWStream      *fInStream;       // [+0x210]  (created with direction 1 = input)
    com_openaudio_FWStream      *fOutStream;      // [+0x218]  (direction 0 = output)  -- Q15c had these two swapped
    UInt32 fRate;                                  // [+0x220] Hz
    UInt32 fSytInterval;                           // [+0x224] 8 / 16 / 32
    UInt32 fFdf;                                   // [+0x228] 0..6
    UInt32 fIsochRefs;                             // [+0x22c]
    bool   fMidiWasRunning;                        // [+0x230] MIDI was running when the engine paused
    FWMIDIPort *fMidiIn;                           // [+0x2b0] source ports   (shared.midiInPortArray, count shared.midiInPorts [+0x29c])
    FWMIDIPort *fMidiOut;                          // [+0x2a8] destination ports (shared.midiOutPortArray, count shared.midiOutPorts [+0x2a0])
    UInt32 fPauseCount;                            // [+0x234]
    bool   fAudioStarted;                          // [+0x238]
    FWDCLSharedSyncData fShared;                   // [+0x240] (sytTable = the 4 KiB buffer at [+0x258]; engine = this at [+0x2b8])
    UInt16 *fSytTable;                             // [+0x258] 2048 x UInt16
    bool   fExternalSync;                          // [+0x2d0]
    bool   fExtSyncLocked;                         // [+0x2d1] -- meaning unverified (gates ChangeSampleRateToFDFValue / RecalculateBandwidth)
    bool   fConnected;                             // [+0x2d3]
    bool   fAC3;                                   // [+0x2d4]
    bool   fFlagA;                                 // [+0x2d5]  set by SetupAudioEngine
    bool   fRxRefreshAtHighRates;                  // [+0x2d6]  set by SetupAudioEngine
    UInt32 fOutStreamBytes;                        // [+0x2d8]
    UInt32 fInStreamBytes;                         // [+0x2dc]
    UInt32 fOutChannels;                           // [+0x2e0]  DeviceChannels.out
    UInt32 fInChannels;                            // [+0x2e4]  DeviceChannels.in
    UInt32 fMidiChannels;                          // [+0x2e8]  m
    UInt32 fOutStreamChannels;                     // [+0x2ec]  DeviceChannels.outStream
    UInt32 fInStreamChannels;                      // [+0x2f0]  DeviceChannels.inStream
    UInt64 fClipOffset[3];                         // [+0x320 +0x328 +0x330]
    FWRearrangeProc    fRearrangeProc;             // [+0x300] output frames -> AM824 quadlets (NULL = no proc for this channel count)
    FWPeakProc         fPeakProc;                  // [+0x308] output peak proc, set by FW1814Device::SwitchDigitalSignals (NULL until the first clock-source change)
    FWInputConvertProc fInputProc;                 // [+0x310]
    void              *fInputProcArg;              // [+0x318]
    UInt32 fInputConvertedEnd;                     // [+0x33c] first + num of the last input conversion
    UInt32 fPending;                               // [+0x344]
    bool   fTwoPhaseConnect;                       // [+0x348]
    com_openaudio_FWDCLOutputProgram *fTx;         // [+0x350]
    com_openaudio_FWDCLInputProgram  *fRx;         // [+0x358]
    // sample rings (ResetSampleBuffers @0x57f8): sizes (channels + midi) * 0x8000 bytes
    TRingBuffer fOutSampleRing;                    // [+0x1e0] output stream sample buffer
    struct ClipStats { bool have; UInt32 expect, calls, back, fwd, maxNum, minNum; UInt64 start; UInt32 nev; struct { UInt32 atMs, expect, first, num, kind; } ev[10]; } fClip;
    TRingBuffer fOutMixRing;                       // [+0x148] output stream mix buffer
    TRingBuffer fInRing180;                        // [+0x180]
    TRingBuffer fInRing1b0;                        // [+0x1b0]
    // the input sample ring is fShared.inputRing ([+0x268]) -- filled by the RX program
    FWStreamControl fStreamControl;
};

#endif
