# B5 — Snow Leopard reference Mac ground-truth

> **Correction (2026-10-14) — SL capture files** Copied from the rig's `~/fw1814-captures/2026-10-09/` into `captures/pulled/`: `62-dtrace-mixer-slide`, `63-dtrace-direct-monitor`, `64-dtrace-momentary-switch` and `64b-…-nopidfilter` (both **empty on the rig**, 1 byte — consistent with 'zero setProperties'), `65-dtrace-rate-change`, `65b-dtrace-rate-change-full` (these were cited by the README but missing from the repo); `66-dtrace-smux-toggle.txt` was **truncated** in the repo (36,292 B) and is replaced by the full rig file (51,852 B). 68, 69 and 70 are byte-identical to the rig. Not copied: the non-DTrace snapshots (`01…26`, `40`, `41`, `67a-c` — already in `captures/`/`pulled/` or superseded).


> **Correction (2026-10-14) — captures 72/73** (evidence: [PROVENANCE-RESOLUTIONS.md §R25](../PROVENANCE-RESOLUTIONS.md), captures 72 and 73). **Measured latency at every rate (SL v1.9, device idle, internal clock; `captures/72-…txt`)** — engine properties from `ioreg -r -c com_m_audio_FWEngine2`: 44.1 kHz: SampleOffset 40 / InputSampleLatency 32 / OutputSampleLatency 216 (streams out 12 / in 10 ch); 48 kHz: 48 / 32 / 231 (12/10); 88.2 kHz: 88 / 64 / 352 (8/10); 96 kHz: 96 / 64 / 387 (8/10); 176.4 kHz: 176 / 128 / 387 (4/2); 192 kHz: 192 / 128 / 387 (4/2). The 44.1/48 kHz rows reproduce the earlier B5 values (40/216/32 and 48/231/32). These are v1.9 numbers, not 1.11.3. Empirically SampleOffset equals the rate in kHz rounded down to a multiple of 8 (44→40); input latency steps 32→64→128 at 88.2 and 176.4 kHz (hypothesis: tied to the SYT interval 8/16/32, unverified). Capture 73 (interface power cycle): see the annotated timeline in `captures/73-sl-power-cycle-bus-reset.txt`.


> **Correction (2026-10-14) — MIDIServer + stale user clients** (evidence: [PROVENANCE-RESOLUTIONS.md §R16](../PROVENANCE-RESOLUTIONS.md)). MIDIServer: no `MIDIServer` process was running during the 2026-10-14 session and the live `com_m_audio_FWUserClient` creators were only the helper and the Panel, so B5:530 ('MIDIServer userclients … presumably') stays **unverified** (needs a session with CoreMIDI active). New observation: stale `FWUserClient` instances remain registered after their process exits (SL v1.9: helper pids 463/492/534 and a killed Panel pid 319) — the kext never calls `terminate()` (R16).


> **Correction (2026-10-14) — settings persistence** (evidence: [A4 H11](../A4-helper/H11-settings-persistence.md)). Capture 71 decoded: the 60-byte helper write is `MAFirmwareStart=YES`; the 6570-byte writes are the helper's saved-prefs replay (H11).


> **Correction (2026-10-14) — Part A completion batch 1** (evidence: [PROVENANCE-RESOLUTIONS.md §R8](../PROVENANCE-RESOLUTIONS.md)). Capture 71 (SL v1.9): the **helper daemon** reapplies saved settings (3 property writes at start: 60/6570/6570 B), the Panel does **not** write properties on launch; payload content not decoded. See `captures/71-sl-helper-prefs-reapply-and-panel-launch.txt`. Kext `IOSleep` could not be observed (fbt = mach_kernel only).


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - `vtable[0xce8]` on the engine is `FWEngine2::SetExternalSyncEnabled(bool) @0x11bc0`, not 'SetDigitalInputActive(false)'. Line ~530 'MIDIServer userclients … presumably' is a **guess — needs verification** (check = `ioreg -c IOAudioEngineUserClient`-style capture on the SL rig).


**Status:** done 2026-10-13 (live-hardware questions resolved; the two remaining pre-close items reclassified as A1 static follow-ups — see §13).
**Target:** `fw1814-sl` @ `192.168.100.17` — Mac Pro running Snow Leopard (10.6) with M-Audio FireWire driver **v1.9** + control panel **1.4**, FW1814 attached and enumerated.
**Scope:** the only hardware-input leg of the system map. Captures the runtime shape and runtime bus traffic of the shipping M-Audio stack as it actually behaves on real hardware, as the ground truth against which the static decode (Part A, `_extracted/` = v1.11.2) is cross-checked and against which Part C synthesis closes.

## Status / next step

**Next step:** B5 is done. The two remaining items from the pre-close list — rate-change downstream path and rate-locked guard behavior — have been **reclassified as A1 static follow-ups** (see §13) because: (a) the Panel-side write envelope is identical to every other MA* key (same `setProperties` XML blob shape as caps 61..70); (b) the kext-internal `performFormatChange` + rate-locked gate are not reachable from DTrace on this SL build (fbt covers mach_kernel only, see §12); and (c) the symbols are statically present in the v1.11.2 binary (`FWEngine2::PerformFormatChange @ 0x124cc`, `MARateLocked` compile-time string literal). Part B now advances to **B1 (FFADO)** → B2 (snd-bebob) → B3 (Sierra hack) → B4 (macfw) static correlation.

**Done so far:** full idle ioreg capture, user-prefs plist, panel appdomain plist, kextstat/launchctl/ps, StartupItem inventory, Panel v1.9 binary + Resources pulled, 46-key vendor property inventory diff'd clean between v1.9 and v1.11.2 Panel, playback-active snapshots at 44.1k (afplay) and 48k (Studio One 2 multichannel session), **full `MAClockSource` composite bitfield locked via caps 61 + 66 + 68 + 69** (sync-source bits [1:0], S/PDIF-port bit 2, optical-IN bit 3, optical-OUT bit 4, S/MUX bit 5, gate bit 30 — see §10 and `A1-ghidra/Q5-clock-source.md §Final bijection`), mixer-slide first-order capture (`MASWReturnLevels` via `setProperties`, cap 62), direct-monitor + physical-A/B-button paths captured (caps 63 + 64), rate-change-while-streaming panic documented (cap 65), **meter-path confirmed as `clientMemoryForType(1)` shared ring (cap 70, see §11)**.

## Headline findings (so far)

1. **Panel and helper daemon each open their own `com_m_audio_FWUserClient`** — not a middleman architecture. See [§3](#3-user-client-topology--panel-and-helper-each-open-the-vendor-userclient-directly). This revises the earlier "panel talks to kext only via helper" hypothesis and *validates* the Proposal.md simplification about Panel ↔ kext directness for the vendor channel.
2. **The sync-source selector is not exposed as `IOAudioSelectorControl`** — it only exists as the `MAClockSource` composite property, written via `IORegistryEntrySetCFProperties` on the device registry entry (NOT the vendor `FWUserClient`, see finding #9 + §10). See [§4](#4-selector-surface--no-clock-source-ioaudioselectorcontrol-vendor-channel-only). IOAudioFamily only sees status-level selectors (`DigitalIn`, `UnlockedClock`).
3. **`FWEngine1` + `FWEngine2` are both build-time class names** (not instance counters) — verified in **both** v1.9 (SL) and v1.11.2 (`_extracted/`) kext binaries. FWEngine2 is the multichannel flavor (`IOAudioEngineFlavor=1`); FWEngine1 is the stereo-pair flavor, gated by `MAEnableStereoPairEngines` (= 0 on this machine, so not instantiated).
4. **Helper is a 2007-era `StartupItem` bundle, not a launchd daemon** on this SL build — `/Library/StartupItems/M-Audio FireWire Audio Helper/M-Audio FireWire Device Helper` (pid 64, label `com.m-audio.firewire.helper`). Co-resident with two sibling StartupItems — `M-Audio Firmware Loader` (USB Cypress firmware uploader, out of scope) and `MAudioAudiophileUSB/AudiophileCS` (also USB, out of scope).
5. **Playback and control plane are fully independent at the IOKit level** — streaming activates FWEngine2 (state 0 → 1) via a dedicated `IOAudioEngineUserClient` owned by the user process (afplay in our test). No change in the vendor `com_m_audio_FWUserClient` retain counts, no MA* property change on FW1814Device. See [§6a](#6a-playback-active-state-afplay-owns-the-engine-user-client-directly). Validates that the vendor-selector channel is only used for *configuration*, not streaming.
6. **Panel + v1.9 vs v1.11.2 vendor-property surface is identical** — 46 `MA*` key literals match exactly between the pulled v1.4 Panel (SL) and the `_extracted/` v1.5.2 Panel. Control-plane protocol was stable 2009→2014. Eases cross-version synthesis in Part C.
7. **Runtime sample-rate change captured (44.1k → 48k)** under a Studio One 2 multichannel session. `IOAudioEngineSampleOffset` moves 40 → 48 and `IOAudioEngineOutputSampleLatency` moves 216 → 231 — **latency and sample-offset are rate-dependent and must be recomputed on `performFormatChange`.** `IOAudioEngineInputSampleLatency` stays 32 (rate-independent). `MABytesPerCycle` pref stays stale at 976 — not a runtime truth source. See [§6c](#6c-multichannel-daw-session--studio-one-2-pid-552).
8. **The vendor `com_m_audio_FWUserClient` class serves Panel + helper + CoreMIDI MIDIServer concurrently** (4 live instances during the Studio One session: pid 64 helper, pid 172 Panel, pid 545 + pid 556 MIDIServer). Not a per-role class; the clone's vendor userclient must multiplex connection types. See [§6c](#6c-multichannel-daw-session--studio-one-2-pid-552).
9. **Panel clock-source changes go through `IORegistryEntrySetCFProperties`, NOT `IOConnectCallMethod`** — Panel sends a serialized XML plist `<dict><key>MAClockSource</key><integer size="32">0x4000????</integer></dict>` to the FW1814Device registry entry; the kext receives it in `com_m_audio_FW1814Device::setProperties`. The vendor `com_m_audio_FWUserClient` is NOT in the clock-change path at all. `MAClockSource` is a **composite 32-bit bitfield** (sync source in bits [1:0], S/PDIF-port bit 2, optical-IN bit 3, optical-OUT bit 4, S/MUX bit 5, required gate bit 30) — full lock via caps 68 + 69. See [§10](#10-clock-source-change-path--registry-setproperties-with-the-composite-bitfield) and `A1-ghidra/Q5-clock-source.md §Final bijection`.
10. **Panel meters read from a pre-mapped `clientMemoryForType(1)` shared ring on the vendor `com_m_audio_FWUserClient`** — zero IOKit MIG traffic from Panel or helper during a 30s capture while meters animate on a steady tone (cap 70, see [§11](#11-meter-path--panel-reads-from-a-pre-mapped-clientmemoryfortype1-shared-ring)). Panel has three `IOKit SM=SHM` regions mapped at startup; one is the vendor meter ring per A1-Q11 + A2-P9. Helper is idle at ~0.4 msg/s = not on the meter path. Clone's vendor userclient must implement `clientMemoryForType(1)` with a kernel-updated meter ring.

## 1. Setup — bundle versions in play

| Component | Bundle | Version | Role in B5 |
|---|---|---|---|
| v1.9 kext | `/System/Library/Extensions/M-AudioFireWireBeBoB.kext` on SL | 1.9 (c. 2009) | **running** on hardware |
| v1.11.2 kext | `_extracted/kext/M-AudioFireWireBeBoB.kext` | 1.11.2 (2014) | **shipping target**, decoded in Part A1 |
| Panel 1.4 | `/Applications/M-Audio FireWire.app` on SL | 1.4 | **running** (pid 172) |
| Panel 1.4 (static) | `_extracted/panel/M-Audio FireWire.app` | 1.4 | decoded in Part A2 |
| Helper (SL) | `/Library/StartupItems/M-Audio FireWire Audio Helper/` | undated (StartupItem bundle) | **running** (pid 64) |
| Helper (2014) | `_extracted/helper/` | 1.11.2 era | decoded in A4 |
| FW1814 firmware | onboard device | build "20070713080440" | **present and loaded**, bCoD payload confirmed via `MAFirmwareInfo` |

Per `memory/feedback_version_skew_v19_vs_v1112.md`: **v1.9 and v1.11.2 may diverge in method sets, selectors, FCP bytes, firmware upload protocol, panel plist keys.** Every claim below names its bundle; cross-bundle claims call out that both agree.

## 2. Capture inventory

All captures under `captures/`. Provenance line numbers below cite these files unless otherwise noted.

| File | Source | Purpose |
|---|---|---|
| `12-ioreg-full.txt` | `ioreg -lw0` on SL, 2026-10-09 | authoritative IOKit tree with all properties — 9869 lines |
| `05-kextstat.txt` | `kextstat` on SL | loaded kext list — confirms v1.9 M-AudioFireWireBeBoB running |
| `06-launchctl-list.txt` | `launchctl list` on SL | session launchd items; pid 172 = Panel. Note: pid 64 helper is StartupItem not launchd (see §5) |
| `08-system-profiler.txt` | `system_profiler SPFireWireDataType` | bus topology |
| `23-fw1814-current-prefs.xml` | extracted FW1814 (`D6C040074B9FB`) section of `/Library/Preferences/com.m-audio.device-settings.firewire.plist` | runtime user prefs — mixer, routing, clock source, direct-monitor |
| `pulled/device-settings.firewire.plist` | `/Library/Preferences/` on SL | full device-settings plist (keyed per-GUID) |
| `pulled/panel-appdomain.plist` | `/Library/Preferences/com.m-audio.appc.MAfw.plist` on SL | panel UI state — tab index, scroll positions, splash/tooltip prefs |
| `pulled/M-Audio_Firmware_Loader_2009` | `/Library/StartupItems/M-Audio Firmware Loader/` on SL | the USB firmware loader (sibling StartupItem — out of FW1814 scope but captured for the record) |
| `pulled/panel-app/M-Audio FireWire.app/` | `/Applications/M-Audio FireWire.app/` on SL | full v1.4 Panel bundle (universal i386+ppc; build 01.09.003) for Carbon-structure diff vs `_extracted/panel/` |
| `30-panel-v19-MA-keys.txt` | `strings -arch i386` on pulled Panel binary, grep `^MA[A-Z]` | 46 vendor-property key literals — zero delta vs v1.11.2 Panel |
| `40-ioreg-playback.txt` | `ioreg -lw0` on SL during `afplay` loop with FW1814 as default output | playback-active IOKit tree — FWEngine2 state=1, 7 engine-user-clients, afplay is the active streamer |
| `41-ps-playback.txt` | `ps -ax -o pid,command` concurrent with 40 | pid roster at capture time (pid 503 = afplay, pid 462 = System Preferences) |
| `50-ioreg-logic-playback.txt` | `ioreg -lw0` on SL during Studio One 2 multichannel session at **48 kHz** | DAW-active IOKit tree — rate change evidence, 4 vendor `FWUserClient` instances (Panel + helper + 2× MIDIServer), latency numbers per rate |
| `51-ps-logic-playback.txt` | `ps -ax -o pid,command` concurrent with 50 | pid roster (pid 552 = studioapp, pid 556 = MIDIServer, pid 172 = Panel) |
| `60-dtrace-clock-dropdown-attempt1.txt` | `dtrace fbt::is_io_connect_method*:entry` filtered to pid 172 while rotating Panel clock dropdown | **negative result** — Panel did NOT invoke any `is_io_connect_method*` or `is_io_connect_set_properties` or `is_io_connect_async_method`. Proof that the Panel's clock-change path does NOT go through `IOConnectCallMethod` on `com_m_audio_FWUserClient` |
| `61-dtrace-clock-dropdown.txt` | `dtrace fbt::is_io_registry_entry_set_properties:entry` filtered to pid 172 while rotating Panel clock dropdown | **positive result** — captured 7 property writes with full XML payload; proved `setProperties` as the dispatch channel. The initial bijection labels (0x1C=S/PDIF, …) were later retracted because clicks were not labeled per-click — real bijection comes from caps 68 + 69 |
| `pulled/62-dtrace-mixer-slide.txt` | `is_io_registry_entry_set_properties` while sliding a mixer fader | `MASWReturnLevels` writes via `setProperties` — same channel as clock-source. Kernel-side absorber identified in A1-Q15b |
| `pulled/63-dtrace-direct-monitor.txt` | A/B button-function selector | `MAMomentarySwitchSource = 0/1` picks DM-mode vs HP-A/B-mode (resolves `P-manual-crosscheck.md` Item #8) |
| `pulled/64-dtrace-momentary-switch.txt` + `64b-*` | physical front-panel A/B button | **zero** `setProperties` on any process — physical button bypasses Panel entirely; DM state is kext-internal (resolves `P-manual-crosscheck.md` Item #9) |
| `pulled/65-dtrace-rate-change.txt` | sample-rate dropdown while playing | 1 frame of `IOAudioStreamFormat` dict captured before v1.9 **kernel-panicked** (page-fault in `IOAudioFamily buffer realloc → _blkclr`). See `project_v19_rate_change_while_streaming_panics.md` |
| `pulled/66-dtrace-smux-toggle.txt` | opt-out flip + S/MUX toggle (clock held at Internal-muted) | First evidence `MAClockSource` is composite — bits 4/5 flip independent of bits [1:0] |
| `pulled/67a/b/c-ioreg-channels-*.txt` | ioreg snapshots in 3 digital modes | `IOAudioStreamAvailableFormats` is **dynamic**: rebuilt on digital-mode + active-rate change |
| `pulled/68-dtrace-digital-mute.txt` | sync-source radio toggles (muted ↔ unmuted internal) + digital-port-input toggle | Sync source lives in bits [1:0] of `MAClockSource`: `00`=int-unmuted, `11`=int-muted; `01/10`=ext-dig/wc (pinned by cap 69). Bit 3 = optical-IN mode (1=ADAT, 0=S/PDIF) |
| `pulled/69-dtrace-syncradios-spdifport.txt` | external-digital + word-clock radios + S/PDIF-port toggle | **Full sync-source bijection locked**: bits[1:0] `01` = External-digital, `10` = Word-Clock. **Bit 2 = S/PDIF input port** (0=optical, 1=coaxial). Supersedes the retracted cap-61 labeling |
| `pulled/70-dtrace-meter-path.txt` | 30s dtrace of all IOKit MIG stubs + `mach_msg_trap` on pid 65 (helper) + pid 167 (Panel) with afplay tone + Panel on Mixer tab | **Meter path = pre-mapped `clientMemoryForType(1)` shared ring on vendor FWUserClient.** Zero IOKit MIG calls from either process; Panel's three `IOKit SM=SHM` vmmap regions prove the mapping. Helper is idle (13 msg / 30s). Resolves hypothesis (c) in §11; rules out (a) `IOAudioLevelControl` polling and (b) helper → Panel Mach-port notifications |

**Panel bundle pulled 2026-10-09** — captures/pulled/panel-app/M-Audio FireWire.app/

| | SL v1.9 (pulled) | `_extracted/` v1.11.2 |
|---|---|---|
| CFBundleShortVersionString | 1.4 | 1.5.2 |
| CFBundleVersion | 1.4 | 1.5.2 |
| CFBundleIdentifier | com.m-audio.appc.MAfw | com.m-audio.appc.MAfw |
| Resources/build-number.txt | 01.09.003 | — (not present) |
| Architectures | universal i386 + ppc | i386 only (ppc dropped in 2014 rebuild) |
| Binary size | 4,956,652 bytes | 2,551,104 bytes |
| Timestamp on disk | 2009-11-04 | 2014-era |

**Vendor-property surface is stable across the two shipping Panel binaries** — grep of `^MA[A-Z]` compile-time string literals in each binary (i386 slice) returns the same 46 keys in both, with zero deltas (`captures/30-panel-v19-MA-keys.txt`). The control-plane protocol between Panel and kext did not change between 2009 and 2014. **This means findings from either Panel apply to the other** — easing the version-skew concern for Panel-side work.

(Note: this doesn't mean the kext's internal protocol is byte-identical — only that the Panel's compile-time key literals match. A FCP-byte-level cross-check still requires live dtrace, pending below. Also: a few pref-plist keys like `MAInputLink`, `MAHeadphoneLink`, `MABytesPerCycle`, `MASampleRate`, `MADirectMonitoringActive` are not compile-time literals in the Panel strings — they are either kext-only or built at runtime from the prefix strings `MAInput`, `MAHeadphone`, `MAAuxOutput`, `MASWReturn`, `MAOutput` which **are** compile-time literals. See §5 for the pref-plist superset.)

**Playback-active ioreg captured 2026-10-09** — `captures/40-ioreg-playback.txt`, taken ~4s into a looped `afplay /System/Library/Sounds/Submarine.aiff` loop with FW1814 selected as default output. Diff vs idle:

| | Idle | Playback |
|---|---|---|
| FWEngine2 node | `busy 0 (6 ms), retain 78` | `busy 0 (17 ms), retain 81` |
| `IOAudioEngineState` | 0 (stopped) | **1 (running)** |
| `IOAudioEngineNumActiveUserClients` | (property absent) | **1** |
| `IOAudioEngineUserClient` subscribers on FWEngine2 | 5 (coreaudiod, loginwindow, UserEventAgent, SystemUIServer, Panel) | 7 — same 5 **plus** System Preferences (open on Sound prefs panel) and **afplay (pid 503)** |
| `IOAudioEngineUserClientActive = 1` on | — | pid 503 afplay (id 0x100000717) — the only subscriber with the active flag set |
| `IOAudioEngineUserClientActive = 0` on | — | pid 462 System Preferences (id 0x1000005c9) — explicitly non-streaming |
| `m_audio_b_FWStream` retain (both input + output streams) | 7 | 8 |
| `IOAudioStreamNumClients` (both streams) | 0 | 1 |

**Confirmed:**

- **afplay owns the IOAudioEngineUserClient directly** — on Snow Leopard, user-space audio clients open `IOAudioEngineUserClient` themselves, not via a coreaudiod proxy. coreaudiod is just one of several passive subscribers. This is a Snow Leopard-era HAL characteristic; modern macOS routes all engine access through coreaudiod. **Impact on Part C C5 (userclient surface):** the engine must correctly accept N passive subscribers + 1 active streamer concurrently (standard IOAudioFamily behavior).
- **Audio flow activates FWEngine2 without touching the vendor `com_m_audio_FWUserClient`.** No new vendor-userclient creator appeared; retain counts on the two `com_m_audio_FWUserClient` instances (Panel, helper) unchanged. **The streaming path and the control-plane path are fully independent at the IOKit level.**
- **No MA* property changes on FW1814Device during playback.** The pref/control-plane state is quiescent during audio I/O.

**Still pending (planned captures):**

- FCP wire traffic via dtrace (`fbt` on `IOFireWireController::sendAVCCommand` family) while exercising Panel.
- ~~Meter data path trace~~ — resolved by cap 70, see §11: meter telemetry flows via the vendor FWUserClient's `clientMemoryForType(1)` shared ring, mapped once at Panel startup. No helper daemon involvement; no `IOAudioLevelControl` polling.

## 3. User-client topology — Panel and helper each open the vendor userclient directly

Two `com_m_audio_FWUserClient` instances exist in the ioreg tree, both as children of the FW1814Device node:

```
line 5807: com_m_audio_FWUserClient id 0x100000542
line 5809:   IOUserClientCreator = "pid 64, M-Audio FireWire"      ← helper daemon
line 5812: com_m_audio_FWUserClient id 0x10000057b
line 5814:   IOUserClientCreator = "pid 172, Panel"                 ← Panel app
```

(Cites `captures/12-ioreg-full.txt:5807-5814`.)

Each userclient is a **separate IOKit connection** owned by a separate process. The helper does **not** proxy the Panel's vendor requests. This revises the earlier conversation-level hypothesis that the helper acts as a middleman for the Panel's control-plane traffic.

**Implications for Part C and Proposal.md:**

- The vendor-channel "control-plane" is a direct Panel ↔ kext IOKit path. The daemon is not on the critical path for mixer/routing/clock-source UI → device-register changes.
- The daemon's role is narrower than previously drafted: firmware upload (bootloader-mode device handling, A4-H3/H4/H5), device-presence tracking, and possibly MIDI port registration. Not control-plane proxy. Verify via FCP dtrace in the next capture pass — if FCP transactions originate from pid 172 (Panel) during panel UI exercise, that closes it.
- Proposal.md's simplification "Panel talks to kext directly, bypass the fwhelper" is **validated** for the vendor-selector channel. The caveat about meter-path routing is still open — pending FCP+meter dtrace.

Additionally, Panel also opens **standard `IOAudioControlUserClient` instances** per per-channel toggle alongside coreaudiod and SystemUIServer — e.g. the four Output Analog mute toggles at `12-ioreg-full.txt:4168-4246` show four creators each: coreaudiod, SystemUIServer, Panel (standard CoreAudio subscription), and no vendor-specific slot. So Panel uses **two parallel channels** to kernel:

1. **Standard IOAudioFamily** per IOAudioControl (one `IOAudioControlUserClient` per control the Panel binds to) — same mechanism Audio MIDI Setup / coreaudiod use.
2. **Custom vendor** via `com_m_audio_FWUserClient` for everything that doesn't map to a standard `IOAudioControl` (clock source, routing matrix, direct-monitor state, aux send levels, HSCI vendor commands).

(Panel userclient subscriptions: `captures/12-ioreg-full.txt:4165, 4191, 4217, 4243`.)

## 4. Selector surface — no clock-source `IOAudioSelectorControl`, vendor channel only

Static grep of all `IOAudioSelectorControl` nodes in the ioreg (`captures/12-ioreg-full.txt:2628, 2832, 3093, 3211, 3463, 3490`):

- Lines 2628, 2832, 3093, 3211 — selectors on the **built-in Mac Pro audio devices** (IntSpeakers, LineOut, DigitalOut, LineIn). Not FW1814.
- Line 3463 — FW1814Device-level selector, only entry `{SelectionValue=1936745574, DescriptionKey="DigitalIn"}` (`captures/12-ioreg-full.txt:3466`). ControlSubType=1768845428. Single-selection — status surface, not a 2+-way selector.
- Line 3490 — FW1814Device-level selector, only entry `{SelectionValue=1970170987, DescriptionKey="UnlockedClock"}`, ChannelName="All Channels" (`captures/12-ioreg-full.txt:3493-3497`). Also single-entry status.

**No `IOAudioSelectorControl` exists under FWEngine2.** The engine subtree (lines ≈3976–5807) contains only `IOAudioLevelControl` + `IOAudioToggleControl` + user-client child nodes.

Interpretation: **the sync-source selector is driven exclusively via `IORegistryEntrySetCFProperties` on the device registry entry** (NOT the vendor `FWUserClient` — see §10 for the dtrace proof) — Panel writes the composite `MAClockSource`; kext receives it in `com_m_audio_FW1814Device::setProperties`; kext issues the HSCI/FCP vendor-dependent CONTROL. The current observed `MAClockSource = 1073741855 = 0x4000001F` value from `captures/23-fw1814-current-prefs.xml:50-51` is a **composite** — bits [1:0] = 3 (Internal-muted sync source), bit 2 = 1 (coaxial S/PDIF input port), bit 3 = 1 (ADAT optical-IN mode), bit 4 = 1 (ADAT optical-OUT mode), gate bit 30 = 1. Never surfaces as a standard IOKit selector.

**Why this matters for the clone:** the shipping Panel drives the composite via `setProperties`, not via a userclient method and not via standard CoreAudio APIs. Any port to AudioServerPlugIn later must synthesize this itself.

**Resolved (caps 68 + 69):** the Panel-side encoding was fully captured. Full composite bitfield is locked in §10 and `A1-ghidra/Q5-clock-source.md §Complete MAClockSource bitfield`.

## 5. User-prefs mixer surface

Full FW1814 pref keys for GUID `D6C040074B9FB` (`captures/23-fw1814-current-prefs.xml`):

| Key | Shape | Current value |
|---|---|---|
| `MAInputLevels` | array[18] | per-input level (8 analog + 2 S/PDIF + 8 ADAT) |
| `MAInputLink`, `MAInputMute`, `MAInputSolo`, `MAInputPan2` | array[18] | per-input flags + pan |
| `MAInputRoutingV2` | array[9] | input routing matrix (9 slots) |
| `MAAuxSendLevels` | array[22] | aux send matrix (22 slots — 18 inputs + 4?) |
| `MAAuxOutputLevels`, `MAAuxOutputLink`, `MAAuxOutputMute`, `MAAuxOutputPan2` | array[2] | aux output stereo pair |
| `MAHeadphoneLevels`, `MAHeadphoneLink`, `MAHeadphoneMute` | array[4] | 2 stereo HP outs |
| `MAHeadphoneSecondSource`, `MAHeadphoneSourceV2` | scalar | HP source selector |
| `MAOutputLevels`, `MAOutputLink`, `MAOutputMute`, `MAOutputSolo` | array[4] | 4 main outs (matches channel map: 4 Analog Out) |
| `MASWReturnLevels`, `MASWReturnLink`, `MASWReturnMute`, `MASWReturnSolo` | array[4] | 4 software-return channels |
| `MASWReturnRoutingV2` | array[2] | SW-return routing (2 slots) |
| `MAMomentarySwitchRoutings` | array[4] (keys 1,2,3,4) | hardware momentary-switch routes |
| `MAMomentarySwitchSource` | scalar | switch source selector |
| `MAMonitorAuxOnOutput` | array[3] | aux-on-output per bus |
| `MABytesPerCycle` | scalar | 976 (= 44.1 kHz AM824 payload estimate — see §6) |
| `MASampleRate` | scalar | 44100 |
| `MARateLocked` | scalar | 1 (locked) |
| `MAClockSource` | scalar | 1073741855 (0x4000001F) |
| `MADirectMonitoringActive` | scalar | 1 |
| `MAEnableAggregateEngine` | scalar | 0 |
| `MAEnableStereoPairEngines` | scalar | 0 (explains why FWEngine1 is not instantiated — see headline §3) |
| `MAGlobalDim`, `MAGlobalMute` | scalar | 0, 0 |
| `MARotaryControlV2` | scalar | 281470681743360 (0xFFFF00000000) |

Panel UI state is in a separate domain `com.m-audio.appc.MAfw` (`captures/pulled/panel-appdomain.plist`): `MainWindowLastTab = 4`, four `Last1814MixerScrollPosition<N>` keys, four `Last1814OutputScrollPosition<N>` keys, `ShowSplashScreen = false`, `ShowToolTipPref = false`. No device data — pure UI state.

## 6. Numeric facts captured

From live `com_m_audio_FW1814Device` properties (`captures/12-ioreg-full.txt:3930-3957`):

| Property | Value | Decoded |
|---|---|---|
| `IOClass` | `com_m_audio_FW1814Device` | |
| `IOAudioDeviceTransportType` | 825440564 | 0x31333934 = `'1394'` (FireWire) |
| `IOAudioDeviceConfigurationApplication` | `com.m-audio.appc.MAfw` | Panel bundle ID |
| `MAMainEngineUniqueID` | `com_m_audio_FWEngine2:0xD6C040074B9FB` | engine class + device GUID |
| `MASampleRate` | 44100 | current rate |
| `MARateLocked` | 1 | locked |
| `MABytesPerCycle` | 976 | ≈ 125 µs cycle payload |
| `MAFirmwareInfo` | bCoD-prefixed blob | firmware ID (see A5) |
| `MAMIDIInputPorts` / `MAMIDIOutputPorts` | `("1")` | 1 MIDI port each direction |
| `IOAudioDeviceCanBeDefaults` | 7 | input + output + system default-capable |
| `IOUserClientClass` | `com_m_audio_FWUserClient` | matches §3 topology |

From FWEngine2 properties (`captures/12-ioreg-full.txt:3976-3992`):

| Property | Value |
|---|---|
| `IOAudioEngineDescription` | `"FireWire 1814 Multichannel"` |
| `IOAudioEngineInputSampleLatency` | 32 frames |
| `IOAudioEngineOutputSampleLatency` | 216 frames |
| `IOAudioEngineSampleOffset` | 40 frames |
| `IOAudioEngineNumSampleFramesPerBuffer` | 8192 frames |
| `IOAudioEngineFlavor` | 1 (Multichannel) |
| `IOAudioEngineClockDomain` | 589718528 |
| `IOAudioEngineState` | 0 (stopped — idle at capture time) |
| `IOAudioSampleRate` | `{Whole=44100, Fraction=0}` |

### 6a. Playback-active state — afplay owns the engine user client directly

Captured 2026-10-09 at `captures/40-ioreg-playback.txt`, with `afplay /System/Library/Sounds/Submarine.aiff` in a 10× loop with FW1814 selected as the default output.

**FWEngine2 subtree during playback** (`captures/40-ioreg-playback.txt:4145-4172`):

- `IOAudioEngineState = 1` (running — vs 0 at idle)
- `IOAudioEngineNumActiveUserClients = 1` (new property; absent at idle)
- `busy 0 (17 ms)` (up from `0 (6 ms)` at idle)
- `retain 81` (up from 78)
- Both `m_audio_b_FWStream` nodes (input id 0x100000488, output id 0x100000489): `IOAudioStreamNumClients = 1` (up from 0), retain 8 (up from 7)

**Seven `IOAudioEngineUserClient` children of FWEngine2** during playback (vs five at idle):

| id | pid | creator | `IOAudioEngineUserClientActive` | role |
|---|---|---|---|---|
| 0x1000004cd | 132 | coreaudiod | (unset) | passive subscriber |
| 0x1000004cf | 45 | loginwindow | (unset) | passive subscriber |
| 0x1000004d0 | 140 | UserEventAgent | (unset) | passive subscriber |
| 0x1000004fb | 130 | SystemUIServer | (unset) | passive subscriber |
| 0x10000056f | 172 | Panel | (unset) | passive subscriber |
| 0x1000005c9 | 462 | System Preferences | **0** (explicit) | passive subscriber (Sound prefs panel open at capture time) |
| 0x100000717 | **503** | **afplay** | **1** (explicit) | **the active streamer** — retain 7 (vs 6 for subscribers) |

(Line references: `captures/40-ioreg-playback.txt:4287, 4293, 4299, 4305, 4311, 4317, 4324`.)

**Interpretation:**

- On Snow Leopard, user-space audio clients open `IOAudioEngineUserClient` **directly** — not via a coreaudiod proxy as modern macOS does. afplay (via AudioToolbox/AudioQueue) ends up as the engine's active streamer. coreaudiod is only one of several passive subscribers (presumably for format-change notification + volume routing).
- Standard IOAudioFamily enforces at-most-one `Active = 1` streamer per engine. `IOAudioEngineNumActiveUserClients = 1` tracks this.
- **Streaming does not touch the vendor `com_m_audio_FWUserClient`** — Panel's and helper's vendor userclient retain counts are identical in idle vs playback captures. The control-plane and the stream-plane are fully decoupled at the IOKit level.
- **Streaming does not change any `MA*` property on FW1814Device** — the full MA* surface diff on `com_m_audio_FW1814Device` returns empty between idle and playback snapshots. Vendor-level device state (clock source, routing, mixer) is quiescent during I/O.

**Clone implication (Part C C5 + Part D):** our engine must accept N passive `IOAudioEngineUserClient` subscribers alongside one active streamer. Standard IOAudioEngine does this by default — no custom override needed. The clone's vendor userclient surface (per `docs/12-USERCLIENT-DESIGN.md` and A1-Q11) is strictly for the control-plane (clock source, routing matrix, mixer, direct monitor, metering (TBD)) — the HAL audio stream flows entirely through standard IOAudioFamily plumbing.

### 6c. Multichannel DAW session — Studio One 2 (pid 552)

Captured 2026-10-09 at `captures/50-ioreg-logic-playback.txt` + `captures/51-ps-logic-playback.txt`, with Studio One 2 actively playing a multichannel session (user prepared Logic initially but Studio One 2 was the DAW running). FW1814 is the active audio I/O.

**Device + engine state during Studio One playback** (`captures/50-ioreg-logic-playback.txt:4242-4258, 3930-3957`):

| Property | Idle | afplay (44.1k) | **Studio One (48k)** |
|---|---|---|---|
| `MASampleRate` (device) | 44100 | 44100 | **48000** |
| `IOAudioSampleRate.Whole` (engine) | 44100 | 44100 | **48000** |
| `IOAudioEngineState` | 0 | 1 | 1 |
| `IOAudioEngineNumActiveUserClients` | (absent) | 1 | 1 |
| `IOAudioEngineSampleOffset` | 40 | 40 | **48** |
| `IOAudioEngineOutputSampleLatency` | 216 | 216 | **231** |
| `IOAudioEngineInputSampleLatency` | 32 | 32 | 32 |
| FWEngine2 `busy` ms | 6 | 17 | **33** |
| FWEngine2 `retain` | 78 | 81 | 81 |
| `MABytesPerCycle` (device pref) | 976 | 976 | 976 |
| `MARateLocked` | 1 | 1 | 1 |

**Confirmed runtime rate change.** First captured evidence in B5 of the device switching sample rate while live (44.1k → 48k). Both `MASampleRate` on `com_m_audio_FW1814Device` and `IOAudioSampleRate` on `com_m_audio_FWEngine2` moved in lockstep. **`IOAudioEngineSampleOffset` and `IOAudioEngineOutputSampleLatency` are rate-dependent — 40/216 at 44.1k, 48/231 at 48k.** The clone must recompute these per rate on `performFormatChange` (currently stubbed in `openaudio-fw1814/kext/`). `IOAudioEngineInputSampleLatency` is rate-independent at 32 frames in both captures.

**`MABytesPerCycle` is a stale pref, not runtime truth** — stays at 976 even after a rate change. **Hypothesis:** written to the pref plist by the Panel at save-time only, not continuously recomputed by the kext. **Needs verification:** rotate the panel's sample-rate dropdown and re-read the device prop. Supersedes the earlier §6 note that treated 976 as a current runtime value.

**Seven `IOAudioEngineUserClient` children of FWEngine2 during Studio One playback** (`captures/50-ioreg-logic-playback.txt:6639-6674`):

| id | pid | creator | `IOAudioEngineUserClientActive` | role |
|---|---|---|---|---|
| 0x1000004cd | 132 | coreaudiod | 0 (explicit) | passive subscriber |
| 0x1000004cf | 45 | loginwindow | (unset) | passive subscriber |
| 0x1000004d0 | 140 | UserEventAgent | (unset) | passive subscriber |
| 0x1000004fb | 130 | SystemUIServer | (unset) | passive subscriber |
| 0x10000056f | 172 | Panel | (unset) | passive subscriber |
| 0x1000005c9 | 462 | System Preferences | 0 (explicit) | passive subscriber |
| 0x1000008d5 | **552** | **studioapp** | **1** (explicit) | **the active streamer** — retain 7 (vs 6 for subscribers) |

Same topology as afplay — one active streamer, N passive subscribers. CoreAudio enforces at-most-one `Active = 1` per engine.

**New finding — the vendor `com_m_audio_FWUserClient` set changed from 2 → 4 instances** (`captures/50-ioreg-logic-playback.txt:6677-6695`):

| id | pid | creator | retain | present at idle? |
|---|---|---|---|---|
| 0x100000542 | 64 | helper (`com.m-audio.firewire.helper`) | 7 | yes |
| 0x10000057b | 172 | Panel | **8 (up from 7)** | yes |
| 0x1000008b6 | 545 | MIDIServer | 5 | **no (new)** |
| 0x100000900 | 556 | MIDIServer | 6 | **no (new)** |

**MIDI uses the same vendor userclient class as Panel + helper.** The CoreMIDI CFPlugIn (`_extracted/midi/`) causes `coremidi.MIDIServer` to open a `com_m_audio_FWUserClient` connection into the kext — not a separate class, not a separate selector namespace at the IOKit level. Two MIDIServer pids are present in the ioreg at creator-time (545 and 556); only 556 is live in `captures/51-ps-logic-playback.txt`, so 545 is a prior MIDIServer whose userclient persisted (either MIDIServer was respawned during Studio One MIDI device scan, or the registry just hasn't torn down the stale node yet). **Clone implication (C5 userclient surface + C3 comm path):** the vendor userclient surface must serve Panel + helper + MIDI concurrently. A single `com_m_audio_FWUserClient` class covers all three — selectors must branch on type or be namespace-partitioned, not expressed as separate IOUserClient classes.

**Panel's vendor userclient retain bumped 7 → 8** during Studio One startup — Panel either subscribed to an engine-state or rate-change notification, or Studio One caused CoreAudio to query a Panel-owned port. Non-conclusive without a dtrace retain-trace.

**Studio One also subscribes to every `IOAudioControl`** (per per-channel toggle: 5 `IOAudioControlUserClient` creators — coreaudiod, SystemUIServer, Panel, System Preferences, studioapp — vs 4 in idle). This is standard IOAudioFamily plumbing — a DAW routinely binds to the full control surface for monitoring.

**No new `MA*` properties on FW1814Device.** The vendor pref/control-plane surface is unchanged by the DAW session — streaming at 48 kHz does not touch `MAClockSource`, `MAInputRoutingV2`, mixer, direct-monitor, aggregate-engine, or any other MA* key. **Reaffirms:** the control-plane and the stream-plane are fully decoupled at the IOKit level; rate changes propagate through the standard CoreAudio `performFormatChange` path, not through the vendor channel.

**Clone implications for Part C/D:**

- `performFormatChange` must recompute `SampleOffset` + `OutputSampleLatency` per rate (table landed in `project_reference_kext_hierarchy.md`'s FDF rows — but the latency numbers need their own table; current clone only publishes 48 kHz so this hasn't bit yet).
- Vendor userclient class must multiplex Panel + helper + CoreMIDI MIDIServer connections. Our current `FW1814UserClient` scaffold assumes a single creator — needs a type/connection-mode discriminator (compare `_extracted/kext/` `com_m_audio_FWUserClient::initWithTask` + `clientMemoryForType` dispatch in A1).
- `MABytesPerCycle` is **not** a runtime truth source — kext must compute cycle payload internally from current rate, FDF, SYT interval. Treat the pref key as cosmetic/Panel-UI only.

### 6b. Numeric channel map

**Channel map from the two `IOAudioStream` nodes** (grep result, lines bracketing 4000-5700):

- InputChannel 1..8 → "Input Analog 1..8"
- InputChannel 9..16 → "Input ADAT 1..8"
- OutputChannel 1..4 → "Output Analog 1..4"
- OutputChannel 5..12 → "Output ADAT 1..8"

Available PCM formats per stream: 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz, 24-bit lpcm in 32-bit container, big-endian aligned-high. Plus one `'cac3'` AC3 passthrough (IEC 61937) at 44.1 / 48 / 96 kHz, 1536 frames/packet.

## 7. StartupItem inventory on SL — reference for A4

Three M-Audio StartupItems co-resident on the SL machine (`ps -ax` output in B5 investigation; also `captures/06-launchctl-list.txt`):

1. **pid 64** — `/Library/StartupItems/M-Audio FireWire Audio Helper/M-Audio FireWire Device Helper` — label `com.m-audio.firewire.helper`. **FW1814 scope.** Opens one `com_m_audio_FWUserClient` (see §3).
2. **pid 93** — `/Library/StartupItems/M-Audio Firmware Loader/M-Audio Firmware Loader` — USB firmware uploader for Cypress FX2 USB devices (M-Audio Audiophile USB, Fast Track etc). **Not FW1814 scope.** Captured as `pulled/M-Audio_Firmware_Loader_2009` for the record.
3. **pid 97** — `/Library/StartupItems/MAudioAudiophileUSB/AudiophileCS` — Audiophile USB control-surface helper. **Not FW1814 scope.**

Confirms A4's assumption that the FW1814 helper is a single-purpose daemon per product; the FW1814 helper is not generic across the M-Audio family.

## 8. Cross-references

- **A1** (`A1-ghidra/`) — class chain to compare: `com_m_audio_FW1814Device` ✓ in live tree; `com_m_audio_FWEngine2` ✓ (also `FWEngine1` present but not instantiated — new to the system map, memory updated).
- **A2** (`A2-panel/`) — Panel bundle ID `com.m-audio.appc.MAfw` ✓ matches live pid 172. UI control → kext call mapping: Panel uses standard IOAudioControlUserClient per standard control + vendor FWUserClient for everything else.
- **A4** (`A4-helper/`) — helper runs as `com.m-audio.firewire.helper`, a **StartupItem bundle**, not a launchd daemon. A4's architectural notes stand; the StartupItem detail is a corollary.
- **A5** (`A5-firmware.md`) — `MAFirmwareInfo` ioreg blob confirmed starting with `bridgeCo` + flags + GUID low + build date "20070713080440" + production date "200403300259D9". Resolves Q19 of the A1 property-set question.
- **Proposal.md** — simplification "Panel talks to kext directly for vendor channel" is **validated** (§3). Meter-path claim is also **validated** — Panel reads directly from the vendor UC's `clientMemoryForType(1)` shared ring (§11, cap 70). Helper daemon is not on either path.

## 9. Correction propagation

- Prior session-level claim "the helper owns both FWUserClient instances" (summary draft) is **wrong**; replaced by §3 (helper owns one, Panel owns one).
- **Engine-class naming** (resolved 2026-10-09): A1 docs now reference `FWEngine1` + `FWEngine2` directly (see `A1-ghidra/Q13-engine-family.md`); the earlier "may say `FWEngine` singular" correction block is no longer applicable.

## 10. Clock-source change path — registry `setProperties` with the composite bitfield

Panel writes to `MAClockSource` go through `IORegistryEntrySetCFProperties`, not `IOConnectCallMethod`. The initial attempt (`captures/60-dtrace-clock-dropdown-attempt1.txt`) armed on `fbt::is_io_connect_method*:entry` + `fbt::is_io_connect_set_properties:entry` + `fbt::is_io_connect_async_method:entry` captured **zero** events from pid 172 — proving the Panel does not call `IOConnectCallMethod` for this surface. Cap 61 then captured the `setProperties` channel; caps 66 / 68 / 69 locked the composite layout.

### Dispatch channel

**Panel path:** Panel → `IORegistryEntrySetCFProperties(fw1814DeviceRef, cfDict)` → MIG `is_io_registry_entry_set_properties` → kernel `OSUnserializeXML` → `com_m_audio_FW1814Device::setProperties(OSDict)` → `FWAVCAudioDevice::setProperties` (fast-paths `MASampleRate`, tail-calls parent) → `FWPropertyDict::SetProperties @ 0xa0e0` (single generic dispatcher; see `A1-ghidra/Q15b-setproperties-dispatch.md`) → workloop queue → `checkForWork` → `ApplyModifiedProperties`. The vendor `com_m_audio_FWUserClient` userclients (helper / Panel / MIDIServer, see §3 and §6c) are **not** involved in clock-source changes — they only carry MIDI selectors (A1-Q11) and the Mach-port notification channel (A1-Q14).

Each click produced one `is_io_registry_entry_set_properties` call with a 77-byte OSSerialize blob containing literal XML:

```xml
<dict><key>MAClockSource</key><integer size="32">0x4000????</integer></dict>
```

### Composite bitfield (locked 2026-10-12 PM via caps 66 + 68 + 69)

`MAClockSource` is a 32-bit **composite** packing six orthogonal fields; it is NOT a scalar clock-source enum. All bits are preserved on round-trip through the kext.

| Bit(s) | Field | Encoding |
|---|---|---|
| bit 30 (`0x40000000`) | required gate | always 1; writes with gate clear return `0xE00002C2` (confirmed at `SetClockSourceInternal @ 0xe25c:218..220`) |
| bits [1:0] | **Sync Source radio** | 00 = Internal-unmuted, 01 = External-digital, 10 = Word Clock, 11 = Internal-muted |
| bit 2 (`0x4`) | S/PDIF input port | 0 = optical, 1 = coaxial |
| bit 3 (`0x8`) | Optical-IN mode | 0 = S/PDIF, 1 = ADAT |
| bit 4 (`0x10`) | Optical-OUT mode | 0 = S/PDIF, 1 = ADAT |
| bit 5 (`0x20`) | S/MUX / ADAT 4-channel | 0 = off, 1 = on |
| bits 6..29 | unused — always 0 in all captures | — |

**Capture-66 sequence** (`pulled/66-dtrace-smux-toggle.txt`, clock held at Internal-muted = bits[1:0]=11 throughout):

- `0x40000027` → `0x40000037` (opt-out S/PDIF→ADAT, +bit 4)
- `0x40000037` → `0x40000017` (S/MUX ON→OFF, −bit 5)
- `0x40000017` → `0x40000037` (S/MUX OFF→ON, +bit 5)
- `0x40000037` → `0x40000027` (opt-out ADAT→S/PDIF, −bit 4)

**Capture-68 + 69** (`pulled/68-dtrace-digital-mute.txt`, `pulled/69-dtrace-syncradios-spdifport.txt`): all four sync-source radios captured with explicit per-click labels, and the S/PDIF-input toggle captured independently. These captures together closed what cap 61 had left ambiguous — see the "Correction" block below.

**Correction (2026-10-12 PM):** this section's original cap-61 bijection table (`0x1C` = S/PDIF, `0x1D` = ADAT, `0x1E` = Word Clock, `0x1F` = Internal) was **wrong**. The cap-61 sweep was not click-labeled per click, so the mapping was inferred from assumed dropdown order; the Panel actually has four Sync Source radio buttons (not a dropdown — see manual §2.3) plus a separate Digital Port Selection quadrant, and the user clicked across both quadrants during cap 61 without annotation. The `0x1C..0x1F` low-byte range was all in **one** sync-source state (Internal-muted; bits[1:0]=11) with other bits (bit 2 S/PDIF-port, bit 3 opt-IN, bit 4 opt-OUT) flipping. Caps 68 + 69 re-captured each radio with explicit labels and settled the real bijection shown above.

### Kext-side bijection (locked 2026-10-12 PM via `A1-ghidra/Q5-clock-source.md`)

Static decompile of `com_m_audio_FW1814Device::SetClockSourceInternal @ 0xe25c` (`docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:248..276`) + `m_audio_b_FWAVCAudioDevice::SetClockSource @ 0x23858` resolves what the kext does with any accepted `MAClockSource` value. Only bits [1:0] affect the HSCI wire byte sent to the device; higher bits are stored into `fMode` and consumed as side-effects on the next engine `SetIOFormat` cycle via `SwitchDigitalSignals @ 0xe5dc`.

| Sync Source radio | bits [1:0] | HSCI wire byte 6 |
|---|---|---|
| Internal — digital inputs **unmuted** | 0 | 3 |
| External — digital | 1 | 1 |
| Word Clock | 2 | 2 |
| Internal — digital inputs **muted** | 3 | 0 |

**Correction (2026-10-14):** the "bytes 7, 8, 9 hardcoded zero" and "bits 4, 5 applied lazily" statements below are superseded. Raw `SetClockSourceInternal` `0xe3e2..0xe425` writes bytes 7/8/9 = bits 3/4/5 immediately (A1-Q5 Correction; `A2-panel/P-ClockUI-encoding.md` §4). The bit meanings captured here stand.

Wire-byte → source assignment: `{0 = Internal-muted, 1 = External-digital, 2 = Word Clock, 3 = Internal-unmuted}`. HSCI payload bytes 7, 8, 9 are **hardcoded zero** (not shifted mode bits as earlier Q5 drafts inferred). The muted/unmuted split is implemented at `SetClockSource @ 0x23858` by calling engine `vtable[0xce8]` (`SetDigitalInputActive`) with `true` for bits[1:0] ∈ {0,1,2} and `false` for bits[1:0]=3 — i.e. "Internal muted" suppresses the digital input stream rather than carrying a separate mute flag. See `A1-ghidra/Q5-clock-source.md §Final bijection` for the full Ghidra citations and the legacy BeBoB-era translator at `SetClockSource @ 0x23858`.

### Dispatcher model (locked)

There is **no single bit-mask dispatcher** on the kext side. The composite word is stored into `fMode`; the kext consumes bits as follows:

- **bits [1:0]** → `SetClockSourceInternal` HSCI wire-byte permutation (table above).
- **bits [1:0] value 3** → side-effect: `SetDigitalInputActive(false)` on the engine via `vtable[0xce8]`.
- **bits 4, 5** → side-effects on the **next** engine `SetIOFormat` cycle via `SwitchDigitalSignals @ 0xe5dc` (reads channel-count from `[+0x2e0]`, selects peak-meter dispatch: 6/8/12/else).
- **bits 2, 3** → side-effects on digital-port routing (not yet pinned to a specific Ghidra function; opt-IN mode is read when the engine resolves the input stream topology).
- **bit 30** → gate; writes with it clear are rejected.

**Clone consequence:** the clone's `setProperties` must store the full 32-bit word into `fMode` and dispatch bits [1:0] immediately through `SetClockSourceInternal`; the remaining bits are applied lazily on the next format resolve. Do NOT mask down to a 4-value clock enum — every bit above the low nibble is load-bearing.

### Payload envelope

Full first cap-61 capture, byte offsets (retained for the clone's XML parser work):

```
00: 03 00 00 00 00 00 00 00 00 00 00 00 4d 00 00 00   serialize version + flags + length (0x4d = 77)
10: 00 00 00 00 34 be 39 32 81 00 00 00 00 00 00 00   kernel pointer + sentinel
20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
30: 00 00 00 00 3c 64 69 63 74 3e 3c 6b 65 79 3e 4d   XML payload begins at offset 0x34: <dict><key>M
40: 41 43 6c 6f 63 6b 53 6f 75 72 63 65 3c 2f 6b 65   AClockSource</ke
50: 79 3e 3c 69 6e 74 65 67 65 72 20 73 69 7a 65 3d   y><integer size=
60: 22 33 32 22 3e 30 78 34 30 30 30 30 30 31 63 3c   "32">0x4000001c<
70: 2f 69 6e 74 65 67 65 72 3e 3c 2f 64 69 63 74 3e   /integer></dict>
```

### Version-skew note (2026-10-09)

Captured against v1.4 Panel + v1.9 kext. The v1.5.2 Panel in `_extracted/` presents the Sync Source UI with four buttons whose labels differ from v1.4:

| Button ID (PPob 5001, `../A2-panel/ghidra-exports/panel-rsrc.txt:14645..`) | v1.5.2 label | v1.4 label captured here |
|---|---|---|
| 0x1452 | "internal - digital inputs muted"   | "Internal - muted"   (bits[1:0]=11) |
| 0x1453 | "internal - digital inputs unmuted" | "Internal - unmuted" (bits[1:0]=00) |
| 0x1454 | "external - digital"                | "External - digital" (bits[1:0]=01) |
| 0x1455 | "word clock"                        | "Word Clock"         (bits[1:0]=10) |

The four v1.5.2 buttons map 1:1 to the four v1.4 radios — same bits [1:0] encoding on both sides. No "fifth value" exists; the earlier draft that speculated five low-byte values was based on the retracted cap-61 bijection.

### Clone implications (Part C C5 userclient surface + Part D)

1. **The vendor userclient `com_m_audio_FWUserClient` is NOT the control-plane entry point for `MAClockSource`** — the clone must accept clock-source writes via `setProperties` on the device IOService. The user-client surface in `docs/12-USERCLIENT-DESIGN.md` was overly inclusive if it listed clock-source selectors; those are **registry-property writes**, not userclient selectors. The vendor UC is MIDI (A1-Q11 selectors 500..507) + Mach-port notifications (A1-Q14) only.
2. **Every Panel-exposed scalar property probably follows the same pattern** — the Panel drives the full 46-key `MA*` surface (see §5) via `IORegistryEntrySetCFProperties` on the device. Partially confirmed: cap 62 showed `MASWReturnLevels` writes going the same way (mixer fader).
3. **Clone `setProperties` must preserve the full 32-bit composite** and route bits per the table above. Do not reduce it to a 4-value enum. The current clone doesn't publish `MAClockSource` at all.
4. **The high flag `0x40000000` must be preserved** on the way back out (ioreg still shows it after writes). Not stripped by the kext.
5. **This revises §4 (and headline finding #2):** the sync-source selector doesn't surface as `IOAudioSelectorControl` AND doesn't go through the vendor userclient — it's a plain IOKit registry property on a composite 32-bit word.

### Still unverified

- Whether rate-locked state (`MARateLocked = 1`) blocks the write server-side or whether the kext accepts and queues it. Cap-61/66/68/69 writes went through even with Studio One present (not actively streaming at click time) — the IOKit layer is unconditional; any gate must be enforced by the kext internally. **Verification path:** dtrace `MAClockSource` writes while the engine is actively streaming (user must drive this on SL — note the v1.9 panic risk from cap 65).
- Whether other scalar MA* keys (`MARotaryControlV2`, `MASampleRate`) share the `>= 0x40000000` gate. Only `MAClockSource` has been confirmed to require it.
- **Session-reset path on `MASampleRate`:** cap 65 captured one `IOAudioStreamFormat` dict before v1.9 kernel-panicked on rate change while streaming (see `project_v19_rate_change_while_streaming_panics.md`). Full `MASampleRate` write envelope + its downstream `performFormatChange` sequence is still only partially captured.

### Session 2026-10-09 PM — additional findings

Captures `pulled/62..69-*` in `captures/pulled/` (also on SL at `~/fw1814-captures/2026-10-09/`):

| File | Panel action | Finding |
|---|---|---|
| `62-dtrace-mixer-slide.txt` | mixer fader | `MASWReturnLevels` write via `setProperties` — confirms mixer-page uses the same channel as clock-source. Architectural dispatch resolved by A1-Q15b (`FWPropertyDict::SetProperties @ 0xa0e0` is the single generic absorber). |
| `63-dtrace-direct-monitor.txt` | A/B button-function selector | `MAMomentarySwitchSource = 0/1` picks DM-mode vs HP-A/B-mode (`P-manual-crosscheck.md` Item #8). |
| `64-dtrace-momentary-switch.txt` / `64b-*` | physical front-panel A/B button | **ZERO `setProperties` on any process** — the physical button bypasses the Panel entirely; DM state is kext-internal (resolves `P-manual-crosscheck.md` Item #9 as "no user-writable key"). |
| `65-dtrace-rate-change.txt` | sample-rate dropdown | 1 frame of `IOAudioStreamFormat` dict captured before **v1.9 driver kernel-panicked** (page-fault in `IOAudioFamily buffer realloc → _blkclr`) because audio was playing — see `project_v19_rate_change_while_streaming_panics.md`. Clone must gate rate-change on busy engine. |
| `66-dtrace-smux-toggle.txt` | opt-out flip + S/MUX toggle | `MAClockSource` composite bitfield first evidence (bits 4/5 flip independently of bits [1:0]). |
| `67a/b/c-ioreg-channels-*.txt` | ioreg snapshots in 3 modes | `IOAudioStreamAvailableFormats` list is **dynamic**: rebuilt on digital-mode AND active-rate change. Channel-counts match the hardware topology (8 analog + 8 ADAT @ 44.1/48, +4 @ 96 S/MUX, +2 @ 192 S/MUX×4; S/PDIF always 8+2 and non-existent at 192). |
| `68-dtrace-digital-mute.txt` | sync-source radio toggles (muted ↔ unmuted internal) + digital-port-input toggle | Sync source lives in bits [1:0] of `MAClockSource`; four radios map `{00:int-unmuted, 11:int-muted, 01/10:ext-dig/wc}`. Bit 3 = optical-IN mode (1=ADAT, 0=S/PDIF). |
| `69-dtrace-syncradios-spdifport.txt` | sync-source external-digital + word-clock + S/PDIF-port toggle | **Full sync-source bijection locked**: bits[1:0] `01` = External-digital, `10` = Word-Clock. **Bit 2 = S/PDIF input port** (0=optical, 1=coaxial). Supersedes the retracted cap-61 bijection. |
| `76-sl-halwatch-optical-switch.txt` (2026-10-17) | HAL notification log + duplex stream (`tools/halwatch.c`, `aduplex.c`) on the SL during an optical ADAT set / restore | official driver changes streams **in place** (ids 264/265 constant); burst after a real change: `ownd`, stream `pft/sfmt`, device `srnd/pft/sfmt/nsr#/slay` per scope, system `dIn/dOut/sOut`, `diff` ~4 s later |
| `77-clone-halwatch-optical-switch.txt` | same script on the clone (10.13) | burst complete (`ownd`, `slay`, `diff`, `pft/pfta/sfmt/sfm#`), ~13 s after the set before the AV/C fix |
| `78-ams-slay-reads-clone.txt`, `78a/b/c-*.png` | DTrace of AMS reads; screenshots: SL sidebar updates (10→16 in), clone sidebar stays "10 ins / 6 outs" | AMS reads 16/12 but the label does not change - open (Q-AVC-unit-and-O1 §3) |
| `79-avc-unit-before-after.txt` | `ioreg` + clone AV/C logs before/after the FWAVCUnit fix | official `m_audio_b_FWAVCUnit` is an `IOFireWireAVCUnit` subclass; clone commands timed out until rebuilt (22 ms after) |
| `80-sl-panel-path-rate-write.txt` | `MASampleRate` write on the SL | official kext keeps the old `MASampleRate` and drops `MARateLocked`; official panel keeps the chosen rate in red |
| `81-gap-registry-parity-and-helper.txt` (2026-10-17) | registry/controls/UID parity SL vs clone; helper persistence and the helper-release bug | same classes and counts; v1.9 has 19 pass-through controls that v1.11.2 lacks (both binaries checked); official UID `com_m_audio_FWEngine2:0x<GUID>`; helper file `com.m-audio.device-settings.firewire` key = GUID, 0600, user-owned |

## 11. Meter-path — Panel reads from a pre-mapped `clientMemoryForType(1)` shared ring

Captured 2026-10-13 at `captures/pulled/70-dtrace-meter-path.txt`. 30-second dtrace window during a 60s `afplay Submarine.aiff` loop on FW1814 with Panel foregrounded on its Mixer tab (meters animating). pids observed: 65 (helper daemon), 167 (Panel).

**Probes armed** (fbt, filtered to `pid==65 || pid==167`):

- `is_io_connect_method`, `is_io_connect_method_scalarI_scalarO`, `is_io_connect_method_scalarI_structureO`, `is_io_connect_method_structureI_structureO`, `is_io_connect_async_method` — covers every `IOConnectCallMethod*` MIG stub.
- `is_io_connect_map_memory`, `is_io_connect_map_memory_into_task` — covers `IOConnectMapMemory64`.
- `is_io_registry_entry_get_property`, `is_io_registry_entry_get_property_bytes` — covers registry polling as a meter path.
- `is_io_connect_set_notification_port` — would arm the helper→Panel Mach-port path.
- `is_io_service_add_notification` — would arm the IOService match-notification path.
- `mach_trap::mach_msg_trap:entry` — Mach-port send/receive baseline.

### Results

```
IOKit — (no rows aggregated: zero events on all armed IOKit probes from pid 65 or 167)
MSG 65  13
MSG 167 14998
```

- **Zero** IOKit MIG calls on either pid for the full 30-second window. No `IOConnectCallMethod`, no `IOConnectMapMemory`, no `IORegistryEntryGetProperty`, no notification-port installation on the vendor channel.
- Panel: 14998 `mach_msg_trap` calls in 30s = ~500 Hz. Matches the Carbon event-loop baseline, not a meter-telemetry rate. The Panel's mixer meters animate at ≥30 Hz, so if meter data arrived via Mach messages we would see a distinct bump; we don't.
- Helper: 13 `mach_msg_trap` calls in 30s ≈ idle. The helper is not pumping meter data to the Panel.

### Supporting IOKit evidence

Both vendor userclients present in ioreg during the capture (`captures/pulled/70-dtrace-meter-path.txt`):

```
+-o com_m_audio_FWUserClient id 0x10000054b, retain 7
    IOUserClientCreator = "pid 65, M-Audio FireWire"
+-o com_m_audio_FWUserClient id 0x10000059a, retain 8
    IOUserClientCreator = "pid 167, Panel"
```

`vmmap -interleaved 167` shows three small `IOKit SM=SHM` regions in Panel's address space plus a large `IOKit 07000000-17000000 [256.0M] rw-/rw- SM=NUL` reservation:

```
IOKit  047a8000-047ac000 [ 16K] r--/r-- SM=SHM
IOKit  047b7000-047bc000 [ 20K] r--/r-- SM=SHM
IOKit  047c9000-047ca000 [  4K] r--/r-- SM=SHM
IOKit  07000000-17000000 [256.0M] rw-/rw- SM=NUL
```

The three `SM=SHM` regions are kernel-shared memory mapped into Panel's process. One of them is almost certainly the vendor `clientMemoryForType(1)` ring documented in A1-Q11 (`FWUserClient::start + clientMemoryForType` closure) + A2-P9 (peak-meter decode shows a 30 Hz pump against this shared buffer). The 256 MB `SM=NUL` region is `IOFireWireFamily`'s shared-DMA window, unrelated to meter telemetry.

### Conclusion

**Hypothesis (c) is confirmed: Panel reads peak meters directly from the vendor `com_m_audio_FWUserClient`'s `clientMemoryForType(1)` shared ring, mapped once at Panel startup.** Hypotheses (a) and (b) are ruled out:

- **(a) standard `IOAudioLevelControl` polling** — would produce one `IORegistryEntryGetProperty` (or `IOConnectCallMethod` on `IOAudioControlUserClient`) per polled control per tick. Zero observed.
- **(b) helper → Panel Mach-port notifications** — would require either `IOConnectSetNotificationPort` traffic (zero) or elevated `mach_msg_trap` from pid 65. Helper is at 13 messages in 30s = idle.

The shared-memory region was mapped *before* the capture window opened (Panel started well before `afplay`), so the `is_io_connect_map_memory` probe correctly returned zero — the mapping is persistent for the Panel's lifetime. A future capture starting from `panel-not-running → launch Panel → begin dtrace → launch dtrace in the first 100 ms` would catch the `IOConnectMapMemory64` call itself, but this is not required to prove the meter path.

### Clone implications (Part C C3 + C5, Part D)

- The vendor `com_m_audio_FWUserClient` must implement `clientMemoryForType(1)` returning an `IOBufferMemoryDescriptor` sized for the meter ring (A2-P9's 30 Hz pump format). The kext writes meter samples into this descriptor on a 30 Hz timer; Panel reads from its mapped view with no further RPC. See A1-Q11 for the selector surface and A2-P9 for the record layout.
- The clone-topology `Proposal.md` already assumes the Panel talks to the kext directly for the vendor channel. This capture **also validates** the implied meter path — the helper is not on the meter critical path. The daemon's role stays narrow (firmware upload, device-presence tracking per A4).
- `clientMemoryForType(1)` is **per-userclient-instance**, not global. Each Panel invocation gets its own mapping; a second Panel would map its own ring. The MIDIServer userclients observed in §6c presumably use a different `type` number (or none) — pin via a future dtrace armed on `clientMemoryForType` itself.

### Version-skew note

Captured against v1.4 Panel + v1.9 kext (SL). A1-Q11 + A2-P9 both decoded against v1.11.2 — selector numbers and `clientMemoryForType` call shape are identical. The 46-key `MA*` property surface is also identical between v1.4 and v1.5.2 Panels (§2). High confidence this behavior is stable across the version skew.

## 12. DTrace-on-SL probe availability — fbt covers mach_kernel only, not kexts

Checked 2026-10-13 on `fw1814-sl`. The 10.6.8 build has fbt enabled, but **the fbt provider instruments only `mach_kernel` symbols** — no kext text is probed. Three queries confirm:

- `sudo dtrace -ln 'fbt:::entry' | awk '{print $3}' | sort -u` returns a single module: `mach_kernel`. Total fbt probes: 9419.
- `sudo dtrace -ln 'fbt:*FireWire*::entry'` returns zero probes. Same for `fbt:*AVC*::entry`, `fbt:*m_audio*::entry`, `fbt:com.apple.iokit.IOFireWireAVC::entry`.
- `kextstat` confirms `com.apple.iokit.IOFireWireFamily`, `com.apple.iokit.IOFireWireAVC`, `com.m-audio.driver.firewire` are loaded — they just aren't fbt-visible.

**Implication for B5:** any question whose answer lives inside a kext's internal functions (AVC send paths, HSCI envelope construction, DCL callbacks, isoch port lifecycle) is **not** empirically reachable from DTrace on this SL build without rebooting the machine with special boot-args to enable kext fbt instrumentation (and risking panics). The channels that DO remain reachable:

- `fbt:mach_kernel::` — any pure-kernel syscall / Mach-trap / MIG stub (used in caps 60..70).
- `syscall::`, `mach_trap::` — user→kernel entry.
- `pid$target::` — user-process instrumentation (Panel/helper, but not into kernel code).
- `io:::` — buf_strategy / journal / biodone (storage I/O, not FireWire).
- `profile-NNN` — periodic PC sampling (would catch kext hot paths statistically, but noisy).

**Policy going forward:** when a B5 question requires kext-internal visibility, resolve it from A1 static disassembly instead; do not re-attempt the dtrace. The mixer-FCP question in the resolved list above is the first example; expect similar for any detailed packet-level question.

## 13. Reclassified to A1 static follow-ups (B5-close rationale)

Two items from the earlier B5 pre-close list are **not** solvable from the SL hardware and are promoted to A1 Ghidra follow-ups against the v1.11.2 `_extracted/kext/`:

### A1-Q15c — decode `FWAVCAudioDevice::ChangeRate` + `FWEngine2::PerformFormatChange @ 0x124cc`

**Why reclassified:** the Panel-side write is a trivial `setProperties` XML `<dict><key>MASampleRate</key><integer>N</integer></dict>` (envelope identical to clock-source caps 61..70 — same channel, A1-Q15b decoded the dispatch at `FWAVCAudioDevice::setProperties @ 0x22e28`). The *interesting* content is the downstream kext work: `ChangeRate` via vtable[0xa78] and `FWEngine2::PerformFormatChange @ 0x124cc`. These are kext-internal, not reachable from fbt on SL (§12).

**Scope:**
- Ghidra decomp of `FWAVCAudioDevice::ChangeRate` (vtable[0xa78/8]) — the fast-path rate-change entry siphoned by `FWAVCAudioDevice::setProperties`.
- Ghidra decomp of `com_m_audio_FWEngine2::PerformFormatChange @ 0x000124cc` (341 B function, 3 ins/3 outs per `ghidra-exports/functions.tsv`).
- Ghidra decomp of `ActionPerformFormatChange @ 0x3e43` (17 B dispatcher) + `m_audio_b_FWBaseEngine::performFormatChange @ 0x3e54` (93 B base impl).

**Output:** the full post-write sequence — AV/C `OUTPUT PLUG SIGNAL FORMAT` writes, HSCI envelope changes, DCL program reload, latency/offset recompute. Firms up §6c's rate-dependent `SampleOffset`/`OutputSampleLatency` table.

### A1-Q15d — string-xref `MARateLocked` to its guard site(s)

**Why reclassified:** `MARateLocked` is a compile-time string literal in the v1.11.2 kext (confirmed via `strings` on the binary). Its reader(s) are statically findable in Ghidra via symbol/string xref. B5 already proved the IOKit layer is unconditional (caps 61/66/68/69 writes went through with Studio One on the engine), so if a guard exists it is kext-internal — exactly the kind of question that resolves faster from a decomp read than from a dtrace that fbt can't reach.

**Scope:** string xref `MARateLocked` in `_extracted/kext/`, then decomp each caller to determine:
- Is `MARateLocked` ever **read** by the kext? (If never read, the key is cosmetic/Panel-UI only; §5 already lists it as `MARateLocked = 1` in prefs.)
- If read, which code path checks it, and does the check gate `ChangeRate` / `PerformFormatChange` / anything else?

**Output:** one of two findings — either (a) rate-lock is a Panel-side UX hint with no kext enforcement, or (b) the kext enforces it at a specific entry point. Either resolves the pre-close question without needing a hardware capture.

## Pending work to close B5

~~Live hardware captures that still require user panel interaction on the SL machine.~~ **None remaining.** All B5 pre-close items are resolved or reclassified (§13).

Resolved (no further SL work needed):

- ~~Clock-source dispatch channel + bijection~~ — done via caps 61/66/68/69 + A1-Q5 static decode (§10 above).
- ~~Mute-indicator bit (bit 2 vs bit 3)~~ — resolved by caps 68 + 69: "mute" is not a flag at all; it's one of the four sync-source radios encoded in bits [1:0] (value 11 = Internal-muted). Bit 2 is the S/PDIF input port, bit 3 is the optical-IN mode.
- ~~Mixer-slide dispatch channel~~ — cap 62 confirmed `setProperties`; A1-Q15b resolved the kext-side absorber (`FWPropertyDict::SetProperties @ 0xa0e0`).
- ~~Direct-monitor path~~ — cap 63 (`MAMomentarySwitchSource` via `setProperties`).
- ~~Physical front-panel A/B button path~~ — cap 64 (zero `setProperties`; kext-internal state).
- ~~Playback-active ioreg + ps snapshot~~ — §6a (afplay 44.1k) + §6c (Studio One 2 multichannel 48k).
- ~~FCP wire-byte translation for `MAClockSource`~~ — resolved by static Ghidra read of `SetClockSourceInternal @ 0xe25c` (A1-Q5), no additional dtrace needed.
- ~~Meter-path trace~~ — cap 70 (`captures/pulled/70-dtrace-meter-path.txt`): zero IOKit MIG calls from Panel or helper during 30s with meters animating; Panel has three `IOKit SM=SHM` regions mapped at startup. Meter path is the vendor FWUserClient's `clientMemoryForType(1)` shared ring, consistent with A1-Q11 + A2-P9. See §11.
- ~~Mixer-slide FCP-level verification~~ — resolved by A1-Q15 §Dependency step 7 + A1-Q15b §Workloop apply: mixer writes **do** trigger AVC/FCP vendor commands. **Correction (2026-10-14):** this was inferred from A1, not captured, and the inference is wrong. Mixer group writes are async block writes to `0xFFC7:0x0070xxxx` via `CustomMixerWriteQuads` (A1-Q15f Correction), not FCP. Live check: DTrace `IOFireWire` async-write probes, not FCP, during a fader move on the SL rig. Each changed channel fires `TFunctor::Call(dev, channelIndex, newLevel)` on the workloop which calls `FWAVCAudioDevice::AVCVendorCommand(subunit, selector=0xBA9, payload)`. The B5 hypothesis "mixer stays kernel-side (no FCP)" was wrong — corrected by A1 static evidence. A live dtrace confirmation is not reachable on SL (fbt provider on 10.6 only instruments `mach_kernel`, not kexts); A1 ground truth stands. §12 below documents the probe-availability check.

Static material already captured — no further SL access needed:

- Panel v1.4 bundle pulled (see Capture inventory). Resources diff vs v1.11.2 Panel is trivial — 121 of 125 files identical; v1.9 has `English.lproj`, `build-number.txt`, `version.plist`; v1.11.2 has `ReadMe.rtfd`. Core UI graphics + nib / rsrc identical. Carbon structure comparison with A2 can proceed offline.

**B5 closure:** done 2026-10-13. Headline findings §1–§10 stand. Status flipped. Part B now advances to B1 (FFADO) → B2 (snd-bebob) → B3 (Sierra hack) → B4 (macfw) static correlation; A1 follow-ups §13 (Q15c, Q15d) run in parallel against the already-decoded `_extracted/kext/`.
