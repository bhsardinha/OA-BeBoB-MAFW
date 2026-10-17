# Q-UC — user-client surface: selectors, meter page, KIPC liveness, notifications

> **Correction (2026-10-14) — power-cycle timeline** (evidence: [PROVENANCE-RESOLUTIONS.md §R25](../PROVENANCE-RESOLUTIONS.md), captures 72 and 73). **Cold-start timeline measured (capture 73, interface switched off/on):** off → two bus resets; on → bootloader found (`M-FW: Bootloader active: starting firmware`, kext `FirmwareStart`) → ~170 ms of 5 ms polling sleeps → bus reset → firmware active +3.3 s after power-on (`Firmware active: registering metanub`) → `Reregister: device node vanished` → clock sets (300×2, 2500) → one-second mixer resend (the log warns `Trying to resend mixer settings too often`) → the standard stream-start chain again; quiet after ~14.7 s. **`ReconnectP2P`'s `IOSleep(1100)` did not occur in this scenario**: the device node vanished and a fresh `FW1814Device` was instantiated, so the live-device BusReset→ReconnectP2P path (R4) remains unobserved. The 11 stale `FWUserClient`s created by my earlier process restarts disappeared with the old device instance (leak is bounded by device lifetime, not permanent).


> **Correction (2026-10-14) — user-client leak** (evidence: [PROVENANCE-RESOLUTIONS.md §R16](../PROVENANCE-RESOLUTIONS.md)). **No `terminate()` anywhere in the user clients.** `FWUserClient::clientClose @0x71a8` (v1.11.2) only clears `[+0x150]`, stops MIDI if `[+0x140]`, returns 0; `FWKIPCUserClient::clientClose @0x15c38` only zeroes `[+0xe8]`; there is no `clientDied`/`terminate` override, and the v1.9 binary has **0** occurrences of `terminate`. Live proof on SL v1.9 (2026-10-14): `ioreg` shows `com_m_audio_FWUserClient` objects (`active`, retain 5-6) created by pids that no longer exist (helper 463/492/534, killed Panel 319). The reference therefore leaks one user-client object per client process exit (and each helper start creates **two**). Clone implication (reasoning): a correct clone calls `terminate()` from `clientClose`; this is a deliberate deviation from the reference and needs the user's sign-off.


> **Correction (2026-10-14) — MIDI ring layout** (evidence: [PROVENANCE-RESOLUTIONS.md §R11](../PROVENANCE-RESOLUTIONS.md)). Kernel side, `m_audio_FWb::TKernelMIDIPort::Init @0x1e64a`: one `IOBufferMemoryDescriptor::withOptions(0x10003, 0x2000)` per port (8 KiB, kernel/user shared), stored at `port[+0x20]` (the descriptor `clientMemoryForType` returns). Its base `B` (`getBytesNoCopy`, vt+0x2e0): `port[+0x00] = B` with `*(u32*)B = 0`; `port[+0x10] = B` with `*(u32*)B = 0`, `*(u32*)(B+4) = 0` (ring header {head, tail}); `port[+0x18] = B + 0x808` with `*(u32*)(B+0x808) = *(u32*)(B+0x80c) = 0` (second ring header). So the **timestamped ring is at mapping offset +0x000 and the realtime ring at +0x808**; the `0x10` in `TUserMIDIPort[0x10]`/`TKernelMIDIPort[0x10]` is the *object field index* that stores the base pointer, not a byte offset (C5:269 '+0x000' is right; C3:194 '+0x010' was wrong). Memory types: `clientMemoryForType` (0x7220) serves `0x400+n` from `[eng+0x2b0]` (input ports, `SetMIDIInputPorts @0x5226`, count `[+0x29c]`) with `*options = 0x1000` (kIOMapReadOnly) and `0x800+n` from `[eng+0x2a8]` (output ports, `SetMIDIOutputPorts @0x5160`, count capped at 8 `[+0x2a0]`, writable). Port objects are 40 bytes (`(n<<3)*5`). Default port names are `"1"`, `"2"`, … when none are passed (0x50ae-0x50ca). The panel's meter map option `0x1001` = `kIOMapAnywhere|kIOMapReadOnly` (not 'DefaultCache').


**Status:** done 2026-10-14. Answers BUILD-MANUAL §2.F: Q-UC-SELECTORS, Q-UC-RING, Q-KIPC-LIVE, Q-NOTIFY. Corrects Q11 (flag/count reading, selectors 506/507) and the "3 notification body types" claim (STATE, CLAUDE.md, B2a, B4, C3, D). A2-P2 had already retracted that claim on the panel side on 2026-10-06.

**Sources:** raw `otool-exports/mafw.asm`, slice bytes for `sMethods` + vtables, `otool -rv` relocations, Ghidra 12.1.4 read-only decompiles (scratchpad, 2026-10-14). The flag enum is from the 10.13 SDK `Kernel.framework/Headers/IOKit/IOUserClient.h:47–51` on the build PC: `kIOUCScalarIScalarO = 0`, `kIOUCScalarIStructO = 2`, `kIOUCStructIStructO = 3`, `kIOUCScalarIStructI = 4`.

## 1. Class relationship — Q-KIPC-LIVE: **yes, live**

`com_m_audio_FWUserClient::MetaClass` registers superclass **`m_audio_b_FWKIPCUserClient`**. `FWUserClient::MetaClass::alloc` (`0x752f`) allocates `0x298` bytes and runs `FWKIPCUserClient::FWKIPCUserClient(gMetaClass)` (`0x7554`). The `FWUserClient` vtable (`0x33c50`) contains `FWKIPCUserClient::AddModifiedProperties` at slot `0x960`.

So the KIPC notification machinery (Q14) is the **base class of the vendor user client** that every Panel/helper/MIDI connection opens (`IOUserClientClass = com_m_audio_FWUserClient` in each device personality, Q-INFO). It is not a separate, possibly-dead class. The clone needs it.

## 2. Selectors — Q-UC-SELECTORS

`getTargetAndMethodForIndex @ 0x72f6` returns `&sMethods[sel − 500]` for `sel − 500 < 8`, with target = `this`, provided `[this+0x290]` (the device) is non-NULL. The table is the **legacy `IOExternalMethod`** layout `{object, func (16-byte PMF), flags, count0, count1}` at `__ZN24com_m_audio_FWUserClient8sMethodsE = 0x339e0`, stride 48 bytes. It is not `IOExternalMethodDispatch`.

| Sel | func | flags → type | count0 | count1 | Meaning |
|---|---|---|---|---|---|
| 500 | `0x732c` `EntryStartMIDI()` | 0 ScalarIScalarO | 0 | 0 | no args |
| 501 | `0x7398` `EntryStopMIDI()` | 0 | 0 | 0 | no args |
| 502 | `0x7398` `EntryStopMIDI()` (alias) | 0 | 0 | 0 | no args |
| 503 | `0x73fa` `EntrySendMIDI(MAMIDIPacket*, uint32_t*)` | **3 StructIStructO** | **0x118** (280-byte input struct) | **4** (4-byte output struct) | send MIDI |
| 504 | `0x746a` `EntryDiceSetRequest(MAFireWireDiceRequest*)` | **4 ScalarIStructI** | 0 scalars | `0xffffffff` (variable struct) | |
| 505 | `0x7476` `EntryDiceGetRequest(req*, void*)` | **3 StructIStructO** | `0xffffffff` | `0xffffffff` | variable in/out |
| 506 | PMF `0x969` → **virtual `vt[0x968]` = `FWUserClient::WaitForTermination()` @ `0x761e`** | 0 | 0 | 0 | |
| 507 | PMF `0x971` → **virtual `vt[0x970]` = `FWUserClient::SetInitializedProperty()` @ `0x71f6`** | 0 | 0 | 0 | |

Evidence for 506/507:
- Entries 500–505 have local `__TEXT` relocations at their func slots (`0x339e8 … 0x33ad8`). Entries 506/507 (`0x33b08`, `0x33b38`) have **none**, so `0x969`/`0x971` are Itanium virtual-PMF values (vtable offset + 1), not addresses.
- `FWUserClient` vtable `0x33c50` + 0x10: slot `0x968` = `0x761e`, slot `0x970` = `0x71f6`. The vtable ends at `0x345e0`.
- `EntryProToolsOpen() @ 0x7482` / `EntryProToolsErrorReport(int*) @ 0x748a` exist but are **not reachable** from `sMethods`.

> **Correction to A1-Q11 (2026-10-14):** Q11's columns ("ScIn/StIn/ScOut/StOut") applied IOExternalMethodDispatch semantics to a legacy IOExternalMethod table, and named 506/507 as the ProTools stubs. Use the table above.

## 3. Meter page — Q-UC-RING

`clientMemoryForType @ 0x7220`:

| type | returns | options |
|---|---|---|
| **1** | device `[+0x100]` | `0x1000` |
| `0x400 + n`, n < engine `[+0x29c]` | stream buffer descriptor from engine `[+0x2b0]`, stride `0x28`, field `+0x20` | `0x1000` |
| `0x800 + n`, n < engine `[+0x2a0]` | descriptor from engine `[+0x2a8]`, stride `0x28`, field `+0x20` | `0` |
| anything else | `0xE00002C7` | — |

`0x1000` is `kIOMapReadOnly` (**unverified constant name — check `IOTypes.h`**). **Mappings `0x400+n` and `0x800+n` are MIDI port rings, not audio buffers.** `0x400|n` = RX ring of MIDI source port n, read-only. `0x800|n` = TX ring of MIDI destination port n, writable. This matches the MIDI plugin's `IOConnectMapMemory(isSource ? 0x400 : 0x800 + portIdx)` (A3-M4 §1, A3-M7) and the kext's options (`0x1000` vs `0`). The bounds `[engine+0x29c]` / `[engine+0x2a0]` are the per-direction MIDI port counts (**reasoning** from the match).

**Meter page creation** — `FWAudioDevice::initHardware`:
- `[+0x100]` = `IOBufferMemoryDescriptor::withOptions(0x10000, 0x1000, 0x1000)` (`0x3598..0x35a7`), one 4 KiB page. `0x10000` = `kIOMemoryKernelUserShared` (**unverified constant name — same check**). Then `prepare` (`vt[0x1f0]`, `0x35c3`).
- `[+0x108]` = `getBytesNoCopy()` (`0x3611`), the kernel pointer into that page.
- `[+0x110]` = `new uint32_t[GetPeakLevelCount()]` (`0x3624..0x3635`), a private kernel array.

**Layout** — `uint32_t peak[52]`. `GetPeakLevelCount()` = `0x34` (`FWAudioDevice @ 0x1d30`; FW1814 does not override it). Each slot holds a 16-bit value. There is no header, generation counter or writer index; the page is a plain array overwritten in place.

| Slots | Written by `FW1814Device::ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*) @ 0xe9ea` |
|---|---|
| 0–37 (`0x00..0x94`) | hardware peaks: `peak[2i] = bswap16(pkt.u16[4+4i])`, `peak[2i+1] = bswap16(pkt.u16[6+4i])`, i = 0..18. **Only when engine FDFValue `[+0x228] < 5`** (rates ≤ 96 kHz) |
| 38–51 (`0x98..0xcc`) | software-return peaks copied from `[+0x110]` (`GetFirstSWReturnPeakMeterIndex() = 0x26`, `GetSWReturnPeakMeterCount() = 0xe`; `0xc3ba`/`0xc3c6`) |
| 22/23 (`0x58/0x5c`), 24–31 (`0x60..0x7c`) | overwritten from `[+0x110]` or from slots 36/37, depending on device byte `[+0x1317] & 4` (**unverified meaning**) |
| at 176.4/192 kHz | only slots 18–21 and 38–41 are refreshed from `[+0x110]` |

The same function also feeds the front-panel rotary encoders (`TRotaryControl` at `+0x2c8`/`+0x2cc`, packet word bits 25:24 / 17:16) into `MAHeadphoneLevels` etc. via `SetPropertyAndNotifyUserClients`.

> **Correction (2026-10-14):** BUILD-MANUAL's hypothesis `MeterSample { uint8_t level[70] }` came from misreading B5's "cap 70" (= capture file `70-dtrace-meter-path.txt`). The page is `uint32_t[52]` as above. Rev2's `UpdatePeakMeters @ 0x12aae` is **not** the FW1814 path (FW1814 is Rev1; Q16 Correction).

## 4. Notifications — Q-NOTIFY

The client registers with `IOConnectSetNotificationPort(conn, type, port, refCon)`:

**Type 0** (`FWKIPCUserClient::registerNotificationPort @ 0x15f3a`) fills a template at `this+0xe0`:

| Offset in kernel template | Value |
|---|---|
| `+0x00` msgh_bits | `0x13` |
| `+0x04` msgh_size | `0x2c` = **44** |
| `+0x08` remote port | the registered port |
| `+0x10` local port | 0 |
| `+0x18`, `+0x1c` | 0, 0 |
| `+0x20` (`this+0x100`) | message type |
| `+0x24` (`this+0x104`) | refCon |
| `+0x28` (`this+0x108`) | `[this+0x118]` |

Two kinds of message are sent on this port:
- **Property change:** `SendChangedPropertiesMessage @ 0x1602a`, reached only from `FWPropertyDict::SendPropertyChangedMessagesToUserClients`. The modified-properties dict is serialized (`OSSerialize`, XML) into an IOBMD of length `0x2c + len` (`SerializeModifiedProperties @ 0x15cba`). The 44-byte template is copied in front, `msgh_size = total length`, and **`+0x20` = 0**.
- **IOService message:** `message() @ 0x15fca` overrides IOService::message. It sends the 44-byte template with **`+0x20` = the IOKit message code**, then chains to `IOUserClient::message`.

**Type 1** (`FWUserClient::registerNotificationPort @ 0x74ab`, override) is a MIDI-in push channel. The v1.11.x MIDI plugin does **not** use it; it polls the `0x400|n` rings every 666 µs (A3-M3/M6). Who registers type 1 is **unverified**. Its template is at `this+0x148`: bits `0x13`, size `0x140` (320), port at `+0x150`, refCon at `+0x16c`.

`ReceivedMIDIPacket @ 0x757f` → `SendMachMessage(1, msg, size)`:

| Field | Value |
|---|---|
| type at `+0x20` | **1** |
| size | `min(pkt->len(u16 @ +0x10) + 0x40, 0x140)` |
| `+0x28` | `pkt->u32[0]` |
| `+0x30` | 0x110 bytes from `pkt + 8` |

Any **other type** is rejected: `IOLog("M-FW: Someone registered for an unknown message type:%X\n")` and `0xE00002C2`.

> **Correction (2026-10-14):** there is no "1 = peak / 2 = rate-lock" body type. Type 1 is the **MIDI-in port** and a type-2 registration is rejected. Peaks travel via the §3 page; rate changes via polling (A2-P8/P9). Kernel offsets above are for the 64-bit kernel `mach_msg_header_t` (32 bytes). **Unverified:** the userspace-received offsets after the kernel→user header conversion. A2-P2 reads the discriminator at user offset `+0x18`, consistent with a 24-byte user header.

## Provenance

`mafw.asm`: `0x72f6`, `0x7220`, `0x7444..0x7505`, `0x752f..0x7554`, `0x757f..0x7610`, `0x15cba`, `0x15f3a`, `0x15faa`, `0x15fca`, `0x1602a`, `0x3598..0x3635`, `0xe9ea`, `0xc3ba`, `0xc3c6`, `0x1d30`. `sMethods` raw bytes `0x339e0..0x33b60` and relocations via `otool -rv`. Vtable `__ZTV24com_m_audio_FWUserClient` `0x33c50..0x345e0`.
