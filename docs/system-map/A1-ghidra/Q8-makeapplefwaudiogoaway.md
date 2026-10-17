# Q8 — `com_m_audio_MakeAppleFWAudioGoAway::probe` — probe-steal + plist scoping

> **Correction (2026-10-15, live, R40):** with the clone loaded on the 10.13.6 build PC the FW1814 shows **no `IOFireWireAVCUnit` / sub-unit and no `AppleFWAudio`** in the registry (the device has a bare `IOFireWireUnit` plus our `FWMetaNub` -> `FW1814Device`), so the clone's go-away personalities (`FW1814AppleGoAway1/12`, deliberate extension) were **never exercised**; whether they are needed is unverified. Log: [`../build-pc-runs/2026-10-15-M-A-M-FW-install.md`](../build-pc-runs/2026-10-15-M-A-M-FW-install.md). (Backlog #15 is answered in part: with our kext, `AppleFWAudio` does not claim the FW1814.)


**Status:** done.

**Scope:** `__ZN34com_m_audio_MakeAppleFWAudioGoAway5probeEP9IOServicePi` @ 0x129cc (docs/system-map/A1-ghidra/otool-exports/mafw.asm:21460). Info.plist personalities `FWSoloAppleGoAway1` and `FWSoloAppleGoAway12` in `_extracted/kext/M-AudioFireWireBeBoB.kext/Contents/Info.plist`.

## Answer in one line

**`MakeAppleFWAudioGoAway` is a zero-functionality service whose only purpose is to score higher than `AppleFWAudio` on `IOFireWireAVCSubUnit` providers whose `FireWire Product Name` matches the personality's `FireWire Product Name` ("FW Solo") — returning score `0x7FFFFF` on a match, 0 otherwise.** In the shipped 2014 plist, this is **only applied to the FW Solo** — the FW1814 personalities do not include a GoAway probe-stealer. The question's premise ("why only FWSolo?") is answered by the plist: it's a plist choice, not a code gate.

## `probe(IOService *provider, int *score)` — full decode

```c
IOService *MakeAppleFWAudioGoAway::probe(IOService *prov, int *score) {
    OSObject *provName = prov->copyProperty("FireWire Product Name");  // via vtable[0x2b8]
    *score = 0;
    if (!provName) return NULL;
    // Fetch the OSString's isEqualTo method from its vtable at +0x40.
    bool (*isEqualTo)(OSObject*, OSObject*) = provName->vtable[0x40];
    OSObject *myName = this->copyProperty("FireWire Product Name");
    if (!isEqualTo(provName, myName)) return NULL;
    *score = 0x7FFFFF;      // 8,388,607 — higher than AppleFWAudio's default
    return this;
}
```

Three steps: (1) copy provider's product name; (2) copy our own personality's `FireWire Product Name` property (set by the plist to `"FW Solo"`); (3) if they string-match, return a very high score. The service itself overrides nothing else — `start()` would just publish an empty IOService, taking the slot away from AppleFWAudio.

## Plist shape (two personalities)

```xml
<key>FWSoloAppleGoAway1</key>
<dict>
    <key>IOClass</key>              <string>com_m_audio_MakeAppleFWAudioGoAway</string>
    <key>IOProviderClass</key>      <string>IOFireWireAVCSubUnit</string>
    <key>SubUnit_Type</key>         <integer>1</integer>     <!-- Audio -->
    <key>Vendor_ID</key>            <integer>3436</integer>  <!-- 0x000D6C M-Audio -->
    <key>FireWire Product Name</key><string>FW Solo</string>
    <key>FireWire Vendor Name</key> <string>M-Audio</string>
</dict>

<key>FWSoloAppleGoAway12</key>  <!-- same thing, SubUnit_Type = 12 (Music) -->
```

The plist pre-filters down to M-Audio AVC subunits of types 1 (Audio) and 12 (Music). The `probe()` then re-filters by exact product-name equality — a belt-and-braces check so this driver never accidentally probe-steals a different M-Audio product that reuses the same vendor ID + subunit type.

**No equivalent personality for the FW1814 exists in the shipped plist.** That means on 10.7-era systems where this kext was authored, AppleFWAudio was expected not to attach to the FW1814's AVC subunits — presumably because the FW1814 publishes additional config-ROM entries that AppleFWAudio rejects, or because `com_m_audio_FW1814Nub` out-scores it at the unit level before AppleFWAudio ever reaches the sub-unit layer.

## Why score `0x7FFFFF`

IOKit matching score is a signed 32-bit int. AppleFWAudio's default is in the 10,000–100,000 range. `0x7FFFFF = 8,388,607` guarantees preemption on any system where both drivers see the same provider, while leaving headroom for even-higher-priority third-party overrides. Classic BeBoB-era "I win" number.

## Clone implications

- The clone's current plist (`openaudio-fw1814/kext/OpenAudioFW1814-Info.plist`) **has a `com_openaudio_MakeAppleFWAudioGoAway` personality for FW1814** (per CLAUDE.md). That's a deliberate departure from the reference — the clone covers the FW1814 case the 2014 driver left to IOKit matching luck, which is prudent on 10.13.6 where AppleFWAudio's matching rules may have changed.
- The clone's probe should mirror the 3-step logic above: copy-property, string-compare, score `0x7FFFFF`. **Not** returning `0x7FFFFF` unconditionally — if the FireWire Product Name doesn't match, return 0 and `NULL` so the probe-steal is strictly scoped.

## Delta summary

| Aspect | Reference | Clone | Impact |
|---|---|---|---|
| Probe-steal class | `com_m_audio_MakeAppleFWAudioGoAway` | `com_openaudio_MakeAppleFWAudioGoAway` | Required (parity) |
| Probe-steal plist scope | FW Solo only (SubUnit 1 + 12) | FW1814 (CLAUDE.md says so) | **Clone extends — deliberate** |
| Match on `FireWire Product Name` string equality | yes | required | Prevents false probe-steal |
| Score constant `0x7FFFFF` | yes | required | Needed to out-rank AppleFWAudio |
| Any lifecycle methods beyond probe | **none** (no start/stop decode in body) | should mirror | Simplicity is the point |

## Follow-up queue impact

- **Q8a (new):** confirm whether the clone's probe currently returns `0x7FFFFF` unconditionally or only on name-match. If unconditional, add the name-match gate to avoid stealing non-FW1814 M-Audio subunits.
- Follow-up: understand why the 2014 driver omitted an FW1814 GoAway personality. Hypothesis: in 10.7-era `com_m_audio_FW1814Nub` match at IOFireWireUnit level made the sub-unit probe-steal unnecessary. Worth validating against the 10.13 behavior where our clone *does* need it.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- `0x7FFFFF = 8,388,607`: direct immediate at 0x12a33 (`movl $0x7fffff, (%rbx)`).
- Property key string `"FireWire Product Name"`: literal pool references at 0x129e0 and 0x12a0b.
- `OSString::isEqualTo` through vtable slot `+0x40`: standard libkern OSMetaClassBase layout — verified by call at 0x12a21 going through `%r12` loaded from provider's vtable.
- Plist personalities `FWSoloAppleGoAway1` / `FWSoloAppleGoAway12`: `_extracted/kext/M-AudioFireWireBeBoB.kext/Contents/Info.plist`.
