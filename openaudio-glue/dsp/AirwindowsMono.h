/* ========================================
 *  OpenAudio Glue - mono ports of four Airwindows plug-ins.
 *  Interstage, Tube2, Channel9, ToTape9: Copyright (c) Airwindows (Chris Johnson), Airwindows uses the MIT license.
 *  Source: github.com/airwindows/airwindows @ dfa54018e788fd67b8e57ae6c8012a05f1f086a6 (2026-10-24),
 *  plugins/MacVST/<name>/source/<name>Proc.cpp, processReplacing (the float path). Licence text: ../NOTICE.md.
 *
 *  Port rules (docs/DSP_v1.md): one channel of state and logic (the original's L). The only place L reads R state is
 *  ToTape9's flutter (sweepR, nextmaxR); that R state never depends on the audio, so the port keeps a shadow of it
 *  (sweepR, nextmaxR, fpdR) and stays identical to the original's L. The per-stage 32-bit float dither is kept, with
 *  ldexp(1, e) for pow(2, e) (same value); per-block constants are computed in prepare(). Checked against the
 *  originals by tools/port-diff (L = R input, same seed, L output compared).
 * ======================================== */
#pragma once
#include <cstdint>

namespace glue {

// Interstage: no parameters (a fixed transformer between stages).
struct Interstage {
    void reset(double sampleRate, uint32_t seed = 2463534242u);
    double process(double x);
private:
    double firstStage = 0, iirAmount = 0, threshold = 0;
    double iirA = 0, iirB = 0, iirC = 0, iirD = 0, iirE = 0, iirF = 0, last = 0;
    uint32_t fpd = 17;
    bool flip = true;
};

// Tube2: A = Input (pad below 1), B = Tube.
struct Tube2 {
    void reset(double sampleRate, uint32_t seed = 2463534242u);
    void prepare(double A, double B);
    double process(double x);
private:
    double overallscale = 1, inputPad = 1, asymPad = 1, gainscaling = 1, outputscaling = 1;
    int powerfactor = 1;
    double prevA = 0, prevC = 0, prevE = 0;
    uint32_t fpd = 17;
};

// Channel9: type 0..4 = Neve, API, SSL, Teac, Mackie; B = Drive; C = Output.
struct Channel9 {
    enum Type { kNeve, kAPI, kSSL, kTeac, kMackie };
    void reset(double sampleRate, uint32_t seed = 2463534242u);
    void prepare(int type, double B, double C);
    double process(double x);
private:
    double sampleRate = 44100, overallscale = 1;
    double localiirAmount = 0, localthreshold = 0, density = 0, phattity = 0, nonLin = 5, C = 1;
    double bqA[15] = {}, bqB[15] = {};
    double iirA = 0, iirB = 0, lastA = 0, lastB = 0, lastC = 0;
    uint32_t fpd = 17;
    bool flip = false;
};

// ToTape9: A Input, B Tilt, C Shape, D Flutter, E FlutSpd, F Bias, G HeadBmp, H HeadFrq, I Output (all 0..1).
struct ToTape9 {
    struct Params { double A = 0.5, B = 0.5, C = 0.5, D = 0.5, E = 0.5, F = 0.5, G = 0.5, H = 0.5, I = 0.5; };
    void reset(double sampleRate, uint32_t seed = 2463534242u);
    void prepare(const Params &p);
    double process(double x);
    double headBumpHz() const { return hdbA[hdb_freq] * sampleRate; }
private:
    enum { prevSamp1, unusedR1, threshold1, prevSamp2, unusedR2, threshold2, prevSamp3, unusedR3, threshold3,
           prevSamp4, unusedR4, threshold4, prevSamp5, unusedR5, threshold5, prevSamp6, unusedR6, threshold6,
           prevSamp7, unusedR7, threshold7, prevSamp8, unusedR8, threshold8, prevSamp9, unusedR9, threshold9, gslew_total };
    enum { hdb_freq, hdb_reso, hdb_a0, hdb_a1, hdb_a2, hdb_b1, hdb_b2, hdb_s1, hdb_s2, hdb_total };
    double sampleRate = 44100, overallscale = 1;
    int spacing = 1, slewsing = 2;
    double inputGain = 1, dublyAmount = 1, outlyAmount = -1, iirEncFreq = 0, iirDecFreq = 0, flutDepth = 0,
           flutFrequency = 0, bias = 0, underBias = 0, headBumpDrive = 0, headBumpMix = 0, headBumpCube = 0, outputGain = 1;
    double iirEnc = 0, iirDec = 0, compEnc = 1, compDec = 1, avgEnc = 0, avgDec = 0;
    double d[1002] = {};
    double sweep = 3.14159265358979323846, nextmax = 0.5;
    double sweepR = 3.14159265358979323846, nextmaxR = 0.5;   // shadow of the original's R flutter state
    uint32_t fpdR = 17;
    int gcount = 0;
    double gslew[gslew_total] = {};
    double hysteresis = 0, headBump = 0;
    double hdbA[hdb_total] = {}, hdbB[hdb_total] = {};
    double avg32[33] = {}, avg16[17] = {}, avg8[9] = {}, avg4[5] = {}, avg2[3] = {};
    double post32[33] = {}, post16[17] = {}, post8[9] = {}, post4[5] = {}, post2[3] = {};
    double lastDark = 0;
    int avgPos = 0;
    double lastSample = 0, intermediate[18] = {}, slew[34] = {};
    bool wasPosClip = false, wasNegClip = false;
    uint32_t fpd = 17;
};

} // namespace glue
