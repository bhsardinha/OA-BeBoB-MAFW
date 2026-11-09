// OpenAudio Glue - per-channel compressor (see GlueComp.h).
#include "GlueComp.h"
#include <cmath>

namespace glue {

static const char *kSourceNames[kSourceCount] = { "General", "Drums", "Vocals", "Heavy guitar", "Bass", "Synth" };
const char *CompSourceName(int s) { return (s >= 0 && s < kSourceCount) ? kSourceNames[s] : "?"; }

// Design values (docs/DSP_v1.md "Compressor"), starting points to tune by ear.
static const CompVoicing kVoicings[kSourceCount] = {
    //  peak   rms   ratio knee  fastA fastR slowA  slowR  transients
    { false, 10.0, 3.0,  6.0,  10.0,  80.0, 150.0, 600.0, false },   // General (the v1 compressor)
    { true,   0.0, 4.0,  6.0,  30.0, 120.0, 200.0, 400.0, true  },   // Drums: slow attack + hit lift
    { false,  5.0, 3.0, 10.0,   5.0,  60.0, 120.0, 800.0, false },   // Vocals: smooth, opto-like release
    { false, 20.0, 2.0, 10.0,  20.0, 200.0, 300.0, 900.0, false },   // Heavy guitar: gentle glue on dense material
    { false, 30.0, 4.0,  6.0,  15.0, 120.0, 200.0, 500.0, false },   // Bass: detector spans a 30 Hz cycle
    { false, 10.0, 2.5,  6.0,   8.0, 100.0, 150.0, 600.0, false },   // Synth
};
const CompVoicing &GetCompVoicing(int s) { return kVoicings[(s >= 0 && s < kSourceCount) ? s : 0]; }

static double Coef(double ms, double rate) { return 1.0 - exp(-1.0 / (rate * ms * 0.001)); }

double CompStaticGainReductionDb(double L, double T, int source)
{
    const CompVoicing &v = GetCompVoicing(source);
    const double R = v.ratio, W = v.kneeDb;
    const double over = L - T;
    if (2.0 * over < -W) return 0.0;
    if (2.0 * fabs(over) <= W) {                         // soft knee (quadratic)
        const double x = over + W / 2.0;
        return (1.0 / R - 1.0) * x * x / (2.0 * W);
    }
    return (T + over / R) - L;
}

double CompMakeupDb(double T, int source)
{
    if (T >= 0.0) return 0.0;
    return -CompDesign::kMakeupShare * CompStaticGainReductionDb(CompDesign::kMakeupRefDb, T, source);
}

void Comp::reset(double sampleRate)
{
    fRate = sampleRate;
    updateCoefs();
    fMs = fPeak = fFast = fSlow = fGrDb = 0.0;
    fHitPeak = fHitFast = fHitSlow = fHitDb = 0.0;
    setThreshold(fThresh);
}

void Comp::updateCoefs()
{
    const CompVoicing &v = GetCompVoicing(fSource);
    cRms = v.peakDetector ? 0.0 : Coef(v.rmsMs, fRate);
    cFastA = Coef(v.fastAttackMs, fRate);
    cFastR = Coef(v.fastReleaseMs, fRate);
    cSlowA = Coef(v.slowAttackMs, fRate);
    cSlowR = Coef(v.slowReleaseMs, fRate);
    cHitFA = Coef(CompDesign::kHitFastAttackMs, fRate);
    cHitSA = Coef(CompDesign::kHitSlowAttackMs, fRate);
    cHitR = Coef(CompDesign::kHitReleaseMs, fRate);
}

void Comp::setThreshold(double t)
{
    if (t < CompDesign::kMinThresholdDb) t = CompDesign::kMinThresholdDb;
    const bool on = (t < 0.0);
    if (!on && fOn) { fMs = fPeak = fFast = fSlow = fGrDb = 0.0; fHitPeak = fHitFast = fHitSlow = fHitDb = 0.0; }
    fOn = on;
    fThresh = on ? t : 0.0;
    fMakeupDb = CompMakeupDb(fThresh, fSource);
}

void Comp::setSource(int source)
{
    if (source < 0 || source >= kSourceCount) source = kSourceGeneral;
    if (source == fSource) return;
    fSource = source;
    updateCoefs();
    fMakeupDb = CompMakeupDb(fThresh, fSource);
}

double Comp::process(double x)
{
    if (!fOn) return x;
    detect(x);
    return x * exp((fGrDb + fMakeupDb + fHitDb) * 0.11512925464970229);   // ln(10)/20
}

void Comp::processLinked(Comp &a, Comp &b, double &xa, double &xb)
{
    if (!a.fOn || !b.fOn) { xa = a.process(xa); xb = b.process(xb); return; }
    a.detect(xa);
    b.detect(xb);
    const double gr = a.fGrDb < b.fGrDb ? a.fGrDb : b.fGrDb;
    const double hit = a.fHitDb > b.fHitDb ? a.fHitDb : b.fHitDb;
    xa *= exp((gr + a.fMakeupDb + hit) * 0.11512925464970229);
    xb *= exp((gr + b.fMakeupDb + hit) * 0.11512925464970229);
}

void Comp::detect(double x)
{
    const CompVoicing &v = GetCompVoicing(fSource);
    double levelDb;
    if (v.peakDetector) {
        // Instant-attack peak follower; the voicing's attack envelope below sets how fast the reduction comes in.
        const double a = fabs(x);
        fPeak = a > fPeak ? a : fPeak + (a - fPeak) * cFastR;
        levelDb = 20.0 * log10(fPeak + 1e-10);
    } else {
        fMs += (x * x - fMs) * cRms;
        // +3.01 dB: a sine's RMS sits 3 dB under its peak, so the threshold reads in sine-peak dBFS like the panel's meters.
        levelDb = 10.0 * log10(fMs + 1e-20) + 3.0103;
    }
    const double gr = CompStaticGainReductionDb(levelDb, fThresh, fSource);
    fFast += (gr - fFast) * (gr < fFast ? cFastA : cFastR);
    fSlow += (gr - fSlow) * (gr < fSlow ? cSlowA : cSlowR);
    fGrDb = fFast < fSlow ? fFast : fSlow;               // the deeper reduction wins: transients recover fast, sustained slowly

    fHitDb = 0.0;
    if (v.keepTransients) {
        // Transient shaper: a fast and a slow peak follower share one release; on a hit the fast one leads, and the gap
        // (in dB) lifts the hit. The lift scales with the compression depth, so the threshold stays the only amount control.
        const double a = fabs(x);
        fHitPeak = a > fHitPeak ? a : fHitPeak + (a - fHitPeak) * cHitR;
        fHitFast += (fHitPeak - fHitFast) * (fHitPeak > fHitFast ? cHitFA : cHitR);
        fHitSlow += (fHitPeak - fHitSlow) * (fHitPeak > fHitSlow ? cHitSA : cHitR);
        double hit = 20.0 * log10((fHitFast + 1e-10) / (fHitSlow + 1e-10));
        if (hit < 0.0) hit = 0.0;
        double depth = -fThresh / CompDesign::kHitFullDepthDb;
        if (depth > 1.0) depth = 1.0;
        fHitDb = hit * CompDesign::kHitGain * depth;
        if (fHitDb > CompDesign::kHitMaxDb) fHitDb = CompDesign::kHitMaxDb;
    }
}

} // namespace glue
