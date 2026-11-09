// OpenAudio Glue - port-diff: runs one Airwindows original (ref/, fetched by fetch-ref.sh) on L = R input and returns its
// L output. One translation unit per original (their headers clash), selected with -DORIG=<name>.
#include <vector>
#define STR2(x) #x
#define STR(x) STR2(x)
// The originals keep their dither state private and seed it from rand(); the test opens it so both sides start from the
// port's fixed seed (glue::*::reset default, 2463534242). Test-only: nothing in dsp/ does this.
#define private public
#include STR(ORIG.h)
#undef private

#define CAT2(a, b) a##b
#define CAT(a, b) CAT2(a, b)

void CAT(RunOriginal_, ORIG)(const std::vector<float> &params, double rate, const float *in, float *outL, int frames)
{
    ORIG plugin(nullptr);
    plugin.portDiffSampleRate = rate;
    plugin.fpdL = plugin.fpdR = 2463534242u;
    for (size_t i = 0; i < params.size(); i++) plugin.setParameter((VstInt32)i, params[i]);
    std::vector<float> l(64), r(64), ol(64), orr(64);
    for (int n = 0; n < frames; n += 64) {
        const int k = (frames - n) < 64 ? (frames - n) : 64;
        for (int i = 0; i < k; i++) l[i] = r[i] = in[n + i];
        float *ins[2] = { l.data(), r.data() }, *outs[2] = { ol.data(), orr.data() };
        plugin.processReplacing(ins, outs, k);
        for (int i = 0; i < k; i++) outL[n + i] = ol[i];
    }
}
