# BUILD_AUDIOPHILE_PLAN v5 — OpenAudio BeBoB MAFW 0.2.0 (FW1814 + FireWire Audiophile)

Status: **v6, 2026-10-20 (user decision: the firmware tool is done for now as a check-only add-on; phase 9 stops at AP-9.1, AP-9.2…AP-9.5 shelved; R113)**. v5 kept as `BUILD_AUDIOPHILE_PLAN_v5.md`. Previous status line: **v5, 2026-10-20, approved by the user (phase 9 included; panel AP-5.2 approved)** = v4 + phase 9 (firmware check and update tool, user request 2026-10-20, reversing the v3/v4 "no firmware update" decision for this tool only). v4 kept as `BUILD_AUDIOPHILE_PLAN_v4.md`. Previous status line: **v4, 2026-10-20** (objective, user 2026-10-20: a fully working, bug-free 0.2.0 with the new panel for the FW1814 and the Audiophile; the panel/port manual comes after, AP-8.5) = v3 (approved 2026-10-19, in execution: AP-1.1…AP-2.4 done, R93-R97) + the user's **"generic first"** rule (2026-10-20, §0 and §1) and task **AP-2.5** that applies it to what is already written. v3 is kept as `BUILD_AUDIOPHILE_PLAN_v3.md`; v1/v2 are in git history (`audiophile-phase0` branch). Tracker: **`PROVENANCE-MATRIX-AUDIOPHILE.xlsx`** (Itens = findings with evidence, Build = these tasks, Flags = audit). Phase 0 is complete: resolutions **R77-R92**, captures **89-93**.

## 0. Decisions in force (user, 2026-10-19)

| Topic | Decision |
|---|---|
| Structure | **One driver for both models** in a new tree `openaudio-mafw-bebob/`; `openaudio-fw1814/` (released 0.1.0) is never edited |
| Multi-device principle | **Follow the official driver's own class hierarchy** for handling several devices: same base classes, same per-model subclasses, same virtual methods, so every model of the official list can be added the same way (§3) |
| **Generic first** (user, 2026-10-20) | This version is written to work with **all models** of the official list: **what can be generic, must be generic**. Model classes hold only what the hardware really makes different (tables, function-block maps, register writers, status-packet decode, names); base classes, user client, helper, panel, tools, scripts, logs and class matching are model-agnostic and read the model through virtuals/tables. Adding a model = adding a model class + its plist personality, never editing shared code |
| **Modular build** (user, 2026-10-20) | The driver is assembled **in modules: the common part on a base, the model-specific part imported on top** to make each model work as it should, following the **logic and structure of the official driver** (its class chain: `FWAudioDevice` → `FWAVCAudioDevice` → `FWRev1/Rev2AudioDevice` → model; settings base classes → model settings). Source tree: `kext/src/base/` (everything shared) and `kext/src/models/<Model>/` (one folder per model: device class, alias class, its settings subclasses, its tables, register map, peak procs). The official driver's logic is **documented while it is built**: each module's header states what the reference does there (addresses, call order) and `docs/ARCHITECTURE_MAFW_v1.md` describes the layers and how a model plugs in |
| Product | **"OpenAudio BeBoB MAFW"** (no brand name in the product name), driver bundle id `com.openaudio.OpenAudioBeBoBMAFW`, class prefix `com_openaudio_`, version **0.2.0**; 0.1.0 stays the public download until 0.2.0 passes both hardware ladders |
| Panel | **Our new 1814 panel, extended** with the Audiophile's controls; same look and behaviour; official screens are only a checklist; English |
| Firmware | **v6 (user, 2026-10-20): the Firmware Tool only checks a unit against the official 1.11.2 build of its model (AP-9.1, done); nothing in 0.2.0 writes flash; the update path (AP-9.2…9.5) is shelved, its decode (`Q-P3-kext-firmware-path.md`) stays. The tool ships only build dates and SHA-256 hashes, no M-Audio image.** Earlier: v3/v4: no firmware update (AP-4.1/4.4 cut). **v5 (user, 2026-10-20): a separate small tool checks firmware and can update it with the official 1.11.2 images (phase 9); the driver and panel themselves still never write flash.** The official images are M-Audio's: kept in the private repository (`_vendor/firmware-1.11.2/`, 8 models, manifest); a public build must not contain them (see AP-9.5) |
| Test machine | First load with the Audiophile on the **iMac build PC** (lab/test machine); the unit moves there from the SL rig when phase 7 starts |
| Method | Airliner rule: no hardware test of a partial build; phases 1-6 are verified by build, host tests, conformance and `kextutil -t -n`; one integrated hardware run (phase 7) |

## 1. Rules (unchanged from the project)

Generic first (§0): no model name, channel count, rate/latency table, meter index or class name in shared code; where the reference itself repeats code per model (e.g. the 1814's own control-packet start), one shared implementation is used if the behaviour is identical. 1814 behaviour never changes (AP-8.1). Mirror the reference (v1.11.2 `_extracted/`); the official driver is the main reference, third-party sources only decode bytes. Every value cites its source (R-entry, asm address, capture). Deviations only when 10.13.6 forces them or the user decides, each with a comment naming the reason. Homebrew, unsigned, SIP off. Hardware etiquette: ask before loading/unloading on the iMac, engine idle, never hot-plug (power-cycle), never flash, stage in `/tmp` + `kextutil -t -n` first; a panic reboots the iMac. `/close-item` after each task (R-entry, tracker, STATE/NEXT_STEPS). Public repository untouched until the user says so.

## 2. Scope

**In:** FW1814 (all of 0.1.0, unchanged behaviour) + FireWire Audiophile: enumeration and firmware start, 4 in / 6 out at 44.1/48/88.2/96 kHz, S/PDIF in/out and S/PDIF sync, MIDI 1/1, AV/C mixer/router (3 mixers, aux bus, output/headphone selectors), front panel (Level Controller, headphone knob, A/B switch + LEDs, Direct Monitoring mode), mute/dim, meters, per-GUID persistence, AC-3 pass-through as in the reference, panel, installer.
**Out:** firmware update, signing, the other six models (only the structure that makes them possible, Appendix A), multi-unit on one bus (M-K, later), Windows-only features.

## 3. Architecture = the official class hierarchy

Reference (v1.11.2; class list `A1-ghidra/ghidra-exports/classes.md`, superclasses from the metaclass constructors, Q16 Correction 2026-10-14) and what the new tree contains (✓ = exists in the 1814 clone and is copied; **new** = to write; **re-shape** = exists in the clone but merged differently from the reference, restored to the reference split):

```
IOService
└─ com_openaudio_FWMetaNub                      one nub for every M-Audio BeBoB device; kDeviceSpecs 8 rows   ✓ (table 1 → 8 rows)
IOAudioDevice
└─ FWDiscreteDevice → FWAudioDevice → FWAVCAudioDevice                                                          ✓
   ├─ FWRev1AudioDevice  (plain 0x18/0x19 stream format)                                                         ✓
   │  └─ com_m_audio_FW1814Device (alias, MIDI plug-in) → com_openaudio_FW1814Device                              ✓
   │     (└─ ProjectMix later)
   └─ FWRev2AudioDevice  (0x2F stream format, SIGNAL SOURCE sync, control packets/HSCI, peak update)               new
      └─ com_m_audio_FWAudiophileDevice (alias) → com_openaudio_FWAudiophileDevice                                new
         (└─ FW410 / Solo / Ozonic / NRV10 / Lightbridge later, same pattern)
FWSettings (per function, generic AV/C path)
├─ FWSettingsLevels      └─ FWSettings1814Levels (SetInputPan, SetInputRouting)     (└─ FWSettings410Levels later)   re-shape
├─ FWSettingsAuxSend     └─ FWSettings1814AuxSend (SetAuxMonitor)                                                    re-shape
├─ FWSettingsABSwitch                                                                                                ✓ (now used by the Audiophile too)
└─ FWSettingsRotaryControl                                                                                           ✓
FWBaseEngine → FWDiscreteEngine → FWEngine2 (+ pair / time-slave engines), FWStream, DCL/CMP/AV/C/HSCI/MIDI plumbing   ✓ unchanged
```

How a model plugs in (exactly as the reference): a nub row (`kDeviceSpecs`) + an Info.plist personality matching `FireWire Product Name` (Q-AP0 §1) + a device subclass overriding the per-model virtuals:

| Virtual (reference vtable) | FW1814 (clone) | Audiophile (new, values from) |
|---|---|---|
| `GetDeviceChannelsForSampleRate`, `GetSupported{Input,Output}SampleRates` | rate × mode tables ✓ | one table, 44.1-96 (Q-AP0 §6) |
| `SetupAudioEngine` | ✓ | 6/4 ch, MIDI 1/1, LLRR converters, peak proc (Q-AP0d) |
| `GetRoundTripLatencyForFDF` | ✓ | 182/196/330/354 (Q-AP0 §2.4) |
| `AVCControlPlugSignalFormat` (`vt 0xb50`) | Rev1 ✓ | Rev2 `0x2F` (Q-AP0 §2.1) |
| `GetAddressAndPlugForSyncType`, `AVCSetupSyncStream`, `SetClockSourceInternal` | 1814 HSCI path ✓ | Rev2 SIGNAL SOURCE; internal/S-PDIF only (Q-AP0 §2.2-2.3) |
| `SetBlankSlateClockSource` | 1814 override ✓ | base version: internal + 2.5 s (Q-AP0 §5) — new in the base class |
| `Start/Stop/Pause/ResumeReceptionOfControlPackets`, `GetControlPacketSize`, `ReadHSCIData`, `UpdatePeakMeters` | 1814 HSCI ✓ | Rev2 base + Audiophile 60-byte packet (Q-AP0b §1-2) |
| `MapChannelAndFunctionBlock`, `MapRoutingParameters` | 1814 ✓ | Audiophile kinds 0-9 + 15-entry route table (Q-AP0c §2-3) |
| `InitializeUserSettableProperties`, `ResendCurrentMixerSettings`, `ResetToFactorySettings` | ✓ | groups `<6,4,6,2,10,2>`, settings objects, factory state (Q-AP0c §4-5, R91) |
| `AVCControlSetLEDStatus`, `SetHeadphoneSelector` | 1814 ✓ | vendor `00 0D 6C 02 00 01 <v>`, selector FB 4 (Q-AP0b §4) |
| `IndexForAC3OutputChannels`, `GetFirstSWReturnPeakMeterIndex`, `NeedsControlPacketsForReadingHardwareControls` | ✓ | (2,5), 14, 1 |

The panel's model layer mirrors the official panel the same way (`CXDevice` → `CX1814Device` / `CXAudiophileDevice`; `CDeviceSettings` → `CFW1814DeviceSettings` / `CFWDJDeviceSettings`; PNAP §1-2): a device class with the four counts, the two control-group tables and the pair maps per model; the views stay ours.

## 4. Reuse and new work (file level, new tree)

| Area | Reused as is (copied from `openaudio-fw1814/`) | Re-shape (same behaviour for the 1814) | New |
|---|---|---|---|
| kext transport | DCL program/builder, packet pool, CIP, isoch channel, CMP, bus reset, stream control, shared sync, engines, streams, MIDI, user clients, AV/C unit + async queue + the four FB builders (`FWAvc.cpp:185-210`, byte-identical to Q-AP0c §1) | — | — |
| kext devices | `FWAudioDeviceChain` (FWDiscrete/Audio/AVCAudio/Rev1, 1814) | move 1814-only code that the reference keeps in base classes back to the base (blank-slate stays 1814-overridden) | `FWRev2AudioDevice`, `FWAudiophileDevice` + alias class, Audiophile tables |
| kext settings | channel groups, property dict/props, ABSwitch, RotaryControl | split `FWSettingsLevels/AuxSend` (generic AV/C setters through `MapChannelAndFunctionBlock` + feature/selector/mixer builders) and `FWSettings1814Levels/AuxSend` (the 1814 overrides); `FW1814Mixer` duties go to the 1814 device + its settings subclasses, as in the reference | — |
| kext nub/plist | nub | `kDeviceSpecs` 1 → 8 rows; Info.plist personalities for 1814 + Audiophile (+ go-away for the 1814) | Audiophile personality |
| kext samples | 1814 converters | — | `_ConvertLLRR4SkipMIDI1`, `_RearrangeSampleFrameLLRR6`, `_SamplePeaks6` (Q-AP0d) |
| helper | firmware start, settings persistence, gating | single `kDeviceClass` → device-class table (reference 8-entry loop, H10) | — |
| panel | all views, widgets, theme, IOKit bridge, presets, meters, CoreAudio rate path | `DeviceModel`/`ChannelLayout`/`MaKeys` become per-model (official `CXDevice` split) | Audiophile device model + the Audiophile strips/tabs content (§6) |
| MIDI | M-Audio plug-in unmodified | — | alias class `com_m_audio_FWAudiophileDevice` (kext) |
| installer/scripts/tools/tests | pkg, dmg, uninstaller, guards, bench tools | product name/ids 0.2.0, removes the 0.1.0 kext/helper on install; scripts target `~/oa-build-mafw`; bench tools take channel counts | Audiophile host tests and conformance section |

## 5. Phases, tasks, acceptance

Task ids as in the tracker's Build tab. "Source" = where the exact behaviour is written down.

### Phase 1 — bootstrap (no behaviour change)
| Id | Task | Acceptance |
|---|---|---|
| AP-1.1 | Copy the 1814 tree into `openaudio-mafw-bebob/`; rename product to OpenAudio BeBoB MAFW / `com.openaudio.OpenAudioBeBoBMAFW` / 0.2.0 (kext, helper, panel, installer); class prefix kept | builds on the iMac; `git diff main -- openaudio-fw1814` empty |
| AP-1.2 | Scripts point at the new tree and `~/oa-build-mafw` (sync, stage, load-test, install/uninstall, conformance, self-contained guard) | each script runs against the new tree; `~/oa-build` untouched |
| AP-1.3 | Build + `kextutil -t -n` on the iMac, no load | "appears to be loadable" |
| AP-1.4 | Load the renamed copy on the iMac with the 1814 (test kext in `/tmp`, user OK), `loopcheck` 34 s | same as capture 88 |

### Phase 2 — reference hierarchy, 1814 unchanged
| Id | Task | Source | Acceptance |
|---|---|---|---|
| AP-2.1 | `kDeviceSpecs` 8 rows; nub selection by product name; Info.plist personalities per model | Q-AP0 §1 | registry of the 1814 identical to 0.1.0 (diff script) |
| AP-2.2 | Settings re-shape: generic `FWSettingsLevels/AuxSend` (AV/C path through device virtuals) + `FWSettings1814Levels/AuxSend` overrides; `FW1814Mixer` responsibilities moved to the reference owners | Q-ME, Q15f, Q-AP0c §4 | conformance pass; kext builds and passes `kextutil -t -n`; the 1814 register writers keep their 0.1.0 bodies and every 1814 key reaches the same writer (code reading). **Amended 2026-10-19 (user):** no before/after write recorder — the variables are already mapped (system-map + tracker); reorganise only. The 1814 on hardware is checked in AP-8.1 |
| AP-2.3 | `FWRev2AudioDevice`: `0x2F` stream format, SIGNAL SOURCE sync + `GetAddressAndPlugForSyncType`, Rev2 control packets (`Start/Stop/Pause/ResumeReceptionOfControlPackets` @0x13138…, sharing the existing `FWHSCI` class; the 1814 keeps its own override `FW1814Device::StartReceptionOfControlPackets @0xe064`, as in the reference), `UpdatePeakMeters`, base `SetBlankSlateClockSource` in `FWAVCAudioDevice` | Q-AP0 §2, §5; R77, R85 | golden-byte host tests: `0x2F` C1/C0 frames, `0x1A` STATUS/CONTROL frames, blank-slate order |
| AP-2.4 | User client / peak page per model (Audiophile 14 + 6 in the 52-slot page) | Q-AP0b §2, PNAP §6 | host test of slot layout (done 2026-10-20, R97; the Audiophile check lands with AP-3.1) |
| AP-2.5 | **Genericity pass** (user rule 2026-10-20), from the audit of the tree after AP-2.4: (a) engine description from `GetDescriptionString()` (was the literal "FireWire 1814", `FWBaseEngine.cpp:54`); (b) user client renamed to the reference's generic `com_openaudio_FWUserClient` (ref `com_m_audio_FWUserClient`), plist `IOUserClientClass` follows; (c) `FWAudioDevice` base defaults that are 1814 tables (`GetDeviceChannelsForSampleRate` = 1814 S/PDIF/ADAT composition, `GetRoundTripLatencyForFDF` = 1814 table, supported-rate list) move to the FW1814 class; the base keeps only reference base bodies or labelled neutral defaults; (d) one shared control-packet start/stop/pause/resume for the 1814 and Rev2 (same HSCI code today); (e) log lines and comments in shared code without the model name; (f) panel, helper and bench tools match a generic base class (`com_openaudio_FWAVCAudioDevice`) instead of `com_openaudio_FW1814Device`; (g) conformance check gains a "no model literal in shared files" test; (h) **modular source tree**: split `FWAudioDeviceChain.{h,cpp}` into one module per reference class (`base/FWAudioDevice`, `base/FWAVCAudioDevice`, `base/FWRev1AudioDevice`, `base/FWRev2AudioDevice`) and move every FW1814-only piece (device + alias class, `FWSettings1814*`, rate/channel/latency tables, mixer register map, `_Sample1814Peaks*`, input-delay workaround) to `models/FW1814/`; Makefile builds `base/` + every `models/*/`; (i) `docs/ARCHITECTURE_MAFW_v1.md`: the official driver's layers, call order (attach → probe rounds → initHardware → InitializeUserSettableProperties → blank slate → resend → control packets; rate change; sleep/wake), which virtuals a model overrides, and the checklist to add a model | audit 2026-10-20 (R98); user 2026-10-20 | build + `kextutil -t -n` + conformance (incl. (g)) + host tests; 1814 code paths unchanged by code reading; hardware in AP-8.1 |

### Phase 3 — Audiophile device
| Id | Task | Source | Acceptance |
|---|---|---|---|
| AP-3.1 | `FWAudiophileDevice` + alias class: names, channel/rate table, latency, AC-3 index, SW-peak index, control-packet size | Q-AP0 §6, §2.4, §7 | host tests against the tables |
| AP-3.2 | Converters + peak proc | Q-AP0d (asm-confirmed permutations) | golden frames |
| AP-3.3 | Bring-up path: CMP plug 0 both ways, Rev2 kick in `ConnectAudioMIDIChannels`, clock via `MAClockSource` | Q-AP0 §2, §5, §7; Q-START | staged logs (hardware gate phase 7) |
| AP-3.4 | `ReadHSCIData`: peaks, lock + FDF (w3/w2), encoders (detents), A/B button | Q-AP0b §2; capture 93 | host test with recorded packets (capture 93 bytes) |
| AP-3.5 | Audiophile `MapChannelAndFunctionBlock` (kinds 0-9), `MapRoutingParameters` (15 rows), `SetHeadphoneSelector` | Q-AP0c §2-3 | golden bytes per kind |
| AP-3.6 | `InitializeUserSettableProperties` (six groups, four settings objects, functors), `ResendCurrentMixerSettings`, `ResetToFactorySettings`, `MAMetaResetToFactorySettings` | Q-AP0c §4-5; R91 | factory state equals capture 92's reset plist |
| AP-3.7 | A/B switch + LEDs (`AVCControlSetLEDStatus`), Level Controller step law | Q-AP0b §3-4; R87 | host test of the state machine and step law |
| AP-3.8 | AC-3 path (base behaviour, S/PDIF pair 2/5) | Q-AP0 §7 | registry lists the AC-3 formats at 44.1/48/96 |
| AP-3.9 | Diagnostics ring for the new paths; teardown safe with async AV/C in flight | R-kextunload lesson | `kextunload` ×5 on the iMac |
| AP-3.10 | EXTENDED PLUG INFO query (read-only, STATUS) at bring-up, logged to the diagnostics ring — settles the physical channel order before any audio | R92 | log shows the cluster/channel order |

### Phase 4 — helper
| Id | Task | Acceptance |
|---|---|---|
| AP-4.2 | Device-class table (1814 + Audiophile); bootloader → runtime through `MAFirmwareStart` | helper log shows both classes watched (host run) |
| AP-4.3 | Per-GUID persistence for a second model; replay after reload | as R57 for the 1814 |

### Phase 5 — panel (our new panel, extended)
| Id | Task | Acceptance |
|---|---|---|
| AP-5.1 | Per-model device layer (official `CXDevice` split): counts 1814 / Audiophile, control groups, pair maps, key mapping; the 1814 model reproduces today's UI | `--demo --snapshot` of the 1814 equals 0.1.0 |
| AP-5.2 | Audiophile content in our layout: every control of PNAP §7 (mixer, output, hardware tabs, Level Controller, mute/dim, presets, reset) | snapshots approved by the user; checklist of capture 91 complete |
| AP-5.3 | Device menu with both models, as the official panel does with two units (capture 94, R93): one window, `unit n – <name>` entries with ⌘n, the shown unit checked, title and window size follow the model, switching is panel-side only (no driver writes); per-GUID state | panel works with only the 1814, only the Audiophile, and both attached |

### Phase 6 — MIDI
| AP-6.1 | Unmodified plug-in finds the Audiophile through the alias class; `MAMIDI*Ports = ("1")` | `midiprobe` lists the port (phase 7) |

### Phase 7 — hardware ladder on the iMac (one integrated run; user present; no hot-plug)
| Id | Check | Pass |
|---|---|---|
| AP-7.1 | Audiophile moved to the iMac (power off → cable → power on); load (test kext in `/tmp`): bootloader → runtime | device node "FireWire Audiophile"; bootloader Model_ID noted (closes AP-0.1.6, AP-0.8.3) |
| AP-7.2 | CoreAudio: 4 in / 6 out at 44.1/48/88.2/96 | `halinfo` |
| AP-7.3 | EXTENDED PLUG INFO log + tone per output/input at low volume | channel order recorded (closes AP-0.7.3) |
| AP-7.4 | Playback, record, loopback, `looplat`, `loopcheck` | no breaks; latency noted |
| AP-7.5 | Rate sweep, S/PDIF sync lock/unlock, rate change refused while a client streams | no panic; `MARateLocked` follows the status word |
| AP-7.6 | Every panel control audible (incl. SW-return levels — closes AP-0.3.6 — and mixing without streaming, AP-0.9.3) | checklist signed |
| AP-7.7 | Front panel: Level Controller, headphone knob, A/B + LEDs, Direct Monitoring | checklist |
| AP-7.8 | MIDI, sleep/wake, bus reset by power-cycle, 5 min and 1 h soak | notes |
| AP-7.9 | Firmware Tool with both units attached (closes AP-9.1, v6) | both "Current"; no registry writes |

### Phase 8 — 1814 regression, installer, release
| AP-8.1 | Full 0.1.0 ladder with the 1814 on the new driver | everything that passed on 0.1.0 passes |
| AP-8.2 | Installer 0.2.0 (removes 0.1.0 kext/helper), uninstaller, dmg, self-contained guard; includes the Firmware Tool (check only, no images; v6) | clean install on the Air |
| AP-8.3 | README/NOTICE/known issues for two models (the Firmware Tool only checks; nothing writes flash; v6); public sync only on the user's go-ahead | docs honest |
| AP-8.4 | Release asset 0.2.0 by the user | — |
| AP-8.5 | **Panel / port manual** (user, 2026-10-20: only after 0.2.0 is complete and bug-free with the new panel; for the future): formal document(s) for a UI/app specialist to build the panel on any OS — device discovery, control plane (MA* keys, encodings, per-model counts), notifications, meter page, clock/rate rules, front-panel interactions, persistence, safety rules — written from the finished modular 0.2.0 | AP-8.1…8.3 | a specialist can rebuild the panel from the manual alone |

### Phase 9 — firmware tool (v5; check first, flash-writing only after approval)
> **v6 (2026-10-20, user): phase 9 ends at AP-9.1.** The tool is an add-on that tells the user whether a unit runs the official 1.11.2 build of its model, to avoid incompatibilities. AP-9.2…AP-9.5 below are **shelved** (kept for the record, not planned). Official rule, for the tool's wording (R113): the 1.11.2 helper re-flashes automatically, without asking, whenever the unit's firmware date is *not equal* to the bundled `.bcd` date (`isEqual:`, `A4-helper/ghidra-exports/decomp/a4-h2-bootloader-detection.log:160`), on every `deviceMatched:`; the official installer does not create the inhibit files. So the official driver has no "minimum firmware"; our `minimumFirmware` / "Official 1.11.2" value means *the official build of that model*. Older and newer units both get the advice to use the official driver on a supported Mac.

Reference: A1 `Q-P3-kext-firmware-path.md` (complete decode of the kext update path), A4 H4/H10 (helper picks the image by product name, "dates match -> just start"), A5 `A5-firmware.md`, `_extracted/helper/FirmwareStatusWindow.app` (the official progress window), `_vendor/firmware-1.11.2/README.md` (the 8 official images, build dates, model ids, SHA-256).
| Id | Task | Acceptance |
|---|---|---|
| AP-9.1 | **Firmware check** (read-only): a small app, "OpenAudio Firmware Tool", lists every attached unit (generic nub + device classes): product name, GUID, runtime / bootloader state (`MAFirmwareInfo[0x0C]`), firmware / bootloader dates, hardware model; compares the firmware build with the official 1.11.2 build of that model (the model's `minimumFirmware` / the manifest): **current**, **older**, **newer**, **unknown model** | runs with both units attached; both show "current" (1814 2007-07-13 08:04:40, Audiophile 2007-05-04 10:22:12); no registry writes (DTrace/log) |
| AP-9.2 | *(shelved, v6)* **Kext update path** restored in `base/FWMetaNub` byte-for-byte from Q-P3 §4-§5: `MAFirmwareHalt` (Reset to bootloader), `MAFirmwareUpgradeImage` (bootloader mode only; dispatch on `bcd[0x34]`: BeBoB 2.51 two sections / BeBoB 3.2 single stream), `DownloadStart/Block/End` with the 512-byte loop, retries, error path (bus reset + 3.5 s); `MAFirmwareStart` unchanged | golden-byte host tests of every command against Q-P3 §4 and FFADO `bebob_dl_codes.cpp`; section math for all 8 images; build + `kextutil -t -n` |
| AP-9.3 | *(shelved, v6)* **Update flow** in the tool, mirroring the official helper: pick unit -> check -> confirm (warning text) -> Halt -> wait for the bootloader nub -> send the image of that model -> Start -> re-read `MAFirmwareInfo` and compare the date; progress like FirmwareStatusWindow; refuses unknown models, images whose header model id / CRCs do not match, and any unit that is streaming | dry run against a fake backend (every step and refusal); host tests of the image checks (CRC32 of header, payload, appendix as `audiophile-manual-reference.md` §6) |
| AP-9.4 | *(shelved, v6)* **Hardware test, user present**: re-flash the *same* official build on one unit (no unit needs an update today); recovery = power-cycle + `MAFirmwareStart` | tool reports success, unit runs, `MAFirmwareInfo` date unchanged, audio loopcheck OK |
| AP-9.5 | *(shelved, v6)* **Image source for public builds**: the tool does not ship M-Audio images; it reads them from the user's own copy of the official 1.11.2 installer (`M-Audio_FireWire_1-11.2.dmg` -> pkg payload `Library/StartupItems/M-Audio FireWire Audio Helper/Firmware/`), verified by the SHA-256 list | extraction from the dmg reproduces the 8 hashes |

## 6. Panel extension (from the measured control map, PNAP §7)

Our new panel gets an Audiophile device model and these contents, in its existing layout:
* **Mixer:** 3 SW-return strips (1/2, 3/4, S/PDIF) and 2 input strips (analog, S/PDIF): fader, aux L/R, pan (inputs), solo/mute/link/ctrl, out 1/2·3/4·spd routing; output-bus and aux-bus meters.
* **Output:** 1/2, 3/4, S/PDIF out (fader, balance, solo/mute/link/ctrl, main↔aux), aux master (fader, pan, mute/link/ctrl), phones (fader, mute/link, mon 1/2·3/4·spd·aux).
* **Hardware:** sample rate (CoreAudio path, as today), sync internal/external, headphone A/B source matrix, A/B switch mode.
* **Side:** Level Controller (sw rtn / output / input / aux send), main mute/dim, presets load/save/delete, reset (with confirmation).
* Keys, encodings and defaults exactly as PNAP §7 and capture 92.

## 7. Tests and conformance

Host tests with golden bytes from the decompiles and captures (AV/C frames, function-block map, route table, status packet from capture 93, converters, factory state from capture 92). `conformance-check.sh` gets an Audiophile section (every key ↔ kext handler ↔ panel control). 1814 regression is by byte-identical register writes (host) and the 0.1.0 ladder (hardware, AP-8.1).

## 8. Risks

| Risk | Mitigation |
|---|---|
| Re-shaping the 1814 settings changes 1814 behaviour | AP-2.2 acceptance = byte-identical writes before/after; AP-8.1 full ladder |
| Rev2 stream/sync commands wrong → no audio | asm-verified layouts + FFADO field match (R77); first run logs every AV/C response |
| Physical channel order unknown | AP-3.10 query + AP-7.3 before speakers at volume |
| SW-return level blocks are kext-only | logged responses in AP-7.6; reference behaviour kept either way |
| First load panics the iMac | `/tmp` staging, `kextutil -t -n`, user told before loading |

## 9. Effort (estimate; one session = one working session like 2026-10-16/17)

Phase 1: 1 · Phase 2: 3-4 (the settings re-shape is the careful part) · Phase 3: 3-4 · Phase 4: 0.5 · Phase 5: 2-3 · Phase 6: 0.2 · Phase 7: 2-3 (user present) · Phase 8: 2 — **≈ 14-18 sessions** (v2 estimated 18-26 before phase 0 removed the unknowns).

## 10. Approval

Approve as written, or name the sections to change. On approval: tracker Build tab already lists the tasks; work starts with AP-1.1, each task closed with `/close-item`.

---

# Appendix A — what it takes to support every FW device of the official driver (future work)

The reference kext implements eight product classes behind one nub. Product names and bootloader names are from `kDeviceSpecs @0x46450` (read from the binary); classes, base, channel counts and rates from the kext symbol tables and static data (read 2026-10-19; **only the first per-rate table of each model was read**, so counts at other rates and modes are not complete; every row is a starting point for its own phase 0). Ownership: the project has an FW1814 and an Audiophile; the others are not owned or testable.

| # | Product name (runtime / bootloader) | Class, base | Mixer path | Channels (in / out, first table) | Rates | MIDI quadlets | Firmware file | Notes |
|---|---|---|---|---|---|---|---|---|
| 0 | `FW 410` / `FW Bootloader` | `FW410Device : Rev2` (34 methods) | AV/C FB ("normal"); `ctl-services` has its FB table (10 out incl. HP; 14×10 mixer, 14×2 aux, 10×2 HP mixer) | 4 / 10 | 44.1…192 (like the 1814: rate-dependent tables not read) | 1 | `FW 410/` | closest sibling of the Audiophile: same backend, other FB ids; the Audiophile is the pilot for it |
| 1 | `FW Audiophile` / `FW Audiophile Bootloader` | `FWAudiophileDevice : Rev2` | AV/C FB | 4 / 6 | 44.1-96 | 1 | `FW Audiophile/FWAP.bcd` | **this plan (v3)** |
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
