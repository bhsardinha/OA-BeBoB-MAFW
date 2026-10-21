#include "FWIsochChannel.h"
#include "FWCMP.h"
#include "Trace.h"

static const IOReturn kErrBadArg = (IOReturn)0xE00002C2;

#define super IOFWIsochChannel
OSDefineMetaClassAndStructors(com_openaudio_FWIsochChannel, IOFWIsochChannel)
#undef super
#define super IOFWIsochPort
OSDefineMetaClassAndStructors(com_openaudio_FWRemoteIsochPort, IOFWIsochPort)
#undef super

void com_openaudio_FWIsochChannel::free()
{
    if (fTalker) { fTalker->release(); fTalker = 0; }          // ref @0x6dca
    IOFWIsochChannel::free();
}

IOReturn com_openaudio_FWIsochChannel::ReleaseTalker()
{
    if (fRunning) {                                            // ref: panic FWIsochChannel.cpp:25
        OA_LOG("FWIsochChannel::ReleaseTalker while running");
        return kErrBadArg;
    }
    if (fTalker) {
        fTalker->releasePort();                                // ref vt[0x128]
        fTalker->release();
        fTalker = 0;
    }
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWIsochChannel::setTalker(IOFWIsochPort *talker)
{
    if (!talker || fTalker == talker) return kErrBadArg;
    if (fRunning) {                                            // ref: panic FWIsochChannel.cpp:47
        OA_LOG("FWIsochChannel::setTalker while running");
        return kErrBadArg;
    }
    talker->retain();
    ReleaseTalker();
    return IOFWIsochChannel::setTalker(talker);
}

IOReturn com_openaudio_FWIsochChannel::start()
{
    if (fRunning) return kErrBadArg;
    IOReturn r = IOFWIsochChannel::start();
    fRunning = (r == kIOReturnSuccess);
    return r;
}

IOReturn com_openaudio_FWIsochChannel::stop()
{
    if (!fRunning) return kErrBadArg;
    IOReturn r = IOFWIsochChannel::stop();
    fRunning = false;
    return r;
}

// ---------------------------------------------------------------------------------------------------------------------

bool com_openaudio_FWRemoteIsochPort::init(com_openaudio_FWAvcConnectionManager *mgr, bool remoteIsInput, UInt32 plug)
{
    if (!IOFWIsochPort::init()) return false;
    fMgr = mgr;
    fRemoteIsInput = remoteIsInput;
    fPlug = plug;
    return true;
}

void com_openaudio_FWRemoteIsochPort::free()
{
    fMgr = 0;
    IOFWIsochPort::free();
}

IOReturn com_openaudio_FWRemoteIsochPort::getSupported(IOFWSpeed &maxSpeed, UInt64 &chanSupported)
{
    maxSpeed = (IOFWSpeed)0x7fffffff;                          // ref @0x183d8
    chanSupported = 0xffffffffffffffffULL;
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWRemoteIsochPort::allocatePort(IOFWSpeed speed, UInt32 chan)
{
    (void)speed;                                               // ref @0x183ee: the manager takes channel and plug only
    if (!fMgr) return kIOReturnNotReady;
    return fMgr->Connect(fRemoteIsInput, chan, fPlug);
}

IOReturn com_openaudio_FWRemoteIsochPort::releasePort()
{
    if (!fMgr) return kIOReturnNotReady;
    return fMgr->Disconnect(fRemoteIsInput, 0, fPlug);         // ref @0x18416
}
