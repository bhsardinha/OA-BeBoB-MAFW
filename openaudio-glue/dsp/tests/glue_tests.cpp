// OpenAudio Glue - host tests for the DSP chain (make -C dsp test). Same style as the panel's Tests/LayoutTests.swift.
#include "../Glue.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>

using namespace glue;

static int gFailures = 0;
static void check(bool cond, const char *msg)
{
    if (!cond) { gFailures++; printf("FAIL: %s\n", msg); }
}
static void checkNear(double got, double want, double tol, const char *msg)
{
    if (!(fabs(got - want) <= tol)) { gFailures++; printf("FAIL: %s (got %.4f, want %.4f +- %.4f)\n", msg, got, want, tol); }
}

static const double kPi = 3.14159265358979323846;
static const double kRates[] = { 44100, 48000, 88200, 96000, 176400, 192000 };

static double Db(double rms) { return 20.0 * log10(rms + 1e-30); }
// dB of a sine's peak from its RMS (sine-peak dBFS, the panel's scale)
static double SinePeakDb(double rms) { return Db(rms * sqrt(2.0)); }

// Runs `seconds` of a sine at `hz`, `peakDb` through the chain in 64-frame blocks; returns the RMS of the last `measure` s.
static double RunSine(ChannelChain &c, double rate, double hz, double peakDb, double seconds, double measure)
{
    const double amp = pow(10.0, peakDb / 20.0), w = 2.0 * kPi * hz / rate;
    const int total = (int)(seconds * rate), from = total - (int)(measure * rate);
    float buf[64];
    double sum = 0.0; long n = 0, count = 0;
    while (n < total) {
        const int k = (total - n) < 64 ? (int)(total - n) : 64;
        for (int i = 0; i < k; i++) buf[i] = (float)(amp * sin(w * (double)(n + i)));
        c.process(buf, buf, k);
        for (int i = 0; i < k; i++) if (n + i >= from) { sum += (double)buf[i] * buf[i]; count++; }
        n += k;
    }
    return sqrt(sum / (double)count);
}

static uint32_t gRand = 22222;
static float Noise() { gRand ^= gRand << 13; gRand ^= gRand >> 17; gRand ^= gRand << 5; return (float)((double)gRand / 4294967295.0 * 2.0 - 1.0); }

static void TestBypass()
{
    Presets p; p.calibrate(48000);
    ChannelChain c; c.reset(&p, 7);
    std::vector<float> in(4096), out(4096);
    for (auto &v : in) v = Noise() * 0.5f;
    const int L = p.maxLatency();
    bool exact = true;
    for (int b = 0; b < 4096; b += 64) {
        c.process(&in[b], &out[b], 64);
        for (int i = b; i < b + 64; i++) if (out[i] != (i >= L ? in[i - L] : 0.0f)) exact = false;
    }
    check(exact, "bypass: default settings are bit-exact, delayed by the fixed latency");
    check(c.bypassed(), "bypass: bypassed() at default settings");

    // Switch everything on, then everything off again: once the EQ has settled the chain is bit-exact again.
    GlobalSettings g; for (int s = 0; s < kStageCount; s++) g.level[s] = (uint8_t)(StageOptionCount(s) - 1);
    ChannelSettings ch; ch.thresholdDb = -20; for (int b = 0; b < kBandCount; b++) ch.band[b].gainDb = 6;
    c.setGlobal(g); c.setChannel(ch);
    for (int b = 0; b < 4096; b += 64) c.process(&in[b], &out[b], 64);
    check(!c.bypassed(), "bypass: not bypassed with everything on");
    c.setGlobal(GlobalSettings()); c.setChannel(ChannelSettings());
    for (int r = 0; r < 20; r++) for (int b = 0; b < 4096; b += 64) c.process(&in[b], &out[b], 64);   // ~1.7 s
    exact = true;
    for (int b = 0; b < 4096; b += 64) {
        c.process(&in[b], &out[b], 64);
        for (int i = b + L; i < b + 64; i++) if (out[i] != in[i - L]) exact = false;
    }
    check(c.bypassed(), "bypass: bypassed() after everything is switched off");
    check(exact, "bypass: bit-exact again after everything is switched off (delayed by the fixed latency)");
}

// A channel switched off passes through bit-exact even with every stage at its strongest, and costs nothing.
static void TestChannelOff()
{
    Presets p; p.calibrate(48000);
    ChannelChain c; c.reset(&p, 9);
    GlobalSettings g; for (int s = 0; s < kStageCount; s++) g.level[s] = (uint8_t)(StageOptionCount(s) - 1);
    ChannelSettings ch; ch.on = false; ch.thresholdDb = -30; ch.band[1].gainDb = 6;
    c.setGlobal(g); c.setChannel(ch);
    std::vector<float> in(2048), out(2048);
    for (auto &v : in) v = Noise() * 0.5f;
    const int L = p.maxLatency();
    bool exact = true;
    for (int b = 0; b < 2048; b += 64) {
        c.process(&in[b], &out[b], 64);
        for (int i = b; i < b + 64; i++) if (out[i] != (i >= L ? in[i - L] : 0.0f)) exact = false;
    }
    check(c.bypassed(), "channel off: bypassed() with every stage on");
    check(exact, "channel off: bit-exact pass-through with every stage on, delayed by the fixed latency");
    ch.on = true; c.setChannel(ch);
    c.process(&in[0], &out[0], 64);
    check(!c.bypassed(), "channel back on: processing again");
}

// Delay compensation: the whole chain's delay at 1 kHz stays at the fixed latency (within half a sample) whatever stages are on.
static double ChainPhaseDelay(const Presets &p, double fs, const GlobalSettings &g, bool on)
{
    ChannelChain c; c.reset(&p, 21); c.setGlobal(g);
    ChannelSettings ch; ch.on = on; c.setChannel(ch);
    const double w = 2 * kPi * 1000 / fs, a = pow(10.0, -18.0 / 20.0);
    double yr = 0, yi = 0, xr = 0, xi = 0; float buf[64];
    const int N = (int)(0.6 * fs);
    for (int n = 0; n < N; n += 64) {
        for (int i = 0; i < 64; i++) buf[i] = (float)(a * sin(w * (n + i)));
        c.process(buf, buf, 64);
        if (n > N / 2) for (int i = 0; i < 64; i++) {
            yr += buf[i] * cos(w * (n + i)); yi += buf[i] * sin(w * (n + i));
            xr += a * sin(w * (n + i)) * cos(w * (n + i)); xi += a * sin(w * (n + i)) * sin(w * (n + i));
        }
    }
    double ph = atan2(yr, yi) - atan2(xr, xi);
    while (ph > kPi) ph -= 2 * kPi;
    while (ph < -kPi) ph += 2 * kPi;
    return -ph / w;
}

static void TestDelayCompensation()
{
    for (double fs : kRates) {
        Presets p; p.calibrate(fs);
        const int L = p.maxLatency();
        double worst = 0;
        GlobalSettings g;
        auto probe = [&](const GlobalSettings &gs, bool on) { const double d = fabs(ChainPhaseDelay(p, fs, gs, on) - L); if (d > worst) worst = d; };
        probe(g, true);                                                   // nothing active
        probe(g, false);                                                  // switched off
        for (int s = 0; s < kStageCount; s++) for (int o = 1; o < StageOptionCount(s); o++) { GlobalSettings one; one.level[s] = (uint8_t)o; probe(one, true); }
        for (int s = 0; s < kStageCount; s++) g.level[s] = (uint8_t)(StageOptionCount(s) - 1);
        probe(g, true);                                                   // everything at its strongest
        probe(g, false);
        char msg[128];
        snprintf(msg, sizeof msg, "delay: every configuration within 0.6 samples of the fixed latency %d at %.0f Hz (worst %.2f)", L, fs, worst);
        check(worst <= 0.6, msg);
        printf("delay compensation %6.0f Hz: fixed latency %d samples, worst deviation %.2f\n", fs, L, worst);
    }
}

// Stereo link: the same reduction on both sides; identical inputs give identical outputs; unlinked sides differ.
static void TestStereoLink()
{
    const double fs = 48000;
    Presets p; p.calibrate(fs);
    ChannelSettings ch; ch.thresholdDb = -30; ch.source = kSourceDrums;
    GlobalSettings g; g.level[kStageTape] = kLevelMedium;
    ChannelChain a, b; a.reset(&p, 41); b.reset(&p, 42);
    a.setGlobal(g); b.setGlobal(g); a.setChannel(ch); b.setChannel(ch);
    // left loud, right quiet: linked, the quiet side is pulled down as much as the loud one
    std::vector<float> l(64), r(64), ol(64), orr(64);
    double grL = 0, grR = 0;
    for (int blk = 0; blk < 750; blk++) {                     // 1 s
        for (int i = 0; i < 64; i++) { const double t = (blk * 64 + i) / fs; l[i] = (float)(0.5 * sin(2 * kPi * 200 * t)); r[i] = (float)(0.05 * sin(2 * kPi * 300 * t)); }
        ChannelChain::processPair(a, b, l.data(), r.data(), ol.data(), orr.data(), 64);
    }
    grL = a.gainReductionDb(); grR = b.gainReductionDb();
    // the right side's own detector sees little; its applied gain is the left's: compare its output with an unlinked chain
    ChannelChain u; u.reset(&p, 42); u.setGlobal(g); u.setChannel(ch);
    std::vector<float> ou(64);
    double sumLinked = 0, sumAlone = 0;
    for (int blk = 0; blk < 750; blk++) {
        for (int i = 0; i < 64; i++) { const double t = (blk * 64 + i) / fs; r[i] = (float)(0.05 * sin(2 * kPi * 300 * t)); }
        u.process(r.data(), ou.data(), 64);
        if (blk > 600) for (int i = 0; i < 64; i++) { sumAlone += (double)ou[i] * ou[i]; }
    }
    ChannelChain a2, b2; a2.reset(&p, 41); b2.reset(&p, 42);
    a2.setGlobal(g); b2.setGlobal(g); a2.setChannel(ch); b2.setChannel(ch);
    for (int blk = 0; blk < 750; blk++) {
        for (int i = 0; i < 64; i++) { const double t = (blk * 64 + i) / fs; l[i] = (float)(0.5 * sin(2 * kPi * 200 * t)); r[i] = (float)(0.05 * sin(2 * kPi * 300 * t)); }
        ChannelChain::processPair(a2, b2, l.data(), r.data(), ol.data(), orr.data(), 64);
        if (blk > 600) for (int i = 0; i < 64; i++) { sumLinked += (double)orr[i] * orr[i]; }
    }
    char msg[160];
    snprintf(msg, sizeof msg, "link: the quiet side is reduced like the loud one (own detector %.1f dB, linked output %.1f dB below unlinked)",
             grR, 10 * log10(sumAlone / sumLinked));
    check(10 * log10(sumAlone / sumLinked) > 3.0, msg);
    check(grL < grR - 3.0, "link: each side's detector still measures its own level");
    // identical inputs: identical outputs
    ChannelChain c1, c2; c1.reset(&p, 7); c2.reset(&p, 7);
    c1.setGlobal(g); c2.setGlobal(g); c1.setChannel(ch); c2.setChannel(ch);
    bool same = true;
    for (int blk = 0; blk < 200; blk++) {
        for (int i = 0; i < 64; i++) l[i] = r[i] = Noise() * 0.4f;
        ChannelChain::processPair(c1, c2, l.data(), r.data(), ol.data(), orr.data(), 64);
        if (memcmp(ol.data(), orr.data(), 64 * 4) != 0) same = false;
    }
    check(same, "link: identical inputs give identical outputs");
}

// Global Drive: more harmonics, about the same level.
static void TestDrive()
{
    const double fs = 48000;
    Presets p; p.calibrate(fs);
    double thd[2], lvl[2];
    for (int k = 0; k < 2; k++) {
        ChannelChain c; c.reset(&p, 5);
        GlobalSettings g; g.level[kStageTube] = kLevelMedium; g.driveDb = k ? 12.0f : 0.0f; c.setGlobal(g);
        const int N = 48000, S = 24000; std::vector<float> y(N);
        for (int n = 0; n < N; n++) y[n] = (float)(pow(10, -18.0 / 20) * sin(2 * kPi * 1000 * n / fs));
        for (int n = 0; n < N; n += 64) c.process(&y[n], &y[n], 64);
        double h1 = 0, hs = 0, ss = 0;
        for (int h = 1; h <= 10; h++) { double re = 0, im = 0; for (int n = S; n < N; n++) { re += y[n] * cos(2 * kPi * 1000 * h * n / fs); im += y[n] * sin(2 * kPi * 1000 * h * n / fs); } if (h == 1) h1 = re * re + im * im; else hs += re * re + im * im; }
        for (int n = S; n < N; n++) ss += (double)y[n] * y[n];
        thd[k] = 100 * sqrt(hs / h1); lvl[k] = 10 * log10(ss / (N - S));
    }
    char msg[128]; snprintf(msg, sizeof msg, "drive: +12 dB raises THD (%.2f %% -> %.2f %%)", thd[0], thd[1]);
    check(thd[1] > thd[0] * 2, msg);
    snprintf(msg, sizeof msg, "drive: the level stays within 3 dB (%.1f -> %.1f dB)", lvl[0], lvl[1]);
    check(fabs(lvl[1] - lvl[0]) < 3.0, msg);
}

static void TestEQAnalytic()
{
    const double fs = 48000;
    checkNear(EQBandMagnitudeDb(kBandLowMid, 400, 6, fs, 400), 6.0, 0.01, "eq: low-mid +6 dB at its centre");
    checkNear(EQBandMagnitudeDb(kBandHighMid, 2500, -4, fs, 2500), -4.0, 0.01, "eq: high-mid -4 dB at its centre");
    checkNear(EQBandMagnitudeDb(kBandLowShelf, 100, 6, fs, 100), 3.0, 0.05, "eq: low shelf is half its gain at f0");
    checkNear(EQBandMagnitudeDb(kBandLowShelf, 100, 6, fs, 10), 6.0, 0.3, "eq: low shelf reaches its gain far below f0");
    checkNear(EQBandMagnitudeDb(kBandLowShelf, 100, 6, fs, 5000), 0.0, 0.05, "eq: low shelf flat far above f0");
    checkNear(EQBandMagnitudeDb(kBandHighShelf, 8000, 6, fs, 8000), 3.0, 0.05, "eq: high shelf is half its gain at f0");
    checkNear(EQBandMagnitudeDb(kBandHighShelf, 8000, 6, fs, 23900), 6.0, 0.3, "eq: high shelf reaches its gain near Nyquist");
    checkNear(EQBandMagnitudeDb(kBandHighShelf, 8000, 6, fs, 100), 0.0, 0.05, "eq: high shelf flat far below f0");
    check(EQProportionalQ(12) > EQProportionalQ(3), "eq: proportional Q narrows as gain grows");
    // Proportional Q: one octave above the centre, a +12 dB bell keeps less of its gain than a +3 dB bell does.
    const double r12 = EQBandMagnitudeDb(kBandLowMid, 400, 12, fs, 800) / 12.0;
    const double r3 = EQBandMagnitudeDb(kBandLowMid, 400, 3, fs, 800) / 3.0;
    check(r12 < r3, "eq: a bigger boost is relatively narrower (API-style)");
    checkNear(EQBandMagnitudeDb(kBandLowMid, 5, 30, fs, 60), 12.0, 0.01, "eq: frequency and gain are clamped to the band's range");
}

static void TestEQFilter()
{
    for (double fs : kRates) {
        Presets p; p.calibrate(fs);
        ChannelChain c; c.reset(&p, 3);
        ChannelSettings ch;
        ch.band[kBandLowMid] = { 400, 6 };
        ch.band[kBandHighMid] = { 2500, -4 };
        c.setChannel(ch);
        const double want = EQBandMagnitudeDb(kBandLowMid, 400, 6, fs, 1000) + EQBandMagnitudeDb(kBandHighMid, 2500, -4, fs, 1000);
        const double got = SinePeakDb(RunSine(c, fs, 1000, -18, 1.0, 0.3)) + 18.0;
        char msg[96]; snprintf(msg, sizeof msg, "eq: filter matches its formula at 1 kHz, %.0f Hz", fs);
        checkNear(got, want, 0.05, msg);
    }
}

static void TestCompStatic()
{
    checkNear(CompStaticGainReductionDb(-10, -30), (-30 + 20.0 / 3.0) + 10, 1e-9, "comp: 3:1 above the knee");
    checkNear(CompStaticGainReductionDb(-30, -30), (1.0 / 3.0 - 1.0) * 9.0 / 12.0, 1e-9, "comp: soft knee at the threshold");
    checkNear(CompStaticGainReductionDb(-34, -30), 0.0, 1e-9, "comp: no reduction below the knee");
    checkNear(CompMakeupDb(0), 0.0, 1e-9, "comp: no makeup when off");
    checkNear(CompMakeupDb(-30), 4.0, 1e-9, "comp: makeup is half the reduction at -18 dBFS");
}

static void TestCompDynamic()
{
    const double fs = 48000;
    Presets p; p.calibrate(fs);
    ChannelChain c; c.reset(&p, 5);
    ChannelSettings ch; ch.thresholdDb = -30; c.setChannel(ch);
    const double outDb = SinePeakDb(RunSine(c, fs, 1000, -10, 3.0, 0.5));
    checkNear(c.gainReductionDb(), CompStaticGainReductionDb(-10, -30), 0.5, "comp: steady-state reduction follows the curve");
    checkNear(outDb, -10 + CompStaticGainReductionDb(-10, -30) + CompMakeupDb(-30), 0.5, "comp: output level = input + reduction + makeup");
    // Program dependence: after a short burst the reduction recovers faster than after a long one.
    ChannelChain a; a.reset(&p, 6); a.setChannel(ch);
    ChannelChain b; b.reset(&p, 6); b.setChannel(ch);
    RunSine(a, fs, 1000, -6, 0.03, 0.01);   // 30 ms burst
    RunSine(b, fs, 1000, -6, 2.0, 0.01);    // 2 s sustained
    RunSine(a, fs, 1000, -60, 0.15, 0.01);  // 150 ms of near-silence
    RunSine(b, fs, 1000, -60, 0.15, 0.01);
    check(a.gainReductionDb() > b.gainReductionDb(), "comp: a short burst recovers faster than a sustained signal");
}

// Every voicing: the steady-state reduction of a -10 dBFS sine follows that voicing's curve.
static void TestCompSources()
{
    const double fs = 48000;
    Presets p; p.calibrate(fs);
    for (int src = 0; src < kSourceCount; src++) {
        ChannelChain c; c.reset(&p, 5);
        ChannelSettings ch; ch.thresholdDb = -30; ch.source = (uint8_t)src; c.setChannel(ch);
        RunSine(c, fs, 1000, -10, 3.0, 0.5);
        char msg[96]; snprintf(msg, sizeof msg, "comp %s: steady-state reduction follows its curve", CompSourceName(src));
        checkNear(c.gainReductionDb(), CompStaticGainReductionDb(-10, -30, src), 0.6, msg);
        if (src == kSourceDrums) checkNear(c.hitLiftDb(), 0.0, 0.3, "comp Drums: no hit lift on a steady tone");
    }
}

// Drum-like hits (1 kHz, instant onset, 150 ms decay, every 500 ms, -6 dBFS): "punch" = peak of the first 5 ms over the
// RMS 100-150 ms after the onset. General's 10 ms attack already lets the first 5 ms through (its punch rises too); the
// Drums voicing must go clearly further than both dry and General.
static double Punch(ChannelChain *c, double fs)
{
    const int period = (int)(0.5 * fs), hits = 6;
    std::vector<float> x((size_t)period * hits);
    for (int n = 0; n < (int)x.size(); n++) {
        const int t = n % period;
        x[(size_t)n] = (float)(0.5 * exp(-t / (0.15 * fs)) * sin(2 * kPi * 1000 * t / fs));
    }
    if (c) for (size_t b = 0; b < x.size(); b += 64) c->process(&x[b], &x[b], (int)std::min<size_t>(64, x.size() - b));
    const int start = period * (hits - 1);                   // the last hit, after the envelopes have settled
    double pk = 0, ss = 0; int sn = 0;
    for (int t = 0; t < (int)(0.005 * fs); t++) pk = std::max(pk, (double)fabs(x[(size_t)(start + t)]));
    for (int t = (int)(0.100 * fs); t < (int)(0.150 * fs); t++) { ss += (double)x[(size_t)(start + t)] * x[(size_t)(start + t)]; sn++; }
    return 20 * log10(pk) - 20 * log10(sqrt(ss / sn));
}

static void TestTransientKeep()
{
    const double fs = 48000;
    Presets p; p.calibrate(fs);
    const double dry = Punch(nullptr, fs);
    ChannelChain g; g.reset(&p, 8);
    ChannelSettings ch; ch.thresholdDb = -24; ch.source = kSourceGeneral; g.setChannel(ch);
    const double general = Punch(&g, fs);
    ChannelChain d; d.reset(&p, 8);
    ch.source = kSourceDrums; d.setChannel(ch);
    const double drums = Punch(&d, fs);
    printf("punch (hit peak over sustain): dry %.1f dB, General %.1f dB, Drums %.1f dB\n", dry, general, drums);
    check(drums > dry + 3.0, "comp Drums: the hit stands out more than dry (expander on the hit, compressor on the sustain)");
    check(drums > general + 3.0, "comp Drums: keeps clearly more of the hit than General");
}

static void TestLevelMatch()
{
    for (double fs : kRates) {
        Presets p; p.calibrate(fs);
        for (int s = 0; s < kStageCount; s++) for (int l = 1; l < StageOptionCount(s); l++) {
            ChannelChain c; c.reset(&p, 11);
            GlobalSettings g; g.level[s] = (uint8_t)l; c.setGlobal(g);
            const double got = SinePeakDb(RunSine(c, fs, 1000, -18, 0.8, 0.3));
            char msg[96]; snprintf(msg, sizeof msg, "presets: %s %s keeps -18 dBFS at %.0f Hz", StageName(s), StageOptionName(s, l), fs);
            checkNear(got, -18.0, 0.5, msg);
        }
    }
}

static void TestStability()
{
    for (double fs : kRates) {
        Presets p; p.calibrate(fs);
        ChannelSettings ch; ch.thresholdDb = -40;
        ch.band[0].gainDb = 12; ch.band[1].gainDb = -12; ch.band[2].gainDb = 12; ch.band[3].gainDb = 12;
        bool finite = true; double peak = 0;
        for (int deskOpt = 1; deskOpt < kDeskOptionCount; deskOpt++) {      // every other stage Full, each desk option
            GlobalSettings g; for (int s = 0; s < kStageCount; s++) g.level[s] = kLevelFull;
            g.level[kStageDesk] = (uint8_t)deskOpt;
            ChannelChain c; c.reset(&p, 13); c.setGlobal(g); c.setChannel(ch);
            float buf[64];
            const int blocks = (int)(1.0 * fs / 64);
            for (int b = 0; b < blocks; b++) {
                for (int i = 0; i < 64; i++) buf[i] = (b < blocks / 2) ? Noise() : (float)(4.0 * sin(2 * kPi * 50 * (b * 64 + i) / fs));
                c.process(buf, buf, 64);
                for (int i = 0; i < 64; i++) { if (!std::isfinite(buf[i])) finite = false; if (fabs(buf[i]) > peak) peak = fabs(buf[i]); }
            }
        }
        char msg[96]; snprintf(msg, sizeof msg, "stability: finite under overload at %.0f Hz", fs);
        check(finite, msg);
        snprintf(msg, sizeof msg, "stability: output peak %.2f stays under 2.0 at %.0f Hz", peak, fs);
        check(peak < 2.0, msg);
    }
}

int main()
{
    TestBypass();
    TestChannelOff();
    TestDelayCompensation();
    TestStereoLink();
    TestDrive();
    TestEQAnalytic();
    TestEQFilter();
    TestCompStatic();
    TestCompDynamic();
    TestCompSources();
    TestTransientKeep();
    TestLevelMatch();
    TestStability();
    if (gFailures) { printf("%d FAILURE(S)\n", gFailures); return 1; }
    printf("ALL PASS\n");
    return 0;
}
