# macOS 12+ port of the FW1814 stack — perspective and effort (not a plan)

Written 2026-10-19 at the user's request ("how much work would it take to port 0.1.0 to macOS 12+, at least Sequoia, based on the existing macfw project but with our full solution: panel and full digital functionality"). This is an assessment for a decision, not an approved plan. Session unit as in `BUILD_AUDIOPHILE_PLAN_v2.md` §8. Estimates are estimates; every fact below says where it comes from.

## 1. Facts checked today

| Fact | Source |
|---|---|
| The dev Mac is a **MacBookAir7,2, Intel, macOS 14.8.9 (23J631)**, SIP custom with **Kext Signing: disabled**, Thunderbolt 2 (so it can take Apple's Thunderbolt-to-FireWire adapter); no FireWire controller attached now | `sysctl hw.model`, `sw_vers`, `csrutil status`, `system_profiler SPFireWireDataType` (empty), 2026-10-19 |
| On that Sonoma, **`IOAudioFamily` 540.3 is loaded** (AppleHDA uses it) and `IOFireWireFamily` 4.8.3 + `IOFireWireAVC` are installed | `kextstat`, `/System/Library/Extensions/*/Info.plist` |
| The **macOS 15.2 SDK** still ships the kernel headers the clone needs: `Kernel.framework/Headers/IOKit/audio/` (16 files, `IOAudioEngine.h` with `#warning IOAudioEngine is deprecated use <CoreAudio/AudioServerPlugIn.h>` for min-version ≥ 10.10) and `IOKit/firewire/` (32 files, incl. classic `IOFWDCLProgram.h`, `IOFWDCL.h`) | `/Library/Developer/CommandLineTools/SDKs/MacOSX15.2.sdk` |
| macfw is **GPLv3**, version 0.05.000 (2026-09-29; no newer upstream commit at `git fetch` today) | `_refs/macfw/LICENSE`, `git log origin/main` |
| macfw architecture: `AudioServerPlugIn` HAL driver + launchd user-space transport (IOFireWireLib **NuDCL** from user space) + shared-memory audio ring + Unix-socket control; no kext | `_refs/macfw/README.md` "Architecture" |
| macfw hardware-validated on Intel 12.7.6 / 13.7.8 / 14.8.9 / 15.x for FW410 and FW1814; **Tahoe 26 blocked** ("Apple removed the native FireWire stack"); **Apple Silicon not supported** | `COMPATIBILITY.md`, `KNOWN-LIMITATIONS.md` (their claims; not tested by us) |
| macfw FW1814 scope: **analog only** (out 1-4, in 1-8), 44.1-96 kHz validated, 176.4/192 experimental, **no S/PDIF, ADAT, MIDI**; no firmware upload (boot cue only); round trip 12-16 ms at 48 kHz in their loopback test (ours: ~9 ms at 48 kHz / 64 frames) | `devices/fw1814/README.md`, `KNOWN-LIMITATIONS.md`, `B4-macfw-correlate.md` §7, CLAUDE.md pitfalls |
| macfw's pace: 1,241 commits from 2026-08-10 to 2026-09-29; FW1814 work from 2026-09-06 (≈ 3.5 weeks for analog, on top of the FW410 base), with one engine binary per rate (`analog44_main.cpp` … `analog192_main.cpp`) | `git log` of `_refs/macfw` |
| Our 0.1.0 code size: kext 9,652 lines, panel 4,402, helper 419, tools 726 | `wc -l`, 2026-10-19 |

Not verified (each needs its own check): that the **classic DCL from kernel** path still works on 12-15 (`createLocalIsochPort`); IOAudioFamily behaviour differences between the 10.13.6 build and 540.3 (the build PC was off today, so its version was not read); that CoreMIDI on 12+ still loads M-Audio's x86_64 CFPlugIn; that DriverKit has no FireWire family (no DriverKit SDK on this Mac to check); whether FireWire works on Apple Silicon through adapters.

## 2. Hard limits, whatever the route

* **Sequoia 15 is the ceiling.** Tahoe has no FireWire stack (macfw's statement; consistent with no route around it short of a third-party stack such as ASFireWire, which nobody has integrated).
* **Intel is the realistic floor.** Apple Silicon is unverified for every route; for a kext it would also need an arm64e build and Permissive Security.
* The 0.1.0 deliberate deviations that exist *because of 10.13.6* (`VerifyStreamFormats`, the classic-DCL choice, the bundled Swift runtime) must be re-examined on each target OS — some may become unnecessary, some may break.

## 3. The three routes

### R1 — forward-port the kext (same architecture as 0.1.0)

Rebuild kext, helper, panel and installer with a current SDK; keep IOAudioFamily + classic DCL + the `MA*` registry control plane + M-Audio's MIDI plug-in. Loading needs SIP's kext-signing protection off (already the project's homebrew rule, R73) and the Big Sur+ flow (`kmutil`, auxiliary kernel collection, the approval/reboot step).

* **Reuse:** essentially all of 0.1.0 — panel, all digital modes, meters, helper replay, MIDI.
* **Work:** build system for the new SDK and min-version 12 (deprecation warnings become noise, not errors); installer rewritten around `kmutil` and the AuxKC reboot; drop the bundled Swift runtime (not needed on 12+); re-run the whole 0.1.0 ladder per OS (12, 13, 14, 15).
* **Unknowns that decide it:** in-kernel classic DCL and IOAudioFamily 540 behaviour. Both are answered by **one probe**: build 0.1.0 against the 15.2 SDK, load it on one Intel 12+ Mac with the 1814 behind the Thunderbolt adapter, run the existing checks.
* **Effort:** probe 1-2 sessions; fixes 2-6 (unknown until the probe); installer + per-OS ladder 2-3. **≈ 5-11 sessions.** If the probe shows a fundamental failure (no isoch port, CoreAudio refuses the engine), R1 is abandoned and the probe cost is the only loss.
* **Lifespan:** Intel 12-15, as long as Apple keeps IOAudioFamily in those releases (it does on 14.8.9 today). Deprecated API: no future past Sequoia anyway because of FireWire.

### R2 — own user-space port on the macfw architecture (clean-room)

`AudioServerPlugIn` + a launchd transport daemon (IOFireWireLib NuDCL in user space) + IPC to the panel + a new CoreMIDI driver; macfw used only as a source of *facts* (as FFADO is today), no code copied.

* **Reuse from 0.1.0:** the protocol layer (AV/C, CMP, HSCI, mixer register writes, `MAClockSource` semantics, rate × mode channel tables, `FWSamples` converters, `.bcd` reader, settings persistence) ports to user space with modest changes; the panel's UI and model are reusable if the daemon keeps the `MA*` dictionary as its IPC payload (the panel's backend changes from `IORegistryEntrySetCFProperties` + KIPC + shared peak page to one XPC or socket connection).
* **New:** the real-time transport (the hardest part: macfw needed per-rate engines, rolling TX, prefill tuning, recovery logic, and still lists artifacts under load), the HAL plug-in with **dynamic channel counts** for the digital modes (10/16/12/… per rate and mode — macfw never did this), MIDI over the transport (M-Audio's plug-in talks to the kext's selectors 500-507, so it cannot be reused), meters from user space, sleep/wake and bus-reset recovery.
* **Effort:** **≈ 25-40 sessions**, most of it the transport and the digital modes; latency likely worse than 0.1.0's ~9 ms (macfw: 12-16 ms).
* **Gains over R1:** no kext, so no SIP change; better aligned with Apple's direction for 12-15; the only route with any chance on Apple Silicon (unverified).

### R3 — build on macfw itself (fork or contribute)

Add S/PDIF/ADAT modes, MIDI, meters and the panel features to macfw's FW1814 target.

* **Effort:** **≈ 12-20 sessions** (their transport, rate switching and recovery exist and are validated on 12-15).
* **Blocking issue: licence.** macfw is GPLv3; anything derived from it must be GPLv3, which conflicts with this project's PolyForm Noncommercial licence. It would be a separate GPLv3 project (or contributions upstream, whose acceptance is the macfw maintainer's decision). Their architecture also differs from the reference-mirroring rule of this project, so the "mirror the 2014 driver" method would apply only to protocol facts.

### Not a route — AudioDriverKit

A DriverKit audio driver still needs a FireWire transport in DriverKit; none is known (unverified here: no DriverKit SDK on the dev Mac). R2 is the user-space equivalent.

## 4. Recommendation

1. **Run the R1 probe first** (1-2 sessions). It is cheap, uses the whole working 0.1.0 stack, and fits the homebrew/SIP-off policy. Candidate test machine: an Intel Mac on 12-15 with a Thunderbolt-to-FireWire adapter — the dev Mac qualifies technically (Sonoma 14.8.9, kext signing already disabled, Thunderbolt 2), but CLAUDE.md keeps it edit-only, so using it is the user's call; a kernel panic there would reboot the dev Mac.
2. If the probe passes, R1 to a release for Intel 12-15: ≈ 5-11 sessions in total.
3. Only if R1 fails, or if a no-SIP-change / Apple Silicon version becomes a goal, choose between R2 (own code, PolyForm NC, 25-40 sessions) and R3 (GPLv3, 12-20 sessions).
4. Where the code lives: like the Audiophile work, never inside `openaudio-fw1814/`. Natural home: a macOS-12 build target of `openaudio-mafw-bebob/` once it exists (then the Audiophile comes along for free under R1), or a separate folder if the user prefers to keep the two efforts apart.
5. Sequencing against the Audiophile plan: the R1 probe is independent and could run before or after; doing it **after** phase 2 of the Audiophile plan means the probe tests the multi-model tree and one port serves both models.

## 5. Decisions for the user

* D1. Allow an R1 probe, and on which Intel 12+ Mac (dev Mac or another)?
* D2. Is a no-SIP-change or Apple Silicon version a goal at all? (If not, R2/R3 stay shelved.)
* D3. If R3 is ever chosen: accept a separate GPLv3 project?
* D4. Order relative to the Audiophile plan (before it, or after its phase 2).
