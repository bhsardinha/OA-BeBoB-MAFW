# A1 Ghidra overview — kext structural pass

**Status:** done (first full pass). Supplements the hand-decoded Q1-Q12 queue in `README.md` with Ghidra's class/function/decompiler view.

**Inputs:** `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` imported into `ghidra-project/fw1814-kext` on 2026-10-06 (Ghidra 12.1.4, headless, default analysis).

**Outputs under `ghidra-exports/`:**

| File | What | Size |
|---|---|---|
| `functions.tsv` | 2862 functions — address, size, demangled name, prototype, call-depth | 371 KB |
| `classes.md` | 222 classes grouped by namespace, with per-method listings | 141 KB |
| `decomp/*.c` | 80 decompiled pseudo-C files for the Q1-Q12 target set + lifecycle anchors | 664 KB total |
| `decomp/_INDEX.md` | hit/miss index for the decomp set | — |

## Headline numbers

- **2862 functions** in the kext. Hand-decoded Q1-Q12 touched ~90 of them.
- **222 distinct classes** (not counting `<EXTERNAL>` import stubs). The reference bundle is far more deeply layered than our clone — see hierarchy below.
- **80 / 98 decompile targets hit.** All 18 misses are explained by naming (methods live on a base class with a slightly different name than I guessed) and are tracked below.

## Class hierarchy — what Ghidra confirms

The reference uses a 3-prefix naming scheme:

| Prefix | Role | Examples |
|---|---|---|
| `com_m_audio_` | IOKit-published classes (match personalities, user-visible IORegistry entries) | `com_m_audio_FW1814Device`, `com_m_audio_FWEngine2`, `com_m_audio_FWMetaNub`, `com_m_audio_FWUserClient`, `com_m_audio_MakeAppleFWAudioGoAway` |
| `m_audio_b_` | Internal framework base classes (the "b" = BeBoB framework) | `m_audio_b_FWAudioDevice`, `m_audio_b_FWBaseEngine`, `m_audio_b_FWDCLProgram`, `m_audio_b_FWHSCI`, `m_audio_b_FWAVCAudioDevice`, `m_audio_b_FWDCLBuilder` |
| `m_audio_FWb::` | Internal utility templates / container classes | `TRingBuffer`, `TPacketPool`, `TFunctor<…>`, `TPTMFFunctor…`, `BaseChannelGroup`, `ConcreteChannelGroup<N>` |

### Engine inheritance — new finding not visible in `otool`

Our clone collapses everything into one `FW1814Engine`. The reference has a **4-way engine class family**:

```
m_audio_b_FWBaseEngine          (72 methods — top of chain, holds IOAudioEngine wiring)
├── com_m_audio_FWEngine2         (36 methods — primary FW1814/Audiophile engine; direct subclass)
├── m_audio_b_FWDiscreteEngine    (31 methods — "all pairs" secondary engine)
├── m_audio_b_FWPairEngine        (21 methods — single-pair proxy → shared Discrete via [+0x118])
├── m_audio_b_FWAggregateEngine   (19 methods — aggregate-device membership)
└── m_audio_b_FWTimeSlaveEngine   (13 methods — slave-clock aggregate member)
```

**Correction (2026-10-06, Q13):** `FWEngine2` is a *direct* subclass of `FWBaseEngine`, not of Discrete/Pair/TimeSlave. Those are sibling classes — secondary IOAudioEngines that CoreAudio sees as independent audio devices (one per channel-pair) and that forward their clip/convert back to a shared engine via `[+0x118]`. See `Q13-engine-family.md` for the full decode. The FW1814 publishes one primary FWEngine2 plus N secondary pair engines.

Clone implication: we can keep the single-class collapse for M3-M5, but note that **AC3 pass-through branches off a Discrete-style engine**, not off the main PCM engine — see `m_audio_b_FWDiscreteEngine::clipOutputSamples` @ `0x188c6` vs the AC3 clip hook at `FWEngine2::ClipAC3Output` @ `0x11c44` (Q12).

### Device hierarchy

```
m_audio_b_FWAudioDevice           (176 methods — concrete FireWire BeBoB device base)
├── m_audio_b_FWAVCAudioDevice    (60 methods — AV/C transport layer)
├── m_audio_b_FWAggregateDevice   (15 methods)
├── m_audio_b_FWDiscreteDevice    (12 methods)
├── FWRev1AudioDevice             (10 methods — original BeBoB1 silicon)
├── FWRev2AudioDevice             (34 methods — BeBoB2 — the FW1814 generation)
├── com_m_audio_FW1814Device      (59 methods)   ← our target
├── com_m_audio_FWAudiophileDevice (29 methods)
├── com_m_audio_FW410Device       (34 methods)
├── com_m_audio_FWSoloDevice      (27 methods)
├── com_m_audio_FWLightbridgeDevice (38 methods)
├── com_m_audio_FWOzonicDevice    (28 methods)
├── com_m_audio_FWNRV10Device     (23 methods)
└── com_m_audio_FWProjectMixDevice (18 methods)
```

Nine products share this stack. The FW1814 overrides more than any other (59 methods), consistent with it being the flagship.

### Nub — confirms Q7

- **Only `com_m_audio_FWMetaNub` exists** (40 methods). No `FW1814Nub` / `FWAudiophileNub` / `FW1814Bootloader` as separate classes.
- The nub is a single polymorphic class that drives both bootloader-mode firmware upload *and* runtime matching, selected by `Model_ID` property. Our clone's 2-class split (`FW1814Nub` + `FW1814Bootloader`) is a legitimate 10.13.6-era simplification, but we should document that the reference used one class with internal state.

### Settings / property model

Not previously surfaced:

- `m_audio_b_FWPropertyDict` (34 methods) — kernel-side property dict distinct from `OSDictionary`.
- `m_audio_b_FWBaseProp` → `FWBaseArrayProp`, `FWNumberProp`, `FW32BitNumberProp`, `FW64BitNumberProp`, `FWFunctionProp`, `FWArrayProp`, `FWChannelGroupProperty` — a full typed-property framework published to CoreAudio controls.
- `FWSettings*` family (`FWSettings1814Levels`, `FWSettings1814AuxSend`, `FWSettings410Levels`, `FWSettingsAuxSend`, `FWSettingsLevels`, `FWSettingsABSwitch`, `FWSettingsRotaryControl`) — product-specific control objects.
- `m_audio_FWb::BaseChannelGroup` + `ConcreteChannelGroup<N>` for N∈{2,4,6,10,14,18,22} — strongly suggests mixer-channel arities exposed to CoreAudio. The 22-channel group is likely the full FW1814 output matrix (18 ADAT + 2 S/PDIF + 2 analog) and 18 is the FW1814 input matrix (8 analog + 8 ADAT + 2 S/PDIF).

This is live intel for M5 (mixer): the clone should mirror `BaseChannelGroup` + a `ConcreteChannelGroup<N>` template-style abstraction before building the mixer surface.

### Transport & timing primitives

- `m_audio_b_FWKernelTimer` (11 methods) — IOWorkLoop-based timer used by HSCI / FWMetaNub (Q6/Q7 confirmed 50 ms and 2500 ms periods).
- `m_audio_b_FWIsochChannel` (15 methods) — subclass of `IOFWIsochChannel`, wraps the DCL program.
- `m_audio_b_FWRemoteIsochPort` (14 methods) — remote-port side of the plug connect (Q3).
- `m_audio_b_FWPairStream` + `m_audio_b_FWStream` — stream descriptors inside the engine.
- `m_audio_b_FWTPacketIndex` + `m_audio_FWb::TPacketPool` — packet-ring bookkeeping.
- `m_audio_FWb::TRingBuffer` — generic ring buffer (likely MIDI-byte queue, see Q11).

### UserClient surface — confirms Q11

- `com_m_audio_FWUserClient` (31 methods) — matches Q11's 8-selector surface + lifecycle (clientClose, clientMemoryForType).
- `m_audio_b_FWKIPCUserClient` (21 methods) — a *second* UserClient class, previously unknown. Likely the Carbon panel's control-channel UC (K-IPC = Kernel IPC). **Not reachable from CoreAudio** — only from the panel. Worth decoding for M6 panel work.
- `m_audio_b_FWEngineUserClient` (8 methods) — thin wrapper, probably exposed by the audio engine for low-privilege clients.

## Decomp misses — explanations

The 18 misses are all naming-mismatch; functions exist under slightly different names:

| Target | Actual location | Notes |
|---|---|---|
| `FWDCLInputProgram::Initialize` | `m_audio_b_FWDCLProgram::Initialize` @ `0x27458` | Inherited from base |
| `FWDCLInputProgram::ReconnectAfterBusReset` | `m_audio_b_FWBaseEngine::ReconnectAfterBusReset` @ `0x394a` | 6-byte stub (Q4) |
| `FWDCLInputProgram::AddTimestampedTransferPackets` | `m_audio_b_FWDCLBuilder::AddTimestampedTransferPackets` @ `0x6ad8` | On the builder, not the program (matches Q1a decode) |
| `FWDCLProgram::Disassemble` | `m_audio_b_FWDCLBuilder::Disassemble` @ `0x683c` | Builder owns it |
| `FWHSCI::FWHSCIDeviceCallback` | **free function** `FWHSCIDeviceCallback` @ `0x1d78c` | 10-byte trampoline, not a method |
| `FW1814Device::start/init` | inherited from `IOAudioDevice::start` + `m_audio_b_FWAudioDevice::init` | 1814 doesn't override these |
| `FW1814Nub::probe/start/matchPropertyTable` | `com_m_audio_FWMetaNub` methods | Single polymorphic nub (Q7) |
| `FW1814Bootloader::probe` | same — `com_m_audio_FWMetaNub::probe` | Same class, Model_ID-switched |
| `FWEngine2::performAudioEngineStart/Stop` | `m_audio_b_FWBaseEngine::performAudioEngineStart/Stop` @ `0x556e` / `0x543c` | Base-engine impl |
| `FWEngine2::performFormatChange` | `m_audio_b_FWBaseEngine::performFormatChange` @ `0x3e54` | Base-engine impl |
| `FWEngine2::clipOutputSamples` | `m_audio_b_FWDiscreteEngine::clipOutputSamples` @ `0x188c6` | Discrete-engine impl (see engine hierarchy) |
| `FWEngine2::convertInputSamples` | `m_audio_b_FWDiscreteEngine::convertInputSamples` @ `0x18aa6` | Discrete-engine impl |
| `FWEngine2::ConvertSampleRate` | free function `ConvertSampleFramesFromInput` @ `0x27bb6` | Called by convert hooks |
| `FWAVCAudioDevice::IsAC3Supported` | `m_audio_b_FWAudioDevice::SupportsAC3` @ `0xaff6` | Already decoded in Q12 (returns 0) |

Follow-up: Q12a will re-decompile the correct symbols above now that we have their real addresses.

## What Ghidra added that `otool` didn't

1. **Full class hierarchy with method counts** — makes the engine-family split (Discrete/Aggregate/Pair/TimeSlave) obvious. Hand-reading x86 would have missed this.
2. **Named structure field inference** — the decomp `.c` files show stack-local vars with sizes (e.g. `undefined1 local_6c[20]` for the 16-byte HSCI envelope + alignment slop). Confirms Q5's 16-byte payload size.
3. **Call-graph depth per function** (TSV `call_depth` + `calls_count` columns) — lets us rank "deep orchestration" functions (SetupAudioEngine, initHardware) against "leaf" functions (clip/convert).
4. **Free-function discovery** — `FWHSCIDeviceCallback` is a C-ABI trampoline, not a method. Same pattern likely exists for other IOFWCommand completions; worth grepping.
5. **The second UserClient (`FWKIPCUserClient`)** — invisible in Q11's selector-table decode because it isn't reached through the standard CoreAudio UserClient path. This is the Carbon panel's back-channel and must be mapped before the AppKit panel is written.
6. **Settings / property / channel-group framework** (`FWBaseProp`, `BaseChannelGroup`, `ConcreteChannelGroup<N>`) — M5 mixer design needs to mirror this; the clone currently has nothing equivalent.

## What's still worth decoding (new follow-ups)

- **Q13** — the engine family: which base class does `FWEngine2` actually inherit from? Decompile `FWEngine2::MetaClass::alloc` + vtable to see inheritance edge. Determines whether the FW1814 is Discrete or Pair.
- **Q14** — `m_audio_b_FWKIPCUserClient` surface: 21 methods, selector table, data shape. Needed before the AppKit panel can replace the Carbon one.
- **Q15** — `FWSettings1814Levels` + `FWSettings1814AuxSend` + `ConcreteChannelGroup<N>`: this is the mixer model. Needed for M5.
- **Q16** — `FWRev2AudioDevice` vs `FWRev1AudioDevice`: what BeBoB-silicon-generation differences does the reference encode? FW1814 is Rev2; worth knowing which 34 methods differ.
- **Q17** — `TPacketPool::Initialize` @ `0xa3e6` (258 bytes) — the DCL packet-pool allocator. Clone's `FW1814DCLBuilder` currently open-codes allocation; aligning to this would make the DCL path look more like the reference.

## Resume

1. Fresh work starts with Q13 above (engine family inheritance), then Q14-Q17 in order of M-milestone pressure.
2. Rerun `ExportDecompilations.java` after the 18 miss corrections above are folded into the TARGETS list, to pick up the base-class implementations.
3. If the Ghidra project is lost, re-import with:
   ```
   /opt/local/share/java/ghidra-12.1.4/support/analyzeHeadless \
       docs/system-map/A1-ghidra/ghidra-project fw1814-kext \
       -import _extracted/kext/M-AudioFireWireBeBoB.x86_64-slice
   ```
   then rerun the three post-scripts under `scripts/`.

## Provenance

- All class method counts: `ghidra-exports/classes.md` (2026-10-06).
- All function addresses: `ghidra-exports/functions.tsv` (2026-10-06).
- Decomp bodies: `ghidra-exports/decomp/*.c` (2026-10-06).
- Ghidra version: 12.1.4 (MacPorts, `/opt/local/share/java/ghidra-12.1.4`).
- Analysis settings: default MachoLoader + full auto-analysis; no manual type imports or signature overrides. 63 seconds of auto-analysis per the import log.
