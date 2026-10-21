// FWRegisterWindows -- the BeBoB vendor register windows, common to the M-Audio BeBoB models (base). The mixer window 0xFFC7:0x00700000 is
// written by FWAVCAudioDevice::CustomMixerWriteQuads (ref @0x22c7a, a base-class method); the status / meter window 0xFFC7:0x00600000 is
// read by FWHSCI for every model with control packets (FW1814 0x58 bytes, Audiophile 0x3C; Q-ME §5, Q-AP0 §2.5). What each offset means is
// per model (models/<Model>/<Model>MixerRegs.h).
#ifndef OA_FWREGISTERWINDOWS_H
#define OA_FWREGISTERWINDOWS_H

#define kFWMixerWindowHi   0xFFC7
#define kFWMixerWindowLo   0x00700000u      // + register offset
#define kFWHsciWindowLo    0x00600000u      // status / meter block read by FWHSCI (same 0xFFC7 high word)

#endif
