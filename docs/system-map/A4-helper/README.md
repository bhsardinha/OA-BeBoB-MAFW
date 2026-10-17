# A4 — Reference firmware helper static analysis

**Targets:**

- `_extracted/helper/M-Audio FireWire Device Helper` — Mach-O **i386** executable (the LaunchDaemon payload).
- `_extracted/helper/FirmwareStatusWindow.app/Contents/MacOS/FirmwareStatusWindow` — Mach-O **i386** executable (user-facing progress UI, `LSUIElement=1`, Xcode 4.6.3 / SDK 10.7).
- `_extracted/helper/com.m-audio.firewire.helper.plist` — launchd job (`RunAtLoad`, `argv = {"...FireWire Device Helper", "start"}`).
- `_extracted/helper/Firmware/FW 1814/fw1814.bcd` — payload consumed by the helper (full byte-level decode belongs to A5).

**Bundle meta:**

- Helper plist points at `/Library/StartupItems/M-Audio FireWire Audio Helper/M-Audio FireWire Device Helper` — SVN r56476:56479. StartupItem path, not `LaunchDaemons/`, which is itself a lifecycle fact worth citing (StartupItems were deprecated by 10.4 but still honored on 10.7–10.13).
- Status app `CFBundleIdentifier` = `com.m-audio.firewire.app.firmwarestatus`, version 1.2.2, 2014 inMusic copyright.
- Both binaries are **i386-only**. 10.13.6 is the last macOS that runs 32-bit userland processes — the helper runs on the shipping target, but a 10.14+ port of this component is a hard rewrite. Record as a C1-lifecycle constraint.

## Status / next step

**Scaffolded 2026-10-06.** Recon pass pending. Resume anchor: run `ExportClasses.java` / `ExportSymbols.java` / `DumpStrings.java` against both i386 binaries, then start the H-queue below with H1.

**Decode toolchain for i386:** Ghidra x86 32-bit decompiler is the primary; `otool -tv -arch i386` is the fallback (same role `otool` played for A3 M9). Note Objective-C i386 runtime uses the fragile ivar layout — class ivar offsets are baked into the binary, so `__objc_classlist` + `__objc_methname` give the full class map directly.

## Q-queue (seed)

In priority order. Each `H*` lands as `H<n>-<topic>.md` with pseudocode + cross-reference + clone implication.

| # | Topic | File | Rationale |
|---|---|---|---|
| H1 | `main()` + argv dispatch — `start` subcommand plus any `stop` / `upload` / `status` siblings. Which subcommand launchd calls, which the UI app invokes, which the uninstaller invokes. | `H1-entry-dispatch.md` | **done** — argv is `start`/`stop` only; both are DO-lifecycle verbs on `MAFireWireCommunicator`; no firmware subcommand. H1a/H1b small follow-ups queued. |
| H2 | Bootloader-mode detection — IOKit `IOServiceAddMatchingNotification` on Model_ID `0x10070`? Polling? AV/C probe? Which notification fires the upload path. | `H2-bootloader-detection.md` | **done** — nub-class matching on `com_m_audio_FWMetaNub` (not Model_ID); re-match drives the three-step `MAFirmwareHalt` → `MAFirmwareUpgradeImage` → `MAFirmwareStart` CFProperty writes. Resolves A1-Q7 #1. |
| H3 | Firmware upload wire protocol — FCP write-block vs AV/C vendor-dependent vs raw CSR async write; destination register; chunk size; verify/CRC; retry. | `H3-upload-wire-protocol.md` | **superseded by H2** — the FireWire wire protocol lives in the kext under `setProperty:@"MAFirmwareUpgradeImage"`, not the helper. Reopened as **A1-Q18** (kext side). |
| H4 | Firmware file selection — product-name → `.bcd` mapping (the `Firmware/<Product>/` directory layout). Where the match happens relative to H2. | `H4-firmware-selection.md` | **done** — strip-`" Bootloader"` + two aliases; FW1814 → `Firmware/FW 1814/fw1814.bcd`. Also decoded the full 96-byte BCD header (massive A5 payoff). |
| H5 | FirmwareStatusWindow IPC — Mach port name, `CFMessagePort`, distributed notifications, or spawn-with-stdin? Which process owns the NSApplication runloop. | `H5-status-ui-ipc.md` | **done 2026-10-09** — NSConnection Distributed Objects; UI is both DO server and transient client via reflection; 3 remote methods (`showWindowWithDeviceIcon:name:GUID:`, `terminate`, `setProgressIndicatorCurrent:maximum:`); argv verbs `launch`/`quit`/`progress`. H5a/b/c follow-ups queued. |
| H6 | Error handling + retry — upload failure path, device-disappeared mid-upload, verify mismatch, user prompt vs silent retry. | `H6-error-retry.md` | **done 2026-10-09** — no retry on upload itself (fail → log + `quitView`); mid-upload disconnect = silent abandon, next bus-reset restarts; only retry in binary is `unloadTheDriver`'s 3× `IOCatalogueTerminate` on `0xDC004002`. Resolves H5a (quitView = `sudo -u ... quit`). H6a/b/c follow-ups queued. |
| H7 | Launchd lifecycle — `start` daemon lifetime (resident vs exit-after-upload), SIGTERM handling, KeepAlive behavior absent from plist, interaction with `RunAtLoad`. | `H7-launchd-lifecycle.md` | **done 2026-10-09** — resident NSConnection DO server; argv `start` cosmetic on boot (init auto-starts loader); runloop never exits; no signal handler; no KeepAlive. Clone fwhelper must be resident, not one-shot (invalidates M2 "exit after upload"). |
| H8 | Interaction with the runtime kext — does the helper watch for the `0x10071` runtime Model_ID appearance too, or only `0x10070`? Any post-upload bus reset / rediscover kick? | `H8-runtime-handoff.md` | **done 2026-10-09** — matches by class name (`com_m_audio_FWMetaNub`), implicitly covers both IDs; 8 per-product `driverInstance*` watchers as the runtime-handoff signal; no explicit bus kick; sleep/wake/login all re-walk the ioreg. Resolves A1-Q7 fully. |
| H9 | `.bcd` parsing in the helper — section walker, header fields the helper reads (version / product-id / length / checksum). Full byte-level layout is **A5**; this entry captures only *what the helper itself reads* from the file. | `H9-bcd-reader.md` | **done 2026-10-09** — helper reads only header offsets `0x08..0x17` (date), `0x30/0x38` (CRC range), `0x3c` (expected CRC32). Payload beyond `0x60` is A5. Sentinel paths finally resolved: `/Library/Preferences/com.m-audio.firewire.firmware.autoupdate.{not,force}`. |
| H10 | Multi-product scope — single binary handles all 8 M-Audio FireWire products. How product-specific code paths branch (table-driven? per-product subclass? per-product subcommand?). Is FW1814 special-cased anywhere? | `H10-multi-product-dispatch.md` | **done 2026-10-09** — table-driven: 8-entry class table (FW1814/FW410/FWAudiophile/FWLightbridge/FWNRV10/FWOzonic/FWProjectMix/FWSolo). Zero FW1814-specific branches. Clone drops 7/8. |

## Cross-references into prior parts

- **A1-Q7** (`A1-ghidra/Q7-fwmetanub-lifecycle.md`) — the kext side of the bootloader↔runtime transition. H2 + H8 are the userland complement; both must agree before C1 lifecycle can be synthesized.
- **A1-Q11** (`A1-ghidra/Q11-userclient-selectors.md`) — if the helper opens a user-client on the bootloader-mode kext (as opposed to writing FireWire CSR directly), the selectors live here.
- **A2-P2** / **A3-M3** notification patterns — H5's status-UI IPC may reuse the same CoreFoundation `CFMachPort` + runloop-source idiom; cite the shared pattern rather than re-decoding it.

## How to resume

1. Pick the next unblocked `H*` from the table.
2. Import both i386 binaries into Ghidra project `helper` under `ghidra-project/`. (Keep them in one project — the status app may share Obj-C classes with the daemon.)
3. Run `ExportClasses.java` / `ExportSymbols.java` / `DumpStrings.java` once; log to `ghidra-exports/`.
4. Add scoped decompile script under `scripts/`, run headless, log to `ghidra-exports/decomp/a4-<name>.log`.
5. Write `H<n>-<topic>.md` with pseudocode + prior-part cross-reference + clone implication.
6. **Cite every claim** (log file:line + function/offset, FFADO/snd-bebob file:line, or dated capture). Mark hypotheses with `**hypothesis**` / `**unverified**` and name the check that would confirm them. See `memory/feedback_no_unmarked_guesses.md`.

## Known binary quirks

- **i386-only** — use Ghidra's x86 32-bit decompiler; fall back to `otool -tv -arch i386` for spot checks.
- **Objective-C i386 fragile ivar layout** — class instance-variable offsets are literal in the binary (unlike arm64 non-fragile). This simplifies struct recovery but means any Apple-side class resize would have been an ABI break; not a concern for a 2014 binary on 10.13.
- **StartupItem path** in the plist, not `/Library/LaunchDaemons/` — the installer drops both a StartupItem bundle (for pre-10.4 compat / Apple-ism) and a launchd plist. Record where the clone's own helper is installed (we use `/Library/LaunchDaemons/` directly).
- **SVN rev `r56476:56479`** in the plist comment — the whole 2014 release is one SVN snapshot; expect code sharing across helper / panel / prefpane / midi plugin, so symbols seen in A2/A3 may reappear here under the same names.
- **`LSUIElement=1`** on the status app — no Dock icon, no menu bar; it is a background-only Cocoa app driven by the daemon.

## Status

A4 scaffolded 2026-10-06. Recon pass done 2026-10-06. H1 + H1a done 2026-10-09. H2 done 2026-10-09 (resolved A1-Q7 #1; raised A1-Q18 + A1-Q19). H3 **closed-by-H2** (scope moved to A1-Q18). **H4 done 2026-10-09** (massive A5 payoff — 96-byte BCD header now mapped). **H5 done 2026-10-09** (NSConnection DO; 3 remote methods; argv `launch`/`quit`/`progress`). **H6 done 2026-10-09** (no retry on upload; mid-upload disconnect = silent; one 3× retry loop on `IOCatalogueTerminate`; resolved H5a). **H7 done 2026-10-09** (resident NSConnection DO server; no SIGTERM handler; no KeepAlive; clone fwhelper must be resident — invalidates M2 "exit after upload"). **H8 done 2026-10-09** (class-name matching covers both bootloader + runtime; 8 per-product driverInstance watchers; sleep/wake/login re-walk the ioreg; resolves A1-Q7 fully). **H9 done 2026-10-09** (helper reads only date + CRC-range + expected-CRC32; payload is kext-opaque; resolved sentinel paths). **H10 done 2026-10-09** (table-driven 8-class dispatch; zero FW1814 special-casing; clone drops 7/8). **A4 H-queue COMPLETE.**

**A4 sub-follow-ups resolved 2026-10-09:** H1b (`_kMAFireWireServerName` = `"MAudioFireWireHelper"`), H5b (UI `_kServerName` = `"MAudioFireWireFirmwareStatus"`), H5c (icon-ext ladder = `png`/`tif`/`tiff`/`jpg`), H6b (`0xDC004002` = `kOSMetaClassHasInstances`, **not** `kIOFireWireBusReset` — the retry waits for OSObject teardown, not FireWire), H6c (4 corruption reason strings: `Shouldbeatleast…`, `Embeddedlengthis…`, `CRCchecksumfailed…`, `Year…`), H8a (8 per-product classes: FW410, FWAudiophile, FW1814, FWSolo, FWOzonic, FWNRV10, FWProjectMix, FWLightbridge — the earlier `FW1200`/`FWMobilePre`/`FW1 ProFire` guess was wrong), H8b (helper opens one UC per nub purely for `IOConnectSetNotificationPort`; **no** `IOConnectCallMethod` anywhere; confirms the "CFProperty-only control plane" model). H9a (ivar `+0x10` on `MAFireWireFirmwareFile` = cached `NSDate` from `CalendarDateFromFirmwareDate(header+0x08..0x17)`, written once by `validateFirmwareUpdate:` after CRC passes). H9b (`CRC32WithRange:` = 5-line libz `crc32(0,NULL,0)` seed + `crc32(seed, bytes+off, len)`; standard IEEE polynomial). H10a (**closed-by-H8a** — class names verified in-binary; FFADO cross-check deferred to B1). Only remaining A4 follow-ups: H10b (A1 cross-check — belongs in A1 Q-queue). **A4 fully closed.** Next: A5-firmware (payload decode, ~60% done inline via H4).

**Daemon class landscape** (from `ghidra-exports/classes-M-Audio_FireWire_Device_Helper.md` + function listing):
- `MAFireWireCommunicator` — DO server singleton; `init` / `start` / `stop` / `getServer:` / `runServer:` (methods inferred from strings + `_main` call-sites — bodies pending H1a).
- `MAFireWireLoader` — top-level device-match controller: `deviceMatched:`, `driverInstanceMatched:`, `driverInstanceTerminated:`, `updateViewForDevice:`, `systemWillSleep` / `systemWokeUp`, `userDidLogIn:...` / `LogOut:`, `unloadTheDriver` / `unloadDriverOnce`. **H2 target.**
- `MAFireWireDeviceInstance` — per-device state; `configureDCLLatency`, `defaultsForUser:`, `commitSettings:`, `loadPrefsForUser:`, `receivedPropertyChangeMessage:`.
- `MAFireWireFirmwareFile` — `.bcd` parser; `+firmwareFilesAtPath:forDevice:`, `-validateFirmwareUpdate:`, `-embeddedCRC`, `-actualCRC`, `+automaticUpdatesInhibited`, `+setInhibitAutomaticUpdates:`, `+forceNextAutomaticUpdate`. **H9 target; informs A5.**
- `MAIOKitRegistryNode` — IOKit traversal wrapper (`entry`, likely iterator helpers).
- `WithCurrentEffectiveUserAndGroup` — C++ RAII guard that drops euid/egid temporarily (ctor/dtor + overload at `0x51a4..0x51e6`).
- Linked imports confirm: `_crc32` (libz; A5 CRC path — body dumped under H9b), `_popen`/`_system` (used by `-updateViewForDevice:`/`-quitView` to shell out `sudo -u <user> launch/quit …` — see H5), `IOCatalogueTerminate` (kext cache invalidation after upload — see H6), `IOServiceOpen` (**confirmed via H8b** — opened in `+[MAFireWireDeviceInstance deviceInstanceForService:]` and `-[MAIOKitService openConnection]` purely to install async-notification ports; **no** `IOConnectCallMethod` is imported — the UC is a one-way notification sink), `IORegistryEntrySetCFProperty` / `SetCFProperties` (confirmed in H2 as the 3-step `MAFirmwareHalt`→`MAFirmwareUpgradeImage`→`MAFirmwareStart` upload channel — the wire protocol itself lives in the kext under A1-Q18, not the helper).

**Status-UI class landscape** (from `ghidra-exports/classes-FirmwareStatusWindow.md`): to be summarized in H5.
