# BUILD_AUDIOPHILE_PLAN — adding the M-Audio FireWire Audiophile (and leaving the door open for every FW device of the official driver)

Status: **DRAFT v2 for the user's review** (written 2026-10-19, revised the same day). No code has been written. **The Audiophile is now connected to the SL rig** (official v1.9 driver; checked 2026-10-19 over SSH: `ioreg` shows `FW Audiophile`, Model_ID 65632 = `0x10060`, engine idle) and three captures exist: 89 (device subtree), 89b (FireWire node), 90 (266 panel `setProperties` writes). v2 adds those facts (§2), an SL capture task (AP-0.9), the airliner sequencing rule (§4 Phase 7), a step-by-step order (§11) and effort per model for the full official list (Appendix A). Needs the user's answers in §9 before phase 1; phase 0 and AP-0.9 can start now.

Inputs (read these first): `docs/audiophile-manual-reference.md` (facts, with provenance and open items), `docs/audiophile-manual.md` (the manual's text), `docs/manual-reference.md` (FW1814 counterpart), `CLAUDE.md` (working rules). Evidence tags below follow the project convention: *kext @addr*, *man p.N*, *ref §N* = the Audiophile reference doc.

## 0. Ground rules (in force for the whole plan)

1. **New code goes in a new folder `openaudio-mafw-bebob/` at the repository root, next to `openaudio-fw1814/`. `openaudio-fw1814/` is never edited by this work** (user instruction 2026-10-19; memory `project_multimodel_new_folder`). Code is *copied* from it; fixes that also belong in the released 1814 tree are a separate decision by the user.
2. **Mirror the reference, don't invent.** The 2014 kext (v1.11.2, `_extracted/`) is the spec: class chain, AV/C commands, plug maps, tables. Deviations only when 10.13.6 forces them, each with a comment naming what forced it. Unverified facts are labelled and carry the check that settles them (`audiophile-manual-reference.md` §8 lists 10).
3. **Homebrew, always unsigned, SIP off** (R73). No signing, notarisation or SIP-on work.
4. **Hardware etiquette** (CLAUDE.md): ask before loading/unloading the kext or switching anything; test only when the engine is idle; **never ask for FireWire hot-plug** — power-cycle the Audiophile (the manual says the same, *man p.5-6*); never flash firmware; stage the kext in `/tmp` and run `kextutil -t -n` first; a kernel panic reboots the user's Mac — say so before risky tests.
5. **Close-out rule**: after each completed item run `/close-item` (evidence → `docs/system-map/`, `PROVENANCE-RESOLUTIONS.md` next free `R<n>`, tracker, STATE/NEXT_STEPS). For this phase the tracker is **`PROVENANCE-MATRIX-AUDIOPHILE.xlsx`** (Itens/Build/Flags/Painel), not the 1814 workbook. Report outcomes plainly; label what was not verified.
6. **Public repository**: nothing is pushed or copied to `../openaudio-fw1814-public` without the user's go-ahead; M-Audio's binaries/firmware/PDFs stay out of it (`docs/audiophile manual.pdf` and `FWAP.bcd` are private-only).
7. **Method lesson (user's standing instruction):** when a symptom looks like a driver bug, decompile the same function in the official driver and diff the behaviour before theorising.

## 1. Goal, scope, non-goals

**Goal.** A single multi-model driver stack (kext, helper, panel, MIDI plug-in, installer) for macOS 10.13.6 that supports the **M-Audio FireWire 1814 (unchanged behaviour) and the FireWire Audiophile**, structured so that the remaining official-driver models can be added by data plus a small per-model class (Appendix A).

**In scope (Audiophile, what the reference does):** enumeration + firmware start (bootloader → runtime, Model_ID `0x00010060`), 4-in/6-out audio at 44.1/48/88.2/96 kHz, S/PDIF in/out incl. external sync, MIDI 1 in/1 out, the AV/C function-block mixer/router (10×6 mixer, 10×2 aux, main/aux selectors, headphone source selector), headphone/Level-Controller encoders and the A/B switch with its LEDs, mute/dim, peak meters, per-GUID settings persistence, AC-3 pass-through as in the reference, panel with the Audiophile's Mixer/Output/Hardware/About pages.

**Non-goals.** Firmware flashing (never); signing/SIP-on; Windows-only features (ASIO, WDM buffer size, bit-depth); the Carbon panel and preference pane; other models (only the design hooks, Appendix A); multi-unit chaining beyond what falls out of the structure (the manual itself says one unit, *man p.35*); changes to the released `openaudio-fw1814/`.

## 2. What is known, and what that means for the work

From the reference doc (each item has its evidence there):

| Finding | Consequence |
|---|---|
| Audiophile is a **"normal" BeBoB device mixed through standard AV/C**; no HSCI, no `0xFFC7:0x00700000` window, no vendor clock command | The 1814's mixer stack (`FWHsci`, `FWMixer`, `FWMixerRegs`, `FWPropertyDict` batch writer, HSCI clock bits) **does not apply**; a second mixer backend (AV/C function blocks) is needed. This is the largest new piece |
| Class chain: **`FWAudiophileDevice : FWRev2AudioDevice`**; the FW1814 is `FWRev1AudioDevice` (ctor @0xbe6c vs `FW1814Device::C2`) | The clone has Rev1 only (`com_openaudio_FWRev1AudioDevice`). **A Rev2 base must be written**: extended AV/C stream-format command (`0x2F`) vs plain `0x18/0x19`, `AVCSetupSyncStream`, `GetAddressAndPlugForSyncType`, `ReadHSCIData`/`UpdatePeakMeters`, `Start/Stop/Pause/ResumeReceptionOfControlPackets`, `initHardware`, `AllocateAudioEngine`, `GetRoundTripLatencyForFDF` (the 34-method class, *kext* symbol list) |
| 4 in / 6 out at every rate, 1 MIDI quadlet, rates 44.1/48/88.2/96, no modes | Per-model channel table instead of the 1814's rate+mode tables; **no `ResetRearrangementProcs` by mode**; fixed converters `_ConvertLLRR4SkipMIDI1`, `_RearrangeSampleFrameLLRR6`, `_SamplePeaks6` (the clone's `FWSamples.cpp` needs these three; they are small, decoded in ref §3.3) |
| Latency constants 182/196/330/354 frames (44.1/48/88.2/96) | Per-model virtual `GetRoundTripLatencyForFDF` (the clone has the 1814's) |
| 60-byte status block at `0xFFC7:0x00600000` with 14 peaks + one status quadlet (sync flag, rate code, two encoder nibbles, switch bits) | New small reader class; reuse the 52-slot shared peak page (Audiophile fills 14 + 6) |
| Plugs 2 oPCR / 2 iPCR | CMP/plug code must not assume the 1814's 3 iPCR (check `FWCMP`, `FWAvc` for hard-coded plug numbers) |
| `FWAP.bcd` differs from `fw1814.bcd` (header `0x78`, load base `0x20080000`, padded to `0x115000`, all CRCs verified) | The helper's firmware reader must use the offsets in the file, not "end of file" or a fixed header |
| Reference device table `kDeviceSpecs @0x46450`: 8 entries `{product name, bootloader name, class}`; the nub matches **by product-name string**, not Model_ID (`"FW Audiophile"` / `"FW Audiophile Bootloader"`) | The nub's table (today 1 entry) becomes the 8-entry table; the bootloader/runtime split uses the same names |
| Port ↔ wire-slot order is **derived** (LLRR) and conflicts with FFADO's `fwap.xml` | A loopback check on hardware is a gate (task AP-7.3) |
| **SL v1.9 ioreg (capture 89):** device class `com_m_audio_FWAudiophileDevice` under `com_m_audio_FWMetaNub`; one `com_m_audio_FWEngine2` ("FireWire Audiophile Multichannel"); output stream 6 ch (Analog 1-4, S/PDIF 1-2), input stream 4 ch (Analog 1-2, S/PDIF 1-2), 24-bit in 32, rates 44.1/48/88.2/96; **AC-3 output formats** (2 ch, 16-bit, `ac-3`, 1536 frames) at 44.1/48/96 only; `IOAudioEngineOutputSampleLatency` 210, `InputSampleLatency` 32, `SampleOffset` 48 at 48 kHz; buffer 8192 frames; `MABytesPerCycle` 400; one MIDI in/out port; `IOAudioDeviceTransportType` = `1394` | Golden registry for AP-3.1 and AP-7.2; the AC-3 list settles part of AP-3.8 (the Audiophile *does* advertise AC-3 on v1.9 — still to check in v1.11.2, version-skew rule). Channel *names* are not the wire order (AP-7.3 stays) |
| **SL v1.9 panel writes (capture 90):** 14 `MA*` keys only — `MAInputRoutingV2`, `MAInputPan2`, `MAAuxSendLevels`, `MASWReturnLevels`, `MAHeadphoneSourceV2`, `MAMomentarySwitchRoutings`, `MAMomentarySwitchSource`, `MASWReturnMute`, `MASWReturnSolo`, `MASWReturnRoutingV2`, `MAMonitorAuxOnOutput`, `MAClockSource` (`0x40000001` seen), `MAGlobalMute`, `MAGlobalDim` | Starting point of AP-0.6. `MASWReturnMute`/`MASWReturnSolo` have **no handler in the 1814 clone's kext** (grep 2026-10-19), so they are new kext work. The capture mixes many controls in one session; per-control values still need AP-0.9 |
| `snd-bebob` `bebob_maudio.c:26-28`: "Without streaming, the devices except for Firewire Audiophile can mix any input and output … Audiophile cannot be used as standalone mixer" | **Third-party claim, unverified.** If true, the panel's mixer is silent unless the engine streams; the reference's behaviour (does it keep TX running when idle, as the 1814's continuous idle TX does?) must be read in AP-0.2 and observed in AP-0.9 |
| SL DTrace limit (`B5 README §12`): on 10.6.8 `fbt` instruments `mach_kernel` only, so the **FCP bytes the official kext sends cannot be captured** on the rig | AP-0.3 (mixer bytes) stays a static-decompile task, cross-checked against `ctl-services` `normal.rs` (its Audiophile FB table is present, lines 474-570). The rig gives the panel→kext layer and audible behaviour only |

**Reuse from `openaudio-fw1814/kext/src` (copy, then keep identical unless noted):** isoch and DCL stack (`FWDCLProgram`, `FWDCLBuilder`, `TPacketPool`, `FWCip`, `FWIsochChannel`, `FWCMP`, `FWBusReset`, `FWStreamControl`, `FWSharedSync`), engine family (`FWBaseEngine`, `FWEngine2`, `FWDiscreteEngine`, `FWPairEngine`, `FWTimeSlaveEngine`, `FWStream`), MIDI (`FWMidi`, `FWEngineMidi`), `FWAvc` (AV/C unit and async queue), `FWMetaNub` (table + bootloader state machine), user-client base (`FWKIPCUserClient`), `FWChannelGroup`, `FWFunctor`, `MakeAppleFWAudioGoAway`. **Per-model / new:** device classes (`FWAudioDeviceChain`, `FWAudioDeviceProps` split), `FWRates.h` tables, `FWSamples` converters, mixer backends (1814 = existing; Audiophile = new), user-client peak layout, `FWSettings` (the 1814's levels objects).

## 3. Architecture of `openaudio-mafw-bebob/`

```
openaudio-mafw-bebob/
  kext/                 one kext, all models (bundle id new, see §9 Q2)
    src/core/           copied unchanged from the 1814 tree: isoch, DCL, engines, MIDI, AV/C base, nub
    src/models/         ModelTable.cpp (kDeviceSpecs-style), ModelDescriptor.h
    src/models/fw1814/  1814 device class + HSCI + mixer registers (moved, behaviour unchanged)
    src/models/fwap/    Audiophile device class (Rev2), AV/C mixer backend, status-block reader
    src/rev1/ src/rev2/ the two AV/C bases
  firmware-helper/      product table + per-model .bcd reader
  control-panel/        model descriptors drive layout (§6), one app
  installer/            pkg + uninstaller + dmg (new product names)
  tools/ scripts/ tests/
```

* **Model descriptor (data, in the kext and mirrored in the panel):** product names (runtime + bootloader), device class, AV/C revision (Rev1/Rev2), mixer backend (`direct-write` | `avc-fb`), per-rate channel table, rate list, MIDI quadlets, converters, round-trip latency per FDF, status-packet size/layout, peak map, icon, firmware file. The Audiophile row is filled from ref §1-§4; the 1814 row from the existing code. **This is the extension point for Appendix A.**
* **Backend interface (kext):** `MixerBackend { Init; ApplyAll(state); SetLevel/Mute/Solo/Pan/Route/Aux/Selector(...); ResendCurrent; ResetToFactory; }` implemented by the 1814's `FWMixer` (unchanged logic, moved) and by a new `FWAvcMixer` (function-block commands). The panel-facing `MA*` property keys stay the single control plane (`IORegistryEntrySetCFProperties`); the Audiophile's subset is derived in AP-0.6.
* **Coexistence rule:** the new kext replaces the 1814-only kext (same device personalities). Installing both is unsupported; the new installer removes `com.openaudio.OpenAudioFW1814` first (§7). The 1814 path in the new tree must pass the **same hardware checks** as release 0.1.0 before it ships (AP-8).
* **Out of this tree:** nothing is deleted or edited in `openaudio-fw1814/`.

## 4. Phases, tasks, acceptance

Task ids `AP-n.m`. "Gate" = needs the user at the machine / a decision. Status of every task today: not started.

### Phase 0 — static research (no hardware; fills the 10 open items of the reference doc)
| Id | Task | Output / acceptance |
|---|---|---|
| AP-0.1 | Decompile `FWMetaNub::probe` + `GetActualProductName` + `kDeviceSpecs` consumers in the reference; confirm Audiophile selection and bootloader/runtime transitions (ref §8.2) | note in `system-map/`; clone nub design matches |
| AP-0.2 | Decompile the whole `FWRev2AudioDevice` (34 methods) + `FWRev1` for diff: `AVCControlPlugSignalFormat` (0x2F layout), `AVCSetupSyncStream`, `GetAddressAndPlugForSyncType`, `initHardware`, `AllocateAudioEngine`, control-packet start/stop | `Q-REV2-*.md` with byte layouts; table "Rev1 vs Rev2" |
| AP-0.3 | Decompile the settings classes used by the Audiophile (`FWSettingsABSwitch`, `FWSettingsRotaryControl`, `FWSettingsAuxSend`, `FWSettingsLevels`, `TRotaryControl::RotaryValueChanged`) and `AVCControlFunctionBlock` callers: exact AV/C bytes (FB type, id, control attribute, value encoding, ranges, pan law), initial state, send order | `Q-FWAP-avc-mixer.md`; cross-check with `ctl-services` `normal.rs` and FFADO `normal_avdevice.cpp`; resolves ref §8.3, §8.5 |
| AP-0.4 | Decode `AVCVendorCommand` LED call (company id/opcode) and the integer argument of `AVCSetupSyncStream` (ref §8.6) | note |
| AP-0.5 | Decode the full `FWAudiophileDevice` object layout and `$_667` channel groups (levels/mutes/solos/pans) | layout table |
| AP-0.6 | Derive the Audiophile `MA*` key subset (diff against `FW410Device::InitializeUserSettableProperties`, 1814 catalog `PN2-control-key-map.md`) and the panel's `CXAudiophileDevice` view (ranges, labels) | `PNAP-control-key-map.md`; decompile `CXAudiophileDevice` in the panel binary |
| AP-0.7 | Read the converters and the peak proc (`_SamplePeaks6`) in full; confirm the 6-channel peak mapping and clamp/label behaviour | note; ready-to-port C |
| AP-0.8 | File all decompiles under `docs/system-map/A1-ghidra/ghidra-exports/decomp/` (the 7 already done are in `audiophile-overrides-2026-10-19.txt`); `/close-item` for captures 89/89b/90 too | R-entries, tracker rows |
| AP-0.9 | **SL rig captures while the Audiophile is attached** (gate: the user at the panel; engine idle; no rate change while a client streams; restore 48 kHz): (a) one control at a time, every panel control, `setProperties` raw capture with the UI value noted (extends capture 90 into a per-control table); (b) `ioreg` device subtree at 44.1/48/88.2/96 (latency, formats); (c) per-GUID settings plist after "factory reset" = the reference defaults; (d) KIPC notifications for the A/B switch and both encoders (capture-64 method); (e) meter shared page with a signal on each input (capture-70 method); (f) audible check of the snd-bebob "no mixing without streaming" claim (signal on input 1, headphones, no client running); (g) firmware info (`MAFirmwareInfo` already in 89) | numbered captures 91+ in `B5-sl-dtrace-truth/captures/`; `PNAP-control-key-map.md` filled; ref §8 items 4, 7, 8 partly closed |

Acceptance of phase 0: ref doc §8 items 2, 3, 4, 5, 6, 10 closed or re-labelled with a reason; 1, 7, 8, 9 remain hardware items (7 and 8 partly answered by AP-0.9). Version-skew rule: every AP-0.9 finding is v1.9; the spec is v1.11.2, so each is cross-checked against the decompile before it drives code.

### Phase 1 — bootstrap the new tree (no behaviour change)
| Id | Task | Acceptance |
|---|---|---|
| AP-1.1 | Create `openaudio-mafw-bebob/` by copying the 1814 tree (kext, helper, panel, installer, tools, scripts, tests); rename identifiers to the new product (bundle ids, class prefixes stay `com_openaudio_` — **decision Q2**) | tree builds on the build PC; `git diff` shows no change inside `openaudio-fw1814/` |
| AP-1.2 | Scripts parameterised for the new tree: `sync-to-buildpc.sh` → `~/oa-build-mafw` (never `~/oa-build`), `stage-kext.sh`, `load-test-kext.sh`, `install.sh`/`uninstall.sh`, `conformance-check.sh`, `check-self-contained.sh` | each runs against the new tree; the old build dir on the iMac is untouched |
| AP-1.3 | Build + `kextutil -t -n` stage on the iMac; **no load** | "appears to be loadable" |
| AP-1.4 | Prove the copy is faithful: load the unmodified copy on the Air (test kext in `/tmp`, user present), `loopcheck` 34 s — same result as capture 88 | gate: user OK to load |

### Phase 2 — kext refactor to model-agnostic core (1814 behaviour unchanged)
| Id | Task | Acceptance |
|---|---|---|
| AP-2.1 | `ModelDescriptor` + `ModelTable` with 8 product-name entries (the 7 non-1814 map to "no class yet" until implemented); `FWMetaNub` uses it; bootloader names from the table | with only the 1814 implemented, behaviour identical (registry diff of the device tree against capture) |
| AP-2.2 | Introduce the backend interface and move the 1814 mixer behind it; per-model virtuals for channels, rates, latency, converters, peak map, status size | 1814 host tests (`make test`) and conformance script pass; `ioreg` property parity with the 0.1.0 run |
| AP-2.3 | Write `com_openaudio_FWRev2AudioDevice` from AP-0.2 | builds; unit-level tests on command encoders (golden bytes from the decompile) |
| AP-2.4 | Per-model user-client layout (peak page: Audiophile 14 + 6 in the 52-slot page; notification dictionary) | panel can read both |

### Phase 3 — Audiophile kext
| Id | Task | Acceptance |
|---|---|---|
| AP-3.1 | `FWAudiophileDevice : FWRev2AudioDevice`: names/icon strings, rates, `sDeviceChannels` (4/4/6/6/1), `SetupAudioEngine`, latency table, `IndexForAC3OutputChannels (2,5)`, `GetFirstSWReturnPeakMeterIndex 14` | host tests against the tables in ref §3 |
| AP-3.2 | Converters/peak procs (`FWSamples`): `LLRR4SkipMIDI1` in, `LLRR6` out (clamp ±24 bit, label `0x40`, permutation `0,2,4,1,3,5`), `SamplePeaks6` | host test: golden frames; later hardware gate AP-7.3 |
| AP-3.3 | Plug/format bring-up with Rev2 commands for 2 oPCR / 2 iPCR; CMP; sync (internal/S-PDIF via `AVCSetupSyncStream`); stream kick sequence **to be taken from the reference decompile, not from the 1814** (memory: the 1814 needs a kick after CMP — whether the Audiophile does is unknown) | stages and logs; hardware gate |
| AP-3.4 | Status-block reader: 60-byte read at `0xFFC7:0x00600000`, polling policy from AP-0.2/0.3, peaks, sync flag → `DetectExternalSyncChanges`, encoders → settings, switch → ABSwitch | decoded values match `ctl-services` fields |
| AP-3.5 | `FWAvcMixer` backend: levels, balance, mute, solo (software), routing matrix (15-entry table), aux sends, main/aux selectors, headphone selector (`SetHeadphoneSelector`), mute/dim, factory reset/resend; read-back avoided (ASIC times out, ref §4.1) | golden-byte tests from AP-0.3; AV/C command rate-limited (async queue already exists) |
| AP-3.6 | LED/ABSwitch behaviour: HP-source A/B and Direct-Monitoring mode, LED vendor command; `momentary switch` property | host test with the state machine |
| AP-3.7 | Defaults per *man p.55* (output faders −6 dB etc.) and `ResetToFactorySettings` | conformance entry |
| AP-3.8 | AC-3 path: confirm the reference's `AC3Activated` for the Audiophile (the 410's is a no-op, Audiophile's list shows none; ref `IndexForAC3OutputChannels`) | decision recorded |
| AP-3.9 | Diagnostics ring (`MADebugTrace` pattern) for the Audiophile paths; **no `IOLog` flood**; teardown safe with async AV/C in flight (lesson R-kextunload) | `kextunload` x5 on the Air |

### Phase 4 — helper
| Id | Task | Acceptance |
|---|---|---|
| ~~AP-4.1~~ | **Cancelled 2026-10-19 (user: scope cut — the driver never updates firmware, the helper never reads a `.bcd`; HAP §2).** Product table in the helper (names, firmware file) and a `.bcd` reader that honours header/offsets/CRCs of `FWAP.bcd` (header `0x78`, base `0x20080000`, padding) | host test on both `.bcd` files; CRC pass |
| AP-4.2 | Firmware bring-up via the kext's `MAFirmware*` path for the Audiophile (bootloader named `"FW Audiophile Bootloader"`, same Model_ID) | gate: power-cycle, observe bootloader → runtime |
| AP-4.3 | Per-GUID settings persistence works for a second model (key = GUID already); replay after a kext reload | like the 1814 test (R57) |
| ~~AP-4.4~~ | **Cancelled 2026-10-19 (user: scope cut, as AP-4.1).** Firmware files ship in the installer for each supported model (private-only repo content; public repo has none) | installer check |

### Phase 5 — panel
| Id | Task | Acceptance |
|---|---|---|
| AP-5.1 | Model descriptor in the panel (channel layout, strip set, tabs, hardware options, default values, icon); `DeviceModel`/`ChannelLayout` become data-driven; the 1814 descriptor reproduces today's UI | `--demo --snapshot` PNG for the 1814 equals the released panel |
| AP-5.2 | Audiophile descriptor and views **in the layout and style of the new 1814 panel** (user decision 2026-10-19: reuse its tabs, components and look; the official screens of capture 91 are only the checklist of controls): Mixer (3 sw returns + analog in + S/PDIF in, pan, aux L/R, `1/2 3/4 spd` routing), Output (3 pairs `main/aux`, aux master, phones with source buttons), Hardware (sync internal/external, A/B sets, momentary function), About | snapshots approved by the user (quality bar: memory `feedback_panel_ui_quality`); every control of capture 91 present |
| AP-5.3 | Level-Controller assignment and `ctrl` buttons, mute/dim, presets (save/load/delete) for the Audiophile | host tests |
| AP-5.4 | Device menu with mixed models; per-GUID state | panel with 1814 only and with Audiophile only both launch |

### Phase 6 — MIDI
| Id | Task | Acceptance |
|---|---|---|
| AP-6.1 | The reference CoreMIDI plug-in is reused unmodified (as for the 1814); confirm it names/handles the Audiophile's single port | `midiprobe` lists the port |
| AP-6.2 | DIN loopback (gate: cable) + SysEx round trip, the same open test as the 1814's M-H | capture |

### Phase 7 — hardware ladder (Audiophile; the user present; engine idle; no hot-plug)

**Sequencing (airliner rule, user directive 2026-10-15):** phases 2-6 are verified by conformance (`docs/CONFORMANCE.md` gets an Audiophile section; `conformance-check.sh` iterates the model descriptors), host tests and `kextutil -t -n` only. **No hardware load of a partial Audiophile build.** Phase 7 is one integrated run once kext, helper, panel and MIDI all exist; its checks run in the order below in the same session block, and a failure goes back to the static side (decompile-and-diff first, the standing method lesson) before the next load. The only earlier hardware touches are AP-1.4 (a *complete* 1814 stack, regression of a copy) and AP-0.9 (the official driver on the SL rig). Before phase 7 the Audiophile is moved from the SL rig to the test Mac: unit powered off, cable connected, power on (no hot-plug).
| Id | Check | Pass criterion |
|---|---|---|
| AP-7.1 | Power-cycle with kext loaded (test kext in `/tmp`): bootloader → runtime, device registered | `ioreg` device node, name "FireWire Audiophile" |
| AP-7.2 | CoreAudio lists device; counts 4 in / 6 out at 44.1/48/88.2/96 | `halinfo`, AMS |
| AP-7.3 | **Channel-order loop test** (resolves ref §3.3/§8.1): tone on each of the 6 outputs, measure at RCA/S/PDIF; same for the 4 inputs | mapping table agrees with the derived LLRR order, or is corrected with a dated note |
| AP-7.4 | Playback, record, full-duplex loopback (analog out → in), `looplat` latency vs the reference constants | no breaks (`loopcheck`), latency noted |
| AP-7.5 | Rate sweep (`ratecycle` pattern), S/PDIF external sync, rate change refused/gated while a client streams | all status 0; no panic |
| AP-7.6 | Mixer/router via panel: every control audible, defaults, mute/dim, aux to S/PDIF out | checklist signed by the user |
| AP-7.7 | Front panel: Level Controller, headphone encoder, A/B switch + LEDs, Direct-Monitoring mode | checklist |
| AP-7.8 | MIDI (AP-6.2), sleep/wake, bus reset by power-cycle, 5-minute and 1-hour soak | notes; failures become items |
| AP-7.9 | ~~Capture the official driver on an SL rig with the Audiophile~~ — the unit is on the rig now; moved to AP-0.9 (phase 0) so its results feed the code instead of checking it afterwards | superseded 2026-10-19 |

### Phase 8 — regression on the 1814, installer, release
| Id | Task | Acceptance |
|---|---|---|
| AP-8.1 | Full 1814 ladder on the new tree (the checks of 0.1.0: audio, rates, digital modes, panel, helper replay, second-client loop test, meters clear on stop) | every item that passed on 0.1.0 passes; identical `ratecycle` transitions to captures 85/86 |
| AP-8.2 | Installer: new `.pkg` (kext, helper, panel, plug-in, firmware for each model), `postinstall` removes the old `com.openaudio.OpenAudioFW1814` kext/helper; uninstaller app; `check-self-contained.sh`; dmg | clean-Mac install test on the Air (user) |
| AP-8.3 | README/NOTICE/known issues for two models; tested-hardware matrix; public-repo sync **only on the user's go-ahead** | docs honest; commands for the user to push |
| AP-8.4 | Version and release asset: decision Q3 | — |

## 5. Test strategy

* **Host tests** (no hardware): encoders/decoders with golden bytes taken from the decompile (Rev2 stream-format command, function-block commands, status-quadlet decode, converters, channel tables, `.bcd` reader). The existing `make test` and `conformance_check.py` get an Audiophile section.
* **Bench tools** (`tools/`): `loopcheck`, `stutter`, `ratecycle`, `looplat`, `halinfo`, `midiprobe` get channel-count parameters (the 1814 assumes 10/6).
* **Conformance** (`scripts/conformance-check.sh`): panel keys ↔ kext keys ↔ system-map for each model descriptor.
* **Evidence**: every hardware run is a numbered capture in `docs/system-map/B5-sl-dtrace-truth/captures/` with kext UUID, machine, rate, buffer size.
* **Honesty rule**: a phase is "done" only when its acceptance test passed and the evidence is filed; anything else is reported as unverified.

## 6. Risks and mitigations

| Risk | Mitigation |
|---|---|
| Mixer commands (FB bytes) wrongly inferred → silent or wrong routing | AP-0.3 from the reference decompile first; golden-byte tests; SL capture (AP-7.9); no read-back, staged apply |
| Rev2 stream-format/sync command differences cause no audio or a bus fight | AP-0.2; first bring-up with logging only; stage-then-load; Air first |
| Kernel panic on a first load (as in every earlier first load) | `/tmp` staging, `kextutil -t -n`, test on the Air while the iMac stays untouched, say so before loading |
| Two kexts claiming one device | new installer removes the old one; the new kext keeps the same go-away personalities only for models it implements |
| Regression of the 1814 while refactoring | AP-2 keeps behaviour identical by construction (copy, then move), AP-8.1 reruns the 0.1.0 ladder |
| Wire-slot order wrong (conflict with FFADO) | AP-7.3 before any listening test with speakers on; low volume first |
| Only one Audiophile and no SL capture | request §9 Q4; otherwise rely on the decompile |
| Hot-plug damage (manual's warning) | power-cycle only; the user connects with the unit off |

## 7. Installer and naming (summary; details in AP-8.2)

New product name and bundle ids are decision Q2. The new `.pkg` carries: kext, helper + LaunchDaemon, panel (with its Swift runtime), MIDI plug-in (M-Audio's, unmodified), firmware files for the models it supports, uninstaller. `postinstall` stops the old helper, `kextunload`s and removes the 1814-only kext if present, then installs. Uninstall removes both generations' files.

## 8. Effort estimate (an estimate, not a measurement; one "session" = one working session like those of 2026-10-16/17)

| Phase | Sessions | Of which the user must be present |
|---|---|---|
| 0 Static research (AP-0.1…0.8) | 2-3 | — |
| 0 SL captures (AP-0.9) | 1 | the whole session (panel clicks, listening) |
| 1 Bootstrap | 1 | AP-1.4 load on the Air (~30 min) |
| 2 Refactor + Rev2 | 2-3 | — |
| 3 Audiophile kext | 3-5 | — |
| 4 Helper | 1 | — |
| 5 Panel | 3-4 | snapshot approvals |
| 6 MIDI | 0.5 | — |
| 7 Hardware ladder (one integrated run, plus fix-and-rerun loops) | 3-4 | all of it; each load is a panic risk |
| 8 Regression + installer + docs | 2-3 | 1814 ladder, clean install on the Air |
| **Total** | **≈ 18-26** | ≈ 7-9 of them |

v2 note: the SL captures (AP-0.9) add one session but remove most of the "if an SL capture is needed" risk of v1, and capture 90 already gives the key set, so the total range is unchanged. The estimate is still most sensitive to AP-0.3 (the AV/C function-block bytes, static only — the rig cannot capture them) and to the snd-bebob "no mixing without streaming" claim (if true, the idle-TX behaviour of the Rev2 path matters for the panel and the helper's replay).

## 9. Decisions needed from the user

* **Q1.** *Answered 2026-10-19: one driver for both models (one tree `openaudio-mafw-bebob/`).* Original question: Confirm the structure: one new tree `openaudio-mafw-bebob/` containing both the 1814 (copied, behaviour unchanged) and the Audiophile — versus an Audiophile-only kext that coexists with the 1814 one (not recommended: two kexts, same device personalities).
* **Q2.** *Answered 2026-10-19: product name **"OpenAudio BeBoB MAFW"** (no brand name in the product name — M-Audio is a brand); driver bundle id `com.openaudio.OpenAudioBeBoBMAFW` (same pattern as `com.openaudio.OpenAudioFW1814`); class prefix stays `com_openaudio_`.* Original question: Product name, bundle ids and class prefix of the new tree (e.g. keep `com.openaudio.*` and class prefix `com_openaudio_`; new bundle id `com.openaudio.OpenAudioMAFW`; app name "OpenAudio MAFW").
* **Q3.** *Answered 2026-10-19: **0.2.0**; the released 0.1.0 stays the public download until 0.2.0 passes the full 1814 ladder and the Audiophile ladder.* Original question: Version of the new product (0.2.0?), and whether the released 1814-only 0.1.0 stays the public default until the new tree has passed AP-8.1.
* **Q4.** *Answered 2026-10-19: rig captures done (91-93); the first load of our driver with the Audiophile is on the **iMac (build PC, the lab/test machine)** — the unit moves there from the SL rig when phase 7 starts.* Earlier text: Still open: (a) a session slot for AP-0.9 with the user at the rig's panel; (b) which machine hosts the first load of our driver (recommended: the Air, as for the 1814 — the iMac stays the build machine and keeps the 1814), and whether the 1814 is disconnected from that Mac during phase 7 (recommended: yes; two units on one bus is M-K, Appendix A item 8). Cable note: the unit has 6-pin FireWire 400 ports; the iMac port is FireWire 800 (adapter cable), the Air uses Apple's Thunderbolt-to-FireWire adapter.
* **Q5.** *Answered 2026-10-19: English.*

## 10. First actions once approved

1. Phase 0 (static) — can start immediately, needs nothing from the user. AP-0.9 as soon as the user can sit at the SL rig.
2. Create `openaudio-mafw-bebob/` (AP-1.1) and add the memory/CLAUDE.md note about the new tree. *Tracker (done 2026-10-19): the Audiophile phase has its own workbook `PROVENANCE-MATRIX-AUDIOPHILE.xlsx` (same tabs, scoring and conventions as `PROVENANCE-MATRIX.xlsx`: Painel, Resumo, Itens `AP-0.n.k`, Build `AP-n.m` + `DEC-Qn`, Flags_auditoria `AUD-AP-nn`, Legenda); statuses are changed with `openaudio-mafw-bebob/scripts/ap-status.py` (a copy of `plan-status.py`, since `openaudio-fw1814/` is not edited). `PROVENANCE-MATRIX.xlsx` stays the 1814 tracker.*

## 11. Step-by-step order (what happens, in which session, who is needed)

Each step ends with `/close-item`. "Desk" = no hardware, Claude alone. Steps 1-2 can run in parallel.

| Step | Where | What | Done when | Needs |
|---|---|---|---|---|
| 0 | — | Answers to Q1-Q5 | answers recorded in this file | the user (5 min) |
| 1 | desk | AP-0.1…0.8: nub table, `FWRev2AudioDevice` (34 methods) vs Rev1, settings classes + `AVCControlFunctionBlock` callers, LED vendor command, object layout, converters, panel `CXAudiophileDevice`; file 89/89b/90 as R-entries | phase-0 acceptance (ref §8 items 2-6, 10) | — |
| 2 | SL rig | AP-0.9 (a)-(g), one control at a time | captures 91+ filed; `PNAP-control-key-map.md` | the user at the rig; Audiophile stays connected |
| 3 | desk + build PC | AP-1.1…1.3: copy tree into `openaudio-mafw-bebob/`, rename per Q2, scripts to `~/oa-build-mafw`, build + `kextutil -t -n` | "appears to be loadable"; `openaudio-fw1814/` untouched (`git diff --stat`) | build PC on |
| 4 | Air | AP-1.4: load the unmodified copy (test kext in `/tmp`), 1814 `loopcheck` 34 s | same result as capture 88 | the user (sudo on the Air), 1814 connected |
| 5 | desk | Phase 2: model table, mixer-backend interface (1814 moved behind it), `FWRev2AudioDevice`, per-model user-client layout | `make test`, conformance, registry-parity script vs 0.1.0 | — |
| 6 | desk | Phase 3 kext (AP-3.1…3.9) with golden-byte host tests | all AP-3 host tests pass; conformance Audiophile section green | — |
| 7 | desk | Phase 4 helper (`.bcd` reader, product table), phase 6 MIDI check against the plug-in's port model | host tests on both `.bcd` | — |
| 8 | desk | Phase 5 panel: descriptors, Audiophile views; `--demo --snapshot` PNGs | the user approves the snapshots | the user (review) |
| 9 | SL rig → Air | Move the Audiophile (power off → cable → power on) | unit visible in `ioreg` on the test Mac, no driver claimed yet | the user |
| 10 | Air | Phase 7 integrated run: AP-7.1 → 7.8 in order; stop at the first failure, fix statically, re-stage, re-run | ladder checklist signed | the user, ~2-4 sessions |
| 11 | Air + iMac | Phase 8: 1814 regression on the new tree, installer, README/NOTICE, version per Q3 | AP-8.1…8.4 | the user; public push only on go-ahead |

---

# Appendix A — what it takes to support every FW device of the official driver (future work)

The reference kext implements eight product classes behind one nub. Product names and bootloader names are from `kDeviceSpecs @0x46450` (read from the binary); classes, base, channel counts and rates from the kext symbol tables and static data (read 2026-10-19; **only the first per-rate table of each model was read**, so counts at other rates and modes are not complete; every row is a starting point for its own phase 0). Ownership: the project has an FW1814 and an Audiophile; the others are not owned or testable.

| # | Product name (runtime / bootloader) | Class, base | Mixer path | Channels (in / out, first table) | Rates | MIDI quadlets | Firmware file | Notes |
|---|---|---|---|---|---|---|---|---|
| 0 | `FW 410` / `FW Bootloader` | `FW410Device : Rev2` (34 methods) | AV/C FB ("normal"); `ctl-services` has its FB table (10 out incl. HP; 14×10 mixer, 14×2 aux, 10×2 HP mixer) | 4 / 10 | 44.1…192 (like the 1814: rate-dependent tables not read) | 1 | `FW 410/` | closest sibling of the Audiophile: same backend, other FB ids; the Audiophile is the pilot for it |
| 1 | `FW Audiophile` / `FW Audiophile Bootloader` | `FWAudiophileDevice : Rev2` | AV/C FB | 4 / 6 | 44.1-96 | 1 | `FW Audiophile/FWAP.bcd` | **this plan** |
| 2 | `FW 1814` / `FW 1814 Bootloader` | `FW1814Device : Rev1` | "special": HSCI + direct writes | by rate and mode | 44.1-192 | 1 | `FW 1814/fw1814.bcd` | released stack |
| 3 | `ProjectMix` / `ProjectMix` | `FWProjectMixDevice : FW1814Device` (ctor calls the 1814's) | special like the 1814 (`snd-bebob` groups them) | (computed table; not read) | 44.1-96 | not read | `ProjectMix/` | needs the unit; includes a control surface (faders, transport): the reference's 18 methods are overrides of the 1814 |
| 4 | `FW Solo` / `FWSolo Bootloader` | `FWSoloDevice : Rev2` | AV/C FB; Solo-specific go-away personalities for `AppleFWAudio` (plist) | 4 / 4 | 44.1-96 | 0 | `FW Solo/` | only model with the `AppleFWAudio` detach personalities in the reference plist; mixer 8×4; stream channels swapped (`snd-bebob` `normal_meter_get`) |
| 5 | `Ozonic` / `Ozonic Bootloader` | `FWOzonicDevice : Rev2` | AV/C FB (`normal.rs` Ozonic) | 4 / 4 | 44.1-96 | 1 | `Ozonic/` | needs output+input streams together (`bebob_maudio.c:19-21`) |
| 6 | `NRV-10` / `NRV-10 Bootloader` | `FWNRV10Device : Rev2`, personality with `Model_ID 65665` | AV/C FB | 10 / 10 | 44.1-96 | 0 | `NRV-10/` | mixer-console style; the plist is the only one with a Model_ID match |
| 7 | `ProFire Lightbridge` / `… Bootloader` | `FWLightbridgeDevice : Rev2` (38 methods; overrides sync-type lookup, `RewriteDevice`) | AV/C FB; `ctl-services` `pfl.rs` | 34 / 36 | 44.1-96 | 1 | `ProFire Lightbridge/` | ADAT-bridge, very large channel count: bandwidth, DCL pool sizes and the 52-slot peak page assumption (`GetPeakLevelCount 0x34`) must be re-derived |

**Effort per model, after the Audiophile plan is done** (estimate, same session unit as §8; "desk" = phase 0 for that model + descriptor + device class + FB table + converters + panel view + host/conformance tests, ending at "implemented from the reference, unverified"; "ladder" = the phase-7 equivalent with the unit on the desk, only possible if the user obtains it):

| Model | Desk | Ladder | Why this size |
|---|---|---|---|
| FW 410 | 3-4 | 2-3 | same backend as the Audiophile, bigger mixer (14×10, two headphone buses), rates up to 192 kHz with rate-dependent tables not yet read; macfw's validated FW410 work is a cross-check of wire facts (GPLv3: facts only, no code) |
| FW Solo | 2-3 | 2 | small (4/4, no MIDI); its own `AppleFWAudio` go-away personalities; swapped stream channels |
| Ozonic | 2-3 | 2 | small (4/4 + MIDI); needs both streams running together (`bebob_maudio.c:19-21`) |
| NRV-10 | 3-4 | 2-3 | 10/10, console-style mixer, the only Model_ID personality in the plist |
| ProFire Lightbridge | 5-7 | 3-4 | 34/36 channels: isoch bandwidth, DCL pool sizes, the 52-slot peak page (`GetPeakLevelCount 0x34`), sync-type override, `RewriteDevice`; a large panel |
| ProjectMix I/O | 4-6 | 3-4 | 1814-derived (Rev1 + special firmware) plus a control surface the project has never decoded |
| Multi-device on one bus (M-K) | 3-5 | 2 | IRM/bandwidth across units, two engines, shared singletons; testable with the 1814 + Audiophile the user owns |
| **Sum** | **≈ 22-32** | **≈ 16-21** | on top of the Audiophile's 18-26 |

So the full official compatibility list is roughly **40-58 sessions to "implemented, unverified"** for all eight models, and **≈ 56-79 sessions to "supported"** — the last figure only if the user can borrow or buy the six missing units and their manuals. The binding constraint is hardware, not code: without a unit a model stays "experimental, untested" in the README and ships disabled by default (item 4 below). Recommended order if hardware appears: 410 (closest to the Audiophile, most common), then Solo/Ozonic (cheap), NRV-10, Lightbridge, ProjectMix.

**What is required, in order of generality:**

1. **Model table as data.** Everything model-specific in the kext, helper and panel is read from a descriptor (§3). The Audiophile phase builds exactly this; adding a model means filling a descriptor, writing a small device class for the per-model overrides listed in the reference's symbol table, and (for a new mixer layout) an FB table for the AV/C backend.
2. **Two AV/C revisions, two mixer families.** Rev1 + "special" (1814, ProjectMix) and Rev2 + "normal" (the other six). With the Audiophile done, 410/Solo/Ozonic/NRV-10 are *data and tests* on the same backend; Lightbridge adds scale; ProjectMix adds a control surface on the 1814 stack.
3. **Per-model research phase** (same as phase 0): decompile the model's class, its settings objects, converters, latency, plug maps, the CX* panel view; fill a reference doc like `audiophile-manual-reference.md` from its own manual (not in the repository for the others — the user would need to supply them) and the parallel sources (`ctl-services` `normal.rs`/`pfl.rs`/`special.rs`, FFADO `maudio/`, `snd-bebob`).
4. **Hardware access.** No model can be called supported on the evidence of the decompile alone (project rule). Without the unit, a model stays "implemented from the reference, unverified"; a descriptor can ship disabled by default.
5. **Panel.** One layout per mixer family plus per-model strip sets, ranges and artwork (artwork for every model exists in the reference panel's resources: `FW410*`, `FWDJ*`, `FWAdat*`… `_extracted/panel/…/Resources`; reuse is the same question as the plug-in's, user-approved for the dmg only).
6. **Helper and firmware.** One `.bcd` reader that follows the file's offsets (not the 1814's layout), a model→file table; `_extracted/helper/Firmware/` already has all eight files. Firmware stays opaque and read-only; no flashing.
7. **Installer and tests.** The installer ships firmware only for enabled models; the conformance script iterates descriptors; bench tools take channel counts.
8. **Multi-device on one bus** (the 1814 + Audiophile owned by the user make a real test): per-GUID state (exists), bandwidth/channel allocation across two units (IRM), two engines and shared kext singletons (debug trace ring, UID scheme), panel device list. This is the open item M-K of the 1814 project; it needs its own plan and a power-cycle-only test protocol.
9. **Maintenance cost.** Every added model multiplies the hardware ladder; recommend keeping a "supported" list = models with a signed-off ladder, plus an "experimental" list = implemented but untested, shown as such in the README (the policy of the public README: honest, current, tested-machine matrix).
10. **Not covered by this driver and out of scope:** the Avid/M-Audio DICE-based interfaces (a different kext; memory `project_dice_conflict`), USB and PCI M-Audio devices.
