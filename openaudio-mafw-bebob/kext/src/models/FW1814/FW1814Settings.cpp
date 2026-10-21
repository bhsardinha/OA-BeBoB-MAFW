#include "FW1814Settings.h"
#include "FW1814Device.h"

static const IOReturn kErrBadArg = (IOReturn)0xE00002C2;

OSDefineMetaClassAndStructors(com_openaudio_FWSettings1814Levels, com_openaudio_FWSettingsLevels)
OSDefineMetaClassAndStructors(com_openaudio_FWSettings1814AuxSend, com_openaudio_FWSettingsAuxSend)

// ref FWSettings1814Levels::SetInputPan @0xd128 / SetInputRouting @0xd15c: safeMetaCast of the device to the FW1814 class, then its register writer
IOReturn com_openaudio_FWSettings1814Levels::SetInputPan(UInt32 idx, UInt32 v)
{
    com_openaudio_FW1814Device *d = OSDynamicCast(com_openaudio_FW1814Device, fDevice);
    return d ? d->SetInputPan(idx, v) : kErrBadArg;
}
IOReturn com_openaudio_FWSettings1814Levels::SetInputRouting(UInt32 idx, UInt32 v)
{
    com_openaudio_FW1814Device *d = OSDynamicCast(com_openaudio_FW1814Device, fDevice);
    return d ? d->SetInputRouting(idx, v) : kErrBadArg;
}

// ref FWSettings1814AuxSend::SetAuxMonitor @0xd0b4 -> FW1814Device vt[0xbc8]
IOReturn com_openaudio_FWSettings1814AuxSend::SetAuxMonitor(UInt32 idx, UInt32 v)
{
    com_openaudio_FW1814Device *d = OSDynamicCast(com_openaudio_FW1814Device, fDevice);
    return d ? d->SetAuxMonitor(idx, v) : kErrBadArg;
}
