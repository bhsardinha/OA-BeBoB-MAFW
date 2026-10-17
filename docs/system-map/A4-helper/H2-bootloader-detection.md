# H2 — Bootloader detection + firmware-update dispatch

> **Correction (2026-10-15, scope cut + live, R40/R42):** the clone's helper implements only the "no update needed" branch (`MAFirmwareStart` on every match; reads `MAFirmwareInfo` for logging). `MAFirmwareHalt`, `MAFirmwareUpgradeImage`, the date comparison, the one-shot/inhibit files and the status-UI hand-off are **not built** (user decision 2026-10-15: the driver never writes the device's flash; BUILD-PLAN tasks H.2 and H.5 cancelled). Live: the bootloader window after power-on or `Halt` is ~3 s and the device boots by itself, so the reference's "Halt -> Upgrade -> Start" had to act within milliseconds of the nub notification; whether the bootloader's auto-boot timer stops once a download starts is **unverified**. Log: [`../build-pc-runs/2026-10-15-M-A-M-FW-install.md`](../build-pc-runs/2026-10-15-M-A-M-FW-install.md).


> **Update (2026-10-14, P3/R37):** the open items A1-Q18 (`setProperty` dispatch for `MAFirmwareHalt/Start/UpgradeImage`) and A1-Q19 (the `MAFirmwareInfo` blob) are closed by `A1-ghidra/Q-P3-kext-firmware-path.md`: the blob is the first 80 bytes of the device info register (FFADO `info_register_t`); `[0xc]` is the **bootloader version** and `[0x20..0x2f]` the software date/time.
> **Correction (2026-10-14) — static audit** (full evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). 8 product classes now read from `strings` (no longer unverified). `MAFirmwareInfo` is an **80-byte** blob read from `0xFFFF:C8020000` by `FWMetaNub::FirmwareCacheInfoRegisters @0x1cc9e`; `info[0xc] != 0` ⇔ bootloader active. Status UI command is `sudo -u %@ "%@" launch "%@" "%@" "%llX" &` with a matching `… quit &`; `launch` is the **UI binary's own subcommand**, not `launch(1)`.


**Target:** `_extracted/helper/M-Audio FireWire Device Helper` (i386).

**Decomp log:** `ghidra-exports/decomp/a4-h2-bootloader-detection.log` — specifically `-[MAFireWireLoader init]` (lines 49-88), `-[MAFireWireLoader deviceMatched:]` (lines 92-213), `-[MAFireWireLoader driverInstanceMatched:]` (217-238), `-[MAFireWireLoader driverInstanceTerminated:]` (243-256), `-[MAFireWireLoader updateViewForDevice:]` (294-320). H1a (`MAFireWireCommunicator init/start/stop`) also captured in the same log (1-45).

## Finding 1 — The helper watches **two** IOKit namespaces in parallel

`-[MAFireWireLoader init]` (log:49-88) registers:

1. **Nub-class matching notification** on `com_m_audio_FWMetaNub` → `-[deviceMatched:]`
   - Calls `IOServiceMatching("com_m_audio_FWMetaNub")` (log:65) then `[MAServiceNotification observeServiceMatched:target:sel:...]` with selector `deviceMatched:` (log:69-70).
   - **This is the firmware-update trigger path.** It fires for both bootloader-mode (`Model_ID 0x10070`) *and* runtime-mode (`0x10071`) devices because the FWMetaNub matches at the config-ROM level regardless of Model_ID (per `A1-ghidra/Q7-fwmetanub-lifecycle.md` — the nub catches every M-Audio device and C++-discriminates inside).
2. **Per-product device-class matching + termination notifications** → `-[driverInstanceMatched:]` / `-[driverInstanceTerminated:]`
   - Loop at log:71-83: `iVar4 = -0x20; do { ... iVar4 += 4; } while (iVar4 != 0)` → 8 iterations, reading 8 class-name pointers from the dataseg immediately before `__ZL15sSystemSleeping_b`.
   - **8 class names = the 8 M-Audio FireWire products** (`FW1814`, `FW410`, `FWSolo`, `FWAudiophile`, `FWLightbridge`, `FWNRV10`, `FWOzonic`, `FWProjectMix` — cross-ref `A1-ghidra/Q13-engine-family.md` + plist personalities). **unverified** — the exact string identities of these 8 pointers need a dataseg dump; the shape (8 × 4-byte pointer stride ending at `sSystemSleeping`) is read-off-decomp certain.
   - Each class gets both a `-ServiceMatched:` observer (driverInstanceMatched:) and a `-ServiceTerminated:` observer (driverInstanceTerminated:) — tracks whether each product's **runtime** device class attached a driver.
3. **Sleep + login observers** (log:84-86) — `[MAPowerNotification observeSleepNotifications:self]` and `[MAConsoleUser startObservingLogins:self]`. Feeds H7 (launchd lifecycle) and `sSystemSleeping` gating in deviceMatched.

The two namespaces are deliberately layered: the nub-level watcher is the firmware-update brain; the per-class watcher is just bookkeeping for `MAFireWireDeviceInstance` objects (added to an NSMutableArray at `self+4`, log:231-234).

## Finding 2 — Firmware update is a **CFProperty write**, not a FireWire wire protocol

`-[MAFireWireLoader deviceMatched:]` (log:92-213) is the firmware-update entry point. Decoded dispatch:

```
deviceMatched:service
    if (sSystemSleeping) return
    if (![MAConsoleUser isUserLoggedIn]) return

    dev = [MAIOKitService deviceWithService:service]
    vendorName    = [dev propertyForKey:@"FireWireVendorName"]
    productName   = [dev propertyForKey:@"FireWireProductName"]
    actualProduct = [dev propertyForKey:@"MAActualProductName"]
    fwInfoBlob    = [dev propertyForKey:@"MAFirmwareInfo"]     // NSData, kext-published
    if (fwInfoBlob == nil):
        NSLog("Firmware info not available from device")
        NSLog("Device properties: %@", [dev properties])
        return

    bytes = [fwInfoBlob bytes]
    deviceDate = CalendarDateFromFirmwareDate(bytes+0x20, bytes+0x24, bytes+0x28, bytes+0x2c)
    guid       = CanonicalGUIDForDevice(bytes+0x20)            // first ulong doubles as GUID input
    fwPath = [[[NSBundle mainBundle] resourcePath]
              stringByAppendingPathComponent:@"Firmware"]
    files = [MAFireWireFirmwareFile firmwareFilesAtPath:fwPath forDevice:dev]
    fwFile = files[0]                                           // nil-safe via count check
    if (![fwFile validateFirmwareUpdate:nil]):
        NSLog("Corrupt firmware file, not updating")
    else:
        if ([fwFile date] != deviceDate):
            needsUpdate = true

    oneShot = [fm fileExistsAtPath:_kMAOneShotUpdatePath]
    inhibit = [fm fileExistsAtPath:_kMAInhibitUpdatesFilePath]
    if (!(oneShot || needsUpdate)) :
        NSLog("Starting %@", productName)
        [dev setProperty:@"YES" forKey:@"MAFirmwareStart"]
        return
    if (inhibit):
        NSLog("Firmware update inhibited")
        [dev setProperty:@"YES" forKey:@"MAFirmwareStart"]
        return

    if (bytes[0xc] != 0):                                      // "ready for upload" state
        NSLog("Starting firmware update")
        [fm removeFileAtPath:_kMAOneShotUpdatePath handler:nil]
        [self updateViewForDevice:dev]                          // spawn status UI
        ok = [dev setProperty:[fwFile data] forKey:@"MAFirmwareUpgradeImage"]
        if (!ok): NSLog("Firmware update attempt failed")
        else:     NSLog("Starting the firmware after successful update"),
                  [dev setProperty:@"YES" forKey:@"MAFirmwareStart"]
        [self quitView]
    else:                                                       // bytes[0xc] == 0 → firmware running
        NSLog("Firmware is running, shutting down the firmware")
        [dev setProperty:@"YES" forKey:@"MAFirmwareHalt"]       // kick device into bootloader mode
```

Three CFProperty **write** keys on `com_m_audio_FWMetaNub` (or the per-product subclass) make the whole protocol:

| Key | Payload | Semantic | Kext side (A1 todo) |
|---|---|---|---|
| `MAFirmwareHalt` | NSString `@"YES"` | Transition running firmware → bootloader mode. Triggers a re-enumeration under Model_ID `0x10070`. | **New A1 Q-item** — decompile `FWMetaNub::setProperty:forKey:` dispatch for this key. |
| `MAFirmwareUpgradeImage` | NSData — the full `.bcd` file bytes | Upload the firmware. **Return value** gates success (log:190). Likely a long-blocking synchronous call. **unverified** — kext may dispatch async with a notification; verify by reading A1 setProperty impl. | **New A1 Q-item** — this is the actual FireWire wire-protocol decode (H3 was mis-scoped as "helper-side"). |
| `MAFirmwareStart` | NSString `@"YES"` | Boot the newly-uploaded firmware. Device re-enumerates under `0x10071`. | **New A1 Q-item.** |

And one **read** key (log:126):

| Key | Payload | Semantic |
|---|---|---|
| `MAFirmwareInfo` | NSData (binary blob, ≥ 0x30 bytes) | Running firmware's self-report. Byte-level layout read by helper: `[0xc]` state flag (0 = running, non-0 = ready for upload); `[0x20..0x2f]` 4 ulongs feeding `CalendarDateFromFirmwareDate(d,m,y,extra)` and `CanonicalGUIDForDevice(firstULong)`. Full layout **unverified** — the helper only touches these offsets; the kext builds the blob (A1 side). |

Plus a handful of read-only identity keys: `FireWireVendorName`, `FireWireProductName`, `MAActualProductName`, `MAIconName`, `GUID`.

### Two-step bootloader kick, confirmed

The reference's upgrade dance is a **two-pass loop driven by IOKit re-matching**:

1. First `deviceMatched:` on the runtime device → `fwInfoBlob[0xc] == 0` ("firmware is running") → write `MAFirmwareHalt`.
2. Kext kicks device into bootloader mode → device re-enumerates with Model_ID `0x10070` → **FWMetaNub matches again** → second `deviceMatched:` → `fwInfoBlob[0xc] != 0` → write `MAFirmwareUpgradeImage` + `MAFirmwareStart`.
3. Kext boots new firmware → device re-enumerates with `0x10071` → third `deviceMatched:` → file-dates match → "Starting <product>" + `MAFirmwareStart`. End.

This resolves `A1-ghidra/Q7-fwmetanub-lifecycle.md` **open question #1** (bootloader↔runtime transition): it is **not** an in-kext C++ state machine — it's driven externally by IOKit re-match, with the kext doing the physical mode-switch under three well-named CFProperty writes.

## Finding 3 — The status UI is spawned via `system("sudo -u <user> launch <path> <args>")`

`-[MAFireWireLoader updateViewForDevice:]` (log:294-320): builds a shell command via `+[NSString stringWithFormat:]` with format `sudo -u %s launch %s %s %s %llX` (icon name, product name, GUID), then `system(cmd)`. The daemon is root, so `sudo -u <consoleUser>` is a **privilege drop** so the UI runs under the logged-in user's session (and gets a Dock-less NSApplication because of `LSUIElement=1` in the status app's `Info.plist`). The `launch` tool is `/usr/bin/launch` (deprecated but present on 10.7–10.13).

**Clone implication (feeds H5):** on 10.13.6, use `launchctl bsexec` + `sudo -u` or (simpler) `NSTask`+`setLaunchPath:` with the status-UI binary spawned under the console user's uid resolved via `SCDynamicStoreCopyConsoleUser`. The shell-out-to-`system()` form is sloppy and shell-injectable if the icon/product strings ever contain shell metacharacters — avoid for the clone.

## Finding 4 — `MAFireWireCommunicator init` self-starts (H1a tail)

`-[MAFireWireCommunicator init]` (log:1-14): `[super init]` then **immediately** `[self start]`. So `[[MAFireWireCommunicator alloc] init]` is enough to spin up the MAFireWireLoader — `[server start]` in `_main` (H1) is a no-op after the DO server registered itself with its own `init`. The explicit `[server start]` call is defensive (idempotent per the `_loader == nil` check in `-start`).

**-[MAFireWireCommunicator start]** (log:18-32): if `self->loader == nil`, alloc+init a `MAFireWireLoader`. One loader per Communicator; always lazy.

**-[MAFireWireCommunicator stop]** (log:36-45): release loader, nil ivar. The whole deviceMatched + sleep + login observer set tears down.

Closes H1a.

## New A1 Q-queue items raised by this decode

Added to `A1-ghidra/README.md` on next edit:

1. **A1-Q18 (new)** — Decompile `com_m_audio_FWMetaNub::setProperty:forKey:` + `setProperties:` dispatch for the three keys `MAFirmwareHalt`, `MAFirmwareStart`, `MAFirmwareUpgradeImage`. This is the real H3 — the FireWire-wire firmware upload lives in the kext under setProperty.
2. **A1-Q19 (new)** — Decompile `com_m_audio_FWMetaNub` (or FW1814-specific) building of the `MAFirmwareInfo` property blob. Document the full byte layout at `[0..0x30+]`, not just the offsets the helper happens to touch (`[0xc]`, `[0x20..0x2f]`).

## Clone implications

- **The clone's kext must publish `MAFirmwareInfo` and respond to `MAFirmwareHalt`/`MAFirmwareStart`/`MAFirmwareUpgradeImage` setProperty writes** if we keep the reference's upgrade protocol. Alternative: our existing M2 fwhelper uses a direct IOUserClient `.open()` + custom selector — that works too but **doesn't match the reference's IOKit-matching-driven re-enumeration loop**. Decide in Part D.
- **Nub-level matching confirmed as the right paradigm** — our clone currently matches at `IOFireWireUnit` with explicit Model_ID. That misses the key insight: the reference nub matches at `IOFireWireDevice` with no plist Model_ID constraint, catches both `0x10070` and `0x10071` under one class, and lets a single IOKit re-match drive the entire upgrade state machine. **Likely root cause of our TX-silence symptom** (nub at wrong level → device state inaccessible to the kext at critical moments). Add this as a C1-lifecycle finding when C1 is written.
- **CFProperty-driven upgrade protocol** is remarkably clean: the helper is pure IOKit CF-API; no FireWire userspace lib. Our clone's fwhelper can be rewritten the same way once the kext exposes the three property keys.

## Status

Done 2026-10-09. All claims cited to `a4-h2-bootloader-detection.log` lines above or `strings-M-Audio_FireWire_Device_Helper.txt`. Open hypotheses marked `**unverified**` with the follow-up that would confirm them. H3 is now **superseded in scope** — the firmware wire-protocol decode belongs in A1-Q18 (kext side), not here; H3's new scope is "helper → kext property-write surface" which is already covered by this file. **H3 can be closed-by-H2.**
