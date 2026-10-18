# NEXT_STEPS — resume anchor (rewritten 2026-10-15)

> **Removal notice (2026-10-14):** all pre-pivot code and docs (`_legacy_pre_ghidra/`, `openaudio-fw1814/firmware-helper/`, `openaudio-fw1814/legacy-mod/`) were deleted from the working tree at the user's request. They are recoverable from git: `git show d1-approved-2026-10-14:<old path>` (tag `d1-approved-2026-10-14`, commit `a9cb63c`). References to those paths in this file or in `docs/system-map/` are historical. `openaudio-fw1814/` is intentionally empty; new code is built from scratch per `docs/BUILD-PLAN.md` once the user approves it.

Read this first after a context clear, then `NEW_PLAN.md` → `docs/STATE.md` → `docs/BUILD-MANUAL.md` §0 → `docs/system-map/README.md`.

## 2026-10-17 (evening) — read this first
M-U done (verified), panel rate list done, MIDI plug-in installed and loading (capture 82, R58). Next: (1) **MIDI loopback** — connect a DIN cable from the 1814 MIDI OUT to its MIDI IN, then `tools/midiprobe loop 0 5` and a SysEx round trip (M-H/M-H2/M-V); (2) M-S: decide on a HAL re-publish at optical switches (the label is only right for a freshly added device); (3) install.sh with the current kext + installer copies the plug-in (IN.5); (4) sleep/wake and bus-reset tests.

## 2026-10-17 (later) — (earlier entry): gap analysis `docs/system-map/GAP-ANALYSIS-2026-10-17.md`
Done since the entry below: PN.8 (rate through CoreAudio), helper H.4 verified + release bug fixed (H.7), engine UID (M-T). Your decisions: **M-U** (v1.9 pass-through controls) and **M-V** (restart MIDIServer to test the MIDI plug-in). Then M-S and the unverified list (K8-K15, H-4) from the gap analysis.

## 2026-10-17 — (earlier entry) (supersedes the 2026-10-16 next-step in CLAUDE.md)
1. **M-S:** the AMS sidebar "N ins / M outs" does not follow an ADAT/S-PDIF switch on 10.13 (official driver on the SL does). First check: open AMS while already in ADAT (hypothesis: label built at device-add). Only if confirmed, consider republishing the device in the HAL (a 10.13 deviation, needs a comment naming what forced it). Evidence: captures 76-78, `Q-AVC-unit-and-O1.md` §3.
2. **PN.8:** the panel must keep a rate chosen in the panel (red = unlocked) until the kext reports another, like the official panel (capture 80), and resync without a click.
3. Continue the ladder (`tests/hw-smoke/integrated.sh`): input/loopback (jig), rate sweep, ADAT/192 kHz, MIDI, sleep/wake, 5 min stable (M-C), `kextunload` with the async AV/C queue busy (M-R).
4. Remove the bench diagnostics (O4): `AVC …` log in `FWAvc.cpp`, HSCI, mixer-write, `LogStreamState`.
5. Commit the session's work (not yet committed), then M-K, M-L, installer IN.5/IN.6 last.
Bench tools: `tools/aduplex.c` (duplex client), `tools/halwatch.c` (HAL notifications), `tools/maprop.c` (`MA_CLASS=` selects the official driver on the SL).

## BUILD phase — next actions, in order (2026-10-15; supersedes the pre-build list below)

1. **Port `openaudio-fw1814/kext/src/FWDCLProgram.cpp`** to the new `FWDCLProgram.h` (the tree does not compile until then). Spec: `docs/system-map/A1-ghidra/Q-MB-dcl-callbacks.md` §3-§6; raw decompile `ghidra-exports/decomp/q-mb-dcl-callbacks.c`. Points to mirror exactly: pool packet for DCL *k* is `(k+1) mod N`; `SetTagSyncBits(1,0)` on the first primer of each TX bank; `AdjustOutputDCL` jump table; `takeTimeStamp` at bank 3 lap >= 3; RX consumed marker `quad1 = 0x80`; RX payload at `pkt+12`. Remove `BringUpPrograms` from `FWAudioDeviceChain.cpp` (programs will belong to the engine). Then `scripts/sync-to-buildpc.sh --build` and `scripts/stage-kext.sh` (static validation only; do NOT load).
2. **Q-START** (before M-C): decompile `FWEngine2::StartAudio` slots `vt[0xd08]`/`vt[0xd10]` and the program `vt[0x128]`/`vt[0x120]`/`vt[0x158]` to learn who calls `Connect`, `channel->start()` and `ResetPort`; correct C2 §7 / Q3a if the wording "program start" was `Connect`. Also decode `Q-CMP-connect` consumers for M-C.
3. Fill the skeleton in sheet order: M-C (`FWIsochChannel`, `FWCMP`, `FWAvc`, `FWStreamControl`, `FWBusReset`), M-D (`FWBaseEngine`, rates, format change), M-E (`FWHsci`, `FWPropertyDict`, `FWMixerRegs`), M-F (user clients), M-J/M-H/M-I, M-K; helper H.3 (`Gating.c`) and H.4 (`Settings.c`); panel PN.2-PN.7 **after** PN.1 (wireframe sign-off by the user); installer IN.5/IN.6; harness IN.2.
4. After each filled item run `/close-item` (sheet `Build` status via `openaudio-fw1814/scripts/plan-status.py set <ID> "<status>" "<note>"`).
5. **User-owned gates:** PN.1 wireframe sign-off; IN.1 SIP-on check in Recovery; Developer ID (IN.6); a second FW1814 for M-K; hardware power-cycles for the integrated run (no hot-plug, no flashing).
6. Integrated hardware run (IN.3/IN.4) only when every part exists (airliner rule).

Headless Ghidra recipe: `JAVA_HOME=~/tools/jdk-21.0.12.1+1/Contents/Home ~/tools/ghidra_12.1.4_PUBLIC/support/analyzeHeadless <copy of docs/system-map/A1-ghidra/ghidra-project> fw1814-kext -process -noanalysis -readOnly -scriptPath <dir with DecompList.java> -postScript DecompList.java <hex addresses...>` (the script decompiles the function containing each address; strip the `INFO  DecompList.java> ` prefixes). Keep `DecompList.java` in `docs/system-map/A1-ghidra/scripts/` (added 2026-10-15).

## Where we are

- **Parts A, B, C: done** (2026-10-09 / 2026-10-13).
- **Part D: D2 (full resumed clone) CHOSEN by the user on 2026-10-14**; the working plan is `docs/BUILD-PLAN.md` (DRAFT; gate G1 = user approval). **Code stays frozen** until G1.
- **Current work: pre-build research.** The live status table is `docs/BUILD-MANUAL.md` §0.

## Key 2026-10-14 findings (details in `docs/STATE.md` → "2026-10-14 findings")

1. **Real cause of the reference kext's 10.13 panic:** `operator delete(NULL)` → `kfree` panic in teardown (`FWAudioDevice::free() @ 0x23b7`; `FW410Device` member dtor `@ 0x13ab3`). Three build-PC panic logs are archived in `docs/system-map/build-pc-panics/`. The panic follows an earlier init/probe/start failure **that is not yet identified**.
2. **No IOAudio/IOFireWire reserved-slot drift** between the reference's SDK and 10.13. Only `IOService` LP64 slots 0–1 were consumed (IOReporting).
3. **HSCI bytes 7/8/9 = `MAClockSource` bits 3/4/5** (optical-IN ADAT, optical-OUT ADAT, S/MUX). Not hardcoded zero, not `clk_lock`.
4. **DCL rings:** TX = 20 segments (4 banks); RX = 64 segments × 4 packets. "64-seg TX" was wrong everywhere.

## Part A status (2026-10-14, later)

Part A: 105/106 items closed. Only remaining A item: `A1-Q17.2` (effect of label 0xCF — hypothesis; capture 74 could not read labels; does not block the clone). `A1-Q-SDK.4` closed 2026-10-14: real kextload with the FW1814 on the iMac reproduced the panic (capture 75, R29); which init step fails first stays unverified. Run the `close-item` skill after each.

## Next actions, in order

0. **[DONE 2026-10-14 except 13 open flags — see `docs/STATE.md` → "provenance sweep" and the `Painel` backlog]** Apply the provenance audit: [`PROVENANCE-AUDIT.md`](PROVENANCE-AUDIT.md) (2026-10-14).** It holds 129 flags, 85 of them category 4 (citation or binary contradiction). Add dated `**Correction (2026-10-14):**` blocks to C1–C6, D, Q15c, Q4, Q5 and Q6, starting with the audit's top 10:
   - the stream-kick order and timing
   - the `vt[0xce0]`/`vt[0xaf8]`/`vt[0xb00]`/`vt[0xb10]`/`vt[0xb58]` misnames
   - the 16-byte HSCI frame
   - C5's selector and KIPC sections
   - C2's FDF/channel columns and CIP header
   - Q4's ReconnectP2P decode
   - `vt[0xbe0]`
   - FirmwareHalt in bootloader mode

   Re-run the D comparison only after these corrections land.
0k. **[DONE 2026-10-14, R39] P6:** verification oracle (`docs/system-map/P6-verification-oracle.md`). All pre-work P1-P6 done; next = your G1 review of `docs/BUILD-PLAN.md`.
0j. **[DONE 2026-10-14, R38] P4:** per-rate table + mixer power-on (`Q-P4-cmp-per-rate-and-mixer-power-on.md`). Next: P6 (verification oracle spec). Then the plan is ready for your review (G1).
0i. **[DONE 2026-10-14, R37] P3:** kext firmware path (`Q-P3-kext-firmware-path.md`); closes A1-Q18/Q19. Next: P4 (CMP per-rate + Q3b defaults), P6 (verification oracle).
0h. **[DONE 2026-10-14, R36] P1 + P2:** DCL acceptance constraints and pool-memory semantics (`Q-P1P2-dcl-acceptance-and-pool-memory.md`). Next: P3 (kext firmware path), P4 (CMP/Q3b tables), P6 (verification oracle).
0g. **[DONE 2026-10-14, R35] P5:** C1-C6 consolidated. Next: P1 (`generateBufferMap`), P2 (IOBMD masks), then P3, P4, P6.
0. **[2026-10-14] D2 chosen; pre-pivot code removed; `docs/BUILD-PLAN.md` is the plan.** Authorised now: static pre-work P1-P6 (P1 `generateBufferMap`, P2 IOBMD physical mask, P3 kext firmware path, P4 CMP/Q3b tables, P5 consolidate C docs, P6 verification oracle). Waiting on the user: approve the plan (G1), panel wireframe, Developer ID, SIP-on signing check, optional `core/` library (default no). History of the D1 detour (R30-R33) is in `PROVENANCE-RESOLUTIONS.md`.
1. **User:** ~~decide whether to re-run the D comparison~~ (done, R30); now decide whether to authorise the Q-SDK.5 read-only log check on the build PC, (directive question answered 2026-10-14: closest-to-official, D1 preferred, D2 fallback). Optionally approve a `kextutil -n` link-only dry run of the reference on the build PC (staged in `/tmp`, never `/Library/Extensions`).
2. **Ghidra-only research: done 2026-10-14** (see STATE batch 2). Remaining Ghidra work:
   - **Q-MAMAP:** Settings `Init` key registration plus `AdjustedLevels` encoding.
   - Q3b open items.
   - The bit-2 (S/PDIF port) consumer.
   - The v1.5.2 sync-radio bit values.
   - `IOFireWireNub` `vt[0x850]` identity.
3. **SL rig session** (`ssh fw1814-sl`): high-rate latency, loopback reference, live FCP capture of a clock change (confirms bytes 7/8/9).
4. **Q-MAMAP** (46-row crosswalk). Its inputs are now in place: Q15f Correction (register offsets) and Q3b (defaults).
5. **User:** panel wireframe sign-off, Developer ID.

## Environment (2026-10-14)

- **Dev Mac:** Ghidra 12.1.4 + Temurin JDK 21 in `~/tools`. Native decompiler built via `support/gradle/gradlew buildNatives`. Headless runs read-only:
  `JAVA_HOME=~/tools/jdk-21*/Contents/Home ~/tools/ghidra_12.1.4_PUBLIC/support/analyzeHeadless <proj-dir> <proj> -process -readOnly -noanalysis -scriptPath scripts -postScript X.java`
- **Build PC** `fw1814-build` (`192.168.100.20`): 10.13.6, Xcode 9.4.1, `MacOSX10.13.sdk`. Nothing of ours loaded.
- **SL rig** `fw1814-sl` (`192.168.100.17`): 10.6.8, v1.9 driver. Key `~/.ssh/id_rsa_sl_macpro`, legacy algorithms in `~/.ssh/config`.
- **Evidence paths:** `docs/system-map/A1-ghidra/otool-exports/mafw.asm` (was `/tmp/mafw.asm`), Ghidra logs under `*/ghidra-exports/decomp/`, third-party clones in `_refs/` (`_refs/README.md` has pins + recipes).

## Non-negotiables (recap)

- No new source files, no binary patches, no plist edits until D is approved.
- Every claim in `docs/system-map/**` carries provenance. Unverified claims are labelled with a verification path. Contradictions get dated `**Correction (YYYY-MM-DD):**` blocks.
- Raw disassembly beats Ghidra decompile output when they disagree. Example: HSCI bytes 7/8/9, where Ghidra mis-typed a parameter.
- Never install the reference kext into `/Library/Extensions` on the build PC.
- v1.9 (SL) vs v1.11.2 (`_extracted/`) vs v1.10.5 (`_vendor/` Sierra installer) are different builds. Name the bundle in every finding.

- [2026-10-15] M-B code done (FWDCLProgram.cpp ported; builds + kextutil -t -n OK). Q-START closed (R43). Next: M-C.
- [2026-10-15] M-C code written (R44; Q-MC). Next: M-D.
- [2026-10-15] M-D engine core written (R45; Q-MD). Next: M-D2 sample converters, then M-E.
- [2026-10-15] M-D2 sample path written (R46; Q-MD2). Next: M-E (mixer FWPropertyDict, CustomMixerWriteQuads, HSCI, SetBlankSlateClockSource).
- [2026-10-15] M-E written (R47; Q-ME). Next: M-F (user client 500-507, notifications type 0/1, peak[52] page, clientMemoryForType) and M-E2 (sleep/wake); then M-J/M-H/M-I.
- [2026-10-15] M-F + M-E2 written (R48; Q-MF). Next: M-J (4-way engine family, pair engines), M-H (MIDI), M-I (AC3).
- [2026-10-15] M-J written (R49; Q-MJ). Next: M-H (MIDI), M-I (AC3), then helper H.3/H.4.
- [2026-10-15] M-H kernel half written (R50; Q-MH). Next: M-I (AC3), M-H2 plug-in, helper H.3/H.4, panel (after PN.1), installer.
- [2026-10-15] M-I written (R51; Q-MI). All kext milestones M-A..M-J, M-I are coded; remaining: M-H2 plug-in, helper H.3/H.4, panel (PN.1 gate), installer, M-K, M-L, integrated run.
- [2026-10-15] Helper H.3/H.4 written (R52). Remaining: panel (gate PN.1 wireframe), installer (IN.5, incl. copying the MIDI plug-in), IN.1-IN.4 harness, M-K, M-L, integrated run.
- [2026-10-16] PN.1 wireframe approved by the user (A2-panel/PN1-wireframe.html, P12-panel-screens-v19.md). Panel rules: state derived from the device channel table (88.2/96 kHz greys ADAT 5-8 and S/MUX is automatic; 176.4/192 kHz = 2 in/4 out analog, digital off). Next: PN.2 (46 MA* keys -> controls), PN.3-PN.7, installer IN.5, IN.1-IN.4 harness, M-K, M-L, integrated run.
- [2026-10-16] PN.2 drafted: docs/system-map/A2-panel/PN2-control-key-map.md (control -> key map, 6 open items) + control-panel/Sources/MaKeys.swift (54-key catalog, MAClockSource codec; parse-checked only). Key count is 54, not 46. Awaiting your review of the map. Next: PN.3-PN.7 Swift panel; close the PN.2 open items by decompiling the CXDevice Choose* helpers.
- [2026-10-16] PN.2 open items 1-5 closed by decompiling the panel translator (R53, A2-panel/P13-translator-index-map.md); MaKeys.swift gained MAIndex/MARotaryMask/MARange. 5 smaller items remain open (P13 §7). Next: PN.3-PN.7 Swift panel.
- [2026-10-16] Panel PN.3-PN.7 sources written and type-checked (R54, A2-panel/P14-clone-panel-design.md): layout/model/bridge/tabs/sidebar/presets; host tests pass on the build PC. Remaining: window controller + mini-meter windows + main.swift + Info.plist + app target, then installer, harness, M-K, M-L, integrated run. Workbook Itens/Build/Painel refreshed.
- [2026-10-16] Panel app shell done: PanelWindowController, mini meters, AppDelegate/main/Info.plist, make app; demo mode (--demo --snapshot) renders all tabs; layout fixes (CardStack, flipped docs, sidebar). Next: your visual review, installer IN.5, harness IN.1-IN.4, M-K, M-L, integrated run.
- [2026-10-16] Panel redesign approved by the user in demo mode (dark minimal, EVO-inspired); level knob wired, live A/B text, centred windows, grouped mini-meter labels, multi-interface device menu, about-tab photo, window size per tab (P14 update). Next: installer IN.5 (incl. reference MIDI plug-in copy), harness IN.1-IN.4, M-K, M-L, integrated run.
- [2026-10-16] Readiness audit (docs/INTEGRATION-READINESS.md): panel<->kext keys conform (54=54); gaps added to the Build tab: M-N (CoreAudio volume/mute/clock-source controls), M-O (rate change while streaming), M-P (stub cleanup), IN.7 (conformance script). First hardware run = kext loaded from /tmp (not /Library/Extensions), helper by hand, panel, 9-step ladder; installer last.
- [2026-10-16] Readiness gaps closed: M-N was a false positive (controls exist), M-O rate-change gate implemented, M-P stubs deleted (FWAc3, FWEngine1, midi-plugin/Sources) and comments fixed, IN.7 conformance script written and passing (docs/CONFORMANCE.md; found MAMonitorAuxOnOutput = 3 elements, not 10), tests/hw-smoke/integrated.sh ladder written (preflight run OK). Next: your go for the first hardware run (user present), or IN.2 oracle compare / M-K prep first.
- [2026-10-16 late] FIRST INTEGRATED HARDWARE RUN: audio plays, CoreAudio lists the device, panel works on the real FW1814 (docs/system-map/build-pc-runs/2026-10-16-first-integrated-run.md). Fixed F1 (IOAudioEngine::registerService), F2 (panel MAMetaAllProperties), F3 (HSCI reinit), F5 live writes, F6 balance apart from faders, F8 rate gate, F9 single instance; built but unverified: F4 phones selector at start-up, F7 A/B entries 0/1, F10 HSCI in-flight read on teardown (cause of the kextunload panic). Open: O1 S/PDIF->ADAT switch while streaming hangs (diagnostics in place), O3 23 s init. Next session: load current build from /tmp (panel/audio closed), verify F4/F7/F10, reproduce O1.

## 2026-10-17 (night) - installer built; next
1. M-S engine re-publish (user said YES): in `SwitchDigitalSignals`, after the format change, quiesce the old engine (stop MIDI/isoch, disconnect plugs), `DetachAudioControlsFromEngine` on every prop, `createAudioEngine()` + `ReallyRegisterService()`, re-establish the CoreAudio controls (factor the block out of `FWSettingsLevels::Init`), carry the MIDI port arrays to the new engine (the MIDI plug-in maps their memory once), restore rate/clock state. Deviation comment: 10.13 AMS builds the sidebar label at device-add (Q-AVC-unit-and-O1 section 3).
2. MIDI wire loopback with a DIN cable; reboot autoload test of the installed kext; uninstall.sh test; clean-machine install; IN.6 signing (needs the user's Developer ID).

## 2026-10-17 (night, later) - M-S done; remaining
Re-publish verified (R60). Open: MIDI bytes after a re-publish (cable), repeated switching soak, DAW project open during a switch, rate changes that alter counts via SMUX (not re-published), install the re-publish package build (not installed yet), reboot autoload, IN.6 signing.

## 2026-10-17 (night, last)
**Correction (2026-10-17, later):** the re-publish is switched OFF by default (`kRepublishAtOpticalSwitch = false` in `FWAudioDeviceChain.cpp`). Reason: reAMP froze when the engine was replaced during playback - a re-publish removes the device from CoreAudio and clients that do not re-attach lose their stream. User decision: follow the reference/official behaviour - pause the stream, change the formats in place, resume (the clients' I/O continues after a short interruption). Consequence accepted: the 10.13 Audio MIDI Setup sidebar label can stay stale after an optical switch. The re-publish code and its idle-only deferral (`RepublishIfIdle`) stay in the tree, hardware-verified for the label (capture 83) but not for client survival.
Next: load the staged build (re-publish off) and re-test the optical switch with reAMP/aduplex playing: the stream must pause and resume by itself.

## 2026-10-17 (evening) - second Mac
R72: clean-Mac install over a Thunderbolt-to-FireWire adapter worked (MacBook Air, SIP off). Remaining install-side gaps: SIP enabled / Developer ID (IN.1, IN.6). Push the public repo and attach the rebuilt .dmg.

## 2026-10-17 (night) - decision (R73)
Signing / notarisation / SIP-enabled operation are out of scope: homebrew, always unsigned, SIP disabled. Ignore earlier next steps about Developer ID, UAKEL or a SIP-on test.
