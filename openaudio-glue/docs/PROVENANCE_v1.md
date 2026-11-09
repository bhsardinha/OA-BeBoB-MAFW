# Glue — provenance log (G-numbers)

Every value or behaviour Glue relies on, with its origin. Same rules as the main project's `PROVENANCE-RESOLUTIONS.md`: cite the
source, label anything unverified, add dated `Correction` blocks instead of rewriting. The main R-log only gets the hooks Glue
adds to the kext and the panel.

## G1 — An AudioServerPlugIn can't use the HAL client API, but can open our user client (2026-10-28)
- Source: `AudioServerPlugIn.h` in the **10.13 SDK** on the build PC
  (`MacOSX10.13.sdk/System/Library/Frameworks/CoreAudio.framework/Headers/AudioServerPlugIn.h`, sha256
  `916a75f928b457077fc274f6021802ab5c11a67d603173460b9952540e7bea33`):
  - line 35: "may not make any calls to the client HAL API in the CoreAudio.framework";
  - line 52: custom user clients must be listed in `AudioServerPlugIn_IOKitUserClients`;
  - line 29: `AudioServerPlugIn_LoadingConditions`;
  - line 72: `RequestDeviceConfigurationChange`;
  - line 81: `GetZeroTimeStamp`;
  - lines 331/333: `kAudioServerPlugInIOOperationReadInput` / `…ProcessInput`;
  - line 493: `WriteToStorage`.
- Same text in the 15.2 SDK on the dev Mac.
- Consequence: Glue can't open the FW1814 Core Audio device. It reads the input ring from the kext through
  `com_openaudio_FWUserClient`, the vendor user client created by `IOUserClientClass` in `openaudio-mafw-bebob/kext/Info.plist`.

## G2 — Build-PC toolchain limits for Glue code (2026-10-28)
- `fw1814-build`: macOS 10.13.6, Xcode 9.4.1, Apple LLVM 9.1.0 (clang-902.0.39.2).
- A test program with a generic lambda, `if constexpr`, `std::atomic` and `std::array`:
  - `-std=c++17 -mmacosx-version-min=10.13`: compiles;
  - `-std=c++11`: error;
  - `-std=c++14`: warning (`if constexpr` is a C++17 extension).
- libc++ headers in the Xcode 9.4.1 toolchain (`…/XcodeDefault.xctoolchain/usr/include/c++/v1/`): **no `optional`, `variant` or
  `filesystem`**.
- **Rule:** Glue C++ uses `-std=c++17` with the C++14 library only. The panel stays Swift 4.1.

## G3 — The kext's input ring can be mapped into a user task on 10.13.6 (2026-10-28)
- **The ring.** `TRingBuffer::InitializeWithSizeInBytes` allocates
  `IOBufferMemoryDescriptor::withOptions(0, bytes, 0x1000)` (`openaudio-mafw-bebob/kext/src/base/FWSharedSync.cpp:7`).
  - Size is `(inChannels + midi) × 0x8000` bytes (`FWBaseEngine.cpp:648-652`), so always a whole number of pages.
- **The 10.13.6 kernel.** `xnu-4570.71.2`, `iokit/Kernel/IOBufferMemoryDescriptor.cpp`:
  - a non-pageable buffer with `alignment == page_size` and capacity ≥ one page goes to `IOMallocAligned(capacity, alignment)`
    (lines 233–244);
  - so the ring owns whole, page-aligned pages, and no other kernel data shares them;
  - `kIOMemoryKernelUserShared` is **not read anywhere** in that file.
- **The macOS 11 kernel.** `xnu-7195.141.2`: neither `IOBufferMemoryDescriptor.cpp` nor `IOMemoryDescriptor.cpp` contains the
  name.
- **The 10.13 SDK header.** `Kernel.framework/Headers/IOKit/IOBufferMemoryDescriptor.h:40,153` defines
  `kIOMemoryKernelUserShared = 0x00010000`: "pass to request memory that will be mapped into both the kernel and client
  applications".
- **Decision (phase 2).** Add `kIOMemoryKernelUserShared` to the ring allocation, as the header documents.
  - It has no effect on 10.13.6's allocator.
  - The meter page (`FWAudioDeviceProps.cpp:279`) and the MIDI rings (`FWMidi.cpp:7`) already use the flag.
  - Deviation from the reference allocation, logged in the main R-log.
- **Unverified until phase 2 runs:** that `clientMemoryForType` returning the ring's descriptor actually maps it. Check:
  `IOConnectMapMemory` from a bench tool on the build PC.

## G4 — What runs the isoch streams, the ring and the timestamps (2026-10-28)
- **Reference-counted start.** `FWBaseEngine::StartIsochStreams` (`FWBaseEngine.cpp:545-560`; ref `StartIsochStreams`) starts
  the DCL programs on the first reference (`fIsochRefs`) and resets `inputRing` and `lastBank`.
  - Callers: `ActivateSubEngine` (`FWDiscreteEngine.cpp:66-80`), i.e. a Core Audio client starting the engine (mask 1) or a pair
    engine;
  - and `StartMIDI` (`FWEngineMidi.cpp:92-99`, `FWEngine2` variant `:123-133`), i.e. selector 500 from the MIDI plug-in.
- **Clock without an audio client.** `OutputDCLCallback` (`FWDCLProgram.cpp:453-480`) updates `sh->lastBank` on every bank and
  calls `sh->engine->takeTimeStamp(true, …)` on bank 3 from lap 3 onward, whatever the IOAudioEngine state.
  - So the clock runs whenever the streams run, with or without an audio client.
  - This is the reference's own MIDI-only situation.
  - Docs: C1 §3.7–3.8 (`docs/system-map/C1-lifecycle-state-machine.md:122-129`); idle TX is data-bearing (capture 74).
- **Consequence (plan correction).** Glue doesn't need the FW1814 device to be running. Phase 2 adds selectors after 507 (additive;
  the MIDI plug-in uses 500–507 only, `FWUserClient.cpp:17-26`):
  - **GlueStart** takes one isoch reference;
  - **GlueStop** releases it, and it is also released when the user client closes or dies.

  Glue then works on its own, or in an aggregate with the FW1814 outputs.
- **Not yet checked:** the bus-reset path and rate changes while only Glue holds a reference. Phase 2 follows the existing
  pause/resume path (`pauseAudioEngine`, `FWBaseEngine.cpp:132-139`), which already stops and resumes MIDI-held streams.

## G5 — Clock domain (2026-10-28)
- The engine publishes `IOAudioEngineClockDomain` in the registry. B5 capture `40-ioreg-playback.txt` (SL v1.9) shows one value
  per engine, e.g. `589718528`; it's listed in `docs/system-map/B5-sl-dtrace-truth/README.md:234`.
- Glue reads that value from the unit's engine (plain IOKit registry read, allowed per G1) and reports it as its
  `kAudioDevicePropertyClockDomain`, so an aggregate sees both devices on one clock.
- **Unverified:** that the 10.13 HAL maps `IOAudioEngineClockDomain` to `kAudioDevicePropertyClockDomain` for IOAudio devices.
  Check on the build PC with `tools/halinfo` once Glue exists. Until then the aggregate's own drift correction is the fallback.
  > **Correction (2026-10-28, verified, G9):** on the build PC (10.13.6) the HAL's `kAudioDevicePropertyClockDomain` for each
  > FW1814 equals its engine's `IOAudioEngineClockDomain` (unit B: 226137504 in both `glue-probe` and `ioreg`).

## G6 — DSP sources and their cost (2026-10-28)
- **Licence.** Airwindows: `github.com/airwindows/airwindows`; GitHub API `license: MIT`; LICENSE text "Copyright (c) 2018 Chris
  Johnson"; source headers "Airwindows uses the MIT license".
- **Cost, measured on the dev Mac** (MacBookAir7,2, i5-5250U 1.6 GHz):
  - setup: each plug-in's own `processReplacing` behind a stub VST base class, 20 s of a 220 Hz sine at 48 kHz in 64-frame
    blocks, default parameters, one run, stereo instance;
  - % of one core: ToTape9 1.49 · TapeHack2 0.40 · IronOxide5 1.02 · Channel9 0.44 · Tube2 0.45 · Interstage 0.22 ·
    ButterComp2 0.51 · Pressure5 0.39 · Console8LiteChannel 0.46 · Console8LiteBuss 0.93 · ConsoleXChannel 3.30.
- **Excluded, GPL-3** (incompatible with PolyForm NC): Chow Tape Model (`jatinchowdhury18/AnalogTapeModel`, GitHub
  `license: GPL-3.0`), BlackHole.

## G7 — The mono ports equal the Airwindows originals (2026-10-28)
- Tool: `tools/port-diff`.
  - `fetch-ref.sh` downloads the originals at commit `dfa54018e788fd67b8e57ae6c8012a05f1f086a6`.
  - Each original is compiled behind a minimal VST stand-in, fed L = R, and its L output is compared with the port.
  - Both start their dither from the same seed (the test opens the originals' private `fpdL`/`fpdR`).
- Signal: 0.6 s of program at −12 dBFS (110 Hz, 1 kHz and 5.5 kHz sines plus noise), then 0.6 s at +3 dBFS to exercise the
  clippers.
- Cases: Interstage; Tube2, Channel9 and ToTape9 at their defaults and every Glue preset level; Channel9 with all five types at
  defaults; ToTape9 with flutter off and on. Each at 44.1, 48, 96 and 192 kHz.
- **Result: 100 cases, worst residual −146.9 dB** (float rounding; many cases bit-identical).
  - Dev Mac (macOS 14.8.9, Apple clang 15): `ALL PORTS MATCH`.
  - Build PC (10.13.6, Xcode 9.4.1): `ALL PORTS MATCH`.
- History: the first ToTape9 port failed with flutter on at 96 kHz (residual −14 to −62 dB). It used its own sweep where the
  original reads the R channel's. The fix is a shadow of the R flutter state (`DSP_v1.md` correction).

## G8 — Host tests and CPU (2026-10-28)
- `make -C dsp test`: `ALL PASS` on the dev Mac and on the build PC (10.13.6). Tests:
  - bit-exact bypass, also after switching everything on and off again;
  - EQ formula checks (centre gains, shelf half-gain at f0, clamping, proportional Q);
  - the EQ filter against its formula at 6 rates (± 0.05 dB);
  - compressor static curve, steady state (± 0.5 dB) and program-dependent release;
  - level match of every preset at 6 rates (± 0.5 dB);
  - stability under +12 dB EQ, −40 dB threshold and overload at 6 rates.
- `glue-bench`, everything Full, 4 EQ bands, threshold −20 dB, 64-frame blocks, one thread (information only; the user put
  quality over CPU, 2026-10-28):
  - **dev Mac (i5-5250U):** 16 ch at 48 kHz = 42.7 % of one core; 18 ch at 44.1 kHz = 40.7 %; 16 ch at 96 kHz = 82.5 %.
  - **Build PC (i3 540, 10.13.6):** 18 ch at 48 kHz = 48.6 %; 12 ch at 96 kHz = 68.8 %; 2 ch at 192 kHz = 24.0 %.
  - Per stage (dev Mac, % of one core per channel): Tape 1.10, Desk 0.36, Tube 0.33, Comp 0.32, Transformer 0.20,
    EQ 4 bands 0.14.
  - Processing two channels interleaved was tried: 2.73 vs 2.43 % per channel, so it was not kept.
- Phase 3 note: the plug-in runs every channel in Core Audio's IO thread. If 96 kHz with small buffers is too tight on the
  build PC, spreading the channels over worker threads keeps the output identical.

## G9 — Two FW1814s: the Dual aggregate as Core Audio has it (2026-10-28)
- Tool: `tools/glue-probe` (read-only), run on the build PC (10.13.6) with two FW1814s attached, ADAT mode, 48 kHz, and the
  panel's Dual mode on.
- **Units.**

  | Unit | Core Audio UID | Clock domain | In / out | Latency in / out | Safety offset in / out |
  |---|---|---|---|---|---|
  | A | `com_m_audio_FWEngine2:0xD6C040012337F` | 210538784 | 16 / 12 | 113 / 112 | 48 / 48 |
  | B | `com_m_audio_FWEngine2:0xD6C040074B9FB` | 226137504 | 16 / 12 | 113 / 112 | 48 / 48 |

  The buffer is 512. The domain equals each engine's `IOAudioEngineClockDomain` in `ioreg`.
- **Aggregate.** "Dual FireWire 1814", UID `com.openaudio.bebobmafw.dual1814`, 32 in / 24 out. Sub-devices A then B; clock
  master A; drift compensation 0 on both.
  - That matches the panel's description in `openaudio-mafw-bebob/control-panel/Sources/AdvancedDual.swift:48-54`.
  - B is locked to A's word clock (BNC), which is why the panel turns drift correction off even though the domains differ.
- **Consequence for Glue:** see `GLUE_PLAN_v1.md` "Dual".

## G10 — Two FW1814s synced over ADAT instead of word clock (2026-10-28)
- Setup (user): A's optical out → B's optical in; B set to external digital sync.
- `tools/glue-probe/glue-clock` (read-only; `MAClockSource` decoded as `MaKeys.swift` does):

  | Unit | `MAClockSource` | Sync | Optical in / out | `MARateLocked` | `MAExternalSyncRate` |
  |---|---|---|---|---|---|
  | A `0xD6C040012337F` | `0x4000001B` | internal, digital inputs muted | ADAT / ADAT | 1 | 48000 |
  | B `0xD6C040074B9FB` | `0x40000019` | external digital | ADAT / ADAT | 1 | 48000 |
- `tools/glue-probe/glue-lockcheck`: IOProcs on both units at once, output silence, rate unchanged, both engines idle before.
  The measured rate is sample time over host time.

  | Run | A | B | B vs A |
  |---|---|---|---|
  | 20 s | 48000.9862 Hz | 48000.9427 Hz | −0.906 ppm |
  | 60 s | 48000.9413 Hz | 48000.9094 Hz | −0.666 ppm |
- **What it shows:** both streams run at one rate, about 20 ppm over the host clock, the same for both; the aggregate sees one
  clock.
- **What it does not show:** that B's converters follow A. The kext's timestamps come from the FireWire bus cycle time
  (`FWDCLProgram.cpp:453-480`), and both units are on one bus, so this measures the bus-derived stream rate, not the units'
  sample clocks.
- **Unverified:** sample-level lock through ADAT. Check: a −50 dBFS tone from A's ADAT out 1 (aggregate output 5) captured on
  B's ADAT in 1 (aggregate input 25) for 60 s; the delay must stay constant, with no slips.
- **Panel note** (main project, not changed): `AdvancedDual.swift:65-66` and `AdvancedDualWindow.swift:420,679` require B on
  word clock, so the Dual checklist marks ADAT sync as failing.

## G11 — First hardware session (build PC, 10.13.6, two FW1814s, 2026-10-28)
- **Installed through the packages** (user's go-ahead): `OA-BeBoB-MAFW-0.6.0-glue1-classic.pkg` → driver **0.4.0** loaded after a
  restart (no panic); then `OpenAudio-Glue-1.0.0-dev.pkg`.
- **Devices.** Both "FireWire 1814 Glue" devices were published 0.2 s after Core Audio started (plug-in log: units found, 3 DSP
  worker threads).
  - The page and the input ring **map into Core Audio's plug-in process**. This settles G3 and confirms G1 (custom user client
    from an AudioServerPlugIn).
  - `glue-probe` per Glue device: transport `virt`; clock domain equal to its unit's; input latency **114** (the unit's 113 + 1
    sample of compensation); safety offset 48; 16 inputs at 48 kHz.
- **Crash found and fixed.**
  - Symptom: `coreaudiod` SIGSEGV on a Glue DSP thread in `Device::processChannel`. Disassembly: the ring read
    `movl (%r10,%rsi,4)`; base plus index × 4 = the faulting address, 60 KB into a 544 KB ring, so the mapping had lost its
    memory.
  - `tools/glue-probe/glue-tapread` (an ordinary process) mapped the same page and ring and read every page while the unit
    started: fine, so the kext side was sound.
  - Cause: on every generation change the plug-in mapped the same ring again and, 2 s later, called
    `IOConnectUnmapMemory64(conn, 0x1001, task, oldAddress)`. That unmaps a mapping of the type's *current* descriptor and can
    tear down the live mapping. The crash came 2 s after the unit's start bumped the generation.
  - Fix:
    - **Plug-in:** maps the ring once; remaps only when the page's new `ringGeneration` changes; never unmaps a ring while
      connected (`IOServiceClose` releases them).
    - **Kext:** `ringGeneration`, bumped only by `ResetSampleBuffers` (reserved slot, layout unchanged; 0 on the installed
      0.4.0 = never remap). Built, not installed yet.
- **Bypass test** (`tools/glue-probe/glue-bypasscheck`): a unit's own inputs and its Glue inputs recorded at once, timelines aligned
  through host time (they share wrap times; the lap counts matched, Glue − unit = 0.3 samples at the same host time).
  - **Unit B: 100.000000 % bit-identical (9,981,952 of 9,981,952 samples, 15 s, 16 channels) at a lag of exactly 1 sample** (the
    compensation).
  - **Unit A: channels 2–16 bit-identical at lag 1**, including channels 3–8, which use the FW1814's 16-frame delay map, so that
    map is right on the hardware.
  - Unit A's channel 1 differed because the user had set it (Glue app: Source Drums, threshold −21.5 dB, read back from the
    device's `glst`). That also shows the app → plug-in settings path working on the real device.
  - Signal: analog inputs 1–8 carried the preamps' noise floor (≈ 650,000 non-zero samples each); ADAT 9–16 were digital zero
    (nothing connected).
- **Core Audio SIGILL at restarts** (`AHS_DeviceSettingsManager`, Apple code).
  - Seen at 22:24 (Glue not loaded) and 22:30 (Glue loaded), both at the moment Glue's postinstall restarted Core Audio, and in
    older reports from 2026-10-01 and 2026-10-16, before Glue existed.
  - **Unverified** whether Glue contributes. Check: restart Core Audio with and without Glue installed and compare.

## G12 — High Sierra iMac: the user's tests pass (2026-11-02)
- **Source: the user's report**, 2026-11-02: "the tests in the real high sierra iMac ran fine. everything works as should." The
  machine is the build PC (iMac11,2, 10.13.6) with the two FW1814s of G9–G11.
- What was installed for those tests and which checks were run aren't recorded here: no capture, log or package digest came with
  the report, and the build PC didn't answer SSH when this entry was written (2026-11-02). So the entry stands on the user's
  word, not on a capture.
- Items still open after this report:
  - **Kext `ringGeneration` (G11).** The main test package in the repo, `openaudio-mafw-bebob/dist/OA-BeBoB-MAFW-0.6.0-glue1-classic.pkg`
    (built 2026-10-28 22:16), predates the `ringGeneration` commit (`628fb4c`, 22:33), so that half of the fix is in source only
    unless a newer package was installed. The plug-in doesn't depend on it: when it reads 0 it never remaps.
  - **Core Audio SIGILL at restarts (G11).** This is Apple code and it also happened before Glue existed. A working setup doesn't
    tell whether Glue adds to it. **Unverified.**
  - The **Modern edition** (macOS 11–15) and the **main panel's menu item** are not covered by a High Sierra test.

## G13 — Release 1.0.0 (2026-11-09)
- **Packages** (built 2026-11-06, commit `e0d455b`): `OpenAudio-Glue-1.0.0-classic.pkg` / `-modern.pkg` (same binaries, each accepts
  only its macOS range), each in a disk image with an Uninstall app. The plug-in matches the FireWire 1814 only. The Airwindows MIT
  notice ships in the app and in the plug-in.
- **Published** with the main package 0.6.0 (driver 0.4.0 with the tap and `ringGeneration`, panel 2.2 with the menu item) as GitHub
  release v0.6.0. Digests are in `openaudio-mafw-bebob/installer/release-notes/RELEASE-0.6.0.md`.
- **Main driver, same day:** a panic when a second FW1814 was switched on (2026-11-06) was traced to the driver's bus-reset
  reconnect, not to the tap, and fixed (main `PROVENANCE-RESOLUTIONS.md` R144). The user then switched the second unit on and off
  with the fixed driver: audio paused and came back, no panic (the user's report; no capture of the Glue device across it).
- **Not verified:** the Modern edition has not been run.
