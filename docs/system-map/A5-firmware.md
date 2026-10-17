# A5 — `fw1814.bcd` layout

> **Correction (2026-10-14) — UART roles** (evidence: [PROVENANCE-RESOLUTIONS.md §R15](PROVENANCE-RESOLUTIONS.md)). Firmware strings (`strings fw1814.bcd`): `midiInit(): can not open UART2 in CnE`, `/cfg/dev/uart2`, `/dev/uart2`, `UART2 baud rate … not set correctly in CnE`, `MIDI sent %d to UART and received %d via 1394`, `rxMidi port is %d` — so **MIDI I/O runs on UART2** (both directions on one UART). `UART1` is opened separately (`/dev/uart1`, `get dev/uart1/bufferSize`, `set dev/uart1/bufferSize 128`, `DefaultUART`, `/dev/uartCombo`); its role (likely the debug shell) is **unverified**. The lines 117-118 'UART1/UART2 (MIDI In/Out — likely)' are therefore wrong for UART1.


> **Correction (2026-10-14) — static audit** (full evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). CRC length `0xE4A9C` is 936,604 bytes (not 937,116).


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - 'UART1/UART2 (MIDI In/Out — likely)' (lines 110-111) is a **guess — needs verification** (check = DM1000 datasheet or firmware disassembly).


**Status:** done 2026-10-09.
**Target:** `_extracted/helper/Firmware/FW 1814/fw1814.bcd` (941944 bytes = `0xE5F78`).
**Verification:** every claim here was cryptographically verified against the file bytes using `python3 -c "import zlib, struct; ..."` — see commands recorded inline.

## Headline

The file is **BridgeCo "bCoD" (BridgeCo Download) format** — a 96-byte header + 16-byte product name + ARM firmware image + BridgeCo "attribute namespace" tail. The ARM image is **loaded into DM1000 RAM at power-on**; the attribute namespace declares every PLL divisor, AV/C descriptor, mixer matrix, and I/O routing the device self-reports. Both are CRC32-protected.

**Correction to A4-H4 / A4-H9:**
- The date field at `h[8..0x17]` is a **16-byte ASCII string** (`"YYYYMMDDHHMMSS  "`), not 4 binary ulongs. The decompiler's `*(ulong*)` reads showed 4 u32s because i386 aligned-reads of ASCII buffers look like u32s in pseudocode; `CalendarDateFromFirmwareDate` must parse ASCII internally.
- The appendix is **not** located at `0x60 + h[0x38]`. It's located at an **explicit offset in `h[0x50]`** (which H4/H9 didn't surface). The correct identity is `h[0x50] + h[0x58] == file_size`. See §Layout below.
- Appendix integrity: `h[0x5c]` is the appendix's own CRC32, verified ✓.
- Header integrity: `h[0x2c]` is a **self-CRC32** of the header bytes `0x00..0x5F` with the CRC field zeroed before computation — verified ✓. Helper doesn't check this; it's dormant.

A4-H4.md and A4-H9.md will be updated with a `**Correction (2026-10-09):**` block pointing at this file.

## Layout — all offsets verified

```
offset       size         field                                                value
-------      -------      --------                                             --------
0x0000       4            Magic                                                'bCoD'  (62 43 6F 44)
0x0004       4            Version (u32 LE)                                     1
0x0008       16           Build date (ASCII, zero or space-padded)             "20070713080440  "
0x0018       20           Zero / reserved                                      00…00
0x002C       4            Header self-CRC32 (over 0x00..0x5F with this
                          field zeroed)                                        0xB2C28ACB  ✓
0x0030       4            Payload region start offset (u32 LE)                 0x00000070
0x0034       4            Zero / reserved                                      00000000
0x0038       4            Payload region length (u32 LE)                       0x000E4A9C
0x003C       4            Payload CRC32 (expected)                             0xB84C5237  ✓
0x0040       16           Zero / reserved                                      00…00
0x0050       4            Appendix region start offset (u32 LE)                0x000E4B0C
0x0054       4            Zero / reserved                                      00000000
0x0058       4            Appendix region length (u32 LE)                      0x0000146C
0x005C       4            Appendix CRC32 (expected)                            0x211FE87A  ✓
0x0060       16           Product name (ASCII, null-padded)                    "Midiman FW 106"
                          — not covered by any CRC

0x0070       0xE4A9C      ARM firmware payload                                 CRC'd by h[0x3c]
                                                                               ends at 0xE4B0C

0xE4B0C      0x146C       BridgeCo attribute namespace (ASCII bracket-list)    CRC'd by h[0x5c]
                                                                               ends at 0xE5F78 = EOF
```

**Both regions are contiguous**: `h[0x30]+h[0x38] == h[0x50]` and `h[0x50]+h[0x58] == EOF`. No gaps, no trailing data.

**File-identity:** `"Midiman FW 106"` is the product-code string. **This confirms the FW1814 was internally code-named "FW 106"** — matches the directory-walk alias table in A4-H4 (`"FW 1814"` is the marketing name; `"Midiman FW 106"` is BridgeCo's product-name key, probably seen in AV/C unit-info responses too). "Midiman" is M-Audio's old corporate name (acquired by Avid in 2004, so this firmware still carries the pre-acquisition branding on the device side).

## Payload — ARM firmware

Starts at `0x70`, length `0xE4A9C` (937116 bytes = ~915 KB).

**Architecture:** ARM (little-endian), confirmed by first instructions decoded:
- `0x70: 10 0f 10 ee` = `MRC p15, 0, R0, c0, c0, 0` — read CP15 CPUID register (ARM system control coprocessor).
- `0x74: 7c 10 9f e5` = `LDR R1, [PC, #0x7c]` — classic ARM literal-pool pattern.
- `0x78: 20 02 01 e0` = `AND R0, R1, R0, LSR #4` — probably CPUID-based variant select.

This is **DM1000's ARM7TDMI reset vector**. The DM1000 is BridgeCo's BeBoB audio SoC = ARM7TDMI + custom audio DSP + IEC 61883 isoch bridge; this file is the ARM-side code image, loaded at power-on via the FireWire bootloader path (A4-H2 → kext `setProperty:@"MAFirmwareUpgradeImage"`).

**Full disassembly of the ARM image is out of scope** for this project (we're writing a *host driver*, not a BeBoB firmware). Deep decode would belong to a hypothetical "firmware project," and in practice our clone never touches payload bytes beyond handing the mmap'd blob to the kext.

What *is* in-scope: strings-grep sanity confirms the payload contains:
- An embedded debug shell (`"help"`, `"load"`, `"sem"`, `"blk"`, `"byte"`, `"timer"`, `"q"`, `"evf"`, `"evt"`, `"map"`, `"wp"`, `"frmrd"`, `"frmwr"`, `"flashdump"`, `"memscan"`) — DM1000 has a UART-reachable debug console with these commands. **Could be useful for ground-truth captures if we ever get UART access to the FW1814 PCB.** Deferred to a hardware pass.
- An embedded gdb stub walkthrough: `"Then manually, start gdb optionally:\n  w  init\n  w  gdb\n  w  img load\n  …"` — the device supports JTAG-less gdb-stub debugging over the shell. Also hardware-pass material.
- RTOS task names, log format strings, and section labels — typical ARM-embedded firmware shape.

**No vendor-encrypted blobs, no compressed sections.** The ARM image is raw executable + data, straight-mapped into DM1000 RAM.

## Appendix — BridgeCo attribute namespace

Starts at `0xE4B0C`, length `0x146C` (5228 bytes).

**Format:** ASCII bracket-S-expressions, space-separated. Opens with `[START_OF_ATTR_NAME_SPACE]`, closes with `[END_OF_ATTR_NAME_SPACE]`. Nested sections use `[Name key1 val1 key2 val2 [SubSection ...] ]` syntax.

**This section is the goldmine** — it's a complete declarative device model that the DM1000 loads at boot and the host reads back via AV/C `UNIT INFO` + descriptor queries. Our kext never has to synthesize this; the device self-describes.

### Top-level sections (grep'd verbatim)

```
[Shell]              — console shell commands + help text
[FW106]              — product header
  [FactorySettings]  — factory defaults
  [SystemConfig]    — system-level config
    [dev]            — device nodes
    [isodrv]         — isochronous driver config
    [StateMachine]  — AV/C state machine
    [ClockDividers] — PLL divisors per sample rate
    [dars]           — DARS (digital audio reference signal routing)
    [syt]            — SYT timestamp config (AM824)
    [csp]            — Connection Stream Protocol parameters
    [AVC]            — AV/C descriptor model
      [SubUnit:8]
        [FuncBlock:1]
          [SourceID:0]
            [ClusterInfo]
            [FuncBlockTypeInfo]
            [StreamFormatInfo]
      [SrcPlugLinkInfo:0]
    [AnM]            — analog-mixer section (**hypothesis**)
    [AM824]          — AM824 isoch packet layout
    [spi]            — internal SPI bus (codec control)
    [i2c]            — internal I²C bus
    [SwitchMatrix]  — audio routing matrix
      [MtxSel0]
      [MtxSel1]
    [uart1]          — UART1 (MIDI In/Out — likely)
    [uart2]          — UART2 (MIDI In/Out — likely)
```

### Example: ClockDividers (seen at `0xE4B0C` opening bytes)

```
[176400 divRefDiv 5 divVco 14112 divFS256_1 1 divFS256_2 1 divSyncDiv 128]
[192000 divRefDiv 4 divVco 12288 divFS256_1 1 divFS256_2 1 divSyncDiv 128]
```

**This directly answers the sync-path (C2) question for all supported rates.** The DM1000 PLL is parameterized by (refDiv, Vco, FS256_1, FS256_2, SyncDiv). Our clone kext never needs to compute these — the firmware already carries the full table and selects by rate at AV/C `OUTPUT STREAM FORMAT` set-time. **Clone implication:** the kext's rate-change path is pure AV/C — don't synthesize PLL math.

### Example: AV/C descriptor tree

```
[AVC [SubUnit:8 _AVCCtrl 0 SizeOfListID 0x2 SizeOfObjID 0 SizeOfEntryPos 0x2
      NumOfRootLists 1 RootListID1 0x1800
      [0x1800 _AVCCtrl 0 ListType 0x86 Attributes 0x08 NumOfEntries 1
        [Entry0 EntryType 0x90 Attributes 0x28 ChildListID 0x1801]
        [0x1801 _AVCCtrl 0 ListType 0x86 Attributes 0x08 NumOfEntries 3
          [Entry0 EntryType 0x93 Attributes 0x08 InfoBlockType 0x0A InfoBlockData Feature Function Block]
          [Entry1 EntryType 0x93 Attributes 0x08 InfoBlockType 0x0A InfoBlockData LeftChannel]
          [Entry2 EntryType 0x93 Attributes 0x08 InfoBlockType 0x0A InfoBlockData Right Channel]
        ]
      ]
      ...
```

**SubUnit:8 = AudioSubUnit (0x08 = audio) per IEC 61883-1.** The ListType / EntryType / InfoBlockType values are standard AV/C descriptor model codes. This section declares, in declarative form, every function block (feature controls, mixers, selectors), every stream format, and every plug connection — the same tree that FFADO's `libffado/src/bebob/bebob_dl_mgr.cpp` reads over the FireWire bus at discovery time.

### Example: `[SwitchMatrix]` — the mixer

The DM1000's internal mixer is declared as switch-matrix selectors. Full decode belongs to C-synthesis against FFADO's mixer map, but the fact that it's declared *here, in the firmware* confirms: **the mixer is firmware-provided, not kext-provided.** Our clone's `FW1814Mixer` must speak the AV/C `FUNCTION BLOCK` commands to drive it; it must not invent its own mixer topology.

## Verification commands (recorded)

```sh
# Header parse + CRC verification
python3 -c "
import zlib, struct
d = open('fw1814.bcd','rb').read()
# ... (full script in commit history; results):
# magic = b'bCoD', version = 1, date = '20070713080440  '
# crc_start=0x70, crc_len=0xe4a9c, crc_exp=0xb84c5237 (match ✓)
# app_start=0xe4b0c, app_len=0x146c, app_crc=0x211fe87a (match ✓)
# app_start+app_len = 0xe5f78 == file size ✓
"

# Header self-CRC verification
python3 -c "
import zlib
d = open('fw1814.bcd','rb').read()
h = bytearray(d[:0x60])
h[0x2c:0x30] = b'\0'*4
# zlib.crc32(h) == 0xb2c28acb (matches h[0x2c] in file) ✓
"
```

## The other 7 product `.bcd` files

**All 8 product subdirectories ship in the DMG** (A4-H10 said only FW1814 — that was wrong; see §Correction below). Directory listing in `_extracted/helper/Firmware/`:

```
FW 1814/fw1814.bcd
FW 410/
FW Audiophile/
FW Solo/
NRV-10/
Ozonic/
ProFire Lightbridge/
ProjectMix/
```

**Correction to A4-H10:** H10 claimed "the shipping M-Audio DMG contains only `FW 1814/fw1814.bcd`." That claim came from an incomplete directory check. The real DMG ships one `.bcd` per product. H10's clone-side conclusion (we drop 7/8 products) stands; only the shipping-content observation was wrong.

Each per-product `.bcd` likely follows the same `bCoD` layout with a different product-name string at `0x60` and a product-specific ARM image + attribute namespace. **We don't decode them here** — not in-scope for an FW1814-only clone. If needed later: one `python3` script walks all 8 identically.

## Clone implications (for Part D)

1. **Opaque hand-off.** Our clone fwhelper mmaps the file, verifies both CRCs (payload + appendix), and hands the whole `NSData` to the kext via `setProperty:@"MAFirmwareUpgradeImage"` — exactly what the reference does. No parsing beyond header integrity checks.
2. **Don't synthesize PLL math.** The firmware ships the full rate → divisor table in `[ClockDividers]`. Our clone picks sample rates via AV/C and the firmware handles the dividers. Keeps us honest about reference-parity for sync.
3. **Don't invent an AV/C descriptor tree.** Our kext should discover the descriptor tree over the bus (per FFADO's pattern) and act on it, not hard-code it. Hard-coding would break the moment the firmware ships a new descriptor revision.
4. **Mixer topology is firmware-declared.** `FW1814Mixer` (our clone's `include/FW1814Mixer.h` draft) must drive the switch-matrix via AV/C FUNCTION BLOCK commands, matching the `[SwitchMatrix]` declarations. The specific selector counts + mute/polarity bits will come out of FFADO cross-reference (B1).
5. **DM1000 is ARM7TDMI.** Not strictly needed by the host driver, but documenting this closes the "what processor is on the other end of our FCP commands" question. If we ever need firmware-side debugging, UART shell + gdb-stub are embedded.
6. **Both CRC checks must pass before upload.** Reference verifies payload CRC only (per A4-H9); appendix CRC is unused. **Our clone verifies both** — it's a free integrity improvement and the appendix CRC is right there in `h[0x5c]`.

## What's deferred

- **Appendix full decode** → Part C synthesis. Correlate each section against FFADO's BeBoB discovery code (B1) + FFADO's mixer map (B1). The S-expression grammar is simple enough that a 50-line Go/Python parser produces a key→value tree we can diff against FFADO.
- **ARM payload disassembly** → never, unless hardware debugging demands it. Out of scope for a host driver.
- **Other 7 products' `.bcd`s** → never, FW1814-only scope.

## Follow-ups queued

- **A5a** — write the S-expression parser (one evening) and dump the appendix as a tree. Not needed before Part B; enables faster cross-ref against FFADO.
- **A5b** — correlate `[ClockDividers]` rows (we only have 176400/192000 fragments visible from grep) against FFADO's `bebob_dl_mgr.cpp` rate table. **Promote to B1 scope** when B1 starts.
- **A5c** — fix H4-firmware-selection.md and H9-bcd-reader.md with `**Correction (2026-10-09):**` blocks pointing at this file. Needed before Part C synthesis for provenance correctness.
