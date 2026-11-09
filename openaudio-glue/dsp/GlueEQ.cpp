// OpenAudio Glue - per-channel 4-band EQ. Formulas: RBJ Audio-EQ-Cookbook (see GlueEQ.h).
#include "GlueEQ.h"
#include <cmath>

namespace glue {

static const double kPi = 3.14159265358979323846;

// Ranges and defaults: docs/DSP_v1.md (design values).
const EQBandRange kEQBandRanges[kBandCount] = {
    {   30.0f,   600.0f,   100.0f },   // low shelf
    {   60.0f,  2000.0f,   400.0f },   // low-mid
    {  500.0f, 12000.0f,  2500.0f },   // high-mid
    { 1500.0f, 16000.0f,  8000.0f },   // high shelf
};

static double Clamp(double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }

static void ClampBand(int band, double sampleRate, double &hz, double &db)
{
    hz = Clamp(hz, kEQBandRanges[band].minHz, kEQBandRanges[band].maxHz);
    if (hz > 0.45 * sampleRate) hz = 0.45 * sampleRate;
    db = Clamp(db, -kEQMaxGainDb, kEQMaxGainDb);
}

BiquadCoeffs EQBandCoeffs(int band, double hz, double db, double sampleRate)
{
    ClampBand(band, sampleRate, hz, db);
    const double A = pow(10.0, db / 40.0);
    const double w0 = 2.0 * kPi * hz / sampleRate;
    const double cw = cos(w0), sw = sin(w0);
    double b0, b1, b2, a0, a1, a2;
    if (band == kBandLowShelf || band == kBandHighShelf) {
        const double alpha = sw / 2.0 * sqrt((A + 1.0 / A) * (1.0 / kEQShelfSlope - 1.0) + 2.0);
        const double k = 2.0 * sqrt(A) * alpha;
        if (band == kBandLowShelf) {
            b0 = A * ((A + 1) - (A - 1) * cw + k);
            b1 = 2 * A * ((A - 1) - (A + 1) * cw);
            b2 = A * ((A + 1) - (A - 1) * cw - k);
            a0 = (A + 1) + (A - 1) * cw + k;
            a1 = -2 * ((A - 1) + (A + 1) * cw);
            a2 = (A + 1) + (A - 1) * cw - k;
        } else {
            b0 = A * ((A + 1) + (A - 1) * cw + k);
            b1 = -2 * A * ((A - 1) + (A + 1) * cw);
            b2 = A * ((A + 1) + (A - 1) * cw - k);
            a0 = (A + 1) - (A - 1) * cw + k;
            a1 = 2 * ((A - 1) - (A + 1) * cw);
            a2 = (A + 1) - (A - 1) * cw - k;
        }
    } else {
        const double alpha = sw / (2.0 * EQProportionalQ(db));
        b0 = 1 + alpha * A;
        b1 = -2 * cw;
        b2 = 1 - alpha * A;
        a0 = 1 + alpha / A;
        a1 = -2 * cw;
        a2 = 1 - alpha / A;
    }
    BiquadCoeffs c = { b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 };
    return c;
}

double EQBandMagnitudeDb(int band, double hz, double db, double sampleRate, double atHz)
{
    const BiquadCoeffs c = EQBandCoeffs(band, hz, db, sampleRate);
    const double w = 2.0 * kPi * atHz / sampleRate;
    // H(e^jw) = (b0 + b1 e^-jw + b2 e^-2jw) / (1 + a1 e^-jw + a2 e^-2jw)
    const double nr = c.b0 + c.b1 * cos(w) + c.b2 * cos(2 * w), ni = -(c.b1 * sin(w) + c.b2 * sin(2 * w));
    const double dr = 1.0 + c.a1 * cos(w) + c.a2 * cos(2 * w), di = -(c.a1 * sin(w) + c.a2 * sin(2 * w));
    return 10.0 * log10((nr * nr + ni * ni) / (dr * dr + di * di));
}

void EQ::reset(double sampleRate)
{
    fRate = sampleRate;
    for (int b = 0; b < kBandCount; b++) {
        fBand[b] = Band();
        fBand[b].tgtHz = fBand[b].curHz = kEQBandRanges[b].defaultHz;
    }
    fActiveMask = 0;
}

void EQ::setTarget(const EQBandSetting bands[kBandCount])
{
    for (int b = 0; b < kBandCount; b++) {
        double hz = bands[b].freqHz, db = bands[b].gainDb;
        ClampBand(b, fRate, hz, db);
        fBand[b].tgtHz = hz;
        fBand[b].tgtDb = db;
    }
}

// One-pole smoothing per block, 20 ms time constant (design value); frequency moves in the log domain.
void EQ::beginBlock(int frames)
{
    const double k = 1.0 - exp(-(double)frames / (fRate * 0.020));
    for (int b = 0; b < kBandCount; b++) {
        Band &d = fBand[b];
        if (d.curDb == d.tgtDb && d.curHz == d.tgtHz) continue;
        d.curDb += (d.tgtDb - d.curDb) * k;
        d.curHz = exp(log(d.curHz) + (log(d.tgtHz) - log(d.curHz)) * k);
        if (fabs(d.curDb - d.tgtDb) < 0.01) d.curDb = d.tgtDb;
        if (fabs(d.curHz / d.tgtHz - 1.0) < 1e-4) d.curHz = d.tgtHz;
        const bool on = (d.curDb != 0.0);
        if (on) {
            d.c = EQBandCoeffs(b, d.curHz, d.curDb, fRate);
            fActiveMask |= (1u << b);
        } else {
            fActiveMask &= ~(1u << b);
            d.z1 = d.z2 = 0.0;                     // a band at exactly 0 dB is skipped (bit-exact bypass)
        }
    }
}

double EQ::process(double x)
{
    if (!fActiveMask) return x;
    for (int b = 0; b < kBandCount; b++) {
        if (!(fActiveMask & (1u << b))) continue;
        Band &d = fBand[b];
        const double y = d.c.b0 * x + d.z1;        // transposed direct form II
        d.z1 = d.c.b1 * x - d.c.a1 * y + d.z2;
        d.z2 = d.c.b2 * x - d.c.a2 * y;
        x = y;
    }
    return x;
}

} // namespace glue
