// OpenAudio Glue - port-diff: each mono port (dsp/AirwindowsMono.*) against its Airwindows original (ref/), fed L = R,
// for the originals' defaults and every Glue preset level, at 44.1/48/96/192 kHz. Prints the residual (original L minus
// port) relative to the original's output. Both sides start the per-stage dither from the same seed (originals.cpp), so a
// faithful port matches to float rounding.
// ToTape9 is compared with flutter off (D = 0) and on. With it on, the mono port picks its next random flutter speed against
// its own sweep instead of the other channel's (docs/DSP_v1.md); with L = R those differ by one sweep step (~0.001 rad), so
// the choice matches the original in practice and the case is checked like the others.
#include "Glue.h"
#include <cmath>
#include <cstdio>
#include <vector>

using namespace glue;

void RunOriginal_Interstage(const std::vector<float> &, double, const float *, float *, int);
void RunOriginal_Tube2(const std::vector<float> &, double, const float *, float *, int);
void RunOriginal_Channel9(const std::vector<float> &, double, const float *, float *, int);
void RunOriginal_ToTape9(const std::vector<float> &, double, const float *, float *, int);

static const double kPi = 3.14159265358979323846;
static int gFail = 0;

// 0.6 s of program at -12 dBFS (three sines and a little noise), then 0.6 s driven to +3 dBFS.
static std::vector<float> Signal(double rate)
{
    const int half = (int)(0.6 * rate);
    std::vector<float> s((size_t)half * 2);
    uint32_t r = 12345;
    for (int n = 0; n < half * 2; n++) {
        r ^= r << 13; r ^= r >> 17; r ^= r << 5;
        const double t = n / rate, noise = ((double)r / 4294967295.0 * 2.0 - 1.0) * 0.01;
        const double prog = 0.5 * sin(2 * kPi * 110 * t) + 0.3 * sin(2 * kPi * 1000 * t) + 0.2 * sin(2 * kPi * 5500 * t) + noise;
        s[(size_t)n] = (float)(prog * (n < half ? 0.2512 : 1.4125));
    }
    return s;
}

template <class Port>
static void Compare(const char *name, const char *label, double rate, const std::vector<float> &in,
                    void (*orig)(const std::vector<float> &, double, const float *, float *, int),
                    const std::vector<float> &params, Port &port, double limitDb, bool info)
{
    std::vector<float> o(in.size());
    orig(params, rate, in.data(), o.data(), (int)in.size());
    double se = 0, so = 0, maxd = 0;
    for (size_t n = 0; n < in.size(); n++) {
        const float p = (float)port.process(in[n]);
        const double d = (double)o[n] - p;
        se += d * d; so += (double)o[n] * o[n];
        if (fabs(d) > maxd) maxd = fabs(d);
    }
    const double resDb = 10 * log10((se + 1e-300) / (so + 1e-300));
    const bool ok = info || resDb <= limitDb;
    if (!ok) gFail++;
    printf("%-4s %-10s %-22s %6.0f Hz  residual %7.1f dB  max|diff| %.2e%s\n", ok ? (info ? "info" : "ok") : "FAIL",
           name, label, rate, resDb, maxd, info ? "  (expected to drift)" : "");
}

int main()
{
    const double limitDb = -120.0;
    const double rates[] = { 44100, 48000, 96000, 192000 };
    for (double rate : rates) {
        const std::vector<float> in = Signal(rate);
        { Interstage p; p.reset(rate); Compare("Interstage", "(no parameters)", rate, in, RunOriginal_Interstage, {}, p, limitDb, false); }
        for (int l = 0; l < kLevelCount; l++) {
            const StagePreset sp = GetStagePreset(kStageTube, l);
            const double A = l ? sp.tubeA : 0.5, B = l ? sp.tubeB : 0.5;
            Tube2 p; p.reset(rate); p.prepare(A, B);
            char label[40]; snprintf(label, sizeof label, "%s A%.2f B%.2f", l ? StageOptionName(kStageTube, l) : "defaults", A, B);
            Compare("Tube2", label, rate, in, RunOriginal_Tube2, { (float)A, (float)B }, p, limitDb, false);
        }
        for (int l = 0; l < StageOptionCount(kStageDesk); l++) {
            const StagePreset sp = GetStagePreset(kStageDesk, l);
            const double B = l ? sp.deskDrive : 0.0;
            for (int type = 0; type < 5; type++) {
                if (l && type != sp.deskType) continue;                    // a preset uses its own type; defaults try every type
                Channel9 p; p.reset(rate); p.prepare(type, B, 1.0);
                char label[40]; snprintf(label, sizeof label, "%s type%d B%.2f", l ? StageOptionName(kStageDesk, l) : "defaults", type, B);
                Compare("Channel9", label, rate, in, RunOriginal_Channel9, { (float)(type / 4.0), (float)B, 1.0f }, p, limitDb, false);
            }
        }
        for (int l = 0; l < kLevelCount; l++) {
            ToTape9::Params tp = l ? GetStagePreset(kStageTape, l).tape : ToTape9::Params();
            for (int flutter = 0; flutter < 2; flutter++) {
                ToTape9::Params q = tp;
                if (!flutter) q.D = 0.0;
                ToTape9 p; p.reset(rate); p.prepare(q);
                char label[40]; snprintf(label, sizeof label, "%s%s", l ? StageOptionName(kStageTape, l) : "defaults", flutter ? " flutter" : " D=0");
                Compare("ToTape9", label, rate, in, RunOriginal_ToTape9,
                        { (float)q.A, (float)q.B, (float)q.C, (float)q.D, (float)q.E, (float)q.F, (float)q.G, (float)q.H, (float)q.I },
                        p, limitDb, false);
            }
        }
    }
    if (gFail) printf("%d FAILURE(S)\n", gFail);
    else printf("ALL PORTS MATCH (limit %.0f dB)\n", limitDb);
    return gFail ? 1 : 0;
}
