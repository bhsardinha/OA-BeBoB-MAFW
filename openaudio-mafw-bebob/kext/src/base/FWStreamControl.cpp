#include "FWStreamControl.h"
#include "FWAVCAudioDevice.h"
#include "Trace.h"

static const IOReturn kErrBadArg  = (IOReturn)0xE00002BC;
static const IOReturn kErrNoBandw = (IOReturn)0xE00002BE;

// ref ConnectAllPlugs @0x12476
IOReturn FWStreamControl::ConnectAllPlugs(UInt32 bandwidthBytesPerCycle)
{
    if (fConnected) return kErrBadArg;
    if (!SufficientBandwidthAvailable(bandwidthBytesPerCycle)) return kErrNoBandw;
    IOReturn r = ConnectAudioMIDIChannels();
    fConnected = (r == kIOReturnSuccess);
    return r;
}

// ref ConnectAudioMIDIChannels @0x12156 (decomp FWEngine2__ConnectAudioMIDIChannels.c). The kick return values are ignored, as in the
// reference; the channel-count and 100 arguments of vt[0xb50] are read by neither revision (Rev1 Q3a §3; Rev2 asm 0x12e7e-0x1305a).
IOReturn FWStreamControl::ConnectAudioMIDIChannels()
{
    if (!fTx || !fRx) return kIOReturnNotReady;
    if (!fTwoPhase) {                                         // [+0x348] == 0: kick INPUT first
        if (fDevice) fDevice->AVCControlPlugSignalFormat(true, 0, fFdf);
        IOSleep(500);
    }
    IOReturn r = fTx->Connect(0);
    if (r != kIOReturnSuccess) return r;
    IOSleep(500);
    if (fDevice) fDevice->AVCControlPlugSignalFormat(fTwoPhase /* [+0x348] != 0 -> INPUT; else OUTPUT */, 0, fFdf);
    IOSleep(500);
    r = fRx->Connect(0);
    if (r == kIOReturnSuccess) {
        if (fTwoPhase && fDevice) fDevice->AVCControlPlugSignalFormat(false, 0, fFdf);   // OUTPUT, last, no sleep
    } else {
        fTx->Disconnect();                                    // ref vt[0x128] on [+0x350]
    }
    return r;
}

// ref DisconnectAllPlugs @0x11bfa: RX first, then TX.
void FWStreamControl::DisconnectAllPlugs()
{
    if (!fConnected) return;
    fConnected = false;
    if (fRx) fRx->Disconnect();
    if (fTx) fTx->Disconnect();
}
