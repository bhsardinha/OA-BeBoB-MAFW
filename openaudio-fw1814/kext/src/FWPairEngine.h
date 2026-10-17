// FWPairEngine / FWPairStream -- the stereo-pair engines exported next to the multichannel engine (A1 Q13, Q-MJ; decomp FWPairEngine__*.c,
// FWPairStream__*.c, FWAudioDevice__{CreateStereoPairSubEngines,NameAndActivatePairEngines}.c). One pair engine per (signal type, pair
// index) with an input and/or an output stream of two float channels; the multichannel engine owns the real buffers and the transport.
// Engine bit: 1 << fIndex in the master's sub-engine masks (bit 0 = the main engine).
#ifndef OA_FWPAIRENGINE_H
#define OA_FWPAIRENGINE_H

#include "FWTimeSlaveEngine.h"
#include "FWBaseEngine.h"
#include <IOKit/audio/IOAudioStream.h>

class com_openaudio_FWPairStream : public IOAudioStream
{
    OSDeclareDefaultStructors(com_openaudio_FWPairStream)
public:
    // ref @0x1b59e: the stream hands mixing to its engine (the pair engine adds the pair into the master's mix buffer)
    virtual IOReturn mixOutputSamples(const void *sourceBuf, void *mixBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames,
                                      const IOAudioStreamFormat *streamFormat, IOAudioStream *audioStream) APPLE_KEXT_OVERRIDE;
};

class com_openaudio_FWPairEngine : public com_openaudio_FWTimeSlaveEngine
{
    OSDeclareDefaultStructors(com_openaudio_FWPairEngine)
public:
    virtual bool init(OSDictionary *properties) APPLE_KEXT_OVERRIDE;           // ref @0x1afb8
    virtual bool initHardware(IOService *provider) APPLE_KEXT_OVERRIDE;        // ref @0x1aff8
    virtual void free() APPLE_KEXT_OVERRIDE;                                   // ref @0x1ab30
    virtual IOReturn performAudioEngineStart() APPLE_KEXT_OVERRIDE;            // ref @0x1ae44: master->ActivateSubEngine(bit)
    virtual IOReturn performAudioEngineStop() APPLE_KEXT_OVERRIDE;             // ref @0x1ae22: master->DeactivateSubEngine(bit)
    virtual void stopEngineAtPosition(IOAudioEnginePosition *pos) APPLE_KEXT_OVERRIDE;   // ref @0x1ace6
    virtual IOReturn performFormatChange(IOAudioStream *s, const IOAudioStreamFormat *f, const IOAudioSampleRate *r) APPLE_KEXT_OVERRIDE { return kIOReturnSuccess; }   // ref @0x1ad32
    virtual void resetStatusBuffer() APPLE_KEXT_OVERRIDE {}                    // ref @0x1ad3a: the status page is the master's
    virtual IOAudioEngineState setState(IOAudioEngineState newState) APPLE_KEXT_OVERRIDE;   // ref @0x1ad40: state + property only
    virtual IOReturn clipOutputSamples(const void *mixBuf, void *sampleBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames,
                                       const IOAudioStreamFormat *fmt, IOAudioStream *stream) APPLE_KEXT_OVERRIDE;   // ref @0x1af04
    virtual IOReturn convertInputSamples(const void *sampleBuf, void *destBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames,
                                         const IOAudioStreamFormat *fmt, IOAudioStream *stream) APPLE_KEXT_OVERRIDE; // ref @0x1ae64
    virtual IOReturn mixOutputSamples(const void *sourceBuf, void *mixBuf, UInt32 firstSampleFrame, UInt32 numSampleFrames,
                                      const IOAudioStreamFormat *fmt, IOAudioStream *stream) APPLE_KEXT_OVERRIDE;    // ref @0x1af32

    // ref ResetIOFormat @0x1ab42 (vt[0xbf8]): re-apply the master's rate to both streams and copy the master's latency properties
    void ResetIOFormat(const IOAudioSampleRate *rate);
    // set by CreateStereoPairSubEngines
    void SetIndex(UInt32 index) { fIndex = index; }                            // [+0x12c]: set once, at creation
    void AddDirection(bool isOutput, UInt32 channelOffset);                    // [+0x17d]/[+0x130] output, [+0x17c]/[+0x134] input
    UInt32 bit() const { return 1u << (fIndex & 0x1f); }
    bool hasInput() const  { return fHasIn; }
    bool hasOutput() const { return fHasOut; }
private:
    com_openaudio_FWBaseEngine *base() const;                                   // the master as FWBaseEngine
    bool fIniting;                         // [+0x128]
    UInt32 fIndex;                         // [+0x12c]
    UInt32 fOutOffset, fInOffset;          // [+0x130] [+0x134]: first channel of this pair in the multichannel buffers
    IOAudioStream *fOutStream, *fInStream; // [+0x138] [+0x140]
    TRingBuffer fRing;                     // [+0x148]
    UInt32 fRingBytes;                     // [+0x178]
    bool fHasIn, fHasOut;                  // [+0x17c] [+0x17d]
};

#endif
