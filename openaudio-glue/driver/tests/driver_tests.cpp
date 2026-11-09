// OpenAudio Glue driver - host tests of the device's IO path against a simulated Glue tap (make -C driver test). No kext, no
// Core Audio daemon: a page and an AM824 ring in memory, laid out as an FW1814 in ADAT mode (16 inputs + 1 MIDI quadlet), and a fake
// host for storage and notifications.
#include "GlueDevice.h"
#include <mach/mach_time.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace gluedrv;

static int gFailures = 0;
static void check(bool cond, const char *msg) { if (!cond) { gFailures++; printf("FAIL: %s\n", msg); } }

// ---- fake host ----
static CFMutableDictionaryRef gStore;
static int gChanged = 0;
static OSStatus HPropertiesChanged(AudioServerPlugInHostRef, AudioObjectID, UInt32, const AudioObjectPropertyAddress *) { gChanged++; return noErr; }
static OSStatus HCopy(AudioServerPlugInHostRef, CFStringRef key, CFPropertyListRef *out)
{
    CFTypeRef v = CFDictionaryGetValue(gStore, key);
    *out = v ? CFRetain(v) : nullptr;
    return noErr;
}
static OSStatus HWrite(AudioServerPlugInHostRef, CFStringRef key, CFPropertyListRef v) { CFDictionarySetValue(gStore, key, v); return noErr; }
static OSStatus HDelete(AudioServerPlugInHostRef, CFStringRef key) { CFDictionaryRemoveValue(gStore, key); return noErr; }
static OSStatus HRequest(AudioServerPlugInHostRef, AudioObjectID, UInt64, void *) { return noErr; }
static AudioServerPlugInHostInterface gHostIf = { HPropertiesChanged, HCopy, HWrite, HDelete, HRequest };

// ---- the simulated unit: FW1814, ADAT mode, 48 kHz ----
static const uint32_t kCh = 16, kFQ = 17;                  // 16 audio quadlets + 1 MIDI per wire frame
struct Sim {
    GlueTapPage page;
    std::vector<uint32_t> ring;
    Sim()
    {
        memset(&page, 0, sizeof page);
        page.magic = kGlueTapMagic; page.version = kGlueTapVersion; page.generation = 3; page.state = kGlueTapRunning;
        page.rateHz = 48000; page.inChannels = kCh; page.wireQuadlets = kCh; page.frameQuads = kFQ;
        page.ringFrames = kGlueTapRingFrames; page.ringBytes = kFQ * 4 * kGlueTapRingFrames;
        // FW1814Device::GetGlueInputMap (kext): host 0 <- 0, 1 <- 4, 2..7 <- 1, 5, 2, 6, 3, 7 delayed 16, 8.. <- same slot
        static const uint16_t wire[8] = { 0, 4, 1, 5, 2, 6, 3, 7 }, delay[8] = { 0, 0, 16, 16, 16, 16, 16, 16 };
        for (uint32_t c = 0; c < kGlueTapMaxChannels; c++) { page.map[c].slot = (uint16_t)c; page.map[c].delay = 0; }
        for (uint32_t c = 0; c < 8; c++) { page.map[c].slot = wire[c]; page.map[c].delay = delay[c]; }
        ring.resize((size_t)kFQ * kGlueTapRingFrames);
        uint32_t r = 0x9e3779b9u;
        for (auto &q : ring) { r ^= r << 13; r ^= r >> 17; r ^= r << 5; q = (r & 0xffffff00u) | 0x40u; }   // label byte 0 = 0x40
        page.wrapCount = 5;
        page.wrapHostTime = mach_absolute_time();
        page.writeOff = 1600 * kFQ * 4;                    // frames up to 1599 of this lap have arrived. The kext's rule accepts a
                                                           // block only if it ends within 2048 frames behind this position.
    }
};

static UnitInfo Info()
{
    UnitInfo i;
    i.guid = 0xD6C040012337FULL; i.guidHex = "0xD6C040012337F"; i.name = "FireWire 1814 Glue";
    i.clockDomain = 210538784; i.inputLatency = 113; i.inputSafetyOffset = 48;
    return i;
}

static inline int32_t Unpack(uint32_t x) { return (int32_t)(((x & 0xff00u) << 16) | (x & 0xff0000u) | ((x >> 24) << 8)) >> 8; }

// The kext's own conversion for frames [0, frames): com_openaudio_FW1814Device::WorkaroundFor1814InputDelay (history of 16 frames,
// zero-initialised) followed by FWInt24ScaledToFloat32. Copied from the kext source, frame by frame.
static std::vector<float> KextReference(const Sim &s, uint32_t frames)
{
    std::vector<float> out((size_t)frames * kCh);
    uint32_t hist[16 * 6] = {}; uint32_t counter = 0;
    for (uint32_t f = 0; f < frames; f++) {
        const uint32_t *src = &s.ring[(size_t)f * kFQ];
        uint32_t *h = &hist[((++counter) & 0xf) * 6];
        const uint32_t o0 = h[0], o1 = h[1], o2 = h[2], o3 = h[3], o4 = h[4], o5 = h[5];
        h[0] = src[1]; h[1] = src[2]; h[2] = src[3]; h[3] = src[5]; h[4] = src[6]; h[5] = src[7];
        int32_t d[16];
        d[0] = Unpack(src[0]); d[1] = Unpack(src[4]);
        d[2] = Unpack(o0); d[3] = Unpack(o3); d[4] = Unpack(o1); d[5] = Unpack(o4); d[6] = Unpack(o2); d[7] = Unpack(o5);
        for (int i = 8; i < 16; i++) d[i] = Unpack(src[i]);
        for (uint32_t c = 0; c < kCh; c++) out[(size_t)f * kCh + c] = (float)d[c] * 1.1920928955078125e-07f;
    }
    return out;
}

static Device *NewDevice(Sim &s, Workers *w, AudioObjectID id)
{
    Device *d = new Device(id, IO_OBJECT_NULL, &gHostIf, w);
    TapClient *t = new TapClient();
    t->attachForTest(&s.page, s.ring.data(), (uint32_t)(s.ring.size() * 4));
    d->attachForTest(t, Info());
    return d;
}

static void SetSettings(Device *d, CFDictionaryRef plist)
{
    AudioObjectPropertyAddress a = { kGluePropertySettings, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    CFPropertyListRef p = plist;
    check(d->set(d->deviceID(), a, 0, nullptr, sizeof p, &p) == noErr, "settings: set accepted");
}

static CFDictionaryRef AllFull()
{
    UnitSettings s;
    for (int k = 0; k < glue::kStageCount; k++) s.global.level[k] = (uint8_t)(glue::StageOptionCount(k) - 1);
    for (uint32_t c = 0; c < kGlueTapMaxChannels; c++) { s.channel[c].thresholdDb = -20; s.channel[c].source = (uint8_t)(c % glue::kSourceCount); s.channel[c].band[1].gainDb = 4; }
    s.channel[3].on = false;
    return SettingsToPlist(s, kGlueTapMaxChannels);
}

int main()
{
    gStore = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    Workers none; none.start(0);
    Workers three; three.start(3);
    Sim sim;

    // 1. Timeline and bit-exact bypass against the kext's conversion.
    Device *d = NewDevice(sim, &none, 16);
    check(d->startIO() == noErr, "io: start");
    Float64 st = 0; UInt64 ht = 0, seed = 0;
    d->zeroTimeStamp(st, ht, seed);
    check(st == 5.0 * kGlueTapRingFrames && ht == sim.page.wrapHostTime, "timeline: zero time stamp = wrap count x 8192 at the wrap's host time");
    const std::vector<float> ref = KextReference(sim, 2048);
    AudioObjectPropertyAddress lat = { kAudioDevicePropertyLatency, kAudioObjectPropertyScopeInput, kAudioObjectPropertyElementMaster };
    UInt32 latency = 0, lsz = 0;
    d->get(d->deviceID(), lat, 0, nullptr, sizeof latency, lsz, &latency);
    const uint32_t L = latency - 113;                      // Glue's fixed delay compensation at this rate
    check(L >= 1 && L <= 8, "latency: the unit's 113 plus the fixed compensation");
    std::vector<float> out(512 * kCh);
    d->readInput(st + 1000 - L, L, out.data());            // prime the compensation delay with the frames before 1000
    d->readInput(st + 1000, 512, out.data());
    bool exact = true;
    for (uint32_t i = 0; i < 512 * kCh; i++) if (memcmp(&out[i], &ref[(size_t)(1000 - L) * kCh + i], 4) != 0) {
        if (exact) printf("first mismatch: frame %u channel %u: got %.9g want %.9g\n", 1000 + i / kCh, i % kCh, out[i], ref[(size_t)1000 * kCh + i]);
        exact = false;
    }
    check(exact, "bypass: bit-identical to the kext's WorkaroundFor1814InputDelay + FWInt24ScaledToFloat32, delayed by the fixed latency");

    // 2. Underrun: frames not written yet come out as silence (FWBaseEngine::ConvertInputSamplesCore rule).
    d->readInput(st + 1590, 512, out.data());
    bool silent = true;
    for (float v : out) if (v != 0.0f) silent = false;
    check(silent, "underrun: a block reaching past the write position is silent");

    // 3. A block across the ring's end is split, and both halves are read.
    sim.page.writeOff = 400 * kFQ * 4;                     // the next lap has reached frame 400
    d->readInput(st + 8000, 512, out.data());
    bool across = true;
    for (uint32_t i = L; i < 512; i++)                     // output i = input frame i - L (the first L continue the previous call)
        for (uint32_t c = 8; c < kCh; c++) {               // channels without delay: wire slot = channel
            const uint32_t f = (8000 + i - L) & 8191;
            const float want = (float)Unpack(sim.ring[(size_t)f * kFQ + c]) * 1.1920928955078125e-07f;
            if (out[(size_t)i * kCh + c] != want) across = false;
        }
    check(across, "wrap: a block from frame 8000 to 319 of the next lap reads both segments");
    sim.page.writeOff = 1600 * kFQ * 4;

    // 4. Settings: set, read back, persisted, and they change the sound; a channel switched off stays bit-exact.
    CFDictionaryRef full = AllFull();
    SetSettings(d, full);
    AudioObjectPropertyAddress a = { kGluePropertySettings, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    CFPropertyListRef back = nullptr; UInt32 sz = 0;
    d->get(d->deviceID(), a, 0, nullptr, sizeof back, sz, &back);
    UnitSettings parsed;
    check(back && SettingsFromPlist(back, parsed) && parsed.global.level[glue::kStageDesk] == glue::kDeskSSLHigh && !parsed.channel[3].on &&
          parsed.channel[2].source == 2, "settings: read back what was set");
    if (back) CFRelease(back);
    check(CFDictionaryGetCount(gStore) == 1, "settings: persisted through the host's storage");
    d->readInput(st + 1000, 512, out.data());
    bool changed = false, offExact = true;
    for (uint32_t i = 0; i < 512; i++) {
        if (out[(size_t)i * kCh + 0] != ref[(size_t)(1000 - L + i) * kCh + 0]) changed = true;
        if (i >= L && memcmp(&out[(size_t)i * kCh + 3], &ref[(size_t)(1000 - L + i) * kCh + 3], 4) != 0) offExact = false;
    }
    check(changed, "settings: a processed channel differs from the input");
    check(offExact, "settings: the channel switched off is bit-exact");

    // 5. Worker threads: the same output as one thread, block after block.
    Sim simB;
    Device *one = NewDevice(simB, &none, 20), *many = NewDevice(simB, &three, 24);
    SetSettings(one, full); SetSettings(many, full);
    one->startIO(); many->startIO();
    Float64 s1, s2; UInt64 h1, h2, z1, z2;
    one->zeroTimeStamp(s1, h1, z1); many->zeroTimeStamp(s2, h2, z2);
    std::vector<float> o1(256 * kCh), o2(256 * kCh);
    bool same = true;
    for (int b = 0; b < 16; b++) {
        simB.page.writeOff = (uint32_t)(((b + 1) * 256 + 60) % kGlueTapRingFrames) * kFQ * 4;   // just past this block, as in real IO
        one->readInput(s1 + b * 256, 256, o1.data());
        many->readInput(s2 + b * 256, 256, o2.data());
        if (memcmp(o1.data(), o2.data(), o1.size() * 4) != 0) same = false;
        bool any = false;
        for (float x : o1) if (x != 0.0f) any = true;
        if (!any) same = false;                            // silence on both sides would match trivially
    }
    check(same, "workers: 3 worker threads give the same output as 1 thread over 16 blocks");

    // 6. Not locked yet: silence on the host clock, then a new seed when the real timeline appears.
    Sim simC;
    simC.page.wrapCount = 0;
    Device *e = NewDevice(simC, &none, 28);
    e->startIO();
    Float64 sa; UInt64 ha, za, zb;
    e->zeroTimeStamp(sa, ha, za);
    e->readInput(sa, 256, out.data());
    bool quiet = true;
    for (uint32_t i = 0; i < 256 * kCh; i++) if (out[i] != 0.0f) quiet = false;
    check(quiet, "locking: silence before the first wrap time stamp");
    simC.page.wrapCount = 3;
    e->zeroTimeStamp(sa, ha, zb);
    check(zb != za && sa == 3.0 * kGlueTapRingFrames, "locking: the real timeline comes with a new seed");

    // 6b. Stereo link through the device: channel 2 follows channel 1 (same settings, same gain reduction).
    {
        Sim simL;
        Device *lk = NewDevice(simL, &three, 32);
        UnitSettings ls;
        ls.channel[0].thresholdDb = -30; ls.channel[0].linkNext = true; ls.channel[0].band[2].gainDb = 5;
        ls.channel[1].thresholdDb = 0;                     // ignored: the follower takes the leader's settings
        CFDictionaryRef lp = SettingsToPlist(ls, kGlueTapMaxChannels);
        SetSettings(lk, lp);
        CFRelease(lp);
        lk->startIO();
        Float64 ls0; UInt64 lh, lz;
        lk->zeroTimeStamp(ls0, lh, lz);
        std::vector<float> lo(256 * kCh);
        for (int b = 0; b < 8; b++) {
            simL.page.writeOff = (uint32_t)(((b + 1) * 256 + 60) % kGlueTapRingFrames) * kFQ * 4;
            lk->readInput(ls0 + b * 256, 256, lo.data());
        }
        AudioObjectPropertyAddress ma = { kGluePropertyMeters, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        CFPropertyListRef mp = nullptr; UInt32 msz = 0;
        lk->get(lk->deviceID(), ma, 0, nullptr, sizeof mp, msz, &mp);
        bool sameGr = false;
        if (mp) {
            CFArrayRef gr = (CFArrayRef)CFDictionaryGetValue((CFDictionaryRef)mp, CFSTR("gr"));
            float g0 = 0, g1 = 1;
            CFNumberGetValue((CFNumberRef)CFArrayGetValueAtIndex(gr, 0), kCFNumberFloatType, &g0);
            CFNumberGetValue((CFNumberRef)CFArrayGetValueAtIndex(gr, 1), kCFNumberFloatType, &g1);
            sameGr = g0 < 0 && g1 < 0;                     // both compress (the follower got the leader's threshold)
            CFRelease(mp);
        }
        check(sameGr, "link: the follower compresses with the leader's settings");
        delete lk;
    }

    // 6c. The global colour stages are in every channel's path: stages only (compressor off, EQ flat), each stage alone and all four.
    {
        const char *what[5] = { "Transformer", "Tube", "Desk", "Tape", "all four" };
        for (int k = 0; k < 5; k++) {
            Sim simS;
            for (auto &q : simS.ring) q = (q & 0x00ffff00u) | 0x40u;          // quieter: top byte 0 -> about -48 dBFS at most per sample
            for (size_t i = 0; i < simS.ring.size(); i += 3) simS.ring[i] |= 0x08000000u;   // and some samples near -18 dBFS
            Device *sd = NewDevice(simS, &none, 36 + 4 * k);
            UnitSettings ss;
            for (int st = 0; st < glue::kStageCount; st++)
                if (k == 4 || k == st) ss.global.level[st] = (uint8_t)(glue::StageOptionCount(st) - 1);
            CFDictionaryRef sp = SettingsToPlist(ss, kGlueTapMaxChannels);
            SetSettings(sd, sp); CFRelease(sp);
            sd->startIO();
            Float64 s0; UInt64 h0, z0; sd->zeroTimeStamp(s0, h0, z0);
            const std::vector<float> rref = KextReference(simS, 8192);
            std::vector<float> so(256 * kCh);
            int changedChannels = 0;
            bool changed[16] = {};
            for (int b = 0; b < 6; b++) {
                simS.page.writeOff = (uint32_t)(((b + 1) * 256 + 60) % kGlueTapRingFrames) * kFQ * 4;
                sd->readInput(s0 + b * 256, 256, so.data());
                if (b < 2) continue;
                for (uint32_t i = 0; i < 256; i++) for (uint32_t c = 0; c < kCh; c++)
                    if (so[(size_t)i * kCh + c] != rref[(size_t)(b * 256 + i - 1) * kCh + c]) changed[c] = true;
            }
            for (uint32_t c = 0; c < kCh; c++) changedChannels += changed[c];
            char msg[96]; snprintf(msg, sizeof msg, "colour: %s changes every channel (%d of 16)", what[k], changedChannels);
            check(changedChannels == 16, msg);
            delete sd;
        }
    }

    // 7. Properties Core Audio needs.
    AudioObjectPropertyAddress p = { kAudioDevicePropertyLatency, kAudioObjectPropertyScopeInput, kAudioObjectPropertyElementMaster };
    UInt32 v = 0;
    d->get(d->deviceID(), p, 0, nullptr, sizeof v, sz, &v);
    check(v == 113 + L, "properties: input latency = the unit's + the fixed compensation");
    p.mSelector = kAudioDevicePropertySafetyOffset; d->get(d->deviceID(), p, 0, nullptr, sizeof v, sz, &v);
    check(v == 48, "properties: input safety offset = the unit's");
    p.mSelector = kAudioDevicePropertyClockDomain; p.mScope = kAudioObjectPropertyScopeGlobal; d->get(d->deviceID(), p, 0, nullptr, sizeof v, sz, &v);
    check(v == 210538784, "properties: clock domain = the unit's");
    AudioStreamBasicDescription f = {};
    p.mSelector = kAudioStreamPropertyVirtualFormat; d->get(d->streamID(), p, 0, nullptr, sizeof f, sz, &f);
    check(f.mChannelsPerFrame == kCh && f.mSampleRate == 48000 && f.mBitsPerChannel == 32, "properties: 16-channel float32 stream at 48 kHz");
    CFStringRef uid = nullptr;
    p.mSelector = kAudioDevicePropertyDeviceUID; d->get(d->deviceID(), p, 0, nullptr, sizeof uid, sz, &uid);
    char ub[128] = {}; if (uid) { CFStringGetCString(uid, ub, sizeof ub, kCFStringEncodingUTF8); CFRelease(uid); }
    check(strcmp(ub, "com.openaudio.glue:0xD6C040012337F") == 0, "properties: UID com.openaudio.glue:<GUID>");

    CFRelease(full);
    three.stop(); none.stop();
    if (gFailures) { printf("%d FAILURE(S)\n", gFailures); return 1; }
    printf("ALL PASS\n");
    return 0;
}
