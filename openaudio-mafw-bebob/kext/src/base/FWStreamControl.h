// FWStreamControl -- the reference stream-connect sequence. Mirrors com_m_audio_FWEngine2::ConnectAudioMIDIChannels @0x12156
// (decomp FWEngine2__ConnectAudioMIDIChannels.c), ::ConnectAllPlugs @0x12476 and FWAudioDevice::SufficientBandwidthAvailable
// @0x22e8. Evidence + corrected order: docs/system-map/A1-ghidra/Q-MC-cmp-isoch-avc-start.md.
//
// Every "kick" is the DEVICE's vt[0xb50] AVCControlPlugSignalFormat(direction, plug 0, FDF, channels, 100): Rev1 = the plain 8-byte
// 0x19 / 0x18, Rev2 = the 0x2F C1/C0 walk (Q-AP0 §2.1). Correction (2026-10-20, AP-3.3): the clone called the Rev1 builder directly, which
// is right only for the FW1814; it now calls the device, as the reference does.
// Two-phase sequence ([+0x348] = 1, FW1814 SetupAudioEngine @0xed73):
//   1. TX program Connect(0)  2. IOSleep(500)  3. kick INPUT (dir 1)  4. IOSleep(500)  5. RX program Connect(0); on failure TX Disconnect
//   6. kick OUTPUT (dir 0), no sleep after.
// One-phase sequence ([+0x348] = 0: the Audiophile and the other Rev2 models):
//   0. kick INPUT (dir 1), IOSleep(500)  1. TX Connect(0)  2. IOSleep(500)  3. kick OUTPUT (dir 0)  4. IOSleep(500)  5. RX Connect(0).
// The DCL programs are STARTED later (StartAudio -> StartPCMAudio: RX Start then TX Start, Q-START).
#ifndef OA_FWSTREAMCONTROL_H
#define OA_FWSTREAMCONTROL_H

#include "FWAvc.h"
#include "FWDCLProgram.h"

class com_openaudio_FWAVCAudioDevice;

class FWStreamControl
{
public:
    FWStreamControl() : fDevice(0), fTx(0), fRx(0), fFdf(0), fTwoPhase(true), fConnected(false) {}
    void Configure(com_openaudio_FWAVCAudioDevice *device, com_openaudio_FWDCLOutputProgram *tx, com_openaudio_FWDCLInputProgram *rx,
                   UInt8 fdfIndex, bool twoPhase)
    { fDevice = device; fTx = tx; fRx = rx; fFdf = fdfIndex; fTwoPhase = twoPhase; }
    void SetFdf(UInt8 fdfIndex) { fFdf = fdfIndex; }          // ref engine [+0x228], rewritten per rate by PerformSampleRateChangeCore

    // ref ConnectAllPlugs @0x12476: refuses when already connected (0xE00002BC) or when the bandwidth check fails (0xE00002BE).
    IOReturn ConnectAllPlugs(UInt32 bandwidthBytesPerCycle);
    // ref ConnectAudioMIDIChannels @0x12156
    IOReturn ConnectAudioMIDIChannels();
    // ref DisconnectAllPlugs @0x11bfa (decomp FWEngine2__DisconnectAllPlugs.c)
    void     DisconnectAllPlugs();
    bool     isConnected() const { return fConnected; }

    // ref SufficientBandwidthAvailable @0x22e8: fails when bytes/cycle >= 0x186b (6251).
    static bool SufficientBandwidthAvailable(UInt32 bytesPerCycle) { return bytesPerCycle < 0x186b; }

private:
    com_openaudio_FWAVCAudioDevice   *fDevice;   // ref engine [+0x138] cast to FWAVCAudioDevice
    com_openaudio_FWDCLOutputProgram *fTx;
    com_openaudio_FWDCLInputProgram  *fRx;
    UInt8 fFdf;
    bool  fTwoPhase;       // ref engine [+0x348]
    bool  fConnected;      // ref engine [+0x2d3]
};

#endif
