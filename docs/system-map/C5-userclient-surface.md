# C5 — User-client surface (Panel / Prefpane / MIDI plug-in / firmware helper → kext)

**Status:** consolidated 2026-10-14 (task P5 of `docs/BUILD-PLAN.md`). The three 2026-10-14 Correction blocks that sat on top of this file and the stale body text they contradicted (KIPC "dead code", 506/507 "ProTools no-ops", "30 Hz kext pump", dirty-dictionary memory map, 3-region meter layout) are folded in; details of the change are in the log at the end. The 2026-10-13 original is in git history (`git show a9cb63c:docs/system-map/C5-userclient-surface.md`). Supersedes the archived `docs/12-USERCLIENT-DESIGN.md`.

**Authoritative evidence for this document:** `A1-ghidra/Q-UC-surface.md` (selector table, meter page, notifications) and `Q-MAMAP-key-crosswalk.md` (property surface). Bundle: kext 1.11.3 (`otool-exports/mafw.asm`); "SL" = Snow Leopard rig, driver **v1.9**. Scope: UC *surface* only — transport = C4, clock bits = C2, wire bytes = C3, lifecycle = C1.

**Provenance rule:** every claim cites A1/A2/A3/A4/A6/B5 or a kext offset; hypotheses carry **unverified** and a check.

## 0. Headline

Userspace reaches the kext through **four** mechanisms: (a) `IORegistryEntrySetCFProperties` on `com_m_audio_FW1814Device` (the whole `MA*` control plane); (b) the **legacy `IOExternalMethod` table** of `com_m_audio_FWUserClient`, selectors 500–507; (c) `clientMemoryForType` (meter page, MIDI rings); (d) Mach-port notifications registered with `IOConnectSetNotificationPort` (type 0 properties/IOKit messages, type 1 MIDI-in). There is **one** user-client class chain: `FWUserClient` derives from `FWKIPCUserClient` (live, not dead code). The vendor UC is multiplexed (Panel, helper, ≥1 MIDIServer each hold their own connection). The prefpane opens nothing.

## 1. Class inventory

| Class | Role | Evidence |
|---|---|---|
| `com_m_audio_FWUserClient` | the user client named by each product personality (`IOUserClientClass`, Q-INFO); selector table, `clientMemoryForType`, type-1 MIDI-in port, `WaitForTermination`/`SetInitializedProperty` | `MetaClass::alloc @0x752f` allocates `0x298` bytes and runs `FWKIPCUserClient::FWKIPCUserClient` (`0x7554`); vtable `0x33c50` |
| `m_audio_b_FWKIPCUserClient` (base) | type-0 notification port, dirty-property collection (`AddModifiedProperties @0x1620c`), `SendChangedPropertiesMessage @0x1602a`, `message() @0x15fca`, static `sUserClients` OSSet | the `FWUserClient` vtable contains `FWKIPCUserClient::AddModifiedProperties` at slot `0x960` |

The 2026-10-13 hypothesis that `FWKIPCUserClient` is "dead compiled code" (because B5's live ioreg lists only `FWUserClient` instances) is **refuted**: those instances *are* `FWKIPCUserClient`s through inheritance. IOAudioFamily's own user clients (`IOAudioControlUserClient` ×5 and `IOAudioEngineUserClient` per streaming client; B5 §3/§6a, `40-ioreg-playback.txt:4287-4324`) are the HAL stream plane and out of scope here; the vendor UC and those are decoupled (a rate change through `performFormatChange` touches no `MA*` property, B5 §6c).

## 2. Selector table (`getTargetAndMethodForIndex @0x72f6`)

`sel − 500 < 8` indexes `sMethods` (legacy `IOExternalMethod` records `{object, func(16-byte PMF), flags, count0, count1}`, 48-byte stride, `__ZN24com_m_audio_FWUserClient8sMethodsE = 0x339e0`) provided the device pointer `[this+0x290]` is non-NULL (else the call is rejected). Flags per `IOUserClient.h:47–51` (10.13 SDK): `kIOUCScalarIScalarO = 0`, `ScalarIStructO = 2`, `StructIStructO = 3`, `ScalarIStructI = 4`.

| Sel | Function | Type (count0/count1) | Semantics | Caller in v1.11.x |
|---|---|---|---|---|
| 500 | `EntryStartMIDI @0x732c` | ScalarIScalarO (0/0) | gates `ActionStartMIDI → CoreStartMIDI` onto the workloop; returns `kIOReturnBusy` until isoch is ready | MIDI plug-in `[driver start]` (A3-M5/M9; retries ≤ 60 s) |
| 501 | `EntryStopMIDI @0x7398` | 0/0 | gates `ActionStopMIDI → CoreStopMIDI` | plug-in `[driver stop]` and on sleep |
| 502 | `EntryStopMIDI` (alias) | 0/0 | same handler | no shipping client found (A2/A3/A4) |
| 503 | `EntrySendMIDI(MAMIDIPacket*, uint32_t*) @0x73fa` | StructIStructO, 0x118 in / 4 out | per-packet MIDI TX | **not called by the v1.11.x plug-in** (it writes the `0x800+n` ring, A3-M5) — callers elsewhere unverified |
| 504 | `EntryDiceSetRequest(MAFireWireDiceRequest*) @0x746a` | ScalarIStructI, 0 / variable | transport-agnostic mixer/levels RPC (pre-property-dict) | **unverified** — no caller found in panel/prefpane/plug-in |
| 505 | `EntryDiceGetRequest(req*, void*) @0x7476` | StructIStructO, variable / variable | same | **unverified** |
| 506 | **virtual `vt[0x968]` = `FWUserClient::WaitForTermination() @0x761e`** (PMF `0x969` = vtable offset + 1) | 0/0 | lazily creates a Mach semaphore at `[+0x288]` (`semaphore_create`) and **blocks in `semaphore_wait`**; `FWUserClient::stop @0x766c` destroys the semaphore, releasing the waiter; returns 0 | the Panel's `CXDevice::TerminationWaitThread @0x6d108` (A2-P9 §7.3): returns when the UC is torn down, then the thread nulls its shared-memory pointer and exits |
| 507 | **virtual `vt[0x970]` = `FWUserClient::SetInitializedProperty() @0x71f6`** | 0/0 | `device->setProperty("MADeviceInitialized", 1)` (`vt[0x278]`, `0x7209..0x7215`); returns 0 | not called by the Panel/helper in the decoded paths — **unverified** who calls it |

Notes: 506/507 have no relocation at their `func` slots (entries 500–505 do, `0x339e8…0x33ad8`), which is how they are recognised as virtual-PMF values (Q-UC-surface §2); `EntryProToolsOpen @0x7482` and `EntryProToolsErrorReport @0x748a` exist but are **unreachable** from `sMethods`. The earlier statements that 506 is a `xor eax,eax; ret` no-op that "MUST remain a no-op or the Panel hangs" are **wrong**: 506 is a real blocking wait, and the Panel's termination thread depends on that behaviour (it must block until the UC stops, not return immediately). The older `Dice*` ScIn/StIn column values in A1-Q11 applied `IOExternalMethodDispatch` semantics to a legacy table and are superseded by the table above.

Clone: implement 500–502 and 506/507 faithfully; 503–505 as the same entries (stubs returning an error are acceptable only after a caller search, or keep the reference behaviour, which is binary-compatible with old Panels).

## 3. Notification ports (`IOConnectSetNotificationPort`)

- **Type 0** (`FWKIPCUserClient::registerNotificationPort @0x15f3a`): fills a 44-byte template at `this+0xe0` (`msgh_bits 0x13`, `msgh_size 0x2c`, remote port, local 0, type at `+0x20`, refCon at `+0x24`, dirty counter `[this+0x118]` at `+0x28`; kernel header is 32 bytes on the 64-bit kernel). Two message kinds: **property change** — `SendChangedPropertiesMessage @0x1602a` (called only from `FWPropertyDict::SendPropertyChangedMessagesToUserClients`) serializes the modified-properties dictionary into an IOBMD of length `0x2c + len`, copies the template in front, sets `msgh_size` to the total and type `0`; the XML therefore rides **in the message**; **IOService message** — `message() @0x15fca` sends the template with the IOKit message code at `+0x20`, then chains to `IOUserClient::message`.
- **Type 1** (`FWUserClient::registerNotificationPort @0x74ab`): MIDI-in push; template at `this+0x148` (size 320, port `+0x150`, refCon `+0x16c`); `ReceivedMIDIPacket @0x757f` → `SendMachMessage(1, …)`, size `min(len + 0x40, 0x140)`. The v1.11.x plug-in does **not** register it (it polls the rings). Registrant **unverified**.
- Any other type → `IOLog("M-FW: Someone registered for an unknown message type:%X\n")` and `0xE00002C2`. There are **no peak or rate-lock body types** (A2-P2 retracted the three-type draft on the panel side on 2026-10-06; Q-UC-surface §4 confirms on the kext side).
- **Userspace offsets** after the kernel→user header conversion are **unverified**; A2-P2 reads the discriminator at user offset `+0x18` (consistent with a 24-byte user header).
- **Dirty collection** (per connection, A1-Q14): lazy `OSDictionary` `[+0x110]`, `IORecursiveLock` `[+0x120]`, dirty counter `[+0x118]`, IOBMD/`OSSerialize` `[+0x128]/[+0x130]`. **Fan-out:** static `sUserClients` OSSet — populated via `UserClientAdded(this)` in `start()` and drained via `UserClientRemoved(this)` in `stop()` (A1-Q14); every live UC (Panel, helper, MIDIServers) receives each property-change message.
- **What the Panel does on a type-0 message** (A2-P2 Dispatch): it ignores the body, calls `IORegistryEntryCreateCFProperties` for the device, diffs against `CXDevice[+0x6c]`, and calls the handler registered for each changed key (`CXDevice[+0x74]`). **The helper** opens its own UC per device, subscribes to the same port, merges changes and writes them to CFPreferences ~4 s later (A4-H11); the SL helper also re-applies the saved settings after a restart (capture 71).

## 4. `clientMemoryForType @0x7220` and the meter page

Accepted types: **1** → device `[+0x100]` (options `0x1000`); `0x400 + n` (n < `[eng+0x29c]`) → MIDI RX ring of source port n (read-only, `0x1000`); `0x800 + n` (n < `[eng+0x2a0]`, capped at 8) → MIDI TX ring of destination port n (writable, options 0). **Everything else, including type 0, returns `0xE00002C7`** — so there is no dirty-dictionary memory map and the earlier "type-0 BMD handoff" open question is closed (§3: the XML is in the message). `0x1000` = `kIOMapReadOnly` (constant name **unverified**, check `IOTypes.h`); the Panel's map option `0x1001` = `kIOMapAnywhere|kIOMapReadOnly`.

**Meter page.** `FWAudioDevice::initHardware`: `[+0x100] = IOBufferMemoryDescriptor::withOptions(0x10000, 0x1000, 0x1000)` (`0x3598..0x35a7`, one 4 KiB page; `0x10000` = `kIOMemoryKernelUserShared`, name **unverified**) + `prepare` (`0x35c3`); `[+0x108] = getBytesNoCopy` (`0x3611`); `[+0x110] = new uint32_t[GetPeakLevelCount()]` (`0x3624..0x3635`, a private array — the NULL-safe-delete subject of C1 §1). **Layout: `uint32_t peak[52]`** (`GetPeakLevelCount() = 0x34` @`0x1d30`, not overridden by FW1814); each slot holds a 16-bit value; **no header, generation counter or writer index** — a plain array overwritten in place. Writer: `FW1814Device::ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*) @0xe9ea` on every HSCI poll packet (C3 §5): slots 0–37 = hardware peaks (`peak[2i] = bswap16(pkt.u16[4+4i])`, `peak[2i+1] = bswap16(pkt.u16[6+4i])`, i = 0..18, **only when the engine FDF index `[+0x228] < 5`**, i.e. ≤ 96 kHz), slots 38–51 = software-return peaks from `[+0x110]` (`GetFirstSWReturnPeakMeterIndex() = 0x26`, `GetSWReturnPeakMeterCount() = 0xe`); slots 22/23 and 24–31 are overwritten from `[+0x110]` or slots 36/37 depending on a device byte (`[+0x1317] & 4`, meaning **unverified**); at 176.4/192 kHz only slots 18–21 and 38–41 are refreshed. **The kext has no 30 Hz pump**: the 30 Hz in the old text is the *Panel's reader timer* (`CMiddleWareDevice::SpendTime @0x638e2`, `LPeriodical` ≈ 33.3 ms, A2-P9); data change at the HSCI cadence (`50 + [+0xd4]` ms). Rev2's `UpdatePeakMeters @0x12aae` is not the FW1814 path (FW1814 is Rev1).

**Panel side** (A2-P9 §7.5, `CXDevice::CXDevice @0x6c2b8`): `IOConnectMapMemory(conn, 1, mach_task_self, &CXDevice[+0x68], &sz, 0x1001)` once at construction; unmapped in the destructor; not rebuilt on sleep/wake or rate change. `CX1814Device::GetSWReturnPeaksAreEmbedded @0x72e86 = 0` → the Panel interprets the page as `[inCh | outCh | swRetCh]` regions. The old inference `(18 + 14 + 4) × 4 = 144 B` is superseded by the kext-side 52-slot page; **how the Panel's channel regions map onto the 52 slots is not reconciled** (check: A2-P9 reader vs `ReceiveControlPacket`). B5 cap 70: 30 s of animated meters produced zero IOKit MIG calls; the Panel's `vmmap` shows three `SM=SHM` regions (16 K, 20 K, 4 K) — attribution of the other two is **unverified**. Not in the page: sample-rate state, MIDI data (own rings), rate-lock (the `MARateLocked` property).

## 5. Property control plane

One `IORegistryEntrySetCFProperties(deviceEntry, dict)` per UI commit (B5 §10); the Panel queues edits in `CXDevice::Set32BitNumberProperty` (`dirtyKeys` + `cachedPropertyDict`) and flushes in `SendQueuedProperties @0x6db2c` on slider mouse-up/popup selection. Kernel path, gate semantics and `MARateLocked`: **C3 §9**; clock bits: **C2 §2**. `MAClockSource` needs bit 30 set (else `0xE00002C2`); `MASampleRate` is the only fast-pathed key and is un-gated in the reference — the clone gates it. **Selectors carry no control-plane writes.**

**Key surface.** The kext side is documented per key in **Q-MAMAP** (43 table rows + read-only descriptors `MAActualProductName`, `MAIconName`, `MAMainEngineUniqueID`, `MAMaxBytesPerCycle`, `MABytesPerCycle`, `MAMIDIInput/OutputPorts`, `MADeviceInitialized`, `MAFirmwareInfo`, and the three firmware write keys on `FWMetaNub`). The Panel side (A2-P3) binds 21 handlers (`MASWReturn/Input/Output/Headphone/AuxOutput/AuxSend Levels`, the Solo/Mute bitmasks, `MAMonitorAuxOnOutput`, `MASWReturnRoutingV2`, `MAInputRoutingV2`, `MAHeadphoneSourceV2`, `MAClockSource`, `MARateLocked`, `MARotaryControlV2`), 3 constructor keys (`MAMetaAllProperties`, `MAFirmwareInfo`, `GUID`) and pref-visible keys with no handler (links, pans, `MAHeadphoneSecondSource`, `MABytesPerCycle` — stale cosmetic, `MASampleRate`, `MADirectMonitoringActive`, `MAEnableAggregateEngine`, `MAEnableStereoPairEngines`, `MAGlobalDim/Mute`, `MAMomentarySwitchRoutings/Source`, `MAMIDIInput/OutputPorts`); the Panel binary's string table counts 46. **The two lists are not reconciled into one authoritative set** (check: `strings` of the Panel and kext against Q-MAMAP). The old "opcode byte" table (`0x3e…`, `0x90`, `0x9c` passed to a "vendor opcode" `vt[0xba8]`) is retracted: those are **register offsets** written by `CustomMixerWriteQuads` (C3 §3). Keys the FW1814 kext does not register (`MASPDIFMixerBypass`, `MASPDIFMirrorToAnalog12`, `MAChannelOrderSPDIFFirst`) are known to the Panel's shared translator only.

## 6. MIDI surface (A3)

Plug-in call sites: `[MAFWMIDIDeviceBeBoB start]` → `IOConnectCallMethod(conn, 500)`; `stop`/sleep → 501; TX → `IOConnectMapMemory(conn, 0x800|portIdx)` and direct writes into the ring; RX → 1500 Hz `CFRunLoopTimer` reading `IOConnectMapMemory(conn, 0x400|portIdx)`; device open → `IOServiceOpen(fw1814DeviceRef, mach_task_self, 0, &conn)` → `newUserClient`; endpoint counts from the `MAMIDIInputPorts`/`MAMIDIOutputPorts` properties; add/remove via IOKit matching/interest notifications on `com_m_audio_FW1814Device`. `MAFWBeBoBAsyncNotification` is scaffolded but unused on the data path (A3-M3). The factory UUID `29F28FC6-B0F8-11D7-B850-000393942016` is CFPlugIn registration only.

Ring layout (kernel side, `TKernelMIDIPort::Init @0x1e64a`, R11): per port one 8 KiB `IOBufferMemoryDescriptor::withOptions(0x10003, 0x2000)`; ring headers `{head, tail}` at mapping offsets **+0x000** (timestamped ring) and **+0x808** (realtime ring); rings are `{uint32 writeIdx; uint32 readIdx; 128 × 16-byte slot {u64 hostTimeNanos; u32 length; u8 data[4]}}`; RX byte stream `{uint32 writeIdx; u8 bytes[512]}` (A3-M4); port objects 40 bytes, default names `"1"`, `"2"`, …. The old `+0x010` offset was an object-field index, not a byte offset. FW1814: 1 MIDI in + 1 out (A3); each MIDIServer connection gets its own mappings (two MIDIServer pids observed, B5 §6c).

## 7. Helper surface (A4-H1/H8b/H11/H12)

The helper imports `IOServiceOpen` (`+[MAFireWireDeviceInstance deviceInstanceForService:] @0x23a0`, `-[MAIOKitService openConnection] @0x4032`) and `IOConnectSetNotificationPort` (`-[MAAsyncNotification initWithType:…] @0x4951`, call `0x4ac4`); **no** `IOConnectCallMethod`, `IOConnectMapMemory`. It opens the vendor UC purely as a notification sink (and creates **two** instances per start, A4-H11). Firmware control is by registry-property writes on the nub (C1 §1, C3 §8), not through the UC. The helper is **not** on the Panel's control-plane path (B5 §3 refutes a "helper proxies Panel" idea).

## 8. Prefpane (A6)
Opens no user client, holds no `io_connect_t`, reads no `MA*` property; it launches the Panel (`[NSWorkspace launchAppWithBundleIdentifier:]`). No kext-side prefpane code path.

## 9. Per-client lifecycle

| Client | UC | memTypes | Selectors | Opens | Closes |
|---|---|---|---|---|---|
| Panel (`com.m-audio.appc.MAfw`) | `FWUserClient` ×1 | 1 (meter page, persistent) | 506 (termination thread) | `CXDevice` ctor | `CXDevice` dtor / device removal |
| Helper | `FWUserClient` ×2 per start (A4-H11) | — | none | when the product class matches (`driverInstanceMatched:`) | process exit (the reference never terminates the UC: see below) |
| MIDI plug-in / MIDIServer | `FWUserClient` ×1 per device per server | `0x400|n`, `0x800|n` | 500, 501 | `initWithDevice:driver:service:` after matching | `terminateInstanceOfDevice:` → `IOServiceClose`; sleep → 501, mappings kept (A3-M9) |
| Prefpane | — | — | — | never | — |

Vendor-UC traffic is quiescent across stream states; it exercises only on device attach/removal, any `MA*` write (broadcast to every live UC), and sleep (plug-in stops MIDI; mappings stay live, A3-M9). **Leak:** `FWUserClient::clientClose @0x71a8` only clears `[+0x150]` and stops MIDI if `[+0x140]`; `FWKIPCUserClient::clientClose @0x15c38` only zeroes `[+0xe8]`; there is no `clientDied`/`terminate` override (v1.9 binary: 0 occurrences of `terminate`). On SL, UCs of dead pids stay registered (`active`, retain 5–6) until the device instance goes away (capture 73: they vanished with the old device). A correct clone calls `terminate()` from `clientClose` — a **deliberate deviation** that needs the user's sign-off (backlog A1-Q-UC.6).

## 10. Multiplexing and concurrency

Per-connection state: `fAttached`/device pointer `[+0x290]`, dirty dictionary + lock + serializer (`[+0x110..+0x138]`), Mach template `[+0xe0..+0x108]` (type 0) and `[+0x148..+0x16c]` (type 1), the `0x400/0x800` mappings, the MIDI semaphore `[+0x288]`. Per-class state: `sMethods`, `sUserClients`. B5 §6c saw four concurrent instances (helper pid 64, Panel 172, MIDIServer 545 and 556); no cap is visible in `newUserClient`/`getTargetAndMethodForIndex` (**unverified** that none exists elsewhere). Selector bodies run on the device workloop (`TPTMFFunctor1<FWBaseEngine*>` + command gate); the meter page is written lock-free and read with tolerated tearing (A2-P9); `ApplyModifiedProperties` runs on the workloop; `sUserClients` iteration relies on that serialization (no per-UC lock beyond `[+0x120]`).

## 11. Open questions
1. Callers of selectors 503/504/505 and of 507 anywhere in `_extracted/` (dtrace `IOConnectCallMethod` userside on SL during a Panel + DAW session, or Obj-C xrefs).
2. Reconcile the Panel's `[inCh | outCh | swRetCh]` regions with the 52-slot page, and the meaning of `[+0x1317] & 4`.
3. Attribution of the other two `SM=SHM` regions; registrant of the type-1 port; userspace message offsets (§3).
4. Constant names `kIOMapReadOnly` / `kIOMemoryKernelUserShared` for `0x1000` / `0x10000` (check `IOTypes.h`).
5. Authoritative merged MA* key list (§5) and cap (if any) on concurrent user clients.
6. Clone-only: confirm the `terminate()`-on-`clientClose` deviation (user sign-off).

## 12. Clone-build implications (D2)
1. One UC class chain: `FWUserClient : FWKIPCUserClient`, `IOUserClientClass` set in each product personality.
2. Legacy `IOExternalMethod` table of 8 entries, selectors 500–507, semantics per §2 — **506 must block until `stop()`** (Mach semaphore) and 507 must set `MADeviceInitialized`; 503–505 keep the reference entries.
3. `clientMemoryForType`: types 1, `0x400+n`, `0x800+n` only; everything else `0xE00002C7`; meter page = 4 KiB `uint32 peak[52]` written by the HSCI packet handler (C3 §5); mirror the slot map of §4 and the ≤ 96 kHz gate.
4. Type-0 port (44-byte template; property-change messages carry the serialized XML after the header; IOKit message codes) and type-1 MIDI-in port; `sUserClients` fan-out; unknown types rejected with `0xE00002C2`.
5. Control plane = `setProperties` on the device entry (C3 §9); no selector-based control plane.
6. Support ≥ 4 concurrent connections; do not cap.
7. Deliberate deviation: `terminate()` from `clientClose` (needs the user's sign-off).
8. Prefpane: none needed.

## Change log (2026-10-14 corrections folded in)
MIDI ring layout (R11) → §6; audit items on KIPC/506/507/meter page/type-1 port (R6) and the device class chain (Rev1, Q16) → §1–§4; Q-UC-surface selector table, memory types and notifications → §2–§4; `WaitForTermination`/`SetInitializedProperty` decoded from the asm (new, this pass) → §2; helper persistence (A4-H11) and leak (Q-UC-surface Correction) → §3, §7, §9; `0xBA9`/opcode-byte wording retracted (Q15f) → §5. D1/D2/D3 wording removed (R34).
