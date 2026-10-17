# C3 — Comm path: userspace ↔ kext ↔ device wire formats and dispatch channels

**Status:** consolidated 2026-10-14 (task P5 of `docs/BUILD-PLAN.md`). The 2026-10-14 Correction blocks that used to sit on top of this file are applied to the body and listed in the change log at the end; the 2026-10-13 original is in git history (`git show a9cb63c:docs/system-map/C3-comm-path.md`).

**Sources:** A1-Q5/Q6 (HSCI), A1-Q15/Q15b/Q15f + **Q-MAMAP-key-crosswalk** (mixer, property surface), **Q-UC-surface** (selectors, rings, notifications; supersedes Q11/Q14 where they differ), A3-M4/M5/M6 (MIDI), A4-H2/H11/H12 (firmware, persistence), B1/B2/B2a/B4 (third-party cross-checks), B5 §3/§10/§11 (live topology, control plane, meter path). Bundle: kext 1.11.3 (`otool-exports/mafw.asm`); "SL" = Snow Leopard rig, driver **v1.9** (version-skew rule).

**Scope:** every byte, selector, Mach message, shared-memory ring, FCP/async transaction and `setProperties` write that moves data between Panel / helper / CoreMIDI plug-in / coreaudiod and the kext, and between the kext and the device. Lifecycle = C1, sync decode = C2, isoch transport = C4, selector table = C5, latency = C6.

## 0. Headline

The Panel's control plane is `IORegistryEntrySetCFProperties` writing the `MA*` property surface on the **device registry entry** — not `IOConnectCallMethod`. The vendor `com_m_audio_FWUserClient` carries MIDI RPC, Mach-port notifications and shared-memory rings only. Toward the device the kext uses **four** control-plane wire forms (HSCI vendor-dependent CONTROL for the clock source; **direct async block writes** to `0xFFC7:0x00700000+` for most mixer state; **AV/C** commands for stream format, two mixer keys, selector and LED functions; and the bootloader/firmware writes of `FWMetaNub`) and one telemetry read (the HSCI poll).

---

## 1. Channels at a glance

| # | Direction | Sender → receiver | Channel | Wire format | When |
|---|---|---|---|---|---|
| 1 | Panel → kext | Panel → `FW1814Device` registry entry | `IORegistryEntrySetCFProperties` (MIG `is_io_registry_entry_set_properties`) | OSSerialize XML dict of `MA*` keys | each panel edit (one call per click, B5 §10) |
| 2 | kext → device | `SetClockSourceInternal @0xe25c` | `[+0x268]->vt[0x8b8]` (HSCI send) | **16-byte** vendor-dependent CONTROL, company_id `0x040004` (§2) | clock-source change |
| 3 | kext → device | `CustomMixerWriteQuads @0x22c7a` | async block write via controller `vt[0x890]` | big-endian int16 pairs / bit images at `0xFFC7:0x00700000 + offset` (§3) | mixer deltas, routing, headphone selector, aux monitor, pan |
| 4 | kext → device | `AVCControlPlugSignalFormat @0x13734` etc. | AV/C over FCP (`IOFireWireAVCUnit` command) | plain 8-byte INPUT/OUTPUT PLUG SIGNAL FORMAT; Feature function block (`MAAuxOutputPan2`); mixer connect pair (`MASWReturnRoutingV2`); selector command; LED status (§4) | stream start (C2 §7), two mixer keys, A/B LEDs |
| 5 | device → kext | `FWHSCI::HSCITimerFired @0x1d600` | async read of `0xFFC7:0x00600000`, `fRate>>2` quadlets | packet decoded by `FW1814Device::ReceiveControlPacket @0xe9ea` (§5) | every `50 + [+0xd4]` ms (1 s after an error) |
| 6 | kext → userland | `FWKIPCUserClient` type-0 port | `mach_msg_send_from_kernel_proper` | 44-byte template; property-change messages carry the serialized XML dict after the header; `message()` forwards IOKit message codes (§6) | property changes, IOKit messages |
| 7 | kext → userland | `FWUserClient` type-1 port | same | MIDI-in push, ≤ 320 bytes (§6) | MIDI input (plug-in does **not** use it) |
| 8 | kext ↔ userland | `clientMemoryForType` | shared memory | type 1: `uint32 peak[52]` meter page; `0x400+n`: MIDI RX ring (read-only); `0x800+n`: MIDI TX ring (writable) (§7) | continuous |
| 9 | plug-in → kext | `IOConnectCall…Method` selectors 500–505 | legacy `IOExternalMethod` table (§7) | MIDI start/stop/send; DiceSet/Get; 506/507 = `WaitForTermination`/`SetInitializedProperty` | MIDI lifecycle, per-packet send |
| 10 | helper → kext | helper → `FWMetaNub` registry entry | `IORegistryEntrySetCFProperty` | `MAFirmwareHalt` / `MAFirmwareStart` = `"YES"`; `MAFirmwareUpgradeImage` = `.bcd` bytes (§8) | device arrival / update |
| 11 | kext → helper | `FWKIPCUserClient` type-0 port | same as #6 | same | helper opens its own UC purely for this sink (A4-H8b/H11) |
| — | off-channel | coreaudiod / DAW ↔ `IOAudioEngineUserClient` | standard IOAudioFamily | — | streaming; the vendor channel is quiescent (B5 §6a) |

---

## 2. HSCI clock-source frame (channel 2)

Transport: AV/C vendor-dependent CONTROL carried over FCP. **16 bytes** (`bzero(…,0x10)` `0xe284/0xe2a2`; length `0x10` at `0xe441`; R3). Fields (A1-Q5; `SetClockSourceInternal` raw disassembly `0xe3e2..0xe425`):

| Byte | Field | Value |
|---|---|---|
| 0 | ctype | `0x00` CONTROL |
| 1 | subunit | `0xFF` unit |
| 2 | opcode | `0x00` vendor-dependent |
| 3–5 | company_id | `0x04 0x00 0x04` (the clock frame; A1-Q6) |
| 6 | clock source | permutation of `mode & 3` (C2 §2.4): 0→3, 1→1, 2→2, 3→0 |
| 7 | optical-IN format | `(MAClockSource >> 3) & 1` (0 S/PDIF, 1 ADAT) |
| 8 | optical-OUT format | `(MAClockSource >> 4) & 1` |
| 9 | S/MUX | `(MAClockSource >> 5) & 1` |
| 10–15 | — | content **unverified** (kext sends 16 bytes; the 12-byte layout with a `clk_lock` byte 9 is snd-bebob's, `bebob_maudio.c:183-198`) |

Timing: the fixed sleeps around the send are in C2 §2.7. The stream-start AV/C order is in C2 §7 (the 100 ms OUTPUT→INPUT pattern is snd-bebob's, not the reference's). AV/C responses echo the frame with the status in byte 0 (`0x08` NOT_IMPLEMENTED, `0x09` ACCEPTED, `0x0a` REJECTED; B2 §4.5); the kext checks this inside `AVCCommand`.

---

## 3. Mixer register writes (channel 3) — the main mixer path

**There is no AV/C opcode `0xBA9`.** `0xBA9`/`0xBE9` are Itanium virtual-member-function pointers (vtable byte offset + 1) stored in batch functors by `BaseChannelGroup::SetupBatchChangeMethod @0xe184`: `0xBA9` → `FW1814Device vt[0xba8]` = `CustomMixerWriteQuads @0x22c7a`; `0xBE9` → `vt[0xbe8]` = `AVCControlSetLEDStatus @0xdaaa` (A/B-switch LEDs) (Q15f Correction; R2).

**Mechanism (Q-MAMAP §1).** `FW1814Device::InitializeUserSettableProperties @0xe6ca` builds six channel groups (SW return `0x00`, Output `0x08`, Input `0x10`, Headphone `0x38`, Aux output `0x34`, Aux send `0x64`) and four settings objects (rotary, aux send, levels, A/B switch). Each group's batch sub-index equals the byte offset of the register block (the same offsets as FFADO's map, `special_avdevice.h:55-69`). A property write travels `FWPropertyDict::SetOneProperty @0x9f74` → `ModifyProperty` (queue) → workloop `checkForWork` → `ApplyModifiedProperties @0x9b40` → `FWChannelGroupProperty::CallSetter @0x940e` → `SendLevelsToDevice @0x1ef64` → functor → `CustomMixerWriteQuads`: async block write to node address `0xFFC7:(0x00700000 + offset)`, `nQuads × 4` bytes.

**Level encoding.** Property level −255..0 (0.5 dB steps); wire int16 = level × `0x80`, **byte-swapped to big-endian**, two channels per quadlet in channel order (`-255` → `0x8080`, `0` → 0 dB). **Mute and solo have no register**: `AdjustedLevelForChannel @0x1ee66` returns the group minimum (−255) when the global mute flag is set or another channel is soloed, and applies dim as −20 units (−10 dB). Delta apply: only changed groups are re-sent (same shape as Rust `special.rs:1053-1093`, B2a).

**Which keys use which wire** (full 43-row table with registration function, setter, default and unverified cells: **`A1-ghidra/Q-MAMAP-key-crosswalk.md`**):

| Wire | Keys |
|---|---|
| Register `0x00/0x08/0x10/0x38/0x34` | `MA{SWReturn,Output,Input,Headphone,AuxOutput}{Levels,Mute,Solo,Link}` (Headphone has no Solo) |
| Register `0x64` | `MAAuxSendLevels` (22 values; rotated 11-quad write `{in[9],in[10],in[0..8]}`, R14) |
| Register `0x40+4·((ch−1)/2)` | `MAInputPan2` (half-word byte order on the wire **not derived**) |
| Register `0x90` | `MAInputRoutingV2` (bit image `[+0x1308]`) |
| Register `0x98` | `MAHeadphoneSourceV2`, `MAHeadphoneSecondSource`, `MAMomentarySwitchRoutings/Source` (bit image `[+0x1310]`) |
| Register `0x9c` | `MAMonitorAuxOnOutput` (bit image `[+0x1314]`) |
| **AV/C** | `MAAuxOutputPan2` → `AVCControlFeatureFunctionBlock` (selector 3, value<<7); `MASWReturnRoutingV2` → `SetAVCRouting` → `AVCMixerConnectPair` |
| RAM only | `MARotaryControlV2`, `MAMomentarySwitchLatent`, `MARateLocked` |
| Other | `MAClockSource` → HSCI (channel 2); `MASampleRate` → engine format change (C2 §4); `MAEnableStereoPairEngines`/`MAEnableAggregateEngine` → engine export; `MAMetaResetToFactorySettings` → reset + resend; `MADirectMonitoringActive` ignores its value (sets `[+0x11c]=1`, clears the Input mute flag) |

Group offsets also match macfw's register map (B4 §5). Routing-bit meanings within `0x90/0x98/0x9c` beyond the bit positions in Q-MAMAP are **unverified** (needs FFADO/Rust cross-read or a live write capture); the exact `AVCMixerConnectPair` frame and the Feature-function-block selector semantics (2 = volume, 3 = balance assumed from call sites) are **unverified**.

---

## 4. AV/C commands (channel 4)

- **Stream format** (plain 8-byte CONTROL, Rev1): INPUT `00 FF 19 00 90 <SFC> FF FF`, OUTPUT `00 FF 18 00 90 <SFC> FF FF` (`AVCControlPlugSignalFormat @0x13734`), issued by `ConnectAudioMIDIChannels` in the order of C2 §7 / C1 §3.7. The `0x2F` extended form is Rev2-only (Q3a).
- **Selector function-block command** `FWAVCAudioDevice::AVCSelectorCommand(uchar,uchar) @0x23074`: builds the 12-byte buffer `00 08 B8 80 <id> 10 02 <value> 01 00 00 00` and sends it through `[+0x268]->vt[0x8b0]` (code read `0x23089..0x2313c`; reading the frame as an AV/C FUNCTION BLOCK selector command to audio subunit `0x08` is **reasoning**).
- **Feature function block** (`AVCControlFeatureFunctionBlock`, aux-out balance), **mixer connect pair** (`AVCMixerConnectPair`, SW-return routing), **LED status** (`AVCControlSetLEDStatus @0xdaaa`).
- The kext does **not** send `ExtendedPlugInfo` (Q3a §4); the plug map is hard-coded.
- Cost per AV/C transaction ("about 1 ms") and the "88 ms per edit" arithmetic of the 2026-10-13 text are **estimates, unverified** (check: timestamped FCP capture on SL; owned by C6).

---

## 5. HSCI poll (channel 5) and what it feeds

`FWHSCI::HSCITimerFired @0x1d600` reads `0xFFC7:0x00600000` (`fRate>>2` quadlets) and re-arms after `50 + [+0xd4]` ms (1000 ms on error; `0x1d50d..0x1d513`); completion is dispatched with `InvokeOnNewThread` (`0x1d89e`) (Q6, R6). The listener is `FW1814Device::ReadHSCIData(unsigned*) @0xd952` (`vt[0xbf8]`, registered by `StartReceptionOfControlPackets @0xe064`): it reads the packet word at byte offset `0x50` (**bit 24 = external sync, bits 18:16 = FDF index**), calls `DetectExternalSyncChanges`, then tail-calls `ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*) @0xe9ea` (`vt[0xbd8]`), which decodes (Q-UC-surface §3; P4/R38):
- hardware peak levels → the meter page (`peak[2i] = bswap16(pkt.u16[4+4i])`, `peak[2i+1] = bswap16(pkt.u16[6+4i])`, i = 0..18; slots 0–37; only when the engine FDF index `[+0x228] < 5`, i.e. ≤ 96 kHz);
- software-return peaks copied from `[+0x110]` into slots 38–51;
- the front-panel rotary encoders (`TRotaryControl` at `+0x2c8/+0x2cc`) → `MAHeadphoneLevels` etc. via `SetPropertyAndNotifyUserClients`;
- sync state → `DetectExternalSyncChanges` → `MARateLocked` (C2 §3).

Consequently **meter and rotary data arrive at the HSCI cadence**, not from a separate 30 Hz kext timer (the "30 Hz pump" of the earlier text is unsupported; R6). That PCR reads also happen elsewhere (`updateP2PCount`, C4) means "no async reads outside the HSCI window" is wrong. FFADO notes the firmware "can't respond against frequent requests" (`special_avdevice.cpp:161`) — a third-party remark, not a measured invariant; the clone should mirror the reference's access pattern.

---

## 6. Notifications (channels 6, 7, 11)

`IOConnectSetNotificationPort(conn, type, port, refCon)` on `com_m_audio_FWUserClient` (derived from `FWKIPCUserClient`; Q-UC-surface §1):
- **Type 0** (`FWKIPCUserClient::registerNotificationPort @0x15f3a`) fills a 44-byte template at `this+0xe0`: `msgh_bits 0x13`, `msgh_size 0x2c`, remote port, local 0, message type at kernel offset `+0x20`, refCon at `+0x24`, dirty generation at `+0x28`. Two message kinds use it: **property change** — `SendChangedPropertiesMessage @0x1602a` (reached only from `FWPropertyDict::SendPropertyChangedMessagesToUserClients`) serializes the modified-properties dict (`SerializeModifiedProperties @0x15cba`) into an IOBMD of length `0x2c + len`, copies the template in front, sets `msgh_size` to the total and `+0x20 = 0` — **the XML follows the 44-byte header inside the message itself**; and **IOService messages** — `message() @0x15fca` sends the template with `+0x20 =` the IOKit message code, then chains to `IOUserClient::message`.
- **Type 1** (`FWUserClient::registerNotificationPort @0x74ab`, override) is a **MIDI-in push channel**: template at `this+0x148` (size `0x140` = 320, port `+0x150`, refCon `+0x16c`); `ReceivedMIDIPacket @0x757f` → `SendMachMessage(1, …)` with size `min(len+0x40, 0x140)`. The v1.11.x plug-in does not register it (it polls the rings); who registers type 1 is **unverified**.
- Any other type: `IOLog("M-FW: Someone registered for an unknown message type:%X\n")` and `0xE00002C2`. **There are no "peak" or "rate-lock" body types** (the earlier three-body-type table is retracted; A2-P2 had already retracted it panel-side). Peaks travel in the meter page; rate-lock changes arrive as ordinary property changes (`MARateLocked`).
- The kernel offsets above are for the 64-bit kernel `mach_msg_header_t` (32 bytes). The userspace-received offsets after header conversion are **unverified**; A2-P2 reads the discriminator at user offset `+0x18` (consistent with a 24-byte user header).
- **Dirty collection:** `AddModifiedProperties @0x1620c` (lazy `[+0x110]` dictionary + `[+0x120]` recursive lock; `OSIncrementAtomic(+0x118)`); fan-out via the static `sUserClients` OSSet (one entry per open UC, including the helper's). The helper merges changes and persists them to CFPreferences ~4 s later (A4-H11).

---

## 7. User-client surface and shared memory (channels 8, 9)

**Selector table** (`getTargetAndMethodForIndex @0x72f6`, legacy `IOExternalMethod`, `sMethods` at `0x339e0`, 48-byte stride; Q-UC-surface §2 — supersedes Q11's columns):

| Sel | Function | Type | Notes |
|---|---|---|---|
| 500 | `EntryStartMIDI @0x732c` | ScalarIScalarO | returns `kIOReturnBusy` until isoch streams are up; the plug-in retries (A3-M5) |
| 501 / 502 | `EntryStopMIDI @0x7398` (502 is an alias) | ScalarIScalarO | |
| 503 | `EntrySendMIDI(MAMIDIPacket*, uint32_t*) @0x73fa` | StructIStructO, 0x118 in / 4 out | per-packet TX |
| 504 | `EntryDiceSetRequest @0x746a` | ScalarIStructI, variable | callers inside `_extracted/` **unverified** |
| 505 | `EntryDiceGetRequest @0x7476` | StructIStructO, variable | same |
| 506 | virtual `vt[0x968]` = `FWUserClient::WaitForTermination() @0x761e` | ScalarIScalarO | **not** a ProTools no-op |
| 507 | virtual `vt[0x970]` = `FWUserClient::SetInitializedProperty() @0x71f6` | ScalarIScalarO | sets `MADeviceInitialized` |

`EntryProToolsOpen @0x7482` and `EntryProToolsErrorReport @0x748a` exist but are unreachable from `sMethods`. The full selector discussion is C5.

**`clientMemoryForType @0x7220`** accepts only: **1** → device `[+0x100]` (options `0x1000`); `0x400+n` (n < `[eng+0x29c]`) → MIDI RX ring of source port n (read-only, `0x1000`); `0x800+n` (n < `[eng+0x2a0]`, capped at 8) → MIDI TX ring of destination port n (writable, options 0); anything else, **including type 0**, → `0xE00002C7`. So there is no "type-0 dirty-dictionary mapping" (the old §5/§6/§11 idea is retracted: the dict travels in the Mach message, §6).

- **Meter page (type 1):** one 4 KiB page (`withOptions(0x10000, 0x1000, 0x1000)`, `0x3598..0x35a7`), `uint32_t peak[52]` (`GetPeakLevelCount() = 0x34`), no header or writer index, overwritten in place by `ReceiveControlPacket` (§5). The Panel maps it once with option `0x1001` (`kIOMapAnywhere|kIOMapReadOnly`); B5 cap 70 shows zero MIG traffic for 30 s. The three `IOKit SM=SHM` regions seen in the Panel's `vmmap` (16 K, 20 K, 4 K) — attribution of the other two to memory types is **unverified**.
- **MIDI rings (`0x400+n` / `0x800+n`):** per port one 8 KiB `IOBufferMemoryDescriptor::withOptions(0x10003, 0x2000)` (`TKernelMIDIPort::Init @0x1e64a`). Ring headers `{head, tail}` at mapping offsets **+0x000** (timestamped ring) and **+0x808** (realtime ring); the "0x10" in `TUserMIDIPort[0x10]` is an object-field index, not an offset (R11). Ring = `{uint32 writeIdx; uint32 readIdx; 128 × 16-byte slot}`, slot `{u64 hostTimeNanos; u32 length; u8 data[4]}`; full at 125/128; mod-128 arithmetic must be bit-identical. RX stream (A3-M4): `{uint32 writeIdx; u8 bytes[512]}`, drained by the plug-in at 1500 Hz (666 µs). Port objects are 40 bytes; default port names `"1"`, `"2"`, …; FW1814 has 1 MIDI in + 1 out (A3). Whether the RX-ring header offsets for `0x400+n` equal the TX ones is **unverified**.
- **MIDI selectors in practice:** 500/501/503 only (A3-M5).

**Vendor UC multiplexing:** Panel, helper (two instances per start, A4-H11) and MIDIServer (two pids) open independent `FWUserClient`s (B5 §3/§6c); the reference never terminates them on `clientClose` (Q-UC-surface Correction; the clone should — deliberate deviation, needs the user's sign-off, backlog A1-Q-UC.6).

---

## 8. Firmware wire (channel 10)

Not the vendor UC and not the kext's selectors: three registry-property writes on the matched `com_m_audio_FWMetaNub` entry (A4-H2), plus one read:
1. **`MAFirmwareHalt`** (`"YES"`) → `FirmwareHalt @0x1bff2` writes a RESET opcode to `0xFFFF:0xC8021000` (`0x01020000 | subcode 1`, Q7) — **only when `[+0xf0]` (bootloader active) is clear** (`cmpb $0,0xf0; jne` at `0x1c007`); `fState = 4` is stored before that check. The device re-enumerates as `0x10070`.
2. **`MAFirmwareUpgradeImage`** (NSData: the whole `.bcd`) → `FirmwareUpdate…` in the kext runs the chunked BeBoB bootloader protocol itself (`DownloadSectionStart/SendNextBlock/Stop`; **decoded in `Q-P3-kext-firmware-path.md`**). The helper reads only the `.bcd` header date/CRC (A4-H9). On success there is no bus reset; only the error path calls `resetBus` + `IOSleep(3500)` (Q-P3 §5). Full protocol: `A1-ghidra/Q-P3-kext-firmware-path.md` (command words `0x0A040000/0x03050000/0x00060000`, registers `0xFFFF:C8021000/C8021040/C8029000`, 512-byte blocks).
3. **`MAFirmwareStart`** (`"YES"`) → `FirmwareStart @0x1beb4`: in bootloader mode writes the 12-byte cue `{[+0x98], 0x01110000, 0}` to `0xFFFF:0xC8021000` (`0x1befa..0x1bf34`) and arms the 2500 ms re-register; in runtime mode registers the nub a second time (C1 §1) — **required for the product class to start**.
4. **Read:** `MAFirmwareInfo` = 80 bytes read from `0xFFFF:0xC8020000` (`FirmwareCacheInfoRegisters`); `[0xc]` = state (0 running, ≠0 ready for upload), `[0x20..0x2f]` = firmware date; rest of the layout **unverified** (P3).

macfw's boot-from-flash cue is `01 00 00 00 00 00 11 01 00 00 00 00` at the same address (B4 §7). `[+0x98]` is **not** a node handle: it is info word 2 of the cached info block = the bootloader **protocol version** (`FirmwareCacheInfoRegisters` copies the block to `this+0x90`), which is 1 on the live device (SL capture 12:3920). So the kext's `{protocol version, 0x01110000 (Go), 0}` is byte-identical to macfw's cue (Q-P3 §3–§4; verified by asm + capture). macfw's decode of the info block at `0xC8020000` (image base `0x20080000`, max `0x00180000`) vs the A5 `.bcd` header fields is **unverified**.

The helper is one-way: it opens the vendor UC only for the notification sink (A4-H8b).

---

## 9. Panel write channel (channel 1)

B5 §10 (caps 60, 61, 62, 63, 66, 68, 69) is definitive: control-plane writes are one `IORegistryEntrySetCFProperties(deviceEntry, dict)` per click; the kernel unserializes XML like `<dict><key>MAClockSource</key><integer size="32">0x4000001c</integer></dict>`; `FW1814Device::setProperties` → `FWAVCAudioDevice::setProperties` (fast-paths **`MASampleRate`**, the only hard-coded key) → `FWPropertyDict::SetProperties @0xa0e0` (single generic dispatcher) → workloop `ApplyModifiedProperties`. Property values seen on SL: `MASWReturnLevels` = array of signed 32-bit ints (cap 62), `MAMomentarySwitchSource` = 0/1 (cap 63).

The key surface is documented per key in **Q-MAMAP** (43 settable rows + read-only descriptors `MAActualProductName`, `MAIconName`, `MAMainEngineUniqueID`, `MAMaxBytesPerCycle`, `MABytesPerCycle`, `MAMIDIInput/OutputPorts`, `MADeviceInitialized`, `MAFirmwareInfo`; A2-P3 counts 46 keys on the panel side — reconciliation of the two counts is in Q-MAMAP). Keys the FW1814 kext does **not** register (`MASPDIFMixerBypass`, `MASPDIFMirrorToAnalog12`, `MAChannelOrderSPDIFFirst`) are known to the panel's shared translator only.

**`MARateLocked`** has two writers and no readers (C2 §3). The kext accepts rate writes even while streaming; v1.9 panics on that (B5 cap 65). **Clone rule:** gate `MASampleRate` in `setProperties` on the engine state/active client.

---

## 10. Serialisation

Device-bound traffic is dispatched on the **device workloop** (`IOCommandGate::runCommand` / event sources): property application (`checkForWork` → `ApplyModifiedProperties`), mixer register writes, AV/C commands, format change (`performFormatChange` → `runAction` → `FWEngine2::PerformFormatChange`; chain in C2 §4), MIDI start (`TPTMFFunctor1<FWBaseEngine*>` via the command gate; `kIOReturnBusy` while the gate/engine is not ready). **Exceptions (not on the workloop):** HSCI poll *completion* (`InvokeOnNewThread`, §5) and `BusReset` → `ReconnectP2P` (`InvokeOnNewThread`, C1 §3.10). During a format change the sleeps of C2 §7 run on the workloop and block everything else. The meter page is written by `ReceiveControlPacket` with no lock (readers tolerate torn 16-bit values by design, A2-P9). The DCL ring is self-sustaining (`CheckDCLOverrun` is dead code, Q1c); the workloop is the serialisation primitive. XPC bridging, if ever added, must marshal back onto the workloop before touching device state.

---

## 11. Open questions

1. Bytes 10–15 of the HSCI clock frame (§2) — SL FCP capture of a clock change.
2. Wire byte order of `MAInputPan2` half-words, routing-bit meanings in `0x90/0x98/0x9c`, `AVCMixerConnectPair` frame, Feature-FB selector semantics (§3).
3. ~~Kext firmware protocol, `MAFirmwareInfo` layout, value of `[+0x98]`~~ **Resolved (P3, R37).** Open residue: what the device does with the CRC quadlet appended to `DownloadEnd` (Q-P3 §8).
4. Who registers the type-1 notification port, and the user-side message offsets (§6).
5. Whether anything in `_extracted/` calls selectors 504/505 (§7).
6. Attribution of the two other `SM=SHM` regions and the RX-ring header offsets (§7).
7. Cost of an AV/C transaction and any rate limit the firmware imposes (§4/§5) — SL timestamped capture.
8. MIDI-side `fDestPortInSysex` consumer in the kext (A3-M8).

## 12. Clone-build implications (D2)

**Must implement:**
1. HSCI clock frame (16 bytes, §2) and the HSCI poll + `ReceiveControlPacket` decode (§5).
2. `IORegistryEntrySetCFProperties` dispatch on the device entry (override `setProperties`; fast-path `MASampleRate` with an engine-busy gate; everything else through one generic dispatcher; Q-MAMAP key surface) (§9).
3. **Both** mixer wires: register block writes to `0xFFC7:0x00700000 + offset` (§3) and the two AV/C-routed keys (`MAAuxOutputPan2`, `MASWReturnRoutingV2`) (§3/§4); mute/solo/dim as level operations; level encoding as in §3.
4. AV/C plug-signal-format, selector, LED commands (§4).
5. `FWUserClient` with the legacy `IOExternalMethod` table, selectors 500–507 (506/507 real), `clientMemoryForType` types 1, `0x400+n`, `0x800+n` (§7).
6. `FWKIPCUserClient` base behaviour: type-0 port (property-change XML in the message body + IOKit message codes) and type-1 MIDI-in port; `sUserClients` fan-out (§6).
7. Firmware property handshake on the nub (§8), including the second registration on `MAFirmwareStart`.
8. One workloop per device; keep the two `InvokeOnNewThread` exceptions (§10).

**Optional / deliberate deviations (need the user's sign-off):** terminate user clients on `clientClose` (reference leaks); replace the Mach-port notification transport (the *events* must still be raised); omit `MASPDIFMixerBypass` etc. (not FW1814 keys).

## Change log (2026-10-14 corrections folded in)
MA* crosswalk and the two-wire mixer (Q-MAMAP, R20) → §3; MIDI ring layout (R11) → §7; settings persistence (A4-H11) → §6; HSCI 16-byte frame, stream-kick order, UC surface, meter page, HSCI threading, unverified timing claims, 12-byte cue (R1, R3, R5, R6, R17) → §2, §4–§8, §10; `0xBA9`/`0xBE9` as virtual-member pointers and the "second mixer path" question (Q15f) → §3 (the old §3 "AV/C opcode 0xBA9" and §4 "second path (open)" are removed); notification body types and "type-0 memory map" retracted (Q-UC-surface) → §6/§7; D1/D2/D3 wording removed (R34).
