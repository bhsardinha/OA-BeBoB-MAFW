/* ========================================
 *  OpenAudio Glue - mono ports of four Airwindows plug-ins (see AirwindowsMono.h for source, commit and rules).
 *  Copyright (c) Airwindows (Chris Johnson), Airwindows uses the MIT license.
 *  Comments of the form "orig: ..." name the original's variable or block; the arithmetic is unchanged.
 * ======================================== */
#include "AirwindowsMono.h"
#include <cmath>

namespace glue {

static const double kPi = 3.14159265358979323846;

// orig: "begin 32 bit stereo floating point dither" (one channel). pow(2, expon+62) -> ldexp(1.0, expon+62), same value.
static inline double FloatDither(double x, uint32_t &fpd)
{
    int expon; frexpf((float)x, &expon);
    fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
    return x + ((double(fpd) - uint32_t(0x7fffffff)) * 5.5e-36l * ldexp(1.0, expon + 62));
}

// orig: the constructor's "fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;" - a fixed non-zero seed here, so a
// run is reproducible; the xorshift never reaches 0 from a non-zero state.
static inline uint32_t Seed(uint32_t s) { return s < 16386 ? s + 16386 : s; }

// ---------------------------------------------------------------- Interstage (InterstageProc.cpp, processReplacing)
void Interstage::reset(double sampleRate, uint32_t seed)
{
    const double overallscale = sampleRate / 44100.0;
    firstStage = 0.381966011250105 / overallscale;
    iirAmount = 0.00295 / overallscale;
    threshold = 0.381966011250105;
    iirA = iirB = iirC = iirD = iirE = iirF = last = 0.0;
    fpd = Seed(seed);
    flip = true;
}

double Interstage::process(double inputSample)
{
    if (fabs(inputSample) < 1.18e-23) inputSample = fpd * 1.18e-17;
    const double drySample = inputSample;
    inputSample = (inputSample + last) * 0.5;                                 // orig: start the lowpassing with an average
    if (flip) {
        iirA = (iirA * (1 - firstStage)) + (inputSample * firstStage); inputSample = iirA;
        iirC = (iirC * (1 - iirAmount)) + (inputSample * iirAmount); inputSample = iirC;
        iirE = (iirE * (1 - iirAmount)) + (inputSample * iirAmount); inputSample = iirE;
        inputSample = drySample - inputSample;                                // orig: make highpass
        if (inputSample - iirA > threshold) inputSample = iirA + threshold;   // orig: slew limit against lowpassed reference
        if (inputSample - iirA < -threshold) inputSample = iirA - threshold;
    } else {
        iirB = (iirB * (1 - firstStage)) + (inputSample * firstStage); inputSample = iirB;
        iirD = (iirD * (1 - iirAmount)) + (inputSample * iirAmount); inputSample = iirD;
        iirF = (iirF * (1 - iirAmount)) + (inputSample * iirAmount); inputSample = iirF;
        inputSample = drySample - inputSample;
        if (inputSample - iirB > threshold) inputSample = iirB + threshold;
        if (inputSample - iirB < -threshold) inputSample = iirB - threshold;
    }
    flip = !flip;
    last = inputSample;
    return FloatDither(inputSample, fpd);
}

// ---------------------------------------------------------------- Tube2 (Tube2Proc.cpp, processReplacing; L uses A, C, E)
void Tube2::reset(double sampleRate, uint32_t seed)
{
    overallscale = sampleRate / 44100.0;
    prevA = prevC = prevE = 0.0;
    fpd = Seed(seed);
    prepare(0.5, 0.5);
}

void Tube2::prepare(double A, double B)
{
    inputPad = A;
    const double iterations = 1.0 - B;
    powerfactor = (9.0 * iterations) + 1;                                     // orig: int powerfactor = (9.0*iterations)+1;
    asymPad = (double)powerfactor;
    gainscaling = 1.0 / (double)(powerfactor + 1);
    outputscaling = 1.0 + (1.0 / (double)(powerfactor));
}

double Tube2::process(double inputSample)
{
    if (fabs(inputSample) < 1.18e-23) inputSample = fpd * 1.18e-17;
    if (inputPad < 1.0) inputSample *= inputPad;
    if (overallscale > 1.9) {                                                 // orig: simple average at high sample rates
        double stored = inputSample;
        inputSample += prevA; prevA = stored; inputSample *= 0.5;
    }
    if (inputSample > 1.0) inputSample = 1.0;
    if (inputSample < -1.0) inputSample = -1.0;

    // orig: flatten bottom, point top of sine waveshaper
    inputSample /= asymPad;
    double sharpen = -inputSample;
    if (sharpen > 0.0) sharpen = 1.0 + sqrt(sharpen);
    else sharpen = 1.0 - sqrt(-sharpen);
    inputSample -= inputSample * fabs(inputSample) * sharpen * 0.25;
    inputSample *= asymPad;

    // orig: original Tube algorithm: powerfactor widens the more linear region of the wave
    double factor = inputSample;
    for (int x = 0; x < powerfactor; x++) factor *= inputSample;
    if ((powerfactor % 2 == 1) && (inputSample != 0.0)) factor = (factor / inputSample) * fabs(inputSample);
    factor *= gainscaling;
    inputSample -= factor;
    inputSample *= outputscaling;

    if (overallscale > 1.9) {
        double stored = inputSample;
        inputSample += prevC; prevC = stored; inputSample *= 0.5;
    }

    // orig: hysteresis and spiky fuzz
    double slew = prevE - inputSample;
    if (overallscale > 1.9) {
        double stored = inputSample;
        inputSample += prevE; prevE = stored; inputSample *= 0.5;
    } else prevE = inputSample;
    if (slew > 0.0) slew = 1.0 + (sqrt(slew) * 0.5);
    else slew = 1.0 - (sqrt(-slew) * 0.5);
    inputSample -= inputSample * fabs(inputSample) * slew * gainscaling;
    if (inputSample > 0.52) inputSample = 0.52;
    if (inputSample < -0.52) inputSample = -0.52;
    inputSample *= 1.923076923076923;
    return FloatDither(inputSample, fpd);
}

// ---------------------------------------------------------------- Channel9 (Channel9Proc.cpp + Channel9.cpp setParameter)
void Channel9::reset(double sr, uint32_t seed)
{
    sampleRate = sr;
    overallscale = sr / 44100.0;
    for (int x = 0; x < 15; x++) { bqA[x] = 0.0; bqB[x] = 0.0; }
    iirA = iirB = 0.0;
    lastA = lastB = lastC = 0.0;
    fpd = Seed(seed);
    flip = false;
    prepare(kNeve, 0.0, 1.0);
}

void Channel9::prepare(int type, double B, double Cout)
{
    double iirAmount = 0.005832, threshold = 0.33362176, cutoff = 28811.0;   // orig: setParameter, (VstInt32)(A * 4.999)
    switch (type) {
    case kAPI:    iirAmount = 0.004096; threshold = 0.59969536; cutoff = 27216.0; break;
    case kSSL:    iirAmount = 0.004913; threshold = 0.84934656; cutoff = 23011.0; break;
    case kTeac:   iirAmount = 0.009216; threshold = 0.149;      cutoff = 18544.0; break;
    case kMackie: iirAmount = 0.011449; threshold = 0.092;      cutoff = 19748.0; break;
    default: break;                                                           // kNeve
    }
    localiirAmount = iirAmount / overallscale;
    localthreshold = threshold;
    density = B * 2.0;
    phattity = density - 1.0;
    if (density > 1.0) density = 1.0;
    if (phattity < 0.0) phattity = 0.0;
    nonLin = 5.0 - density;
    C = Cout;

    bqB[0] = bqA[0] = cutoff / sampleRate;
    bqA[1] = 1.618033988749894848204586;
    bqB[1] = 0.618033988749894848204586;
    double K = tan(kPi * bqA[0]);                                             // orig: lowpass
    double norm = 1.0 / (1.0 + K / bqA[1] + K * K);
    bqA[2] = K * K * norm;
    bqA[3] = 2.0 * bqA[2];
    bqA[4] = bqA[2];
    bqA[5] = 2.0 * (K * K - 1.0) * norm;
    bqA[6] = (1.0 - K / bqA[1] + K * K) * norm;
    K = tan(kPi * bqA[0]);
    norm = 1.0 / (1.0 + K / bqB[1] + K * K);
    bqB[2] = K * K * norm;
    bqB[3] = 2.0 * bqB[2];
    bqB[4] = bqB[2];
    bqB[5] = 2.0 * (K * K - 1.0) * norm;
    bqB[6] = (1.0 - K / bqB[1] + K * K) * norm;
}

double Channel9::process(double inputSample)
{
    if (fabs(inputSample) < 1.18e-23) inputSample = fpd * 1.18e-17;
    double tempSample;
    if (bqA[0] < 0.49999) {                                                   // orig: DF1 left
        tempSample = bqA[2]*inputSample + bqA[3]*bqA[7] + bqA[4]*bqA[8] - bqA[5]*bqA[9] - bqA[6]*bqA[10];
        bqA[8] = bqA[7]; bqA[7] = inputSample; if (fabs(tempSample) < 1.18e-37) tempSample = 0.0; inputSample = tempSample;
        bqA[10] = bqA[9]; bqA[9] = inputSample;
    }
    const double dielectricScale = fabs(2.0 - ((inputSample + nonLin) / nonLin));
    if (flip) {
        if (fabs(iirA) < 1.18e-37) iirA = 0.0;
        iirA = (iirA * (1.0 - (localiirAmount * dielectricScale))) + (inputSample * localiirAmount * dielectricScale);
        inputSample = inputSample - iirA;
    } else {
        if (fabs(iirB) < 1.18e-37) iirB = 0.0;
        iirB = (iirB * (1.0 - (localiirAmount * dielectricScale))) + (inputSample * localiirAmount * dielectricScale);
        inputSample = inputSample - iirB;
    }                                                                         // orig: highpass section
    const double drySample = inputSample;
    if (inputSample > 1.0) inputSample = 1.0;
    if (inputSample < -1.0) inputSample = -1.0;
    const double phatSample = sin(inputSample * 1.57079633);
    inputSample *= 1.2533141373155;
    const double distSample = sin(inputSample * fabs(inputSample)) / ((fabs(inputSample) == 0.0) ? 1 : fabs(inputSample));
    inputSample = distSample;                                                 // orig: purest form is full Spiral
    if (density < 1.0) inputSample = (drySample * (1 - density)) + (distSample * density);
    if (phattity > 0.0) inputSample = (inputSample * (1 - phattity)) + (phatSample * phattity);

    double clamp = (lastB - lastC) * 0.381966011250105;
    clamp -= (lastA - lastB) * 0.6180339887498948482045;
    clamp += inputSample - lastA;                                             // orig: regular slew clamping added
    lastC = lastB;
    lastB = lastA;
    lastA = inputSample;
    if (clamp > localthreshold) inputSample = lastB + localthreshold;
    if (-clamp > localthreshold) inputSample = lastB - localthreshold;
    lastA = (lastA * 0.381966011250105) + (inputSample * 0.6180339887498948482045);
    flip = !flip;
    if (C < 1.0) inputSample *= C;
    if (bqB[0] < 0.49999) {
        tempSample = bqB[2]*inputSample + bqB[3]*bqB[7] + bqB[4]*bqB[8] - bqB[5]*bqB[9] - bqB[6]*bqB[10];
        bqB[8] = bqB[7]; bqB[7] = inputSample; if (fabs(tempSample) < 1.18e-37) tempSample = 0.0; inputSample = tempSample;
        bqB[10] = bqB[9]; bqB[9] = inputSample;
    }
    return FloatDither(inputSample, fpd);
}

// ---------------------------------------------------------------- ToTape9 (ToTape9Proc.cpp, processReplacing, L path)
void ToTape9::reset(double sr, uint32_t seed)
{
    *this = ToTape9();                                                        // orig constructor values are the member defaults
    sampleRate = sr;
    overallscale = sr / 44100.0;
    fpd = fpdR = Seed(seed);
    prepare(Params());
}

void ToTape9::prepare(const Params &p)
{
    spacing = floor(overallscale);                                            // orig: usually 2 or 4
    if (spacing < 1) spacing = 1; if (spacing > 16) spacing = 16;
    slewsing = floor(overallscale * 2.0);
    if (slewsing < 2) slewsing = 2; if (slewsing > 32) slewsing = 32;
    inputGain = pow(p.A * 2.0, 2.0);
    dublyAmount = p.B * 2.0;
    outlyAmount = (1.0 - p.B) * -2.0;
    if (outlyAmount < -1.0) outlyAmount = -1.0;
    iirEncFreq = (1.0 - p.C) / overallscale;
    iirDecFreq = p.C / overallscale;
    flutDepth = pow(p.D, 6) * overallscale * 50;
    if (flutDepth > 498.0) flutDepth = 498.0;
    flutFrequency = (0.02 * pow(p.E, 3)) / overallscale;
    bias = (p.F * 2.0) - 1.0;
    underBias = (pow(bias, 4) * 0.25) / overallscale;
    double overBias = pow(1.0 - bias, 3) / overallscale;
    if (bias > 0.0) underBias = 0.0;
    if (bias < 0.0) overBias = 1.0 / overallscale;
    static const int th[9] = { threshold9, threshold8, threshold7, threshold6, threshold5,
                               threshold4, threshold3, threshold2, threshold1 };
    for (int i = 0; i < 9; i++) { gslew[th[i]] = overBias; overBias *= 1.618033988749894848204586; }
    headBumpDrive = (p.G * 0.1) / overallscale;
    headBumpMix = p.G * 0.5;
    hdbA[hdb_freq] = (((p.H * p.H) * 175.0) + 25.0) / sampleRate;
    hdbB[hdb_freq] = hdbA[hdb_freq] * 0.9375;
    hdbB[hdb_reso] = hdbA[hdb_reso] = 0.618033988749894848204586;
    hdbB[hdb_a1] = hdbA[hdb_a1] = 0.0;
    double K = tan(kPi * hdbA[hdb_freq]);
    double norm = 1.0 / (1.0 + K / hdbA[hdb_reso] + K * K);
    hdbA[hdb_a0] = K / hdbA[hdb_reso] * norm;
    hdbA[hdb_a2] = -hdbA[hdb_a0];
    hdbA[hdb_b1] = 2.0 * (K * K - 1.0) * norm;
    hdbA[hdb_b2] = (1.0 - K / hdbA[hdb_reso] + K * K) * norm;
    K = tan(kPi * hdbB[hdb_freq]);
    norm = 1.0 / (1.0 + K / hdbB[hdb_reso] + K * K);
    hdbB[hdb_a0] = K / hdbB[hdb_reso] * norm;
    hdbB[hdb_a2] = -hdbB[hdb_a0];
    hdbB[hdb_b1] = 2.0 * (K * K - 1.0) * norm;
    hdbB[hdb_b2] = (1.0 - K / hdbB[hdb_reso] + K * K) * norm;
    outputGain = p.I * 2.0;
    headBumpCube = 0.0618 / sqrt(overallscale);                               // orig computes this per sample; same value
}

double ToTape9::process(double inputSample)
{
    if (fabs(inputSample) < 1.18e-23) inputSample = fpd * 1.18e-17;
    if (inputGain != 1.0) inputSample *= inputGain;

    // orig: Dubly encode
    iirEnc = (iirEnc * (1.0 - iirEncFreq)) + (inputSample * iirEncFreq);
    double highPart = ((inputSample - iirEnc) * 2.848);
    highPart += avgEnc; avgEnc = (inputSample - iirEnc) * 1.152;
    if (highPart > 1.0) highPart = 1.0; if (highPart < -1.0) highPart = -1.0;
    double dubly = fabs(highPart);
    if (dubly > 0.0) {
        double adjust = log(1.0 + (255.0 * dubly)) / 2.40823996531;
        if (adjust > 0.0) dubly /= adjust;
        compEnc = (compEnc * (1.0 - iirEncFreq)) + (dubly * iirEncFreq);
        inputSample += ((highPart * compEnc) * dublyAmount);
    }

    // orig: begin Flutter. L compares against the R flutter state, which is kept here as a shadow (see the header).
    if (flutDepth > 0.0) {
        if (gcount < 0 || gcount > 999) gcount = 999;
        d[gcount] = inputSample;
        int count = gcount;
        double offset = flutDepth + (flutDepth * sin(sweep));
        sweep += nextmax * flutFrequency;
        if (sweep > (kPi * 2.0)) {
            sweep -= kPi * 2.0;
            double flutA = 0.24 + (fpd / (double)UINT32_MAX * 0.74);
            fpd ^= fpd << 13; fpd ^= fpd >> 17; fpd ^= fpd << 5;
            double flutB = 0.24 + (fpd / (double)UINT32_MAX * 0.74);
            if (fabs(flutA - sin(sweepR + nextmaxR)) < fabs(flutB - sin(sweepR + nextmaxR))) nextmax = flutA; else nextmax = flutB;
        }
        count += (int)floor(offset);
        inputSample = (d[count - ((count > 999) ? 1000 : 0)] * (1 - (offset - floor(offset))));
        inputSample += (d[count + 1 - ((count + 1 > 999) ? 1000 : 0)] * (offset - floor(offset)));
        // orig: the R channel's flutter-state update (its audio part is not needed)
        sweepR += nextmaxR * flutFrequency;
        if (sweepR > (kPi * 2.0)) {
            sweepR -= kPi * 2.0;
            double flutA = 0.24 + (fpdR / (double)UINT32_MAX * 0.74);
            fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;
            double flutB = 0.24 + (fpdR / (double)UINT32_MAX * 0.74);
            if (fabs(flutA - sin(sweep + nextmax)) < fabs(flutB - sin(sweep + nextmax))) nextmaxR = flutA; else nextmaxR = flutB;
        }
        gcount--;
    }

    // orig: start bias routine (gslew is the original's interleaved array; the R slots are unused here)
    if (fabs(bias) > 0.001) {
        for (int x = 0; x < gslew_total; x += 3) {
            if (underBias > 0.0) {
                double stuck = fabs(inputSample - (gslew[x] / 0.975)) / underBias;
                if (stuck < 1.0) inputSample = (inputSample * stuck) + ((gslew[x] / 0.975) * (1.0 - stuck));
            }
            if ((inputSample - gslew[x]) > gslew[x + 2]) inputSample = gslew[x] + gslew[x + 2];
            if (-(inputSample - gslew[x]) > gslew[x + 2]) inputSample = gslew[x] - gslew[x + 2];
            gslew[x] = inputSample * 0.975;
        }
    }

    // orig: begin tiny hysteresis
    double applyHysteresis = (1.0 - fabs(inputSample)) * (1.0 - fabs(inputSample)) * 0.012;
    hysteresis = fmax(fmin(hysteresis + ((inputSample * fabs(inputSample))), 0.011449), -0.011449) * 0.999;
    inputSample += (hysteresis * applyHysteresis);

    // orig: begin TapeHack2
    double darkSample = inputSample;
    if (avgPos > 31) avgPos = 0;
    if (slewsing > 31) {
        avg32[avgPos] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 32; x++) darkSample += avg32[x];
        darkSample /= 32.0;
    } if (slewsing > 15) {
        avg16[avgPos % 16] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 16; x++) darkSample += avg16[x];
        darkSample /= 16.0;
    } if (slewsing > 7) {
        avg8[avgPos % 8] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 8; x++) darkSample += avg8[x];
        darkSample /= 8.0;
    } if (slewsing > 3) {
        avg4[avgPos % 4] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 4; x++) darkSample += avg4[x];
        darkSample /= 4.0;
    } if (slewsing > 1) {
        avg2[avgPos % 2] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 2; x++) darkSample += avg2[x];
        darkSample /= 2.0;
    }                                                                         // orig: avgPos updated after the post stage
    double avgSlew = fmin(fabs(lastDark - inputSample) * 0.12 * overallscale, 1.0);
    avgSlew = 1.0 - (1.0 - avgSlew * 1.0 - avgSlew);
    inputSample = (inputSample * (1.0 - avgSlew)) + (darkSample * avgSlew);
    lastDark = darkSample;

    // orig: begin TapeHack (degenerate Taylor series for sin())
    inputSample = fmax(fmin(inputSample, 2.305929007734908), -2.305929007734908);
    double addtwo = inputSample * inputSample;
    double empower = inputSample * addtwo;
    inputSample -= (empower / 6.0);
    empower *= addtwo;
    inputSample += (empower / 69.0);
    empower *= addtwo;
    inputSample -= (empower / 2530.08);
    empower *= addtwo;
    inputSample += (empower / 224985.6);
    empower *= addtwo;
    inputSample -= (empower / 9979200.0f);

    darkSample = inputSample;
    if (avgPos > 31) avgPos = 0;
    if (slewsing > 31) {
        post32[avgPos] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 32; x++) darkSample += post32[x];
        darkSample /= 32.0;
    } if (slewsing > 15) {
        post16[avgPos % 16] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 16; x++) darkSample += post16[x];
        darkSample /= 16.0;
    } if (slewsing > 7) {
        post8[avgPos % 8] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 8; x++) darkSample += post8[x];
        darkSample /= 8.0;
    } if (slewsing > 3) {
        post4[avgPos % 4] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 4; x++) darkSample += post4[x];
        darkSample /= 4.0;
    } if (slewsing > 1) {
        post2[avgPos % 2] = darkSample; darkSample = 0.0;
        for (int x = 0; x < 2; x++) darkSample += post2[x];
        darkSample /= 2.0;
    } avgPos++;
    inputSample = (inputSample * (1.0 - avgSlew)) + (darkSample * avgSlew);   // orig: the previously calculated depth

    // orig: begin HeadBump
    double headBumpSample = 0.0;
    if (headBumpMix > 0.0) {
        headBump += (inputSample * headBumpDrive);
        headBump -= (headBump * headBump * headBump * headBumpCube);
        double headBiqSample = (headBump * hdbA[hdb_a0]) + hdbA[hdb_s1];
        hdbA[hdb_s1] = (headBump * hdbA[hdb_a1]) - (headBiqSample * hdbA[hdb_b1]) + hdbA[hdb_s2];
        hdbA[hdb_s2] = (headBump * hdbA[hdb_a2]) - (headBiqSample * hdbA[hdb_b2]);
        headBumpSample = (headBiqSample * hdbB[hdb_a0]) + hdbB[hdb_s1];
        hdbB[hdb_s1] = (headBiqSample * hdbB[hdb_a1]) - (headBumpSample * hdbB[hdb_b1]) + hdbB[hdb_s2];
        hdbB[hdb_s2] = (headBiqSample * hdbB[hdb_a2]) - (headBumpSample * hdbB[hdb_b2]);
    }
    inputSample += (headBumpSample * headBumpMix);

    // orig: Dubly decode
    iirDec = (iirDec * (1.0 - iirDecFreq)) + (inputSample * iirDecFreq);
    highPart = ((inputSample - iirDec) * 2.628);
    highPart += avgDec; avgDec = (inputSample - iirDec) * 1.372;
    if (highPart > 1.0) highPart = 1.0; if (highPart < -1.0) highPart = -1.0;
    dubly = fabs(highPart);
    if (dubly > 0.0) {
        double adjust = log(1.0 + (255.0 * dubly)) / 2.40823996531;
        if (adjust > 0.0) dubly /= adjust;
        compDec = (compDec * (1.0 - iirDecFreq)) + (dubly * iirDecFreq);
        inputSample += ((highPart * compDec) * outlyAmount);
    }

    if (outputGain != 1.0) inputSample *= outputGain;

    // orig: begin ClipOnly3
    double noise = 1.0 - ((double(fpd) / UINT32_MAX) * 0.076);
    if (wasPosClip == true) {
        if (inputSample < lastSample) lastSample = (0.9085097 * noise) + (inputSample * (1.0 - noise));
        else lastSample = 0.94;
    } wasPosClip = false;
    if (inputSample > 0.9085097) { wasPosClip = true; inputSample = (0.9085097 * noise) + (lastSample * (1.0 - noise)); }
    if (wasNegClip == true) {
        if (inputSample > lastSample) lastSample = (-0.9085097 * noise) + (inputSample * (1.0 - noise));
        else lastSample = -0.94;
    } wasNegClip = false;
    if (inputSample < -0.9085097) { wasNegClip = true; inputSample = (-0.9085097 * noise) + (lastSample * (1.0 - noise)); }
    slew[spacing * 2] = fabs(lastSample - inputSample);
    for (int x = spacing * 2; x > 0; x--) slew[x - 1] = slew[x];
    intermediate[spacing] = inputSample; inputSample = lastSample;            // orig: latency = one 44.1k sample
    for (int x = spacing; x > 0; x--) intermediate[x - 1] = intermediate[x];
    lastSample = intermediate[0];
    if (wasPosClip || wasNegClip) {
        for (int x = spacing; x > 0; x--) lastSample += intermediate[x];
        lastSample /= spacing;
    }
    double finalSlew = 0.0;
    for (int x = spacing * 2; x >= 0; x--) if (finalSlew < slew[x]) finalSlew = slew[x];
    double postclip = 0.94 / (1.0 + (finalSlew * 1.3986013));
    if (inputSample > postclip) inputSample = postclip; if (inputSample < -postclip) inputSample = -postclip;
    fpdR ^= fpdR << 13; fpdR ^= fpdR >> 17; fpdR ^= fpdR << 5;              // orig: R's dither advances fpdR once a sample
    return FloatDither(inputSample, fpd);
}

} // namespace glue
