# BUILD_FW410_PLAN v2 — FireWire 410 in OpenAudio BeBoB MAFW

Status: **v2, 2026-10-26, approved by the user** ("go ahead, run it non stop"). Decisions (user, 2026-10-26): versions package **0.4.0** / driver **0.3.0** / panel **1.6**; **add all hidden features** (§6); **Classic edition first** (build PC, 10.13); R134 (sleep/power-loss mixer recovery) is treated as quasi-finished and stays in the tree. v1 kept as `BUILD_FW410_PLAN_v1.md`. Since v1 the open points were worked (R136): the 192 kHz remap is decoded (B-3a done), all 8 outputs stream at 192 kHz (capture 105), `[+0x34b]` has no reader (B-5 done: stored, unused), phones bit 5 is inconclusive on v1.9 (capture 104: implement as v1.11.2, identify by ear in F7); the manual reference is `docs/fw410-manual-reference.md`. Branch `fw410`. Tracker: `PROVENANCE-MATRIX-FW410.xlsx` (Itens = findings, Build = the tasks below, Flags = audit). Evidence so far: R135; `A1-ghidra/Q-410-0-reference-map.md`, `A2-panel/PN410-control-key-map.md`, captures 102 (baseline) and 103 (panel, stereo pairs, controls, rates).

## 0. Decisions in force
| Topic | Decision |
|---|---|
| Structure | Same driver and panel as the FW1814 and the Audiophile (`openaudio-mafw-bebob`): base + `models/FW410/` (kext) + `Models/FW410/` (panel). **Generic first**: anything the 410 needs that is not model-specific goes into the base, so the other models get it too (user, 2026-10-20) |
| Reference | v1.11.2 `_extracted/` (kext class `com_m_audio_FW410Device : FWRev2AudioDevice`, panel `CFW410*`); SL v1.9 captures are wire truth; snd-bebob / FFADO / ctl-services decode bytes only |
| Panel | **The 410 gets its own panel layout** (user, 2026-10-26), in the style of our 1814/Audiophile panel; the official 410 screens (capture 103) are the checklist of controls; plus the generic stereo-pairs checkbox |
| Hidden features | **All added** (user, 2026-10-26): sixth phones source, per-strip level-controller membership on every group, `SetInputLevel` reachable (§6) |
| Firmware | Check only (the catalog already has FW 410 `20070504102656` = the user's unit); nothing writes flash |
| Testing | No formal testing (R123). First integrated run with the 410 on the **Classic** edition (iMac build PC, 10.13) when the user decides (§5 F7) |
| Signing | Unsigned, SIP off (unchanged) |

## 1. What the 410 is (summary of the evidence)
* Rev2 device (0x2F stream format, SIGNAL SOURCE sync) like the Audiophile, but with a **register mixer** (block writes to `0xFFC7:0x00700000 + off` through `CustomMixerWriteQuads`) like the FW1814 — this corrects plan v9 Appendix A row 0 ("AV/C FB").
* 4 in / 10 out at 44.1-96 kHz, 2 in / 8 out at 176.4/192 kHz, MIDI 1/1; outputs analog 1/2…7/8 + S/PDIF; inputs analog 1/2 + S/PDIF; coax or optical S/PDIF input; one front encoder (level controller), no A/B button.
* 76-byte status packet, meter page: 4 hardware inputs, 10 SW returns (host peaks), 14 hardware outputs.
* Stereo-pair engines: 5 CoreAudio pair devices when enabled (already generic in the base).

## 2. Reuse (no work) — already in the base
Nub + `kDeviceSpecs` row "FW 410" / "FW Bootloader", vendor-name probe (capture 102: "M-AUDIO"); Rev2 device (stream format, sync stream, control-packet poll, hardware peaks); `CustomMixerWriteQuads` and the channel-group batch writers (FW1814); generic settings objects (Levels, AuxSend, Rotary) and the AV/C function-block paths (Audiophile); `_ConvertLLRR4SkipMIDI1`; stereo-pair engines; user client, meter page, MIDI transport; helper persistence per GUID; firmware check.

## 3. New base work (generic, every model benefits)
| ID | Task | Reference | Acceptance |
|---|---|---|---|
| B-1 | `FWRearrangeLLRR10WithMIDI` and `FWSamplePeaks10` in `FWSamples` | asm 0x2b951 (wire order 8 0 2 4 6 9 1 3 5 7, stride 10/11), decomp `_SamplePeaks10` | host test against hand-computed frames; frames = 0 guarded |
| B-2 | Engine flag `[+0x34a]` "legacy UID class name" → `getGlobalUniqueID` uses `com_m_audio_FWEngine1` | asm 0x5b95-0x5ba8; live UID capture 103 | UID string `com_m_audio_FWEngine1:<GUID>` in a host/conformance check; FW1814/Audiophile UIDs unchanged |
| B-3 | 192 kHz channel-remap branch: `DeviceChannels +0x18/+0x20` tables in `ConvertInputSampleData` and the clip path | `ConvertInputSampleData @0x4624`, clip path asm 0x11e41-0x11e5b, tables 0x2f9e0 / 0x2f9f0 | decode first (desk task B-3a), then host test; FW1814/Audiophile rows (tables 0) unchanged |
| B-4 | `FWSettingsLevels`: SW-return routing as an overridable hook + a batch hook (the 410 overrides `SetSWReturnRouting` and adds `BatchSetSWReturnRouting`, sent after the batch); verify the base slots in the `FWSettingsLevels` vtable | `FWSettings410Levels__*` decomps, `FWSettingsLevels::SetSWReturnRouting` (mafw.asm line 40374) | FW1814/Audiophile behaviour unchanged (conformance) |
| B-5 | `[+0x34b]` engine flag | written at 0x15526, no reader found | desk: find the reader or document it as unused |
| B-6 | Pair-engine UID suffix = engine number 1..n, not the engine bit index | capture 103 (`m_audio_b_FWPairEngine:1..5`) vs Q-MJ §8 | registry check when it runs |

## 4. Model module `models/FW410/` (kext)
| ID | Task | Reference |
|---|---|---|
| M-1 | `com_m_audio_FW410Device` alias (MIDI plug-in matches it: `com.m-audio.midiplug.fw410`) + `com_openaudio_FW410Device : FWRev2AudioDevice`; names "FireWire 410" / "FW410" / "FW 410 box.png"; plist personality (product name "FW 410") | Q-410-0 §1 |
| M-2 | `FW410Tables.h`: two channel rows (`0x4ae20`, `0x4ad40`), rates `0x2fa10`, latency per FDF, AC-3 indices 0/5, first SW-return meter 4, control packet 0x4c | Q-410-0 §2-3 |
| M-3 | `SetupAudioEngine`: MIDI 1/1, counts, LLRR10 procs (B-1), flags `[+0x2d6]` `[+0x34a]` `[+0x34b]` | decomp |
| M-4 | `SetClockSourceInternal`: internal / coax (type 4) / optical (type 3) | asm 0x14eda |
| M-5 | `ReadHSCIData`: 76-byte packet, meter slots 0-3 / 14-27 from hardware, 4-13 from host peaks, lock/FDF, one encoder | decomp |
| M-6 | Mixer map: `MapChannelAndFunctionBlock @0x14600` (incl. kind 4 by bits, aux map `0x2fa50`), `MapRoutingParameters` fix-ups | decomp |
| M-7 | Register writers: `SetInputPan`, `SetInputRouting`, `SetSWReturnRouting` + `BatchSetSWReturnRouting`, `SetHeadphoneSource` (`MAHeadphoneSourceV2`), `SetInputLevel`; shadow words; `FWSettings410Levels` | decomps, offsets Q-410-0 §5 |
| M-8 | `InitializeUserSettableProperties` (groups, three batch writers at 0x00 / 0x14 / 0x28, Rotary, AuxSend with out/2 + 1 switches, 410Levels, `MAHeadphoneSourceV2`, initial input-level write), `ResendCurrentMixerSettings`, `ResetToFactorySettings` (phones source = 1) | decomps |
| M-9 | Host tests (tables, map, register words for each setter against the decomp formulas, status decode) + conformance rows | — |

## 5. Helper, panel, MIDI, installer, run
| ID | Task |
|---|---|
| F3 | Helper: nothing expected (generic); check the per-GUID keys of the 410 (capture 103 list) are persisted and replayed |
| F4.1 | Panel `Models/FW410/`: profile + its own layout — mixer (5 sw-rtn strips, analog/spdif in with pan, aux sends, multi-select out buttons), output (5 outs + aux + phones with multi-select sources), hardware card (coax/optical input), level controller with 5 targets, meters (4 in / 10 sw rtn / 14 out) |
| F4.2 | Panel (generic): "Show stereo pairs as CoreAudio devices" checkbox (`MAEnableStereoPairEngines`) on the hardware tab for every model whose driver publishes it |
| F4.3 | Panel (generic): phones source as a bitmask (multi-select) where the model allows it |
| F4.4 | Panel: 176.4/192 kHz view (2 in / 8 out, as the 1814's high-rate view) |
| F5 | MIDI: the unmodified plug-in matches the alias; check port names |
| F6 | Installer, README/NOTICE (410 listed as supported only after the run), CHANGELOG, versions (proposal: package 0.4.0, driver 0.3.0, panel 1.6) |
| F7 | First integrated run with the 410 (user decides when and where: dev Mac Modern, or the iMac Classic) — attach, firmware state, playback/record at 48/96/192, every mixer control by ear, stereo pairs on/off, MIDI, sleep/wake |

## 6. Hidden features (driver supports, official panel does not show) — for decision
1. **Sixth phones source** (`SetHeadphoneSource` bit 5): unknown source; desk + one SL-rig test (`MAHeadphoneSourceV2 = 32`, listen) settles what it is.
2. **Per-strip level-controller membership** (`MARotaryControlV2` bits; the official panel greys the mixer `ctrl` buttons for some targets).
3. **`SetInputLevel`** (registers 0x28/0x2c, no caller found): probably dead; do not expose unless a caller is found.
4. Fan-out routing (already in the official panel; keep it).

## 7. Risks
* B-3 (192 kHz remap) is undecoded — 192 kHz stays out of scope if it cannot be decoded.
* The register mixer on a Rev2 device is a new combination for the clone (batch writers were only used with Rev1).
* Hardware and device-specific behaviour (control packet, encoder) unverified until F7.
* The 176.4/192 kHz row keeps the S/PDIF groups while the channel count is 8 (`0x4ad40`): what the pair engines and the panel do at those rates is to be read in the reference before F4.4.

## 8. Effort (estimate)
Desk B-3a/B-5 + base B-1…B-6: 1-2 sessions; model M-1…M-9: 2 sessions; panel F4: 2 sessions; F3/F5/F6: 1 session; run F7: 1-2 sessions with the user.

## 9. Approval
Approved 2026-10-26 (v2 decisions above).
