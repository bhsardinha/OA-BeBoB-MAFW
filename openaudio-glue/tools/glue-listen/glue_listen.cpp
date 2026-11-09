// OpenAudio Glue - glue-listen: plays an audio file in a loop through the Glue chain to an output device, so the presets can
// be heard and A/B'd before the driver exists (recipe in ../README.md). An ordinary Core Audio client: it does not touch the
// kext, and it never changes the device's rate (the file is converted to the device's current rate).
//
//   glue-listen <file> [--device <name part>] [--out <first output, 1-based>] [--tour]
//
// Keys:  t / u / p       Transformer / Tube / Tape: next level (Off, Soft, Medium, High, Full)
//        d               Desk: next option (Off, Neve Medium, Neve High, API Medium, API High, SSL Medium, SSL High)
//        a               Transformer, Tube and Tape to the next level together (Desk keeps its option)
//        space           A/B: Glue <-> dry (the chain keeps running, so the switch is instant)
//        , / .           compressor threshold -2 / +2 dB (0 dB = off)   (keys chosen for a Spanish keyboard, user 2026-10-28)
//        s               compressor source: General, Drums (keeps transients), Vocals, Heavy guitar, Bass, Synth
//        e               next EQ curve (flat, smile, low-mid cut, presence, low-end cut, scoop, punch, air)
//        ' / ¡           output level -1 / +1 dB (both A and B); ¡ arrives as UTF-8 C2 A1, the A1 byte is taken
//        q               quit
// --tour runs without keys, 8 s per step: dry; Transformer + Tube + Tape at Soft, Medium, High, Full (Desk Off); then
// Desk alone through its six options.
#include "Glue.h"
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

using namespace glue;

struct Shared {                                   // written by the key loop, read by the render callback
    std::atomic<int> level[kStageCount];
    std::atomic<int> thresholdDb{ 0 };
    std::atomic<int> eqCurve{ 0 };
    std::atomic<int> source{ 0 };
    std::atomic<int> outDb{ -6 };
    std::atomic<bool> dry{ false };
    std::atomic<float> grDb{ 0 }, hitDb{ 0 }, peakDb{ -120 };
};

static Shared gS;
static std::vector<float> gL, gR;                 // the whole file at the device rate
static size_t gPos = 0;
static Presets gPresets;
static ChannelChain gChain[2];
static int gApplied[kStageCount + 3] = { -1, -1, -1, -1, -999, -1, -1 };

// Listening curves for `e` (v2, user 2026-10-28: "more aggressive, and one with cuts in the low end, around 5 dB in each
// frequency; another 4 presets"). Gains within the EQ's +-12 dB; the mids narrow as they grow (proportional Q).
static const int kEQCurveCount = 8;
static const char *kEQNames[kEQCurveCount] = { "flat", "smile", "low-mid cut", "presence", "low-end cut", "scoop", "punch", "air" };
static ChannelSettings EQCurve(int curve, int threshold, int source)
{
    ChannelSettings c;
    c.thresholdDb = (float)threshold;
    c.source = (uint8_t)source;
    switch (curve) {
    case 1: c.band[kBandLowShelf] = { 80, 6 }; c.band[kBandHighShelf] = { 10000, 5 }; break;                    // smile
    case 2: c.band[kBandLowMid] = { 400, -6 }; break;                                                             // low-mid cut
    case 3: c.band[kBandHighMid] = { 3000, 6 }; break;                                                            // presence
    case 4: c.band[kBandLowShelf] = { 100, -5 }; c.band[kBandLowMid] = { 250, -5 }; break;                      // low-end cut
    case 5: c.band[kBandLowShelf] = { 100, 4 }; c.band[kBandLowMid] = { 600, -6 }; c.band[kBandHighShelf] = { 8000, 4 }; break;   // scoop
    case 6: c.band[kBandLowShelf] = { 60, 5 }; c.band[kBandLowMid] = { 350, -4 }; c.band[kBandHighMid] = { 4000, 4 }; break;      // punch
    case 7: c.band[kBandLowMid] = { 200, 3 }; c.band[kBandHighShelf] = { 12000, 6 }; break;                     // air
    default: break;
    }
    return c;
}

static OSStatus Render(void *, AudioUnitRenderActionFlags *, const AudioTimeStamp *, UInt32, UInt32 frames, AudioBufferList *io)
{
    bool changed = false;
    for (int s = 0; s < kStageCount; s++) if (gS.level[s].load() != gApplied[s]) changed = true;
    const int th = gS.thresholdDb.load(), eq = gS.eqCurve.load(), src = gS.source.load();
    if (changed) {
        GlobalSettings g;
        for (int s = 0; s < kStageCount; s++) { gApplied[s] = gS.level[s].load(); g.level[s] = (uint8_t)gApplied[s]; }
        gChain[0].setGlobal(g); gChain[1].setGlobal(g);
    }
    if (th != gApplied[kStageCount] || eq != gApplied[kStageCount + 1] || src != gApplied[kStageCount + 2]) {
        gApplied[kStageCount] = th; gApplied[kStageCount + 1] = eq; gApplied[kStageCount + 2] = src;
        const ChannelSettings c = EQCurve(eq, th, src);
        gChain[0].setChannel(c); gChain[1].setChannel(c);
    }
    float *outL = (float *)io->mBuffers[0].mData, *outR = (float *)io->mBuffers[1].mData;
    float inL[4096], inR[4096];
    UInt32 done = 0;
    float peak = 0;
    const float gain = powf(10.0f, gS.outDb.load() / 20.0f);
    const bool dry = gS.dry.load();
    while (done < frames) {
        const UInt32 k = (frames - done) > 4096 ? 4096 : (frames - done);
        for (UInt32 i = 0; i < k; i++) { inL[i] = gL[gPos]; inR[i] = gR[gPos]; if (++gPos >= gL.size()) gPos = 0; }
        gChain[0].process(inL, outL + done, (int)k);
        gChain[1].process(inR, outR + done, (int)k);
        for (UInt32 i = 0; i < k; i++) {
            float l = dry ? inL[i] : outL[done + i], r = dry ? inR[i] : outR[done + i];
            l *= gain; r *= gain;
            outL[done + i] = l; outR[done + i] = r;
            if (fabsf(l) > peak) peak = fabsf(l);
            if (fabsf(r) > peak) peak = fabsf(r);
        }
        done += k;
    }
    gS.grDb.store(gChain[0].gainReductionDb());
    gS.hitDb.store(gChain[0].hitLiftDb());
    const float pk = 20.0f * log10f(peak + 1e-9f);
    if (pk > gS.peakDb.load()) gS.peakDb.store(pk);
    return noErr;
}

static AudioObjectID FindDevice(const char *part, UInt32 &outChannels)
{
    AudioObjectPropertyAddress a = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    UInt32 size = 0;
    AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &a, 0, nullptr, &size);
    std::vector<AudioObjectID> ids(size / sizeof(AudioObjectID));
    AudioObjectGetPropertyData(kAudioObjectSystemObject, &a, 0, nullptr, &size, ids.data());
    for (AudioObjectID id : ids) {
        CFStringRef name = nullptr; UInt32 sz = sizeof(name);
        AudioObjectPropertyAddress na = { kAudioObjectPropertyName, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        if (AudioObjectGetPropertyData(id, &na, 0, nullptr, &sz, &name) != noErr || !name) continue;
        char buf[256]; CFStringGetCString(name, buf, sizeof buf, kCFStringEncodingUTF8); CFRelease(name);
        if (!strcasestr(buf, part)) continue;
        AudioObjectPropertyAddress ca = { kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };
        UInt32 csz = 0;
        if (AudioObjectGetPropertyDataSize(id, &ca, 0, nullptr, &csz) != noErr || csz == 0) continue;
        std::vector<char> mem(csz);
        AudioBufferList *bl = (AudioBufferList *)mem.data();
        AudioObjectGetPropertyData(id, &ca, 0, nullptr, &csz, bl);
        UInt32 ch = 0;
        for (UInt32 i = 0; i < bl->mNumberBuffers; i++) ch += bl->mBuffers[i].mNumberChannels;
        if (ch == 0) continue;
        printf("output device: %s (%u outputs)\n", buf, ch);
        outChannels = ch;
        return id;
    }
    return kAudioObjectUnknown;
}

static bool LoadFile(const char *path, double rate)
{
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(nullptr, (const UInt8 *)path, (CFIndex)strlen(path), false);
    ExtAudioFileRef f = nullptr;
    OSStatus e = ExtAudioFileOpenURL(url, &f);
    CFRelease(url);
    if (e != noErr) { fprintf(stderr, "cannot open %s (%d)\n", path, (int)e); return false; }
    AudioStreamBasicDescription ff = {}; UInt32 sz = sizeof ff;
    ExtAudioFileGetProperty(f, kExtAudioFileProperty_FileDataFormat, &sz, &ff);
    AudioStreamBasicDescription cf = {};
    cf.mSampleRate = rate; cf.mFormatID = kAudioFormatLinearPCM;
    cf.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
    cf.mChannelsPerFrame = 2; cf.mBitsPerChannel = 32; cf.mBytesPerFrame = 4; cf.mFramesPerPacket = 1; cf.mBytesPerPacket = 4;
    if (ff.mChannelsPerFrame == 1) cf.mChannelsPerFrame = 1;
    ExtAudioFileSetProperty(f, kExtAudioFileProperty_ClientDataFormat, sizeof cf, &cf);
    std::vector<float> a(8192), b(8192);
    for (;;) {
        char mem[sizeof(AudioBufferList) + sizeof(AudioBuffer)];
        AudioBufferList *bl = (AudioBufferList *)mem;
        bl->mNumberBuffers = cf.mChannelsPerFrame;
        bl->mBuffers[0] = { 1, 8192 * 4, a.data() };
        if (cf.mChannelsPerFrame == 2) bl->mBuffers[1] = { 1, 8192 * 4, b.data() };
        UInt32 n = 8192;
        if (ExtAudioFileRead(f, &n, bl) != noErr || n == 0) break;
        gL.insert(gL.end(), a.begin(), a.begin() + n);
        gR.insert(gR.end(), (cf.mChannelsPerFrame == 2 ? b : a).begin(), (cf.mChannelsPerFrame == 2 ? b : a).begin() + n);
    }
    ExtAudioFileDispose(f);
    printf("file: %s, %.1f s at %.0f Hz (%u channel%s in the file)\n", path, gL.size() / rate, rate,
           ff.mChannelsPerFrame, ff.mChannelsPerFrame == 1 ? "" : "s");
    return !gL.empty();
}

static void Status()
{
    printf("\r%s | ", gS.dry.load() ? "DRY " : "GLUE");
    for (int s = 0; s < kStageCount; s++)
        printf("%s %-*s ", StageName(s), s == kStageDesk ? 11 : 6, StageOptionName(s, gS.level[s].load()));
    printf("| %-12s thr %3d gr %5.1f hit %+4.1f | EQ %-11s | out %+d | peak %6.1f   ", CompSourceName(gS.source.load()),
           gS.thresholdDb.load(), gS.grDb.load(), gS.hitDb.load(), kEQNames[gS.eqCurve.load()], gS.outDb.load(), gS.peakDb.load());
    fflush(stdout);
    gS.peakDb.store(-120);
}

int main(int argc, char **argv)
{
    const char *path = nullptr, *dev = "1814";
    int firstOut = 1; bool tour = false;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--device") && i + 1 < argc) dev = argv[++i];
        else if (!strcmp(argv[i], "--out") && i + 1 < argc) firstOut = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--tour")) tour = true;
        else path = argv[i];
    }
    if (!path) { fprintf(stderr, "usage: glue-listen <file> [--device <name part>] [--out <first output>] [--tour]\n"); return 2; }
    for (int s = 0; s < kStageCount; s++) gS.level[s].store(kLevelOff);

    UInt32 devOuts = 0;
    AudioObjectID device = FindDevice(dev, devOuts);
    if (device == kAudioObjectUnknown) { fprintf(stderr, "no output device whose name contains \"%s\"\n", dev); return 1; }
    if (firstOut < 1 || (UInt32)firstOut + 1 > devOuts) { fprintf(stderr, "--out %d does not fit %u outputs\n", firstOut, devOuts); return 1; }
    Float64 rate = 0; UInt32 sz = sizeof rate;
    AudioObjectPropertyAddress ra = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    AudioObjectGetPropertyData(device, &ra, 0, nullptr, &sz, &rate);
    if (!LoadFile(path, rate)) return 1;

    gPresets.calibrate(rate);
    gChain[0].reset(&gPresets, 1); gChain[1].reset(&gPresets, 2);

    AudioComponentDescription d = { kAudioUnitType_Output, kAudioUnitSubType_HALOutput, kAudioUnitManufacturer_Apple, 0, 0 };
    AudioUnit au = nullptr;
    AudioComponentInstanceNew(AudioComponentFindNext(nullptr, &d), &au);
    AudioUnitSetProperty(au, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &device, sizeof device);
    AudioStreamBasicDescription f = {};
    f.mSampleRate = rate; f.mFormatID = kAudioFormatLinearPCM;
    f.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
    f.mChannelsPerFrame = 2; f.mBitsPerChannel = 32; f.mBytesPerFrame = 4; f.mFramesPerPacket = 1; f.mBytesPerPacket = 4;
    AudioUnitSetProperty(au, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &f, sizeof f);
    std::vector<SInt32> map(devOuts, -1);                     // our L/R -> the chosen pair of device outputs
    map[(size_t)firstOut - 1] = 0; map[(size_t)firstOut] = 1;
    AudioUnitSetProperty(au, kAudioOutputUnitProperty_ChannelMap, kAudioUnitScope_Input, 0, map.data(), (UInt32)(map.size() * sizeof(SInt32)));
    AURenderCallbackStruct cb = { Render, nullptr };
    AudioUnitSetProperty(au, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &cb, sizeof cb);
    if (AudioUnitInitialize(au) != noErr || AudioOutputUnitStart(au) != noErr) { fprintf(stderr, "cannot start output\n"); return 1; }
    printf("playing to outputs %d/%d; output level %d dB\n", firstOut, firstOut + 1, gS.outDb.load());

    if (tour) {
        auto step = [](const char *what) { printf("\n%s\n", what); for (int t = 0; t < 16; t++) { usleep(500000); Status(); } };
        step("all stages Off (dry)");
        for (int l = kLevelSoft; l < kLevelCount; l++) {
            for (int s = 0; s < kStageCount; s++) gS.level[s].store(s == kStageDesk ? 0 : l);
            char what[64]; snprintf(what, sizeof what, "Transformer + Tube + Tape: %s", StageOptionName(kStageTape, l));
            step(what);
        }
        for (int o = 1; o < kDeskOptionCount; o++) {
            for (int s = 0; s < kStageCount; s++) gS.level[s].store(s == kStageDesk ? o : 0);
            char what[64]; snprintf(what, sizeof what, "Desk alone: %s", StageOptionName(kStageDesk, o));
            step(what);
        }
    } else {
        printf("keys: t u p = levels, d = desk, a = t+u+p, space = A/B, , . = threshold, s = source, e = EQ, ' \xc2\xa1 = level, q = quit\n");
        termios old, raw; tcgetattr(0, &old); raw = old; raw.c_lflag &= ~(ICANON | ECHO); raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 2;
        tcsetattr(0, TCSANOW, &raw);
        for (;;) {
            char c = 0;
            if (read(0, &c, 1) == 1) {
                if (c == 'q') break;
                const int st = c == 't' ? kStageTransformer : c == 'u' ? kStageTube : c == 'd' ? kStageDesk : c == 'p' ? kStageTape : -1;
                if (st >= 0) gS.level[st].store((gS.level[st].load() + 1) % StageOptionCount(st));
                if (c == 'a') {
                    const int l = (gS.level[kStageTransformer].load() + 1) % kLevelCount;
                    for (int s = 0; s < kStageCount; s++) if (s != kStageDesk) gS.level[s].store(l);
                }
                if (c == ' ') gS.dry.store(!gS.dry.load());
                if (c == ',') gS.thresholdDb.store(std::max(-40, gS.thresholdDb.load() - 2));
                if (c == '.') gS.thresholdDb.store(std::min(0, gS.thresholdDb.load() + 2));
                if (c == 's') gS.source.store((gS.source.load() + 1) % kSourceCount);
                if (c == 'e') gS.eqCurve.store((gS.eqCurve.load() + 1) % kEQCurveCount);
                if (c == '\'') gS.outDb.store(gS.outDb.load() - 1);
                if ((unsigned char)c == 0xA1) gS.outDb.store(std::min(0, gS.outDb.load() + 1));
            }
            Status();
        }
        tcsetattr(0, TCSANOW, &old);
    }
    printf("\n");
    AudioOutputUnitStop(au);
    AudioComponentInstanceDispose(au);
    return 0;
}
