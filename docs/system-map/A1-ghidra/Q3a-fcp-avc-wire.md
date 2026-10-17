# Q3a — AV/C FCP wire bytes: plug signal format, CMP connect, ExtendedPlugInfo

> **Correction (2026-10-15, M-C / R44):** confirmed by `ConnectAudioMIDIChannels` decompile: TX `Connect` → 500 ms → INPUT `0x19` → 500 ms → RX `Connect` → OUTPUT `0x18`; the "program start" steps are `Connect`. `[+0x268]->vt[0x8b8]` is `FWAVCUnit::AVCCommandWithErrorResponse` (25 retries on bus reset, `CheckAVCError` pauses), not a raw send. See [`Q-MC-cmp-isoch-avc-start.md`](Q-MC-cmp-isoch-avc-start.md).

> **Correction (2026-10-15, M-B / R41):** the start order recorded here and in C2 §7 is built on `ConnectAudioMIDIChannels @0x12156`, whose program calls are `vt[0x120](program, 0)` = `Connect(channel)`; whether they also start the channel is **unverified** (hypothesis: `StartIsochStreams`). The AV/C frames and the 500 ms gaps are unaffected. See C2's correction and `Q-MB-dcl-callbacks.md` §7 item 5.


**Status:** done 2026-10-14

**Scope:** exact byte sequences for AV/C commands issued by the kext during format-set, status-read, and CMP connect. Sub-question of Q3 (ConnectAllPlugs). Sources: `docs/system-map/A1-ghidra/otool-exports/mafw.asm` direct disassembly (v1.11.2 bundle).

---

## 1. AV/C STATUS OUTPUT PLUG SIGNAL FORMAT (opcode 0x18) — read-back

Function: `m_audio_b_FWAVCAudioDevice::AVCStatusOutputPlugSignalFormat(unsigned char plug_num, FDFValue& fdf_out)` @ `docs/system-map/A1-ghidra/otool-exports/mafw.asm:42398` (`0x23374`)

**REQUEST (8 bytes):**

| Byte | Value | Meaning |
|---|---|---|
| 0 | `0x01` | ctype = STATUS (`0x23419: movb $0x1, (%rax)`) |
| 1 | `0xFF` | unit address (`0x233d0: movb $-0x1`) |
| 2 | `0x18` | opcode = OUTPUT PLUG SIGNAL FORMAT (`0x233e1: movb $0x18`) |
| 3 | `plug_num` | arg `%r14b` — output plug number (`0x233f2: movb %r14b`) |
| 4 | `0xFF` | (`0x23403: movb $-0x1`) |
| 5 | `0xFF` | (`0x23414: movb $-0x1`) |
| 6 | `0xFF` | (`0x23425: movb $-0x1`) |
| 7 | `0xFF` | (`0x23436: movb $-0x1`) |

**RESPONSE:** byte at `response[5]` = FDF value written to `*fdf_out` (`0x2347c: movl %edx, (%rbx)` after `movzbl (%rdx), %edx` where rdx = response_ptr+5).

Returns `kIOReturnSuccess` (0) if `response[0] == 0x0C` (ACCEPTED/IMPLEMENTED), else `kIOReturnError` (`0xE00002C7`).

**Usage:** called before `AVCControlPlugSignalFormat` to probe current format; also called post-format-change to verify.

---

## 2. AV/C Extended Stream Format Information — CONTROL SET (opcode 0x2F/0xC0)

> **Correction (2026-10-14) — Rev2-only; not used by the FW1814.** This is `FWRev2AudioDevice::AVCControlPlugSignalFormat @ 0x12e7e` (vtable slot `0xb50` in Rev2 classes). `com_m_audio_FW1814Device` inherits `FWRev1AudioDevice`, whose slot `0xb50` is the plain 8-byte `0x18/0x19` command (§3 Correction).

Function: `FWRev2AudioDevice::AVCControlPlugSignalFormat(PlugDirection direction, unsigned char plug_num, FDFValue fdf, unsigned int sample_rate, PlugFormat format)` @ `docs/system-map/A1-ghidra/otool-exports/mafw.asm:21878` (`0x12e7e`)

The function first fires a **STATUS probe** (0x2F/0xC1) to read the current format, then retries with **CONTROL SET** (0x2F/0xC0) until the FDF byte in the response matches the target FDF, up to `r14b` iterations (rate-dependent loop guard at `0x12f79`).

### 2a. STATUS probe (10 bytes)

Constructed on stack at `-0x36(%rbp)` (`0x12f0b`–`0x12f37`):

| Byte | Value | Meaning |
|---|---|---|
| 0 | `0x01` | ctype = STATUS |
| 1 | `0xFF` | unit address |
| 2 | `0x2F` | opcode = EXTENDED STREAM FORMAT INFO (`0x12f1a: movb $0x2f`) |
| 3 | `0xC1` | subfunction = LIST/SINGLE-read (`0x12f1e: movb $-0x3f` = 0xC1) |
| 4 | `dir` | direction: 0 = output, 1 = input (`%bl` = `sete esi==0`) |
| 5 | `0x00` | plug index hi |
| 6 | `0x00` | plug index lo |
| 7 | `plug_num` | plug number from arg (`0x12f2d`) |
| 8 | `0xFF` | don't-care |
| 9 | `0xFF` | don't-care |

### 2b. CONTROL SET (14+ bytes)

Constructed on VLA stack at `%r13` (`0x12fab`)–(`0x12fde`):

| Byte | Value | Meaning |
|---|---|---|
| 0 | `0x00` | ctype = CONTROL (`0x12fab: movb $0x0`) |
| 1 | `0xFF` | unit address (`0x12fb0: movb $-0x1`) |
| 2 | `0x2F` | opcode = EXTENDED STREAM FORMAT INFO (`0x12fb5: movb $0x2f`) |
| 3 | `0xC0` | subfunction = SET SINGLE (`0x12fba: movb $-0x40` = 0xC0) |
| 4 | `dir` | direction (0=out, 1=in) (`0x12fbf: movb %bl`) |
| 5 | `0x00` | plug index hi (`0x12fc3`) |
| 6 | `0x00` | plug index lo (`0x12fc8`) |
| 7 | `plug_num` | plug number (`0x12fcd`) |
| 8 | `0xFF` | (`0x12fd4: movb $-0x1`) |
| 9 | `0xFF` | (`0x12fd9: movb $-0x1`) |
| 10 | `0x90` | AM824 stream format tag (`0x12fde: movb $-0x70` = 0x90) |
| 11 | `0x40` | compound_flag or length-hint (`0x12fe3: movb $0x40`) |
| 12 | `FDF_byte` | rate byte from `%r14b` (`0x12fe8`) |
| 13 | `0x01` | "single stream" continue indicator (`0x12ff3`) |
| 14+ | ext | product-name bytes via `memmove` from `r15+0xf` (`0x12ffd`–`0x1300b`); `r15` = STATUS response buffer; length = `2 * response[0xf] + 1` |

**FDF byte ↔ rate map** (from Q2 SetupCIP @ `0x27102`):

| Rate | FDF | Decimal |
|---|---|---|
| 32 kHz | `0x03` | 3 |
| 44.1 kHz | `0x04` | 4 |
| 48 kHz | `0x02` | 2 |
| 88.2 kHz | `0x0A` | 10 |
| 96 kHz | `0x05` | 5 |
| 176.4 kHz | `0x06` | 6 |
| 192 kHz | `0x07` | 7 |

---

## 3. CMP P2P connect — `ConnectAudioMIDIChannels @ 0x12156`

> **Correction (2026-10-14) — this section misread the call.** `vt[0xb50]` on the device is **`AVCControlPlugSignalFormat`**, not a CMP connect. In `com_m_audio_FW1814Device`'s vtable (`0x369b0` + 0x10 + `0xb50`) it resolves to **`FWRev1AudioDevice::AVCControlPlugSignalFormat @ 0x13734`**, because FW1814Device's OSMetaClass superclass is `FWRev1AudioDevice` (`0xf473`; see the Q16 correction). Raw `0x13734..0x13827` builds an **8-byte AV/C CONTROL** and sends it via `[this+0x268]->vt[0x8b8]`:
>
> | Byte | Value | Source |
> |---|---|---|
> | 0 | `0x00` CONTROL | `0x13784` |
> | 1 | `0xFF` unit | `0x13794` |
> | 2 | `0x18 \| (dir != 0)` → **`0x18` OUTPUT / `0x19` INPUT PLUG SIGNAL FORMAT** | `0x137a5..0x137ae` (`setne; or $0x18`) |
> | 3 | plug (`edx`) = 0 | `0x137be` |
> | 4 | `0x90` (AM824 format hierarchy) | `0x137cf` |
> | 5 | FDFValue (`ecx` = engine `[+0x228]`) | `0x137e0` |
> | 6, 7 | `0xFF 0xFF` | `0x137f0`, `0x13801` |
>
> Engine `[+0x228]` is the FDFValue enum 0–6 written per rate by `FWBaseEngine::PerformSampleRateChangeCore` (`0x47fc..0x48b7`). It equals the IEC 61883-6 SFC code, so byte 5 is SFC. **It is not an isoch channel.** Rev1 **ignores** args 5 and 6 (`[+0x2e0]/[+0x2e4]` in `r8d`, `0x64` in `r9d`); only the Rev2 override (extended stream format `0x2F/0xC0`, §2) uses them. **The "100 ms delay" in the table below does not apply to the FW1814.** `[+0x350]` = `FWDCLOutputProgram` (TX) and `[+0x358]` = `FWDCLInputProgram` (RX), created in `FWEngine2::initHardware` (`0x1266e`, `0x126ca`); `vt[0x120]` on them starts the program.
>
> **FW1814 sequence.** `[+0x348]` is set to 1 by `FW1814Device::SetupAudioEngine @ 0xed73`, so the `!= 0` branch is always taken (raw `0x121f7..0x122bb`):
> 1. start TX program (`[+0x350]->vt[0x120](0)`, `0x121c9`)
> 2. `IOSleep(500)` (`0x121d7`)
> 3. **`00 FF 19 00 90 <SFC> FF FF`** — INPUT PLUG SIGNAL FORMAT, `esi=1` (`0x12211` → `0x12246`)
> 4. `IOSleep(500)` (`0x1224d`)
> 5. start RX program (`[+0x358]->vt[0x120](0)`, `0x12263`); on failure stop TX via `[+0x350]->vt[0x128]` (`0x122ce`)
> 6. **`00 FF 18 00 90 <SFC> FF FF`** — OUTPUT PLUG SIGNAL FORMAT, `esi=0`, no sleep (`0x122ae` → `0x122bb`)
>
> This is the reference form of the "stream-kick" in memory `project_fw1814_stream_kick.md`. The memory's order (OUTPUT, then 100 ms, then INPUT) came from pre-Ghidra hardware experiments. The reference does INPUT then OUTPUT, 500 ms apart, with the DCL program starts in between. **Where CMP oPCR/iPCR connect happens is still open (Q-CMP-CONNECT):** most likely inside the program start (`vt[0x120]` → `CreateIsochChannelAndPorts`, Q1b) — **unverified**, needs a decode of `FWDCLProgram` `vt[0x120]`.
>
> The original text below is kept for history.


The M-Audio kext builds the P2P connection by calling through `m_audio_b_FWAVCConnectionManager` vtable entries; the actual CMP register byte construction is inside `IOFireWireAVC.kext` (not in the M-Audio binary). The M-Audio side passes:

| Argument | Source offset | Value / meaning |
|---|---|---|
| direction | `%esi` | 1 = OUTPUT (making output P2P plug), 0 = INPUT |
| plug_index | `%edx` | 0 (fixed; plug 0 = PCM per Q-plug-map memory) |
| delay_ms | `%r9d` = `0x64` | **100 ms** — stream-kick delay (`0x121a3: movl $0x64`) |
| isoch_channel | `[+0x228]` | FWEngine2 field; channel allocated by IRM |
| bandwidth | `[+0x2e4]` (output) / `[+0x2e0]` (input) | channel-count × 2 bytes/frame |

**Sequence** (from `0x12163`–`0x122c4`):

1. If `[+0x348] == 0` (not yet connected): call vtable+0xb50 with esi=1 (OUTPUT), r9d=0x64 → P2P output connect + 100ms kick.
2. `IOSleep(500)` (`0x121b4: movl $0x1f4`).
3. `*vtable[0x120]` on `[+0x350]` ptr → connect RX isoch channel.
4. `IOSleep(500)`.
5. If `[+0x348] != 0` (reconnect path): repeat with INPUT direction.
6. `*vtable[0x120]` on `[+0x358]` ptr → connect MIDI channel.
7. Set `[+0x348] = 1` on success.

> **Correction (2026-10-14):** wrong — the M-Audio kext builds and locks the PCR quadlets itself (`FWAVCConnectionManager::updateP2PCount @ 0x244f0`, device oPCR `0xF0000904+4n`, iPCR `0xF0000984+4n`). See `Q-CMP-connect.md`.

~~**No direct CMP byte construction in M-Audio code.**~~ IOFireWireAVC handles the oPCR/iPCR quadlet writes (LOCK, bitfield update for connection_count + channel_number + bandwidth).

---

## 4. Q3a — ExtendedPlugInfo not called

**Question:** does the kext call AV/C ExtendedPlugInfo (opcode 0xC2) on Model_ID 0x10071 during probe or start?

**Answer: No.**

Grep for `movb $0xC2` in `docs/system-map/A1-ghidra/otool-exports/mafw.asm` → **0 hits**. Grep for `movb $0xC3` → **0 hits**. No byte sequence in the disassembly constructs a packet with opcode 0xC2 or 0xC3.

The reference kext does NOT issue ExtendedPlugInfo queries. Format discovery uses:
- AV/C STATUS OUTPUT PLUG SIGNAL FORMAT (0x18) to read back FDF.
- AV/C Extended Stream Format Information STATUS (0x2F/0xC1) as a probe before each CONTROL SET.

---

## 5. AV/C Selector Command (clock source)

Function: `m_audio_b_FWAVCAudioDevice::AVCSelectorCommand(unsigned char selector_id, unsigned char value)` @ `docs/system-map/A1-ghidra/otool-exports/mafw.asm:42167` (`0x23074`)

**12-byte CONTROL packet** (`0x23113`–`0x23126`):

| Byte | Value | Meaning |
|---|---|---|
| 0 | `0x00` | ctype = CONTROL (`0x23113`) |
| 1 | `0x08` | subunit address = audio subunit inst 0 (`0x23116: movb $0x8`) |
| 2 | `0xB8` | opcode = AV/C AUDIO SELECTOR (`0x2311a: movb $-0x48` = 0xB8) |
| 3 | `0x80` | selector_length (`0x2311e: movb $-0x80` = 0x80) |
| 4 | `selector_id` | arg `%r14b` (`0x23122`) |
| 5 | `0x10` | (`0x23126: movb $0x10`) |
| 6..11 | see below | |

Bytes 6–11 built in earlier loop (`0x230b0`–`0x23104`):
- [6] = `0x02`
- [7] = `value` (arg `%bl`)
- [8] = `0x01`
- [9..11] = `0x00`

Called from `SetClockSourceInternal @ 0x23148` after reading current selector state via vtable+0xb48.

---

## Provenance

- `AVCStatusOutputPlugSignalFormat`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:42398` (`0x23374`–`0x2349a`), direct byte-by-byte read.
- `AVCControlPlugSignalFormat` (FWRev2): `docs/system-map/A1-ghidra/otool-exports/mafw.asm:21878` (`0x12e7e`–`0x1304a`), STATUS at `0x12f0b`, CONTROL SET at `0x12fab`.
- `ConnectAudioMIDIChannels`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:20779` (`0x12156`–`0x122c4`); 100ms delay at `0x121a3: movl $0x64`.
- `AVCSelectorCommand`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:42167` (`0x23074`–`0x23146`).
- ExtendedPlugInfo absence: `grep -n "movb.*\$0xc2\|movb.*\$0xC2" docs/system-map/A1-ghidra/otool-exports/mafw.asm` → 0 hits; `grep -n "movb.*\$0xc3"` → 0 hits.
- FDF byte table: Q2-setupcip-rate-table.md (`0x27102`).
- INPUT format (**resolved 2026-10-14, §3 Correction**): set by `FWRev1AudioDevice::AVCControlPlugSignalFormat` with `dir=1` → opcode `0x19`. No STATUS read-back of the input plug exists in the kext. Original note: no dedicated `AVCStatusInputPlugSignalFormat` function found in the disassembly; input format is set via `AVCControlPlugSignalFormat` with `direction=1`; input read-back path **unverified — needs grep for opcode 0x19 packet construction site or DTrace capture on 192.168.100.17**.
