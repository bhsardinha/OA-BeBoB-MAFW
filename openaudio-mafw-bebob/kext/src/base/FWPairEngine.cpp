#include "FWPairEngine.h"
#include "FWSamples.h"
#include "Trace.h"

#define super com_openaudio_FWTimeSlaveEngine
OSDefineMetaClassAndStructors(com_openaudio_FWPairEngine, com_openaudio_FWTimeSlaveEngine)
#undef super
#define super IOAudioStream
OSDefineMetaClassAndStructors(com_openaudio_FWPairStream, IOAudioStream)
#undef super

static const IOReturn kErrBadArg = (IOReturn)0xE00002C2;

IOReturn com_openaudio_FWPairStream::mixOutputSamples(const void *sourceBuf, void *mixBuf, UInt32 first, UInt32 num,
                                                      const IOAudioStreamFormat *fmt, IOAudioStream *stream)
{
    return audioEngine->mixOutputSamples(sourceBuf, mixBuf, first, num, fmt, stream);
}

com_openaudio_FWBaseEngine *com_openaudio_FWPairEngine::base() const { return OSDynamicCast(com_openaudio_FWBaseEngine, fMaster); }

void com_openaudio_FWPairEngine::AddDirection(bool isOutput, UInt32 channelOffset)
{
    if (isOutput) { fHasOut = true; fOutOffset = channelOffset; }
    else          { fHasIn = true;  fInOffset = channelOffset; }
}

// ref init @0x1afb8
OSString *com_openaudio_FWPairEngine::getGlobalUniqueID()
{
    OSString *local = getLocalUniqueID();
    char buf[96];
    snprintf(buf, sizeof(buf), "m_audio_b_FWPairEngine:%s", local ? local->getCStringNoCopy() : "");
    if (local) local->release();
    return OSString::withCString(buf);
}

bool com_openaudio_FWPairEngine::init(OSDictionary *properties)
{
    fIniting = true;
    fIndex = 0; fOutOffset = fInOffset = 0; fOutStream = fInStream = 0; fRingBytes = 0; fHasIn = fHasOut = false;
    bool ok = IOAudioEngine::init(properties);
    if (ok) fIniting = false;
    return ok;
}

// the 24-byte format of C_58 @0x2fc80: 2 ch, 'lpcm', 'sint', 24 bit in 32, high-byte aligned, big endian, mixable
static void PairFormat(IOAudioStreamFormat &f)
{
    bzero(&f, sizeof(f));
    f.fNumChannels = 2;
    f.fSampleFormat = kIOAudioStreamSampleFormatLinearPCM;
    f.fNumericRepresentation = kIOAudioStreamNumericRepresentationSignedInt;
    f.fBitDepth = 0x18; f.fBitWidth = 0x20;
    f.fAlignment = kIOAudioStreamAlignmentHighByte;
    f.fByteOrder = kIOAudioStreamByteOrderBigEndian;
    f.fIsMixable = 1;
}

// ref initHardware @0x1aff8. The streams' sample buffer is a private ring of 8 bytes per frame (never read: convertInputSamples writes the
// client buffer directly); the output stream's MIX buffer is the master's multichannel mix buffer.
bool com_openaudio_FWPairEngine::initHardware(IOService *provider)
{
    if (!fMaster) return false;
    if (!IOAudioEngine::initHardware(provider)) return false;
    IOAudioSampleRate r44 = { 44100, 0 };
    setSampleRate(&r44);
    setNumSampleFramesPerBuffer(fMaster->numSampleFramesPerBuffer);
    fRingBytes = numSampleFramesPerBuffer << 3;
    if (!fRing.InitializeWithSizeInBytes(fRingBytes)) return false;
    IOAudioStreamFormat fmt;
    PairFormat(fmt);
    if (fHasOut) {
        com_openaudio_FWPairStream *s = OSTypeAlloc(com_openaudio_FWPairStream);
        if (!s) return false;
        if (!s->initWithAudioEngine(this, kIOAudioStreamDirectionOutput, 1, 0, 0)) { s->release(); return false; }
        s->addAvailableFormat(&fmt, &r44, &r44);
        s->setSampleBuffer(fRing.data, fRingBytes);
        com_openaudio_FWBaseEngine *b = base();
        if (b && b->mainOutputStream()) s->setMixBuffer(b->mainOutputStream()->getMixBuffer(), fRingBytes);
        addAudioStream(s);
        fOutStream = s;
        s->release();
    }
    if (fHasIn) {
        com_openaudio_FWPairStream *s = OSTypeAlloc(com_openaudio_FWPairStream);
        if (!s) return false;
        if (!s->initWithAudioEngine(this, kIOAudioStreamDirectionInput, 1, 0, 0)) { s->release(); return false; }
        s->addAvailableFormat(&fmt, &r44, &r44);
        s->setSampleBuffer(fRing.data, fRingBytes);
        addAudioStream(s);
        fInStream = s;
        s->release();
    }
    ResetIOFormat(&fMaster->sampleRate);                                        // ref vt[0xbf8](master [+0x8c])
    setRunEraseHead(false);                                                     // ref vt[0xb80](0)
    return true;
}

void com_openaudio_FWPairEngine::free()
{
    fRing.Invalidate();
    com_openaudio_FWTimeSlaveEngine::free();
}

// ref ResetIOFormat @0x1ab42
// Deviation (R141): the engine rate is set BEFORE the streams' setFormat; the reference sets it after (vt[0xb58] follows vt[0xa38]).
// IOAudioStream::setFormat validates the format against the engine's CURRENT rate (validateFormat rate check, IOAudioFamily 540.3
// IOAudioStream.cpp:264, 206.5 :261), so with the reference order every call with a rate other than the engine's previous one (44.1 kHz
// from initHardware) is refused and the pair streams get no IOAudioStreamFormat. The 10.6 HAL showed the devices anyway (capture 103);
// the macOS 14 HAL gives them 0 channels (dev Mac, FW1814 at 48 kHz, 2026-10-27).
void com_openaudio_FWPairEngine::ResetIOFormat(const IOAudioSampleRate *rate)
{
    IOAudioStreamFormat fmt;
    PairFormat(fmt);
    beginConfigurationChange();
    setSampleRate(rate);
    IOAudioStream *streams[2] = { fHasIn ? fInStream : 0, fHasOut ? fOutStream : 0 };
    for (int i = 0; i < 2; i++) {
        if (!streams[i]) continue;
        streams[i]->clearAvailableFormats();
        streams[i]->addAvailableFormat(&fmt, rate, rate);
        streams[i]->setFormat(&fmt, true);
    }
    hardwareSampleRateChanged(rate);
    if (fMaster) {
        static const char *keys[3] = { "IOAudioEngineInputSampleLatency", "IOAudioEngineOutputSampleLatency", "IOAudioEngineSampleOffset" };
        for (int i = 0; i < 3; i++)
            if (OSObject *v = fMaster->getProperty(keys[i])) setProperty(keys[i], v);
    }
    completeConfigurationChange();
}

// ref performAudioEngineStart/Stop @0x1ae44 / @0x1ae22
IOReturn com_openaudio_FWPairEngine::performAudioEngineStart()
{
    if (fMaster) fMaster->ActivateSubEngine(bit());
    return kIOReturnSuccess;
}
IOReturn com_openaudio_FWPairEngine::performAudioEngineStop()
{
    if (fMaster) fMaster->DeactivateSubEngine(bit());
    return kIOReturnSuccess;
}

// ref stopEngineAtPosition @0x1ace6 (the master's StopSubEngineAtPosition is an empty function)
void com_openaudio_FWPairEngine::stopEngineAtPosition(IOAudioEnginePosition *pos)
{
    if (!pos) return;
    stopAudioEngine();
}

// ref setState @0x1ad40
IOAudioEngineState com_openaudio_FWPairEngine::setState(IOAudioEngineState newState)
{
    IOAudioEngineState old = state;
    state = newState;
    setProperty("IOAudioEngineState", (unsigned long long)newState, 32);
    return old;
}

IOReturn com_openaudio_FWPairEngine::clipOutputSamples(const void *, void *, UInt32 first, UInt32 num, const IOAudioStreamFormat *fmt, IOAudioStream *stream)
{
    if (!fMaster) return kErrBadArg;
    return fMaster->ClipSubEngineOutput(bit(), first, num, fmt, stream);
}

// ref convertInputSamples @0x1ae64: convert (once per period) in the master, then copy this pair's two channels out of the multichannel float buffer
IOReturn com_openaudio_FWPairEngine::convertInputSamples(const void *, void *destBuf, UInt32 first, UInt32 num, const IOAudioStreamFormat *, IOAudioStream *)
{
    com_openaudio_FWBaseEngine *b = base();
    if (!b || !b->mainInputStream() || !destBuf) return kErrBadArg;
    const IOAudioStreamFormat *mf = b->mainInputStream()->getFormat();
    const UInt32 stride = mf->fNumChannels;
    b->ConvertSubEngineInput(bit(), first, num);
    const float *src = static_cast<const float *>(b->GetMultichannelInputBuffer());
    if (!src) return kErrBadArg;
    float *dst = static_cast<float *>(destBuf);
    for (UInt32 f = 0; f < num; f++) {                                          // ref _CopyChannelPairFromMultichannelBuffer
        dst[2 * f]     = src[f * stride + fInOffset];
        dst[2 * f + 1] = src[f * stride + fInOffset + 1];
    }
    return kIOReturnSuccess;
}

// ref mixOutputSamples @0x1af32: add the pair into the master's multichannel mix buffer at this pair's channels
IOReturn com_openaudio_FWPairEngine::mixOutputSamples(const void *sourceBuf, void *, UInt32 first, UInt32 num, const IOAudioStreamFormat *, IOAudioStream *)
{
    com_openaudio_FWBaseEngine *b = base();
    if (!b || !b->mainOutputStream() || !sourceBuf) return kErrBadArg;
    float *mix = static_cast<float *>(b->mainOutputStream()->getMixBuffer());
    if (!mix) return kErrBadArg;
    const UInt32 stride = b->mainOutputStream()->getFormat()->fNumChannels;
    const float *src = static_cast<const float *>(sourceBuf);
    float *dst = mix + (size_t)first * stride + fOutOffset;                     // ref: mix + (first * channels) * 4 bytes, channel offset added in the loop
    for (UInt32 f = 0; f < num; f++) {                                          // ref _MixChannelPairToMultichannelBuffer
        dst[f * stride]     += src[2 * f];
        dst[f * stride + 1] += src[2 * f + 1];
    }
    return kIOReturnSuccess;
}
