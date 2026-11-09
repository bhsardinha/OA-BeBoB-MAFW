// OpenAudio Glue - preset tables and level calibration (docs/DSP_v1.md "Colour stages").
// The table values are STARTING POINTS chosen 2026-10-28, to be tuned by ear with the user (tuning log in DSP_v1.md).
#include "Glue.h"
#include <cmath>

namespace glue {

static const char *kStageNames[kStageCount] = { "Transformer", "Tube", "Desk", "Tape" };
static const char *kLevelNames[kLevelCount] = { "Off", "Soft", "Medium", "High", "Full" };
static const char *kDeskNames[kDeskOptionCount] = { "Off", "Neve Medium", "Neve High", "API Medium", "API High",
                                                    "SSL Medium", "SSL High" };
const char *StageName(int s) { return (s >= 0 && s < kStageCount) ? kStageNames[s] : "?"; }
int StageOptionCount(int s) { return s == kStageDesk ? (int)kDeskOptionCount : (int)kLevelCount; }
const char *StageOptionName(int s, int o)
{
    if (o < 0 || o >= StageOptionCount(s)) return "?";
    return s == kStageDesk ? kDeskNames[o] : kLevelNames[o];
}

// ToTape9 parameters per level (tape v2, user 2026-10-28: "a little more bright, maybe 30 ips but with more saturation").
// A = input drive (pow(A*2, 2)); B = Tilt, chosen per level so 10 kHz sits +0.5 dB over 1 kHz at -18 dBFS (more drive
// darkens ToTape9, so the tilt rises with it); H 0.63 puts the head bump at about 94 Hz ((H^2 * 175 + 25) Hz), an octave
// above the 15 ips setting - 30 ips character by ear, not measured against a machine; D (flutter) and G (head bump) a little
// lower than v1, as faster tape is steadier. C, E, F (bias neutral, routine skipped) and I (unity output) at 0.5.
// Measurements: docs/DSP_v1.md "Tape v2".
static ToTape9::Params TapeParams(double A, double B, double D, double G)
{
    ToTape9::Params p;
    p.A = A; p.B = B; p.C = 0.5; p.D = D; p.E = 0.5; p.F = 0.5; p.G = G; p.H = 0.63; p.I = 0.5;
    return p;
}

StagePreset GetStagePreset(int stage, int level)
{
    StagePreset p = { 1.0, 1.0, 0.5, 0.5, Channel9::kNeve, 0.0, ToTape9::Params() };
    if (level <= 0 || level >= StageOptionCount(stage)) return p;
    const int i = level - 1;                                     // Soft..Full, or the desk's six options
    switch (stage) {
    // Colour v2 (user, 2026-10-28: the stages must be heard at tracking level): drives found by tools/glue-voice so that at -12 dBFS peaks
    // the ladder reaches its targets (docs/DSP_v1.md "Colour v2"). The wet path is still normalised, so the level stays matched.
    case kStageTransformer: {
        static const double driveDb[4] = { 11.0, 12.3, 14.1, 17.4 };     // v1: 0, +3, +6, +9 dB
        static const double wet[4]     = { 0.5, 0.75, 1.0, 1.0 };
        p.drive = pow(10.0, driveDb[i] / 20.0); p.wet = wet[i];
        break;
    }
    case kStageTube: {
        static const double A[4]   = { 0.35, 0.5, 0.7, 0.9 };
        static const double B[4]   = { 0.25, 0.45, 0.65, 0.85 };
        static const double wet[4] = { 0.5, 0.75, 1.0, 1.0 };
        static const double driveDb[4] = { 6.7, 7.3, 8.4, 7.0 };       // v1: 0 dB
        p.tubeA = A[i]; p.tubeB = B[i]; p.wet = wet[i]; p.drive = pow(10.0, driveDb[i] / 20.0);
        break;
    }
    case kStageDesk: {                                           // Medium = Drive 0.4, High = Drive 0.6, per console type
        static const int type[6] = { Channel9::kNeve, Channel9::kNeve, Channel9::kAPI, Channel9::kAPI,
                                     Channel9::kSSL, Channel9::kSSL };
        static const double B[6] = { 0.4, 0.6, 0.4, 0.6, 0.4, 0.6 };
        static const double driveDb[6] = { 6.5, 8.0, 6.5, 8.0, 6.5, 8.0 };   // v1: 0 dB
        p.deskType = type[i]; p.deskDrive = B[i]; p.drive = pow(10.0, driveDb[i] / 20.0);
        break;
    }
    case kStageTape: {
        static const double A[4] = { 0.56, 0.63, 0.71, 0.80 };    // +1.9, +4.0, +6.0, +8.2 dB into the tape
        static const double B[4] = { 0.72, 0.74, 0.77, 0.79 };
        static const double D[4] = { 0.15, 0.2, 0.25, 0.3 };
        static const double G[4] = { 0.3, 0.35, 0.4, 0.45 };
        p.tape = TapeParams(A[i], B[i], D[i], G[i]);
        break;
    }
    default: break;
    }
    return p;
}

// Runs one stage alone over a 1 kHz sine at -18 dBFS peak: 0.3 s to settle, then the RMS of 0.3 s. With `phaseDelay`, also the
// output's phase delay against the input at 1 kHz, in samples (a DFT bin over the measured part).
static double MeasureRms(int stage, const StagePreset &p, double rate, double wetNorm, bool blend, double *phaseDelay = nullptr)
{
    Interstage tr; Tube2 tu; Channel9 de; ToTape9 ta;
    tr.reset(rate); tu.reset(rate); de.reset(rate); ta.reset(rate);
    tu.prepare(p.tubeA, p.tubeB);
    de.prepare(p.deskType, p.deskDrive, 1.0);
    ta.prepare(p.tape);
    const double amp = pow(10.0, -18.0 / 20.0), w = 2.0 * 3.14159265358979323846 * 1000.0 / rate;
    const int settle = (int)(0.3 * rate), measure = (int)(0.3 * rate);
    double sum = 0.0, yr = 0.0, yi = 0.0, xr = 0.0, xi = 0.0;
    for (int n = 0; n < settle + measure; n++) {
        const double x = amp * sin(w * n);
        double y = 0.0;
        switch (stage) {
        case kStageTransformer: y = tr.process(x * p.drive); break;
        case kStageTube:        y = tu.process(x * p.drive); break;
        case kStageDesk:        y = de.process(x * p.drive); break;
        default:                y = ta.process(x * p.drive); break;
        }
        y *= wetNorm;
        if (blend) y = x * (1.0 - p.wet) + y * p.wet;
        if (n >= settle) {
            sum += y * y;
            yr += y * cos(w * n); yi += y * sin(w * n); xr += x * cos(w * n); xi += x * sin(w * n);
        }
    }
    if (phaseDelay) {
        double ph = atan2(yr, yi) - atan2(xr, xi);
        while (ph > 3.14159265358979323846) ph -= 2 * 3.14159265358979323846;
        while (ph < -3.14159265358979323846) ph += 2 * 3.14159265358979323846;
        *phaseDelay = -ph / w;
    }
    return sqrt(sum / measure);
}

void Presets::calibrate(double rate)
{
    fRate = rate;
    const double rmsIn = pow(10.0, -18.0 / 20.0) / sqrt(2.0);
    for (int s = 0; s < kStageCount; s++) {
        fTrim[s][kLevelOff] = fWetNorm[s][kLevelOff] = 1.0;
        for (int l = 1; l < StageOptionCount(s); l++) {
            const StagePreset p = GetStagePreset(s, l);
            const double wetRms = MeasureRms(s, p, rate, 1.0, false);
            fWetNorm[s][l] = wetRms > 1e-9 ? rmsIn / wetRms : 1.0;
            const double outRms = MeasureRms(s, p, rate, fWetNorm[s][l], true, &fDelay[s][l]);
            fTrim[s][l] = outRms > 1e-9 ? rmsIn / outRms : 1.0;
        }
    }
    // The largest rounded latency over every combination of options (5 x 5 x 7 x 5 = 875).
    fMaxLatency = 0;
    uint8_t lv[kStageCount];
    for (int a = 0; a < StageOptionCount(0); a++) for (int b = 0; b < StageOptionCount(1); b++)
        for (int c = 0; c < StageOptionCount(2); c++) for (int d = 0; d < StageOptionCount(3); d++) {
            lv[0] = (uint8_t)a; lv[1] = (uint8_t)b; lv[2] = (uint8_t)c; lv[3] = (uint8_t)d;
            const int l = chainLatency(lv);
            if (l > fMaxLatency) fMaxLatency = l;
        }
}

int Presets::chainLatency(const uint8_t level[kStageCount]) const
{
    double sum = 0.0;
    for (int s = 0; s < kStageCount; s++) if (level[s] != kLevelOff) sum += fDelay[s][level[s]];
    return (int)lround(sum);                                     // may be negative: a stage can lead in phase (tape at 176.4/192 kHz)
}

} // namespace glue
