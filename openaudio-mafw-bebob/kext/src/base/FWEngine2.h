// FWEngine2 -- the multichannel engine (IOAudioEngineFlavor 1; on the FW1814 the SL ioreg shows "FireWire 1814 Multichannel").
// Mirrors com_m_audio_FWEngine2 (A1 Q13, Q15c, Q-START, Q-MC, Q-MD; decomp FWEngine2__*.c). The class is allocated by
// FW1814Device::AllocateAudioEngine @0xd78a. M-D scope: connect/start/stop chain, format & rate change, bandwidth. AC3 (M-I)
// and MIDI (M-H) entry points exist as the reference's slots but report "off".
#ifndef OA_FWENGINE2_H
#define OA_FWENGINE2_H

#include "FWBaseEngine.h"

class com_openaudio_FWEngine2 : public com_openaudio_FWBaseEngine
{
    OSDeclareDefaultStructors(com_openaudio_FWEngine2)
public:
    virtual bool initHardware(IOService *provider) APPLE_KEXT_OVERRIDE;              // ref @0x12622
    // Bench diagnostics (integrated run 2026-10-16): engine state, isoch refcount, active sub-engine mask, DCL programs running.
    void LogStreamState(const char *where);
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;                      // ref @0x117d4

    // ---- format / rate ----
    virtual IOReturn PerformFormatChange(IOAudioStream *stream, const IOAudioStreamFormat *fmt, const IOAudioSampleRate *rate) APPLE_KEXT_OVERRIDE;   // ref @0x124cc
    virtual bool PerformSampleRateChangeCore(const IOAudioSampleRate *rate) APPLE_KEXT_OVERRIDE;      // ref @0x117e6
    virtual bool UpdateDCLProgramsWithNewPacketSizes() APPLE_KEXT_OVERRIDE;                          // ref @0x122d6
    virtual IOReturn RecalculateBandwidth() APPLE_KEXT_OVERRIDE;                                      // ref @0x1235a
    virtual IOReturn SetExternalSyncEnabled(bool on) APPLE_KEXT_OVERRIDE;                            // ref @0x11bc0

    // ---- connect / start / stop (Q-START, Q-MC) ----
    virtual IOReturn ConnectAllPlugs() APPLE_KEXT_OVERRIDE;                          // ref @0x12476
    virtual void     DisconnectAllPlugs() APPLE_KEXT_OVERRIDE;                       // ref @0x11bfa
    IOReturn ConnectAudioMIDIChannels();                                             // ref @0x12156
    virtual IOReturn StartAudio() APPLE_KEXT_OVERRIDE;                               // ref @0x12024
    virtual void     StopAudio() APPLE_KEXT_OVERRIDE;                                // ref @0x11ab2
    virtual void     StopIsochStreams() APPLE_KEXT_OVERRIDE;                         // ref @0x11ed2
    IOReturn StartPCMAudio();                                                        // ref @0x11b20 (vt[0xd08])
    IOReturn ActivatePCMInput();                                                     // ref @0x11a5e (vt[0xd28]): RX program Start
    IOReturn ActivatePCMOutput();                                                    // ref @0x11a8c (vt[0xd20]): TX program Start
    void     DeactivatePCMInput();                                                   // ref @0x118a8 (vt[0xd40]): RX Stop
    void     DeactivatePCMOutput();                                                  // ref @0x11892 (vt[0xd38]): TX Stop
    virtual IOReturn StartMIDI() APPLE_KEXT_OVERRIDE;                                // ref @0x120d4
    virtual IOReturn StopMIDI() APPLE_KEXT_OVERRIDE;                                 // ref @0x11b86
    // AC3 (M-I): the reference slots (StartAC3Output vt[0xd10], ActivateAC3Output vt[0xd18], DeactivateAC3Output vt[0xd30])
    virtual IOReturn StartAC3Output();                                               // ref @0x1185e (vt[0xd10])
    virtual IOReturn ActivateAC3Output();                                            // ref @0x11fee (vt[0xd18])
    virtual void     DeactivateAC3Output();                                          // ref @0x11fc6 (vt[0xd30])
    IOReturn SetAC3OutputState(bool on);                                             // ref @0x11f28

    // ---- sample path (M-D2) ----
    virtual void ClipPCMOutput(const float *mix, SInt32 *scratch, UInt32 first, UInt32 num) APPLE_KEXT_OVERRIDE;   // ref @0x11d72 (vt[0xca0])
    virtual void ClipAC3Output(const float *mix, SInt32 *scratch, UInt32 first, UInt32 num) APPLE_KEXT_OVERRIDE;   // ref @0x11c44 (vt[0xca8]) -- M-I

    // Device-side configuration, ref FW1814Device::SetupAudioEngine @0xed34
    // Device-side engine configuration (each model's SetupAudioEngine): channel counts (stream = channels), m, and the three engine flags the
    // reference sets per model -- FW1814 @0xed34 sets [+0x2d5], [+0x2d6] (refresh RX above 96 kHz) and [+0x348] (two-phase connect); the
    // Audiophile sets none of them.
    void SetupFromDevice(UInt32 outChannels, UInt32 inChannels, UInt32 midiChannels, bool flagA, bool rxRefreshAtHighRates, bool twoPhase);
};

#endif
