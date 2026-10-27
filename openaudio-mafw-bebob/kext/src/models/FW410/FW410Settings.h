// FW410Settings -- FWSettings410Levels (module models/FW410): the generic levels object with the 410's register writers for input pan, input
// routing and SW-return routing. Reference: FWSettings410Levels (vtable @0x3c3b0: SetInputPan +0x58, SetInputRouting +0x68, SetSWReturnRouting
// +0x70, BatchSetSWReturnRouting +0x78 forward to com_m_audio_FW410Device vt 0xbd0 / 0xbc8 / 0xbd8 / 0xbe0; Init @0x1437a installs the
// batch functor (tag 0x79 = vt+0x78) on MASWReturnRoutingV2 [+0xa38]; SendToDevice @0x14046 = base, then vt+0x78).
#ifndef OA_FW410SETTINGS_H
#define OA_FW410SETTINGS_H

#include "FWSettings.h"

class com_openaudio_FWSettings410Levels : public com_openaudio_FWSettingsLevels
{
    OSDeclareDefaultStructors(com_openaudio_FWSettings410Levels)
public:
    IOReturn Init410(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *swReturn, FWChannelGroup *output, FWChannelGroup *input,
                     FWChannelGroup *headphone, FWChannelGroup *auxOut);
    virtual IOReturn SendToDevice() APPLE_KEXT_OVERRIDE;
    virtual IOReturn SetInputPan(UInt32 idx, UInt32 v) APPLE_KEXT_OVERRIDE;
    virtual IOReturn SetInputRouting(UInt32 idx, UInt32 v) APPLE_KEXT_OVERRIDE;
    virtual IOReturn SetSWReturnRouting(UInt32 idx, UInt32 v) APPLE_KEXT_OVERRIDE;
    virtual void BatchSetSWReturnRouting() APPLE_KEXT_OVERRIDE;
};

#endif
