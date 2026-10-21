# C1 — Lifecycle state machine

> **Correction (2026-10-20, R113):** where this file says the helper halts the firmware when it is "older than the bundled one" (§ paths A/B/C and the NUB_ROUND1 transition row), the helper's test is **not equal**, not older: `[fwFile date] isEqual: deviceDate` (`A4-helper/ghidra-exports/decomp/a4-h2-bootloader-detection.log:160`). A newer firmware is re-flashed too. There is no minimum firmware version in the official driver.

> **Correction (2026-10-15, live, R40):** observed lifecycle on 10.13.6 (clone, build PC): power-on enumerates a **bootloader** `IOFireWireDevice` (Model_ID `0x10070`, GUID …041…) then, ~3 s later and without any `Go`, a separate **runtime** device (`0x10071`, GUID …040…); the nub is created for both; the product class starts only after `MAFirmwareStart` (written by the LaunchDaemon helper on every first match). `Halt` is a ~3 s detour (bootloader nub, then back to runtime by itself). The firmware **update** branch of the reference lifecycle (Halt -> `UpgradeImage` -> Start) is out of scope for the clone (flash cut, R42). Log: [`build-pc-runs/2026-10-15-M-A-M-FW-install.md`](build-pc-runs/2026-10-15-M-A-M-FW-install.md).


**Status:** consolidated 2026-10-14 (task P5 of `docs/BUILD-PLAN.md`). The body below is one consistent version: every 2026-10-14 Correction block that used to sit on top of this file has been applied to the text and is listed in the change log at the end. The 2026-10-13 original is in git history (`git show a9cb63c:docs/system-map/C1-lifecycle-state-machine.md`).

**Scope:** every state the FW1814 can occupy from cold plug through firmware handling, IOKit matching, user-client open, streaming, format/clock change, bus reset, down to unplug / terminate. Each edge names its trigger, its mechanism (kext offset or capture cite) and its side effects.

**Bundles.** Kext offsets are `_extracted/` kext, bundle 1.11.3 (`otool-exports/mafw.asm`). Captures named "SL" are the Snow Leopard rig running driver **v1.9** (version-skew rule: v1.9 and 1.11.x are different builds; call-site sleeps were checked identical in both). Hypotheses are tagged **unverified** with the check that would settle them.

Companions: C2 (sync), C3 (comm), C4 (transport/control), C5 (user client), C6 (latency). Overlap is cited by name, not restated.

## 1. The one design fact that shapes everything: two registration rounds

`FWMetaNub::start @0x1cf68` publishes the nub and calls `registerService(0)` (`0x1d17b`), which starts IOKit matching of the per-product personalities (`IOProviderClass = com_m_audio_FWMetaNub`, matched on `FireWire Product Name`, plist). The product class's `probe` is `m_audio_b_FWAVCAudioDevice::probe @0x22f78`; it **returns NULL** while the nub's `fBootloaderActive [+0xf0]` is set **or** `[+0xf3]` ("firmware match started") is still 0, and otherwise returns itself with score `0x7FFFFF` (`0x23050..0x23068`). `[+0xf3]` is written only in `FWMetaNub::FirmwareStart @0x1beb4` (`0x1bf7a`), whose only caller is `SetOneProperty @0x1caf8` for the registry key **`MAFirmwareStart`** — i.e. a userland write by the helper. `FirmwareStart` in runtime mode then calls `registerService(0)` a **second** time; this second round is the one that accepts the product class. Every attach therefore produces one rejected, never-started candidate instance that IOKit frees; the reference's `free()`/destructors call `operator delete(NULL)` for members that were never allocated, which 10.13.6's `operator delete` (`__ZdlPv @0xffffff8000830c60`, no NULL test) turns into a kernel panic (`A1-ghidra/Q-SDK5-first-round-probe-reject.md`, R31). **Consequence for the clone: mirror the two-round lifecycle, and make every `free()`/destructor NULL-safe (a 10.13.6-forced deviation; comment each).**

## 2. State graph

```
                          power off / unplug
        +----------+ <------------------------------------------------------+
        |UNPLUGGED |                                                        |
        +----------+                                                        |
             | device appears (Config ROM Vendor_ID 0x000D6C)               |
             v                                                              |
   +----------------------+  FWMetaNub::start: fState=1, registerService(0) |
   | NUB_ROUND1           |  product candidate probed → REJECTED (§1)       |
   |  (bootloader 0x10070 |  [+0xf0]=1 in bootloader mode                   |
   |   or runtime 0x10071)|  [+0xf3]=0 in both                              |
   +----------------------+                                                 |
        |                                                                   |
        |  helper (MAFireWireLoader) matches com_m_audio_FWMetaNub,          |
        |  reads MAFirmwareInfo, then writes registry properties (A4-H2):    |
        |                                                                   |
        |   A) no update needed → MAFirmwareStart ─────────────────┐        |
        |   B) update needed, firmware running → MAFirmwareHalt    │        |
        |        (device resets into bootloader, re-matches ──┐    │        |
        |         as NUB_ROUND1/0x10070)                       │    │        |
        |   C) bootloader + update → MAFirmwareUpgradeImage ─> MAFirmwareStart
        v                                                      v    v        |
   +---------------------+   FirmwareStart (bootloader mode):  fState=3,     |
   | FIRMWARE_UPDATING   |   writes START quadlet 0x01110000 to              |
   |  (fState=5)         |   0xFFFF:0xC8021000, then                         |
   +---------------------+   ReregisterDeviceAfterFirmwareReset (fState=2,   |
        |                    2500 ms timer)                                 |
        v                                                                   |
   +-----------------------+                                                |
   | AWAITING_RUNTIME_RESET|  device resets; new FireWire device, Model_ID  |
   +-----------------------+  0x10071 → new nub → NUB_ROUND1 (runtime) ─────┤
        (re-enters NUB_ROUND1 as a fresh FWMetaNub; helper writes MAFirmwareStart again)
        |
        | FirmwareStart (runtime mode): [+0xf3]=1, registerService(0) = ROUND 2
        v
   +---------+   product class accepted → start → initHardware → engine(s)
   | MATCHED |   published; IOAudioFamily publishes the engine subtree
   +---------+
        | vendor UC opened (Panel / helper / MIDIServer)
        v
   +-------------+  CoreAudio client starts the engine
   | CLIENT_IDLE |────────────────────────┐
   +-------------+                        v
        ^                      +-----------------+  start chain (§3.1, ~3.5 s at idle)
        |                      | STREAM_STARTING |
        |                      +-----------------+
        |                              v
        |                      +------------------+   performFormatChange / MASampleRate
        +----------------------| STREAMING_LOCKED |──────────────┐
          engine stopped       +------------------+              v
                                  |       ^              +---------------+
                                  |       +--------------| FORMAT_CHANGE |
                                  |                      +---------------+
                  bus reset       v
                          +--------------------+  ReconnectP2P on a new thread (§3.2)
                          | BUS_RESET_RECOVERY |──> back to STREAMING_LOCKED
                          +--------------------+
                  device gone     v
                          +--------------+  kIOFWMessageServiceIsRequestingClose 0xE0000020
                          | DISCONNECTED |
                          +--------------+
                                  v
                          +------------+  FWMetaNub::stop, sNubDictionary entry removed
                          | TERMINATED |──> UNPLUGGED
                          +------------+
```

Paths A/B/C are the three passes of the helper's decision logic (A4-H2, pseudocode lines 61/65/75): the helper writes `MAFirmwareStart` even when no update is needed ("Starting <product>"), `MAFirmwareHalt` when the running firmware is older than the bundled one, and `UpgradeImage`+`Start` when the device is already in bootloader mode ("ready for upload" = `MAFirmwareInfo[0xc] != 0`). The helper's decisions are *external*: the kext does not decide.

## 3. State definitions

### 3.1 `UNPLUGGED`
- **Condition:** no FireWire device whose Config ROM carries Vendor_ID `0x000D6C` and Model_ID `0x10070`/`0x10071`.
- **Classes:** none of ours. **Engines:** none.
- **Notifications:** the helper's matching notification for `com_m_audio_FWMetaNub` is armed (A4-H2).

### 3.2 `NUB_ROUND1` (bootloader or runtime)
- **Condition:** `FWMetaNub` matched on `IOFireWireDevice` (plist `FWMetaNub` personality: no `Vendor_ID`/`Model_ID` filter, no `IOProbeScore`; `FWMetaNub::probe @0x1ce62` accepts only vendor name "M-Audio"/"M-AUDIO" and a product name found in the `kDeviceSpecs` table — Q-INFO). `start` copies the provider's `FireWire Product/Vendor Name`, `Vendor_ID`, `Model_ID`, `GUID`, derives `MAActualProductName`, reads the 80-byte info block from `0xFFFF:0xC8020000` (`FirmwareCacheInfoRegisters`, published as `MAFirmwareInfo`; `[+0xf0] = (word at [+0x9c] != 0)` = bootloader flag), sets `fState [+0x88] = 1` ("initial-register") and registers (`0x1d16d..0x1d180`). The product candidate is rejected (§1).
- **Classes:** `FWMetaNub` only (plus the transient rejected candidate). **Engines:** none.
- **Notifications:** the helper's `deviceMatched:` fires for both bootloader and runtime nubs (A4-H2); the product-class watcher (`driverInstanceMatched:`) does not (no product class yet).

### 3.3 `FIRMWARE_UPDATING`
- **Condition:** `fState = 5` ("firmware-update"; written at `0x1c924` in `FirmwareUpdate`). Entered by the helper writing `MAFirmwareUpgradeImage` (the whole `.bcd` as data) while in bootloader mode. The BeBoB bootloader wire protocol runs **inside the kext** (`FirmwareUpdate`, `FirmwareUpdateBeBoB251/BeBoB32`, `DownloadSectionStart/SendNextBlock/Stop`; write-up pending — BUILD-PLAN P3). The helper does not speak the wire protocol (A4-H2 Finding 2, H12). On success `FirmwareUpdate` returns without a bus reset; on its **error path only** it calls `bus->vt[0x880]` (`0x1c9ea`; identified as `resetBus` by header slot order — medium confidence) and `IOSleep(0xdac)` = 3500 ms (Q-P3 §5, R37; the earlier text said it always ends with a bus reset). The full kext-side download protocol is in `A1-ghidra/Q-P3-kext-firmware-path.md`. `FirmwareUpdate` also sets `[+0xf1]` (update in progress) for the duration.
- **Classes:** `FWMetaNub`. **Engines:** none.
- **Notifications:** the helper starts the status UI (`sudo -u <console user> launch …`, A4-H2 Finding 3; progress via the status window, A4-H5).

### 3.4 `AWAITING_RUNTIME_RESET`
- **Condition:** `FirmwareStart` in **bootloader** mode (`[+0xf0] != 0`, log "Bootloader active: starting firmware") wrote START quadlet `0x01110000` to `0xFFFF:0xC8021000` (`FirmwareStart @0x1beb4`, Ghidra `FWMetaNub__FirmwareStart.c`), then `ReregisterDeviceAfterFirmwareReset @0x1bba6` (`fState = 2`, "reregister"; 2500 ms timer `0x9c4` on the nub's workloop). On expiry `ReregisterDeviceTimerFired @0x1bc5e` → `InvokeOnNewThread` → `RegregisterDeviceThread @0x1bcc2` → `registerService(0)`.
- **Observation (capture 73, SL v1.9, interface power-cycled):** off → two bus resets; on → bootloader nub → `M-FW: Bootloader active: starting firmware` → ~170 ms of 5 ms polling sleeps → bus reset → **firmware active +3.3 s after power-on** (`M-FW: Firmware active: registering metanub`) → `Reregister: device node vanished` → clock-source sets (300 ms ×2, then 2500 ms) → one-second mixer resend (the kext logs `Trying to resend mixer settings too often`) → the standard stream-start chain → quiet after ~14.7 s. In that run a **fresh** `FW1814Device` was instantiated (the old node vanished), so `ReconnectP2P`'s `IOSleep(1100)` did not occur.
- **Classes:** `FWMetaNub` (old). **Engines:** none.

### 3.5 `MATCHED`
- **Condition:** round 2 accepted the product class (`probe` score `0x7FFFFF`) → IOAudioFamily `start` → `initHardware` chain (`FWAVCAudioDevice::initHardware @0x22ed2` → `FWAudioDevice::initHardware @0x342e`: `AllocateConnectionManager @0x23c72`, `createAudioEngine @0x2fd8`, `RequestFormatUpdateForSampleRate` + `CompletePendingStateChanges`, `InitializeUserSettableProperties @0xe6ca`, meter page allocation, `ResetToFactorySettings @0xd190`; Q3b §1) → `FWEngine2::initHardware @0x12622` allocates the two DCL programs (TX `0x620` B, RX `0x518` B) and `FWDCLProgram::Initialize @0x27458` builds them (TX 20 segments = 4 banks × (4 one-packet + 1 large) + 4 jumps; RX 64 segments × 4 packets + 64 jumps — Q-DCL-geometry-table). The packet pool is pre-filled `0xCF`/`0x80` and refilled each TX callback before the converters write `0x40` (audio) / `0x80` (MIDI) (Q17, R10).
- **Class chain:** `FWMetaNub` → `com_m_audio_FW1814Device` : `FWRev1AudioDevice` : `FWAVCAudioDevice` : `FWAudioDevice` : `FWDiscreteDevice` : `IOAudioDevice` (superclass at `0xf473`; ctor `0xf04a` → `FWRev1AudioDevice` ctor `0xf060`; Q16). One `FWEngine2` (`MAEnableStereoPairEngines = 0` on SL, R17). `MakeAppleFWAudioGoAway` is registered **only for "FW Solo"** in the reference plist (Q8) — how the reference keeps `AppleFWAudio` off the FW1814 is not shown; the clone adds a FW1814 go-away personality (deliberate extension).
- **Notifications:** `initHardware` seeds `MARateLocked = 1` via `SetStaticPropertyAndNotifyUserClients @0x3644` (Q15d); IOAudioFamily publishes the engine subtree (`IOAudioStream`s, controls). On SL, `40-ioreg-playback.txt:4287-4324` holds five `IOAudioControlUserClient` instances (coreaudiod, SystemUIServer, Panel, System Preferences, afplay) — they are control user clients, **not** engine user clients.

### 3.6 `CLIENT_IDLE`
- **Condition:** at least one vendor `com_m_audio_FWUserClient` is open on `FW1814Device`. SL idle: two (helper pid 64, Panel pid 172; B5 §3, `12-ioreg-full.txt:5807..5814`); during a DAW session four (plus two MIDIServer connections; B5 §6c). `IOAudioEngineState = 0`.
- **User-client class:** `FWUserClient` derives from the live `FWKIPCUserClient` (Q-UC-surface); the Panel maps `clientMemoryForType(1)` = one 4 KiB page, `uint32 peak[52]`, written by `ReceiveControlPacket @0xe9ea` (not by a 30 Hz kext timer) (B5 §11, R6). The helper's UC is a notification sink (`IOConnectSetNotificationPort`; no `IOConnectCallMethod` imports, A4-H8b).

### 3.7 `STREAM_STARTING` (transient)
- **Trigger:** a CoreAudio client sets the engine active → `IOAudioEngine::startAudioEngine` → `FWEngine2::performAudioEngineStart` → `ConnectAllPlugs @0x12476` (`vt[0xcb8]`) → `ConnectAudioMIDIChannels @0x12156`.
- **The reference's start chain (R1, asm `0x121c9..0x122bb`):** start the TX DCL program → `IOSleep(500)` → AV/C INPUT plug-signal-format `00 FF 19 00 90 <SFC> FF FF` → `IOSleep(500)` → start the RX DCL program → AV/C OUTPUT plug-signal-format `00 FF 18 00 90 <SFC> FF FF` (no sleep after). CMP lock retries (`updateP2PCount @0x24690`) add `IOSleep(100)` per retry. **Runtime confirmation (capture 72, SL v1.9; sleep call sites identical in 1.11.3):** at idle a rate change runs 3×`IOSleep(100)` → `IOSleep(500)` → `IOSleep(500)` → `IOSleep(100)` → ~0.9 s gap → `IOSleep(1000)`; ≈3.5 s in total. The frame *contents* are not observable on the SL rig (`fbt` reaches only `mach_kernel`). The order "OUTPUT 0x18 → 100 ms → INPUT 0x19" belongs to snd-bebob (`bebob_maudio.c:312-337`) and the pre-Ghidra notes, **not** to the reference. Whether the 500 ms gaps are *required* by the device is **unverified** (check: timestamped FCP/isoch capture on SL).
- **Side effects:** isoch channel allocated and bound via `createLocalIsochPort(talking, head, NULL, 0, 0, 0)` (7-argument classic DCL call; NuDCL is not used — Q1b). Whether 10.13.6's `AppleFWOHCI_DCLProgram::init` accepts the kext's classic chain is **unverified** (BUILD-PLAN P1/R-1). Segment interrupts come from the **CallProc** callback; the timestamp node does not raise them (Q-OHCI-interrupt); where `takeTimeStamp` is called from is **unverified** (C2 §8.3).

### 3.8 `STREAMING_LOCKED`
- **Condition:** `IOAudioEngineState = 1`, `IOAudioEngineNumActiveUserClients = 1`, one active `IOAudioEngineUserClient` (SL: afplay directly, not coreaudiod; B5 §6a). Both `FWStream` nodes report `IOAudioStreamNumClients = 1`. DCL programs running (`[+0x8e] = 1` on TX and RX).
- **Background activity:** the HSCI poll re-arms every `50 + [+0xd4]` ms (`0x1d50d..0x1d513`, R6) and, through `DetectExternalSyncChanges @0x2bc3`, may update `MARateLocked` and broadcast via `SetStaticPropertyAndNotifyUserClients` (Q6, Q15d). Peak data reaches the Panel through the shared page (§3.6). Idle-time TX is data-bearing (424-byte packets, DBS 13; capture 74); that continuous data-bearing TX is *needed for the INPUT rate command to succeed* is third-party-verified (macfw), while "the PLL locks against it" is a **hypothesis** (STATE Idle-lock Correction).

### 3.9 `FORMAT_CHANGE` (transient, workloop-serialised)
- **Trigger:** CoreAudio `performFormatChange(stream, format, rate)` or the Panel writing `MASampleRate` (fast path → `RequestFormatUpdateForSampleRate`; Q15b/Q15c).
- **Chain (Q15c, slot names per R2):** `FWBaseEngine::performFormatChange @0x3e54` → `IOCommandGate::runAction` → `ActionPerformFormatChange @0x3e43` → `jmp vt[0xc88]` = `FWEngine2::PerformFormatChange @0x124cc` → `pauseAudioEngine` (`vt[0xa50]`) → `FWAudioDevice::RequestFormatUpdateForSampleRate @0x2a84` → `FWBaseEngine::SetIOFormat @0x42aa` → `IOAudioEngine::beginConfigurationChange` (`vt[0xb00]`, skipped for AC3) → `FW1814Device::ResetRearrangementProcs @0xcad4` (device `vt[0xaf8]`) → if the rate changed: `IOAudioEngine::setSampleRate` (`vt[0xb58]`) and `FWEngine2::PerformSampleRateChangeCore @0x117e6` (`vt[0xce0]`; iterates the DCL programs with `ApplyRateChange`, then `ResetLatency`) → `SetStreamFormat` on the output and input streams → the slot-`0xb10` call at the end of `SetIOFormat` (resolves to `cancelConfigurationChange` per R2 — **suspicious; `completeConfigurationChange` is `0xb08`; unverified which is intended**) → `resumeAudioEngine` (`vt[0xa58]`).
- **Not in this chain:** `vt[0xce0]` is *not* an AV/C commit and `vt[0xaf8]` is *not* an HSCI emit. AC3: flag `[+0x2d4]`, `'cac3'` fork in `PerformFormatChange` (Q12/Q15c). The FDF argument is the enum 0..6 = 32/44.1/48/88.2/96/176.4/192 kHz; `[+0x224]` = 8/16/32 is the SYT interval, not a channel count (R5).
- **Observed (SL, 44.1→48 kHz in Studio One 2):** `IOAudioEngineSampleOffset` 40→48, `IOAudioEngineOutputSampleLatency` 216→231; input latency 32, rate-independent (B5 §6c). `MARateLocked` may bounce one HSCI tick later.
- **Caveat (hazard, not a requirement):** v1.9 kernel-panics in `IOAudioFamily` buffer re-allocation when a rate change arrives mid-stream (B5 capture 65). The clone must gate `performFormatChange` on `IOAudioEngineState`/active client and pause audio first.

### 3.10 `BUS_RESET_RECOVERY`
- **Mechanism (R4):** `FW1814Device::BusReset @0xe53a` stores the member-function pointer `0xBF1` (`0xe5ab`; odd = virtual, slot `0xBF0`) and tail-calls `InvokeOnNewThread` (`0xe5cc`), so **BusReset = "run `ReconnectP2P @0xe1ea` on a new thread"**. `ReconnectP2P`: `DisconnectAllPlugs` (`vt[0xcc0]`) → `ConnectAllPlugs` (`vt[0xcb8]`) → `IOSleep(1100)` → if `[engine+0x2d0] == 0` tail-calls `SetClockSourceInternal(fMode, blankSlate = true)` (`vt[0xb60]`). `[+0x2d0]` is the **external-sync mode flag** (written only at `0x53f0`, which refuses with "Cannot change sync mode while audio/MIDI streams are running"), not `fRunning`; so the tail resends the clock source (an HSCI frame) only when the engine is *not* in external-sync mode. It is **not** an AV/C stream kick. The field name `fExternalSync` is ours (**hypothesis**; check: decode a caller of `0x53f0`).
- **Observation gap:** the live-device `ReconnectP2P` (1100 ms) path was never observed — the power-cycle capture produced a fresh device instance instead (§3.4). How `message()` (`0xE0000020`/`0xE0000030`) connects to `BusReset` is **not resolved** here (Q4 does not show the caller; **unverified**).

### 3.11 `DISCONNECTED` / 3.12 `TERMINATED`
- `DISCONNECTED`: `FWMetaNub::message @0x1b9d6` receives `kIOFWMessageServiceIsRequestingClose = 0xE0000020`; `fServiceClosing [+0xf2]` set; the engine goes to state 0 as IOAudioFamily catches the terminate; the helper's `driverInstanceTerminated:` fires (A4-H2/H8). The message is forwarded through a local vtable slot (`0x740`) and a global hook; what the global hook at `0x14467c(%rip)` does is **unresolved** (Q7).
- `TERMINATED`: `FWMetaNub::stop @0x1ba84` sets `fStopped`, removes the GUID from `sNubDictionary`. **The reference kext never terminates user clients** (STATE 2026-10-14 Part-A batch; backlog item A1-Q-UC.6: `clientClose` does not call `terminate()`); user clients created by earlier process restarts disappeared with the old device instance (capture 73), so the leak is bounded by device lifetime. The static `sUserClients` OSSet of `FWKIPCUserClient` is class-level; explicit removal at terminate is **unverified**.
- `FirmwareHalt` writes nothing while `[+0xf0]` is set (`cmpb $0,0xf0; jne` at `0x1c007`).

## 4. Transition catalogue

| Edge | Trigger | Mechanism | Side effects |
|---|---|---|---|
| UNPLUGGED → NUB_ROUND1 | device attach | IOKit matches the `FWMetaNub` personality (`IOFireWireDevice`); `probe @0x1ce62`, `start @0x1cf68` | `fState=1`; `[+0xf0]` from the info block; `sNubDictionary` gains the GUID; product candidate rejected (§1); helper `deviceMatched:` |
| NUB_ROUND1 → (runtime) MATCHED | helper writes `MAFirmwareStart`, no update needed | `SetOneProperty @0x1caf8` → `FirmwareStart @0x1beb4`: runtime branch logs "Firmware active: registering metanub", `[+0xf3]=1`, `registerService(0)` | round 2: product class accepted; `initHardware`; `MARateLocked = 1`; helper `driverInstanceMatched:` |
| NUB_ROUND1 (runtime) → NUB_ROUND1 (bootloader) | helper writes `MAFirmwareHalt` (running firmware older than the bundle) | `FirmwareHalt @0x1bff2` (`fState=4`; writes RESET opcode `0x01020000 | subcode 1` to `0xFFFF:0xC8021000` per Q7; no write if `[+0xf0]` set) | device resets, re-enumerates as `0x10070` |
| NUB_ROUND1 (bootloader) → FIRMWARE_UPDATING | helper writes `MAFirmwareUpgradeImage` (`.bcd`) | `FirmwareUpdate…` in the kext (§3.3) | status UI; on error only: `resetBus` + 3.5 s sleep (Q-P3 §5) |
| NUB_ROUND1 (bootloader) → AWAITING_RUNTIME_RESET | helper writes `MAFirmwareStart` | `FirmwareStart` bootloader branch: START quadlet, `ReregisterDeviceAfterFirmwareReset` | device reboots into runtime; new nub (round 1 again) |
| MATCHED → CLIENT_IDLE | Panel / helper / MIDIServer `IOServiceOpen` | `FWUserClient::initWithTask/start`; `clientMemoryForType(1)` mapped on request | one UC per creator pid |
| CLIENT_IDLE → STREAM_STARTING → STREAMING_LOCKED | engine start | §3.7 chain | isoch allocated, DCL running, `IOAudioEngineState=1` |
| STREAMING_LOCKED ↔ FORMAT_CHANGE | rate/format change | §3.9 chain on the engine workloop | latency republished |
| STREAMING_LOCKED → BUS_RESET_RECOVERY → STREAMING_LOCKED | bus reset | §3.10 | CMP reconnect, 1.1 s sleep, conditional clock resend |
| any → DISCONNECTED → TERMINATED → UNPLUGGED | device removal | §3.11 | engine subtree torn down by IOAudioFamily |

Configuration writes inside `STREAMING_LOCKED` (`MAClockSource`, `MASWReturnLevels`, `MAInputRoutingV2`, `MAMomentarySwitchSource`, …) change no state: `IORegistryEntrySetCFProperties → FW1814Device::setProperties → FWAVCAudioDevice::setProperties → FWPropertyDict::SetProperties @0xa0e0` queue work that `checkForWork` → `ApplyModifiedProperties` applies on the workloop (Q15b; C5 §6).

## 5. Dual user-client lifecycle

Panel and helper are independent opens of `com_m_audio_FWUserClient` (B5 §3); MIDIServer adds up to two during DAW use (B5 §6c). Retain counts at SL idle: 7 each; 8 for the Panel during Studio One (B5 §3/§6c). The vendor UC is scoped to the runtime device, not the nub, so nothing is open during NUB_ROUND1/FIRMWARE_UPDATING/AWAITING_RUNTIME_RESET (firmware control is via nub properties, not the UC). Streaming does not change UC retain counts: audio goes through `IOAudioEngineUserClient` (B5 §6a). The clone's vendor UC class must multiplex Panel, helper and MIDIServer concurrently (B5 headline #8).

## 6. Concurrency

| Transition | Thread / primitive |
|---|---|
| NUB_ROUND1 | IOFireWireFamily matching thread; `FWMetaNub::start` initialises the embedded kernel timer (`+0xf8`) on the nub's workload (`+0x108`) |
| helper writes | user task → `IORegistryEntrySetCFProperty(ies)` MIG into `FWMetaNub::setProperties → SetOneProperty` |
| AWAITING_RUNTIME_RESET | workloop timer (2500 ms) → `InvokeOnNewThread` worker → `registerService(0)`. That the off-workloop dispatch is *deliberate* is a **hypothesis** |
| MATCHED | IOFireWireFamily matching thread; IOAudioFamily publishes the subtree on its own threads |
| STREAM_STARTING | engine workload via `IOCommandGate::runAction`; the DCL programs are installed on the isoch channel's workloop; sleeps are `IOSleep` (R1) |
| FORMAT_CHANGE | `performFormatChange` wraps the real work in `runAction` (`0x3e54`) — single-writer invariant on `[+0x220/+0x224/+0x228/+0x2e0..+0x2f0]` |
| HSCI poll | kernel timer, `50 + [+0xd4]` ms; completion dispatched with `InvokeOnNewThread` (`0x1d89e`) (R6). "No async reads outside the HSCI window" is wrong: `updateP2PCount` reads PCRs (Q-CMP-connect §3) |
| BUS_RESET_RECOVERY | `InvokeOnNewThread` worker (keeps the FireWire workloop free during the 1100 ms sleep) |
| notifications | `FWKIPCUserClient` broadcasts fire from the thread that wrote the property (HSCI poll for `MARateLocked`; workloop for mixer/clock changes). Peak data does **not** use the Mach message (type-0 = properties, type-1 = MIDI-in; no peak/rate-lock types — Q-UC-surface §4) |
| `sNubDictionary` | no explicit mutex visible (**unverified**) |

## 7. Open questions (each names its check)

1. **Does a 2nd registration also happen on bus-reset recovery?** `BusReset`'s caller path from `message()` is unresolved (§3.10). Check: xrefs to `FW1814Device::BusReset` and to the `0xE0000030` branch of `message @0x1b9d6`.
2. **Meaning of the call to slot `0xb10` at the tail of `SetIOFormat`** (§3.9). Check: decode `SetIOFormat @0x42aa` tail against `completeConfigurationChange` (`0xb08`).
3. **Where is `takeTimeStamp` called from?** (C2 §8.3). Check: xrefs to `IOAudioEngine::takeTimeStamp` in the kext.
4. **Does `MAEnableStereoPairEngines = 1` add `FWEngine1`/pair engines only at probe time?** Check: set the key on SL and re-capture ioreg (backlog #4).
5. **Does `AppleFWAudio` claim the FW1814 on 10.13.6?** (§3.5). Check: `kextstat`/`ioreg` with the interface attached and the reference kext absent — build PC, needs the user's OK.
6. **Does terminate remove `sUserClients` entries?** Check: instrument `clientClose`/`clientDied` in the clone's build.
7. ~~Kext-side firmware protocol and `MAFirmwareInfo` layout~~ **Resolved 2026-10-14 (P3, R37): `A1-ghidra/Q-P3-kext-firmware-path.md`.**
8. Resolved (kept for traceability): the 100 ms stream-kick primitive is `IOSleep(500)` ×3 in `ConnectAudioMIDIChannels` (R1); the kext has no gap-count fix for runtime-mode devices — its only bus reset is in `FirmwareUpdate` (B2 Correction); peak data is not a Mach-message type (Q-UC-surface); the meter page is written by `ReceiveControlPacket @0xe9ea` (R6).

## 8. Clone-build implications (D2 — chosen 2026-10-14)

- Implement every state and edge in §4. Sources per area: Q7 (nub, firmware states, timers), Q3 (`initHardware`/engine), Q4 (reconnect), Q15c (format change), Q-CMP-connect (CMP), R1 (start chain).
- **Mirror the two-round lifecycle (§1) and make all `free()`/destructors NULL-safe** — the only lifecycle change forced by 10.13.6.
- Re-implement the two off-workloop dispatches (`RegregisterDeviceThread`, `ReconnectP2P`) so the isoch/FireWire workloop does not stall during the 2500 ms re-register and the 1100 ms reconnect sleep.
- The clone ships a FW1814 go-away personality for `AppleFWAudio` (reference omits it, Q8).
- Gate rate changes while streaming (§3.9 caveat).
- The helper that drives this lifecycle (watch the nub, read `MAFirmwareInfo`, write the three properties) is part of the build (`docs/BUILD-PLAN.md` M-FW, H.*).

## Change log (corrections folded into this version, 2026-10-14)

Power-cycle timeline (R25, capture 73) → §3.4. Runtime stream-start chain and sleep sites (R25, capture 72) → §3.7. Capture reading: five `IOAudioControlUserClient`s (R16) → §3.5. Stream-kick order, slot names, ReconnectP2P/BusReset, FirmwareHalt, HSCI threading, DCL opcodes/ring shape (R1, R2, R4, R5, R6) → §3.7–§3.11, §6. Device class chain Rev1 (Q16) → §3.5. Two-round lifecycle + NULL-delete panic (R31) → §1. Breaker-#3/vtable-drift text removed (Q-SDK); old §7 D1/D2/D3 analysis replaced by §8 after the D2 decision (R34). Firmware ordering (Halt / UpgradeImage / Start) corrected to the three-pass logic of A4-H2. 2026-10-14 consolidation per BUILD-PLAN P5; removed text is in git history.
