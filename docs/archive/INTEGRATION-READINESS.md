# Integration readiness — what exists, what is missing, what must happen before the hardware run (2026-10-16)

Evidence for every line is a file, a build/test run on the build PC, or the `PROVENANCE-MATRIX.xlsx` `Build` tab. "Verified" means run on the build PC; "hardware" means loaded against the real FW1814.

> **Update (2026-10-16, late):** the first integrated run happened; see `docs/system-map/build-pc-runs/2026-10-16-first-integrated-run.md` for what works, fixes F1–F10 and open items O1–O4.

## 1. Stage of the full stack
| Part | Size / state | Verified how | On hardware |
|---|---|---|---|
| Research (system-map A–D) | 184 of 193 items closed (97% research progress in Painel) | docs, Ghidra, captures | — |
| Kext (M-A…M-J, M-I) | ~9.1k lines C++; builds clean on the 10.13 SDK; `scripts/stage-kext.sh` → `kextutil -t -n` OK (re-run today) | static | **only M-A and M-FW** (probe, nub lifecycle, firmware start) were ever loaded; DCL/isoch, CMP/AV-C, engines, mixer, user client, MIDI, AC3 never ran |
| Firmware helper (H.1, H.3, H.4) | ~300 lines C; compiles; gating + settings persistence | compile only | H.1 minimal helper ran; H.3/H.4 never |
| Panel (PN.1–PN.7) | ~4k lines Swift; builds, `make test` ALL PASS; whole UI renders in `--demo`; look approved | host tests + demo snapshots | never talked to the kext |
| MIDI plug-in | decision: reuse the reference bundle unmodified (needs the installer to copy it); `midi-plugin/Sources/*` are 4 leftover stubs | none | load on 10.13 unverified |
| Installer (IN.5/IN.6) | stubs; `scripts/install.sh` installs kext + helper only | — | — |
| Test harness (IN.2/IN.3/IN.4) | `tests/oracle`, `tests/hw-smoke/*` are stubs | — | — |
Build tab: 11 of 39 tasks closed; almost every M-* row is "Em andamento" because it is written but waiting for hardware (airliner rule).

## 2. Panel ↔ kext conformance (checked statically today)
- **Keys:** the panel's 54 keys equal the kext's published keys (43 literals + `MA<SWReturn|Output|Input|Headphone><Levels|Mute|Solo|Link>` expansion): no name the panel uses is missing in the kext, none left over.
- **Matching:** the panel finds `com_openaudio_FW1814Device` (the Info.plist `IOClass`); `IOServiceOpen` type 0 reaches `com_openaudio_FW1814UserClient` through the personality's `IOUserClientClass`; notification type 0 and the 4 KiB meter page (`clientMemoryForType(1)`, `peak[52]`) exist in the kext and are filled from the HSCI control packet (`ReceiveControlPacket`) and the software peak proc.
- **Array writes:** the kext tolerates shorter arrays (`OSArrayToSeriatimValues`), so the panel's 3-entry `MAMomentarySwitchRoutings` write is accepted; every element must be an `OSNumber` (the panel sends `NSNumber`).
- **Known mismatch (harmless today):** the panel addresses up to 14 output / 14 SW-return channels; the kext groups are `<4>`. The visible controls only touch pairs 1–2.
- **Not provable without hardware:** whether the real registry entry publishes `GUID` for the About tab (the panel searches the parents), the notification header layout on 10.13, the meter values.

> **Update (2026-10-16, same day):** G1 was a **false positive** of my first grep: the CoreAudio volume/mute controls (output master + per channel, input per channel, global mute) *are* created in `FWSettingsLevels::Init` (`FWSettings.cpp`); only stale comments said otherwise. The reference binary has **no caller** for the direct-monitoring and clock-source control creators, so the FW1814 has none either (`otool` of `mafw.asm`: only the definitions). G2 (rate-change gate) is implemented (`FWAudioDeviceProps.cpp`), G4 (stubs/comments) cleaned, G5 (conformance script) written and passing (`docs/CONFORMANCE.md`); the check found one real mismatch (`MAMonitorAuxOnOutput` has 3 elements, not 10), fixed. Remaining before the first run: the ladder script (G6) and the user-owned items.

## 3. Missing in the clone (found by this audit)
| # | Gap | Evidence | Needed for | Plan |
|---|---|---|---|---|
| G1 | **CoreAudio-visible controls are not created**: volume and mute controls per channel, direct-monitoring volume/toggle, and the clock-source control. The helpers `EstablishVolumeControl/MuteControl` exist in `FWPropertyDict.cpp` but nothing calls them; `FWSettings.cpp` says "IOAudioControl creation not ported". The reference creates them in `FWSettingsLevels::Init` (decomp `FWSettingsLevels__Init.c`) and `FWBaseProp::EstablishClockSourceControl`. | grep of the kext; Audio MIDI Setup capture of 2026-10-16 shows Master/Output volume sliders and a Clock source popup | parity with the reference in Audio MIDI Setup / system volume | new Build row M-N: read the reference calls, port them |
| G2 | **Rate change while a client streams has no gate** (`setProperties` applies `MASampleRate` at once). The v1.9 reference panics on this (memory `project_v19_rate_change_while_streaming_panics`); v1.11.2 behaviour not decoded. The panel cannot know whether a client streams. | `FWAudioDeviceProps.cpp:108-120` | safe first run | new Build row M-O: decide (refuse with busy, or defer until idle) and implement; during the first run do not change the rate while an app plays |
| G3 | Multi-interface registry (`FWNubRegistry`, static GUID dictionary, `ReplaceExistingMetaNub`) is a stub. | `FWNubRegistry.cpp` | M-K only (needs a second FW1814) | stays M-K |
| G4 | Stale or misleading stubs: `FWAc3.{h,cpp}` (AC3 lives in `FWEngine2`), `FWEngine1.*` (never used on the FW1814), `midi-plugin/Sources/*`; the gap comment in `FWSettings.h` still says the AV/C routing and aux-pan writes are not ported although `FWMixer.cpp` implements them. | files | honesty of the tree | new Build row M-P (trivial cleanup) |
| G5 | **Conformance check script** promised by BUILD-PLAN §0 (`docs/CONFORMANCE.md`, `conformance-check.sh`) does not exist. | BUILD-PLAN §0 | verification under the airliner rule | new Build row IN.7: script that diffs panel keys/sizes against the kext and the system-map |
| G6 | ~~`tests/hw-smoke/integrated.sh` is a stub~~ **written 2026-10-16** (13-step ladder with pass criteria and log); oracle compare, loopback and rate-sweep scripts remain stubs (IN.2). | files | the integrated run | IN.2/IN.3 |
| G7 | Installer does not copy the panel app or the MIDI plug-in; `install.sh` is kext + helper only. | `scripts/install.sh` | final packaging (last, as agreed) | IN.5 |

## 4. Consolidated "VERIFY at the integrated run" list (from the build records R43–R54)
1. `IOFWReadQuadCommand` byte order on 10.13 (HSCI packet decode → meters, front-panel knob/switch). 2. `IOCreateThread` on 10.13.6. 3. `mach_msg_send_from_kernel_proper` header layout (notification `type` at +0x18 on the panel side). 4. AV/C mixer commands are sent synchronously (reference queues them async). 5. Where IOAudioFamily puts client AC3 data for a non-mixable stream; S/PDIF transmit mode in AC3. 6. Pair-engine count vs the SL ioreg, `IOAudioEngine::free` with the shared status page. 7. MIDI plug-in load and busy-code semantics (`0xE00002D2`); host-clock ns timestamps for MIDI rings. 8. The 1814 input-delay workaround reason. 9. `[+0x250]` resync flag has no consumer. 10. `MAInputPan2` half-word wire order. 11. Helper `seteuid`/CFPreferences path on 10.13. 12. Sleep/wake `AddToDeviceList` consumer. 13. Panel: strip↔pair assignment, aux-send index for the SW-return half, aux-strip `ctrl` bit, kernel mode of the three A/B routing entries, firmware info byte order. 14. DCL/isoch: TX silence root cause is a hypothesis (CallProc missing/dropped or 4-segment ring), see CLAUDE.md.

## 5. Proposed path to the first hardware run
**Before (no hardware, Claude):** G1, G2, G4, G5 (new rows M-N, M-O, M-P, IN.7); then a pre-flight review of the highest-risk new kernel paths for NULL-safe `delete`/free (the reference panic class), and fill `tests/hw-smoke/integrated.sh` with the ladder below.
**First run (you present, no hot-plug, no flash)** — scripted in `openaudio-fw1814/tests/hw-smoke/integrated.sh` (`--preflight` runs only the read-only step 0; run today on the build PC: FW1814 on the bus, staged kext, helper and panel built, installed M-FW kext loaded, SIP currently disabled there): keep it reversible: do **not** copy anything to `/Library/Extensions`. Stage with `scripts/stage-kext.sh`, load from `/tmp` with `sudo kextutil` (a reboot clears it), start the helper by hand (`--no-gate` for the bench), run the panel app. Ladder inside the single session, each step with a pass criterion before the next: (1) kext loads, device published, `ioreg` shows `com_openaudio_FW1814Device` with the MA* keys; (2) panel connects: About tab shows firmware info and GUID, properties round-trip (write a level, read it back); (3) HSCI poll: meters and the front-panel knob move the panel; (4) CMP/isoch start: CoreAudio sees the device, silence then a test tone out, clean at 48 kHz; (5) input capture and loopback; (6) rate sweep 44.1→192 kHz and ADAT/S-PDIF switching (with no client streaming during changes, until G2); (7) MIDI loopback; (8) sleep/wake; (9) unload cleanly. Panic logs are read from the dev Mac over SSH.
**After:** installer IN.5/IN.6 last (as agreed), M-K with a second unit, latency validation M-L with a loopback jig.

## 6. What only you can do
Be present for the run; SIP-on / user-approved-kext check (IN.1, Recovery); Developer ID (IN.6); a second FW1814 (M-K); a loopback cable/jig (M-L); power-cycle the interface when a bus reset is needed (never hot-plug).
