// FWStreamControl -- the reference stream-connect sequence. Mirrors com_m_audio_FWEngine2::ConnectAudioMIDIChannels @0x12156
// (decomp FWEngine2__ConnectAudioMIDIChannels.c), ::ConnectAllPlugs @0x12476 and FWAudioDevice::SufficientBandwidthAvailable
// @0x22e8. Evidence + corrected order: docs/system-map/A1-ghidra/Q-MC-cmp-isoch-avc-start.md.
//
// FW1814 sequence ([+0x348] = 1, set by SetupAudioEngine @0xed73; the always-taken branch for this device):
//   1. TX program Connect(0)                         (IRM channel, local DCL port, device iPCR CMP)
//   2. IOSleep(500)
//   3. AV/C  00 FF 19 00 90 <FDF> FF FF               INPUT PLUG SIGNAL FORMAT
//   4. IOSleep(500)
//   5. RX program Connect(0); on failure -> TX Disconnect, return the error
//   6. AV/C  00 FF 18 00 90 <FDF> FF FF               OUTPUT PLUG SIGNAL FORMAT          (no sleep after)
// The DCL programs are STARTED later (StartAudio -> StartPCMAudio: RX Start then TX Start, Q-START).
// The `flag == 0` variant (kick INPUT first, no final OUTPUT) belongs to other products and is ported for completeness.
#ifndef OA_FWSTREAMCONTROL_H
#define OA_FWSTREAMCONTROL_H

#include "FWAvc.h"
#include "FWDCLProgram.h"

class FWStreamControl
{
public:
    FWStreamControl() : fUnit(0), fTx(0), fRx(0), fFdf(0), fTwoPhase(true), fConnected(false) {}
    void Configure(com_openaudio_FWAvcUnit *unit, com_openaudio_FWDCLOutputProgram *tx, com_openaudio_FWDCLInputProgram *rx,
                   UInt8 fdfIndex, bool twoPhase = true)
    { fUnit = unit; fTx = tx; fRx = rx; fFdf = fdfIndex; fTwoPhase = twoPhase; }
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
    com_openaudio_FWAvcUnit          *fUnit;
    com_openaudio_FWDCLOutputProgram *fTx;
    com_openaudio_FWDCLInputProgram  *fRx;
    UInt8 fFdf;
    bool  fTwoPhase;       // ref engine [+0x348]
    bool  fConnected;      // ref engine [+0x2d3]
};

#endif
