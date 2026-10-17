# BUILD-MANUAL — assembly instructions for the D2 clone

> **Removal notice (2026-10-14):** all pre-pivot code and docs (`_legacy_pre_ghidra/`, `openaudio-fw1814/firmware-helper/`, `openaudio-fw1814/legacy-mod/`) were deleted from the working tree at the user's request. They are recoverable from git: `git show d1-approved-2026-10-14:<old path>` (tag `d1-approved-2026-10-14`, commit `a9cb63c`). References to those paths in this file or in `docs/system-map/` are historical. `openaudio-fw1814/` is intentionally empty; new code is built from scratch per `docs/BUILD-PLAN.md` once the user approves it.

**Purpose:** mechanical, think-free assembly instructions for the D2 resumed-clone build. Pair with `system-map/D-decision.md` (the "why") — this file is the "how". Written so a Sonnet-medium pass can execute each milestone by (a) reading the cited system-map section, (b) producing the files listed, (c) running the acceptance test. **No design decisions remain for the build phase; every one is resolved either here or in the Pre-Build Research section below.**

**Non-negotiable before coding starts:** every item in §2 "Pre-build research" must be answered. These are the only blockers between the current state and mechanical implementation.

> **Status (2026-10-14): D2 CHOSEN by the user; this manual holds the per-milestone work orders for the clone, `docs/BUILD-PLAN.md` is the plan and §1 of that file lists corrections that OVERRIDE this manual (Rev1 parent, stream-kick order, HSCI 16 B, one mixer path, UC surface, NULL-safe teardown, two-round lifecycle, firmware path in the kext). Code stays frozen until the user approves the plan.**

## 0. Pre-build research — status snapshot (2026-10-14)

| Item | Blocks | Status | Where |
|---|---|---|---|
| Q-SDK | M-A / D | ✅ done 2026-10-14 — no IOAudio/IOFireWire slot drift; real 10.13 panic = NULL `operator delete` in teardown | `A1-ghidra/Q-SDK-reserved-slots.md` |
| Q-INFO | M-A | ✅ done 2026-10-13 (M-A below corrected 2026-10-14) | `A1-ghidra/Q-INFO-plist.md` |
| Q-DCL + Q-POOL | M-B | ✅ done 2026-10-13 | `A1-ghidra/Q-DCL-geometry-table.md` |
| Q3a ExtendedPlugInfo | M-C | ✅ done — kext never calls it; hardcode plug map | `A1-ghidra/Q3a-fcp-avc-wire.md` §4 |
| Q-CMP-CONNECT | M-C | 🟡 partial — `ConnectAudioMIDIChannels @ 0x12156` decoded; per-rate BW/channel/speed not yet tabulated | Q3a §3 |
| Q-FCP-OUTPUT / INPUT | M-C | ✅ resolved 2026-10-14 — FW1814 is Rev1: plain 8-byte CONTROL `00 FF 19 00 90 SFC FF FF` (INPUT) then `00 FF 18 00 90 SFC FF FF` (OUTPUT), interleaved with TX/RX DCL start, 500 ms apart. The `0x2F` form is Rev2-only | Q3a §3 Correction |
| Q-SYT | M-D | ✅ done | `A1-ghidra/Q15e-syt-interval-format-change.md` §1 |
| Q15e commit | M-D | ✅ SetupCIP cases 3–6 resolved 2026-10-14 (jump table `0x272d8`: 1/3/5 and 2/4/6 share constants) | Q15e §3 |
| Q-RATE-LATENCY | M-D | ❌ — SL rig session (rig reachable via `ssh fw1814-sl`) | — |
| Q15f wire paths | M-E | ✅ resolved 2026-10-14 — `0xBA9`/`0xBE9` are virtual member pointers. **One** mixer path: group batch → `CustomMixerWriteQuads` → async block write `0xFFC7:0x00700000 + offset` (offsets 0x00/0x08/0x10/0x34/0x38 = FFADO map). `0xBE9` = `AVCControlSetLEDStatus` (A/B LEDs). Clock = HSCI. Open: rotated-aux count/order | `A1-ghidra/Q15f-mixer-control-plane-wire.md` §5 |
| Q-HSCI-9 | M-E | ✅ resolved 2026-10-14 — bytes 7/8/9 = `MAClockSource` bits 3/4/5 (opt-IN ADAT, opt-OUT ADAT, S/MUX); not `clk_lock` | A1-Q5 Correction 2026-10-14 (raw `0xe3e2..0xe425`) |
| Q-OPCODE | M-E | ✅ moot 2026-10-14 — no AV/C `0xBA9` command exists (Q15f Correction) | — |
| Q3b | M-E | 🟡 partial 2026-10-14 — factory-reset defaults decoded (`ResetToFactorySettings @ 0xd190`, MAClockSource `0x40000003`); key names + register encoding open | `A1-ghidra/Q3b-mixer-power-on-defaults.md` |
| Q-MAMAP | M-E | ❌ — needs Settings `Init` key registration + `AdjustedLevels` encoding | — |
| Q-UC-SELECTORS, Q-UC-RING, Q-KIPC-LIVE, Q-NOTIFY | M-F | ✅ done 2026-10-14 — legacy IOExternalMethod table; FWUserClient derives from FWKIPCUserClient (live); meter page `uint32 peak[52]`; notify type 0 (props, 44 B + dict) / type 1 (MIDI-in) | `A1-ghidra/Q-UC-surface.md` |
| Q-CLOCK-UI | M-G | ✅ done 2026-10-14 (v1.5.2 sync-radio bit values unverified) | `A2-panel/P-ClockUI-encoding.md` |
| Q-PANEL-LAYOUT | M-G | ❌ **user sign-off** | — |
| Q-MIDI-WIRE | M-H | ✅ checked 2026-10-14 — A3 consistent with kext; 503 + type-1 port exist but plugin uses the 0x400/0x800 rings | `A3-midi/README.md` §Q-MIDI-WIRE |
| Q-LOOPBACK | M-L | ❌ — SL rig reference measurement | — |
| Q-SIGN | final build | ❌ **user** | — |
| Q-MB (TX/RX callbacks, CIP/SYT cadence, index table) | M-B | ✅ done 2026-10-15 (R41); **corrects Q-DCL/C4: SetTagSyncBits on the first primer of each bank; pool offset by one; takeTimeStamp from the bank-3 callback** | `A1-ghidra/Q-MB-dcl-callbacks.md` |
| Q-START (who starts the channels after `Connect`; `ResetPort` caller) | M-C | ❌ **new, 2026-10-15** — `ConnectAudioMIDIChannels` calls `vt[0x120]` = `Connect`, not `Start`; decompile `FWEngine2::StartAudio` slots `0xd08/0xd10` and program `vt[0x128]` before writing M-C | `Q-MB` §7 item 5 |
| Q-MIDI-TX/RX decode | M-H | 🟡 decoded in `ghidra-exports/decomp/q-mb-dcl-callbacks.c` (`ScheduleMidiPacket`, `ProcessCompoundMIDIOutput`, `ProcessCompoundMIDIInputForPacket`), not yet written up | A3 M5/M6 |

**Environment (updated 2026-10-15):**
- Build PC `fw1814-build` (`192.168.100.20`): macOS 10.13.6 17G14042, Xcode 9.4.1, `MacOSX10.13.sdk`. **Installed by us:** `/Library/Extensions/OpenAudioFW1814.kext` (the M-FW build, 0.1.0), `/Library/PrivilegedHelperTools/com.openaudio.fw1814.helper` + LaunchDaemon `com.openaudio.fw1814.helper`; the pre-pivot daemon `com.openaudio.fw1814.fwhelper` was removed. No reference/M-Audio kext. Work area `~/oa-build` (rsynced from the dev Mac by `openaudio-fw1814/scripts/sync-to-buildpc.sh`; never edit there). Bare git remote `~/git/openaudio-fw1814.git` (local remote name `buildpc`).
- SL rig `fw1814-sl` (`192.168.100.17`): 10.6.8.
- Dev Mac: Ghidra 12.1.4 + JDK 21 in `~/tools`, native decompiler built.
- Evidence formerly in `/tmp` now lives in the repo or in `_refs/` (see `_refs/README.md`).

---

## 1. Repo structure — final tree after D2 ships

```
firewireDriver/
├── CLAUDE.md                       [existing, no change]
├── NEW_PLAN.md                     [existing, no change]
├── NEXT_STEPS.md                   [existing, no change]
├── README.md
├── docs/
│   ├── STATE.md                    [existing, keep in sync per milestone]
│   ├── BUILD-MANUAL.md             [this file]
│   ├── Proposal.md                 [existing, read-only reference]
│   ├── manual.pdf                  [existing hardware manual]
│   ├── manual-reference.md         [existing]
│   └── system-map/                 [existing, read-only reference during build]
│       ├── A1-ghidra/              [kext decode, Q1–Q17]
│       ├── A2-panel/               [panel decode, P1–P10]
│       ├── A3-midi/                [MIDI decode, M1–M10]
│       ├── A4-helper/              [helper decode, H1–H10]
│       ├── A5-firmware.md
│       ├── A6-prefpane.md
│       ├── B1-ffado-correlate.md
│       ├── B2-snd-bebob-correlate.md
│       ├── B2a-rust-bebob-correlate.md
│       ├── B3-sierra-hack-delta.md
│       ├── B4-macfw-correlate.md
│       ├── B5-sl-dtrace-truth/
│       ├── C1-lifecycle-state-machine.md
│       ├── C2-sync-path.md
│       ├── C3-comm-path.md
│       ├── C4-transport-control-path.md
│       ├── C5-userclient-surface.md
│       ├── C6-latency-budget.md
│       └── D-decision.md
│
├── _extracted/                     [existing, read-only ground truth]
├── _legacy_pre_ghidra/             [existing, archived — do NOT read for implementation]
├── _vendor/M-AUDIO FWAUDIOPHILE KEXT-LOAD SIERRA/   [existing, reference]
├── _vendor/M-Audio_FireWire_1-11.2.dmg               [existing]
│
└── openaudio-fw1814/
    │
    ├── core/                       [NEW — C++17 transport-agnostic protocol library]
    │   ├── CMakeLists.txt
    │   ├── include/openaudio/
    │   │   ├── Avc.h               [AV/C envelope encode/decode]
    │   │   ├── Hsci.h              [12-byte HSCI envelope: company 0x040004]
    │   │   ├── Fdf.h               [7-rate FDF table + SYT-interval table]
    │   │   ├── Cip.h               [CIP header templates per rate]
    │   │   ├── Am824.h             [AM824 silence constants: label=0xCF data=0x80]
    │   │   ├── PropertyDict.h      [typed-property + ChannelGroup model]
    │   │   ├── MixerRegMap.h       [40-quadlet 0xFFC7:0x00700000+ semantic map]
    │   │   ├── MaClockSource.h     [composite 32-bit bitfield codec]
    │   │   ├── BcdParser.h         [.bcd firmware image parser]
    │   │   └── Model.h             [FW1814 constants: OUI, Model_IDs, plug map]
    │   └── src/                    [matching .cpp]
    │
    ├── kext/                       [NEW — kernel driver, 10.13 SDK, Xcode 9.4.1]
    │   ├── OpenAudioFW1814.xcodeproj/
    │   ├── Info.plist              [IOKit matching dict; see §3.A]
    │   ├── Entitlements.plist
    │   └── src/
    │       ├── FWMetaNub.{h,cpp}              [polymorphic nub; plist matches IOFireWireDevice, bootloader 0x10070 vs runtime 0x10071 decided in probe()/start() — Q-INFO]
    │       ├── FWRev1AudioDevice.{h,cpp}      [device class; parent of FW1814Device — Rev1 per Q16 Correction 2026-10-14]
    │       ├── FW1814Device.{h,cpp}           [top-level device, hosts FWPropertyDict]
    │       ├── FWBaseEngine.{h,cpp}           [common engine base]
    │       ├── FWEngine1.{h,cpp}              [stereo-pair engine]
    │       ├── FWEngine2.{h,cpp}              [multichannel engine, flavor=1]
    │       ├── FWPairEngine.{h,cpp}           [paired-engine variant]
    │       ├── FWTimeSlaveEngine.{h,cpp}      [time-slave engine]
    │       ├── FW1814UserClient.{h,cpp}       [vendor UC — 8 selectors + shared ring]
    │       ├── FWKIPCUserClient.{h,cpp}       [base class of FW1814UserClient — live (Q-UC-surface §1)]
    │       ├── FW1814DCLBuilder.{h,cpp}       [20-seg (4-bank) TX + 64-seg × 4-pkt RX (Q-DCL) classic DCL]
    │       ├── TPacketPool.{h,cpp}            [AM824 packet backing store with 0xCF/0x80 pre-fill]
    │       ├── MakeAppleFWAudioGoAway.{h,cpp} [match "FW 1814" on IOFireWireAVCSubUnit, probe score 0x7FFFFF — Q-INFO, Q8]
    │       └── Trace.h                        [os_log + os_signpost macros]
    │
    ├── control-panel/              [NEW — AppKit + Swift modern UI]
    │   ├── OpenAudioFW1814Panel.xcodeproj/
    │   ├── Info.plist
    │   ├── Sources/
    │   │   ├── AppDelegate.swift
    │   │   ├── DeviceList/                    [multi-interface picker]
    │   │   ├── MixerView/                     [22×4 main + 22×2 aux routing matrix]
    │   │   ├── ChannelStripView/              [fader, mute, solo, pan, aux send]
    │   │   ├── HeadphoneView/                 [3-valued HP source + level]
    │   │   ├── HardwareTabView/
    │   │   │   ├── ClockSourceSelector.swift  [composite MAClockSource dropdown]
    │   │   │   ├── SpdifPortSelector.swift
    │   │   │   ├── OpticalModeSelector.swift  [S/PDIF ↔ ADAT S/MUX]
    │   │   │   ├── Ac3PassthroughToggle.swift
    │   │   │   ├── StereoPairEnginesToggle.swift
    │   │   │   ├── BufferSizePicker.swift     [64..4096, default 512]
    │   │   │   └── SampleRatePicker.swift     [44.1/48/88.2/96/176.4/192 kHz]
    │   │   ├── MetersView/                    [shared-ring reader, cap 70]
    │   │   ├── Presets/                       [saved presets — modern feature]
    │   │   ├── IOKitBridge/
    │   │   │   ├── DeviceRegistry.swift       [IORegistryEntrySetCFProperties channel]
    │   │   │   ├── UserClient.swift           [8-selector dispatch]
    │   │   │   ├── MeterRing.swift            [IOConnectMapMemory]
    │   │   │   └── Notification.swift         [44-byte Mach notification]
    │   │   └── Model/
    │   │       └── MaKeys.swift               [46 MA* key catalog, typed]
    │   └── Resources/
    │       ├── Assets.xcassets
    │       └── MainMenu.xib
    │
    ├── midi-plugin/                [NEW — CoreMIDI CFPlugIn]
    │   ├── OpenAudioFW1814MIDI.xcodeproj/
    │   ├── Info.plist
    │   └── Sources/
    │       ├── Factory.cpp                    [CFPlugIn factory boilerplate]
    │       ├── Driver.cpp                     [CoreMIDI entry points]
    │       ├── Device.cpp                     [MIDI endpoint model]
    │       └── Wire.cpp                       [MIDI wire packet layout from A3]
    │
    ├── firmware-helper/            [existing, A4-verified — NO CHANGE in build phase]
    │   └── ...
    │
    ├── scripts/                    [NEW]
    │   ├── install.sh              [LaunchDaemon-only installer]
    │   ├── uninstall.sh
    │   ├── stage-kext.sh           [stage to /tmp, kextutil-test, THEN /Library/Extensions]
    │   └── sign.sh                 [codesign with user's Developer ID]
    │
    ├── tools/                      [NEW — dev utilities]
    │   ├── bcd-dump                [parse .bcd files; mirrors A5]
    │   ├── fcp-probe               [one-shot AV/C send-and-print for bringup]
    │   └── meter-dump              [attach to kext shared ring, print 70-channel peak]
    │
    └── tests/                      [NEW]
        ├── core-tests/             [GoogleTest or Catch2 for `core/`]
        ├── kext-userclient-tests/  [userspace client that exercises 8 selectors]
        └── hw-smoke/
            ├── loopback.sh         [M-L round-trip measurement]
            └── ratesweep.sh        [M-D measure SampleOffset/Latency at each rate]
```

**Package IDs / class names (pinned now, mechanical):**

| Thing | Value |
|---|---|
| Kext bundle ID | `com.openaudio.OpenAudioFW1814` |
| Kext class prefix | `com_openaudio_` (e.g. `com_openaudio_FW1814Device`) |
| Vendor UC class | `com_openaudio_FW1814UserClient` |
| Notification UC base class (required — Q-UC-surface §1) | `com_openaudio_FWKIPCUserClient` |
| Panel bundle ID | `com.openaudio.FW1814Panel` |
| MIDI plugin bundle ID | `com.openaudio.FW1814MIDI` |
| Helper bundle ID | `com.openaudio.fwhelper` (existing) |
| Deployment target | macOS 10.13 (`-mmacosx-version-min=10.13`) |
| C++ standard | C++17 (both kext — `-std=gnu++14` — and `core/` — `-std=gnu++17`; the kext can't go above gnu++14 on 10.13 SDK, so `core/` headers must be C++14-safe when consumed by kext) — **unverified: kext C++ dialect ceiling on Xcode 9.4.1 not yet tested; verify with a hello-kext build on the build PC** |
| Swift | Swift 4.1 (Xcode 9.4.1) |
| Team ID | user's own Developer ID (fill in `scripts/sign.sh`) |

---

## 2. Pre-build research — the answers yet lacking

Every item here must land as a doc update under `docs/system-map/` **before** its blocking milestone starts. Each item names the exact capture (Ghidra function, SDK header grep, hardware ioreg key, or Panel-binary decompile) that resolves it. These are mechanical; none require design judgement.

### 2.A — Blocks M-A (kext skeleton + probe)

**Q-SDK — 10.13 SDK `_RESERVED*` slot inventory.** **Done 2026-10-14 — `docs/system-map/A1-ghidra/Q-SDK-reserved-slots.md`.** Result: no IOAudio/IOFireWire drift; only `IOService` LP64 slots 0–1 consumed; real 10.13 panic is NULL `operator delete` in teardown (§5 there). Original task text: Grep `/Applications/Xcode-9.4.1.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.13.sdk/System/Library/Frameworks/{Kernel.framework,IOKit.framework}/Headers/` for `_RESERVED` in `IOAudioEngine.h`, `IOAudioStream.h`, `IOAudioControl.h`, `IOFireWireLib.h`, `IOFWIsochPort.h`, `IOFWIsochChannel.h`. Produce a table of (header, class, reserved slot indexes present). Deliverable: `docs/system-map/A1-ghidra/Q-SDK-reserved-slots.md`.

**Q-INFO — Info.plist IOKit personalities.** **DONE 2026-10-13 — see `docs/system-map/A1-ghidra/Q-INFO-plist.md`.** Summary: 4 personalities — `FWMetaNub` matches on `IOProviderClass = IOFireWireDevice` (NOT `IOFireWireUnit`), no `Model_ID` plist filter (bootloader/runtime discrimination inside `probe()`), no `IOProbeScore` key (reference uses runtime `probe()` score only); `com_openaudio_FW1814Device` matches on `com_openaudio_FWMetaNub` provider + `FireWire Product Name = "FW 1814"`; `FW1814AppleGoAway1` + `FW1814AppleGoAway12` extend reference's FW-Solo-only probe-steal to FW1814. Three BUILD-MANUAL §2.A originals (IOFireWireUnit / two Model_ID entries / IOProbeScore+1) were speculative and wrong — Q-INFO corrects them.

### 2.B — Blocks M-B (DCL rings + continuous silence TX)

**Q-DCL — Exact per-segment packet count + per-rate `cipDataQ` / `cipPadQ`.** A1-Q1 pins "20-seg (4-bank) TX + 64-seg × 4-pkt RX (Q-DCL)" and "4 banks × (4 primers + 1 big-cb + jump)" for TX. We need:
- Packet count per segment at each rate (should be derivable from `samplesPerPkt` and FDF; verify from `kext @ 0x???? FW1814DCLBuilder::BuildTxProgram` — grep `docs/system-map/A1-ghidra/otool-exports/mafw.asm` for `BuildTx` / `BuildRx`).
- `cipDataQ` = AM824 data quadlets per packet per rate. For 48 kHz × 8 channels, `cipDataQ = 8` (one quadlet per channel per sample at SYT-interval 8). Verify across all 7 rates.
- `cipPadQ` = AM824 pad quadlets per packet per rate.
Deliverable: `docs/system-map/A1-ghidra/Q-DCL-geometry-table.md` — a single table with columns `(rate, SYT-interval, samples/pkt, cipDataQ, cipPadQ, bytes/pkt, pkts/segment, segments, total pool pages)`.

**Q-POOL — `TPacketPool` page allocation per rate.** **Done 2026-10-13 — `Q-DCL-geometry-table.md` §Pool page allocation.** A1-Q17 says `inTaskWithPhysicalMask(kernel_task, N_pages*4096, physMask & ~0xFFF)` but `N_pages` is parameterized. Decompile the caller — grep for `TPacketPool::Initialize` xrefs in `docs/system-map/A1-ghidra/otool-exports/mafw.asm` and extract the `N_pages` constant passed per rate (likely a table). Deliverable: append to `Q-DCL-geometry-table.md`.

### 2.C — Blocks M-C (CMP connect + first streaming)

**Q-FCP-OUTPUT — Exact bytes of the OUTPUT 0x18 AV/C command used in the stream-kick.** Memory `project_fw1814_stream_kick.md` names the sequence; `_refs/snd-firewire-ctl-services/protocols/bebob/src/maudio/special.rs` has `special_set_rate` which B2 cross-ref matched byte-for-byte. Extract the exact 10-byte envelope from `special.rs:????` and record in `docs/system-map/A1-ghidra/Q-FCP-stream-kick.md`.

**Q-FCP-INPUT — Exact bytes of the INPUT 0x19 AV/C command.** Same source; same deliverable.

**Q-CMP-CONNECT — CMP connect parameters.** From A1-Q3: 2 oPCRs + 3 iPCRs, plug 0 = PCM. We need per-rate: bandwidth units (BW), isoch channel number, speed code (S100/S200/S400). Grep `docs/system-map/A1-ghidra/otool-exports/mafw.asm` for `ConnectAllPlugs` + `AllocateChannel`. Deliverable: `docs/system-map/A1-ghidra/Q-CMP-params.md`.

**Q3a — ExtendedPlugInfo probe.** **Done — `Q3a-fcp-avc-wire.md` §4: not called by the kext; hardcode the plug map.** B1 noted FFADO explicitly skips `ExtendedPlugInfo` on Model_ID `0x10071`. Does the kext probe it? Grep `docs/system-map/A1-ghidra/otool-exports/mafw.asm` for `0xC2 0x18` AV/C EXTENDED PLUG INFO opcode. If kext skips it, we can hardcode plug map from memory `project_fw1814_plugs.md` (preferred). Deliverable: short answer in `docs/system-map/A1-ghidra/Q3a-extended-plug-info.md`.

### 2.D — Blocks M-D (all 7 rates + format change)

**Q15e — `FW1814Device.vt[0xce0]` AV/C commit byte layout.** **Partial — choreography in `Q15e-syt-interval-format-change.md` §4 + wire bytes in `Q3a-fcp-avc-wire.md` §2; SetupCIP cases 3–6 unverified.** Decompile `docs/system-map/A1-ghidra/otool-exports/mafw.asm` at the function the vtable slot points to; follow `SetIOFormat @ 0x42aa` → `vt[0xce0]`. Extract the full AV/C command bytes. Deliverable: `docs/system-map/A1-ghidra/Q15e-vt-0xce0-commit-bytes.md`.

**Q-SYT — SYT-interval per rate.** **Done 2026-10-13** — `A1-ghidra/Q15e-syt-interval-format-change.md` §1 (`SetPacketParameters` caller `0x272f3`):

| Rate (Hz) | Rate key (Hz) | FDFValue | SYT-interval | CIP_INCREMENT |
|---|---|---|---|---|
| 32000  | 0x7D00  | 0 | 8  | 6144 |
| 44100  | 0xAC44  | 1 | 8  | 4458 |
| 48000  | 0xBB80  | 2 | 8  | 4096 |
| 88200  | 0x15888 | 3 | 16 | 4458 |
| 96000  | 0x17700 | 4 | 16 | 4096 |
| 176400 | 0x2B110 | 5 | 32 | 4458 |
| 192000 | 0x2EE00 | 6 | 32 | 4096 |

SetupCIP internal fields for FDFValue 3–6 remain **unverified** (Q15e §3). Deliverable: `core/include/openaudio/Fdf.h` + append to C2 §5 if the table isn't already definitive.

**Q-RATE-LATENCY — Per-rate `IOAudioEngineSampleOffset` + `IOAudioEngineOutputSampleLatency` for 88.2/96/176.4/192 kHz.** B5 §4 has 44.1k and 48k only. Capture on SL rig: boot v1.9 panel at each of the 4 high rates, `ioreg -c com_m_audio_FWEngine2 -l | grep -E 'SampleOffset|SampleLatency'`. Deliverable: append rows to C6 §1 table.

### 2.E — Blocks M-E (mixer control plane)

**Q15f — Second mixer write path resolution.** **Resolved 2026-10-14 — `Q15f-mixer-control-plane-wire.md` Correction:** one mixer path (group batch → `CustomMixerWriteQuads` direct block write); `0xBE9` = A/B LED call; HSCI for clock only. B1 §3 + B4 §1 agree `CustomMixerWriteQuads @ 0x22c7a` writes directly to `0xFFC7:0x00700000+` in parallel with the AV/C `0xBA9` dispatcher. Decompile xrefs to `CustomMixerWriteQuads` in `docs/system-map/A1-ghidra/otool-exports/mafw.asm` and map each caller to the MA* key it handles. Produce a table `(MA* key → wire path)`. Deliverable: `docs/system-map/A1-ghidra/Q15f-mixer-wire-paths.md`.

**Q-OPCODE — Vendor opcode `0xBA9` on-wire payload format.** **Moot (2026-10-14):** `0xBA9` is a C++ virtual member pointer to `CustomMixerWriteQuads`, not an AV/C opcode; the wire payload is the raw quadlet block written to `0xFFC7:0x00700000 + offset` (two 16-bit channel values per quadlet, upper = left; FFADO `special_avdevice.h:50–54`). Original task text: Decompile `FWPropertyDict::SetProperties @ 0xa0e0` and the `TFunctor` callbacks on `FW1814Device +0x38` that build the batch. Extract: AV/C header bytes, batch-record format `(key_id | old_value | new_value)*`, length encoding. Deliverable: `docs/system-map/A1-ghidra/Q-opcode-0xBA9-payload.md`.

**Q3b — Mixer power-on defaults.** **Partial 2026-10-14 — `A1-ghidra/Q3b-mixer-power-on-defaults.md`** (defaults come from `ResetToFactorySettings`, not from writes in `initHardware`). Decompile `initHardware` (function entry — `0x3644` is a mid-function instruction, the `MARateLocked` seed; locate the entry symbol in `A1-ghidra/otool-exports/mafw.asm`) and extract every write that targets the 40-quadlet register window `0xFFC7:0x00700000+` before any `setProperties` fires. Produce the initial-state register snapshot. Deliverable: `docs/system-map/A1-ghidra/Q3b-mixer-power-on-defaults.md`.

**Q-MAMAP — The 46 MA* keys → wire bytes crosswalk.** This is the biggest build-blocker and the one piece the system-map does **not** yet produce in one place. For each of the 46 keys in A2-P3, record:

| Column | Example |
|---|---|
| Key name | `MAMainLevelCh01` |
| CF type | `NSNumber` |
| Value encoding | `int16_t clamped 0..32767, log-mapped from dB` |
| Wire path | direct register block write `0xFFC7:0x00700000 + group offset` (Q15f Correction 2026-10-14) OR HSCI (clock only) |
| Default value (post-boot) | 0x7FFE (unity, from Q3b) |
| Byte layout | 2-byte big-endian in quadlet LSW |
| UI binding | `ChannelStripView.channel01.fader` |

Deliverable: `docs/system-map/A2-panel/P-MAMAP-crosswalk.md` — a single table with all 46 rows, sourced from A2-P3 + B1 register map + Q15f + Q3b. **This table is the panel's spec.**

**Q-HSCI-9 — HSCI byte 9 `clk_lock` latch semantics.** **Resolved 2026-10-14:** bytes 7/8/9 = `MAClockSource` bits 3/4/5 (optical-IN ADAT, optical-OUT ADAT, S/MUX), raw `SetClockSourceInternal` `0xe3e2..0xe425`; see A1-Q5 Correction 2026-10-14. Deliverable folded into Q5; no separate file needed. B2 flagged byte 9 writes in `bebob_maudio.c:196`; A1-Q6 didn't surface it. Decompile `SetClockSourceInternal @ 0xe25c` and record the full 12-byte envelope including byte 9 behavior across every clock source. Deliverable: `docs/system-map/A1-ghidra/Q-HSCI-byte9.md`.

### 2.F — Blocks M-F (user-client surface)

**Q-UC-SELECTORS — 8 selectors 500–507 full arg layouts.** **Done 2026-10-14 — `A1-ghidra/Q-UC-surface.md`.** A1-Q11 names them; need full input/output arg table (type, size, direction) for each. Grep `docs/system-map/A1-ghidra/otool-exports/mafw.asm` for `com_m_audio_FWUserClient::externalMethod` dispatch and extract the `IOExternalMethodDispatch` table. Deliverable: `docs/system-map/A1-ghidra/Q-UC-selector-args.md`.

**Q-UC-RING — Shared-ring meter struct layout (kernel writer side + Panel reader side).** **Done 2026-10-14 — `A1-ghidra/Q-UC-surface.md`.** C5 §4 marks this as hypothesis. Two decodes needed:
- Kernel side: find `clientMemoryForType(1)` in `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, trace the `IOBufferMemoryDescriptor` size + layout. Trace which function writes to the ring and at what cadence.
- Panel side: open `_extracted/panel/` binary in Ghidra, find `IOConnectMapMemory(1)` calls, trace reader loop.

~~Produce: `struct MeterRingHeader { uint32_t writerIdx; uint32_t generation; ... }` + `struct MeterSample { uint8_t level[70]; ... }`.~~ **Answer:** no header and no ring; a 4 KiB `kIOMemoryKernelUserShared` page holding `uint32_t peak[52]`, overwritten in place by `ReceiveControlPacket` ("70" came from B5 capture-file number 70). Deliverable: `docs/system-map/A1-ghidra/Q-UC-shared-ring.md`.

**Q-KIPC-LIVE — Is `FWKIPCUserClient` live code in v1.11.2?** **Done 2026-10-14 — `A1-ghidra/Q-UC-surface.md`.** Grep `docs/system-map/A1-ghidra/otool-exports/mafw.asm` for `newUserClient` dispatch and `__ZN22com_m_audio_FWKIPCUserClient*` xrefs. If no live caller references it, treat as dead and omit from the clone (saves one class). Deliverable: `docs/system-map/A1-ghidra/Q-KIPC-live.md` — single yes/no with xref count.

**Q-NOTIFY — 44-byte Mach notification message layout.** **Done 2026-10-14 — `Q-UC-surface.md` §4** (the "3 body types" premise was wrong: type 0 = props/IOKit messages, type-1 port = MIDI-in). Original text: A1-Q14 names 3 body types (0=props, 1=peak, 2=rate-lock). Extract exact byte layout for each body type (field offsets, endianness). Deliverable: `docs/system-map/A1-ghidra/Q-Notify-44byte-layout.md`.

### 2.G — Blocks M-G (panel baseline)

**Q-CLOCK-UI — MAClockSource composite bitfield → UI widget mapping.** **Done 2026-10-14 — `A2-panel/P-ClockUI-encoding.md`.** Note: bit 2 is 0 = optical, 1 = coaxial (B5 §10); the list below has it reversed. Memory `project_maclocksource_is_composite_bitfield.md` names bits:
- bits[1:0] = sync source
- bit 2 = S/PDIF port (coax=0 / optical=1)
- bit 3 = optical IN mode (S/PDIF=0 / ADAT=1)
- bit 4 = optical OUT mode (S/PDIF=0 / ADAT=1)
- bit 5 = S/MUX enable
- bit 30 = gate

Produce the UI → bit encoding table for every combination the Panel exposes (dropdowns + checkboxes). Deliverable: `docs/system-map/A2-panel/P-ClockUI-encoding.md`.

**Q-PANEL-LAYOUT — Modern UI wireframe sign-off.** The user approved modern UI; we need one sketch to lock scope before SwiftUI/AppKit coding starts. Produce `docs/BUILD-MANUAL-UI-WIREFRAME.md` with:
- Window chrome (title bar, device picker, menu bar).
- Main view tabs (Mixer, Hardware, Channels, Meters, Presets, Info).
- Mixer tab layout (22×4 main matrix + 22×2 aux + master section + headphone section).
- Hardware tab layout (clock source + S/PDIF port + optical mode + S/MUX + AC3 + buffer size + sample rate + stereo-pair toggle).
- Channels tab (per-channel strip: fader + mute + solo + pan + aux send + source select).
Deliverable: user-reviewed wireframe doc. **User sign-off required before M-G starts.**

### 2.H — Blocks M-H (MIDI)

**Q-MIDI-WIRE — MIDI wire packet layout confirmation.** **Done 2026-10-14 — `A3-midi/README.md` §Q-MIDI-WIRE.** A3-M docs should have this; verify completeness. Check: Mach-port callback cadence, bytes-per-packet in AM824, SysEx handling, running status. Deliverable: short confirmation note; if gaps found, extract from `_extracted/midi/`.

### 2.I — Blocks M-L (latency validation)

**Q-LOOPBACK — Loopback round-trip measurement harness.** Build `tests/hw-smoke/loopback.sh`: emit tone from Main-L via CoreAudio, capture on Analog-In-1, measure phase delay, convert to frames at the active rate. Compare to C6 §9 pass bar `≤ reference`. Deliverable: the script + a reference measurement from SL rig for each rate.

### 2.J — Signing / notarization

**Q-SIGN — User's Developer ID + Team ID.** User supplies: `Developer ID Application: NAME (TEAMID)`. Fill into `scripts/sign.sh` + Xcode project signing settings. **User action required.** On 10.13 pre-notarization, Developer ID signing is sufficient — no notarization step needed.

---

## 3. Per-milestone work orders

Each milestone has: **input docs** (what to read), **output files** (what to create), **pseudocode skeleton** (the shape of the code), **acceptance test** (how to know it's done).

Everything else — architecture, class layout, byte offsets, buffer sizes — is already decided in the system-map. If something is missing from a milestone, it's listed in §2 and must be answered before that milestone starts.

### M-A — Kext skeleton + probe

**Blocked on:** §2.A (Q-SDK, Q-INFO).

**Input docs:** `C1-lifecycle-state-machine.md` §2 (state definitions), `A1-ghidra/Q7` (bootloader/runtime state machine), `A1-ghidra/Q8` (`MakeAppleFWAudioGoAway`), `D-decision.md` §7.1.

**Output files:**
- `kext/OpenAudioFW1814.xcodeproj`
- `kext/Info.plist` (per Q-INFO)
- `kext/src/FWMetaNub.{h,cpp}` — personality `IOProviderClass = IOFireWireDevice`, no plist `Model_ID` filter, no `IOProbeScore` (Q-INFO). Overrides `probe` / `start` / `stop` / `message` / `matchPropertyTable`. State machine from Q7 (6 states). Superclass per Q7 (**check Q7 before writing — Q-INFO notes the class-inheritance vs matching distinction**).
- `kext/src/FWRev1AudioDevice.{h,cpp}` — the FW1814's parent class is `FWRev1AudioDevice` → `m_audio_b_FWAVCAudioDevice` → `m_audio_b_FWAudioDevice` → `m_audio_b_FWDiscreteDevice` (OSMetaClass superclass chain; Q16 Correction 2026-10-14). It carries the plain `0x18/0x19` AVCControlPlugSignalFormat (Q3a §3).
- `kext/src/FW1814Device.{h,cpp}` — empty shell, no engines yet.
- `kext/src/MakeAppleFWAudioGoAway.{h,cpp}` — personalities `FW1814AppleGoAway1`/`12` on `IOFireWireAVCSubUnit`, `FireWire Product Name = "FW 1814"` (with the space), `SubUnit_Type` 1 and 12; runtime `probe()` score `0x7FFFFF` (Q-INFO, Q8).
- `kext/src/Trace.h` — os_log + os_signpost macros.
- `scripts/stage-kext.sh` — stage `build/` → `/tmp` → `kextutil -tn /tmp/OpenAudioFW1814.kext` → ONLY on green copy to `/Library/Extensions`.

**Pseudocode skeleton:**
```cpp
bool com_openaudio_FWMetaNub::probe(IOService *provider, SInt32 *score) {
    if (!super::probe(provider, score)) return false;
    // Read Vendor_ID + Model_ID off the IOFireWireDevice provider property dict (Q-INFO: provider is IOFireWireDevice, not IOFireWireUnit).
    // No plist IOProbeScore; runtime score only (Q-INFO). Value per Q7 — unverified for FWMetaNub (0x7FFFFF is the GoAway score, Q8).
    return true;
}
bool com_openaudio_FWMetaNub::start(IOService *provider) {
    if (!super::start(provider)) return false;
    // Dispatch by Model_ID: 0x10070 → publish bootloader nub; 0x10071 → publish runtime nub.
    // State: UNPLUGGED → BOOTLOADER_ATTACHED or RUNTIME_ATTACHED (C1 §2).
    return true;
}
```

**Acceptance test:** `kextload` succeeds on build PC; `ioreg -c com_openaudio_FWMetaNub` shows the nub with the correct Model_ID; no panic in 60s with device attached; `com.apple.AppleFWAudio` is NOT bound to the FW1814 unit (verify via `ioreg`).

---

### M-B — DCL rings + continuous silence TX

**Blocked on:** §2.B (Q-DCL, Q-POOL).

**Input docs:** `C4-transport-control-path.md` §3/§4/§11, `A1-ghidra/Q1` + `Q1a` + `Q1b`, `A1-ghidra/Q17` (TPacketPool corrected), memory `reference_dcl_structure_authoritative.md`, memory `project_fw1814_idle_lock_mechanism.md`.

**Output files:**
- `kext/src/FW1814DCLBuilder.{h,cpp}` — builds TX program (4 banks × [4 one-packet primers + 1 segment of P−4 packets] + 4 jumps = 20 segments, per Q-DCL) + RX program (64 segments × 4 packets + 64 jumps). Each TX segment: 4 primer packets + 1 big-callback packet + `DCLPtrTimeStamp` + jump.
- `kext/src/TPacketPool.{h,cpp}` — layout per Q17 field map (56-byte struct). `inTaskWithPhysicalMask(kernel_task, N_pages*4096, physMask & ~0xFFF)` where `N_pages` from Q-POOL. `ZeroElements` fills `0x000000CF` in data region, `0x00000080` in pad region.
- `kext/src/FWBaseEngine.{h,cpp}` — minimal engine that owns the DCLBuilder + packet pool; starts/stops the ring.
- `core/include/openaudio/Am824.h` — constants `AM824_LABEL_IDLE = 0xCF`, `AM824_LABEL_NODATA = 0x80`.

**Pseudocode skeleton:**
```cpp
IOReturn FW1814DCLBuilder::BuildTxProgram(uint32_t rate) {
    const auto geom = kDCLGeometry[rateIndex(rate)];  // from Q-DCL
    txPool_.Initialize(geom.poolPages * 4096, geom.cipDataQ, geom.cipPadQ);
    for (int bank = 0; bank < 4; ++bank) {
        for (int primer = 0; primer < 4; ++primer) addPrimerPacket(...);
        addBigCallbackPacket(...);
        addJump(nextBank);
    }
    return kIOReturnSuccess;
}
```

**Acceptance test:** With kext loaded and streaming started at 48 kHz, device PLL locks (observable via front-panel sync LED or `MARateLocked = 1` in ioreg). TX runs continuously without underrun for 60 s. RX produces exactly `48000 × 60` samples ±1 per channel. No clicks at idle.

---

### M-C — CMP connect + stream-kick + first streaming @ 48k

**Blocked on:** §2.C (Q-FCP-OUTPUT, Q-FCP-INPUT, Q-CMP-CONNECT, Q3a).

**Input docs:** `C4-transport-control-path.md` §2/§13, `A1-ghidra/Q3`, `C2-sync-path.md` §7, memory `project_fw1814_stream_kick.md`, memory `project_fw1814_plugs.md`.

**Output files:**
- `kext/src/FW1814Device.cpp` — add `ConnectAllPlugs()` from Q3; uses `IOFireWireController::allocateIsochChannel` with Q-CMP-CONNECT params.
- `core/include/openaudio/Avc.h` + `src/Avc.cpp` — AV/C envelope encode. One-shot sender: `avcSendControl(unit, 10, bytes)`.
- `kext/src/FWBaseEngine.cpp` — add `KickStream()`: emit OUTPUT 0x18 bytes → `IOSleep(100)` → emit INPUT 0x19 bytes.
- `kext/src/FWEngine2.{h,cpp}` — multichannel engine subclass. Registers as `IOAudioEngine` with CoreAudio. Flavor = 1 per Q13.

**Pseudocode skeleton:**
```cpp
IOReturn FW1814Device::ConnectAllPlugs() {
    // From Q3 + Q-CMP-CONNECT.
    for (int o = 0; o < 2; ++o) connectoPCR(o, speed=S400, channel=auto, bw=BW_48K_8CH);
    for (int i = 0; i < 3; ++i) connectIPCR(i, speed=S400, channel=auto, bw=...);
    return KickStream();
}
IOReturn FWBaseEngine::KickStream() {
    sendAvc(FCP_CMD_OUTPUT_0x18_BYTES);  // Q-FCP-OUTPUT
    IOSleep(100);
    sendAvc(FCP_CMD_INPUT_0x19_BYTES);   // Q-FCP-INPUT
    return kIOReturnSuccess;
}
```

**Acceptance test:** CoreAudio sees FWEngine2 at 48 kHz with the reference channel counts (**unverified — "10-in / 12-out" in the first draft had no source; take counts from Q15c per-rate table + B5 ioreg captures**). Play `/System/Library/Sounds/Submarine.aiff` to the device; audible on headphone jack. Record from Analog-In-1 for 10 s; capture produces non-zero samples with no gaps.

---

### M-D — All 7 rates + format change

**Blocked on:** §2.D (Q15e, Q-SYT, Q-RATE-LATENCY).

**Input docs:** `C2-sync-path.md` §4/§5, `A1-ghidra/Q15c` (format-change chain), `A1-ghidra/Q2` (SetupCIP + FDF table).

**Output files:**
- `core/include/openaudio/Fdf.h` + `src/Fdf.cpp` — 7-rate FDF + SYT-interval table.
- `kext/src/FWBaseEngine.cpp` — add `performFormatChange(IOAudioStreamFormat const*, IOAudioSampleRate const*)`. Walks Q15c chain: `runAction(ActionPerformFormatChange)` → `FWEngine2::PerformFormatChange` → `vt[0xa78] RequestFormatUpdateForSampleRate` → `SetIOFormat` → `vt[0xce0]` (Q15e bytes) + `vt[0xaf8]` HSCI emit → `PerformSampleRateChangeCore` → walk DCL program collection + `ResetLatency`.
- `core/include/openaudio/Hsci.h` + `src/Hsci.cpp` — 12-byte envelope encoder (`company_id = 0x040004`, byte 6 = clock source, bytes 7+8 = digital in/out).

**Pseudocode skeleton:**
```cpp
IOReturn FWEngine2::PerformFormatChange(IOAudioStreamFormat const *f, ...) {
    if (f->fSampleFormat == kIOAudioStreamSampleFormat_AC3) return performAc3Change(f);
    const uint32_t rate = f->fSampleRate.whole;
    sendAvc(buildVtCe0CommitBytes(rate));           // Q15e
    sendAvc(buildHsciEnvelope(/*byte6=*/clockSrc, ...));  // Q-HSCI-9 for byte 9
    rebuildDcl(rate);
    resetLatency(rate);                             // from C6 Q-RATE-LATENCY
    return kIOReturnSuccess;
}
```

**Acceptance test:** Set rate to each of 44.1/48/88.2/96/176.4/192 kHz via Audio MIDI Setup; CoreAudio reports the new rate; loopback at each rate produces continuous audio with no clicks; `MARateLocked = 1` holds. Capture ioreg `SampleOffset` + `OutputSampleLatency` at each rate and verify ≤ reference (updates C6 §1 table).

---

### M-E — Mixer control plane

**Blocked on:** §2.E (Q3b, Q-MAMAP; Q15f / Q-OPCODE / Q-HSCI-9 resolved 2026-10-14).

**Input docs:** `C4-transport-control-path.md` §8/§9/§10, `C3-comm-path.md` §3/§4, `A1-ghidra/Q15` + `Q15b`, `B1-ffado-correlate.md` §4, `A2-panel/P3` (46 MA* keys).

**Output files:**
- `core/include/openaudio/PropertyDict.h` + `src/PropertyDict.cpp` — typed-property dict with `ChannelGroupProperty` entries; delta-apply model.
- `core/include/openaudio/MixerRegMap.h` — the 40-quadlet semantic map (per B1 + Q-MAMAP).
- `kext/src/FW1814Device.cpp` — add `SetProperties(OSDictionary*)` override. Dispatches via Q-MAMAP crosswalk to the channel-group batch path (→ `CustomMixerWriteQuads` block write at the group's register offset), the A/B-switch LED call (`AVCControlSetLEDStatus`, Q15f), or HSCI (clock source, Q5).
- `kext/src/FW1814Device.cpp` — `initHardware()` seeds Q3b power-on defaults.

**Pseudocode skeleton:**
```cpp
IOReturn FW1814Device::setProperties(OSObject *properties) {
    auto dict = OSDynamicCast(OSDictionary, properties);
    if (!dict) return kIOReturnBadArgument;
    // Walk every MA* key in dict, dispatch via Q-MAMAP table.
    // Mirrors FWPropertyDict::SetProperties @ 0xa0e0 → workloop → ApplyModifiedProperties.
    for (auto key : dict) {
        const auto &entry = kMaMap[key.name];  // Q-MAMAP crosswalk
        switch (entry.wirePath) {
        case WirePath::Group:  groups[entry.group].set(entry.channel, key.value); break; // dirty-marks the group
        case WirePath::AbLed:  avcControlSetLEDStatus(encode(key.value)); break;        // vt[0xbe8], Q15f
        case WirePath::Hsci:   setClockSourceInternal(key.value); break;                 // Q5, bytes 6..9
        }
    }
    for (auto &g : dirtyGroups())   // BaseChannelGroup::SendLevelsToDevice @ ~0x1efc0
        customMixerWriteQuads(g.regOffset /* 0x00/0x08/0x10/0x34/0x38 */, g.count() >> 1, g.packedLevels());
        // → async block write 0xFFC7:0x00700000 + regOffset (CustomMixerWriteQuads @ 0x22c7a)
    return kIOReturnSuccess;
}
```

**Acceptance test:** `ioreg -c com_openaudio_FW1814Device -l` lists all 46 MA* keys with Q3b power-on defaults. `IOObjectSetCFProperty` with a new `MAMainLevelCh01 = 0x4000` moves the physical main-L output to -6 dB (verify with a scope or a loopback-at-known-tone). Hot-swap clock source Internal → ADAT; PLL re-locks within 500 ms.

---

### M-F — User-client surface

**Blocked on:** nothing in §2.F (all four resolved 2026-10-14, `Q-UC-surface.md`).

**Input docs:** `C5-userclient-surface.md` §2/§3/§4, `A1-ghidra/Q11` (8 selectors), `A1-ghidra/Q14` + `Q14a` (notification).

**Output files:**
- `kext/src/FW1814UserClient.{h,cpp}` — 8 selectors from Q-UC-SELECTORS; `clientMemoryForType(1)` returns the shared meter ring from Q-UC-RING.
- `kext/src/FWKIPCUserClient.{h,cpp}` — required: base class of the vendor UC (Q-UC-surface §1); type-0 notification port + property serialization.
- `kext/src/MeterRing.{h,cpp}` — the 4 KiB shared page (`IOBufferMemoryDescriptor::withOptions(0x10000, 0x1000, 0x1000)`); written whenever a device control packet arrives (`ReceiveControlPacket`, Q-UC-surface §3). Cadence = the device's control-packet rate (**unverified** — measure on hardware).
- `tools/meter-dump.cpp` — userspace tool that attaches to the ring and prints 70-channel peak values.

**Pseudocode skeleton:**
```cpp
IOReturn com_openaudio_FW1814UserClient::externalMethod(
    uint32_t selector, IOExternalMethodArguments *args, ...) {
    if (selector < 500 || selector > 507) return kIOReturnBadArgument;
    return kSelectorDispatch[selector - 500](args);  // Q-UC-surface §2 table (legacy IOExternalMethod semantics: flags/count0/count1)
}
IOReturn com_openaudio_FW1814UserClient::clientMemoryForType(
    UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory) {
    if (type != 1) return kIOReturnUnsupported;
    *memory = meterRing_->getBMD();
    (*memory)->retain();
    return kIOReturnSuccess;
}
```

**Acceptance test:** `tools/meter-dump` attached to the kext prints live peak values that respond to audio input. All 8 selectors round-trip correctly from a userspace test harness. 44-byte Mach notification arrives on userspace listener when a MA* property changes.

---

### M-G — Panel baseline

**Blocked on:** §2.G (Q-CLOCK-UI, Q-PANEL-LAYOUT — **requires user sign-off**).

**Input docs:** `C5-userclient-surface.md` §5/§6, `A2-panel/P3` (46 MA* keys), `Q-MAMAP-crosswalk.md`, approved `BUILD-MANUAL-UI-WIREFRAME.md`.

**Output files:**
- `control-panel/OpenAudioFW1814Panel.xcodeproj`
- `control-panel/Sources/Model/MaKeys.swift` — 46-entry enum with per-key type + encoding (from Q-MAMAP).
- `control-panel/Sources/IOKitBridge/DeviceRegistry.swift` — `IORegistryEntrySetCFProperties` wrapper.
- `control-panel/Sources/IOKitBridge/UserClient.swift` — 8-selector dispatch.
- `control-panel/Sources/IOKitBridge/MeterRing.swift` — `IOConnectMapMemory(1)`-based meter reader.
- `control-panel/Sources/MixerView/` — 22×4 + 22×2 routing matrix.
- `control-panel/Sources/HardwareTabView/*.swift` — all hardware-tab controls (clock, S/PDIF, optical, S/MUX, AC3, buffer, rate, stereo-pair).
- `control-panel/Sources/DeviceList/` — multi-interface picker.

**Pseudocode skeleton:**
```swift
final class DeviceRegistry {
    func set<T>(_ key: MaKey, to value: T, on device: io_registry_entry_t) throws {
        let dict = [key.rawValue: encode(value, as: key.encoding)] as CFDictionary
        let kr = IORegistryEntrySetCFProperties(device, dict)
        guard kr == KERN_SUCCESS else { throw IOKitError(kr) }
    }
}
```

**Acceptance test:** Launch panel. Device picker lists all attached FW1814s. Mixer tab shows 22×4 matrix with Q3b power-on defaults. Hardware tab toggles clock source → device re-locks. Meters animate when audio plays. Multi-interface: attach two devices, each gets its own window.

---

### M-H — MIDI

**Blocked on:** §2.H (Q-MIDI-WIRE).

**Input docs:** `A3-midi/README.md` + M1..M10, `C5-userclient-surface.md` §7.

**Output files:**
- `midi-plugin/OpenAudioFW1814MIDI.xcodeproj`
- `midi-plugin/Sources/Factory.cpp` — CoreMIDI CFPlugIn factory.
- `midi-plugin/Sources/Driver.cpp` — CoreMIDI entry points.
- `midi-plugin/Sources/Device.cpp` — endpoint model.
- `midi-plugin/Sources/Wire.cpp` — MIDI wire packet layout; calls UC selectors 500 (MIDIStart), 501 (MIDIStop), 503 (MIDISend).

**Acceptance test:** `midi` command-line in Audio MIDI Setup shows the FW1814's MIDI I/O. External MIDI keyboard playing into DIN-In sends notes to CoreMIDI endpoints; CoreMIDI playback appears at DIN-Out.

---

### M-I — AC3 passthrough

**Input docs:** `C4-transport-control-path.md` §12, `A1-ghidra/Q15c` (`cac3` fork).

**Output files:**
- `kext/src/FWEngine2.cpp` — add `performAc3Change(IOAudioStreamFormat const*)`. Branches off `cac3` fork at the kext's format-change dispatcher.

**Acceptance test:** Set format to AC3 passthrough via AMS; S/PDIF out carries a valid AC3 bitstream (verify with a DD-capable receiver or `dcadec`).

---

### M-J — Stereo-pair engines + 4-engine family

**Input docs:** `C1-lifecycle-state-machine.md` §2 (CLIENT_IDLE / STREAMING_LOCKED), `A1-ghidra/Q13` (4-engine family).

**Output files:**
- `kext/src/FWEngine1.{h,cpp}` — stereo-pair engine.
- `kext/src/FWPairEngine.{h,cpp}` — paired-engine variant.
- `kext/src/FWTimeSlaveEngine.{h,cpp}` — time-slave engine.
- `kext/src/FW1814Device.cpp` — probe-time branch on `MAEnableStereoPairEngines`: if true, instantiate the stereo-pair engine set; if false, 1× `FWEngine2` (multichannel). **Unverified: engine count when stereo-pair mode is on — the first draft's "5× FWEngine1" had no source; take it from A1-Q13 + B5 §3 before M-J.**

**Acceptance test:** Toggle `MAEnableStereoPairEngines` in panel → kext teardown → 5 stereo engines appear in AMS instead of 1 multichannel engine (or vice versa). Each engine streams independently.

---

### M-K — Multi-interface

**Input docs:** `A1-ghidra/Q7` (polymorphic FWMetaNub), `C1-lifecycle-state-machine.md` §4 (dual UC lifecycle).

**Output files:** none new — M-A's `FWMetaNub` already handles arbitrary nub instances. Panel's `DeviceList/` already iterates.

**Acceptance test:** Attach two FW1814s to the same FireWire bus. `ioreg` shows two independent `com_openaudio_FW1814Device` instances with distinct GUIDs. CoreAudio sees them as two separate aggregate devices. Open both in panel; set different rates on each; both stream independently without interference.

---

### M-L — Latency validation

**Blocked on:** §2.I (Q-LOOPBACK).

**Input docs:** `C6-latency-budget.md` §1/§2/§9.

**Output files:**
- `tests/hw-smoke/loopback.sh` + supporting Swift CLI tool.
- `docs/system-map/C6-latency-budget.md` — updated with measured per-rate numbers.

**Acceptance test:** At every rate (44.1/48/88.2/96/176.4/192 kHz) at every supported buffer size (64/128/256/512/1024/2048/4096), round-trip loopback latency is `≤` reference numbers. If any rate fails, open a defect against the relevant milestone's engine code.

---

### M-M — Modernization pass

**No new system-map research required.** UX-only.

**Input docs:** approved `BUILD-MANUAL-UI-WIREFRAME.md`, `feedback_clone_scope_full_parity_modern_ui.md`.

**Output files:** scoped Swift additions under `control-panel/Sources/`:
- Dark-mode asset set.
- Resizable window with saved frame.
- Saved presets (JSON on disk; serialize/deserialize MA* key dict).
- Per-channel meter history (ring buffer, N-second scrollback).
- Keyboard shortcuts (focus-ring on mixer; ↑↓ fader; arrow-nav the matrix).

**Acceptance test:** All reference-parity controls from M-G still work. Dark mode toggles via System Preferences. Preset round-trips (save → close → re-open panel → load → device state matches). Keyboard shortcuts documented in `control-panel/Resources/Help.html`.

---

## 4. Standing rules during build

- **Stage before autoload.** `scripts/stage-kext.sh` is mandatory on every kext change — `/tmp` → `kextutil -tn` → `/Library/Extensions`. See memory `feedback_stage_kext_before_autoload.md`.
- **Never touch `_legacy_pre_ghidra/`.** It's informational only; the pre-Ghidra scaffold is known-wrong on the UC surface and the mixer model.
- **Provenance rule still applies.** Any new `.md` under `docs/system-map/` must cite source bytes (Ghidra offset, FFADO file:line, SDK header, or dated hardware capture). No unmarked guesses.
- **Correction blocks.** If a milestone's measurement contradicts an earlier doc, add a dated `**Correction (YYYY-MM-DD):**` block to the earlier doc — don't leave contradictory statements.
- **One commit per acceptance test.** Each milestone's acceptance test = one commit boundary.

---

## 5. Pre-build research — execution checklist

Order the Pre-Build Research pass as follows. Each item produces a short `.md` under `docs/system-map/` with provenance.

1. **Q-SDK** (header grep, no device needed) — unblocks M-A.
2. **Q-INFO** (plutil read + edit) — unblocks M-A.
3. **Q-DCL + Q-POOL** (Ghidra `docs/system-map/A1-ghidra/otool-exports/mafw.asm` reads) — unblocks M-B.
4. **Q-FCP-OUTPUT + Q-FCP-INPUT + Q-CMP-CONNECT + Q3a** — unblocks M-C.
5. **Q15e + Q-SYT + Q-RATE-LATENCY** — Q15e + Q-SYT are Ghidra reads; Q-RATE-LATENCY is a 10-minute SL rig session. All three unblock M-D.
6. **Q3b + Q-MAMAP** (Q15f, Q-OPCODE, Q-HSCI-9 resolved 2026-10-14) — the biggest batch; Q-MAMAP is the single biggest file (46-row table). Unblocks M-E.
7. **Q-UC-SELECTORS + Q-UC-RING + Q-KIPC-LIVE + Q-NOTIFY** — unblocks M-F.
8. **Q-CLOCK-UI + Q-PANEL-LAYOUT** — Q-PANEL-LAYOUT requires user wireframe sign-off. Unblocks M-G.
9. **Q-MIDI-WIRE** — unblocks M-H.
10. **Q-LOOPBACK** — unblocks M-L.
11. **Q-SIGN** — user supplies Developer ID + Team ID. Blocks final signed-build; not blocking per-milestone test loads under `kextutil -s`.

Doing all pre-build research first (option A) is the think-free path: no milestone will pause for a sub-task mid-code. Doing it just-in-time (option B) parallelizes but requires occasional context-switching. Recommend option A so Sonnet-medium runs mechanically straight through M-A..M-M.
