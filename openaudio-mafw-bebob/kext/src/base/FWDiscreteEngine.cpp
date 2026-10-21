#include "FWDiscreteEngine.h"
#include "FWAudioDeviceChain.h"
#include "Trace.h"
#include "FWPairEngine.h"
#include <libkern/c++/OSCollectionIterator.h>

#define super IOAudioEngine
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWDiscreteEngine, IOAudioEngine)

static const IOReturn kErrBadArg = (IOReturn)0xE00002C2;

// ref @0x18824: a main engine releases its own status buffer holder; clone: nothing extra to release.
void com_openaudio_FWDiscreteEngine::free()
{
    super::free();
}

void com_openaudio_FWDiscreteEngine::stop(IOService *provider)
{
    super::stop(provider);
    if (fSubEngines) { fSubEngines->release(); fSubEngines = 0; }
}

// ref clipOutputSamples @0x188c6: the IOAudioEngine call is the main engine's request (mask bit 0); mixBuf / sampleBuf are ignored,
// the main engine fetches its own buffers.
IOReturn com_openaudio_FWDiscreteEngine::clipOutputSamples(const void *, void *, UInt32 first, UInt32 num,
                                                           const IOAudioStreamFormat *fmt, IOAudioStream *stream)
{
    ClipSubEngineOutput(1, first, num, fmt, stream);
    return kIOReturnSuccess;
}

// ref ClipSubEngineOutput @0x18884
IOReturn com_openaudio_FWDiscreteEngine::ClipSubEngineOutput(UInt32 mask, UInt32 first, UInt32 num,
                                                             const IOAudioStreamFormat *fmt, IOAudioStream *stream)
{
    const UInt32 seen = fClipMask | mask;
    fClipMask = seen;
    if ((seen & fActiveMask) != fActiveMask) return kIOReturnSuccess;        // wait for the other active sub-engines
    fClipMask = 0;
    return MainEngineClipOutputSamples(first, num, fmt, stream);             // vt[0xc38]
}

// ref convertInputSamples @0x18aa6: convert once per period (ConvertSubEngineInput), then copy the float result to CoreAudio's buffer.
// destBuf is sized for numSampleFrames only (memory reference_convert_input_samples; the converted buffer is filled from index 0).
IOReturn com_openaudio_FWDiscreteEngine::convertInputSamples(const void *, void *destBuf, UInt32 first, UInt32 num,
                                                             const IOAudioStreamFormat *fmt, IOAudioStream *)
{
    void *conv = GetMultichannelInputBuffer();                               // vt[0xc10]
    ConvertSubEngineInput(1, first, num);
    if (conv && destBuf && fmt) memcpy(destBuf, conv, (size_t)fmt->fNumChannels * num * 4);
    return kIOReturnSuccess;
}

// ref ConvertSubEngineInput @0x188e4
void com_openaudio_FWDiscreteEngine::ConvertSubEngineInput(UInt32 mask, UInt32 first, UInt32 num)
{
    UInt32 seen = fConvertMask;
    if (seen == 0) ConvertInputSamplesCore(first, num);                      // vt[0xc40]
    seen |= mask;
    fConvertMask = ((seen & fActiveMask) == fActiveMask) ? 0 : seen;
}

// ref ActivateSubEngine @0x18c44 (Q-MD §2). The reference's `IOAudioEngine vtable + 0xa38` is a non-virtual call of
// IOAudioEngine::resetStatusBuffer (raw offset - 0x10 = slot 0xa28).
IOReturn com_openaudio_FWDiscreteEngine::ActivateSubEngine(UInt32 mask)
{
    const UInt32 old = fActiveMask;
    com_openaudio_FWDiscreteDevice *dev = OSDynamicCast(com_openaudio_FWDiscreteDevice, audioDevice);
    if (!dev) return kErrBadArg;
    fActiveMask |= mask;
    if (((fActiveMask != 0) && old == 0) || state == kIOAudioEngineResumed) {
        if (mask != 1) dev->audioEngineStarting();
        IOAudioEngine::resetStatusBuffer();
        if (state != kIOAudioEnginePaused) {
            IOReturn r = StartIsochStreams();
            if (r != kIOReturnSuccess) {
                if (mask != 1) dev->audioEngineStopped();
                return r;
            }
            if (mask == 1) return kIOReturnSuccess;
            setState(kIOAudioEngineRunning);
            sendNotification(kIOAudioEngineStartedNotification);
            return kIOReturnSuccess;
        }
    }
    return kIOReturnSuccess;
}

// ref DeactivateSubEngine @0x18b1e
void com_openaudio_FWDiscreteEngine::DeactivateSubEngine(UInt32 mask)
{
    const UInt32 old = fActiveMask;
    com_openaudio_FWDiscreteDevice *dev = OSDynamicCast(com_openaudio_FWDiscreteDevice, audioDevice);
    if (!dev) return;
    fActiveMask &= ~mask;
    if (fActiveMask == 0 && old != 0) {
        StopIsochStreams();
        if (mask != 1) {
            setState(kIOAudioEngineStopped);
            sendNotification(kIOAudioEngineStoppedNotification);
            dev->audioEngineStopped();
        }
    }
}

// ref stopAudioEngine @0x18d8c
IOReturn com_openaudio_FWDiscreteEngine::stopAudioEngine()
{
    IOReturn r = kIOReturnSuccess;
    const IOAudioEngineState s = getState();
    if (s == kIOAudioEngineRunning) {
        // Correction (2026-10-16, O1): the reference calls vt[0xa68] here (mafw.asm 0x18dba..0x18dc0) = performAudioEngineStop (vtable order:
        // pause 0xa50, resume 0xa58, performStart 0xa60, performStop 0xa68, getState 0xa70). The clone called pauseAudioEngine(), which bumped
        // FWBaseEngine's nesting counter ([+0x234]) on every play/stop and never released it; after a few cycles the pause around a clock-source
        // change was a silent no-op, the isoch streams kept running and UpdateDCLProgramsWithNewPacketSizes hit "IsochStreamRunning".
        r = performAudioEngineStop();
        if (r != kIOReturnSuccess) return r;
    } else if (s != kIOAudioEngineResumed) {
        return kIOReturnSuccess;
    }
    if (fActiveMask < 2) {
        com_openaudio_FWDiscreteDevice *dev = OSDynamicCast(com_openaudio_FWDiscreteDevice, audioDevice);
        setState(kIOAudioEngineStopped);
        sendNotification(kIOAudioEngineStoppedNotification);
        if (dev) dev->audioEngineStopped();
    }
    return r;
}

// ref stopEngineAtPosition @0x18786
void com_openaudio_FWDiscreteEngine::stopEngineAtPosition(IOAudioEnginePosition *pos)
{
    if (pos) audioEngineStopPosition = *pos;
    else     { audioEngineStopPosition.fLoopCount = 0; audioEngineStopPosition.fSampleFrame = 0; }
}

// ref performFlush @0x18bbc
void com_openaudio_FWDiscreteEngine::performFlush()
{
    if (numActiveUserClients != 0) return;                      // ref [+0x100] == 0 (offset verified against the 10.13 IOAudioEngine layout)
    if (getState() != kIOAudioEngineRunning) return;
    if (fActiveMask < 2) {
        const UInt32 loop = status->fCurrentLoopCount;
        const UInt32 frame = getCurrentSampleFrame();
        if (audioEngineStopPosition.fLoopCount < loop ||
            (loop == audioEngineStopPosition.fLoopCount && audioEngineStopPosition.fSampleFrame < frame))
            stopAudioEngine();
    } else if (fActiveMask & 1) {
        DeactivateSubEngine(1);
    }
}

// ref AddSubEngine @0x18f0c / RemoveSubEngine @0x186bc
void com_openaudio_FWDiscreteEngine::AddSubEngine(OSObject *pair)
{
    if (!fSubEngines) fSubEngines = OSSet::withCapacity(4);
    if (fSubEngines) fSubEngines->setObject(pair);
}
void com_openaudio_FWDiscreteEngine::RemoveSubEngine(OSObject *pair)
{
    if (fSubEngines) fSubEngines->removeObject(pair);
}

// ref RemoveAllSubEngines @0x189a6: terminate every pair engine, empty the set
void com_openaudio_FWDiscreteEngine::RemoveAllSubEngines()
{
    if (!fSubEngines) return;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(fSubEngines);
    if (it) {
        while (IOService *s = OSDynamicCast(IOService, it->getNextObject())) s->terminate(0);
        it->release();
    }
    fSubEngines->flushCollection();
}

// ref ResetStereoPairIOFormats @0x18a2a: every pair engine re-applies this engine's rate
void com_openaudio_FWDiscreteEngine::ResetStereoPairIOFormats()
{
    if (!fSubEngines) return;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(fSubEngines);
    if (!it) return;
    while (com_openaudio_FWPairEngine *p = OSDynamicCast(com_openaudio_FWPairEngine, it->getNextObject())) p->ResetIOFormat(&sampleRate);
    it->release();
}
