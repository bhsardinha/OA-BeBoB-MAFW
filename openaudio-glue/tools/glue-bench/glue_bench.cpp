// OpenAudio Glue - CPU bench and value report for the DSP chain (recipe in ../README.md).
//   glue-bench [channels] [rate] [frames]      defaults: 16 channels, 48000 Hz, 64 frames per block
// Runs 20 s of audio per channel through the full chain (every stage Full, 4 EQ bands active, compressor at -20 dB)
// and prints the share of one core it took, then the calibrated trims and a few measured values.
#include "Glue.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace glue;

int main(int argc, char **argv)
{
    const int channels = argc > 1 ? atoi(argv[1]) : 16;
    const double rate = argc > 2 ? atof(argv[2]) : 48000.0;
    const int frames = argc > 3 ? atoi(argv[3]) : 64;
    const double seconds = 20.0;

    Presets presets;
    auto t0 = std::chrono::steady_clock::now();
    presets.calibrate(rate);
    const double calMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();

    std::vector<ChannelChain> chain(channels);
    GlobalSettings g; for (int s = 0; s < kStageCount; s++) g.level[s] = (uint8_t)(StageOptionCount(s) - 1);
    ChannelSettings ch; ch.thresholdDb = -20;
    ch.band[0].gainDb = 3; ch.band[1].gainDb = -2; ch.band[2].gainDb = 2; ch.band[3].gainDb = 3;
    for (int c = 0; c < channels; c++) { chain[c].reset(&presets, (uint32_t)(c + 1)); chain[c].setGlobal(g); chain[c].setChannel(ch); }

    std::vector<float> buf((size_t)channels * frames);
    const long blocks = (long)(seconds * rate / frames);
    double phase = 0.0, peak = 0.0;
    t0 = std::chrono::steady_clock::now();
    for (long b = 0; b < blocks; b++) {
        for (int i = 0; i < frames; i++) {
            const float v = (float)(0.25 * sin(phase)); phase += 2 * 3.14159265358979323846 * 220.0 / rate;
            for (int c = 0; c < channels; c++) buf[(size_t)c * frames + i] = v;
        }
        for (int c = 0; c < channels; c++) chain[c].process(&buf[(size_t)c * frames], &buf[(size_t)c * frames], frames);
        for (int i = 0; i < frames; i++) if (fabs(buf[i]) > peak) peak = fabs(buf[i]);
    }
    const double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    printf("glue-bench: %d channels, %.0f Hz, %d frames: %.3f s for %.0f s of audio = %.1f %% of one core "
           "(%.2f %% per channel)\n", channels, rate, frames, s, seconds, 100.0 * s / seconds, 100.0 * s / seconds / channels);
    printf("calibration: %.0f ms\n", calMs);
    printf("trims (dB, after blend) and wet norms (dB):\n");
    for (int st = 0; st < kStageCount; st++) {
        printf("  %-11s", StageName(st));
        for (int l = 1; l < StageOptionCount(st); l++)
            printf("  %s %+6.2f / %+6.2f", StageOptionName(st, l), 20 * log10(presets.trim(st, l)), 20 * log10(presets.wetNorm(st, l)));
        printf("\n");
    }
    printf("channel 0: gain reduction %.2f dB, output peak %.3f (input peak 0.25 = -12 dBFS)\n", chain[0].gainReductionDb(), peak);
    return 0;
}
