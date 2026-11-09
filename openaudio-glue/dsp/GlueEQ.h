// OpenAudio Glue - per-channel 4-band EQ (docs/DSP_v1.md "EQ").
// Filters: R. Bristow-Johnson, "Cookbook formulae for audio EQ biquad filter coefficients" (low shelf, high shelf,
// peaking EQ). Shelves keep a fixed slope S = 1; the two mid bands use proportional Q (narrower as the gain grows).
#pragma once

namespace glue {

enum EQBand { kBandLowShelf, kBandLowMid, kBandHighMid, kBandHighShelf, kBandCount };

struct EQBandSetting { float freqHz; float gainDb; };

struct EQBandRange { float minHz, maxHz, defaultHz; };
extern const EQBandRange kEQBandRanges[kBandCount];
const float kEQMaxGainDb = 12.0f;

// Design values (docs/DSP_v1.md), shared with the panel's curve so the graph shows what the filter does.
inline double EQProportionalQ(double gainDb) { return 0.5 + 0.1 * (gainDb < 0 ? -gainDb : gainDb); }
const double kEQShelfSlope = 1.0;

struct BiquadCoeffs { double b0, b1, b2, a1, a2; };   // normalised by a0

// Coefficients of one band; frequency and gain are clamped to the band's range and to 0.45 x fs.
BiquadCoeffs EQBandCoeffs(int band, double freqHz, double gainDb, double sampleRate);
// |H| in dB of one band at `atHz` (analytic, the same formula the filter uses).
double EQBandMagnitudeDb(int band, double freqHz, double gainDb, double sampleRate, double atHz);

class EQ {
public:
    void reset(double sampleRate);
    void setTarget(const EQBandSetting bands[kBandCount]);   // clamped; takes effect through the smoothing
    void beginBlock(int frames);                              // advances the smoothing, recomputes changed bands
    double process(double x);
    bool active() const { return fActiveMask != 0; }
private:
    struct Band {
        double tgtHz = 1000, tgtDb = 0, curHz = 1000, curDb = 0;
        BiquadCoeffs c = { 1, 0, 0, 0, 0 };
        double z1 = 0, z2 = 0;
    };
    Band fBand[kBandCount];
    double fRate = 48000;
    unsigned fActiveMask = 0;
};

} // namespace glue
