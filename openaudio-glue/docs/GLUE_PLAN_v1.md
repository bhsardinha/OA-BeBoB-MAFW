# Glue — a pre-processing input device for the FW1814 (plan v1, approved 2026-10-28)

## Context
User request 2026-10-28: a simple, mostly fixed "glue" stage applied to the FW1814 inputs **while recording**, mainly for the
Advanced window's Console mode (analog console → FW1814 at unity → DAW). It is not a summing bus: each input is processed on its
own before it reaches the DAW. Desk colour, transformer, tube and tape are global, with fixed character. Each channel gets a
threshold-only auto compressor and a 4-band EQ edited on an XY graph.

**Hard requirement (user, 2026-10-28): Glue MUST work on macOS 10.13.6 High Sierra (Classic edition).** Every Glue binary is
built on the build PC (`fw1814-build`, 10.13.6, Xcode 9.4.1, 10.13 SDK) with `-mmacosx-version-min=10.13`; nothing may use an API
newer than the 10.13 SDK. The Modern edition (11–15 Intel) is secondary: same binaries, checked there afterwards. Toolchain limits
found on the build PC (G2): C++17 language is accepted (Apple LLVM 9.1.0, clang-902.0.39.2), but libc++ has **no `<optional>`,
`<variant>`, `<filesystem>`**, so Glue code uses C++17 language with the C++14 library; the panel stays Swift 4.1.

Decisions (AskUserQuestion, 2026-10-28):
- **EQ:** 4 bands per channel (low shelf, low-mid, high-mid, high shelf). Every band is a dot on the XY graph (frequency, gain).
  Shelves have a fixed slope; the two mids use API-style proportional Q (narrower as gain rises).
- **Presets:** one selector per stage (Colour, Transformer, Tube, Tape), each Off / Soft / Medium / High / Full, global for all
  channels of a unit.
  > **Correction (user, 2026-10-28):** Desk has its own options: Off, Neve Medium, Neve High, API Medium, API High, SSL Medium,
  > SSL High.
- **DAW device:** a separate Core Audio input device per unit, "FW1814 Glue", on the FW1814's clock. The plain FW1814 device is
  unchanged.

**Spin-off project (user, 2026-10-28):** its own branch, its own folder, its own documentation.
- **Branch `glue`**, from `main` at `227cb06` (0.5.1 published; the working tree was already clean, so step 1 below was not needed).
  - Nothing merges into `main` until the user says so; nothing goes public until the user says so (sync-to-public rules apply).
  - The uncommitted work on `main` (user, 2026-10-28):
    1. I commit it first on `main`, only the files already modified, with a plain message (the deleted
       `.claude/scheduled_tasks.lock` included);
    2. then I create `glue` from that commit. Nothing is pushed.
- **Folder `openaudio-glue/`** at the repo root. It holds everything Glue owns:

  ```
  openaudio-glue/
    README.md, CHANGELOG.md, VERSION (1.0.0), NOTICE.md (Airwindows MIT credit)
    docs/   GLUE_PLAN_v1.md (this plan), ARCHITECTURE_v1.md, DSP_v1.md (stages, presets, tuning log),
            PROVENANCE_v1.md (own R-numbering G1, G2…), STATE.md, captures/
    dsp/    C++ chain library + tests/   (phase 1)
    driver/ OpenAudioGlue.driver (AudioServerPlugIn)  (phase 3)
    tools/  glue-bench, port-diff  (+ README recipes)
    installer/ its own .pkg + uninstall (phase 5)
  ```
- **Hooks outside the folder** stay minimal, each with a comment pointing to `openaudio-glue/docs/`:
  - the kext glue tap (phase 2, in `openaudio-mafw-bebob/kext/`);
  - the panel views (phase 4, in `openaudio-mafw-bebob/control-panel/`).

  > **Correction (2026-11-02):** after the phase 4 correction (the Glue app is standalone) the panel hook is only the menu item
  > that opens the app; there are no panel views. See `ARCHITECTURE_v1.md`.

  Each hook is also logged in the main `PROVENANCE-RESOLUTIONS.md`, since it changes the driver and panel.

Research already done (this session):
- DSP source: Airwindows, MIT (`github.com/airwindows/airwindows`; GitHub reports `license: MIT`). CPU measured on the dev Mac (i5-5250U, 48 kHz / 64 frames, default
  settings, stereo instance, % of one core): ToTape9 1.49, Channel9 0.44, Tube2 0.45, Interstage 0.22, ButterComp2 0.51.
  Bench harness: a scratch bench harness (not kept)
  (moved into `openaudio-glue/tools/` in phase 1).
- Chow Tape Model is GPL-3 and can't be combined with this project's PolyForm NC licence, so it is not used. BlackHole (GPL-3) is
  not used either.
- `AudioServerPlugIn.h` (10.13 SDK on the build PC and the 15.2 SDK here), lines 32–53:
  - a plug-in **may not call the client HAL API**, so it can't open the FW1814 device;
  - it **may open a custom IOKit user client** if the class is listed in `AudioServerPlugIn_IOKitUserClients`.

  So the Glue device reads the FW1814 inputs straight from our kext.

## Architecture

```
FW1814 ─RX DCL→ kext input ring (raw AM824) ──┬─ IOAudioFamily → "FW1814" device (unchanged)
                                               └─ glue page + ring mapped read-only by a new user-client memory type
                                                       │
           OpenAudioGlue.driver (AudioServerPlugIn, in coreaudiod)
           per channel: Transformer → Tube → EQ → Comp → Desk colour → Tape
                                                       │
                                     "FW1814 Glue" input device (same clock, same timestamps)
           DAW: aggregate "FW1814 Glue (in) + FW1814 (out)", FW1814 as clock source
           Panel ── AudioObjectSetPropertyData (custom properties) ──> plug-in; plug-in persists via host storage
```

**Latency:** no added buffer. The DSP runs inside the plug-in's `ReadInput` on the HAL's IO thread, reading frames the kext
already holds. The device reports the FW1814's input latency and safety offset.

## Phase 0 — checks before code (done 2026-10-28, `PROVENANCE_v1.md` G1–G6)
1. **Ring mapping (G3).** On 10.13.6 the ring is page-aligned whole pages from `IOMallocAligned`, so it is mappable. The kernel
   ignores `kIOMemoryKernelUserShared`; phase 2 adds the flag anyway, as the SDK header documents. Actual mapping is confirmed in
   phase 2 with a bench tool.
2. **Idle running (G4).** The streams run when an audio client or MIDI holds an isoch reference, and the TX callback updates
   `lastBank` and the timestamps whatever the IOAudioEngine state.
   > **Correction (2026-10-28, G4):** v1 does **not** need the FW1814 device to be running. Phase 2 adds GlueStart/GlueStop
   > selectors (after 507) that take and release one isoch reference.
3. **Clock domain (G5).** Glue reports the engine's `IOAudioEngineClockDomain`. That the HAL uses it is unverified; check with
   `halinfo`.
4. **C++ level (G2).** `-std=c++17` with the C++14 library (no `<optional>`/`<variant>`/`<filesystem>`).
5. The branch, folder and docs exist; this plan is `openaudio-glue/docs/GLUE_PLAN_v1.md`.

## Phase 1 — DSP library `openaudio-glue/dsp/` (host-testable, no hardware)
- Plain C++ with no dependencies, no allocation on the audio thread, float32 I/O.
- **Ports** (MIT headers kept, credit in `NOTICE.md`), refactored from stereo to one per-channel state struct:
  - Interstage → transformer
  - Tube2 → tube
  - Channel9 → desk colour (Neve / API / SSL type is part of the preset table)
  - ToTape9 → tape, including head bump; its frequency set near 15 ips character, tuned by ear

  The mono port uses the L path. ToTape9's flutter references the R state: the port keeps the L logic only. Verify each port
  against the original by feeding L = R and comparing L output (tolerance logged).
- **Presets** in `GluePresets.h`: per stage, Off (exact bypass) / Soft / Medium / High / Full → that stage's parameters plus a
  drive and dry/wet. Interstage has no parameters, so its presets set drive and blend.
  - First values are starting points, labelled as such, and tuned by ear with the user.
  - Output level is matched per preset, so switching presets doesn't jump in loudness.
- **Compressor** (own code, about 60 lines):
  - feed-forward, RMS detector, fixed 3:1 ratio, 6 dB soft knee;
  - program-dependent attack/release (fast/slow two-stage release);
  - threshold −40…0 dB, where 0 means off;
  - fixed partial makeup referenced to −18 dBFS;
  - gain reduction exposed for meters.

  Ratio, times and makeup are design values to tune.
- **EQ:** RBJ Audio-EQ-Cookbook biquads (cite).
  - Low shelf 30–600 Hz and high shelf 1.5–16 kHz, slope S = 1 fixed.
  - Low-mid bell 60 Hz–2 kHz and high-mid bell 500 Hz–12 kHz.
  - Gain ±12 dB on every band.
  - Proportional Q = 0.5 + 0.1·|gain dB| (design value, API 550-like).
  - Coefficients recomputed per block with parameter smoothing, never per sample.
- **Tests and bench:**
  - a host test binary (same `check()` style as `control-panel/Tests/LayoutTests.swift`);
  - port-vs-original diffs;
  - EQ magnitude at test points against the analytic formula;
  - a CPU bench for the full chain × the channel count (informational).
    > **Correction (user, 2026-10-28): "CPU is no problem, the important is to keep the quality."** The 30 % target is dropped.
    > No speed change may alter the sound; optimisations are kept only if their output is identical.
  - The bench goes in `openaudio-glue/tools/` with a README recipe.

## Phase 2 — kext: an additive "glue tap" (deliberate deviation, R-entry + comment naming why)
- In `kext/src/base/FWUserClient.cpp:77-98` (`clientMemoryForType`), add two **read-only** types. Selectors 500–507 stay
  untouched, so the unmodified M-Audio MIDI plug-in is unaffected.
  - **0x1000 = glue page** (one page):
    - magic, version, generation (bumped on format change, engine stop or re-publish);
    - rate, inChannels, frameQuads (= inChannels + MIDI), ring frames;
    - the host-channel map as (wire slot, delay frames) pairs. Generic, filled by each model: FW1814 channels 2–7 use slots
      1, 5, 2, 6, 3, 7 with a 16-frame delay (`FW1814Device.cpp:76-100`); Audiophile/FW410 use [1, 3, 0, 2]
      (`FWSamples.cpp:76-84`); FW410 high rates `{0, 1}`;
    - live `writeOff`, `lastBank`, the last wrap timestamp (ns) and loop count;
    - a running flag.
  - **0x1001 = the input ring** (`fShared.inputRing.mem`, allocated with `kIOMemoryKernelUserShared`, G3).
- **New selectors after 507 (G4):**
  - GlueStart = `StartIsochStreams` (one reference held per user client);
  - GlueStop = `StopIsochStreams`, also run from `clientClose`/`clientDied`.

  The `IOExternalMethod` table only grows.
- Live fields are written with plain stores from the existing callbacks: the RX copy in `FWDCLProgram.cpp:822-866` and the
  `takeTimeStamp` site at `:475-478`. No new locks.
- The map is published by a generic model virtual next to `ResetRearrangementProcs`, with no 1814 tables in shared code.
- Build both editions (`make -C kext` on the build PC, `BAND=modern` here); `scripts/stage-kext.sh` / `kextutil -t -n` only.

## Phase 3 — HAL plug-in `openaudio-glue/driver/` → `OpenAudioGlue.driver`
- C++ (standard per phase 0), written from scratch on the `AudioServerPlugIn.h` interface; Apple's NullAudio sample is used as a
  structure reference only.
  - Build: x86_64, `-mmacosx-version-min=10.13`, on the build PC; the same binary goes into both editions.
  - Info.plist: `AudioServerPlugIn_IOKitUserClients = [com_openaudio_FWUserClient]`.
- **Devices.** Watch `com_openaudio_FWAudioDevice` with IOKit matching notifications and publish one input-only device per unit.
  - Name "<model> Glue", UID `com.openaudio.glue.<GUID>`.
  - Channels follow the unit's mode: on a change, `RequestDeviceConfigurationChange`, then remap on generation change.
  - The rate is read-only and follows the unit; rate changes stay with the FW1814 device (R57).
- **Clock.** `GetZeroTimeStamp` comes from the glue page: period 8192 frames, the same timestamps as the FW1814 engine. Glue
  reports the FW1814's clock domain from phase 0.
- **ReadInput:**
  1. unpack AM824 with the map (with delay history);
  2. scale by 2^-23 (`FWSamples.cpp:21-25`);
  3. run the chain per channel and write interleaved float.

  An underrun or a generation change outputs silence and resyncs.
- **Settings.** Custom properties on the device, as CFPropertyList:
  - stage presets;
  - per-channel {threshold, 4 × (freq, gain)};
  - a bypass;
  - a read-only gain-reduction array for meters.

  They are persisted per GUID with the host's `WriteToStorage` / `CopyFromStorage`, and changes reach the audio thread through a
  lock-free double buffer.

## Dual — two FW1814s, all 16 analog inputs (user, 2026-10-28; G9)
The user's target setup is the panel's Dual mode: two FW1814s as one aggregate, "Dual FireWire 1814", 32 in / 24 out at 44.1
or 48 kHz. Unit A is the clock master; B is locked to A's word clock; drift correction is off.

- **One Glue device per unit, as planned.** Each Glue device reports its own unit's timestamps and clock domain (G5, verified
  G9).
- **A second aggregate, "Dual FireWire 1814 Glue"**, made by the panel's Dual window the same way it makes the Dual aggregate
  (`DualAggregate.create`). Sub-devices, in this order:
  1. Glue A
  2. Glue B
  3. FW1814 A
  4. FW1814 B

  Clock master FW1814 A; drift correction off on all four.
  - Result: inputs 1–32 are the processed A + B inputs, with exactly the Dual channel numbers (B after A).
  - Inputs 33–64 are the plain A + B inputs, a raw safety copy that can be recorded alongside, or ignored.
  - Outputs 1–24 are A then B, as in Dual.
  - The plain "Dual FireWire 1814" aggregate stays as it is.
- **Real-time load (phase 3 requirement).** In an aggregate the HAL calls both Glue devices on the aggregate's one IO thread:
  32 channels ≈ 86 % of one core on the build PC (`glue-bench`: 2.70 % per channel at 48 kHz). The plug-in therefore
  processes its channels on worker threads (real-time, one per core), with the IO thread waiting for them each cycle.
  - The output is identical to single-threaded processing, so quality is unchanged.
  - The bench gets a threaded mode to measure the real per-cycle time at 64-frame buffers.
- **Hardware session** (phases 2+3): run on the build PC with both units, Dual on.

## Phase 4 — the Glue app (standalone; user, 2026-10-28)
> **Correction (user, 2026-10-28):** "this panel is independent of the control panel, it is a different thing." The Glue app is its
> own application, `openaudio-glue/panel/` → `OpenAudio Glue.app`; the main control panel is not changed. It looks like the OA MAFW
> panels by compiling the main panel's style sources unmodified (`openaudio-mafw-bebob/control-panel/Sources/Theme.swift`,
> `Widgets.swift`, `FaderLaw.swift`, `MaKeys.swift`). It is plug-in-simple (user rules above).

- **Layout:** "a matrix with 6 channels per line" (user).
  - **Header:** the unit selector (one tab per Glue device, A/B when there are two), the unit's bypass, its state (running or
    idle, rate, Glue latency), and, with two units, "Create Dual Glue aggregate" (see "Dual").
  - **Global card:** the four stage selectors as segmented bars. Transformer, Tube and Tape: Off / Soft / Medium / High / Full.
    Desk: Off / Neve M / Neve H / API M / API H / SSL M / SSL H.
  - **Channel matrix:** 6 cards per row (16 channels = 6 + 6 + 4). Each card has:
    1. a header: number, label (the device's channel name, e.g. "Input Analog 1"; double-click to rename, stored per unit),
       routing (Glue device input n and, with Dual, the aggregate's input number), and the ON pill;
    2. the compressor: Source (a menu: General, Drums, Vocals, Heavy guitar, Bass, Synth), threshold (a slider −40…0 dB,
       0 = off) and the gain-reduction meter (and the hit lift for Drums);
    3. the EQ: an XY graph (20 Hz–20 kHz log × ±12 dB), with the response curve and 4 draggable dots (low shelf, low-mid,
       high-mid, high shelf); double-click a dot to reset it to 0 dB.
- **Names and stereo link (user, 2026-10-28):**
  - Cards rename by double-click or right-click "Rename…"; "Use the Device's Name" goes back.
  - A **Names** menu applies standard names:
    - *Band, 16 inputs*: Kick, Snare Top, Snare Bottom, Hi-Hat, Tom 1, Floor Tom, OH L, OH R, then 2 bass, 3 guitars,
      3 vocals;
    - *Band, 32 inputs (Dual)*: the 16 most important sources on the 16 analog inputs (A 1–8 the kit close mics; B 1–8
      overheads, bass, guitars, lead vocal and first backing vocal), the rest on ADAT.
  - By default the templates also set each channel's Source, on/off and stereo links. A menu switch turns that off, leaving
    names only.
  - **Stereo link** per pair (1–2, 3–4 …), switchable with LINK on the odd channel: the even channel follows the odd one's EQ,
    compressor and on/off.
  - The DSP links the compressors: each side detects, and the deeper reduction and the larger hit lift go to both
    (`Comp::processLinked`, `ChannelChain::processPair`); the driver processes the pair together.
- **Backend:** Core Audio. Glue devices are found by UID prefix `com.openaudio.glue:`; settings are read and written as the
  `glst` property (the whole plist, throttled to ≤ 30 Hz while dragging); meters come from `glmt` at 15 Hz; the app follows
  device-list changes.
- **Curve maths:** `GlueEQMath.swift` uses the same formulas as `dsp/GlueEQ.cpp`, with host tests against values computed by the
  C++.
- **Demo mode:** `--demo` (two simulated 16-channel units) and `--snapshot DIR` (PNG of the window) for checking the layout
  without hardware.
- **Build:** Swift 4.1, Xcode 9.4.1, 10.13 SDK on the build PC; Swift runtime bundled (`scripts/bundle-swift-runtime.sh`).
- **Opened from the main panel (user, 2026-10-28: "like the firmware tool: can be opened from the panel, but is an autonomous
  thing").** The main control panel gets one menu item that launches `OpenAudio Glue.app` when it is installed. That is its only
  change, made after the app works (it is a main-project hook, logged in the main R-log).

## Phase 5 — installer and docs (inside `openaudio-glue/`)
- **Its own package**, `openaudio-glue/installer/make-pkg.sh` → `OpenAudio Glue.pkg`, built like the main one: on the build PC
  over SSH, root:wheel, `check-self-contained.sh` passes.
  - It installs `OpenAudioGlue.driver` into `/Library/Audio/Plug-Ins/HAL` and restarts `coreaudiod` (audio drops for a moment;
    the welcome page says so).
  - Its preinstall checks that the installed OpenAudio driver has the glue tap (glue page version), and otherwise stops with a
    message.
  - It has its own `uninstall.sh` and never touches the main package's files.
- **Main package:** only the kext and panel hooks change, so it gets a VERSION / CHANGELOG entry when the user decides to
  release them. The main README gets one line pointing to the spin-off, at merge time.
- **Docs:** `openaudio-glue/README.md` (what it is, setup: the aggregate device, presets, limits), CHANGELOG, STATE, NOTICE.
- `/close-item` after each phase: Glue evidence goes in `openaudio-glue/docs/PROVENANCE_v1.md`; the hooks also go in the main
  R-log.

## Verification
- **Phase 1:** the host test binary passes; port-vs-original diffs within tolerance; CPU bench recorded (16 channels, dev Mac).
- **Phase 2:** `kextutil -t -n` passes for both editions; the M-Audio MIDI plug-in path (selectors 500–507) is unchanged by
  diff review.
- **Phase 3:** `plutil -lint`; the plug-in builds on the build PC; `check-self-contained.sh` passes.
- **Phase 4:** `make -C control-panel typecheck test app` on the build PC; `--demo --advanced --snapshot` PNGs of Console with
  the GLUE column and the EQ editor, inspected before reporting.
  > **Correction (2026-11-02):** with the standalone app, phase 4 is checked with the app's own tests and `--demo --snapshot`
  > (`openaudio-glue/panel/`), as done in phase 4; the main panel is not built for Glue.
- **Hardware:** only when the user asks (R123), on an idle engine, after warning that a kext panic reboots the machine:
  - install the `.pkg`;
  - make the aggregate;
  - record a sine and program material through Glue with all stages Off: the result must be bit-identical to the FW1814
    device (the bypass test);
  - then by ear with the presets;
  - check `looplat` latency is unchanged.
