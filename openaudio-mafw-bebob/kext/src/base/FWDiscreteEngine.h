// FWDiscreteEngine -- IOAudioEngine layer between IOAudioEngine and FWBaseEngine. Mirrors m_audio_b_FWDiscreteEngine
// (A1 Q13 + Q-MD; decomp FWDiscreteEngine__*.c). Class chain (verified from the FWBaseEngine constructor @0x633a, which
// calls FWDiscreteEngine's): IOAudioEngine -> FWDiscreteEngine -> FWBaseEngine -> FWEngine2. (Q13's "FWEngine2 derives
// directly from FWBaseEngine" is right, but FWBaseEngine itself derives from FWDiscreteEngine.)
// The sub-engine mask [+0x128] is the set of stereo-pair/main engines that want the shared transport; bit 0 = the main engine.
#ifndef OA_FWDISCRETEENGINE_H
#define OA_FWDISCRETEENGINE_H

#include <IOKit/audio/IOAudioEngine.h>
#include <IOKit/audio/IOAudioDevice.h>
#include <libkern/c++/OSArray.h>
#include <libkern/c++/OSSet.h>

class com_openaudio_FWDiscreteEngine : public IOAudioEngine
{
    OSDeclareAbstractStructors(com_openaudio_FWDiscreteEngine)
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;                 // ref @0x167c: super::stop, drop sub-engine array
    // pauseAudioEngine @0x187b8 / resumeAudioEngine @0x187f0 are used by the pair engines only: FWBaseEngine overrides both and
    // calls IOAudioEngine's directly (decomp FWBaseEngine__pauseAudioEngine.c, GOT call), so they arrive with M-J.
    virtual IOReturn stopAudioEngine() APPLE_KEXT_OVERRIDE;                     // ref @0x18d8c
    virtual void stopEngineAtPosition(IOAudioEnginePosition *pos) APPLE_KEXT_OVERRIDE;   // ref @0x18786
    virtual void performFlush() APPLE_KEXT_OVERRIDE;                            // ref @0x18bbc
    virtual void resetStatusBuffer() APPLE_KEXT_OVERRIDE {}                     // ref @0x18878: intentionally empty

    // ref ActivateSubEngine @0x18c44 / DeactivateSubEngine @0x18b1e. mask bit 0 = main engine.
    IOReturn ActivateSubEngine(UInt32 mask);
    void     DeactivateSubEngine(UInt32 mask);
    // Transport hooks (ref vt[0xc18] / vt[0xc20] = StartIsochStreams / StopIsochStreams).
    virtual IOReturn StartIsochStreams() = 0;
    virtual void     StopIsochStreams() = 0;
    // ---- sample path (M-D2). The IOAudioEngine entry points collect the sub-engine masks and call the main engine once all
    // active sub-engines have asked (Q-MD2 §2). ----
    virtual IOReturn clipOutputSamples(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames,
                                       const IOAudioStreamFormat *fmt, IOAudioStream *stream) APPLE_KEXT_OVERRIDE;   // ref @0x188c6
    virtual IOReturn convertInputSamples(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames,
                                         const IOAudioStreamFormat *fmt, IOAudioStream *stream) APPLE_KEXT_OVERRIDE; // ref @0x18aa6
    IOReturn ClipSubEngineOutput(UInt32 mask, UInt32 first, UInt32 num, const IOAudioStreamFormat *fmt, IOAudioStream *stream);   // ref @0x18884
    void     ConvertSubEngineInput(UInt32 mask, UInt32 first, UInt32 num);                                                       // ref @0x188e4
    virtual IOReturn MainEngineClipOutputSamples(UInt32 first, UInt32 num, const IOAudioStreamFormat *fmt, IOAudioStream *stream) = 0;   // vt[0xc38]
    virtual void     ConvertInputSamplesCore(UInt32 first, UInt32 num) = 0;                                                      // vt[0xc40]
    virtual void    *GetMultichannelInputBuffer() = 0;                                                                           // vt[0xc10] (ref @0x1178a)

    // ref ResetStereoPairIOFormats @0x18a2a: re-applies the current rate to every pair engine (M-J).
    virtual void ResetStereoPairIOFormats();
    // ref RemoveAllSubEngines @0x189a6 (M-J): the pair-engine array is dropped here; detaching each pair engine arrives with M-J.
    void RemoveAllSubEngines();
    void AddSubEngine(OSObject *pairEngine);                                    // ref AddSubEngine @0x18f0c
    void RemoveSubEngine(OSObject *pairEngine);                                 // ref RemoveSubEngine @0x186bc

protected:
    OSSet   *fSubEngines;            // [+0x118] FWPairEngine collection (NULL until the first pair engine)
    UInt32   fActiveMask;            // [+0x128]
    UInt32   fClipMask;              // [+0x12c] sub-engines that already asked for the current clip period
    UInt32   fConvertMask;           // [+0x130] sub-engines that already took the converted input of the current period
    UInt32   fSavedMask;             // [+0x134] mask saved across pause (pair engines, M-J)
};

#endif
