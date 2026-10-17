#include "FWStreamControl.h"
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

// ref ConnectAudioMIDIChannels @0x12156 (decomp FWEngine2__ConnectAudioMIDIChannels.c). The kick return values are ignored,
// as in the reference. 100 (the last vt[0xb50] argument) and the channel-count arguments are ignored by Rev1 (Q3a §3).
IOReturn FWStreamControl::ConnectAudioMIDIChannels()
{
    if (!fTx || !fRx) return kIOReturnNotReady;
    if (!fTwoPhase) {                                         // [+0x348] == 0: kick INPUT first
        FWAvcControlPlugSignalFormat(fUnit, true, 0, fFdf);
        IOSleep(500);
    }
    IOReturn r = fTx->Connect(0);
    if (r != kIOReturnSuccess) return r;
    IOSleep(500);
    FWAvcControlPlugSignalFormat(fUnit, fTwoPhase /* [+0x348] != 0 -> INPUT 0x19; else OUTPUT 0x18 */, 0, fFdf);
    IOSleep(500);
    r = fRx->Connect(0);
    if (r == kIOReturnSuccess) {
        if (fTwoPhase) FWAvcControlPlugSignalFormat(fUnit, false, 0, fFdf);   // OUTPUT 0x18, last, no sleep
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
