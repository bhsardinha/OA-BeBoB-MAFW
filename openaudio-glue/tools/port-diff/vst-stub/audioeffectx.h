// OpenAudio Glue - the smallest stand-in for the VST 2 SDK's AudioEffectX that lets the Airwindows originals compile
// for tools/port-diff. Only what their sources call; nothing here is from Steinberg's SDK.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#define __audioeffect__
typedef int32_t VstInt32;
typedef void *audioMasterCallback;
enum VstPlugCategory { kPlugCategEffect };
enum { kVstMaxProgNameLen = 24, kVstMaxParamStrLen = 8, kVstMaxProductStrLen = 64, kVstMaxVendorStrLen = 64 };
static inline void vst_strncpy(char *d, const char *s, int n) { strncpy(d, s, n); d[n] = 0; }
static inline void float2string(float v, char *t, int n) { snprintf(t, n, "%f", v); }
static inline void int2string(int v, char *t, int n) { snprintf(t, n, "%d", v); }
class AudioEffect { public: virtual ~AudioEffect() {} };
class AudioEffectX : public AudioEffect {
public:
    AudioEffectX(audioMasterCallback, int, int) {}
    virtual ~AudioEffectX() {}
    double portDiffSampleRate = 44100.0;
    double getSampleRate() { return portDiffSampleRate; }
    void setNumInputs(int) {} void setNumOutputs(int) {} void setUniqueID(long) {}
    void canProcessReplacing() {} void canDoubleReplacing() {} void programsAreChunks(bool) {}
    virtual VstInt32 getChunk(void **, bool) { return 0; }
    virtual VstInt32 setChunk(void *, VstInt32, bool) { return 0; }
};
