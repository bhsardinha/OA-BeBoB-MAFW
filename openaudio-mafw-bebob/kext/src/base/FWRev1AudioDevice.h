// FWRev1AudioDevice module (base) -- reference FWRev1AudioDevice (Q16 Correction 2026-10-14, Q3a). Models: FW1814 (and ProjectMix through it).
// Differs from Rev2 only in the stream-format command (plain 8-byte INPUT/OUTPUT PLUG SIGNAL FORMAT 0x18/0x19, @0x13734) and the
// round-trip latency table (@0x136da). The FW1814 firmware freezes on the Rev2 0x2F command (snd-bebob bebob_maudio.c:30-33).
#ifndef OA_FWREV1AUDIODEVICE_H
#define OA_FWREV1AUDIODEVICE_H

#include "FWAVCAudioDevice.h"

// Rev1: plain 8-byte 0x18/0x19 AV/C stream-format command (Q3a). Nothing to override until M-C.
class com_openaudio_FWRev1AudioDevice : public com_openaudio_FWAVCAudioDevice
{
    OSDeclareAbstractStructors(com_openaudio_FWRev1AudioDevice)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual IOReturn AVCControlPlugSignalFormat(bool input, UInt8 plug, UInt8 fdf) APPLE_KEXT_OVERRIDE;   // ref @0x13734 (FWAvc.cpp)
    virtual UInt32 GetRoundTripLatencyForFDF(UInt32 fdf) const APPLE_KEXT_OVERRIDE { return FWRev1RoundTripLatencyForFDF(fdf); }   // ref @0x136da
};

#endif
