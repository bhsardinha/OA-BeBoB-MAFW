# H9 — `.bcd` reader (helper side)

**Correction (2026-10-09):** byte-level BCD-header claims below are **superseded by `A5-firmware.md`**. Two errors:
1. The 16 bytes at `h[8..0x17]` are a **single ASCII date string**, not 4 binary u64s / 4 u32s. The decompiler's reads of `*(ulong*)(iVar1 + 8/0xc/0x10/0x14)` saw 4 word-aligned reads of an ASCII buffer. `CalendarDateFromFirmwareDate` parses ASCII.
2. "`h[0x50]` is unknown" is wrong — `h[0x50]` is the **appendix start offset** (verified `0xE4B0C` in `fw1814.bcd`). `h[0x5c]` is the appendix CRC32 (verified ✓ via zlib). The reference helper doesn't check appendix CRC; it does check payload CRC via `h[0x3c]`.

Behavioral conclusions below (helper reads only date + CRC range + expected CRC32; payload is opaque to the helper; sentinel paths) are **unchanged**.

**Status:** done 2026-10-09.
**Target:** `M-Audio FireWire Device Helper` i386 daemon.
**Log:** `ghidra-exports/decomp/a4-h9-bcd-reader.log` + (prior) `a4-h4-firmware-selection.log`.

## Headline

H4 already decoded the 96-byte BCD header. H9's remaining job is **what the helper itself reads from the file** (as opposed to the full byte-level layout, which is A5 scope). The answer: **just the header date, CRC range, and embedded CRC32 — nothing beyond byte offset `0x60`.** The payload is mmap'd and handed to the kext as an opaque `NSData` via `setProperty:@"MAFirmwareUpgradeImage"`. The helper neither parses nor touches it. All byte-level payload decode is A5.

Also fully resolves the inhibit/force sentinel paths and the `MAFireWireFirmwareFile` ivar layout.

## `MAFireWireFirmwareFile` object layout

From `+firmwareWithPath:` (log:1-23) + `-dealloc` (log:28-44):

| Offset | Type | Set by | Role |
|---|---|---|---|
| `+0x00` | isa / NSObject fields | super | - |
| `+0x04` | `NSData *` | `[NSData dataWithContentsOfMappedFile:path]` | mmap of the whole `.bcd` file |
| `+0x08` | `NSString *` | `[path lastPathComponent]` | just the filename, e.g. `fw1814.bcd` |
| `+0x0c` | `NSString *` | retained `path` | full absolute path |
| `+0x10` | `id` | nil at init; released in dealloc | **unknown ivar** — probably a cached parse result (**hypothesis**: an NSDate from `-date`, or a per-section index table). Nothing else assigns to it in the methods decoded; must be populated by `+firmwareFilesAtPath:forDevice:` or by `-validateFirmwareUpdate:`. Verify by grepping ivar writes — queued as H9a. |
| `+0x14` | `const uint8_t *` | `[data bytes]` | raw pointer to the mmap'd header |

Everything else accessed by header offsets through `+0x14`.

## Header fields the helper reads

Only these fields are touched by any method on `MAFireWireFirmwareFile`:

| Offset | Type | Reader | Purpose |
|---|---|---|---|
| `h[0x08..0x17]` | 4 × `uint64_t` | `-date` → `CalendarDateFromFirmwareDate(h[8], h[0xc], h[0x10], h[0x14])` | packed firmware date → NSDate; compared to `MAFirmwareInfo.date` in `deviceMatched:` to decide if update is needed |
| `h[0x30]` | `uint32_t` | `-actualCRC` | CRC32 range *start offset* |
| `h[0x38]` | `uint32_t` | `-actualCRC` | CRC32 range *length* |
| `h[0x3c]` | `uint32_t` | `-embeddedCRC` | expected CRC32 value |

The whole validation reduces to:

```c
- (BOOL)validateFirmwareUpdate:(id)delegate {
    // (details in H4 log — exact byte checks)
    uint32_t expected = *(uint32_t *)(h + 0x3c);
    uint32_t actual   = [data CRC32WithRange:NSMakeRange(h[0x30], h[0x38])];
    if (expected != actual) {
        [self reportCorruptFirmwareToDelegate:delegate
                                      format:@"CRC mismatch: expected %x, got %x",
                                             expected, actual];
        return NO;
    }
    // ... + plus H4's additional checks on offsets 0x58, 0x60 ...
}
```

`[NSData CRC32WithRange:]` is a category on NSData backed by `libz` `_crc32` (confirmed by H6 class listing + import of `_crc32` from `libz.1.dylib`).

## `-reportCorruptFirmwareToDelegate:format:` (log:107-125)

```c
- (void)reportCorruptFirmwareToDelegate:(id)delegate format:(NSString *)fmt, ... {
    NSString *msg = [[NSString alloc] initWithFormat:fmt arguments:va_list];
    if (delegate == nil) delegate = self;
    [delegate firmwareValidationFailed:msg];
    [msg release];
}
```

- Variadic. `initWithFormat:arguments:` consumes the stack-passed `va_list`.
- Delegate defaults to `self` — meaning `firmwareValidationFailed:` fires on `MAFireWireFirmwareFile` itself (H6 showed that's just an `NSLog`).
- `deviceMatched:` passes `nil` as the delegate when calling `validateFirmwareUpdate:` (H2/H6), so the "corrupt firmware" string ends up in the daemon's syslog.

## Automatic-update inhibit / force sentinels (log:129-214)

**Resolves the exact filesystem paths** that H6 only knew by symbol name:

| Sentinel | Path | Checked by |
|---|---|---|
| `kMAInhibitUpdatesFilePath` | `/Library/Preferences/com.m-audio.firewire.firmware.autoupdate.not` | `deviceMatched:` log:89 (H6) — skip update if present |
| `kMAOneShotUpdatePath` | `/Library/Preferences/com.m-audio.firewire.firmware.autoupdate.force` | `deviceMatched:` log:82 (H6) — force update if present, then remove |

**`+setInhibitAutomaticUpdates:(BOOL)yn`**:
- Write an empty `NSData` to the `.not` path if `yn == YES` and file doesn't already exist.
- Remove the file if `yn == NO` and file exists.
- No-op in both already-at-target-state cases.

**`+automaticUpdatesInhibited`**: `fileExistsAtPath:` on `.not`.

**`+forceNextAutomaticUpdate`**: `fileExistsAtPath:` on `.force` (returns bool).

**`+automaticUpdatedForced`** (sic — probably typo for "automaticUpdateForced"): checks `forceNextAutomaticUpdate` and, if true, **removes the `.force` sentinel immediately**. The removal in `deviceMatched:` (H6 log:93) is redundant with this; one of them is dead code. **Hypothesis**: the removal in `+automaticUpdatedForced` is called from `userDidLogIn:` or `systemWokeUp:` as a one-shot gate check before the device-level removal. Not strictly needed to decode further — functional behavior is the same either way.

**Who writes these sentinels?** Not the helper itself (except `setInhibitAutomaticUpdates:`). The `.force` sentinel is almost certainly dropped by an external installer script or the panel's "update now" button. H6c follow-up (reason strings in `validateFirmwareUpdate:`) might catch any further writers, but hypothesis: external installer only.

## Cross-references

- **H4** (`H4-firmware-selection.md`) — primary decode of `validateFirmwareUpdate:` and the full 96-byte header map. H9 is a thin consolidation on top.
- **H6** (`H6-error-retry.md`) — the inhibit/one-shot sentinels decoded here are the ones H6 only knew by symbol.
- **A5-firmware** (not yet started) — byte offsets `0x60..` (payload) are A5 scope. H9 confirms the helper itself does **not** parse them.

## Clone implications

1. **Don't parse the `.bcd` payload in our clone fwhelper.** Match the reference: mmap, CRC-verify the header-declared range, hand the whole blob to the kext as `MAFirmwareUpgradeImage`. Payload decode (if ever needed) belongs to A5 and runs in whichever side needs it — probably neither.
2. **CRC via libz, not a custom implementation.** `_crc32` is the only algorithm used; mirror that in Go via `hash/crc32.ChecksumIEEE`.
3. **Reproduce the two sentinel paths verbatim** if we want install-script compatibility: `/Library/Preferences/com.m-audio.firewire.firmware.autoupdate.not` and `.autoupdate.force`. **Alternatively**, since our clone uses a different label (`com.openaudio.firewire.*`), we rename them. **Clone decision: rename** — these are our own private sentinels, not an interop surface.
4. **Delegate-less `firmwareValidationFailed:` → NSLog** is fine for the clone's log-only error UX (H6).

## H9a resolved — ivar `+0x10` is the cached parsed NSDate

Decode in `ghidra-exports/decomp/a4-h4-firmware-selection.log:129-134` (inside `-[MAFireWireFirmwareFile validateFirmwareUpdate:]`):

```c
iVar5 = *(int *)(param_1 + 0x14);                          // raw header pointer (ivar fHeader)
uVar2 = CalendarDateFromFirmwareDate(
            *(ulong *)(iVar5 + 0x08),                      // date subfield bytes 0x08..0x17
            *(ulong *)(iVar5 + 0x0c),
            *(ulong *)(iVar5 + 0x10),
            *(ulong *)(iVar5 + 0x14));
*(undefined4 *)(param_1 + 0x10) = uVar2;                   // store NSDate ptr at ivar+0x10
_objc_msgSend(uVar2, retain);
```

**Confirms the first H9 hypothesis.** Ivar `+0x10` is the cached `NSDate` returned by `CalendarDateFromFirmwareDate` over the 16-byte date subfield at header offset `0x08..0x17`. Populated **once** inside `-validateFirmwareUpdate:` after the CRC passes, so subsequent calls to `-[MAFireWireFirmwareFile date]` (or the `yearOfCommonEra` sanity-check two lines later) don't re-parse. Released in `-dealloc` (`a4-h9-bcd-reader.log:39`).

**Not populated by `+firmwareFilesAtPath:forDevice:`** (the class method) — files that have never been validated carry `+0x10 = nil`. The `-date` accessor at `a4-h9-bcd-reader.log:81-93` **does not read `+0x10`** — it re-parses the raw bytes each call. So `+0x10` is a validation-only cache; the public `-date` property is cache-cold. Trade-off is deliberate: valid-but-never-validated firmware files behave the same from the outside; only the inner year-of-era corruption check uses the cached form.

## H9b resolved — `CRC32WithRange:` body

Full disassembly of `-[NSData(MAudioAdditions) CRC32WithRange:]` @ `0x5270` (85 bytes):

```asm
0x5270: pushl %ebp ; movl %esp,%ebp ; pushl %esi ; subl $0x14,%esp
0x527f: movl  $0x0, 0x4(%esp)                 ; buf = NULL
0x5287: movl  $0x0, (%esp)                    ; crc = 0
0x528e: call  _crc32                          ; seed = crc32(0, NULL, 0)
0x5293: movl  %eax, %esi                      ; esi = seed
0x5295: movl  0xa134, %eax                    ; selector_ref: @selector(bytes)
0x529a: movl  %eax, 0x4(%esp)
0x529e: movl  0x8(%ebp), %eax                 ; self (NSData *)
0x52a1: movl  %eax, (%esp)
0x52a4: call  _objc_msgSend                   ; eax = [self bytes]
0x52a9: movl  0x14(%ebp), %ecx                ; ecx = range.length  (arg 3)
0x52ac: movl  %ecx, 0x8(%esp)                 ; crc32 arg2 = length
0x52b0: addl  0x10(%ebp), %eax                ; eax += range.location (arg 2)
0x52b3: movl  %eax, 0x4(%esp)                 ; crc32 arg1 = bytes + offset
0x52b7: movl  %esi, (%esp)                    ; crc32 arg0 = seed
0x52ba: call  _crc32                          ; crc32(seed, bytes+offset, length)
0x52bf: addl  $0x14, %esp ; popl %esi ; popl %ebp ; retl
```

Equivalent C:

```c
- (uint32_t)CRC32WithRange:(NSRange)r {
    uint32_t seed = crc32(0, NULL, 0);        // libz: priming value = 0
    const uint8_t *bytes = (const uint8_t *)[self bytes];
    return crc32(seed, bytes + r.location, r.length);
}
```

**Standard IEEE 802.3 CRC-32** via libz (`_crc32` is `/usr/lib/libz.*`). No BridgeCo-custom polynomial. Clone equivalent in Go: `hash/crc32.ChecksumIEEE(data[off:off+len])`. In Swift: `zlib.crc32(0, bytes, len)` or any of the standard `CRC32` libraries.

## Follow-ups queued

- **H9a** — resolved 2026-10-09 above.
- **H9b** — resolved 2026-10-09 above.
