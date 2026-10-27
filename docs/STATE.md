# STATE — M-Audio FireWire 1814 driver, current project state

> **Removal notice (2026-10-14):** all pre-pivot code and docs (`_legacy_pre_ghidra/`, `openaudio-fw1814/firmware-helper/`, `openaudio-fw1814/legacy-mod/`) were deleted from the working tree at the user's request. They are recoverable from git: `git show d1-approved-2026-10-14:<old path>` (tag `d1-approved-2026-10-14`, commit `a9cb63c`). References to those paths in this file or in `docs/system-map/` are historical. `openaudio-fw1814/` is intentionally empty; new code is built from scratch per `docs/BUILD-PLAN.md` once the user approves it.

Snapshot as of **2026-10-27**. Updated with each Part A/B/C/D commit and each build milestone.

Resume anchor: `/NEW_PLAN.md` → this file → `system-map/README.md` → the in-progress Part N.

## 2026-10-27 (night) - stereo-pair devices without channels: cause found, fixed in source (R141); driver 0.3.1 / package 0.4.1
User (dev Mac, Modern 0.4.0, FW1814 at 48 kHz): the pair devices appear without channels. Capture 107: no pair stream has `IOAudioStreamFormat`; IOAudioFamily 540.3 `setFormat` refuses a format outside the engine's current rate, and `FWPairEngine::ResetIOFormat` (reference order) set the rate after `setFormat`, so only 44.1 kHz (the rate `initHardware` sets, the rate of the 410 check in capture 106) worked. Fix: `setSampleRate` first (deviation, R141). User chose to fix the driver and keep the option for every model. Package 0.4.1 Modern installed on the dev Mac: every pair device has its channels at 48 kHz (capture 107 `after-0.4.1-*`; user: "worked as intended"). Then (user): Classic 0.4.1 built, README / release notes / installer pages / dmg text simplified (every interface and feature listed as supported in 0.4.1, no per-version test notes), panel 1.7: the launch-at-login LaunchAgent is rewritten only when it points at another copy of the panel (it was rewritten at every launch, and macOS 14 showed "Background Items Added" each time; user); postinstall scripts likewise; v0.4.1 published (see NEXT_STEPS). Public README: known issue 10 + note in the stereo-pairs section; CHANGELOG 0.4.1.

## 2026-10-27 (later) - 0.4.0 documentation for three interfaces; both packages rebuilt
README rewritten around the FireWire 1814, Audiophile and 410 (interfaces table, 410 results, stereo pairs, unload fix, honest status: 0.4.0 Classic tested with the 410 only; Modern 0.4.0 built, not tried; 1814/Audiophile not re-tested on 0.4.0); NOTICE, EDITIONS, CHANGELOG 0.4.0, release notes `installer/release-notes/RELEASE-0.4.0.md`, dmg art text and the Classic welcome page updated. FW410 About photo (cut-out of a product photo, 624x118). Packages `dist/OA-BeBoB-MAFW-0.4.0-{classic,modern}.pkg` rebuilt. **Published 2026-10-27:** public repo `9e5c2a2`, tag `v0.4.0`, GitHub release v0.4.0 with `OA-BeBoB-MAFW-0.4.0-classic.dmg` (SHA-256 `965518e2…`) and `-modern.dmg` (`20d2f07e…`), created with `gh` (digests on GitHub match).

## 2026-10-27 - unload and power-off panics fixed (R139, R140)
`kextunload` failed on the first try (status poller waiting for its last read): `stop()` now waits for it (R139). Two panics when the user switched the 410 off: a host PCR plug left registered by an earlier unload (any unload, idle or streaming) called into the unloaded image at the next bus reset (= R117's panic; its fix never worked). `stop()` now frees the host plugs (`Close()`), verified by the plug slot the next driver gets (always 0) after idle / streaming unloads and a power-cycle (no panic). Installed on the iMac: package 0.4.0 Classic, kext ED7E32BD. Capture 106.

## 2026-10-26 (late) - FireWire 410 works on the Classic iMac; package 0.4.0 Classic installed there (R136-R138)
Branch `fw410`: kext model `models/FW410` + generic base additions (192 kHz channel maps, LLRR10 procs, legacy engine UID, pair-engine UID, SW-return routing hooks), panel 1.6 (`Models/FW410` own layout, CoreAudio Devices card for every model, multi-select phones, five level-controller targets), driver 0.3.0, package 0.4.0. iMac (10.13.6, FW410 alone): attach from bootloader and from running firmware, 4/10 at 44.1-96, 2/8 at 176.4/192, every output by meter, recording at every rate, latency within 0.5 ms of a measured loop, stereo pairs identical to the official driver, mixer writes; by the user: phones volume, Level Controller with the panel. Phones bit 5 = aux bus, now cleared with the mask (deviation, user, R138). The unit's phones pass only the right side (hardware, also under v1.9). Installed through `OA-BeBoB-MAFW-0.4.0-classic.pkg`; after restart everything comes up by itself (capture 106). Open: kextunload leaves one FWHsci (R138); MIDI, S/PDIF / external sync, optical switch, sleep/wake not checked; Modern edition not built.

## 2026-10-26 - FireWire 410, phase 0 done; plan v1 waiting for approval (R135), branch `fw410`
0.3.2 work committed on `main` (`3fb0ded`). FW410 on the SL rig (v1.9): reference map `A1-ghidra/Q-410-0-reference-map.md` (Rev2 device with a **register mixer**; 4/10 at ≤96 kHz, 2/8 at 176.4/192; coax/optical S/PDIF; one encoder; 76-byte status), official panel map `A2-panel/PN410-control-key-map.md` (204 functions; its own UI), captures 102/103 (stereo pairs = 5 CoreAudio devices; 263 panel writes; rates). Plan `docs/BUILD_FW410_PLAN_v1.md`, tracker `PROVENANCE-MATRIX-FW410.xlsx`. No driver/panel code yet.

## 2026-10-22 (late night) - panic on wake from sleep fixed in source; package 0.3.2 Modern built (R133)
Dev Mac (14.8.9, Modern 0.3.1, one FW1814 `0xD6C040074B9FB`) panicked 5.9 s after wake: `StartQueueHead+0x52`, an AV/C command refused at once (interface off the bus) was released by its own callback inside `submit` and again by the failure path. Fix: retain across submit + still-head check (`FWAvc.cpp`); driver **0.2.3**, package **0.3.2** (test build). `dist/OA-BeBoB-MAFW-0.3.2-modern.pkg` built (kext `4EB8D407-…`); not installed yet; Classic 0.3.2 not built.

## 2026-10-22 (night) - Uninstall app: status line + progress bar, dry run (R132)
Both editions share the app; dry run checked on 14.8.9 (capture 100); shipped as a silent update of 0.3.1 (both `.dmg`s replaced on the v0.3.1 release; packages unchanged).

## 2026-10-22 (evening) - external clock rate: detection measured, follow feature in code (R131)
* Capture 98 (SL rig, official 1.9, FW1814, POD X3 on S/PDIF coaxial): within the interface's speed range (44.1/48 or 88.2/96) the status word gives the source's exact rate; outside it nothing. The official driver does not follow.
* Code (not yet on hardware): kext publishes `MAExternalSyncRate`; panel `RateFollower` follows through CoreAudio when idle, one try of the other range, setting on by default (`docs/RATE_FOLLOW_PLAN_v1.md`). Both kexts build; panel tests and typecheck pass on the build PC. Tracker AP-5.7.
* **Works on the hardware (user, capture 99):** package 0.3.1 Modern (driver 0.2.2, panel 1.5) on the dev Mac with a second FW1814 (`0xD6C040074B9FB`, first FW1814 on Modern) and the POD X3: out of range it tries the other speed range, in range it takes the exact rate and locks. 0.3.1 Classic package + both `.dmg`s built (Classic kext `3FA69E4A-…`, not installed anywhere yet). **Published 2026-10-22:** public `d4a1a68`, tag v0.3.1, GitHub release with both `.dmg`s (user).

## 2026-10-22 - scope floor 10.9; Legacy edition dropped (R130)
Plan v4: bands and editions are Classic (10.9 target, installer from 10.13) and Modern (11-15); Apple Silicon open. The SL rig remains a reference, not a target.

## 2026-10-21 (night, last) - compatibility docs after the Sonoma result (R129)
* The Modern edition (macOS 11-15 Intel, built with the 15.2 SDK) works on 14.8.9 with the FW Audiophile; Classic (10.13) is the shipped 0.2.2 line. Both 0.3.0 packages are built, not published.
* **Finding:** the Modern kext is the Classic source: `OA_BAND_MODERN` is unused by any source file. Differences are SDK, load procedure (kmutil, Allow, restart), installer and panel caption. Full list and open items: `docs/EDITIONS.md`.
* Documents: plan v3 (`BROAD_COMPATIBILITY/`), `docs/EDITIONS.md`, `docs/MACOS12_PORT_PERSPECTIVE.md` status note, README, `docs/README.md`; `sync-to-public.sh` reorganises the public docs (R129). Nothing built, loaded or installed for this.

## 2026-10-21 (night) — broad compatibility plan v1 (R124), proposal only
At the user's request: `BROAD_COMPATIBILITY/BROAD_COMPATIBILITY_PLAN_v1.md` + `.xlsx` cover one product for 10.6.8 to 15.
* **Verdict:** one package and one version is feasible, with 2 kext builds inside it. **Legacy** is i386 + x86_64 against an old SDK. **Main** is x86_64 against the 10.13 SDK plus arm64e against a 15 SDK. The panel would need an Obj-C port for 10.6 to 10.8 (Swift's floor is 10.9). Apple Silicon needs a MIDI clone.
* **Static facts (R124):** the `_darwin14` / IOReporting imports block old kernels. The 0.2.2 kext resolves on 14.8.4 Intel except two reserved slots. The FireWire and IOAudio families ship for arm64e. The 0.2.2 panel starts on 14.8.9.
* **Effort:** S1 Intel 10.9-15 is 13-18.5 sessions; the full range is 33.5-57.5.
* **Status:** decisions DEC-1...DEC-10 are open. No code was changed.

## 2026-10-21 (evening) — user trials; testing closed (R123) — read this first
Package **0.2.2** (driver 0.2.0, panel 1.3) is published (v0.2.2, R122) and installed on the MacBook Air by update. User report: S/PDIF from a POD X3 locks at every rate, but the driver's rate must be set by hand (not detected; official behaviour not compared); the Uninstall app and the status menu work; everything works in daily use. **User decision: no formal tests.** The remaining checks are `Sob demanda (uso real)` in both trackers and run only if real use shows a problem; soaks dropped. Plan `docs/BUILD_AUDIOPHILE_PLAN_v9.md`. Trackers: Audiophile 53/53 done (6 on demand), 1814 38/38 (11 on demand). Nothing is planned; next work only on the user's request (diagnostics clean-up, other models).

## 2026-10-21 (afternoon) — public repo v0.2.2; CPU on the Air (R122)
`sync-to-public.sh --tag v0.2.2 --push` → public commit `49b1c6e`. 0.2.2 installed on the Air as an update over 0.2.1; panel worst case about 5 % CPU there. The dmg release asset is the user's.

## 2026-10-21 (late morning) — package 0.2.2 (R120, R121)
Separate version numbers (package / driver / panel) with CHANGELOG.md; installer updates in place; panel renamed OA BeBoB MAFW with the firmware check built in; OA names under icons; dmg background for both models; uninstall no longer matches the Uninstall app's own process. Driver unchanged (0.2.0).

## 2026-10-21 (later) — 0.2.1: panel CPU fix, segmented big meter (R119)
0.2.0 worked on the MacBook Air (user), but the panel / big meter used 30-40 % CPU. Meters are now GPU-composited layers with cached segment images (official CMAVUMeter method); panel 95 % -> 2 %, big meter 20 % -> 1.4 % on the iMac. Version 0.2.1 (user), panel 1.2; release notes `openaudio-mafw-bebob/installer/release-notes-0.2.1.md`.

## 2026-10-21 — panel menu bar mode + big meter (AP-5.5/5.6), installer 0.2.0 built and checked statically (R118)
Panel host tests pass; the package holds kext, helper, panel, Firmware Tool (check only), M-Audio MIDI plug-in, icons and uninstaller; it removes 0.1.0, never unloads, asks for a restart; self-contained guard passes. Not yet on any Mac: install on the MacBook Air (user), menu/meter on the hardware. Next: Air install, then AP-8.3 docs.

## 2026-10-20 (night) — phase 7 with the Audiophile alone (R115, capture 96); monitor law + headphone safety (R116, plan v7 AP-5.4)
The 0.2.0 driver ran the FireWire Audiophile on the iMac: bootloader 0x10060 -> runtime, channel order confirmed (device meters + receiver), loopback 9.1 ms, rates 44.1-96, factory state = official (34/34), persistence, panel and front panel checked by the user. New, user-requested: helper/panel headphone ceiling, dB-linear monitor law for output-tab levels with a per-unit knob step (1.5 dB default). Next: both units together, then installer 0.2.0 and the MacBook Air.

## 2026-10-20 (later) — phase 7 preparation done (R114)
Session script `openaudio-mafw-bebob/tests/hw-smoke/phase7.sh` + `p7util.py` (recorders for bus nodes and `MADebugTrace`), `OA_DEV` unit selection in the bench tools, `maprop setxml` (settings write-back, used by the factory and persistence tests). Dry run of the read-only steps on the iMac OK; the state-changing steps are unexercised until the session. Open: AppleFWAudio vs the Audiophile on 10.13 (checked by `pre`/`bringup`); cabling topology (user).

## 2026-10-20 (late) — plan v6: Firmware Tool is check-only; phase 9 closed at AP-9.1 (R113)
User decision: the Firmware Tool is an add-on that checks a unit against the official 1.11.2 build of its model; AP-9.2…AP-9.5 (kext update path, update flow, re-flash test, public image source) are shelved (`docs/BUILD_AUDIOPHILE_PLAN_v6.md`). Finding (R113): the official driver has **no minimum firmware**; its helper re-flashes automatically, without asking, whenever the unit's date is not equal to the bundled `.bcd` (`isEqual:`), on by default (the official pkg installs no inhibit file). Tool "Older"/"Newer" notes and the panel About tooltip reworded; host tests, panel `make test`, tool `make test` ALL PASS on the iMac; demo snapshot `~/oa-snap/fwtool/v6-demo.png`. Audiophile live check = new AP-7.9. Custom firmware: not justified (no observed defect is the firmware's).

## 2026-10-19 (night) — plan v3 approved; AP-1.1 done
`openaudio-mafw-bebob/` now holds the 0.1.0 tree renamed to **OpenAudio BeBoB MAFW** (kext `com.openaudio.OpenAudioBeBoBMAFW` 0.2.0, helper/panel/pkg/uninstaller ids `com.openaudio.bebobmafw.*`, panel 1.1). Builds on the iMac in `~/oa-build-mafw` (kext, helper, panel; panel tests ALL PASS). `openaudio-fw1814/` unchanged. Next: AP-1.2 scripts, AP-1.3 staging. AP-1.4 needs the 0.1.0 kext unloaded first (same `com_openaudio_*` class names) and the user's OK.

## 2026-10-19 (later) — Audiophile phase 0: static mapping complete (R77-R89)
Write-ups: `A1-ghidra/Q-AP0-nub-and-rev2.md` (nub, Rev2, clock, layout, start-up/sleep, channel table, generic paths), `Q-AP0-status-packet-front-panel.md` (status packet, encoders, A/B, LEDs, Level Controller), `Q-AP0-avc-mixer.md` (AV/C frames, function-block map, routes, keys, factory state), `Q-AP0-converters.md`, `A2-panel/PNAP-control-key-map.md` (panel FWDJ = Audiophile, cookies, meters), `A4-helper/HAP-helper-firmware-midi-audiophile.md`. Tracker `PROVENANCE-MATRIX-AUDIOPHILE.xlsx`: 55 items (47 closed), 0 open flags. Key reuse finding: the 1814 clone already has the AV/C function-block builders, the channel-group engine and the property framework; new work = Rev2 base (0x2F, SIGNAL SOURCE, control packets, blank-slate clock), Audiophile tables/settings classes, converters, status packet, LED, panel descriptor.
Remaining before the final plan: AP-0.9 on the SL rig with the user (+ two read-only reads if authorised); decisions Q1-Q5 and the AP-4.1/4.4 scope cut. Hardware-only items: bootloader Model_ID, physical channel order, boot from flash, SW-return level blocks.

## 2026-10-19 — Audiophile phase 0 started (branch `audiophile-phase0`)
Plan `docs/BUILD_AUDIOPHILE_PLAN_v2.md`; tracker **`PROVENANCE-MATRIX-AUDIOPHILE.xlsx`** (separate from the 1814 workbook; status script `openaudio-mafw-bebob/scripts/ap-status.py`). **User rule:** map and disassemble everything, organize it in the xlsx, plan approved — **no code before approval**; the official v1.11.2 driver is the main reference.
* Done (R77, `system-map/A1-ghidra/Q-AP0-nub-and-rev2.md`): AP-0.1 (device table, nub selection by product name) and AP-0.2 (Rev2 base: `0x2F` stream format, SIGNAL SOURCE sync, Audiophile clock = internal / S/PDIF only, latencies) — verified in asm, cross-checked with FFADO and ctl-services. 141 new decompiles (tag `ap0`). Corrections in Q16, Q10, `audiophile-manual-reference.md`.
* The Audiophile is attached to the SL rig (official v1.9); captures 89/89b/90 exist. Open: AP-0.3 mixer bytes, AP-0.4 status packet asm + LED, AP-0.5-0.7, helper/firmware, AP-0.9 rig captures (needs the user).

## 2026-10-17 (later) — PN.8, helper verified, gap analysis
Details: `system-map/GAP-ANALYSIS-2026-10-17.md` (official vs clone, every difference with a status), `PROVENANCE-RESOLUTIONS.md` §R57, captures `79`-`81`.
* **PN.8 closed:** the panel sets the rate through CoreAudio (`setNominalSampleRate:` after 1 s) and shows CoreAudio's rate, as the reference (P8 §6); user-confirmed.
* **Helper = persistence owner (A4 H11), now verified:** installed H.4 build writes `com.m-audio.device-settings.firewire` (key = GUID, 0600, user-owned) and replays after a kext reload with the panel closed; the panel's duplicate "last settings" was removed. Bug fixed: the helper pinned the device object (`kextunload` exit 3) — release on `kIOMessageServiceIsTerminated`; install/uninstall scripts stop the helper first.
* **Engine UID** now `com_m_audio_FWEngine2:0x<GUID>` like the official (apps that stored the clone's old UID re-select once).
* **Decisions pending (yours):** M-U (v1.9's 19 pass-through controls: keep v1.11.2 or add), M-V (OK to restart MIDIServer to test the official MIDI plug-in). **Open:** AMS sidebar label (M-S), K8-K15 unverified list in the gap analysis (sleep/wake, bus reset, MIDI, AC3, latency, boot install of the current kext).
* SL restored to its original state (clock `0x40000004`, 48 kHz, locked). Nothing committed yet.

## 2026-10-17 — second integrated session: optical switch, AV/C unit, routing
Details: `system-map/A1-ghidra/Q-AVC-unit-and-O1.md`, `system-map/PROVENANCE-RESOLUTIONS.md` §R56, run log `system-map/build-pc-runs/2026-10-16-first-integrated-run.md` ("Second session"), captures `B5-sl-dtrace-truth/captures/76`-`80`. SL v1.9 was used as ground truth (ioreg, halwatch, panel screenshots).
* **O1 resolved:** `stopAudioEngine` must call `performAudioEngineStop` (not `pauseAudioEngine`); ADAT/S-PDIF switch under duplex load works, CoreAudio 16/12 <-> 10/6.
* **AV/C layer rebuilt like the reference:** `FWAvcUnit` is an `IOFireWireAVCUnit` subclass (init/attach/start as asm 0x23c72, `fCommand`/`avcLock`/`cmdLock`, async queue). Before, no FCP response ever arrived (all commands timed out 1.27 s; bring-up 23 s; SW return 1/2 -> out 3/4 never sent). Now 22 ms, bring-up 13 s (5-14 s), routing works (user-confirmed).
* `FWHsci::Release` no longer cancels synchronously (kextunload hang); 5+ clean unload/reload cycles incl. with panel + AMS open. F4 reverted; panel caps phones at -86 on open and restores last settings per GUID (`DeviceModel.applyStartupSettings`). Stream format: alignment 0 + format extension.
* **Open:** AMS sidebar "N ins / M outs" stays 10/6 after a switch (M-S); panel reverts a rate chosen in the panel (PN.8; official kext behaves the same, its panel keeps the choice in red); ladder (loopback, rate sweep, MIDI, sleep/wake, 5 min); remove diagnostics (O4). **Nothing committed yet** (working tree dirty: kext, panel, tools, docs, workbook).

## 2026-10-16 (late) — first integrated hardware run

**Result:** the full clone (kext + panel) ran on the build PC with the FW1814: CoreAudio lists "FireWire 1814 Multichannel", music plays cleanly on out 1/2, the panel shows the real device (firmware info, meters, faders, front knob all working after fixes). Full log, fixes F1–F10 and open items O1–O4: `docs/system-map/build-pc-runs/2026-10-16-first-integrated-run.md`.
**Open:** O1 switching optical S/PDIF -> ADAT while streaming hangs the stream (`IsochStreamRunning during SetPacketSize`); a later `kextunload` **panicked** the build PC (HSCI async read completing after free, fixed in source as F10, not yet loaded). F4 (phones selector at start-up), F7 (A/B entries 0/1) and F10 are built but unverified on hardware.
**Build PC state:** rebooted after the panic; the old M-FW kext from `/Library/Extensions` is loaded; the current build is staged in `/tmp/OpenAudioFW1814.kext` (`kextutil -t -n` OK). No `boot-args` (kext-dev-mode 0), SIP disabled.
**Next:** see the run log §"Next session"; then the rest of the ladder (`tests/hw-smoke/integrated.sh`), M-K (second unit), M-L, installer IN.5/IN.6 last.

## 2026-10-16 — panel in progress

**Kext side:** M-A…M-J and M-I (AC3) are written, build clean on the 10.13 SDK and pass `kextutil -t -n` on the build PC; none of it has been loaded on hardware (airliner rule). Helper H.3/H.4 written (compiles, not run). MIDI plug-in: reuse the reference one (M-H2).
**Panel (PN.1–PN.7):** PN.1 wireframe **approved** by the user (`A2-panel/PN1-wireframe.html`, `P12-panel-screens-v19.md`); PN.2 map + 54-key catalog (`PN2-control-key-map.md`, `MaKeys.swift`), its open items 1–5 closed by decompiling the reference panel translator (`P13-translator-index-map.md`, R53); PN.3–PN.7 sources written and type-checked on the build PC (Swift 4.1.2): `ChannelLayout`, `DeviceModel`, `IOKitBridge`, `FirmwareInfo`, widgets, strips, mixer/output/hardware/about tabs, sidebar, presets (`P14-clone-panel-design.md`, R54). `make test` (host logic tests) passes. Window controller, mini-meter windows, `main.swift`, Info.plist and `make app` are written: the unsigned app builds on the build PC, opens without kext/device, and every tab/variant renders in demo mode (`A2-panel/clone-snapshots/`). The view layer was then redesigned (dark minimal, Audient-EVO-inspired) with the level knob wired, live A/B text, centred windows, a multi-interface device menu and the about-tab photo; the user approved the look in demo mode (P14). Nothing run against hardware.
**Design decisions this session (user):** buffer size is CoreAudio-only (no panel control); the panel follows the device channel table (88.2/96 kHz greys ADAT 5–8 and S/MUX is automatic; 176.4/192 kHz = 2 in / 4 out analog, all digital off); the manual's "aux not above 48 kHz" is outdated; the lock icon + green/red rate box = sync indicator; the output-tab aux strip is the aux out bus; ctrl = front-panel encoder assignment per strip.
**Corrections made:** P5 (cookie 0x608 = a/b function, not Monitor-Aux; S/MUX rule conflict flagged), P8 (no Global Settings tab / buffer control in v1.9), P9 (pair map owner, peak slots), P11 (cookie 0x609 → 3-entry `MAMomentarySwitchRoutings`, 0x5fd), PN2 map (54 keys, not 46).
**Readiness audit:** `docs/INTEGRATION-READINESS.md`. Closed the same day: M-N was a false positive (the CoreAudio controls exist), M-O rate-change gate implemented, M-P stubs/comments cleaned, IN.7 `scripts/conformance-check.sh` passing (it found `MAMonitorAuxOnOutput` = 3 elements, fixed in the panel catalog), the hardware ladder `tests/hw-smoke/integrated.sh` written (preflight run OK on the build PC). Ready for the first hardware run when the user is present.
**Environment:** the dev Mac's Swift toolchain cannot build for 10.13 (CLT module clash): panel builds and tests run on the build PC (`scripts/sync-to-buildpc.sh`, then `make -C control-panel typecheck|test`).

## 2026-10-15 — build phase started

**Phase:** BUILD (G1 approved 2026-10-14). The code freeze of the headline below is **lifted**; the "Code freeze" and "Build PC idle" bullets under Headline are historical.

**Rules in force (user directives):** full stack (kext, daemon, panel .app, MIDI plug-in, installer — all required); **airliner rule** (no partial hardware tests; verify by conformance, one integrated run); **dev local, sync to the build PC** (`scripts/sync-to-buildpc.sh`, bare remote `buildpc`); **no flash writes** (H.2/H.5 cancelled). Details: `BUILD-PLAN.md` §0, `PROVENANCE-RESOLUTIONS.md` R42.

**Done and verified on hardware (10.13.6 build PC, `build-pc-runs/2026-10-15-M-A-M-FW-install.md`, R40):** M-A (kext skeleton: `FWMetaNub` [superclass `IOService`], two-round `FWAVCAudioDevice::probe`, NULL-safe frees, Info.plist personalities), M-FW (reduced: Info/Start/`Go`/2500 ms re-register), H.1 (LaunchDaemon helper `com.openaudio.fw1814.helper`). Installed on the build PC: kext in `/Library/Extensions`, helper in `/Library/PrivilegedHelperTools` + LaunchDaemon; survives reboot; interface power-on gives `FWMetaNub` -> `FW1814Device` with no manual step. Observed: bootloader device (0x10070) appears ~3 s before the runtime device (0x10071) and boots by itself; no AV/C unit/sub-unit and no `AppleFWAudio` in the registry (go-away personalities untested).

**M-B in progress (static only, never loaded):** Ghidra/otool decode of the TX/RX callbacks (`A1-ghidra/Q-MB-dcl-callbacks.md`, R41); written `FWSharedSync`, `FWCip`, `TPacketPool`(+index), `FWDCLBuilder`, new `FWDCLProgram.h`. **`FWDCLProgram.cpp` is still the part-1 version → the kext tree does not compile until it is ported** (first thing to do on resume). Corrections to older docs: SetTagSyncBits on the first primer of each bank; pool offset by one; takeTimeStamp from the bank-3 TX callback; Q2 address; R1 start-order wording flagged (Q-START).

**Skeleton:** 54 plan-headed stubs created (kext FWIsochChannel, FWCMP, FWAvc, FWStreamControl, FWBusReset, FWCip, FWBaseEngine, FWEngine1/2, FWPairEngine, FWTimeSlaveEngine, FWHsci, FWPropertyDict, FWMixerRegs, FWKIPCUserClient, FW1814UserClient, FWAc3, FWNubRegistry; helper Gating.c/Settings.c; panel Swift sources; MIDI plug-in sources; installer; tests). Tracker: `PROVENANCE-MATRIX.xlsx` → `Build` (38 tasks) and `Painel`.

**Environment:** dev Mac has Ghidra 12.1.4 + JDK 21 (`~/tools`); headless decompile of any function: `scratchpad` script `DecompList.java` (decompile the function containing an address; copy of the project in the session scratchpad; recipe in NEXT_STEPS). Build PC: `~/oa-build` (rsync target), `~/git/openaudio-fw1814.git` (bare), staged kext `/tmp/OpenAudioFW1814.kext` (the M-B part-1 build, validated, NOT loaded), `/tmp/oa-fw/` (copy of fw1814.bcd, no longer needed).

## Headline

- **Project phase:** system-map, per `/NEW_PLAN.md`.
- **Code freeze:** in force. No new source files, no binary patches, no plist edits until Part D lands.
- **Build PC (`192.168.100.20`):** idle — `com.openaudio.driver.fw1814` kextunloaded, kext removed from `/Library/Extensions`, kextcache rebuilt. Nothing of ours is live.
- **Pre-pivot code + docs:** archived to `_legacy_pre_ghidra/` (safety only; not live). See `_legacy_pre_ghidra/README.md`.

## Progress

| Part | Scope | State | Gate for |
|---|---|---|---|
| **A** — static disassembly of `_extracted/` | Ghidra over kext/panel/prefpane/MIDI/helper/firmware | **done 2026-10-09** | Part B synthesis + Part C sync/comm paths |
| **B** — cross-reference | FFADO, snd-bebob, Sierra hack, macfw, Rust bebob, live DTrace | **done 2026-10-13** (B1, B2, B2a, B3, B4, B5 all closed) | Part C |
| **C** — unified system map | lifecycle, sync, comm, transport, UC surface, latency | **done 2026-10-13** (C1–C6 all closed) | Part D |
| **D** — build decision | one page: modified reference / resumed clone / hybrid | **D2 (full resumed clone) CHOSEN by the user 2026-10-14**; D1/D3 withdrawn. Plan: `docs/BUILD-PLAN.md` (DRAFT, gate G1). Code still frozen. | code work resume (M-A through M-M) |

### Part A — status (all done 2026-10-09)

| Component | File | Highlights |
|---|---|---|
| A1 kext | `system-map/A1-ghidra/` | Q1–Q17 + Q15e/Q15f + Q3a + Q-INFO all closed. 2862 fns / 222 classes / 80 decompiled. Engine family (`FWEngine1`/`FWEngine2`/Pair/TimeSlave), polymorphic `FWMetaNub`, dual UserClient surface (`FWUserClient` + `FWKIPCUserClient`), typed-property + ChannelGroup framework, DCL 64-seg ring, `AddTimestampedTransferPackets`, `TPacketPool` AM824 silence pre-fill, HSCI envelope, 7-rate FDF table, `WorkaroundFor1814InputDelay`, per-rate round-trip latency table. Four mixer write paths (AVC `0xBA9`/`0xBE9`, direct FW vtable[0xba8], HSCI). AV/C wire bytes (0x18/0x2F/CMP). SYT-interval/CIP-increment table. Info.plist personalities. |
| A2 panel | `system-map/A2-panel/` | P1–P10. 46-key `MA*` property surface, Carbon mixer/hardware/settings views, peak-meter path, rate+buffer UI, AC3 passthrough toggle, notification receive wiring (3 body types). |
| A3 MIDI | `system-map/A3-midi/` | M1–M10. CoreMIDI CFPlugIn entry points, wire packet layout, TX/RX paths, endpoint model, SysEx, sleep handling. |
| A4 helper | `system-map/A4-helper/` | H1–H10. Entry dispatch, bootloader detection, firmware selection, status UI IPC (`sudo -u` + Distributed Objects), error-retry, launchd lifecycle, runtime handoff, BCD reader. Helper opens vendor UC purely for `IOConnectSetNotificationPort` — one-way notification sink; no `IOConnectCallMethod` imports. |
| A5 firmware | `system-map/A5-firmware.md` | Full BCD header + both CRCs verified (payload `h[0x3c]`, appendix `h[0x5c]`). BridgeCo attribute namespace enumerated. |
| A6 prefpane | `system-map/A6-prefpane.md` | Trampoline-only. Launches panel via `[NSWorkspace launchAppWithBundleIdentifier:]`. No IPC, no IOKit, no FireWire. |

### Part B — status

| File | Status | Note |
|---|---|---|
| `B1-ffado-correlate.md` | **done 2026-10-13** | Q2/Q5/Q6/Q15/Q15c cross-referenced. Candidate **A1-Q15f** gap discovered: `CustomMixerWriteQuads @ 0x22c7a` writes directly to `0xffc7:0x00700000+` — same window FFADO/Rust/macfw use. A1-Q15 only decoded the AV/C `0xBA9` path. 40-quadlet register-offset table from `special_avdevice.h:50..134` is independent ground truth for the mixer semantic layout. FFADO stubs clock/HSCI/rate control — A1 is sole truth there. |
| `B2-snd-bebob-correlate.md` | **done 2026-10-13** | Linux uses NuDCL so A1-Q1 stays sole-source on DCL ring shape. AM824 silence disagreement: `_extracted/` emits label `0xCF` (vendor idle), snd-bebob emits standard `0x40` MBLA — clone must mirror `0xCF` for PLL lock. Three A1 gaps surfaced: `CIP_EMPTY_HAS_WRONG_DBC` at ≥88 kHz (bebob.c:174), post-registration bus-reset gap-count fix (bebob.c:289), HSCI byte 9 = `clk_lock` latch. Stream-kick 100 ms interval matches `special_set_rate` byte-for-byte. |
| `B2a-rust-bebob-correlate.md` | **done 2026-10-13** | 46 MA* keys mapped onto 4 Rust protocol structs over a 160-byte `MaudioSpecialStateCache` with 18 contiguous offset-ranges (special.rs:308-399). A1-Q15b single-dispatcher validated — Rust's `partial_update` (special.rs:1053-1093) is the same pattern. `Fw1814ClkProtocol` deliberately omits clock-source selection → confirms `MAClockSource` is HSCI-only on macOS. Stream pairs 2/3+ bypass the mixer → likely explanation for A1-Q13's 4-engine family split. `HeadphoneSource` resolved to 3-valued enum (MixerOutputPair0/1 + AuxOutputPair0). |
| `B3-sierra-hack-delta.md` | **done 2026-10-13** | Exact 10.11→10.12 deltas. Three cumulative 10.13 breakers = UAKEL + SHA-256 CMS + reserved-slot drift. Recommends clone-rebuild for Part D. |
| `B4-macfw-correlate.md` | **done 2026-10-13** | **Independent corroboration of A1-Q15f gap** — macfw uses the same `0xffc7:0x00700000+` register window as FFADO + Rust. macfw uses NuDCL from userspace (non-starter for our kernel on 10.13.6 but validates per-rate packet geometry). macfw does not upload firmware, only sends the boot-from-flash cue (`0xffff:0xc8021000`). macfw has no clock-source dropdown and no ADAT — Part C digital surface relies on A1 + FFADO + snd-bebob. |
| `B5-sl-dtrace-truth/` | **done 2026-10-13** | 10 headline findings + §11 meter-path (`clientMemoryForType(1)` shared ring, cap 70) + §12 fbt-on-SL limit documented + §13 rate-change / rate-locked questions reclassified as A1-Q15c + Q15d static follow-ups. |

### Part B5 headline findings (bear on Part C synthesis)

1. **Panel + helper each open their own `com_m_audio_FWUserClient`** — not a middleman architecture. Validates `Proposal.md`'s "panel talks to kext directly" simplification. (B5 §3)
2. **Clock source is NOT an `IOAudioSelectorControl`** — it only exists as the `MAClockSource` property, crossed over `IORegistryEntrySetCFProperties` on the device registry entry. `MAClockSource` is a composite 32-bit bitfield (sync-source bits[1:0], S/PDIF-port bit 2, opt-IN bit 3, opt-OUT bit 4, S/MUX bit 5, gate bit 30); the sync-source wire-byte mapping is `{00:int-unmuted→wire 3, 01:ext-dig→1, 10:wc→2, 11:int-muted→0}` per A1-Q5. (B5 §4, §10)
2a. **Meter path = pre-mapped `clientMemoryForType(1)` shared ring on the vendor FWUserClient** (B5 §11, cap 70). Zero IOKit MIG traffic from Panel or helper during 30s of animating meters. Not `IOAudioLevelControl`, not Mach-port notifications. Clone's vendor UC must implement `clientMemoryForType(1)` with a kernel-updated meter ring.
3. **`FWEngine1` + `FWEngine2` are both class names** (not instance counters) — multichannel (`FWEngine2`, flavor=1) and stereo-pair (`FWEngine1`, gated by `MAEnableStereoPairEngines`). Confirmed in v1.9 and v1.11.2 kext binaries.
4. **Helper is a StartupItem** on v1.9 (SL), a LaunchDaemon on v1.11.2. Shipping target migrates to LaunchDaemon.
5. **Playback and control plane are independent at the IOKit level** — `FWEngine2` state 0→1 goes through a dedicated `IOAudioEngineUserClient` owned by the user process. Vendor UC retain counts don't change. Vendor channel is config-only.
6. **46 `MA*` property keys are identical v1.9 ↔ v1.11.2** — control-plane protocol was stable 2009→2014.
7. **Runtime rate change (44.1k → 48k) in Studio One 2** — `IOAudioEngineSampleOffset` moves 40→48; `IOAudioEngineOutputSampleLatency` moves 216→231; input latency rate-independent at 32. Latency must be recomputed on `performFormatChange`.
8. **Vendor UC class serves Panel + helper + 2× MIDIServer concurrently** — must multiplex connection types in the clone.
9. **The ADC pre-mix path (A1-Q9 `WorkaroundFor1814InputDelay`) uses a 16-entry per-channel delay ring** replacing the convert proc when `fInStreamChannels != 2`. Only ch0..6 delayed. FW1814-specific.

## 2026-10-14 provenance sweep — status

Audit flags (`PROVENANCE-AUDIT.md`, 126 rows in `PROVENANCE-MATRIX.xlsx` → `Flags_auditoria`) were worked through: **82 resolved** (asm-verified or audit-verified, dated `Correction` blocks inserted in C1–C6, D, Q1/1a/1b, Q4, Q5, Q6, Q7, Q15c, Q16, Q17, B1/B2/B4/B5, A1 README, A2–A6, system-map README), **31 labelled** unverified with a verification path, **13 still open** (OHCI interrupt mechanism, PLL/plug settle times, AV/C timing, 0xCF hypothesis, firmware read-shy). Evidence: `docs/system-map/PROVENANCE-RESOLUTIONS.md` (R1–R6). New facts found while re-verifying: `[+0x2d0]` is the **external-sync mode flag** (not `fRunning`), so the ReconnectP2P tail resends the clock source only when external sync is off; `FDFValue` is the enum 0..6; BusReset == ReconnectP2P on a new thread. Dashboard = `Painel` tab. Rule: run the `close-item` skill after every item.

## 2026-10-14 — Part A focus (user directive: get A to ~100%)

Closed so far: OHCI interrupt mechanism (`A1-ghidra/Q-OHCI-interrupt.md`), `[+0xa5]`, `vt[0x850]`, pool-page formula, SwitchDigitalSignals semantics (R7/R8). New SL fact: the helper reapplies saved prefs (capture 71). Remaining A items are listed in `PROVENANCE-MATRIX.xlsx` → `Painel` (sub-branches with 'Com pendência'/'Parcial'); the "Não auditado" A2/A3/A4/A6 docs need a static audit against their binaries, the rest need hardware captures or user authorization.

## 2026-10-14 (later) - Part A at ~95%

Part A: 100 of 105 matrix items closed (see `PROVENANCE-MATRIX.xlsx` -> `Painel`). Static audits of A2 (P1, P7, P8, P10, P11), A3 (M1, M2, M8, M9, M4), A4 (H1-H8 + new `H11-settings-persistence.md`) and A6 are in `*/AUDIT-2026-10-14.md`; evidence R7-R19 in `PROVENANCE-RESOLUTIONS.md`; new OHCI note `A1-ghidra/Q-OHCI-interrupt.md`. Key corrections: OHCI interrupt = CallProc callback (not timestamp); `SupportsAC3` = 1 on the FW1814; panel cookie 0x615 = 192000 on the FW1814 (no aux gating by rate); outputs 6/12 (bit 4) vs inputs 10/16 (bit 3); FW1814 has 1 MIDI in + 1 out; helper persists/replays settings; reference kext never terminates user clients. Open in A: Q-MAMAP (Q3b.2), Q5.5 (sleep purpose), Q17.2 (firmware reaction to 0xCF), Q-SDK.2 (needs your OK for kextutil -n), Q-SDK.4 (failure before panic).

## 2026-10-14 (end of day) - Part A at 97%

103 of 106 Part A items closed (`PROVENANCE-MATRIX.xlsx` -> `Painel`). New since the previous entry: Q-MAMAP crosswalk (`A1-ghidra/Q-MAMAP-key-crosswalk.md`: vendor register window vs AV/C per key), settings persistence (`A4-helper/H11`), clock-change sleeps measured on SL (R21), panic symbolication (R22: four of six build-PC panics are `probeCandidates -> free() -> operator delete(NULL)`), firmware lock-detection strings (R23). Bundle note: the kext binary is **bundle 1.11.3** (same sha256 as `legacy-mod`); panel 1.5.2 / helper UI 1.2.2 / MIDI plug-in 1.11.2 come from the 1.11.2 installer. The FW1814 is attached only to the SL rig, not to the High Sierra machine. Open in A: `Q17.2` (label 0xCF effect: wire capture), `Q-SDK.2` (needs your OK for `kextutil -n`), `Q-SDK.4` (why the candidate instance was released).

## 2026-10-14 (night) - Part B SL captures

Captures 72 (rate sweep, latency at six rates, stream-start sleep chain) and 73 (interface power cycle, cold-start timeline) are in `docs/system-map/B5-sl-dtrace-truth/captures/`; evidence R25. The user power-cycled the interface (no cable hot-plug — see memory `feedback_no_hot_plug`). Not observable on this rig: FCP/HSCI frame contents and the live-device `ReconnectP2P` (1100 ms) path. Part B now 16/19 items closed; open: B2.2, B3.2 (static, unaudited), B5.6 (v1.9 rate-change-while-streaming panic: deliberately not repeated).

## Decisions pending (both reserved for Part D — do not pre-commit)

### D.A — Kext scaffold origin (anticipates D1/D2/D3)

- **D1** ship modified reference bundle — now feasible because A1 pinpointed the KPI drift surface.
- **D2** resume a from-scratch clone — only if C exposes a concrete mechanical blocker in the reference on 10.13.6.
- **D3** hybrid — reference binary as the IOKit shell, our code only where 10.13.6 kernel ABI genuinely diverges.

Pre-Ghidra clone kext is archived under `_legacy_pre_ghidra/kext/`; nothing is deleted. Transferable bits enumerated in `_legacy_pre_ghidra/README.md`.

### D.B — Control-panel topology (confirmed by B5 §10 ahead of C5)

- Panel talks to the kext directly via `IORegistryEntrySetCFProperties` for the control plane (`MAClockSource`, …) — **not** through the helper.
- Panel talks to the vendor `com_m_audio_FWUserClient` only for notification subscription (`IOConnectSetNotificationPort`), peak-meter shared ring (`clientMemoryForType(1)` for MIDI; separate type for meters per P9), and MIDI RPC selectors (500–507 per Q11).
- Helper is firmware-only — opens the vendor UC purely to receive bootloader/firmware notifications; no `IOConnectCallMethod` imports (A4-H1).
- Pre-Ghidra control-panel scaffold (`_legacy_pre_ghidra/control-panel/Sources/`) assumed direct `IOConnectCallMethod` for everything control-plane — **wrong**. Rebuild from C5 after Part D.

### D.C — User-client surface (feeds C5)

- Reference `FWUserClient::externalMethod` exposes 8 selectors (500–507): MIDI Start / Stop / Stop-alias / Send, DiceSet, DiceGet, 506 = `WaitForTermination` (blocking wait), 507 = `SetInitializedProperty` (corrected 2026-10-14, Q-UC-surface/R35) — Q11.
- Second class `FWKIPCUserClient` is a Mach-port notification channel only — 44-byte message; type-0 port carries props (body 0) + IOKit message codes, type-1 port carries MIDI-in (≤320 B); no peak/rate-lock types (corrected 2026-10-14, A1 Q-UC-surface §4); dirty-property OSDictionary serialized to IOBMD; broadcast via static `sUserClients` OSSet. Not RPC — Q14 + Q14a.
- Clone's pre-Ghidra 15-selector surface (`_legacy_pre_ghidra/kext/src/FW1814UserClient.cpp`) is a bespoke invention. Scrap post-D.

### D.D — Mixer model (feeds C4)

- Reference uses a typed-property dict (`FWPropertyDict`) with `ChannelGroupProperty` entries, delta-apply via `TFunctor` on `FW1814Device +0x38` with vendor opcode `0xBA9`. Single generic dispatcher at `FWPropertyDict::SetProperties @ 0xa0e0` absorbs every non-`MASampleRate` write. Panel writes via `setProperties` on the device registry entry — Q15 + Q15b.
- Clone's `_legacy_pre_ghidra/kext/src/FW1814Mixer.cpp` is a thin register-poking sketch — not compatible with the typed-property model. Rebuild post-D.

## Known-good primitives (survive Part D regardless)

These are hardware truths captured in auto-memory + Part A. They stand independent of which D option is chosen.

- **Vendor OUI** `0x000D6C` (M-Audio); **Model_ID** runtime `0x10071` / bootloader `0x10070`. The older `0x10051` value in FFADO tables is a different revision — do not use.
- **Plug map:** 2 oPCRs + 3 iPCRs; plug 0 = PCM. **Correction (2026-10-14, STATE-KG.2 audit):** the counts are supported by one third-party hardware read (macfw `devices/fw1814/analysis/special-firmware-notes.md:16-20`, macOS, 2026-09-06: `oMPR 0xbfff0002`, `oPCR[0] 0x80000080`, `iMPR 0x80ff0003`, `iPCR[0] 0x80000000`) and by the project's own pre-pivot probe (memory `project_fw1814_plugs.md`; the probe log is **not preserved in the repo**, the code is archived). They were **not read from the reference binary**: the kext never reads the MPRs (it reads/locks PCR quadlets at `0xF0000904+4n` / `0xF0000984+4n` in `updateP2PCount @0x244f0`, A1 Q-CMP-connect); it calls `FWDCLOutputProgram::Connect(0)` and `FWDCLInputProgram::Connect(0)` with a literal 0 in `ConnectAudioMIDIChannels` (`xorl %esi,%esi` @0x121c5 / 0x1225f, then `vt[0x120]`), which is most plausibly plug 0 in both directions (**the uint could also be read as a channel hint; IRM allocates the channel, so plug is the likelier reading - not proven**). Not supported: the memory's "plug 1 = MIDI, plug 2 = AC3" (FFADO/macfw assignment is not in the repo evidence). MIDI is a slot inside the plug-0 stream, not a separate plug: macfw formations are `10 PCM + 1 MIDI` (DBS 11) capture and `6 PCM + 1 MIDI` (DBS 7) playback (`first-duplex-capture-success.md`), and the kext's CIP DBS = audio + MIDI quadlets (audit item 8). Status: counts = third-party-verified, plug-0-in-kext = reasoned; to close, read the MPRs on the SL rig (async quad read at `0xFFFFF0000900/0980`, needs a tool on the SL - not done).
- **Stream-kick after CMP connect:** CMP + start alone leaves the device silent (macfw, third-party hardware); the **reference** order is TX start -> 500 ms -> INPUT 0x19 -> 500 ms -> RX start -> OUTPUT 0x18 (asm 0x121c9..0x122bb, Q3a Correction), not OUTPUT -> 100 ms -> INPUT (that is snd-bebob/macfw). Memory: `project_fw1814_stream_kick.md` (out of date on the order).
- **Buffer-size ladder:** 64 / 128 / 256 / 512 / 1024 / 2048 / 4096 frames, default 512. Memory: `project_fw1814_buffer_sizes.md`.
- **Rates:** 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz, always 24-bit. ch1/ch2 gain is hardware; everything else is software. Memory: `project_fw1814_hardware_capabilities.md`.
- **Clock sources:** Internal / S/PDIF / ADAT / Word Clock. Four selector entries required for panel parity. Memory: `project_fw1814_clock_sources.md`.
- **HSCI envelope:** vendor-dependent CONTROL to the FCP command register, 16-byte buffer in the kext (length `0x10` @0xe441; the 12-byte layout is snd-bebob's); company_id `0x040004`; bytes 7/8/9 = MAClockSource bits 3/4/5. Memory: `project_fw1814_hsci_envelope.md`.
- **DCL structure:** TX = 4 banks × (4 primers + 1 big-cb + jump) = 20 segs + 4 jumps; RX = 64 segs × 4 pkts + 64 jumps (Q-DCL; "single-pkt" was wrong); `talking` polarity inverted vs the pre-Ghidra clone. Memory: `reference_dcl_structure_authoritative.md`. Full decode in A1-Q1.
- **Idle lock mechanism:** **Correction (2026-10-14, STATE-KG.5 audit): split into what is verified and what is not.** Verified: (1) the reference TX ring emits data-bearing packets at idle - 424-byte packets, DBS 13, on the SL (capture 74); (2) macfw (third-party, macOS hardware) found that the INPUT rate command fails while the host->device stream carries only NODATA packets and succeeds once timed AM824 silence is sent (`special-firmware-notes.md`, `first-duplex-capture-success.md`); (3) `ZeroElements` pre-fills `0xCF`/`0x80` and is re-run every TX callback, after which converters write `0x40` (audio) / `0x80` (MIDI) (A1 Q17 / R10, asm 0xa6da, 0xa701, 0x1195d, 0x119bd). **Not verified:** that the PLL "locks against" host silence or drifts when TX stops (macfw's DBC quote at `first-duplex-capture-success.md:77` is about the device->host capture stream, not host TX); that idle clicks come from DCL callbacks not firing or from the label fill (no capture; the callback cause is a hypothesis after Q-OHCI-interrupt); that `0xCF` matters. So: "continuous data-bearing TX is needed for the rate kick to work" = third-party-verified; "PLL lock / click cause" = **hypothesis**. Check: SL idle capture with TX stopped vs running (e.g. dtrace `IOSleep`-free: stop the engine, listen for lock via the front-panel lock LED or the HSCI sync bits), or a bus-analyser capture. Memory `project_fw1814_idle_lock_mechanism.md` over-states the PLL/DBC claims and the cited quote.

## Next steps — in order (rewritten 2026-10-14)

Pre-build research per `docs/BUILD-MANUAL.md` §0 (status table) / §5. Code stays frozen until the user approves a D option.

0. **[DONE 2026-10-14, R30] D1/D2/D3 comparison re-run** (`D-decision.md` top block). Next: Q-SDK.5 discriminator, then consolidate the C-doc bodies (item 2 of the user directive).
1. **D re-evaluation input** (user decision pending): `A1-ghidra/Q-SDK-reserved-slots.md` refutes B3 breaker #3. The real 10.13 reference panic is `operator delete(NULL)` in teardown (`FWAudioDevice::free() @ 0x23b7`), and it follows an earlier, still-unknown init/probe/start failure. Optional confirming step, needs user go-ahead: `kextutil -n` link-only dry run of the reference on the build PC, staged in `/tmp` only.
2. **Ghidra-only research** (dev Mac, Ghidra 12.1.4 in `~/tools`): SetupCIP cases 3–6, INPUT `0x19` path, Q-CMP-CONNECT per-rate params, Q15f open items (Path B formula, Path C keys, rotated aux), Q-OPCODE, Q3b, Q-UC-SELECTORS / Q-UC-RING / Q-KIPC-LIVE / Q-NOTIFY, Q-CLOCK-UI, Q-MIDI-WIRE check.
3. **SL rig session** (`ssh fw1814-sl`): Q-RATE-LATENCY at 88.2/96/176.4/192 kHz, Q-LOOPBACK reference, and an FCP capture of a clock-source change to confirm HSCI bytes 7/8/9 live.
4. **Q-MAMAP** synthesis (46 rows) once Q-OPCODE + Q3b + Q15f land.
5. **User items:** D approval, panel wireframe sign-off, Developer ID.
6. Only after D approval: scaffold `openaudio-fw1814/kext/` etc. per BUILD-MANUAL.

### 2026-10-14 findings (summary)

- **HSCI bytes 7/8/9 = `MAClockSource` bits 3/4/5** (optical-IN ADAT, optical-OUT ADAT, S/MUX). Raw `SetClockSourceInternal` `0xe3e2..0xe425`. The Ghidra decompile showing zeros was a decompiler artifact. Corrected in Q5, C2, C3, B2, Q15f, D.
- **DCL ring shape:** TX = 4 banks × (4 primers + 1 big) + 4 jumps = 20 segments; RX = 64 segments × 4 packets. The "64-seg TX" claim was propagated in error. Corrected in C2, C4, C6, B2, B4, D, Q1, CLAUDE.md.
- **Q2 `+0x4a0` = sample rate in Hz**, not channel count.
- **Reserved-slot drift refuted** (Q-SDK); build-PC panic logs archived to `system-map/build-pc-panics/`.
- **Evidence relocated from `/tmp`** into the repo or `_refs/` (see `_refs/README.md`); ~130 citations rewritten.

### 2026-10-14 findings — batch 2 (Ghidra-only leftovers)

- **FW1814Device derives from `FWRev1AudioDevice`**, not Rev2 (OSMetaClass superclass `0xf473`). All other products except ProjectMix are Rev2. Q16, CLAUDE.md, C1, C5, D and BUILD-MANUAL corrected.
- **Stream format = plain 8-byte AV/C CONTROL.** `00 FF 19 00 90 <SFC> FF FF` (INPUT), then `00 FF 18 00 90 <SFC> FF FF` (OUTPUT), interleaved with TX/RX DCL program start, 500 ms apart (`ConnectAudioMIDIChannels`, raw `0x121f7..0x122bb`). The `0x2F` extended-format command is Rev2-only. Q3a corrected.
- **SetupCIP cases 3–6 resolved** (jump table `0x272d8`); the divisor uses SYTInterval. Q15e and Q2 corrected.
- **`0xBA9`/`0xBE9` are C++ virtual member pointers.** There is one mixer path: group batch → `CustomMixerWriteQuads` → async block write to `0xFFC7:0x00700000 + {0x00, 0x08, 0x10, 0x34, 0x38}`, matching the FFADO register map. Q-OPCODE is moot.
- **CMP:** IRM-allocated channel; read-modify-lock of device oPCR `0xF0000904+4n` / iPCR `0xF0000984+4n` (≤4 tries, 100 ms back-off); packetSize `4·DBS·SYT + 8` (TX) / `+ 12` (RX). New `A1-ghidra/Q-CMP-connect.md`.
- **User client:**
  - `FWUserClient` derives from `FWKIPCUserClient`.
  - The method table is legacy `IOExternalMethod`; selectors 506/507 dispatch to `WaitForTermination`/`SetInitializedProperty`.
  - The meter page is `uint32 peak[52]` in a 4 KiB shared page.
  - `0x400|n` / `0x800|n` memory types are the MIDI rings.
  - Notification type 0 = property changes, type 1 = MIDI-in; there are no peak/rate-lock types.

  New `A1-ghidra/Q-UC-surface.md`.
- **Factory defaults:** `ResetToFactorySettings @ 0xd190`, `MAClockSource = 0x40000003`. New `A1-ghidra/Q3b-mixer-power-on-defaults.md` (partial).
- **[Superseded by Q-SDK.5 / R31 below]** **10.13 panic narrowed:** the NULL `[+0x110]` deleted in `free()` is only allocated at the end of `initHardware`. The earlier failure is one of AllocateConnectionManager / createAudioEngine / RequestFormatUpdate / CompletePendingStateChanges / InitializeUserSettableProperties, or a start failure before that.
- **Q-CLOCK-UI:** every digital-I/O widget is a read-modify-write bit of `MAClockSource` (`ChangeDigitalSourceAttribute @ 0x71fea`). New `A2-panel/P-ClockUI-encoding.md`.
- **macfw clone moved to `_refs/macfw/`.**

### 2026-10-14 — Q-SDK.4 closed (kextload with device) + capture 74

- Real `kextload` with the FW1814 powered on in the iMac reproduces the panic (capture 75, R29): `probeCandidates+0x770` -> `m_audio_b_FWAudioDevice::free` -> `delete(NULL [+0x110])`. Which init/probe step fails first remains unverified.
- Capture 74 / R28 (SL): TX packet geometry and interrupt-bit pattern confirmed live; label bytes unreadable. Q17.2 stays open (hypothesis; does not block the clone).
- Part A: 105/106 items closed; only `A1-Q17.2` left.

### 2026-10-14 - D re-run (R30)

**User directive (same day):** closest to the official driver with only the necessary modifications and a better UI -> D1 (modified reference) preferred, D2 (clone) fallback; decided by Q-SDK.5. Memory `feedback_patch_original_kext` rewritten to match.

D1/D2/D3 compared with the corrected facts (block at the top of `system-map/D-decision.md`; stale sections carry Correction blocks). Findings: breaker #3 refuted; #1/#2 apply to every option and are unproven (SIP-on never tested); D3 has no identified insertion point and collapses into D1; the deciding unknown is which init step fails first on 10.13.6 (new item **Q-SDK.5**; capture 75 did not record the device Model_ID/state). D2 stays the proposed default on new grounds (source-level tracing, NULL-safe teardown). Not approved. Process conflict flagged: memory `feedback_patch_original_kext.md` vs the pivot.

### 2026-10-14 - Q-SDK.5 closed (R31): the panic is the reference's own first-round probe reject

`A1-ghidra/Q-SDK5-first-round-probe-reject.md`. `FWMetaNub::start` registers the nub; the first matching round always fails `FWAVCAudioDevice::probe` (`[+0xf3] == 0`, set only by `MAFirmwareStart` from the helper); the rejected, never-started `FW1814Device` is freed; `free()` and the destructors `delete(NULL)` (218 `__ZdlPv` sites) and 10.13.6's `operator delete` has no NULL test. Panics on every attach. Steps 2-5 of `initHardware` are untested on 10.13.6, not failing. **D1 becomes the recommended path** (NULL-safe delete wrapper + re-sign + helper must send `MAFirmwareStart`); D2 is the fallback if round two fails. Round two never exercised; needs a patched binary (frozen until D approval).

### 2026-10-14 - D1 approved

User: "i approve D1". Build path = official kext + minimal 10.13-forced patch (NULL-safe delete) + re-sign + helper sending `MAFirmwareStart` + new AppKit panel (+ MIDI plug-in check, installer). Freeze stays until the build plan is approved. Next: static round-two review (initHardware steps 2-7, DCL build), helper upload-path check, consolidate C docs, then a work breakdown with acceptance tests in the workbook.

### 2026-10-14 - Helper check (R32) + round-two review (R33) for D1

`A4-helper/H12-project-helper-vs-d1.md`: the project's `firmware-helper` is a pre-pivot, bootloader-only, direct-IOFireWireLib tool that never writes `MAFirmwareStart`; D1 needs a `MAFireWireLoader`-style helper (watch `com_m_audio_FWMetaNub`, read `MAFirmwareInfo`, write `MAFirmwareStart/Halt/UpgradeImage`). `A1-ghidra/Q-D1-round-two-static-review.md`: stage table, risk register and staged test ladder for round two; Q1b's "the 2014 kext proves classic DCL works on 10.13.6" is unsupported (corrected). Still unverified: DCL acceptance by `AppleFWOHCI_DCLProgram::init`, Build*DCLProgram/IOBMD, `AppleFWAudio` competition, signing with SIP on.

### 2026-10-14 - Pre-pivot code removed; from-scratch build plan drafted

At the user's request all pre-pivot code was deleted from the working tree (64 tracked files: `_legacy_pre_ghidra/` and `openaudio-fw1814/{firmware-helper,legacy-mod}`; `legacy-mod` was byte-identical to `_extracted/kext`, sha256 `5532c6e8…ceb17`). `openaudio-fw1814/` is intentionally empty. Recoverable via `git show d1-approved-2026-10-14:<path>` (commit `a9cb63c`). `docs/BUILD-PLAN.md` (DRAFT) is the new working plan: D1 = official kext + minimal patch; helper, panel, patch tooling, installer, tests written from scratch; E0 static pre-work authorised, everything else waits for the user's approval of the plan (gate G1).

### 2026-10-14 - D2 chosen (D1 plan wiped)

User: "wipe D1 plan. go full D2." Supersedes the "D1 approved" entry above (D1 was approved and reversed the same day). Plan = `docs/BUILD-PLAN.md` (DRAFT): clone mirrors the official v1.11.x design, NULL-safe teardown + two-round lifecycle (R31), kext-side firmware path, SL rig as behavioural oracle, milestones M-A..M-M, authorised now = static pre-work P1-P6.

### 2026-10-14 - Workbook restructured

`PROVENANCE-MATRIX.xlsx`: new `Build` tab (38 tasks from `docs/BUILD-PLAN.md` with acceptance tests); Painel roadmap split into research (0-D) and build (P, M, H, PN, IN) blocks; KPI block shows research vs build progress separately (research ~87%, build 0% because code is frozen until G1). The old E1-E4/V/R rows were replaced. `close-item` skill updated (step 3b).

### 2026-10-14 - P5 done: C1-C6 consolidated (R35)

Each C doc now reads as one consistent version with a change log at the end (old versions in git at `a9cb63c`). New facts: 506 = blocking `WaitForTermination`, 507 = `SetInitializedProperty`; `0x620`/`0x518` are class sizes (Q-DCL table withdrawn); measured latencies at all six rates are the primary C6 data (InputSampleLatency 32/32/64/64/128/128). Remaining unverified items are labelled in-text (R35 lists them). Next: P1, P2 (static), then P3, P4, P6; then the user approves the plan.

### 2026-10-14 - P1 + P2 done (R36)

`A1-ghidra/Q-P1P2-dcl-acceptance-and-pool-memory.md`: classic-DCL acceptance hinges on `generateBufferMap` (one fully mapped, 32-bit-DMA span over every transfer buffer and timestamp slot) which the reference satisfies by keeping both in one `TPacketPool` allocation (timestamp slots in its last page); the pool is a `KMA_LOMEM` allocation (virtually contiguous, below 4 GiB, NOT physically contiguous), 0.1-0.7 MB. Corrects Q-DCL (packet sizes/pages) and C4 §4. Run-time acceptance is still unverified (M-B). Next: P3, P4, P6.

### 2026-10-14 - P3 done (R37): the kext firmware path

`A1-ghidra/Q-P3-kext-firmware-path.md`: BeBoB bootloader protocol inside `FWMetaNub` (registers `C8020000/C8021000/C8021040/C8029000`, commands Reset/DownloadStart/DownloadBlock/DownloadEnd/Go, 512-byte blocks, dispatch on `bcd[0x34]`); `MAFirmwareInfo` = FFADO `info_register_t` (live value decoded); `[+0x98]` = protocol version (the Go cue equals macfw's bytes); `resetBus`+3.5 s only on the update error path. Corrections applied to C1, C3, B2, Q7, H2. Next: P4, P6.

### 2026-10-14 - P4 done (R38): per-rate CMP/packet/bandwidth table and the mixer power-on state

`A1-ghidra/Q-P4-cmp-per-rate-and-mixer-power-on.md`: channel counts per mode and rate (four `DeviceChannels` tables), `m = 1`, packet bytes, pool pages, bandwidth bytes/cycle (verified against captures 72, 74 and `MABytesPerCycle = 976`); speed S400; the factory mixer state IS sent at init (blank-slate clock -> resend -> start HSCI poll), also on wake/reset/clock change; HSCI listener `ReadHSCIData` decodes sync from packet word 0x50. Four wire items stay open (input-pan half-word order, headphone-selector initial image, aux-pan and SW-return-routing AV/C frames). Next: P6.

`P6-verification-oracle.md` (R39): oracle classes O1-O6, `tests/` layout and milestone matrix; SL cannot observe FCP/HSCI/isoch bytes, so those are derived statically + third-party logs; new captures (second FireWire node, meter-page copyin) proposed, not run. P1-P6 all done; the plan awaits your G1 review.

## Housekeeping notes

- Panel + helper on the SL reference use `com_m_audio_FWUserClient` as the vendor UC class name. Clone class names (`com_openaudio_*`) in the Info.plist under `_legacy_pre_ghidra/kext/` are correct for ours, but the engine-split class naming (`FW1814Device` / `FW1814Engine`) will need rework after Part D to match the 4-way engine family (Q13).
- Memory under `~/.claude/projects/-Users-bernardosardinha-Documents-openaudio-fw1814/memory/` (migrated from the old Mac's `…-firewireDriver/memory/` on 2026-10-14) is the second source of truth. Keep it in sync when new Part B / Part C findings land.

### 2026-10-15 - Q-START closed (R43)

`A1-ghidra/Q-START-channel-start-order.md`: program `vt[0x120]` = Connect only; `vt[0x138]` = Start (-> `FWIsochChannel::start`); `FWEngine2::StartAudio` -> `StartPCMAudio` starts RX then TX (TX primes 4 banks first). All decompiles saved under `ghidra-exports/decomp/`. Next: M-B (`FWDCLProgram.cpp` port) with Connect/Start split, then M-C.

### 2026-10-15 - M-B code complete (FWDCLProgram.cpp ported)

`kext/src/FWDCLProgram.cpp` rewritten against the new header: `OutputDCLCallback` (cadence/SYT/DBC, external-sync queue, bank-3 `takeTimeStamp`), `AdjustOutputDCL`, TX/RX chain builders, `SetPacketSize` with the pool offset-by-one assignment, RX `InputDCLCallbackCopyPacketGroupToSamples`, `ResyncInputBuffer`, `CycleTimeToHostNanos`, `Start`/`Stop` (Q-START split). The part-1 bring-up hook was removed from `FWAudioDeviceChain.*`. Build PC (10.13 SDK) builds clean; `scripts/stage-kext.sh` -> `kextutil -t -n` OK; nothing loaded. Acceptance (port non-NULL, packet geometry) stays for the integrated run (airliner rule). Next: M-C (CMP, FWIsochChannel/Connect/ResetPort, stream-kick order from the C2/C4 audit).

### 2026-10-15 - M-C code written (R44)

`A1-ghidra/Q-MC-cmp-isoch-avc-start.md`: AV/C unit (`[+0x268]` = FWAVCUnit, `vt[0x8b8]` retrying command), CMP connection manager, isoch channel wiring (local port = talker, remote = listener), `ConnectAudioMIDIChannels` order confirmed, `ResetPort` args. Clone: `FWAvc`, `FWCMP`, `FWIsochChannel`, `FWStreamControl`, `FWDCLProgram::{Connect,Disconnect,ResetPort}`, `AllocateConnectionManager` in the device chain. Build PC (10.13 SDK) builds clean, `kextutil -t -n` OK, nothing loaded. Not yet written for M-C: `FWBusReset` wiring to the bus-reset notification (manager `BusReset()` exists), engine instantiation (M-D/M-J). Next: M-D (all rates, FDF/SYT tables, rate-change chain) per the flowchart.

### 2026-10-15 - M-D engine core written (R45)

`A1-ghidra/Q-MD-engine-rate-chain.md`: class chain IOAudioEngine -> FWDiscreteEngine -> FWBaseEngine -> FWEngine2; DeviceChannels tables and the six-rate list from the binary; rate/format change chain, pending-state machine, latency (`ResetLatency` + safety offsets), start/stop refcounting. Clone: `FWRates.h`, `FWDiscreteEngine`, `FWBaseEngine`, `FWEngine2`, `FWStream`, device `createAudioEngine`/`SetupAudioEngine`. Build PC builds clean, `kextutil -t -n` OK, nothing loaded. NOT ported: the sample path (clip/convert/rearrange/peaks, M-D2), FWStream::setFormat difference, MIDI/AC3/pair engines. Next: M-D2 (sample converters), then M-E.

### 2026-10-15 - M-D2 sample path written (R46)

`A1-ghidra/Q-MD2-sample-path.md`: IOAudioEngine clip/convert entry chain (sub-engine masks, `MainEngineClipOutputSamples`, `ClipPCMOutput`, `ConvertInputSamplesCore`), float<->scaled-int24 converters (floor rounding), LRLR rearrange to AM824 quadlets, FW1814 `ResetRearrangementProcs` and the 16-frame input-delay workaround. Clone: `FWSamples.{h,cpp}`, `FWDiscreteEngine` clip/convert, `FWBaseEngine`/`FWEngine2` sample methods, `FW1814Device::{ResetRearrangementProcs,WorkaroundFor1814InputDelay}`; `SetIOFormat` now installs the procs. Host test of the converters passes; build PC `make` clean + `kextutil -t -n` OK; nothing loaded. Not ported (not on the FW1814 path): `_SamplePeaks*`, LLRR, 1212CS, remap tables. Corrections: Q9, Q3, Q13. Open: reason for the input delay (guess), `[+0x250]` consumer, meter source (M-F).

### 2026-10-15 - M-E mixer / property framework / clock / HSCI written (R47)

`A1-ghidra/Q-ME-mixer-property-framework.md` (new): property dictionary + typed props, six channel groups with batch writers, FWSettings{Levels,AuxSend,Rotary,ABSwitch} and every FW1814 MA* key, HSCI clock frame (bytes from the assembly), SetClockSource / blank slate / SwitchDigitalSignals / ReconnectP2P, AV/C selector / feature / mixer-connect frames, 50 ms control-packet poll and the 52-entry meter page, FW1814 peak procs, init flow, unique IDs, channel names. Clone: `FWFunctor.h`, `FWChannelGroup.*`, `FWPropertyDict.*`, `FWSettings.*`, `FWMixer.*`, `FWMixerRegs.h`, `FWHsci.*`, `FWBusReset.*`, `FWAudioDeviceProps.cpp`, additions to `FWAudioDeviceChain.*`, `FWAvc.*`, `FWSamples.*` (peaks), engine registration/unique IDs. Build PC `make` clean + `kextutil -t -n` OK; nothing loaded; host test of the channel-group word encoding passes. Corrections: Q15f (rotated writer belongs to the aux-send group), Q-MD2 (peak proc IS used by the FW1814), Q-P4 B.3 (three open wire items closed). Deviations (labelled in code): control-packet processing on the work loop instead of a new thread per read; AV/C mixer commands synchronous instead of the async queue; `getProperty` result parked per key instead of leaked. Open: sleep/wake hooks (Build row M-E2), user-client notifications (M-F), stereo-pair engines (M-J), IOFWReadQuadCommand byte order and `IOCreateThread` on 10.13.6 (integrated run).

### 2026-10-15 - M-F user client and M-E2 power hooks written (R48)

`A1-ghidra/Q-MF-userclient-and-power.md`: `FWKIPCUserClient`/`FW1814UserClient` (selectors 500-507, meter page, MIDI rings hook, type 0/1 notifications), property-change delivery via the dictionary, `IOUserClientClass` in the FW1814 personality, device power hooks. Build PC clean + `kextutil -t -n` OK (mach_msg_send_from_kernel_proper links); nothing loaded. Deviations: terminate() in clientClose, IOLock wait, locked registry. Open: MIDI (M-H), integrated-run checks of mach_msg layout and IOCreateThread.

### 2026-10-15 - M-J pair engines written (R49)

`A1-ghidra/Q-MJ-pair-engines.md`: FWTimeSlaveEngine / FWPairEngine / FWPairStream, sub-engine set on the main engine, CreateStereoPairSubEngines + naming, ResetStereoPairIOFormats. Build clean + kextutil -t -n OK; nothing loaded. Optional at run time (MAEnableStereoPairEngines). Open: engine count vs SL and IOAudioEngine::free with the shared status page (integrated run).

### 2026-10-15 - M-H kernel MIDI written (R50)

`A1-ghidra/Q-MH-kernel-midi.md`: MIDI ports (8 KiB shared pages), engine port arrays + MAMIDI*Ports properties, start/stop with isoch refcounting, TX conformant-MIDI scheduler (`0x81|byte<<8` into empty `0x80` slots), RX byte rings, user-client ring memory (0x400+n / 0x800+n). Build clean + kextutil -t -n OK; nothing loaded. Open: user-space CoreMIDI plug-in (Build row M-H2), host-clock timestamps.

### 2026-10-15 - M-I AC3 written (R51)

`A1-ghidra/Q-MI-ac3.md` (corrects Q12): cac3 format offer on the output stream, Enable/DisableAC3Format, TX-only AC3 transport, ClipAC3Output (16-bit words as `40 hi lo 00` in channels 4/5), SetAC3OutputState inert on the FW1814. Build clean + kextutil -t -n OK; nothing loaded. Open: placement of the client data for a non-mixable stream, S/PDIF transmit mode (integrated run).

### 2026-10-15 - helper H.3 / H.4 written (R52)

`firmware-helper/{helper.h,Gating.c,Settings.c,main.c}` (record `A4-helper/H13-clone-gating-and-settings.md`): sleep + console-user gating with re-walk on wake/login, per-GUID settings persistence (4 s coalescing, console-user preferences, one-shot replay), MAFirmwareStart now gated (`--no-gate` for bench). Builds clean on the dev Mac and the build PC; not run. MIDI plug-in decision (reuse the reference plug-in via the abstract `com_m_audio_FW1814Device` alias class) recorded in Q-MH.

## 2026-10-17 (evening) - M-U, panel rate list, MIDI plug-in load
Committed the second integrated session (`2b2e247`). Then: **M-U** pass-through controls added to the kext by user decision (v1.9 asm 0x23cb8/0x23d8f; 18 level + 1 mute verified in the registry); **panel rate list** now from CoreAudio (`availableRates`, 15 host tests); current kext (staged, `kextutil -t -n` OK) loaded from /tmp on the build PC; the **unmodified reference MIDI plug-in installed** in `/Library/Audio/MIDI Drivers` - it loads and publishes "FireWire 1814" 1 in / 1 out, sends accepted (`tools/midiprobe.c`). AMS "Show MIDI Studio" crash analysed: Apple CoreUI frames only, not attributed to the clone. AMS label: new user observation supports the "built at device-add" hypothesis. Open: MIDI wire loopback (needs a DIN cable), M-S republish decision, installer must copy the plug-in (IN.5). Evidence: capture 82, R58.

## 2026-10-17 (night) - icon, tag, first installer
Tag `v0.1.0-first-working-2026-10-17`. MIDI device icon fixed (R59). First unsigned installer `dist/OpenAudioFW1814-0.1.0.pkg` built on the build PC and installed there: the current kext now lives in `/Library/Extensions` (was the Oct 15 build). User approved (a) installing the kext and (b) re-publishing the audio engine at S/PDIF<->ADAT switches so the AMS sidebar label follows (M-S) - (b) is designed, not implemented (needs: MIDI port arrays carried to the new engine, controls re-established, old engine quiesced first).

## 2026-10-17 (night, later) - M-S engine re-publish done
R60 / capture 83: the audio engine is re-published at every S/PDIF<->ADAT switch that changes the channel counts; AMS sidebar follows (16/12 and 16/6 without reopening), MIDI sends survive, no panic. Clients lose the device at the switch (by design). Installed kext in /Library/Extensions is still the pre-re-publish package build; the re-publish build is loaded from /tmp and in a rebuilt package (dist/).

## 2026-10-17 (night, last) - M-S re-publish disabled
**Correction (2026-10-17, later):** the re-publish is switched OFF by default (`kRepublishAtOpticalSwitch = false` in `FWAudioDeviceChain.cpp`). Reason: reAMP froze when the engine was replaced during playback - a re-publish removes the device from CoreAudio and clients that do not re-attach lose their stream. User decision: follow the reference/official behaviour - pause the stream, change the formats in place, resume (the clients' I/O continues after a short interruption). Consequence accepted: the 10.13 Audio MIDI Setup sidebar label can stay stale after an optical switch. The re-publish code and its idle-only deferral (`RepublishIfIdle`) stay in the tree, hardware-verified for the label (capture 83) but not for client survival.

Verified 2026-10-17 (capture 84): with the re-publish off, an optical switch under a duplex client pauses the stream ~4 s, changes the counts in place (12 <-> 6 out) and the client's callbacks resume by themselves (86/s), same device object, no panic.

## 2026-10-17 (night, last) - panel ADAT sw-return meters fixed (R61)
ADAT bus meters on the Mixer page / Meters window were 2 slots early (4 + n instead of 6 + n); fixed, host tests pass, panel restarted from the build tree for the user to re-check.

## 2026-10-17 (afternoon) - stale output format fixed, dmg
Stale output-stream channel count fixed (R68, captures 85/86). Package with the fix installed on the build PC; `dist/OpenAudioFW1814-0.1.0.dmg` (installer + Uninstall app, custom layout) built by `installer/make-dmg.sh` (R69). Still open: streaming-client check of the format fix, the 11:23 mixOutputSamples panic (may be the same stale state), sleep/wake, MIDI on the wire, reboot autoload, clean-machine install, signing.

## 2026-10-17 (evening) - second Mac
R72: rebuilt 0.1.0 disk image installed on a MacBook Air (10.13.6, SIP off) through Apple's Thunderbolt-to-FireWire adapter: install, restart, driver load, panel - worked (user). README updated (two Macs, adapter). Open: SIP enabled, signing.

## 2026-10-17 (night) - decision: homebrew, unsigned, SIP off (R73)
No signing, no notarisation, no SIP-enabled support, ever; docs and installer text updated; IN.1/IN.6 cancelled.

## 2026-10-17 (night) - MacBook Air reachable
SSH key auth to the MacBook Air (192.168.100.22) set up by the user (Remote Login + ssh-copy-id); bench tools copied to ~/fw1814-captures/tools/ there. `sudo` needs a password on the Air. Engine was running (1 client) when checked, so no rate change was made.

## 2026-10-17 (late) - second-client loop fixed (R75)
performErase now clears the mix buffer like the reference; loopcheck shows no breaks with four second-client events on the MacBook Air (test kext loaded from /tmp there). Next: rebuild/ship pkg+dmg, install on the Air, replace the release asset.

## 2026-10-17 (late) - meters clear on stop (R76); pkg + dmg rebuilt with R75 + R76
Test kext FC8AD9DA verified on the MacBook Air by the user. Release asset must be replaced by the user (new sha256 in the build report).

## 2026-10-21 (late) - macOS 14.8.9 Intel port started (R125)
Goal: driver working on the dev Mac. Done: kext links against the 15.2 SDK, 778/778 imports resolve on the live kernel (E-17); `BAND=modern` in `kext/Makefile`. Next: `scripts/install-sonoma.sh` (stage, `kmutil`, approval, restart), helper + panel reused from 0.2.2, then one load-and-play check with the interface (ask before the load; a panic reboots this Mac). Not loaded yet.

## 2026-10-21 (late night) - Modern kext plays on 14.8.9 (R126); phones-only monitor law + edition About (R127); public repo renamed
* **R126:** the Modern kext loads on the dev Mac and the FW Audiophile plays through the Thunderbolt-to-FireWire adapter (user heard it; TX callbacks regular). After 2 min 43 s of real use, one dropout: the adapter's FireWire controller left PCI first. The interface was bus-powered, and the user power-cycled it to recover. Cause open; the user suspects a bad contact (on demand).
* **R127 (user):** the dB-linear monitor law now covers the phones only. Outputs and aux out are back on the official fader and knob laws. The About tab names the edition (`OAEdition`, `make -C control-panel app BAND=modern`). Panel tests ALL PASS on the build PC. The Modern panel is installed on the dev Mac; the kext with the knob change is built but **not installed** (`scripts/install-modern.sh`, sudo, approval, restart).
* **Public repo renamed** to `github.com/bhsardinha/OA-BeBoB-MAFW` (About link, sync-to-public.sh, README, CLAUDE.md, the public copy's remote).
* Trackers: `BROAD_COMPATIBILITY/BROAD_COMPATIBILITY_PLAN_v2.xlsx` (new; v1 kept), Audiophile `Build` AP-5.4 note. Nothing committed.

## 2026-10-21 (night, later) - package 0.3.0 per edition, disk images, installs through .pkg only (R128)
* **User rules:** installs only through the `.pkg` (memory `feedback_install_via_pkg_only`); test package **0.3.0** = driver 0.2.1 + panel 1.4, two editions.
* **Built and checked:** `dist/OA-BeBoB-MAFW-0.3.0-{classic,modern}.{pkg,dmg}`: per-edition OS range enforced (checked with `installer -volinfo` on 14.8.9 and 10.13.6), the Modern postinstall gate (kmutil vs the shipped import list) works, Requirements on the installer pages and the dmg backgrounds.
* **Panel 1.4:** status menu reordered; click = panel, right click = menu; Presets kept; A/B and knob step removed from the menu.
* **Not installed:** Classic waits for the build PC to be idle (the user was running reAMP on the Audiophile there). Modern is for the user to run on the dev Mac. Uninstall on 11+ (`kmutil rebuild`) is unverified.
