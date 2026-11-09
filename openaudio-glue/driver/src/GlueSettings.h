// OpenAudio Glue driver - one unit's settings: what the panel sets through the device's custom property 'glst' (a CFDictionary),
// what is persisted per unit with the host's storage, and how it reaches the IO thread (a seqlock box: no locks on the IO thread).
//
// Plist (version 1):
//   { "version": 1, "bypass": 0|1, "stages": [transformer, tube, desk, tape],              option indices (glue::StageOptionCount)
//     "drive": dB (0...18, the global Drive),
//     "channels": [ { "on": 0|1, "thr": dB, "src": CompSource, "eq": [f1, g1, f2, g2, f3, g3, f4, g4], "link": 0|1 }, ... ] }
// "link" on channel 1, 3, 5 ... (index 0, 2, 4 ...): the next channel follows it (stereo link).
// Missing keys keep their current values, so the panel can send only what changed.
#pragma once
#include <CoreFoundation/CoreFoundation.h>
#include <atomic>
#include <cstring>
#include "Glue.h"
#include "GlueTap.h"

namespace gluedrv {

struct UnitSettings {
    uint32_t version = 0;                                  // bumped by every write
    bool bypass = false;
    glue::GlobalSettings global;
    glue::ChannelSettings channel[kGlueTapMaxChannels];
};

// Merge `plist` into `s`. False if it is not a dictionary.
bool SettingsFromPlist(CFPropertyListRef plist, UnitSettings &s);
// The settings of the first `channels` channels, as a retained CFDictionary.
CFDictionaryRef SettingsToPlist(const UnitSettings &s, uint32_t channels);

// Single writer (serialised by the caller), many readers; a reader that sees a write in progress keeps what it had.
class SettingsBox {
public:
    void write(const UnitSettings &s)
    {
        fSeq.fetch_add(1, std::memory_order_acq_rel);     // odd: writing
        std::atomic_thread_fence(std::memory_order_release);
        memcpy(&fData, &s, sizeof fData);
        std::atomic_thread_fence(std::memory_order_release);
        fSeq.fetch_add(1, std::memory_order_acq_rel);     // even: done
    }
    bool read(UnitSettings &out) const
    {
        const uint32_t a = fSeq.load(std::memory_order_acquire);
        if (a & 1) return false;
        std::atomic_thread_fence(std::memory_order_acquire);
        memcpy(&out, &fData, sizeof out);
        std::atomic_thread_fence(std::memory_order_acquire);
        return fSeq.load(std::memory_order_acquire) == a;
    }
    uint32_t version() const { return fData.version; }    // informational (racy)
private:
    std::atomic<uint32_t> fSeq{ 0 };
    UnitSettings fData;
};

} // namespace gluedrv
