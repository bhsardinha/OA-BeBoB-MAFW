// OpenAudio Glue driver - settings <-> plist (see GlueSettings.h).
#include "GlueSettings.h"

namespace gluedrv {

static bool GetNum(CFDictionaryRef d, CFStringRef key, double &out)
{
    CFTypeRef v = CFDictionaryGetValue(d, key);
    if (!v || CFGetTypeID(v) != CFNumberGetTypeID()) return false;
    return CFNumberGetValue((CFNumberRef)v, kCFNumberDoubleType, &out);
}

static double ArrayNum(CFArrayRef a, CFIndex i, double fallback)
{
    if (i >= CFArrayGetCount(a)) return fallback;
    CFTypeRef v = CFArrayGetValueAtIndex(a, i);
    double x = fallback;
    if (v && CFGetTypeID(v) == CFNumberGetTypeID()) CFNumberGetValue((CFNumberRef)v, kCFNumberDoubleType, &x);
    return x;
}

bool SettingsFromPlist(CFPropertyListRef plist, UnitSettings &s)
{
    if (!plist || CFGetTypeID(plist) != CFDictionaryGetTypeID()) return false;
    CFDictionaryRef d = (CFDictionaryRef)plist;
    double x;
    if (GetNum(d, CFSTR("bypass"), x)) s.bypass = (x != 0.0);
    if (GetNum(d, CFSTR("drive"), x)) s.global.driveDb = (float)(x < 0 ? 0 : (x > 18 ? 18 : x));
    CFTypeRef st = CFDictionaryGetValue(d, CFSTR("stages"));
    if (st && CFGetTypeID(st) == CFArrayGetTypeID()) {
        for (int k = 0; k < glue::kStageCount; k++) {
            const double v = ArrayNum((CFArrayRef)st, k, s.global.level[k]);
            const int o = (int)v;
            s.global.level[k] = (uint8_t)((o >= 0 && o < glue::StageOptionCount(k)) ? o : 0);
        }
    }
    CFTypeRef ch = CFDictionaryGetValue(d, CFSTR("channels"));
    if (ch && CFGetTypeID(ch) == CFArrayGetTypeID()) {
        const CFIndex n = CFArrayGetCount((CFArrayRef)ch);
        for (CFIndex c = 0; c < n && c < (CFIndex)kGlueTapMaxChannels; c++) {
            CFTypeRef e = CFArrayGetValueAtIndex((CFArrayRef)ch, c);
            if (!e || CFGetTypeID(e) != CFDictionaryGetTypeID()) continue;
            glue::ChannelSettings &cs = s.channel[c];
            if (GetNum((CFDictionaryRef)e, CFSTR("on"), x)) cs.on = (x != 0.0);
            if (GetNum((CFDictionaryRef)e, CFSTR("link"), x)) cs.linkNext = (x != 0.0) && (c % 2 == 0);
            if (GetNum((CFDictionaryRef)e, CFSTR("thr"), x)) cs.thresholdDb = (float)(x > 0 ? 0 : x);
            if (GetNum((CFDictionaryRef)e, CFSTR("src"), x)) cs.source = (uint8_t)((x >= 0 && x < glue::kSourceCount) ? x : 0);
            CFTypeRef eq = CFDictionaryGetValue((CFDictionaryRef)e, CFSTR("eq"));
            if (eq && CFGetTypeID(eq) == CFArrayGetTypeID()) {
                for (int b = 0; b < glue::kBandCount; b++) {
                    cs.band[b].freqHz = (float)ArrayNum((CFArrayRef)eq, 2 * b, cs.band[b].freqHz);
                    cs.band[b].gainDb = (float)ArrayNum((CFArrayRef)eq, 2 * b + 1, cs.band[b].gainDb);
                }
            }
        }
    }
    return true;
}

static CFNumberRef N(double v) { return CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &v); }

static void Put(CFMutableDictionaryRef d, CFStringRef k, CFTypeRef v) { CFDictionarySetValue(d, k, v); CFRelease(v); }

CFDictionaryRef SettingsToPlist(const UnitSettings &s, uint32_t channels)
{
    CFMutableDictionaryRef d = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    Put(d, CFSTR("version"), N(1));
    Put(d, CFSTR("bypass"), N(s.bypass ? 1 : 0));
    Put(d, CFSTR("drive"), N(s.global.driveDb));
    CFMutableArrayRef st = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    for (int k = 0; k < glue::kStageCount; k++) { CFNumberRef n = N(s.global.level[k]); CFArrayAppendValue(st, n); CFRelease(n); }
    Put(d, CFSTR("stages"), st);
    CFMutableArrayRef ch = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    for (uint32_t c = 0; c < channels && c < kGlueTapMaxChannels; c++) {
        const glue::ChannelSettings &cs = s.channel[c];
        CFMutableDictionaryRef e = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        Put(e, CFSTR("on"), N(cs.on ? 1 : 0));
        Put(e, CFSTR("link"), N(cs.linkNext ? 1 : 0));
        Put(e, CFSTR("thr"), N(cs.thresholdDb));
        Put(e, CFSTR("src"), N(cs.source));
        CFMutableArrayRef eq = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
        for (int b = 0; b < glue::kBandCount; b++) {
            CFNumberRef f = N(cs.band[b].freqHz), g = N(cs.band[b].gainDb);
            CFArrayAppendValue(eq, f); CFArrayAppendValue(eq, g); CFRelease(f); CFRelease(g);
        }
        Put(e, CFSTR("eq"), eq);
        CFArrayAppendValue(ch, e); CFRelease(e);
    }
    Put(d, CFSTR("channels"), ch);
    return d;
}

} // namespace gluedrv
