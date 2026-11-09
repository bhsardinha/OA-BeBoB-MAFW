// OpenAudio Glue - per-channel compressor (docs/DSP_v1.md "Compressor"). Two controls: the threshold, and the source the
// channel carries (user, 2026-10-28), which picks a voicing - detector, ratio, knee and timing suited to that material.
// Every voicing is feed-forward with program-dependent release (a fast and a slow envelope; the deeper one wins) and fixed
// partial makeup referenced to -18 dBFS. The Drums voicing also keeps the transients: a slow attack lets the hit through
// while the sustain is compressed, and a transient detector lifts the hit (an expander on the hit, a compressor on the
// sustain). All values are design values to be tuned by ear.
#pragma once

namespace glue {

enum CompSource { kSourceGeneral, kSourceDrums, kSourceVocals, kSourceGuitar, kSourceBass, kSourceSynth, kSourceCount };
const char *CompSourceName(int source);

struct CompVoicing {
    bool peakDetector;                 // false = RMS over rmsMs
    double rmsMs;
    double ratio, kneeDb;
    double fastAttackMs, fastReleaseMs, slowAttackMs, slowReleaseMs;
    bool keepTransients;               // the Drums transient shaper
};
const CompVoicing &GetCompVoicing(int source);

struct CompDesign {
    static constexpr double kMakeupRefDb = -18.0, kMakeupShare = 0.5;
    static constexpr double kMinThresholdDb = -40.0;   // 0 dB = off
    // Transient shaper (Drums): a peak detector (instant attack, kHitReleaseMs) feeds a fast and a slow follower with a
    // shared release; on a steady signal both settle on the same value, on a hit the fast one leads, and the gap in dB is
    // the "hit" signal.
    static constexpr double kHitFastAttackMs = 0.5, kHitSlowAttackMs = 20.0, kHitReleaseMs = 60.0;
    static constexpr double kHitGain = 0.7, kHitMaxDb = 6.0;
    static constexpr double kHitFullDepthDb = 20.0;    // the hit lift reaches full strength at a -20 dB threshold
};

// Static gain reduction (dB, <= 0) of a voicing's curve at input level `levelDb` for `thresholdDb`.
double CompStaticGainReductionDb(double levelDb, double thresholdDb, int source = kSourceGeneral);
// Makeup (dB, >= 0) used for `thresholdDb`.
double CompMakeupDb(double thresholdDb, int source = kSourceGeneral);

class Comp {
public:
    void reset(double sampleRate);
    void setThreshold(double thresholdDb);       // >= 0 dB turns the compressor off (exact bypass)
    void setSource(int source);
    double process(double x);
    // Stereo link (user, 2026-10-28): each side's detector runs as usual; the deeper reduction and the larger hit lift go to both
    // sides, so a linked pair keeps its image. Unlinked or one side off: each side on its own.
    static void processLinked(Comp &a, Comp &b, double &xa, double &xb);
    bool active() const { return fOn; }
    float gainReductionDb() const { return (float)fGrDb; }   // latest smoothed reduction, <= 0 (makeup and hit lift excluded)
    float hitLiftDb() const { return (float)fHitDb; }        // Drums: latest lift on the hit, >= 0
private:
    void updateCoefs();
    void detect(double x);                       // updates the envelopes, fGrDb and fHitDb
    double fRate = 48000, fThresh = 0, fMakeupDb = 0;
    int fSource = kSourceGeneral;
    double cRms = 0, cFastA = 0, cFastR = 0, cSlowA = 0, cSlowR = 0;
    double cHitFA = 0, cHitSA = 0, cHitR = 0;
    double fMs = 0, fPeak = 0, fFast = 0, fSlow = 0, fGrDb = 0;
    double fHitPeak = 0, fHitFast = 0, fHitSlow = 0, fHitDb = 0;
    bool fOn = false;
};

} // namespace glue
