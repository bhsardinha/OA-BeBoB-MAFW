// FW410Settings (models/FW410) -- see FW410Settings.h.
#include "FW410Settings.h"
#include "FW410Device.h"
#include "FWPropertyDict.h"

#define super com_openaudio_FWSettingsLevels
OSDefineMetaClassAndStructors(com_openaudio_FWSettings410Levels, com_openaudio_FWSettingsLevels)
#undef super

static com_openaudio_FW410Device *Dev410(com_openaudio_FWAVCAudioDevice *d) { return OSDynamicCast(com_openaudio_FW410Device, d); }   // ref safeMetaCast

IOReturn com_openaudio_FWSettings410Levels::Init410(com_openaudio_FWAVCAudioDevice *device, FWChannelGroup *swReturn, FWChannelGroup *output,
                                                    FWChannelGroup *input, FWChannelGroup *headphone, FWChannelGroup *auxOut)
{
    IOReturn r = Init(device, swReturn, output, input, headphone, auxOut);
    if (r != kIOReturnSuccess) return r;
    typedef com_openaudio_FWSettingsLevels L;
    if (fSWReturnRouting) fSWReturnRouting->SetAllChanged(FWBind0<L, &L::BatchSetSWReturnRouting>(this));   // ref [+0xa38] tag 0x79
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWSettings410Levels::SendToDevice()
{
    IOReturn r = com_openaudio_FWSettingsLevels::SendToDevice();
    if (r == kIOReturnSuccess) BatchSetSWReturnRouting();
    return r;
}

IOReturn com_openaudio_FWSettings410Levels::SetInputPan(UInt32 idx, UInt32 v)
{ com_openaudio_FW410Device *d = Dev410(fDevice); return d ? d->SetInputPan(idx, v) : kIOReturnSuccess; }
IOReturn com_openaudio_FWSettings410Levels::SetInputRouting(UInt32 idx, UInt32 v)
{ com_openaudio_FW410Device *d = Dev410(fDevice); return d ? d->SetInputRouting(idx, v) : kIOReturnSuccess; }
IOReturn com_openaudio_FWSettings410Levels::SetSWReturnRouting(UInt32 idx, UInt32 v)
{ com_openaudio_FW410Device *d = Dev410(fDevice); return d ? d->SetSWReturnRouting(idx, v) : kIOReturnSuccess; }
void com_openaudio_FWSettings410Levels::BatchSetSWReturnRouting()
{ com_openaudio_FW410Device *d = Dev410(fDevice); if (d) d->BatchSetSWReturnRouting(); }
