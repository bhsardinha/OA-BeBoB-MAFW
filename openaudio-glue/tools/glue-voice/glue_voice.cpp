// OpenAudio Glue - glue-voice: finds the drive into each colour preset that makes the Soft...Full ladder audible at tracking level
// (user, 2026-10-28: "the colour buttons need to be in the path"; the stages were transparent at -12 dBFS peaks). One stage at a time,
// run as the chain runs it (drive in, wet path normalised at -18 dBFS 1 kHz, dry/wet blend), its other parameters from the preset table.
// Targets (design values, docs/DSP_v1.md "Colour v2"):
//   Tube, Desk, Tape: THD (harmonics 2-10) of a 1 kHz sine at -12 dBFS peak: Soft 0.3 %, Medium 1 %, High 3 %, Full 8 %
//                     (Desk Medium 1 %, High 3 % for each type)
//   Transformer:      Interstage limits slew, so it acts on transients: "change" (difference energy over dry) on a drum loop at -12 dBFS
//                     peak: Soft -36, Medium -30, High -24, Full -18 dB
//   glue-voice <drum loop, 48 kHz mono float WAV>
#include "Glue.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace glue;
static const double kFs = 48000, kPi = 3.14159265358979323846;

struct Runner {
    int stage; StagePreset p; double drive, wetNorm = 1;
    Interstage tr; Tube2 tu; Channel9 de; ToTape9 ta;
    void reset() {
        tr.reset(kFs); tu.reset(kFs); de.reset(kFs); ta.reset(kFs);
        tu.prepare(p.tubeA, p.tubeB); de.prepare(p.deskType, p.deskDrive, 1.0); ta.prepare(p.tape);
    }
    double wet(double x) {
        switch (stage) {
        case kStageTransformer: return tr.process(x * drive);
        case kStageTube: return tu.process(x * drive);
        case kStageDesk: return de.process(x * drive);
        default: return ta.process(x * drive);
        }
    }
    double run(double x) { const double w = wet(x) * wetNorm; return p.wet < 1 ? x * (1 - p.wet) + w * p.wet : w; }
};

// wet path normalised like Presets::calibrate: a -18 dBFS 1 kHz sine keeps its RMS
static void Normalise(Runner &r)
{
    r.wetNorm = 1; r.reset();
    const double a = pow(10.0, -18.0 / 20.0), w = 2 * kPi * 1000 / kFs; double s = 0; const int S = (int)(0.3 * kFs), M = (int)(0.3 * kFs);
    for (int n = 0; n < S + M; n++) { const double y = r.wet(a * sin(w * n)); if (n >= S) s += y * y; }
    const double rms = sqrt(s / M);
    r.wetNorm = rms > 1e-12 ? (a / sqrt(2.0)) / rms : 1;
}

static double Thd(Runner &r, double peakDb)
{
    r.reset();
    const double a = pow(10.0, peakDb / 20.0), w = 2 * kPi * 1000 / kFs;
    const int S = (int)(0.3 * kFs), N = 48000;
    std::vector<double> y(N);
    for (int n = 0; n < S + N; n++) { const double v = r.run(a * sin(w * n)); if (n >= S) y[n - S] = v; }
    double h[11] = {};
    for (int k = 1; k <= 10; k++) { double re = 0, im = 0; for (int n = 0; n < N; n++) { re += y[n] * cos(w * k * (n + S)); im += y[n] * sin(w * k * (n + S)); } h[k] = re * re + im * im; }
    double sum = 0; for (int k = 2; k <= 10; k++) sum += h[k];
    return 100.0 * sqrt(sum / h[1]);
}

static double Change(Runner &r, const std::vector<float> &x, double gain)
{
    r.reset();
    double e = 0, d = 0;
    for (size_t i = 0; i < x.size(); i++) {
        const double in = x[i] * gain, out = r.run(in);
        if (i > 48000) { e += (out - in) * (out - in); d += in * in; }
    }
    return 10 * log10(e / d + 1e-30);
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: glue-voice <48 kHz mono float WAV>\n"); return 2; }
    FILE *f = fopen(argv[1], "rb"); if (!f) return 1;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    std::vector<char> raw((size_t)n); if (fread(raw.data(), 1, (size_t)n, f) != (size_t)n) return 1; fclose(f);
    long off = 12; while (off + 8 < n && memcmp(&raw[(size_t)off], "data", 4)) off += 8 + *(int *)&raw[(size_t)off + 4];
    const float *xs = (const float *)&raw[(size_t)off + 8]; long N = (*(int *)&raw[(size_t)off + 4]) / 4; if (N > 48000 * 12) N = 48000 * 12;
    std::vector<float> loop(xs, xs + N);
    double pk = 0; for (float v : loop) pk = std::max(pk, (double)fabs(v));
    const double gain = pow(10.0, -12.0 / 20.0) / pk;                       // the loop at -12 dBFS peak
    const double thdTarget[4] = { 0.3, 1.0, 3.0, 8.0 }, changeTarget[4] = { -36, -30, -24, -18 };
    for (int st = 0; st < kStageCount; st++) for (int o = 1; o < StageOptionCount(st); o++) {
        Runner r; r.stage = st; r.p = GetStagePreset(st, o);
        const int rung = st == kStageDesk ? ((o - 1) % 2 == 0 ? 1 : 2) : o - 1;      // desk: Medium = rung 1, High = rung 2
        double lo = -12, hi = 36, best = 0;
        for (int it = 0; it < 26; it++) {                                    // bisection on the drive in dB (the measure grows with it)
            const double mid = 0.5 * (lo + hi);
            r.drive = pow(10.0, mid / 20.0); Normalise(r);
            const double m = st == kStageTransformer ? Change(r, loop, gain) : Thd(r, -12);
            const double t = st == kStageTransformer ? changeTarget[rung] : thdTarget[rung];
            if (m < t) lo = mid; else hi = mid;
            best = mid;
        }
        r.drive = pow(10.0, best / 20.0); Normalise(r);
        printf("%-11s %-11s drive %+6.1f dB  -> THD@-12 %6.2f %%  change@-12 %6.1f dB\n", StageName(st), StageOptionName(st, o), best,
               Thd(r, -12), Change(r, loop, gain));
    }
    return 0;
}
