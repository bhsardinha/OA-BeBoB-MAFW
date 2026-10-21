// FWStream -- IOAudioStream subclass. Mirrors m_audio_b_FWStream (A1 Q-MD; decomp FWStream__*.c). The reference overrides
// setFormat (@0x10bda, 1205 B), validateFormat (@0x11090), mixOutputSamples (@0x10b38) and lock/unlockStreamForIO (18-byte forwards).
// Status (M-D): NO behaviour is overridden yet. validateFormat/setFormat in the reference are private copies of IOAudioFamily's
// bodies written against the 10.7 headers (IOAudioStreamFormatDesc is a 0x48-byte struct there but an opaque type in the 10.13 SDK, so
// it cannot be indexed safely); what they change relative to the stock versions was not identified. Using the stock 10.13
// implementations is an interim choice, recorded as open item Q-MD §4 -- the CoreAudio-visible behaviour is verified at the integrated run.
#ifndef OA_FWSTREAM_H
#define OA_FWSTREAM_H

#include <IOKit/audio/IOAudioStream.h>

class com_openaudio_FWStream : public IOAudioStream
{
    OSDeclareDefaultStructors(com_openaudio_FWStream)
public:
    // ref lockStreamForIO @0x10b06 / unlockStreamForIO @0x10b18: 18-byte forwards to IOAudioStream's; public so the engine
    // (performErase) can take the stream lock (the static IOAudioEngine::lockStreamForIO helpers are not exported on 10.13).
    virtual void lockStreamForIO() APPLE_KEXT_OVERRIDE   { IOAudioStream::lockStreamForIO(); }
    virtual void unlockStreamForIO() APPLE_KEXT_OVERRIDE { IOAudioStream::unlockStreamForIO(); }
};

#endif
