# Q-P3 — The kext-side firmware path (closes A1-Q18 and A1-Q19)

> **Correction (2026-10-15, scope cut, R42):** this document stays the description of the **reference** protocol. The clone implements only `MAFirmwareInfo` (§3), `MAFirmwareStart` incl. the bootloader `Go` command (§4 row 4) and the 2500 ms re-register; **§4 download commands, §5 update procedure and the `Halt`/`Reset` rows are not implemented** (user decision: no flash writes). The live `MAFirmwareInfo` of a second unit matches §3 (see `../build-pc-runs/2026-10-15-M-A-M-FW-install.md`).


**Status:** done 2026-10-14 (BUILD-PLAN task P3; static, dev Mac, plus one live value from an existing SL capture; evidence entry R37). Bundle: kext 1.11.3 (`otool-exports/mafw.asm` lines 32831–34860; Ghidra decompiles `ghidra-exports/decomp/FWMetaNub__*.c`). Third-party cross-check: FFADO `src/bebob/bebob_dl_codes.{h,cpp}`, `bebob_dl_mgr.{h,cpp}` (the same BeBoB bootloader protocol, userspace implementation). Closes the two items A4-H2 raised: **Q18** (how `MAFirmwareHalt/Start/UpgradeImage` are handled) and **Q19** (the `MAFirmwareInfo` blob). Run-time behaviour on 10.13.6 is unverified (nothing of this has run there).

## 1. Answer in one paragraph

The helper does not speak the BeBoB bootloader protocol; the **kext does**, from `com_m_audio_FWMetaNub`. Three registry-property writes on the nub drive it (`SetOneProperty @0x1ca24`): `MAFirmwareUpgradeImage` (an `OSData` holding the whole `.bcd`; accepted **only while the device is in bootloader mode**) runs `FirmwareUpdate`, which downloads the application image and the config ("CNE") section with the standard BridgeCo command set (`DownloadStart 0x04`, `DownloadBlock 0x05`, `DownloadEnd 0x06`) through four device registers; `MAFirmwareStart` sends `Go` (`0x11`); `MAFirmwareHalt` sends `Reset` to bootloader (`0x02`, start-mode 1). The key *value* is never inspected — any value triggers the action. `MAFirmwareInfo` is the first 80 bytes of the device's bootloader info register, cached at `this+0x90` and published as a property; its bytes 0x00–0x4F have the layout of FFADO's `info_register_t`.

## 2. Device registers and transport primitives

| Register (48-bit FireWire address) | Role | Kext use | FFADO (`bebob_dl_mgr.cpp:45-49`) |
|---|---|---|---|
| `0xFFFF:C8020000` | info | read **0x50** bytes (`FirmwareCacheInfoRegisters`) | `AddrRegInfo` |
| `0xFFFF:C8021000` | request (command) | every command is written here | `AddrRegReq` |
| `0xFFFF:C8021040` | request buffer (block payload) | `DownloadSendNextBlock` writes the data block here before the command | `AddrRegReqBuf` |
| `0xFFFF:C8029000` | response | `FirmwareReadResponse` reads it | `AddrRegResp` |
| (`0xFFFF:C8029040`) | response buffer | not used by the kext | `AddrRegRespBuf` |

All through `[this+0xe0]` (the `IOFireWireNub` provider):
- **`FireWireWriteCommand(addr, len, data) @0x1bd3e`**: copies `data` into an `IOBufferMemoryDescriptor`, builds an `IOFWWriteCommand` targeting the nub, sets the command timeout field `[cmd+0x48] = 10,000,000` (µs, 10 s — per the field's use; **unit assumed**), submits (`vt[0x140]`) and returns the status; allocation failures return `0xE00002BC`.
- **`FireWireReadCommand(addr, len, out) @0x1c06e`**: same with `IOFWReadCommand`; up to **10 attempts**; when the status is `0xE00002D7` it `IOSleep(2500)` and retries unless `[+0xf4]` (stopped) is set or the object is inactive; on success `memcpy`s the data out. (The read command also has a 10 s timeout field.)
- **`FirmwareReadResponse(resp, len) @0x1c1f2`**: reads `len` bytes from `0xFFFF:C8029000` with up to **4 attempts**; when the status is `0xE00002D5` or `0xE00002D6` it `IOSleep(2000)` before the next attempt; each failed submission logs `"M-FW: Retry (%u) status %X submitting read command"`.
- **`FirmwareWriteCommand(hdr*, len) @0x1be3c`**: writes `len` bytes to the request register after setting `hdr[0] = [this+0x98]`.

## 3. `MAFirmwareInfo` (A1-Q19): the 80-byte blob

`FirmwareCacheInfoRegisters @0x1cc9e` reads 0x50 bytes from `0xFFFF:C8020000` into `this+0x90..0xDF`, sets **`[+0xf0] = (word at [+0x9c] != 0)`** and publishes the bytes as `MAFirmwareInfo` (`OSData`). Layout (FFADO `info_register_t`, `bebob_dl_mgr.h:96`; numeric fields are **little-endian in the raw block**, text fields are ASCII), checked against a **live runtime-mode value** from the SL rig (`B5-sl-dtrace-truth/captures/12-ioreg-full.txt:3920`):

| Offset | Size | FFADO name | Live value (SL, runtime mode) | Kext use |
|---|---|---|---|---|
| 0x00 | 8 | `m_manId` | `"bridgeCo"` | — |
| 0x08 | 4 | `m_protocolVersion` | 1 | **`[this+0x98]`** = first quadlet of every command (the "protocol version") |
| 0x0C | 4 | `m_bootloaderVersion` | 0 | **`[this+0x9c]`**; non-zero ⇒ bootloader mode ⇒ `[+0xf0] = 1` |
| 0x10 | 8 | `m_guid` | `04 6c 0d 00 fb b9 74 00` = GUID `000d6c040074b9fb` | (GUID property copied separately) |
| 0x18 | 4 | `m_hardwareModelId` | 0x83 | — |
| 0x1C | 4 | `m_hardwareRevision` | 1 | — |
| 0x20 | 8 | `m_softwareDate` | `"20070713"` | helper: firmware date |
| 0x28 | 8 | `m_softwareTime` | `"080440\0\0"` | helper: firmware time |
| 0x30 | 4 | `m_softwareId` | 0 | — |
| 0x34 | 4 | `m_softwareVersion` | 0 | — |
| 0x38 | 4 | `m_baseAddress` | 0x20080000 | — |
| 0x3C | 4 | `m_maxImageLen` | 0x00180000 | — |
| 0x40 | 8 | `m_bootloaderDate` | `"20040330"` | — |
| 0x48 | 8 | `m_bootloaderTime` | `"025909\0\0"` | — |

(FFADO's struct continues past 0x50 with debugger fields; the kext does not read them.) This resolves: A4-H2's "`[0xc]` = state flag (0 running, non-zero ready for upload)" is the **bootloader version**; its "`[0x20..0x2f]`" firmware date/time are `softwareDate`+`softwareTime`; the live date `20070713 080440` equals the date in `fw1814.bcd` (`0x08`/`0x10`), which is why the reference helper finds "dates match → just start" on a current device; and macfw's "image base 0x20080000 / max 0x00180000" are the info words at 0x38/0x3C. The bytes `04 6c 0d 00` of the GUID are M-Audio's OUI `000d6c` plus `04`.

**This corrects an earlier reading:** `this+0x98` (called `fNodeHandle` in the Q7 field table and "written into FireWireRequestHeader[0]") is the cached **bootloader protocol version**, not a node handle. The kext's 12-byte "cue" `{[+0x98], 0x01110000, 0}` is therefore `{protocol version 1, Go, start-mode 0 = application}`, and on the wire it is byte-identical to macfw's `01 00 00 00 00 00 11 01 00 00 00 00` for a device whose protocol version is 1 (verified: version = 1 in the live block).

## 4. Command format

A command is written as host-memory quadlets to `0xFFFF:C8021000`: `q0 = protocolVersion ([this+0x98])`, `q1 = (operandSize << 24) | (commandCode << 16) | commandId` (so the raw bytes are `[id lo][id hi][code][operand size]`, exactly FFADO's `CommandCodes::serialize`, `bebob_dl_codes.cpp:29-…`; the kext always uses `commandId = 0`), then the operands. Codes are FFADO's `EBootloaderCommandCodes`:

| Kext site | q1 | Code | Operands (quadlets) | Total written |
|---|---|---|---|---|
| `FirmwareHalt @0x1bff2` (only if `[+0xf0] == 0`) | `0x01020000` | 0x02 `Reset` | `1`: start-mode = **1 (bootloader)** | 12 B, then `ReregisterDeviceAfterFirmwareReset` |
| `FirmwareReset(mode) @0x1bf92` | `0x01020000` | 0x02 `Reset` | `mode` (caller-supplied; no caller found in this pass) | 12 B + re-register |
| `FirmwareStart @0x1beb4` (bootloader mode) | `0x01110000` | 0x11 `Go` | `0`: start-mode = application | 12 B + `ReregisterDeviceAfterFirmwareReset` |
| `DownloadSectionStart @0x1c468` | `0x0A040000` | 0x04 `DownloadStart` | 10: `object, date(2), time(2), id, version, loadAddress, length, crc` | 48 B (0x30); response 0x28 B |
| `DownloadSendNextBlock @0x1c332` | `0x03050000` | 0x05 `DownloadBlock` | 3: `sequence, address, byteCount` | 20 B (0x14) after the payload is written to `0xC8021040`; response 0x10 B |
| `DownloadStop @0x1c28c` | `0x00060000` | 0x06 `DownloadEnd` | header says 0 operands, **one trailing quadlet = the section CRC** | 12 B; response 0x28 B |

Object codes (`object` operand): 0 = application image, 1 = config/"CNE" (FFADO `CommandCodesDownloadStart::EObject`). `date/time/id/version` are copied verbatim from `.bcd` bytes `0x08..0x1F` (`"20070713"` at `0x08`, `"080440  "` at `0x10`, id `0x18`, version `0x1C`; zeros for `fw1814.bcd`). Differences from FFADO to note: FFADO's `DownloadEnd` request carries no operands and FFADO compares the device-reported CRC in the response (`bebob_dl_mgr.cpp:528`); the kext appends its expected CRC to the request and **does not compare** — it reads the response word at offset `+0xC` and logs `"M-FW: FirmUp- Successful upload"` when it is 0, `"…Upload failed with error $%X"` otherwise. What the device does with the appended CRC is **unverified**.

## 5. The update procedure

1. **Intake** (`SetOneProperty`, `0x1ca24`): `MAFirmwareUpgradeImage` must be an `OSData` (else the call returns false); `[+0x140] = bytes`, `[+0x138] = length`; if the bytes pointer is NULL, or **`[+0xf0] == 0` (device not in bootloader mode), the call returns false without doing anything**; otherwise `FirmwareUpdate()`; the call returns `(status == 0)`.
2. **`FirmwareUpdate @0x1c908`**: `[+0xf2] = 0`, **`[+0xf1] = 1`** (an "update in progress" flag — this is the byte Q7 calls `fCached_IsRequestingClose`: while it is set, the `kIOFWMessageServiceIsRequestingClose` handler copies it to `[+0xf2]`, so a bus reset during the update marks the service as closing), **`fState = 5`** (`0x1c924`). It then dispatches on the **image base address in the `.bcd` header**, `bcd[0x34]`: `0x20080000` → `FirmwareUpdateBeBoB251`; `0x400C0080` or `0x400B0080` → `FirmwareUpdateBeBoB32`; anything else → log `"M-FW: FirmUp- Unknown firmware base address %X"` and fail with `0xE00002BC`. For `fw1814.bcd`, `bcd[0x34] = 0x20080000` → the 2.5.1 path.
3. **`FirmwareUpdateBeBoB251 @0x1c75a`** (two sections, 512-byte blocks):
   - image: `DownloadSectionStart(object 0, load address bcd[0x34], length bcd[0x38], crc bcd[0x3c], data offset bcd[0x30])`; loop `DownloadSendNextBlock` until the byte counter reaches the length (logs every 100 KiB: `% 0x19000`); `DownloadStop`.
   - config: `DownloadSectionStart(object 1, address 0, length bcd[0x58], crc bcd[0x5c], data offset bcd[0x50])`; loop; `DownloadStop`.
   - Any non-zero block status aborts and is returned.
   - With the real `fw1814.bcd` (header read from the file: `0x30 = 0x70`, `0x34 = 0x20080000`, `0x38 = 0xE4A9C`, `0x3C = 0xB84C5237`, `0x50 = 0xE4B0C`, `0x58 = 0x146C`, `0x5C = 0x211FE87A`): image 936,604 bytes → 1,830 blocks; config 5,228 bytes → 11 blocks.
4. **`FirmwareUpdateBeBoB32 @0x1c5f2`** (firmware with base `0x400C0080`/`0x400B0080`): starts both sections (`DownloadSectionStart` for object 0 and object 1), then **resets the counters to stream the whole file from offset 0** (`[+0x148] = 0`, `[+0x178] = 0`, length `[+0x138]`, crc `bcd[0x3c]`) in one pass and `DownloadStop`s once (log `"Starting 3.2 or later upload"`). Not used by the FW1814's current image; relevant to other products.
5. **Block mechanics** (`DownloadSendNextBlock`): block length `[+0x158]` = 0x200, shortened for the last block; payload = `bcd + [+0x148] + [+0x150]` written to `0xC8021040`; then the 20-byte command; then `FirmwareReadResponse(0x10)`; `[+0x150] += length`, `[+0x168]++` (sequence); `more = (length > [+0x150])`. `DownloadSectionStart` initialises `[+0x148]` = data offset, `[+0x178]` = load address, `[+0x160]` = length, `[+0x150] = 0`, `[+0x168] = 0`, `[+0x158] = 0x200`, `[+0x180]` = crc and logs `"M-FW: FirmUp- upload start successful"`.
6. **After the update:** `FirmwareUpdate` logs the status, clears `[+0xf1]`. **On success it returns directly; the `resetBus` (`bus->vt[0x880]`, `0x1c9ea`) + `IOSleep(0xdac)` (3500 ms) is on the *error* path only** (`0x1c9cf`: log `"M-FW: FirmUp- Error during firmware update, %X"`, reset the bus, sleep). *(Earlier notes — B2 Correction, C1 §3.3, C3 §8, the round-two review — said the path always ends with a bus reset; that was wrong.)* The device is then started by the helper's separate `MAFirmwareStart` write (`FirmwareStart`, bootloader branch: Go + `ReregisterDeviceAfterFirmwareReset`, C1 §3.4).

## 6. State fields touched (`com_m_audio_FWMetaNub`, instance 0x188 bytes)

`[+0x88]` fState (0..5, C1 §3); `[+0x90..0xDF]` cached info block (`[+0x98]` protocol version, `[+0x9c]` bootloader version); `[+0xe0]` provider nub; `[+0xe8]` controller (for `resetBus`); `[+0xf0]` bootloader-active; `[+0xf1]` update-in-progress (Q7's `fCached_IsRequestingClose`); `[+0xf2]` service closing; `[+0xf3]` firmware-match-started; `[+0xf4]` stopped; `[+0x138]` image length; `[+0x140]` image pointer; `[+0x148]` section data offset; `[+0x150]` bytes sent; `[+0x158]` block size; `[+0x160]` section length; `[+0x168]` block sequence; `[+0x178]` load address; `[+0x180]` section CRC.

## 7. What the clone needs (D2)

- Same three property keys on the nub, same intake rules (OSData, bootloader-mode only for the image; value ignored for Start/Halt), same dispatch on `bcd[0x34]`. The clone's helper reads `MAFirmwareInfo` exactly as the reference helper does (`[0x08]` is the protocol version it can pass through; `[0x0C] != 0` means bootloader mode; `[0x20..0x2F]` the date/time).
- Implement the three command encodings of §4 byte-for-byte, the 512-byte block loop, the retry/sleep rules of §2, and the status handling of §5 (including the trailing CRC quadlet of `DownloadEnd`).
- The error path's bus reset + 3.5 s sleep is part of the reference behaviour; the success path has none.
- Test order (BUILD-PLAN): this path is exercised last and only with the user present (a failed write could leave the device in the bootloader; recovery is `MAFirmwareStart`/power-cycle).

## 8. Unverified / open

- Units of the 10,000,000 command timeout field; the exact numeric `IOReturn` names for `0xE00002D5/D6/D7` (numbers are from the asm).
- What the BeBoB bootloader does with the CRC quadlet appended to `DownloadEnd`, and the meaning of the other response words (the kext only checks word `+0xC`).
- Behaviour of `FirmwareUpdateBeBoB32` on a real device (no FW1814 image uses it).
- Any caller of `FirmwareReset(mode)` other than `FirmwareHalt`.
- Everything on 10.13.6 (not run there).
- The 0x28-byte response layout is not decoded beyond word `+0xC`; FFADO's `CommandCodes::deserialize` (version, header, operands) is the likely layout.

## Provenance
`mafw.asm`: `SetOneProperty @0x1ca24`, `FirmwareUpdate @0x1c908`, `FirmwareUpdateBeBoB251 @0x1c75a`, `FirmwareUpdateBeBoB32 @0x1c5f2`, `DownloadSectionStart @0x1c468`, `DownloadSendNextBlock @0x1c332`, `DownloadStop @0x1c28c`, `FirmwareReadResponse @0x1c1f2`, `FireWireWriteCommand @0x1bd3e`, `FireWireReadCommand @0x1c06e`, `FirmwareReset @0x1bf92`, `FirmwareHalt @0x1bff2`, `FirmwareStart @0x1beb4`, `FirmwareCacheInfoRegisters @0x1cc9e`; decompiles `ghidra-exports/decomp/FWMetaNub__{DownloadSectionStart,DownloadSendNextBlock,DownloadStop,FirmwareUpdateBeBoB251,FirmwareUpdateBeBoB32,FireWireReadCommand,FireWireWriteCommand,FirmwareWriteCommand,FirmwareReset,FirmwareHalt,FirmwareStart,FirmwareCacheInfoRegisters}.c`. FFADO: `bebob_dl_codes.{h,cpp}`, `bebob_dl_mgr.{h,cpp}:45-49,96-115,508-535`. Live value: `B5-sl-dtrace-truth/captures/12-ioreg-full.txt:3920`. `.bcd` header read from `_extracted/firmware/fw1814.bcd` (A5).
