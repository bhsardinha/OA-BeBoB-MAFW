// FWRev1AudioDevice module (base) -- see FWRev1AudioDevice.h.
#include "FWRev1AudioDevice.h"
#include "FWAvc.h"

#define super com_openaudio_FWAVCAudioDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWRev1AudioDevice, com_openaudio_FWAVCAudioDevice)
#undef super

void com_openaudio_FWRev1AudioDevice::free() { com_openaudio_FWAVCAudioDevice::free(); }

// ref FWRev1AudioDevice::AVCControlPlugSignalFormat @0x13734
IOReturn com_openaudio_FWRev1AudioDevice::AVCControlPlugSignalFormat(bool input, UInt8 plug, UInt8 fdf)
{
    return FWAvcControlPlugSignalFormat(fAvcUnit, input, plug, fdf);
}
