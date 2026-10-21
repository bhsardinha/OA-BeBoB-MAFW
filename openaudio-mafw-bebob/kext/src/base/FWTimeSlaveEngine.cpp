#include "FWTimeSlaveEngine.h"
#include "FWDiscreteEngine.h"
#include "Trace.h"

#define super IOAudioEngine
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWTimeSlaveEngine, IOAudioEngine)

// ref SetMasterEngine @0x19e8a: share the master's status page (pointer + descriptor) and its buffer length
void com_openaudio_FWTimeSlaveEngine::SetMasterEngine(com_openaudio_FWDiscreteEngine *master)
{
    fMaster = master;
    IOBufferMemoryDescriptor *d = master->getStatusDescriptor();
    if (d) d->retain();
    if (reserved) {
        if (reserved->statusDescriptor) reserved->statusDescriptor->release();
        reserved->statusDescriptor = d;                                         // the reference stores the retained pointer here
    }
    fStatusDesc = d;
    status = master->status;                                                    // ref [+0xb0] = descriptor->getBytesNoCopy()
    setNumSampleFramesPerBuffer(master->numSampleFramesPerBuffer);              // ref vt[0xb40](master [+0x88])
}

UInt32 com_openaudio_FWTimeSlaveEngine::getCurrentSampleFrame() { return fMaster ? fMaster->getCurrentSampleFrame() : 0; }
IOReturn com_openaudio_FWTimeSlaveEngine::performAudioEngineStart() { if (fMaster) fMaster->performAudioEngineStart(); return kIOReturnSuccess; }
IOReturn com_openaudio_FWTimeSlaveEngine::performAudioEngineStop()  { if (fMaster) fMaster->performAudioEngineStop();  return kIOReturnSuccess; }

// ref free @0x19d8c: the page belongs to the master; never let IOAudioEngine free it
void com_openaudio_FWTimeSlaveEngine::free()
{
    status = 0;
    if (reserved && reserved->statusDescriptor == fStatusDesc) reserved->statusDescriptor = 0;
    if (fStatusDesc) { fStatusDesc->release(); fStatusDesc = 0; }
    super::free();
}
