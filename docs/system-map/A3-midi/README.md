# A3 MIDI plugin — resume index

Working directory for Part A3 — the CoreMIDI driver plugin `MAudioFireWireMIDI.plugin` (universal `x86_64` + `i386`, 88 KB on x86_64 slice, Objective-C++ / Cocoa + CoreMIDI).

**Target binary:** `_extracted/midi/MAudioFireWireMIDI.plugin/Contents/MacOS/MAudioFireWireMIDI` (x86_64 slice at `_extracted/midi/MAudioFireWireMIDI.x86_64-slice` for Ghidra import; i386 slice at `_extracted/midi/MAudioFireWireMIDI.i386-slice` for cross-check with 32-bit panel).

**Bundle identity:**
- `CFBundleIdentifier = com.m-audio.midiplug.fw410` (one plugin for the whole FireWire family — covers FW1814 too).
- `CFPlugInTypes[ECDE9574-0FE4-11D4-BB1A-0050E4CEA526]` = `kMIDIDriverTypeID` (standard CoreMIDI driver UUID). Factory UUID `29F28FC6-B0F8-11D7-B850-000393942016` → `NewFW410MIDIDriver`.
- `CFPlugInDynamicRegistration = NO` — static registration.
- Build string in binary: `FireWireBeBoB_01.11.002/Source/MIDIPlugin/../Shared/IOKitCocoa/MAServiceNotification.mm` — confirms the "BeBoB" family codebase.

**Toolchain:** Ghidra 12.1.4 headless, project under `ghidra-project/midi-plugin`, exports under `ghidra-exports/`. Scripts in `scripts/` reuse the A2 template pattern (write to project-local `ghidra-exports/`, not A1/A2's).

## Architecture at a glance (from exports + strings)

**CoreMIDI C ABI boundary** (`__Z*MIDIDriver*` symbols — std CoreMIDI driver entry points):
`MIDIDriverFindDevices`, `MIDIDriverStart`, `MIDIDriverStop`, `MIDIDriverSend`, `MIDIDriverFlush`, `MIDIDriverMonitor`, `MIDIDriverConfigure`, `MIDIDriverEnableSource` → all stubs that forward into the C++ glue.

**C++ glue** (bridges C ABI to Obj-C):
- `MAFWMIDIPluginGlueBeBoB` (vtable at `0xaa70`) — `C1`/`C2`/`D0`/`D1`/`D2`, `FindDevices`, `Start`, `Stop`, `Send`.
- Base `MIDIDriver` (vtable at `0xaa10`) — the Carbon-era C++ MIDI driver base.

**Objective-C layer** (`_OBJC_CLASS_$_*`):
- `MAFWMIDIFactoryBeBoB` — holds `fMIDIDriver`; selectors `instantiateDriver:withService:withMIDIDevice:`, `fetchMIDIDeviceForService:`, `deviceFromIOKitDevice:connection:`.
- `MAFWMIDIDeviceBeBoB` — per-device state: `fDevice`, `fDriver`, `fKitDevice`, `fAsyncNotification`, `fDataInputTimer`, `fSourcePorts`/`fDestinationPorts`, `fSourceEndpointArray`/`fDestinationEndpointArray`, `fSourceEndpointCount`/`fDestinationEndpointCount`, `fDestPortInSysex`, `fDestPortCurrentTimestamp`, `fStatusAsleep`, `fStatusRunning`.
- `MAFWBeBoBAsyncNotification` — Mach-port async channel: `fNotificationIOKitPort` (IONotificationPort), `fNotificationCFMachPort`, `fMachPortRunLoopSource`, `fRunLoop`, `fTarget`, `fAction`. Selector `userClientNotificationOfType:connection:runLoop:target:selector:` — maps to IOKit `IOConnectSetNotificationPort`.
- `MAFWBeBoBServiceNotification` — service add/remove: `observeServiceMatched:target:selector:`, `observeServiceTerminated:target:selector:`, `observeServiceFirstMatched:target:selector:`.
- `MAFWBeBoBPowerNotification` — `observeSleepNotifications:`.
- `MAFWBeBoBIOKitService` — holds `fConnection` (`io_connect_t`); selectors `openConnection`, `closeConnection`.
- `MAFWBeBoBIOKitRegistryNode` — `fEntry` (`io_registry_entry_t`); `propertyForKey:`, `setProperty:forKey:`.

**Protocol / packet layer** (`com_m_audio::*`):
- `TMIDIParser::ParseMIDIByte(uint8, &state, &runningStatus, &inSysex, &sysexSize, &packet, &out)` — byte-level MIDI stream reconstruction.
- `TMIDIParser::ReceiveMIDIByte(uint8, &inSysex, &state)`.
- `TMIDIPort::CheckForNewBytes(MIDIMemVector&, uint32)`.
- `TMIDIPort::CheckForNewRealtimePacket(DigiFWMIDIPacket*&)` — "Digi" = Digidesign/Avid, inherited name; the on-wire packet format we need to decode.
- `TMIDIPort::CheckForNewTimestampedPacket(DigiFWMIDIPacket*&, uint64, uint64)`.
- `TUserMIDIPort::Init(uint32, bool, uint32)`.
- `TCoreMIDIDriverPort` encoded struct (from Obj-C type encoding): `^MIDIQueue, uint32, ^MIDIPacketQueue, ^MIDIPacketQueue, TMIDIParser{uint32, MIDIPacket, MIDIPacket, bool}`.

**IOService matching (strings):** `com_m_audio_FW410Device`, `com_m_audio_FW1814Device` — the plugin walks the registry for both device classes (confirms the plugin's device-side symmetry with the kext IOService hierarchy decoded in A1).

**Kext-published MIDI properties (strings):** `MAMIDIInputPorts`, `MAMIDIOutputPorts` — endpoint counts come from `IORegistryEntryCreateCFProperties` on the matched service. Icon: `MAIconName` NSString key resolved to a file under `/Library/Application Support/M-Audio/DeviceIcons/`.

**Exported extra entry points** (`_MAFireWireMIDIDICE_Send`/`_Start`/`_Stop`): DICE-family stubs in the same binary — shared codebase with M-Audio's DICE products (ProFire 2626 etc). Not called by BeBoB path. **Hypothesis:** present only because the SVN tree built one plugin binary for both chip families — **confirm by checking whether any factory UUID in the plist references them (none visible so far).** Flag before using as evidence of anything.

## Q queue (seed)

| # | Question | File | Status |
|---|---|---|---|
| M1 | CoreMIDI factory + driver instantiation path: `NewFW410MIDIDriver` → `MIDIDriverInterface` vtable → C++ glue → `MAFWMIDIFactoryBeBoB` | `M1-factory-and-driver-interface.md` | done — glue thunks to `_MAFireWireMIDIDICE_Start`; factory singleton `sMIDILoader`; `kMIDIDriverInterface2ID`; 6 BeBoB IOService classes observed |
| M2 | Device discovery + user-client open: `MAFWBeBoBServiceNotification` matching on `com_m_audio_FW1814Device`, `openConnection`, endpoint-count read of `MAMIDIInputPorts`/`MAMIDIOutputPorts` | `M2-device-discovery.md` | done — GUID-persisted MIDIDevice; one entity/device; type=0 UC; refcount-driven teardown |
| M3 | Async notification receive path: `MAFWBeBoBAsyncNotification` → `IOConnectSetNotificationPort` → CFMachPort → runloop source → delivered selector. Compare against A2-P2 panel pattern. | `M3-async-notifications.md` | done — class is fully scaffolded but **unused by the MIDI data path** (xref: `fAsyncNotification` ivar has only READ refs in start/stop; RX is pure 1500 Hz polling) |
| M4 | `DigiFWMIDIPacket` on-wire layout: decode `TMIDIPort::CheckForNewRealtimePacket` + `CheckForNewTimestampedPacket` + `CheckForNewBytes`. This is the kext↔plugin shared struct that governs the MIDI shm/pipe. Cross-reference A1-Q11 selector for MIDI (and the type-1 shm at `FWAudioDevice+0x100` noted in P1). | `M4-wire-packet-layout.md` | done — 16-byte packet `{u64 ts, u32 len, u8 data[4]}`; two 128-slot rings per TX port at offsets +0x10 (timestamped) and +0x18 (realtime); RX byte ring 512 B; memType = `(dir<<10) + portIdx`, dir=1 RX / dir=2 TX |
| M5 | TX path: `MIDIDriverSend` → `MAFWMIDIPluginGlueBeBoB::Send` → how bytes land in the kext (shm write? user-client scalar method? async port?). | `M5-tx-path.md` | done — scalar selectors **500**=startMIDI / **501**=stopMIDI; `send:viaPort:` chunks packets to ≤3 bytes, dispatches to realtime or timestamped ring by host-time compare, 125/128-slot backpressure with 100×10ms stall |
| M6 | RX path: `fDataInputTimer` cadence + `checkForMIDIInput` selector — polled read of the kext shm/queue, feeding `TMIDIParser::ParseMIDIByte` → `MIDIReceived` to CoreMIDI. | `M6-rx-path.md` | done — 1500 Hz polled drain per source endpoint; TMIDIParser is the single source of truth for ts-vs-rt classification; kext just appends raw bytes to the 512-byte ring; `do {} while (parserReturnedOne)` re-feeds the same byte on status-interrupt |
| M7 | Endpoint model: `fSourcePorts`/`fDestinationPorts` arrays, how many endpoints per FW1814 (expect **2 in / 2 out** for FW1814 per spec — verify via `MAMIDIInputPorts`/`MAMIDIOutputPorts` on a live device OR decode default fallback in plugin). | `M7-endpoint-model.md` | done — one entity per device, counts from kext `MAMIDIInputPorts`/`MAMIDIOutputPorts`; FW1814 expected **1 in / 1 out** (correction to README's 2in/2out guess; see M7 §4, unverified pending ioreg); 6 parallel per-port arrays allocated by `rebuildEndpointArray` |
| M8 | Sysex handling: `fDestPortInSysex`, `fDestPortCurrentTimestamp` — how multi-packet sysex is reassembled on both sides. | `M8-sysex.md` | done — RX uses parser `remaining=0xFFFFFF` sentinel + 256-byte scratch overflow → synthetic `0xF7` flush; realtime (`0xF8..0xFF`) interleaves without disturbing sysex state; TX `fDestPortInSysex` is set/cleared but **never read** by the plugin (kext consumer hypothesis — deferred to A1 Q11-MIDI) |
| M9 | Sleep/wake: `MAFWBeBoBPowerNotification` + `fStatusAsleep` + `setMIDIDevice:offline:` — what the plugin does across sleep, and whether the kext teardown surfaces as service-terminate or just an offline flag. | `M9-power-sleep.md` | done — `IORegisterForSystemPower` on three msgs (0x270/0x280/0x300); fan-out via factory thunk to each device's `systemWillSleep`/`systemWokeUp`; sleep ≠ offline (sleep is `stop`+`fStatusAsleep=1`, keeps `kMIDIPropertyOffline=NO`); offline is service-terminate-only |
| M10 | DICE exports (`_MAFireWireMIDIDICE_Send/Start/Stop`) — dead code or alternate path? | *closed by M1 §5–§6* — **NOT dead code**: the glue `::Start/Stop/Send` thunk directly to them. Name is historical (SVN tree shared BeBoB + DICE scaffolding). Clone can drop the DICE prefix. | done |

## Cross-references into prior parts

- **A1-Q11** (`A1-ghidra/Q11-userclient-selectors.md`) — kext user-client selectors. MIDI side has shm type 1 at `FWAudioDevice+0x100` (per A2-P1). Selector `0x1fa` is a vestigial ProTools no-op (per P9), **not** a MIDI-related call.
- **A2-P2** (`A2-panel/P2-notification-receive.md`) — same `IOConnectSetNotificationPort` pattern on panel side; only `bodyType==0` is handled there. M3 should check whether MIDI uses a different `bodyType` or the same selector→selector-dispatch model.
- **A2-P1** (`A2-panel/P1-device-discovery.md`) — panel's `IOServiceOpen` path on `com_m_audio_FW1814Device` → `com_m_audio_FWKIPCUserClient`. MIDI plugin may open a different user-client subclass (its own `MAMIDIUserClient` **hypothesis** — verify via decode) or share the panel's user-client.

## How to resume

1. Pick the next unblocked `M*` from the table.
2. Import x86_64 slice into Ghidra (`_extracted/midi/MAudioFireWireMIDI.x86_64-slice` → project `midi-plugin`).
3. Export class/function/symbol tables with `ExportClasses.java` / `ExportSymbols.java` first (one-time pass).
4. Add scoped decompile script under `scripts/`, run with headless analyzer, log to `ghidra-exports/decomp/<name>.log`.
5. Write `M<n>-<topic>.md` with pseudocode + kext cross-reference + clone implication.
6. **Cite every claim** (log file:line + function/offset, or mark `**hypothesis**`/`**unverified**`). See `memory/feedback_no_unmarked_guesses.md`.

## Known binary quirks

- **Universal binary** — primary analysis target is x86_64; i386 slice exists for cross-check (same codebase; shim differs only in word size).
- **Objective-C runtime encoding** of structs appears in `__objc_methtype` — use this to recover `TCoreMIDIDriverPort` / `DigiFWMIDIPacket` layouts.
- **ARC + Obj-C++** — expect `objc_retain/release/autorelease` wrappers around every heap handoff, plus `_objc_retainedObject`/`_objc_unretainedObject` C++ bridges.
- **CFRunLoop + CFMachPort** — notification delivery is on the thread that called `MIDIDriverFindDevices` (CoreMIDI's `MIDIServer` runloop).
- **DICE exports present but likely unused** — `_MAFireWireMIDIDICE_*` are not referenced by the BeBoB factory UUID; treat as dead code until M10 confirms.

## Status

A3 scaffolded 2026-10-06. Recon pass done (plist, exports, strings, class landscape). P-queue seeded with 10 Ms — **all 10 done 2026-10-06**. M9 was decoded via direct `otool -tv` disassembly (Ghidra unavailable in-session); every other M is cited against `ghidra-exports/decomp/a3-*.log` logs.

**Carryovers to A1 Q-queue (kext side):**
- Kext `clientMemoryForType` must honor memType `0x400|idx` (RX) and `0x800|idx` (TX) for every published port — M4 §1, M7 §7.
- Kext MIDI user-client selector surface: **500 = startMIDI**, **501 = stopMIDI** — scalar, 0-in/0-out, `kIOReturnBusy` tolerated with 60 s plugin retry — M5 §1, M9 §9.
- Kext must preserve `IOMemoryDescriptor` identity across stop/start to survive sleep — M9 §9.
- Open: whether the kext consumer reads `fDestPortInSysex` for wire scheduling — M8 §4.

**Open items worth re-reading before Part C synthesis:**
- `MAMIDIInputPorts` / `MAMIDIOutputPorts` live values on FW1814 (M7 §4, M7 §9).
- Factory → device fan-out selector resolution (M9 §4, §10).
- `IOConnectMapMemory` identity across sleep (M9 §6, §10).
- The byte-ring vs. timestamped-ring overlap in `TUserMIDIPort::Init` (M4 §2, §8).

Next: either (a) proceed to Part B (FFADO / snd-bebob / Snow Leopard DTrace cross-reference), or (b) backfill the open items above with live DTrace captures from the SL reference Mac. Per `NEW_PLAN.md`, B is the normal next step.

## Q-MIDI-WIRE check (2026-10-14, BUILD-MANUAL §2.H)

A3 is consistent with the kext-side decode in `A1-ghidra/Q-UC-surface.md`:
- **Shared rings.** Kext `clientMemoryForType` (`0x7220`) serves `0x400|n` (read-only, RX/source ring) and `0x800|n` (writable, TX/destination ring). This matches A3-M4/M7's `IOConnectMapMemory(0x400|0x800 + idx)`.
- **Start/Stop.** Selectors 500/501 are `kIOUCScalarIScalarO` with 0 in / 0 out in the kext's legacy `IOExternalMethod` table. This matches A3-M5's `IOConnectCallScalarMethod(conn, 500/501, NULL, 0, NULL, NULL)`.
- **Unused by the plugin.** Selector 503 `EntrySendMIDI` (StructIStructO, 280 B in → 4 B out) and the type-1 MIDI-in notification port (`FWUserClient::registerNotificationPort`, ≤320 B messages) exist in the kext but are **not used by the v1.11.x MIDI plugin**. It sends via the `0x800` ring and polls the `0x400` ring (A3-M5/M6). **Unverified:** which client (if any) uses 503 / type 1. Check: DTrace on the SL rig while sending MIDI from a DAW.
- **Gaps.** No gaps found that block M-H. The clone kext must still implement 503 and type 1 for parity.

