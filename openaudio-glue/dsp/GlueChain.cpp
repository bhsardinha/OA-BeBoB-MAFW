// OpenAudio Glue - one input channel's chain (see Glue.h).
#include "Glue.h"
#include <cmath>
#include <cstring>

namespace glue {

void ChannelChain::reset(const Presets *presets, uint32_t seed)
{
    fPresets = presets;
    fSeed = seed ? seed : 1;
    resetState();
}

void ChannelChain::resetState()
{
    const double rate = fPresets ? fPresets->sampleRate() : 48000.0;
    fTransformer.reset(rate, fSeed * 4 + 0);
    fTube.reset(rate, fSeed * 4 + 1);
    fDesk.reset(rate, fSeed * 4 + 2);
    fTape.reset(rate, fSeed * 4 + 3);
    fEQ.reset(rate);
    fComp.reset(rate);
    for (int i = 0; i < kMaxCompDelay; i++) fDelayLine[i] = 0.0f;
    fDelayPos = 0;
    updateCompDelay();
    for (int s = 0; s < kStageCount; s++) fPreset[s] = GetStagePreset(s, fLevel[s]);
    fTube.prepare(fPreset[kStageTube].tubeA, fPreset[kStageTube].tubeB);
    fDesk.prepare(fPreset[kStageDesk].deskType, fPreset[kStageDesk].deskDrive, 1.0);
    fTape.prepare(fPreset[kStageTape].tape);
}

void ChannelChain::setGlobal(const GlobalSettings &g)
{
    const double rate = fPresets ? fPresets->sampleRate() : 48000.0;
    const double d = g.driveDb < 0 ? 0.0 : (g.driveDb > 18 ? 18.0 : (double)g.driveDb);
    fDrive = pow(10.0, d / 20.0);
    for (int s = 0; s < kStageCount; s++) {
        const uint8_t l = g.level[s] < StageOptionCount(s) ? g.level[s] : (uint8_t)kLevelOff;
        if (l == fLevel[s]) continue;
        if (fLevel[s] == kLevelOff) {                         // a stage coming back starts from clean state
            switch (s) {
            case kStageTransformer: fTransformer.reset(rate, fSeed * 4 + 0); break;
            case kStageTube:        fTube.reset(rate, fSeed * 4 + 1); break;
            case kStageDesk:        fDesk.reset(rate, fSeed * 4 + 2); break;
            default:                fTape.reset(rate, fSeed * 4 + 3); break;
            }
        }
        fLevel[s] = l;
        fPreset[s] = GetStagePreset(s, l);
        switch (s) {
        case kStageTube: fTube.prepare(fPreset[s].tubeA, fPreset[s].tubeB); break;
        case kStageDesk: fDesk.prepare(fPreset[s].deskType, fPreset[s].deskDrive, 1.0); break;
        case kStageTape: fTape.prepare(fPreset[s].tape); break;
        default: break;
        }
    }
}

// Processed: pad the active stages' latency up to the maximum; passing through (off, bypassed, nothing active): the whole maximum.
void ChannelChain::updateCompDelay()
{
    if (!fPresets) { fCompDelay = 0; return; }
    const int maxL = fPresets->maxLatency();
    int d = bypassed() ? maxL : maxL - fPresets->chainLatency(fLevel);
    if (d < 0) d = 0;
    if (d > kMaxCompDelay - 1) d = kMaxCompDelay - 1;
    fCompDelay = d;
}

inline float ChannelChain::delayed(float x)
{
    fDelayLine[fDelayPos] = x;
    const float y = fDelayLine[(fDelayPos - fCompDelay + kMaxCompDelay) % kMaxCompDelay];
    fDelayPos = (fDelayPos + 1) % kMaxCompDelay;
    return y;
}

void ChannelChain::setChannel(const ChannelSettings &c)
{
    if (c.on && !fOn) {                                       // back on: start clean, keep the stage levels
        resetState();
        fEQ.setTarget(c.band);
        fEQ.beginBlock(1 << 20);                              // settle the EQ at once (no glide from flat)
    }
    fOn = c.on;
    fComp.setSource(c.source);
    fComp.setThreshold(c.thresholdDb);
    fEQ.setTarget(c.band);
}

bool ChannelChain::bypassed() const
{
    if (!fOn || fBypassAll) return true;
    for (int s = 0; s < kStageCount; s++) if (fLevel[s] != kLevelOff) return false;
    return !fEQ.active() && !fComp.active();
}

inline double ChannelChain::runStage(int s, double x)
{
    const StagePreset &p = fPreset[s];
    const int l = fLevel[s];
    // The global Drive pushes every stage harder and takes the same gain back after it, so the level stays about the same.
    const double in = x * p.drive * fDrive;
    double w;
    switch (s) {
    case kStageTransformer: w = fTransformer.process(in); break;
    case kStageTube:        w = fTube.process(in); break;
    case kStageDesk:        w = fDesk.process(in); break;
    default:                w = fTape.process(in); break;
    }
    w *= fPresets->wetNorm(s, l) / fDrive;
    if (p.wet < 1.0) w = x * (1.0 - p.wet) + w * p.wet;
    return w * fPresets->trim(s, l);
}

inline double ChannelChain::pre(double x)
{
    if (fLevel[kStageTransformer]) x = runStage(kStageTransformer, x);
    if (fLevel[kStageTube]) x = runStage(kStageTube, x);
    return fEQ.process(x);
}

inline double ChannelChain::post(double x)
{
    if (fLevel[kStageDesk]) x = runStage(kStageDesk, x);
    if (fLevel[kStageTape]) x = runStage(kStageTape, x);
    return x;
}

inline double ChannelChain::step(double x)
{
    return post(fComp.process(pre(x)));
}

void ChannelChain::begin(int frames)
{
    fEQ.beginBlock(frames);
    updateCompDelay();                                        // stage levels, on/off, bypass and the EQ's activity can change between blocks
}

void ChannelChain::body(const float *in, float *out, int frames)
{
    if (bypassed() || !fPresets) {                            // exact pass-through, delayed like a processed channel
        for (int n = 0; n < frames; n++) out[n] = delayed(in[n]);
        return;
    }
    for (int n = 0; n < frames; n++) out[n] = delayed((float)step(in[n]));
}

void ChannelChain::process(const float *in, float *out, int frames)
{
    begin(frames);
    body(in, out, frames);
}

void ChannelChain::processPair(ChannelChain &a, ChannelChain &b, const float *inA, const float *inB, float *outA, float *outB, int frames)
{
    a.begin(frames);
    b.begin(frames);
    if (a.bypassed() || b.bypassed() || !a.fPresets || !b.fPresets) {
        a.body(inA, outA, frames);
        b.body(inB, outB, frames);
        return;
    }
    for (int n = 0; n < frames; n++) {
        double xa = a.pre(inA[n]), xb = b.pre(inB[n]);
        Comp::processLinked(a.fComp, b.fComp, xa, xb);
        outA[n] = a.delayed((float)a.post(xa));
        outB[n] = b.delayed((float)b.post(xb));
    }
}

} // namespace glue
