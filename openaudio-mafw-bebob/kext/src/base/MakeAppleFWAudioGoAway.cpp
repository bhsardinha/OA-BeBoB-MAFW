#include "MakeAppleFWAudioGoAway.h"
#include <libkern/c++/OSString.h>

#define super IOService
OSDefineMetaClassAndStructors(com_openaudio_MakeAppleFWAudioGoAway, IOService)

// ref @0x129cc: copy provider's "FireWire Product Name", compare with our own personality's copy; equal -> 0x7FFFFF.
IOService *com_openaudio_MakeAppleFWAudioGoAway::probe(IOService *provider, SInt32 *score)
{
    *score = 0;
    OSObject *theirs = provider ? provider->getProperty("FireWire Product Name") : 0;
    OSObject *mine   = getProperty("FireWire Product Name");
    if (!theirs || !mine || !theirs->isEqualTo(mine)) return 0;
    *score = 0x7FFFFF;
    return this;
}
