# H4 — Firmware file selection + validation (and heavy A5 payoff)

> **Correction (2026-10-14) — static audit** (full evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Alias literals are `"FW"`→folder `"FW 410"` and `"FWSolo"`→`"FW Solo"` (asm 0x5b14-0x5b49); the pseudocode's `FW `/`FW410`/`FWSolo` were wrong. `.bcd` date fields are ASCII (confirmed on the real file), CRC32 over `[0x70,0x70+0xE4A9C)` matches `[0x3c]`. `validateFirmwareUpdate:` checks **length ≥ 0x60+[0x38]+[0x58]** (minimum, `jae`), not equality.


**Correction (2026-10-09):** this doc's byte-level BCD-header claims are **superseded by `A5-firmware.md`**. Two errors here, corrected in A5:
1. The date at `h[8..0x17]` is a 16-byte **ASCII string** (`"20070713080440  "`), not 4 binary ulongs. The decompiler's `*(ulong*)` reads looked like u32s because i386 aligned-reads of an ASCII buffer land on word boundaries; `CalendarDateFromFirmwareDate` parses ASCII internally.
2. The appendix is **not** at `0x60 + h[0x38]`. Its offset is explicitly stored at `h[0x50]`. Correct identity: `h[0x50] + h[0x58] == file_size`. Verified cryptographically against `fw1814.bcd` in A5-firmware.md.

The behavioral conclusions below (product-name alias table, strip-"Bootloader", two-aliases rule, firmware-filename pattern) are **unchanged**.

**Target:** `_extracted/helper/M-Audio FireWire Device Helper` (i386).

**Decomp log:** `ghidra-exports/decomp/a4-h4-firmware-selection.log` — `+[MAFireWireFirmwareFile firmwareFilesAtPath:forDevice:]` (lines 1-55), `+firmwareWithPath:` (59-82), `-validateFirmwareUpdate:` (86-153), `-embeddedCRC` (157-163), `-actualCRC` (167-176), `CalendarDateFromFirmwareDate` (180-276), `CanonicalGUIDForDevice` (280-301), `-reportCorruptFirmwareToDelegate:format:` (317-335), `-date` (339-351), `-dealloc` (355-371).

## Finding 1 — Product-name → folder mapping is **strip-"Bootloader" + two aliases**

`+firmwareFilesAtPath:forDevice:` (log:1-55):

```
+firmwareFilesAtPath:fwDir forDevice:dev
    results = [NSMutableArray array]
    name    = [dev propertyForKey:@"FireWireProductName"]
    r       = [name rangeOfString:@"Bootloader"]
    if (r != NSNotFound):
        name = [name substringToIndex:r]        // strip trailing " Bootloader"
    if ([name isEqualToString:@"FW "]):
        folder = @"FW410"                       // alias: the generic "FW Bootloader" product → FW410
    else if ([name isEqualToString:@"FWSolo"]):
        folder = @"FWSolo"                      // explicit identity (defensive)
    else:
        folder = name                            // identity for everything else
    dir = [[fwDir stringByAppendingPathComponent:folder] stringByStandardizingPath]
    files = [[[NSFileManager defaultManager] subpathsAtPath:dir]
             pathsMatchingExtensions:@[@"bcd"]]
    for p in files:
        full = [dir stringByAppendingPathComponent:p]
        [results addObject:[MAFireWireFirmwareFile firmwareWithPath:full]]
    return results
```

**For FW1814**, `FireWireProductName` is `"FW 1814"` (runtime) or `"FW 1814 Bootloader"` (bootloader mode). Both strip to `"FW 1814"` → doesn't hit either alias → folder = `"FW 1814"` → `_extracted/helper/Firmware/FW 1814/fw1814.bcd` is the sole hit. Ground-truth confirmed against the on-disk layout (`ls _extracted/helper/Firmware/FW\ 1814/`).

**unverified aliases need confirming on real hardware:**
- `"FW "` → `"FW410"`: **hypothesis** — the FW 410 publishes `"FW Bootloader"` as its bootloader-mode product name (no `410` suffix). Verify by reading the FW 410's `FireWireProductName` CFProperty off a live FW 410 (we don't have one, so cite from FFADO's `maudio/` or snd-bebob — likely in Part B).
- `"FWSolo"` identity branch: redundant for the FW Solo but included defensively; **unverified** — maybe the Solo publishes `"FWSolo"` without a space (unlike `"FW 1814"` which has one).

## Finding 2 — `+firmwareWithPath:` memory-maps the `.bcd` and caches the header pointer

`+firmwareWithPath:` (log:59-82):

```
+firmwareWithPath:path
    self = [[[self alloc] init] autorelease]
    self->path     = [path retain]                  // ivar @ +0x0c (NSString)
    self->name     = [[path lastPathComponent] retain]  // ivar @ +0x08
    self->data     = [[NSData dataWithContentsOfMappedFile:path] retain]  // ivar @ +0x04
    self->header   = [data bytes]                    // ivar @ +0x14 (void*, into mapped file)
    return self
```

`dataWithContentsOfMappedFile:` → **memory-mapped file**, not a copy. For `fw1814.bcd` on 2026-10-09 this is fine (file is ≤ a few MB) but note it: concurrent writers to the file would corrupt the active decode. Not a real concern on-target.

Additional ivars discovered via dealloc + validate:

| Offset | Type | Field | Set in |
|---|---|---|---|
| +0x04 | NSData* | mapped-file data | firmwareWithPath: |
| +0x08 | NSString* | filename (lastPathComponent) | firmwareWithPath: |
| +0x0c | NSString* | full path | firmwareWithPath: |
| +0x10 | NSCalendarDate* | parsed firmware date (nil until validate) | validateFirmwareUpdate: |
| +0x14 | void* | bytes pointer = header base | firmwareWithPath: |
| +0x69 | char | "validation attempted" flag (fast-path gate) | validate's own entry gate |
| +0x6a | char | "is valid" result flag | validate on success |

## Finding 3 — BCD header layout, decoded end-to-end

`-validateFirmwareUpdate:` (log:86-153) is a tight structural decode. Combining with `-embeddedCRC`, `-actualCRC`, `-date`:

| Offset | Size | Field | Decode citation |
|---|---|---|---|
| `[0x00..0x07]` | 8 | **unverified** — probably product/version string or magic. Not touched by the helper. | — |
| `[0x08]` | 4 (ulong) | **date field 1** (year, likely) | log:339-351 passes `+8` as first arg; `CalendarDateFromFirmwareDate` builds `"%Y %m %d %H %M %S"` so 1st arg = year |
| `[0x0c]` | 4 | **date field 2** (month) | log:349 (+0xc as 2nd arg) |
| `[0x10]` | 4 | **date field 3** (day) | log:349 (+0x10 as 3rd arg) |
| `[0x14]` | 4 | **date field 4** (hour / combined HMS) | log:349 (+0x14 as 4th arg); CalendarDateFromFirmwareDate stack layout embeds extra bytes so this ulong may encode H+M+S packed — **unverified**, needs ground-truth decode on real `fw1814.bcd` |
| `[0x18..0x2f]` | 24 | **unverified** — product name, model id, build tag? Not touched. | — |
| `[0x30]` | 4 | **CRC range start** (byte offset in file where CRC32 begins) | log:172-174 `CRC32WithRange: location=header[0x30]` |
| `[0x34]` | 4 | **unverified** — not touched | — |
| `[0x38]` | 4 | **CRC range length = payload length** | log:119, 172-174 (used both for file-size sanity + CRC range) |
| `[0x3c]` | 4 | **embedded CRC32** (expected) | log:162 |
| `[0x40..0x57]` | 24 | **unverified** — not touched | — |
| `[0x58]` | 4 | **CRC-excluded appendix length** (extra tail bytes beyond the CRC'd payload) | log:119-120 total-size calc = `0x60 + header[0x38] + header[0x58]` |
| `[0x5c]` | 4 | **unverified** — not touched | — |
| `[0x60..]` | var | **payload** starts here | log:114 min length = `0x60` → header is exactly 96 bytes |

Validation sequence (`-validateFirmwareUpdate:` log:99-153):

1. Shortcut: if `self->[0x69]` already set, return cached `self->[0x6a]`.
2. Assert `data != nil` (log:102-108 — assertion handler cites `/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/DeviceHelper/MAFireWireFirmwareFile.mm:274`).
3. `dataLen >= 0x60` else "Corrupt: should be at least %lu bytes long, but is actually %d bytes long" (log:114-116).
4. `dataLen >= 0x60 + header[0x38] + header[0x58]` else "Corrupt: embedded length is %lu bytes, but is actually %d bytes long" (log:119-123).
5. `actualCRC == embeddedCRC`:
    - `actualCRC` = `[data CRC32WithRange:(location=header[0x30], length=header[0x38])]` (log:172-174). **Hypothesis** — `CRC32WithRange:` is an NSData category delegating to libz `crc32` (daemon imports `_crc32` from libz; classes-M-Audio_FireWire_Device_Helper.md:100). Confirm by targeted decomp of the category selector (**A4-H9 or new sub-item**).
    - `embeddedCRC` = `header[0x3c]` (log:162).
    - On mismatch: "Corrupt: CRC checksum failed. expected 0x%lX actual 0x%lX" (log:146-148).
6. Build NSCalendarDate from `header[0x08..0x17]` via `CalendarDateFromFirmwareDate(h[8], h[0xc], h[0x10], h[0x14])`. Cache in `self->[0x10]` + retain.
7. Sanity: `[date yearOfCommonEra] >= 2003` (`0x7d3`) else "Corrupt: Year %d" (log:136-140).
8. All pass → `self->[0x6a] = 1`.

### `CalendarDateFromFirmwareDate` stack trick

`CalendarDateFromFirmwareDate` (log:180-276) builds a C-string on the stack by writing the four input ulongs **plus literal space bytes `0x20`** between them:

- `local_9c = param_1` (year raw)
- `uStack_98 = CONCAT31((int3)param_2, 0x20)` → single byte `0x20` then 3 bytes of `param_2` (month)
- `uStack_94 = (param_2 >> 24)` (last byte of month)
- `uStack_93 = 0x20` (space)
- `uStack_92 = param_3` (day)
- `uStack_8e = 0x20` (space)
- `uStack_8d = param_4` (packed H:M:S)

Then feeds via `+[NSString stringWithCString:]` to `+[NSCalendarDate dateWithString:calendarFormat:@"%Y %m %d %H %M %S"]`. **unverified in exact byte-level** — the `param_4` field is passed as a single `ulong` but the format consumes H/M/S three fields, so `param_4` is almost certainly 3 ASCII tokens packed into a 4- to 8-byte stack slot. Needs a decode against a real `fw1814.bcd` header to confirm. The year check at ≥ 2003 (`0x7d3`) implicitly tells us year is parsed as a decimal integer (so `h[0x08]` is probably ASCII digits like `"2014"`, not a raw year value) — otherwise 0x7d3 wouldn't be the correct cutoff. **Hypothesis confirmed-ish:** the date fields are ASCII strings, not binary integers.

## Finding 4 — `CanonicalGUIDForDevice` reads `"GUID"` CFProperty, formats `%llX`

`CanonicalGUIDForDevice` (log:280-301): Ghidra lost the args in the i386 cdecl frame but the body is `IORegistryEntryCreateCFProperty(...)` → `unsignedLongLongValue` → `+[NSString stringWithFormat:@"%llX", v]`. The missing argument is almost certainly `@"GUID"` as the key (cross-ref `-deviceMatched:` log:142 which hands this function the first ulong from `MAFirmwareInfo` — **wait, that's wrong**, re-reading: `uVar6 = CanonicalGUIDForDevice(uVar12);` where `uVar12 = *(ulong *)(iVar5 + 0x20)`. So `CanonicalGUIDForDevice` takes `fwInfoBlob[0x20]` as its parameter. But its body uses `IORegistryEntryCreateCFProperty` with no obvious tie to the parameter. **This is suspicious** — likely a Ghidra i386-cdecl miss that hid the registry-entry handle. **Hypothesis — the parameter is the IOKit service handle (an `io_service_t` = `mach_port_t` = `uint`), and the function reads the device's `"GUID"` property.** The `fwInfoBlob[0x20]` passed from `deviceMatched:` is **not** the GUID itself but a reference the helper somehow uses — needs re-decomp with explicit arg recovery.

**Follow-up** (deferred): re-run with Ghidra's cdecl override to recover full `CanonicalGUIDForDevice(io_service_t, CFStringRef)` signature; also decompile `+[MAIOKitService deviceWithService:]` which is what wraps the raw `io_service_t`.

## Clone implications

- **File-selection logic is trivial to clone.** For FW1814-only shipping target, the whole `+firmwareFilesAtPath:forDevice:` collapses to one line: `return @[fwBundle/Firmware/fw1814.bcd]`. We don't need the generic multi-product dispatch; the reference has 8 products, we have 1.
- **BCD validation is a hard requirement.** We can't skip CRC check — the kext's `MAFirmwareUpgradeImage` setProperty writes bytes blindly over FireWire; a corrupt file bricks the device. Clone's fwhelper must replicate:
    - 96-byte header length check
    - `0x60 + header[0x38] + header[0x58]` total-size check
    - zlib `crc32` over `[header[0x30] .. header[0x30]+header[0x38])`, compared to `header[0x3c]`
    - date year ≥ 2003 sanity
- **Memory-map is overkill for one small file** — `+dataWithContentsOfFile:` is fine for the clone. The reference uses `+dataWithContentsOfMappedFile:` because it was designed to scan 8 product subdirs with multiple candidate `.bcd`s each; we only have one file.
- **Header is 96 bytes exactly, payload begins at `0x60`.** Confirms the "upload image" NSData blob passed to `MAFirmwareUpgradeImage` is the **full file** (header + payload + optional tail), not just the payload — the kext trusts the helper's validation and uploads the entire blob. **A1-Q18 will reveal whether the kext itself re-validates or just forwards bytes.**

## Massive A5 payoff — status update

**A5 is now ≈60% decoded.** The BCD header layout above *is* the deliverable A5 was going to produce from scratch. What remains for A5-firmware proper:

1. Byte-level content of header offsets `[0x00..0x07]`, `[0x18..0x2f]`, `[0x40..0x57]`, `[0x5c..0x5f]` — ground-truth hex dump of `fw1814.bcd` + hypothesis matching (product tag? version? pad bytes?).
2. Payload structure at `[0x60..]` — this is the actual flashable image; its subsection map lives inside the **kext** (A1-Q18), not inside anything the helper touches.
3. Appendix at `[0x60 + header[0x38]..]` of length `header[0x58]` — purpose unknown. Signature? Decryption table? Secondary CRC? **unverified**, A5 to resolve.
4. Confirm date fields are ASCII (hypothesis #4 above).

Suggested revised A5 scope: **a one-page hex-walk of `fw1814.bcd` byte-by-byte**, labeling each known field against this H4 table and attempting ASCII/numeric interpretation of the unknown ranges. Should be a 1-hour task once we open the file in `xxd`.

## Cross-references

- **A1-Q18** (new, raised by H2): `com_m_audio_FWMetaNub::setProperty:forKey:` dispatch for `MAFirmwareHalt`/`MAFirmwareStart`/`MAFirmwareUpgradeImage`. **Links to H4** — whatever the kext does with the uploaded NSData determines whether the helper's CRC is the only line of defense.
- **A1-Q19** (new, raised by H2): `MAFirmwareInfo` property blob layout. **Confirms H4** — `fwInfoBlob[0x20]`/`[0x24]`/`[0x28]`/`[0x2c]` were already read by `-deviceMatched:` to feed `CalendarDateFromFirmwareDate`, so the device's self-reported running firmware date is also 4 ulongs in the same format as the file header's `[0x08..0x17]`. That's a strong structural signal — kext builds `MAFirmwareInfo` by reading the running firmware's header out of the device.
- **A3-M4** / **A3-M8** — unrelated (MIDI wire protocol).
- **Build archaeology** — `/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/` is the reference source tree root (SVN snapshot). The `Source/DeviceHelper/` subdir is this binary's source. Useful for citations where Ghidra line numbers don't quite land on a human-sensible statement.

## Status

Done 2026-10-09. All claims cited to `a4-h4-firmware-selection.log` + cross-ref to H2. Follow-ups: (a) confirm product-name aliases for FW410 and FWSolo against FFADO/snd-bebob (Part B); (b) decompile `NSData CRC32WithRange:` category selector to confirm libz-crc32 hypothesis; (c) re-decomp `CanonicalGUIDForDevice` with cdecl override. None block H5. A5-firmware now a **1-hour task** rather than a from-scratch decode.
