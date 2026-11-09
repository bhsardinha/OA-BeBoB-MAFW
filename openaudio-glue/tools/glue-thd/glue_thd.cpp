// OpenAudio Glue - glue-thd: harmonic distortion each colour preset adds (compressor off, EQ flat). A 1 kHz sine at 48 kHz (or the
// rate given) at several input levels; THD = harmonics 2..10 over the fundamental, measured over exactly 1000 cycles after 0.5 s of
// settling (whole cycles: no window, no leakage). Also H2 and H3 alone.
//   glue-thd [rate]
#include "Glue.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace glue;

struct Result { double thdPct, thdDb, h2Db, h3Db; };

static Result Measure(const Presets &p, double fs, const GlobalSettings &g, double peakDb)
{
    ChannelChain c; c.reset(&p, 31); c.setGlobal(g); c.setChannel(ChannelSettings());
    const double f0 = 1000.0, a = pow(10.0, peakDb / 20.0), w = 2 * 3.14159265358979323846 * f0 / fs;
    const int settle = (int)(0.5 * fs), n = (int)(1000 * fs / f0);
    std::vector<float> y((size_t)(settle + n));
    for (size_t i = 0; i < y.size(); i++) y[i] = (float)(a * sin(w * (double)i));
    for (size_t i = 0; i < y.size(); i += 64) c.process(&y[i], &y[i], (int)std::min<size_t>(64, y.size() - i));
    double h[11] = {};
    for (int k = 1; k <= 10; k++) {
        double re = 0, im = 0;
        for (int i = 0; i < n; i++) { const double t = (double)(settle + i); re += y[(size_t)(settle + i)] * cos(w * k * t); im += y[(size_t)(settle + i)] * sin(w * k * t); }
        h[k] = re * re + im * im;
    }
    double sum = 0; for (int k = 2; k <= 10; k++) sum += h[k];
    Result r;
    r.thdPct = 100.0 * sqrt(sum / h[1]);
    r.thdDb = 10.0 * log10(sum / h[1]);
    r.h2Db = 10.0 * log10(h[2] / h[1] + 1e-30);
    r.h3Db = 10.0 * log10(h[3] / h[1] + 1e-30);
    return r;
}

int main(int argc, char **argv)
{
    const double fs = argc > 1 ? atof(argv[1]) : 48000.0;
    Presets p; p.calibrate(fs);
    const double levels[4] = { -18, -12, -6, 0 };
    struct Config { const char *name; int lv[kStageCount]; };
    const Config cfg[] = {
        { "dry (Glue on, every stage Off)",          { 0, 0, 0, 0 } },
        { "Transformer Full",                         { kLevelFull, 0, 0, 0 } },
        { "Tube Full",                                { 0, kLevelFull, 0, 0 } },
        { "Desk Neve High",                           { 0, 0, kDeskNeveHigh, 0 } },
        { "Desk API High",                            { 0, 0, kDeskAPIHigh, 0 } },
        { "Desk SSL High",                            { 0, 0, kDeskSSLHigh, 0 } },
        { "Tape Full",                                { 0, 0, 0, kLevelFull } },
        { "All Full + Desk Neve High",                { kLevelFull, kLevelFull, kDeskNeveHigh, kLevelFull } },
        { "All Full + Desk API High",                 { kLevelFull, kLevelFull, kDeskAPIHigh, kLevelFull } },
        { "All Full + Desk SSL High",                 { kLevelFull, kLevelFull, kDeskSSLHigh, kLevelFull } },
    };
    printf("THD (harmonics 2-10) of a 1 kHz sine at %.0f Hz, compressor off, EQ flat: %% / dB, and H2 / H3 in dB\n", fs);
    printf("%-32s", "");
    for (double l : levels) printf("  | input %+3.0f dBFS               ", l);
    printf("\n");
    for (const Config &c : cfg) {
        GlobalSettings g; for (int s = 0; s < kStageCount; s++) g.level[s] = (uint8_t)c.lv[s];
        printf("%-32s", c.name);
        for (double l : levels) {
            const Result r = Measure(p, fs, g, l);
            printf("  | %6.3f%% %6.1f  H2 %6.1f H3 %6.1f", r.thdPct, r.thdDb, r.h2Db, r.h3Db);
        }
        printf("\n");
    }
    return 0;
}
