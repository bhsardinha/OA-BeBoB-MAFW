# NEXT_STEPS — resume anchor (rewritten 2026-10-15)

> **Removal notice (2026-10-14):** all pre-pivot code and docs (`_legacy_pre_ghidra/`, `openaudio-fw1814/firmware-helper/`, `openaudio-fw1814/legacy-mod/`) were deleted from the working tree at the user's request. They are recoverable from git: `git show d1-approved-2026-10-14:<old path>` (tag `d1-approved-2026-10-14`, commit `a9cb63c`). References to those paths in this file or in `docs/system-map/` are historical. `openaudio-fw1814/` is intentionally empty; new code is built from scratch per `docs/BUILD-PLAN.md` once the user approves it.

Read this first after a context clear, then `NEW_PLAN.md` → `docs/STATE.md` → `docs/BUILD-MANUAL.md` §0 → `docs/system-map/README.md`.

## 2026-10-27 (night) - stereo-pair fix verified (R141); v0.4.1 (both editions) built and published
Done: 0.4.1 Modern installed on the dev Mac, pairs have channels at 48 kHz (user + capture 107). Classic and Modern 0.4.1 packages and `.dmg`s built; README simplified (user: every interface and feature as supported in 0.4.1); release v0.4.1 with both `.dmg`s. Next (user decides): FW410 MIDI, S/PDIF, sleep/wake; nothing else pending from this item.

## 2026-10-27 - 0.4.0 documented, packages rebuilt
Next (user decides): try the 0.4.0 Modern package (dev Mac: install + restart, never unload the old 0.2.9 by hand); re-test the FW1814 / Audiophile on 0.4.0; FW410 MIDI, S/PDIF, sleep/wake; v0.4.0 is published (release + dmgs, 2026-10-27).

## 2026-10-26 (late) - FireWire 410 on the Classic iMac, 0.4.0 Classic installed (R136-R138)
Next (user decides): MIDI, S/PDIF in/out + external sync, coaxial/optical switch, sleep/wake with the 410; the kextunload FWHsci leak (R138); the Modern 0.4.0 build (dev Mac kext + build PC panel) and the dev Mac test; stereo pairs on the FW1814 (user tests it himself); README/NOTICE + release when the user says. The 410 is on the iMac's FireWire bus (alone).

## 2026-10-26 - FireWire 410 phase 0 (R135)
Next: the user approves `docs/BUILD_FW410_PLAN_v1.md` and answers §9 (versions, hidden features, first-run machine). Then, on branch `fw410`: desk B-3a/B-5, base B-1..B-6, model M-1..M-9, panel F4, F3/F5/F6, run F7. The FW410 is on the SL rig (alone on the bus; rig at 96 kHz as found).

## 2026-10-22 (late night) - panic on wake fixed in source, 0.3.2 Modern built (R133)
Next: the user installs `openaudio-mafw-bebob/dist/OA-BeBoB-MAFW-0.3.2-modern.pkg` on the dev Mac (Allow + restart), then real use with sleep/wake. On request: build Classic 0.3.2 (build PC) and the `.dmg`s, commit, publish v0.3.2 (sync + `gh release`). The 0.3.1 README lists the panic as a known issue until then.

## 2026-10-22 (night) - Uninstall app progress (R132)
Uninstall app shows `<n> % — <step>` and a 0-100 % bar; `--dry-run` [`--start`] for looking without uninstalling. Shipped 2026-10-22 as a silent update of 0.3.1 (both `.dmg`s replaced on the release, packages unchanged); a real uninstall with it is untried.

## 2026-10-22 (evening) - follow the external clock rate (R131, AP-5.7)
Done: package **0.3.1** (driver 0.2.2, panel 1.5); Modern installed on the dev Mac and working with the second FW1814 + POD X3 (user, capture 99).
Built 2026-10-22: `dist/OA-BeBoB-MAFW-0.3.1-classic.pkg/.dmg` (kext 0.2.2 `3FA69E4A-…`) and `-modern.pkg/.dmg` (the installed package); committed `624317c`. **Published 2026-10-22:** public `d4a1a68`, tag v0.3.1, GitHub release (both `.dmg`s, user). Open (on request): Classic 0.3.1 on a 10.13 machine; 10.14/10.15 (allowed by Classic, untried); README screenshots without the new Follow option.

## 2026-10-22 - scope floor 10.9, Legacy dropped (R130)
User: the older systems work well on the official drivers, so the plan now starts at 10.9 (`BROAD_COMPATIBILITY_PLAN_v4`). Cancelled: the Legacy edition, the Obj-C panel port, the i386 slice, the 10.6 toolchain. Remaining in scope, all on request: Classic down to 10.9-10.12, more Modern checks, Apple Silicon (hardware + DEC-3).

## 2026-10-21 (night, last) - compatibility docs updated after the Sonoma result (R129)
The Modern edition works on macOS 14.8.9 Intel (Audiophile), so the broad-compatibility documents were brought up to date: `BROAD_COMPATIBILITY/BROAD_COMPATIBILITY_PLAN_v4.md` + `.xlsx` (v1, v2 kept), `docs/EDITIONS.md` (what differs from the High Sierra build), `docs/MACOS12_PORT_PERSPECTIVE.md` (status note), README. Public docs layout is done by `sync-to-public.sh` (see R129).
* **Plan v3 order, all on request (R123):** more Modern checks (FW1814, record, MIDI, rates, other versions of 11-15) -> publish 0.3.0 with both editions -> Apple Silicon (needs hardware, DEC-3) -> Legacy (only if wanted). DEC-1...DEC-4, DEC-6, DEC-8 are still open and gate those last two.
* **Before publishing 0.3.0:** commit this working tree (the sync refuses a dirty tree), `sync-to-public.sh --dry-run`, review, then the real sync; the user pushes and attaches the two dmgs.

## 2026-10-21 (night) — broad compatibility plan v1 waiting for decisions (R124) [superseded by v3, R129]
`BROAD_COMPATIBILITY/BROAD_COMPATIBILITY_PLAN_v1.md` (+ `.xlsx`, `evidence/`) is a proposal for one OpenAudio BeBoB MAFW covering 10.6.8 to 15.
* **Nothing starts** until the user answers DEC-1...DEC-10 (plan §10, `Decisions` tab). The main ones are the lowest OS, Apple Silicon, the panel strategy for 10.6-10.8 and packaging.
* **Recommended order:** S1 (Intel 10.9-15), then S2 (Apple Silicon, needs hardware), then S3 (10.6.8-10.8.5) only if wanted.
* **Still to read:** the 10.6.8 symbol sets on the SL rig, which did not answer SSH on 2026-10-21 (BC-1.1).

## 2026-10-21 (evening) — READ THIS FIRST: testing closed, nothing planned (R123)
Package 0.2.2 is published and in use; the user reports everything working (S/PDIF lock with a POD X3 at every rate with the rate set by hand; Uninstall app and status menu work). **No formal tests from now on (user):** the remaining checks are on demand — run one only if real use shows a problem. They are listed in R123 and as `Sob demanda (uso real)` in both `Build` tabs. Plan `docs/BUILD_AUDIOPHILE_PLAN_v9.md`.
Possible work, only when the user asks: remove the bench diagnostics compiled into the kext; external S/PDIF rate following (compare with the official driver first: `DetectExternalSyncChanges`, `w2` encoding unverified); AP-8.5 panel/port manual; other BeBoB models (needs hardware). Entries below are history.

## 2026-10-21 (late morning) — package 0.2.2
Package **0.2.2** = driver 0.2.0, panel **1.3** (renamed `OA BeBoB MAFW.app`, firmware check inside the panel), OA names under icons, update-in-place installer, uninstall self-kill fix (R121; versions R120; CPU R119). Built: `openaudio-mafw-bebob/dist/OA-BeBoB-MAFW-0.2.2.dmg`. Next: user installs it on the Air over 0.2.0/0.2.1 (first real update test: no manual uninstall, settings and launch at login kept, About shows package 0.2.2 / panel 1.3 / driver 0.2.0, CPU with panel + big meter, the Uninstall app's button later); AP-8.3 drafts must follow the new names (no separate Firmware Tool).

## 2026-10-21 (later) — 0.2.1
0.2.0 installed and working on the MacBook Air (user, 2026-10-21). Panel CPU fixed and big meter segmented (R119); version **0.2.1** (user), package + dmg rebuilt. Next: install 0.2.1 on the Air and look at CPU in Activity Monitor with the panel and the big meter open (Metal-capable GPU; not measured yet); then the items below (AP-5.5/5.6 on hardware, AP-5.3b, AP-8.3 docs — the README draft must say 0.2.1).

## 2026-10-21 — 0.2.0 wrap-up
Plan **`docs/BUILD_AUDIOPHILE_PLAN_v8.md`**; resolutions up to **R118**. Done: two units on the iMac (R117); menu bar mode AP-5.5 and big meter AP-5.6 coded, panel tests pass; package/dmg 0.2.0 built (`openaudio-mafw-bebob/dist/`, git-ignored) and checked statically (R118).
Next, in order:
1. **AP-8.2 on the MacBook Air (user):** copy the dmg, run the installer (sudo password), before restarting read `/var/log` helper output / `ps` for the helper (R118 hypothesis), restart, then FW1814 through the Thunderbolt adapter: device in AMS, `loopcheck`, panel, Firmware Tool, Uninstall app button on a real install, dmg window layout.
2. **AP-5.5/5.6 on the hardware (user):** every status-menu item on the checked unit, icon online/muted/no sync, big meter with music.
3. **AP-5.3b:** device-menu checklist with both units (one window, `unit n` entries, ⌘n, size follows model).
4. **AP-8.3:** drafts `docs/README_0.2.0_v1.md` + `docs/NOTICE_0.2.0_v1.md` written 2026-10-21; close their [pending] lines after steps 1-3 and AP-8.1, take the screenshots, then public sync only on your go-ahead. AP-8.4 release (user).
Still open from the ladder (equipment): AP-6.2 MIDI DIN loopback, AP-7.5 S/PDIF sync lock/unlock, AP-7.8 sleep/wake, power-cycle bus reset (done once, R117), 1 h soak, AP-3.9 kextunload x5, AP-8.1 full 1814 ladder on 0.2.0.

## 2026-10-20 (night, 23:30) — 0.2.0 state and next steps
Plan **`docs/BUILD_AUDIOPHILE_PLAN_v7.md`** (v7 = v6 + AP-5.4 monitor level law). Resolutions up to **R116**; capture 96 = the Audiophile-alone run. Session script `openaudio-mafw-bebob/tests/hw-smoke/phase7.sh` (+ `p7util.py`; `ma-watch` records MA* changes). Tracker Build: 45/56 done.
Done on the hardware (Audiophile alone, R115): bring-up, channel order, rates 44.1-96, factory = official, AC-3 formats, MIDI plug-in, Firmware Tool, persistence, panel, front panel. Done in the session (R116): headphone safety (helper + panel reset), monitor law for output / phones / aux-out (dB-linear 72 dB, centre -36, bottom off; knob 1.5 dB per click by default, 3 dB or official selectable). Kext with factory 1.5 dB: UUID `B034F8E6…` (staged in /tmp before the shutdown; `stage` again after boot).
Next, in order (user, 2026-10-20: finish 0.2.0, then two units, then the Air):
1. ~~Two units together on the iMac~~ **done (R117, capture 97)**: panic of the installed 0.1.0 on unload (leaked host PCR plug; reference has the same leak, it never unloads) → 0.2.0 fix `FreeHostPlugs` (kext `78A039D0…`); iMac cleaned (0.1.0 uninstalled, settings in `~/oa-backup/`); both units up from scratch, both Current, 1814 six rates, both streaming 30 s, power-cycles recovered. The iMac now runs NOTHING installed: the 0.2.0 test kext from /tmp + helper from `~/oa-build-mafw` (a reboot leaves it without any driver).
2. **AP-8.2 installer 0.2.0**: package with kext, helper, panel, Firmware Tool (check only, no images), MIDI plug-in; removes the 0.1.0 kext/helper/panel files; **never unloads a running driver (R117): install/uninstall ask for a restart**; uninstaller; dmg; `check-self-contained.sh`. Build on the iMac.
3. **Install on the MacBook Air** (user runs the installer: sudo there needs a password); clean-install check with the FW1814 through the Thunderbolt adapter.
4. AP-8.3 README/NOTICE (monitor law, headphone safety, Firmware Tool check-only), AP-8.4 release (user), AP-8.5 panel manual later.
Later (equipment): MIDI DIN loopback, AC-3 to an S/PDIF receiver, S/PDIF sync lock/unlock, sleep/wake, bus reset by power-cycle, 1 h soak. Known issue: encoder clicks can be lost on fast turns (direction-only status per 50 ms read, official behaviour; worn encoders); measured software path ~65 ms worst case.
Side items: update the iMac's installed 0.1.0 kext to the current 0.1.0 source (R75/R76) when the user wants; the 1814 aux could get dual-mono pan like the Audiophile if the user asks (official 1814 has bal only).

## 2026-10-19 (night) — plan v3 phase 1 done (AP-1.1…1.4); next AP-2.1
Capture 95/R94: renamed driver on the iMac behaves like 0.1.0. Next: AP-2.1 (kDeviceSpecs 8 rows + personalities), then AP-2.2…2.4, no hardware. **Separate pending item:** the iMac's installed 0.1.0 kext is older than the 0.1.0 source (lacks R75/R76: second-client loop, frozen meters) — update it via stage-kext + install.sh in `openaudio-fw1814/` when the user wants.

## 2026-10-19 (evening) — Audiophile plan v3 waiting for approval
Plan: **`docs/BUILD_AUDIOPHILE_PLAN_v3.md`** (v2 kept as `_v2.md`). Product OpenAudio BeBoB MAFW 0.2.0, one driver for 1814 + Audiophile, official class hierarchy for multi-device, panel = new 1814 panel extended, first load on the iMac. On approval: start AP-1.1; no code before.

## 2026-10-19 (later) — Audiophile: static phase 0 done; next = SL rig session (user), then plan v3
1. AP-0.9 on the SL rig (user present): each panel control one at a time (setProperties capture), FWDJ screenshots, factory reset values (−6 dB question), ioreg at 44.1/48/88.2/96, listen for mixing without streaming. Optional read-only reads (user OK): `0xFFC7:0x00600000` with S/PDIF sync (rate byte), `EXTENDED PLUG INFO` (channel order).
2. ~~Write plan v3~~ done: `docs/BUILD_AUDIOPHILE_PLAN_v3.md`, written from the tracker (reuse map, new work list, task order, acceptance) for approval. No code before approval.

## 2026-10-19 — Audiophile phase 0 (branch `audiophile-phase0`)
1814 work is unchanged (entries below). Audiophile: follow `docs/BUILD_AUDIOPHILE_PLAN_v2.md` §11 (superseded by `_v3.md`) and the `Painel` of `PROVENANCE-MATRIX-AUDIOPHILE.xlsx`. Next, in order: AP-0.4 (asm of `ReadHSCIData @0xba92`, LED vendor command, HSCI period) → AP-0.3 (AV/C function-block bytes from the settings classes) → AP-0.5/0.6/0.7 → helper `.bcd` reader and MIDI plug-in mapping → AP-0.9 on the SL rig with the user → final plan for approval. **No code until the plan is approved.**

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

## 2026-10-21 (late) - macOS 14.8.9 port (R125)
Done (R126): `scripts/install-modern.sh` (not `install-sonoma.sh`), load, play. See the entry below.

## 2026-10-21 (late night) - after R126 / R127
1. **User:** run `openaudio-mafw-bebob/scripts/install-modern.sh` to put the R127 kext (knob law: phones only) on the dev Mac; it also installs the Modern panel from `control-panel/build/modern/`. Approve if asked, then restart. Then check by ear: the hardware knob on the outputs uses the official law, and the phones knob uses 1.5 dB.
2. **Dropout (R126), on demand:** if it comes back, try the interface on its own power supply, and check the adapter contacts.
3. **At the next release:** update the README (line 40 monitor law, lines 53-55 status menu) for R127; set the component versions (driver changed: knob law; panel changed). Classic panel built with `BAND=classic` (default). Re-render the README panel images (output page and about page changed).
4. Modern edition still open: record/RX, panel controls against the real kext (U-8), MIDI, other rates, FW1814 on 14 (plan v2 BC-6.1).

## 2026-10-21 (night, later) - after R128
1. **Build PC, when idle (ask first):** `sudo installer -pkg ~/oa-pkg-030/OA-BeBoB-MAFW-0.3.0-classic.pkg -target /`, then restart (user). It replaces today's script install.
2. **Dev Mac (user):** open `openaudio-mafw-bebob/dist/OA-BeBoB-MAFW-0.3.0-modern.dmg` and run *Install OA BeBoB MAFW.pkg*; Allow in Privacy & Security, then restart. Then check: the About tab (Modern, Edition row), the knob law on outputs, phones linear, the status menu (click / right click).
3. **Still unverified:** the Modern uninstall (`kmutil rebuild` with "y" piped, Uninstall app on 14), the FW1814 on the Modern edition, the dropout of R126 (on demand).
4. **Before any public release of 0.3.0:** README (requirements per edition, R127 law, status menu, images), `sync-to-public.sh`; the user attaches the dmgs.
