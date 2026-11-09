// GlueTap -- the read-only input tap for OpenAudio Glue (openaudio-glue/docs/GLUE_PLAN_v1.md phase 2, ARCHITECTURE_v1.md; main R-log R143).
// NOT in the reference driver. A user-space AudioServerPlugIn may not use the HAL client API (AudioServerPlugIn.h, 10.13 SDK, line 35;
// Glue G1), so it reads a unit's input samples from here: the vendor user client maps this page (memory type kGlueTapPageType) and the
// engine's input ring (kGlueTapRingType), both read-only, and selectors kGlueTapSelectorStart/Stop hold one isoch reference per client.
// This header is the contract shared by the kext and openaudio-glue/driver: plain C types, little-endian, fixed layout.
#ifndef OA_GLUETAP_H
#define OA_GLUETAP_H

#include <stdint.h>

#define kGlueTapMagic          0x45554C47u   // "GLUE" as little-endian bytes
#define kGlueTapVersion        1u
#define kGlueTapPageType       0x1000u       // clientMemoryForType: this page (checked before the MIDI destination range 0x800+)
#define kGlueTapRingType       0x1001u       // clientMemoryForType: the engine's input ring (raw AM824 quadlets, 8192 frames)
#define kGlueTapSelectorStart  508u          // after the reference's 500..507 (the unmodified M-Audio MIDI plug-in uses those only)
#define kGlueTapSelectorStop   509u
#define kGlueTapMaxChannels    32u
#define kGlueTapRingFrames     8192u         // setNumSampleFramesPerBuffer(0x2000), FWBaseEngine::initHardware

enum { kGlueTapStopped = 0, kGlueTapRunning = 1, kGlueTapRetired = 2 };

// Host channel n = AM824 wire slot `slot` of the frame `delay` frames earlier (the model's input proc expressed as a table).
typedef struct GlueTapChannel { uint16_t slot; uint16_t delay; } GlueTapChannel;

typedef struct GlueTapPage {
    uint32_t magic;                  // kGlueTapMagic
    uint32_t version;                // kGlueTapVersion
    volatile uint32_t generation;    // bumped whenever the geometry, the ring or the stream changes: remap, resync, reset DSP state
    volatile uint32_t state;         // kGlueTapStopped / Running / Retired (engine gone: reopen the user client)
    // geometry, valid for one generation
    uint32_t rateHz;
    uint32_t inChannels;             // host input channels (the Core Audio input stream's channel count)
    uint32_t wireQuadlets;           // audio quadlets per wire frame (engine inChannels)
    uint32_t frameQuads;             // wireQuadlets + MIDI quadlets: the ring's frame stride in quadlets
    uint32_t ringFrames;             // kGlueTapRingFrames
    uint32_t ringBytes;              // frameQuads * 4 * ringFrames
    GlueTapChannel map[kGlueTapMaxChannels];
    // clock and position. seq is odd while the writer updates the wrap pair (seqlock): read seq, the fields, seq again; retry on change.
    volatile uint32_t seq;
    volatile uint32_t lastBank;      // last TX bank (0..3, 2048 frames each)
    volatile uint64_t wrapHostTime;  // mach_absolute_time of the last ring wrap = the time IOAudioEngine::takeTimeStamp gets
    volatile uint64_t wrapCount;     // wraps since the streams started; 0 = not locked yet (the first two TX laps take no timestamp)
    volatile uint32_t writeOff;      // RX write position in bytes (inputRing.writeOff), for the underrun check
    volatile uint32_t isochRefs;     // engine isoch references (audio, MIDI, Glue)
    volatile uint32_t glueRefs;      // of which Glue's
    // Bumped only when the engine allocates a new input ring (ResetSampleBuffers). A client remaps the ring only then: mapping the
    // same ring again and unmapping the older mapping by memory type can tear down the newer one (IOConnectUnmapMemory64 removes a
    // mapping of the type's *current* descriptor; found 2026-10-28, Glue G11). 0 on kexts before this field (never remap).
    volatile uint32_t ringGeneration;
    uint32_t reserved[15];
} GlueTapPage;

#endif
