# Glue — architecture (v1, 2026-10-28)

> **Correction (2026-11-02):** the control side is the standalone **OpenAudio Glue app** (`openaudio-glue/panel/`), not views in
> the main control panel's Console mode (user, 2026-10-28, plan phase 4 correction). So the main panel has no Glue views; its only
> planned hook is one menu item that opens the app (not added yet). The clock-domain mapping is verified (G5 correction, G9). The
> rows and lines below that said otherwise are marked.

Target: macOS 10.13.6 High Sierra first, built on the build PC with Xcode 9.4.1 and the 10.13 SDK (plan, "Hard requirement").

```
FW1814 ─RX DCL→ kext input ring (raw AM824, 8192 frames) ─┬─ IOAudioFamily → "FW1814" device (unchanged)
                                                          └─ user-client memory 0x1000 (glue page) + 0x1001 (ring), read-only
                                                                    │   GlueStart/GlueStop selectors hold one isoch reference
          OpenAudioGlue.driver  (AudioServerPlugIn inside coreaudiod)
            ReadInput: unpack AM824 by the published map → ×2^-23 → per channel
            Transformer → Tube → EQ → Comp → Desk colour → Tape  → interleaved float
                                                                    │
                                          "<model> Glue" input device, UID com.openaudio.glue.<GUID>
          OpenAudio Glue app ── AudioObjectSetPropertyData ('glst' plist; meters 'glmt') ──> plug-in
          (was "Control panel (Console mode)" — corrected 2026-11-02)
          plug-in ── WriteToStorage / CopyFromStorage ──> settings per GUID
```

## Pieces and where they live

| Piece | Location | Owner |
|---|---|---|
| DSP chain (C++17 language, C++14 library) | `openaudio-glue/dsp/` | Glue |
| AudioServerPlugIn | `openaudio-glue/driver/` | Glue |
| Bench / port-diff tools | `openaudio-glue/tools/` | Glue |
| Installer (`OpenAudio Glue.pkg`) | `openaudio-glue/installer/` | Glue |
| **Hook:** glue tap (memory types 0x1000/0x1001, GlueStart/GlueStop, per-model channel map, ring flag) | `openaudio-mafw-bebob/kext/src/base/FWUserClient.*`, `FWSharedSync.cpp`, `FWDCLProgram.cpp`, model files | main project, logged in the main R-log |
| Glue app (stage selectors, channel matrix, EQ editor, Core Audio backend, `GlueEQMath.swift`) | `openaudio-glue/panel/` → `OpenAudio Glue.app` | Glue |
| ~~**Hook:** Console-mode GLUE column, stage selectors, EQ editor in `openaudio-mafw-bebob/control-panel/Sources/`~~ | dropped (correction 2026-11-02): replaced by the standalone app above | — |
| **Hook (planned, not added):** one menu item in the main control panel that opens `OpenAudio Glue.app` | `openaudio-mafw-bebob/control-panel/Sources/` | main project, to be logged in the main R-log |

## Glue page (memory type 0x1000), draft layout
Fixed little-endian struct, one page. The version field lets the plug-in refuse a page it doesn't understand.

| Field | Meaning |
|---|---|
| magic `'GLUE'`, version | layout check |
| generation | bumped on a format change, engine stop or re-publish; the plug-in remaps and resyncs when it changes |
| rateHz, inChannels, midiQuads, frameQuads, ringFrames | stream geometry |
| map[inChannels] = {wireSlot, delayFrames} | host channel n = AM824 slot `wireSlot` of the frame `delayFrames` earlier (FW1814 channels 2–7: slots 1, 5, 2, 6, 3, 7, delay 16 — `FW1814Device.cpp:76-100`; Audiophile/FW410: [1, 3, 0, 2] — `FWSamples.cpp:76-84`) |
| writeOff, lastBank | live ring position |
| wrapHostTime (mach absolute time), wrapLoopCount | the same timestamp `takeTimeStamp` receives (`FWDCLProgram.cpp:475-478`) |
| running, isochRefs | stream state |

## Clocking
- Period 8192 frames, the same as the FW1814 engine (`setNumSampleFramesPerBuffer(0x2000)`, `FWBaseEngine.cpp:60`).
- `GetZeroTimeStamp` returns (wrapLoopCount × 8192, wrapHostTime).
- Clock domain: the engine's `IOAudioEngineClockDomain` (G5; the HAL mapping is verified on 10.13.6, G9 — corrected
  2026-11-02).

## Failure behaviour
- No unit, an unknown page version, or streams not yet locked (lap < 3): the device reports silence and keeps its timestamps
  advancing from the host clock.
- Underrun (`writeOff` behind the requested frames): silence for that cycle, then resync.
- A generation change: `RequestDeviceConfigurationChange` if the channel count changed, otherwise remap and reset the DSP
  state.
