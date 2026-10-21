// FWTimeSlaveEngine -- an IOAudioEngine that has no clock of its own: it shares the master engine's status page and forwards start /
// stop / sample-frame queries (A1 Q-MJ; decomp FWTimeSlaveEngine__*.c). Mirrors m_audio_b_FWTimeSlaveEngine.
// 10.13 detail: IOAudioEngine keeps the shared status page in `status` and (protected) `reserved->statusDescriptor`; both are pointed at the
// master's page, exactly what the reference does with its [+0xb0] / [+0x110]->[+8].
#ifndef OA_FWTIMESLAVEENGINE_H
#define OA_FWTIMESLAVEENGINE_H

#include <IOKit/audio/IOAudioEngine.h>

class com_openaudio_FWDiscreteEngine;

class com_openaudio_FWTimeSlaveEngine : public IOAudioEngine
{
    OSDeclareAbstractStructors(com_openaudio_FWTimeSlaveEngine)
public:
    void SetMasterEngine(com_openaudio_FWDiscreteEngine *master);              // ref @0x19e8a
    virtual UInt32 getCurrentSampleFrame() APPLE_KEXT_OVERRIDE;                // ref @0x19e06 -> master
    virtual IOReturn performAudioEngineStart() APPLE_KEXT_OVERRIDE;            // ref @0x19dd6 -> master (the pair engine overrides it)
    virtual IOReturn performAudioEngineStop() APPLE_KEXT_OVERRIDE;             // ref @0x19dee
    virtual void free() APPLE_KEXT_OVERRIDE;                                   // ref @0x19d8c
    com_openaudio_FWDiscreteEngine *master() const { return fMaster; }
protected:
    com_openaudio_FWDiscreteEngine *fMaster;     // [+0x118]
    IOBufferMemoryDescriptor *fStatusDesc;       // [+0x120] retained master status descriptor
};

#endif
