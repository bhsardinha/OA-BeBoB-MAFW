# Q-INFO — `OpenAudioFW1814.kext` Info.plist IOKit personalities

> **Correction (2026-10-15, live, R40):** the clone's plist (personalities 1 and 2) matched and ran on 10.13.6 (`IOFireWireDevice` -> `FWMetaNub` -> `FW1814Device`); personality 3 (go-away) was not exercised because no AV/C sub-unit is published (see Q8 correction). `FWMetaNub` matched both the bootloader device (`FireWire Product Name = "FW 1814 Bootloader"`, resolved by the alias slot of `kDeviceSpecs`) and the runtime device. Log: [`../build-pc-runs/2026-10-15-M-A-M-FW-install.md`](../build-pc-runs/2026-10-15-M-A-M-FW-install.md).


**Status:** done 2026-10-13 — corrects BUILD-MANUAL §2.A.

**Scope:** final matching dictionary for the clone kext. Mirrors reference `_extracted/kext/M-AudioFireWireBeBoB.kext/Contents/Info.plist` (CFBundleShortVersionString `1.11.3`), scoped to FW1814-only + extended with the FW1814 go-away personality A1-Q8 flagged as missing in the reference.

## Correction vs BUILD-MANUAL §2.A

BUILD-MANUAL said: *"`IOProviderClass = IOFireWireUnit`; two personalities: `Model_ID = 0x10070` (bootloader) + `Model_ID = 0x10071` (runtime); `IOProbeScore` = reference's score +1".*

**All three statements are wrong.** The reference plist actually uses:

1. **`IOProviderClass = IOFireWireDevice`** on `FWMetaNub` — one layer above `IOFireWireUnit`. Confirmed: `_extracted/kext/.../Info.plist` key `FWMetaNub` → `IOProviderClass` = `IOFireWireDevice`. Q7's internal note that the C++ class *inherits* from `IOFireWireUnit` is a class-inheritance observation, not a matching-dictionary observation; the two are independent in IOKit.
2. **No `Model_ID` filter at the plist level** on `FWMetaNub`. The reference's bootloader-vs-runtime discrimination (Model_ID `0x10070` vs `0x10071`) happens inside the C++ `probe()` / `start()`, which read `Vendor_ID` + `Model_ID` off the `IOFireWireDevice` provider's property dict. Only one plist entry needed. (The reference plist has an explicit `Model_ID = 65665` entry — but that's for `com_m_audio_FWNRV10Device`, a sibling product; `FWMetaNub` itself has no `Model_ID` match.)
3. **No `IOProbeScore` key in the reference plist** anywhere. All scoring is runtime-only via the C++ `probe()` returning `*score = 0x7FFFFF` on positive matches (Q8). The plist leaves `IOProbeScore` unset → IOKit uses default `0` and relies on the `probe()` override.

**Correction (2026-10-13):** `BUILD-MANUAL.md` §2.A should be updated. Pending edit to BUILD-MANUAL.

## Final clone plist shape

Three personalities needed for FW1814-only coverage:

### Personality 1 — `FWMetaNub` (polymorphic nub matching any M-Audio BeBoB device)

```xml
<key>FWMetaNub</key>
<dict>
    <key>CFBundleIdentifier</key>  <string>com.openaudio.OpenAudioFW1814</string>
    <key>IOClass</key>             <string>com_openaudio_FWMetaNub</string>
    <key>IOProviderClass</key>     <string>IOFireWireDevice</string>
</dict>
```

**No `Vendor_ID` / `Model_ID` filter.** The reference's actual probe discipline, from direct disassembly of `com_m_audio_FWMetaNub::probe` @ `docs/system-map/A1-ghidra/otool-exports/mafw.asm:34489` (`0x1ce62`):

1. `getProperty("FireWire Vendor Name")` on provider (`*0x2b8(%rax)` = IORegistryEntry::getProperty) → vendor-name OSString at `%r15` (`0x1ce80`).
2. Build two OSString literals: `"M-Audio"` (`0x1ce93`) and `"M-AUDIO"` (`0x1cea2`).
3. Call `*0x40(%rax)` on vendor-name OSString with each literal — this is `OSObject::isEqualTo` (`0x1ced1`, `0x1ceea`). If **neither** matches → score=0, return `NULL` (`0x1cecc` → `0x1cf47`).
4. On match: `getProperty("FireWire Product Name")` → OSString → `getCStringNoCopy` (`0x1cefe`+`0x1cf0a`).
5. Call `com_m_audio_FWMetaNub::GetActualProductName(const char*)` (`0x1cf16`) — walks static `kDeviceSpecs` table at `0x1ce06`, 8 entries × 0x18 bytes, 2 string-slots per entry (primary + alias), returns the primary name on `strcmp` hit on either slot, else `NULL`.
6. If `GetActualProductName` returned non-NULL: `IOLog("M-FW: M-Audio FireWire device found")`, `*score = 0x7FFFFF` (`0x1cf37`), return self as nub (`0x1cf51`).

**Probe does NOT read `Model_ID`.** The Model_ID property copy at `0x1d02d` is inside `FWMetaNub::start` (which also copies `FireWire Product Name`, `FireWire Vendor Name`, `Vendor_ID`, `GUID` via `CopyPropertyFrom` at `0x1cff4..0x1d04c`) — just populating self's property dict so downstream consumers (per-product device classes) can see them. No conditional branch on Model_ID in that sequence.

**Bootloader-vs-runtime discrimination is not a probe-level concern.** It happens later via the FWMetaNub state flags `[+0xf0]` (bootloader-active) and `[+0xf3]` (firmware-registered). Confirmed read sites: `FirmwareStart @ 0x1beb4` reads `[+0xf0]` (`0x1bec9`), branches to firmware-upload path if set, else to runtime-register path; sets `[+0xf3] = 1` after runtime registration (`0x1bf7a`).

**Scope limit for clone:** we can leave FWMetaNub polymorphic (same as reference) because Personality 2 narrows by `FireWire Product Name = "FW 1814"`. If stricter scoping is desired, override `probe()` to additionally reject any product name whose `GetActualProductName` lookup doesn't resolve to `"FW 1814"`. The 8-entry `kDeviceSpecs` table in the reference lists all BeBoB products; our clone's equivalent carries only the FW1814 entry so `GetActualProductName` returns NULL for every other BeBoB product, score=0, no match — a cleaner mechanism than a numeric Model_ID whitelist and preserves the exact reference control flow.

### Personality 2 — `com_openaudio_FW1814Device` (per-product match on FWMetaNub provider)

```xml
<key>com_openaudio_FW1814Device</key>
<dict>
    <key>CFBundleIdentifier</key>       <string>com.openaudio.OpenAudioFW1814</string>
    <key>IOClass</key>                  <string>com_openaudio_FW1814Device</string>
    <key>IOProviderClass</key>          <string>com_openaudio_FWMetaNub</string>
    <key>IOUserClientClass</key>        <string>com_openaudio_FW1814UserClient</string>
    <key>FireWire Product Name</key>    <string>FW 1814</string>
</dict>
```

Matches on `FireWire Product Name = "FW 1814"` — this property is published onto FWMetaNub by `FWMetaNub::start`'s `CopyPropertyFrom(provider, "FireWire Product Name")` call at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:34614` (`0x1d004`). Where the provider (`IOFireWireDevice`) originally gets it from (config ROM VendorText descriptor vs. IOFireWireFamily-synthesized string) is **unverified — needs IOFireWireFamily source read or ioreg trace on 192.168.100.17**; the FWMetaNub-level property is what the per-product personality matches against, which is the only fact needed for the plist. Mirrors reference's `com_m_audio_FW1814Device` entry byte-for-byte with only the bundle-id / class-prefix renamed. **No `Model_ID` filter at this plist layer** either — IOKit's `matchPropertyTable` on `FireWire Product Name` sufficies (reasoning, not disassembled: this is the plist-matching rule in IOKit, same mechanism the reference relies on).

`IOUserClientClass` points at the vendor user-client that Panel opens via `IOServiceOpen(..., /*type=*/0)` — exposes 8 selectors 500–507 (Q11) + the `clientMemoryForType(1)` shared meter ring (Q-UC-RING).

### Personality 3 — `FW1814AppleGoAway1` + `FW1814AppleGoAway12` (probe-steal AppleFWAudio)

```xml
<key>FW1814AppleGoAway1</key>
<dict>
    <key>CFBundleIdentifier</key>       <string>com.openaudio.OpenAudioFW1814</string>
    <key>IOClass</key>                  <string>com_openaudio_MakeAppleFWAudioGoAway</string>
    <key>IOProviderClass</key>          <string>IOFireWireAVCSubUnit</string>
    <key>SubUnit_Type</key>             <integer>1</integer>   <!-- Audio -->
    <key>Vendor_ID</key>                <integer>3436</integer><!-- 0x000D6C M-Audio -->
    <key>FireWire Product Name</key>    <string>FW 1814</string>
    <key>FireWire Vendor Name</key>     <string>M-Audio</string>
</dict>

<key>FW1814AppleGoAway12</key>
<!-- identical except SubUnit_Type=12 (Music) -->
```

**Deliberate deviation from reference** per Q8 clone-implications §: reference ships this scoping only for `FW Solo`; the clone extends it to `FW 1814` to pre-empt `AppleFWAudio` from attaching to the FW1814's AVC Audio (type 1) and Music (type 12) sub-units on 10.13 where matching behavior differs from 10.7-era. `probe()` returns `0x7FFFFF` on a match (Q8), 0 otherwise.

## Personalities omitted vs reference

The reference 1.11.3 plist ships **9 personalities**: `FWMetaNub` + 7 per-product device classes (`FW1814`, `FW410`, `FWAudiophile`, `FWLightbridge`, `FWNRV10`, `FWOzonic`, `FWProjectMix`, `FWSolo`) + 2 go-away entries. The clone ships **4 personalities** total — one per-product match for `FW1814` only, with no sibling-product support. Dropping those is a scope choice tracked in `feedback_clone_scope_full_parity_modern_ui.md`: clone scope is FW1814-only, not the whole BeBoB family.

## OSBundleLibraries

Mirror the reference's `OSBundleLibraries_x86_64` block (10.7-era KPI version strings are forward-compatible on 10.13):

```xml
<key>OSBundleLibraries_x86_64</key>
<dict>
    <key>com.apple.iokit.IOAudioFamily</key>      <string>1.1</string>
    <key>com.apple.iokit.IOFireWireAVC</key>      <string>1.0</string>
    <key>com.apple.iokit.IOFireWireFamily</key>   <string>1.0.0</string>
    <key>com.apple.kpi.iokit</key>                <string>8.0</string>
    <key>com.apple.kpi.libkern</key>              <string>8.0</string>
    <key>com.apple.kpi.mach</key>                 <string>8.0</string>
</dict>
```

Drop the reference's non-x86_64 `OSBundleLibraries` block — 10.13 is x86_64-only.

## Bundle-level keys

| Key | Value | Source |
|---|---|---|
| `CFBundleIdentifier` | `com.openaudio.OpenAudioFW1814` | BUILD-MANUAL §1 "Package IDs" table |
| `CFBundleExecutable` | `OpenAudioFW1814` | mirrors reference |
| `CFBundlePackageType` | `KEXT` | required for IOKit matching |
| `CFBundleSignature` | `DDRV` | mirrors reference, cosmetic |
| `CFBundleShortVersionString` | `0.1.0` | fresh for the clone |
| `CFBundleVersion` | `0.1.0` | same |
| `OSBundleRequired` | **omit** (user-request-only load) | 10.13 safety per `feedback_stage_kext_before_autoload.md` — autoload-at-boot is disabled until M-A passes |

## Verification steps (before M-A `kextload`)

1. `plutil -lint kext/Info.plist` — syntax clean.
2. `kextlibs -xml -all-symbols kext/.../OpenAudioFW1814` — all KPI symbols resolve to the 4 declared libraries.
3. `kextutil -tn /tmp/OpenAudioFW1814.kext` — matching dictionary parses + loader validates; no load.
4. On runtime load: `ioreg -c com_openaudio_FWMetaNub -w0` shows exactly 1 instance per attached FW1814; `ioreg -c com_openaudio_FW1814Device -w0` shows 1 instance per matched runtime (Model_ID `0x10071`) nub.
5. `ioreg -w0 | grep -i AppleFWAudio` shows AppleFWAudio **not** bound under any FW1814 AVC sub-unit.

## Provenance

- Reference plist: `_extracted/kext/M-AudioFireWireBeBoB.kext/Contents/Info.plist` (read via `plutil -convert xml1 -o -` on 2026-10-13).
- Per-product `FWMetaNub` provider pattern: reference's `com_m_audio_FW1814Device` entry (`IOProviderClass = com_m_audio_FWMetaNub` + `FireWire Product Name = FW 1814`).
- `FWMetaNub` matches on `IOFireWireDevice`, not `IOFireWireUnit`: reference plist key `FWMetaNub` / `IOProviderClass` — authoritative.
- No `Model_ID` plist filter on `FWMetaNub`: reference plist has no `Model_ID` key on the `FWMetaNub` entry; only `FWNRV10Device` carries a plist-level `Model_ID = 65665`.
- No `IOProbeScore` plist key anywhere in reference: `plutil -convert xml1 -o - _extracted/kext/.../Info.plist | grep -c IOProbeScore` → `0` (run 2026-10-13).
- FWMetaNub probe matches on Vendor **Name** string, not numeric Vendor_ID: direct read of `__ZN21com_m_audio_FWMetaNub5probeEP9IOServicePi` at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:34489` (`0x1ce62`); `getProperty("FireWire Vendor Name")` + `OSString::isEqualTo` against literals `"M-Audio"` / `"M-AUDIO"`.
- Product-name lookup: `__ZN21com_m_audio_FWMetaNub20GetActualProductNameEPKc` at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:34457` (`0x1ce06`); 8-entry static table `kDeviceSpecs`, 0x18-byte stride, 2 string-slots per entry (`strcmp` on primary + alias).
- Model_ID is NOT a probe-level filter: no `Model_ID` reference in the `probe()` function body at `0x1ce62..0x1cf66`; sole Model_ID property-copy occurrence inside FWMetaNub is at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:34623` (`0x1d02d`), inside `FWMetaNub::start`, part of a bulk `CopyPropertyFrom` sequence (Product Name, Vendor Name, Vendor_ID, Model_ID, GUID), unconditional.
- Bootloader-vs-runtime discrimination via `[+0xf0]` / `[+0xf3]` state flags: direct read of `__ZN21com_m_audio_FWMetaNub13FirmwareStartEv` at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:33401` (`0x1beb4`); branches on `[+0xf0]` and `[+0xf3]` with IOLog strings `"M-FW: Bootloader active: starting firmware"` and `"M-FW: Firmware active: registering metanub"`. The write sites that set these flags from Model_ID are **unverified — needs forward search for `0xf0` / `0xf3` field writes in FWMetaNub methods** (follow-up: Q7 cross-check).
- MakeAppleFWAudioGoAway probe + score logic: `Q8-makeapplefwaudiogoaway.md` with Ghidra offset `0x129cc` + score immediate `0x7FFFFF` at `0x12a33`.
- Vendor_ID `3436 = 0x000D6C`: reference plist `FWSoloAppleGoAway*` entries.
- Clone scope limit (FW1814-only): memory `feedback_clone_scope_full_parity_modern_ui.md` + `D-decision.md` §3.
