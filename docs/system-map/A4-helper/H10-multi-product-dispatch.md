# H10 — Multi-product dispatch

**Correction (2026-10-09):** H10's §2 claim that "the shipping DMG contains only `FW 1814/fw1814.bcd`" is wrong. Directory listing of `_extracted/helper/Firmware/` shows **all 8 product subdirectories ship**, each with its own `.bcd`:

```
FW 1814/   FW 410/   FW Audiophile/   FW Solo/
NRV-10/    Ozonic/   ProFire Lightbridge/   ProjectMix/
```

Also the two "hypothesis — needs verification" product names (`FWLightbridge` = ProFire Lightbridge, `FWNRV10` = NRV-10) are **confirmed** by matching directory names.

The clone-side conclusion (drop 7/8 branches, FW1814-only) is **unchanged**.

**Status:** done 2026-10-09.
**Target:** `M-Audio FireWire Device Helper` i386 daemon.
**Log:** `ghidra-exports/decomp/a4-h8-runtime-handoff.log` (shares with H8).

## Headline

The daemon handles 8 M-Audio FireWire products through **one catch-all nub class match + a table-driven per-product driver-class watch loop**. There is **no FW1814-specific code path** anywhere in the daemon — FW1814 is one row in a loop of eight identical registrations. Our clone can drop 7 of the 8 branches and reduce the whole loop to a direct single registration.

## The 8-entry class table

`-[MAFireWireLoader init]` @ `0x2c3d` runs this loop (H8 log:23-35):

```c
int i = -0x20;
do {
    io_name_t cls = classNameTable[i / 4];      // ptr table at &sSystemSleeping - 0x20
    io_service_t m = IOServiceMatching(cls);
    [MAServiceNotification observeServiceMatched:[m autorelease] target:self sel:@selector(driverInstanceMatched:)];
    [MAServiceNotification observeServiceTerminated:[m autorelease] target:self sel:@selector(driverInstanceTerminated:)];
    i += 4;
} while (i != 0);
```

Eight entries (0x20 / 4 = 8). Extracted verbatim via `strings -arch i386` grep (`com_m_audio` prefix, filtered `*Device` suffix):

| # | Class name | Product (M-Audio marketing name) |
|---|---|---|
| 1 | `com_m_audio_FW1814Device` | FireWire 1814 |
| 2 | `com_m_audio_FW410Device` | FireWire 410 |
| 3 | `com_m_audio_FWAudiophileDevice` | FireWire Audiophile |
| 4 | `com_m_audio_FWLightbridgeDevice` | ProFire Lightbridge (**hypothesis** — M-Audio ProFire 2626/Lightbridge; verify against FFADO device table) |
| 5 | `com_m_audio_FWNRV10Device` | NRV10 (**hypothesis** — M-Audio NRV10 interface/mixer; verify) |
| 6 | `com_m_audio_FWOzonicDevice` | Ozonic |
| 7 | `com_m_audio_FWProjectMixDevice` | ProjectMix I/O |
| 8 | `com_m_audio_FWSoloDevice` | FireWire Solo |

(The ninth `com_m_audio_` class in the strings output, `com_m_audio_FWMetaNub`, is the catch-all nub — a separate first match before the loop, not an entry in the table.)

**Observation:** 8 ≠ the "7 FireWire products" sometimes cited in M-Audio marketing. The reference binary shipped with support for 8 named device classes. **Note:** some of these may be aliases or marketing-renamed products; `FWLightbridgeDevice` and `FWNRV10Device` are the two names least common in M-Audio documentation and most likely to be hypothesis-worthy.

## Where does product selection *actually* happen?

Three places, in order:

### 1. Nub appearance (any product) — `deviceMatched:`

The catch-all nub (`com_m_audio_FWMetaNub`) is published by every M-Audio FireWire kext *regardless of product*. `deviceMatched:` reads the device's product name through standard FireWire properties (`FireWireProductName`, `MAActualProductName`) — see H4 log and H6 log:30-33.

**No branch on product name within `deviceMatched:`.** The function treats every product identically — it only reads `MAFirmwareInfo`, picks a `.bcd` file (via H4's `firmwareFilesAtPath:forDevice:` which does the product-name → `Firmware/<Product>/` mapping), and sends the three CFProperty writes. Product-specific logic lives *inside* `firmwareFilesAtPath:forDevice:` (H4) and *inside the kext* (A1).

### 2. Firmware file selection — `+firmwareFilesAtPath:forDevice:`

Already decoded by H4. Reads `MAActualProductName`, strips `" Bootloader"` suffix, applies two aliases (`FireWire 410` + `MobilePre` → canonical), and looks up `Firmware/<name>/<name>.bcd`. This is a **string-driven map** with no explicit per-product branch in the helper's code — the filesystem layout under `Firmware/` is the dispatch table.

For the shipping M-Audio DMG, the `Firmware/` directory contains **only** `FW 1814/fw1814.bcd` (confirmed by `_extracted/helper/Firmware/` listing). So even though the daemon supports 8 product classes at the IOKit-match level, it only ships one firmware file. The other 7 products presumably had firmware shipped in earlier releases but were dropped by the 2014 build.

**Hypothesis — needs verification:** the `Firmware/` directory at install time may have included more products that were pruned for the FW 1814-only final release. Or (more likely) the installer places all firmware files at build time but this particular DMG snapshot is the FW1814-only variant. Not worth decoding further — scope is reference parity, not reconstructing M-Audio's release history.

### 3. Audio driver attach — `driverInstanceMatched:`

Fires once per matched class in the table. The handler (H6 log:173-195) is **product-agnostic**: it instantiates `MAFireWireDeviceInstance` for the matched service and adds to a flat `NSMutableArray`. The per-product class name serves only as the IOKit matching filter — the handler code doesn't look at which class fired. Compare with the kext side, where each product's `*Device` subclass carries its own ivars and overrides (per A1 class hierarchy).

So from the helper's perspective, "which product is this" is a *runtime* question answered by reading the `MAFireWireDeviceInstance`'s properties (product name, GUID, model id), not a *compile-time* dispatch.

## Is FW1814 special-cased anywhere?

**No.** Grepping `strings-M-Audio_FireWire_Device_Helper.txt` for `FW1814` or `1814` returns no code-path strings — only the class name `com_m_audio_FW1814Device` (which is a table entry, not an if-branch target). The daemon is uniformly table-driven. **This is a clean architecture decision — reproduce it, don't invent per-product branches in the clone.**

Verified by: grep over function listing + strings file + decomp logs for the H1–H9 queue — zero per-product branches found.

## Cross-references

- **H2** (`H2-bootloader-detection.md`) — catch-all nub match; H10 confirms the per-product watch loop is additive, not a replacement.
- **H4** (`H4-firmware-selection.md`) — product-name → `.bcd` mapping (filesystem-driven).
- **H8** (`H8-runtime-handoff.md`) — the init loop decode; H10 enumerates its 8 entries.
- **A1-ghidra** — the kext side has 8 corresponding `*Device` subclasses + the shared `com_m_audio_FWMetaNub` catch-all. H10 confirms the helper mirrors the kext's own class hierarchy exactly.
- **Project memory `project_dice_conflict.md`** — Avid DICE conflict is unrelated; DICE devices don't publish `com_m_audio_FWMetaNub` and don't match any of these 8 classes.

## Clone implications (for Part D)

1. **Our clone ships one product class.** Drop 7 of 8 table entries, drop the loop entirely, register the single FW1814 driver class directly. Code reduction is cosmetic — the reference-equivalent loop with one entry works identically.
2. **Keep the catch-all nub pattern** (one class name watched by `-deviceMatched:` + one or more product classes watched by `-driverInstanceMatched:`). Even for a single product, the two-level matching mirrors the reference and keeps the lifecycle state machine identical.
3. **Firmware-file lookup can be hardcoded.** We don't need the H4 alias table or the directory walk — one path, `Firmware/FW 1814/fw1814.bcd`, embedded as a const. Mark this as a *simplification* comment in the fwhelper source.
4. **No per-product config files.** The reference's `.bcd` layout and sentinel paths are already uniform across products; our clone inherits that cleanliness for free.

## Follow-ups queued

- **H10a** — closed-by-H8a. The 8 class names were hex-dumped directly from the helper binary (`__DATA.__const` `0x97f4..0x9814`), so `FWLightbridge` and `FWNRV10` are now **verified-in-binary** rather than needing an FFADO cross-check. A B1-side verification against `libffado/src/bebob/maudio/` would still be useful to confirm the class-name → user-visible product-name mapping is consistent across sources; **deferred to B1** (no local FFADO checkout as of 2026-10-09).
- **H10b** — enumerate the per-product classes the kext publishes (A1 cross-check). Likely a 1:1 match with H10's table; if the kext has 9 and the helper watches 8, that gap is interesting. 10 min — fits as an A1 Q-item rather than an A4 follow-up.

## Status closes A4

**With H10 done, the A4-helper H-queue is complete.** All ten items (H1–H10) have primary decode notes; H1a/b, H5a/b/c, H6a/b/c, H7a/b/c, H8a/b, H9a/b, H10a/b are *small* follow-ups and are not blocking. The daemon's full behavior is documented end-to-end.

**Next part of the system map:** A5-firmware (`.bcd` payload decode; ~60% already done inline during H4). After that: A6-prefpane. Then Part B cross-reference. Then Part C synthesis. Then Part D build decision.
