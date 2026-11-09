// OpenAudio Glue - public interface of the per-channel chain (docs/DSP_v1.md).
// Order: Transformer -> Tube -> EQ -> Compressor -> Desk colour -> Tape. No allocation and no locks in process();
// settings are applied by the caller on the audio thread between blocks.
#pragma once
#include <cstdint>
#include "AirwindowsMono.h"
#include "GlueEQ.h"
#include "GlueComp.h"

namespace glue {

enum Stage { kStageTransformer, kStageTube, kStageDesk, kStageTape, kStageCount };
// Options per stage. Transformer, Tube, Tape: Off / Soft / Medium / High / Full. Desk (user, 2026-10-28): Off, then Medium
// and High for each of Channel9's Neve, API and SSL types.
enum Level { kLevelOff, kLevelSoft, kLevelMedium, kLevelHigh, kLevelFull, kLevelCount };
enum DeskOption { kDeskOff, kDeskNeveMedium, kDeskNeveHigh, kDeskAPIMedium, kDeskAPIHigh, kDeskSSLMedium, kDeskSSLHigh,
                  kDeskOptionCount };
const int kMaxStageOptions = kDeskOptionCount;

const char *StageName(int stage);
int StageOptionCount(int stage);                       // including Off (option 0)
const char *StageOptionName(int stage, int option);

struct GlobalSettings {                       // one per unit; level[s] is an option index of stage s
    uint8_t level[kStageCount] = { kLevelOff, kLevelOff, kLevelOff, kLevelOff };
    float driveDb = 0.0f;                     // Drive (user, 2026-10-28): 0...+18 dB more into every colour stage, level-compensated
};

struct ChannelSettings {                      // one per input channel
    bool on = true;                           // off = exact pass-through, no DSP at all (user, 2026-10-28: saves processing)
    float thresholdDb = 0.0f;                 // 0 = compressor off
    uint8_t source = kSourceGeneral;          // compressor voicing (CompSource)
    bool linkNext = false;                    // on the first channel of a pair (1, 3, 5 ...): the next one follows it (stereo link;
                                              // the driver copies these settings to it and processes the pair with processPair)
    EQBandSetting band[kBandCount] = { { 100.0f, 0.0f }, { 400.0f, 0.0f }, { 2500.0f, 0.0f }, { 8000.0f, 0.0f } };
};

// The preset tables (GluePresets.cpp) and the output trims that keep a -18 dBFS 1 kHz sine at the same level for every
// preset. calibrate() runs each stage offline (about 16 x 0.6 s of audio); call it off the audio thread when the
// sample rate changes, then hand the result to the chains.
class Presets {
public:
    void calibrate(double sampleRate);
    double sampleRate() const { return fRate; }
    double trim(int stage, int level) const { return fTrim[stage][level]; }        // after the dry/wet blend
    double wetNorm(int stage, int level) const { return fWetNorm[stage][level]; }  // wet path alone, before the blend
    // Delay compensation (user, 2026-10-28: "delay compensation in bypassed channels to avoid phase issues"). delay() = a stage's
    // phase delay in samples at 1 kHz, -18 dBFS (measured by calibrate()); chainLatency() = the active stages' sum, rounded to whole
    // samples (negative when the chain leads in phase); maxLatency() = the largest chainLatency() of any combination, at least 0
    // (all stages off is one of them). Every channel is delayed up to maxLatency(), so
    // processed, switched-off and bypassed channels stay aligned and the device's latency is one constant per rate.
    double delay(int stage, int level) const { return fDelay[stage][level]; }
    int chainLatency(const uint8_t level[kStageCount]) const;
    int maxLatency() const { return fMaxLatency; }
private:
    double fRate = 0;
    double fDelay[kStageCount][kMaxStageOptions] = {};
    int fMaxLatency = 0;
    double fTrim[kStageCount][kMaxStageOptions] = {};
    double fWetNorm[kStageCount][kMaxStageOptions] = {};
};

// Values each stage gets for a level (exposed for calibration, tests and docs).
struct StagePreset {
    double drive, wet;                        // transformer and tube: drive into the stage and dry/wet (wet path / drive)
    double tubeA, tubeB;                      // Tube2 Input, Tube
    int deskType; double deskDrive;           // Channel9 type, Drive
    ToTape9::Params tape;                     // ToTape9 A..I
};
StagePreset GetStagePreset(int stage, int level);

class ChannelChain {
public:
    void reset(const Presets *presets, uint32_t seed);    // presets->sampleRate() is the rate; resets all state
    void setGlobal(const GlobalSettings &g);
    void setChannel(const ChannelSettings &c);
    void process(const float *in, float *out, int frames);   // in == out is allowed; output delayed to presets->maxLatency()
    // A stereo-linked pair: every stage per side, the compressor linked (Comp::processLinked). Same result as process() on each
    // side when the pair cannot be linked (one side bypassed or switched off).
    static void processPair(ChannelChain &a, ChannelChain &b, const float *inA, const float *inB, float *outA, float *outB, int frames);
    void setBypassAll(bool on) { fBypassAll = on; }           // the unit's bypass: pass-through, still delay-compensated
    int compensationDelay() const { return fCompDelay; }      // samples added after the chain (0..maxLatency)
    float gainReductionDb() const { return fComp.gainReductionDb(); }
    float hitLiftDb() const { return fComp.hitLiftDb(); }
    bool bypassed() const;                                    // nothing would change the signal
private:
    double runStage(int stage, double x);
    void resetState();                        // every stage, the EQ and the compressor from clean state
    inline double step(double x);
    inline double pre(double x);              // transformer, tube, EQ
    inline double post(double x);             // desk, tape
    void begin(int frames);                   // per block: EQ smoothing and the compensation delay
    void body(const float *in, float *out, int frames);
    const Presets *fPresets = nullptr;
    uint32_t fSeed = 1;
    bool fOn = true;
    bool fBypassAll = false;
    double fDrive = 1.0;                      // GlobalSettings::driveDb as a gain
    uint8_t fLevel[kStageCount] = {};
    static const int kMaxCompDelay = 32;
    float fDelayLine[kMaxCompDelay] = {};                    // compensation delay (a ring of the last kMaxCompDelay samples)
    int fDelayPos = 0, fCompDelay = 0;
    void updateCompDelay();
    inline float delayed(float x);
    StagePreset fPreset[kStageCount] = {};
    Interstage fTransformer;
    Tube2 fTube;
    Channel9 fDesk;
    ToTape9 fTape;
    EQ fEQ;
    Comp fComp;
};

} // namespace glue
