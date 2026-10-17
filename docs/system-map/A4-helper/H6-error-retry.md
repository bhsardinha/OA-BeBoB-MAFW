# H6 — Error handling + retry

> **Correction (2026-10-14) — status-UI spawn + retry** (evidence: [PROVENANCE-RESOLUTIONS.md §R15](../PROVENANCE-RESOLUTIONS.md)). (a) The 'few hundred ms' blocking estimate is replaced by a fact: the shelled command ends with `&` (helper strings: `sudo -u %@ "%@" launch "%@" "%@" "%llX" &` and `… quit &`), so `system()` returns once the shell has backgrounded the job — it does not wait for the status UI. (b) Retry: `deviceMatched:` makes a single attempt; a failed `setProperty` logs 'Firmware update attempt failed' and nothing else (decompile a4-h2 log); a later attempt happens only if IOKit matches the nub again (re-enumeration, wake, login — H8). The H6a hypothesis (wake/login re-walk) is resolved by H8.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - 'Likely a few hundred ms, acceptable' (line 45) is an **unsourced estimate**.


**Status:** done 2026-10-09.
**Target:** `M-Audio FireWire Device Helper` i386 daemon.
**Log:** `ghidra-exports/decomp/a4-h6-error-retry.log` (DecompA4H6.java).

## Headline

The helper **has no retry on firmware upload itself.** `setProperty:@"MAFirmwareUpgradeImage"` returning 0 → one `NSLog("Firmware update attempt failed")`, status UI is dismissed, control returns, done. The only retry loop in the whole binary is `unloadTheDriver`'s `IOCatalogueTerminate` retry (3× with 2 s between tries). Device-disappeared mid-upload is silent: `driverInstanceTerminated:` just breaks retain cycles and removes the device from the loader's array; on reconnect `deviceMatched:` restarts the full three-step dance from scratch.

**Clone implication:** we inherit the same "fail → wait for next IOKit re-match" model unless we add retry ourselves. For a shipping clone this is probably fine — the dance is driven by bus-reset re-enumeration, and a mid-upload failure already triggers a bus event — but we must *document* that absence instead of assuming the helper handles it.

## Full error / lifecycle surface — one function at a time

### `-[MAFireWireLoader deviceMatched:]` @ `0x30eb`

Called by IOKit on every nub-class match (per H2). Reads device state and dispatches firmware/status actions. Error / skip branches, in order (log:23-121):

| Condition | Line | Action |
|---|---|---|
| `sSystemSleeping == true` | log:23-25 | silent `return` |
| `[MAConsoleUser isUserLoggedIn] == NO` | log:26-29 | silent `return` (status UI needs an active user session) |
| `[device propertyForKey:@"MAFirmwareInfo"] == nil` | log:35-41 | `NSLog("Firmware info not available from device")` + dump full `[device properties]` + `return` — **no retry, no deferred retry** |
| `firmwareFilesAtPath:forDevice:` returns nil or `[count] == 0` | log:58-59 (`if (iVar10 && count)` falls past the body) | *silent* — no NSLog, no update; falls through to one-shot/inhibit checks with `bVar1 = false` ⇒ update path skipped, drops to plain `MAFirmwareStart` |
| `validateFirmwareUpdate:` returns NO | log:63-65 | `NSLog("Corrupt firmware file - not updating")` — still falls through to one-shot/inhibit guards; **no retry**; eventually sends `MAFirmwareStart` to resume device |
| Firmware date equal (no update needed) | log:69-77 | `NSLog("%@ firmware is current ...")` — bail out of update, drop to inhibit/one-shot checks |
| `kMAInhibitUpdatesFilePath` exists | log:89-90, 115 | `NSLog("Firmware update inhibited")` → fall through to `MAFirmwareStart` without upload |
| `*(firmwareInfo + 0xc) == 0` (device "firmware running" flag is 0 — i.e. still in bootloader) | log:91 (`if (*(iVar5+0xc) != 0)`) | takes the `MAFirmwareHalt` branch at `LAB_0000342c` (log:111-113) → device must re-enumerate before upload |
| `setProperty:@"MAFirmwareUpgradeImage"` returns 0 | log:97-101 | `NSLog("Firmware update attempt failed")` — **no retry**; proceeds to `quitView` (dismiss status UI) and returns |
| `setProperty:@"MAFirmwareUpgradeImage"` returns non-zero | log:102-107 | `NSLog("Starting the firmware after successful update")` + `setProperty:@YES forKey:@"MAFirmwareStart"` + `quitView` |

**Observation chain:** success path = one `deviceMatched:` call that writes `MAFirmwareHalt`; device re-enumerates in bootloader; *second* `deviceMatched:` writes `MAFirmwareUpgradeImage` then `MAFirmwareStart`. Failure path at step 2 = `quitView` and done. Nothing in the helper schedules a retry; recovery relies entirely on the next bus event re-firing `deviceMatched:`.

The guards (`sSystemSleeping`, `isUserLoggedIn`) are set via `systemWillSleep` / `systemWokeUp` / `userDidLogIn:` / `userDidLogOut:` (A4 README class listing). **Hypothesis:** a wake or login re-walks the ioreg and calls `deviceMatched:` for every live nub — need to verify by decompiling those four selectors (queue as **H6a**).

### `-[MAFireWireLoader updateViewForDevice:]` @ `0x2f6a` (log:126-152)

Spawns the status UI by shelling:

```
sudo -u <currentUser> <statusAppPath> launch <iconName> <productName> <GUID-hex>
```

Note:
- Uses `system(3)` via `_system_UNIX2003`. Not fork+exec — meaning the daemon blocks on the shell for the duration of the status-app launch. Likely a few hundred ms, acceptable.
- `sudo -u <currentUser>` resolves H5 question #2 (how does a root daemon get a UI into the user's session) — the StartupItem-era answer was literally `sudo -u`; a modern port would use XPC/`launchctl bsexec`.
- `kMAStatusAppPath` is resolved by `-[MAFireWireLoader statusAppPath]` (not decoded; symbol list line in H5 class landscape).
- **No error check** — `system()`'s return is ignored. If the status UI fails to launch the daemon continues the upload anyway. Clone implication: status UI is advisory; firmware upload must not depend on it.

### `-[MAFireWireLoader quitView]` @ `0x306d` (log:259-275) — resolves H5a

Shells `sudo -u <currentUser> <statusAppPath> quit`. Same pattern as `launch`. Confirms the H5 argv dispatch (`launch` / `quit` / `progress`). The daemon calls `quitView` after both the success and the failure branch of the upload, so the status UI closes either way.

### `-[MAFireWireLoader driverInstanceMatched:]` @ `0x35d5` (log:173-195)

On the per-product subclass match: constructs a `MAFireWireDeviceInstance`, adds it to a lazily-allocated `NSMutableArray` at `ivar+0x4`. No error reporting. No retry.

### `-[MAFireWireLoader driverInstanceTerminated:]` @ `0x3664` (log:156-169)

On per-product driver termination (which includes device unplug or mid-upload bus reset): `[device breakRetainCycles]` + `[array removeObject:device]`. **Nothing is retried; no "resume upload" state.** The next bus event will fire `deviceMatched:` fresh and restart the three-step dance from the top.

### `-[MAFireWireLoader unloadTheDriver]` @ `0x2da7` (log:199-241)

The only retry loop in the binary. Calls `unloadDriverOnce` which invokes `IOCatalogueTerminate(masterPort, 1, "com.m-audio.driver.firewire")`. If it returns the special error `0xDC004002` (sign-extended `-0x23ffbffe`), spins up a 3-try retry:

```
for (tries = 3; tries > 0; tries--) {
    [NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:2.0]];
    err = [self unloadDriverOnce];
    if (err != 0xDC004002) break;
}
```

**Correction (2026-10-09):** the preceding `sub_iokit_firewire = 0x4000` hypothesis was wrong on two counts. See H6b resolution below.

**H6b resolved — `0xDC004002` = `kOSMetaClassHasInstances` (libkern, not FireWire).**

Decode (per `/Library/Developer/CommandLineTools/SDKs/MacOSX15.2.sdk/usr/include/{mach/error.h,libkern/OSReturn.h}`):

- bits 31-26 (`sys`): `0x37` → `err_system(0x37)` = `sys_libkern`, **not** `sys_iokit (0x38)`
- bits 25-14 (`sub`): `0x1` → `err_sub(1)` = `sub_libkern_metaclass`
- bits 13-0 (`code`): `0x2`

`libkern_metaclass_err(2) = kOSMetaClassHasInstances` (OSReturn.h:126). Semantic: the OSMetaClass registry still holds live instances of a class owned by the kext — `IOCatalogueTerminate` can't drop the personality while objects of its classes exist.

**What this means for the retry.** The 3× 2 s loop isn't waiting for a FireWire bus to settle — it's waiting for the kernel to tear down lingering `OSObject` instances. Typically the culprits are: open user-clients (a dangling `IOUserClient`), live `IOAudioStream`s on the engine, or an in-flight DCL program. The 6 s total ceiling is a classic "hope the audio HAL closes its userclient" wait. If a client is wedged (hung CoreAudio HAL plugin, running DAW process), the loop times out and `NSLog("failed to unload device, error: %lX")` fires.

**Clone implication.** The clone's `stop` path must also handle `kOSMetaClassHasInstances` with a short retry. More importantly: aggressively close any userclient the clone's own fwhelper opens (H8b: only one is opened, and only for `IOConnectSetNotificationPort` — releasing the `io_connect_t` before calling `IOCatalogueTerminate` is sufficient). Don't rely on the retry to paper over a leaked UC.

Only this one error gets a retry. Any other `IOCatalogueTerminate` failure → single `NSLog("failed to unload device, error %X")` and return. Called from the `stop` subcommand (per H1) and from `-applicationWillTerminate:` paths.

### `-[MAFireWireLoader unloadDriverOnce]` @ `0x2d6c` (log:245-255)

Wraps the single `IOCatalogueTerminate` call with the bundle id `com.m-audio.driver.firewire`. Note that's the *bundle id*, not a kext class name — IOCatalogueTerminate with 3rd arg = `kCFBundleIdentifier` is the "unload all matched by this bundle id" idiom. Equivalent to `kextunload -b com.m-audio.driver.firewire` in a single IOKit call.

### `-[MAFireWireFirmwareFile firmwareValidationFailed:]` @ `0x5884` (log:279-287)

```
_NSLog(param_3);   // the message is passed in by the caller
```

Trivial: wraps `NSLog` of whatever reason string the validation code passed. No UI alert, no dialog, no retry. The real caller is inside `validateFirmwareUpdate:` (seen in H4) which does the BCD CRC and header checks and calls back with specific reason strings. **No user-visible error** — just the daemon log.

## Error-code inventory (what the helper's log would show on failure)

Grepping the log + decomp:

| Message | Trigger | Terminal? |
|---|---|---|
| `Firmware info not available from device` | `MAFirmwareInfo` property missing | yes — bail, wait for next match |
| `Device properties: %@` | companion dump after the above | — |
| `Corrupt firmware file, not updating` | `validateFirmwareUpdate:` false | no — falls through to `MAFirmwareStart` |
| `Date of current firmware: %@`, `Date of firmware on device: %@. needs update` | version delta | no — proceeds to upload |
| `One-shot firmware update activated` | one-shot sentinel file present | no — forces upload path |
| `Firmware update inhibited` | inhibit sentinel file present | yes — skip to `MAFirmwareStart` |
| `Starting firmware update` | upload beginning | — |
| `Firmware update attempt failed` | `setProperty:MAFirmwareUpgradeImage` ⇒ 0 | yes — dismiss UI, no retry |
| `Starting the firmware after successful update` | upload succeeded | — |
| `Firmware is running, shutting down the firmware` | runtime → bootloader transition needed | no — writes `MAFirmwareHalt`, exits; next match handles upload |
| `Starting %@` | plain post-upload / no-update start | — |
| `Loading %@ with GUID %llX` | announce device matched | — |
| `unloading M-Audio FireWire: %X` | `unloadTheDriver` logs catalogue-terminate return | — |
| `retrying unload (%lu)` | iokit `0xDC004002` retry | — |
| `unloaded audio device at count: %lu` | success | — |
| `failed to unload device, error: %X` | giving up after retries | yes |
| `driver instance removed: %lX` | `driverInstanceTerminated:` | — |
| `driver instance created: %lX` | `driverInstanceMatched:` | — |

Reference `strings-M-Audio_FireWire_Device_Helper.txt` for byte-literal grep.

## Cross-references

- **H2** (`H2-bootloader-detection.md`) — the 3-step `MAFirmwareHalt` → `MAFirmwareUpgradeImage` → `MAFirmwareStart` protocol decoded here lives in `deviceMatched:`. H6 shows the error branches of that same function.
- **H4** (`H4-firmware-selection.md`) — `validateFirmwareUpdate:` and the firmware-file list come from H4; H6 only observes their truth values and failure branches.
- **H5** (`H5-status-ui-ipc.md`) — `updateViewForDevice:` is the "launch" shell-exec, `quitView` is the "quit" shell-exec; both invoke the status app via `sudo -u <user> <path> <verb> ...`. Together with H5's argv table this fully resolves the status-UI lifecycle.
- **A1-Q18** — remains open on the kext side: our clone must decide what to do when `setProperty:@"MAFirmwareUpgradeImage"` fails *inside the kext*. The helper's "give up and log" response is a floor, not a ceiling.
- **A1-Q7 #2** — "mid-upload disconnect" question is now answered: the helper does nothing; recovery is entirely bus-reset-driven.

## Clone implications (for Part D)

1. **Match the helper's one-shot upload semantics by default.** Don't add retry inside the fwhelper's upload path — rely on the kernel's bus-reset re-match. (Note the current clone fwhelper does **not** yet implement the "wait for user login" guard; see implication 3.)
2. **Reproduce the `unloadTheDriver` retry.** If our userland `stop` path uses `IOCatalogueTerminate`, mirror the 3-retry / 2 s delay loop for the `0xDC004002` return. Our current `stop` logic is unspecified. **Follow-up: add this to the fwhelper design doc under M2.**
3. **The "user must be logged in" guard is non-obvious** and will bite us if we skip it. On a headless login window the reference helper **does nothing** on `deviceMatched:` — the UI can't launch into no session, and `sudo -u` would fail. Our clone must either (a) replicate this guard or (b) replace the "shell out as the user" IPC with something that doesn't require an active GUI session (XPC). **Design decision owed; mark hypothesis if we pick (b) that the device will actually accept firmware uploads from the login window — the kext does the FireWire write, so this is probably fine, but needs a verification session.**
4. **Error strings are the only user signal.** The reference ships with no dialog on failure. If we want better UX, that's a clone-side addition — and it's genuinely useful, since right now the only way to notice a failed update is to tail `/var/log/system.log`.
5. **Mid-upload bus reset = full restart.** No partial-upload resume logic exists. For the clone this is fine because the BCD payload is small enough (`~200 KB` on FW1814 per H4's header read) that re-upload is cheap.

## H6c resolved — the four `validateFirmwareUpdate:` corruption reasons

Full decode in `ghidra-exports/decomp/a4-h4-firmware-selection.log:86-153` (`-[MAFireWireFirmwareFile validateFirmwareUpdate:]` @ `0x5cc8`). Four distinct `reportCorruptFirmwareToDelegate:format:` paths and one non-delegate `NSLog`:

| # | Reason format string (CFString) | Trigger | Observable? |
|---|---|---|---|
| 0 | `"No updated firmware"` (`NSLog` only — no delegate) | `fFirmwareData == nil` | daemon log only |
| 1 | `"Corrupt: Should be at least %lu bytes long, but is actually %d bytes long"` | file length < `0x60` (header too short) | `firmwareValidationFailed:` → `NSLog` |
| 2 | `"Corrupt: Embedded length is %lu bytes, but is actually %d bytes long"` | file length < `0x60 + header[0x38] + header[0x58]` | `firmwareValidationFailed:` → `NSLog` |
| 3 | `"Corrupt: CRC checksum failed: expected 0x%lX actual 0x%lX"` | CRC32 over `[0x60+off, 0x60+off+len)` ≠ `header[0x3c]` | `firmwareValidationFailed:` → `NSLog` |
| 4 | `"Corrupt: Year %d"` | `CalendarDateFromFirmwareDate(header+0x08..0x17).yearOfCommonEra < 2003` | `firmwareValidationFailed:` → `NSLog` |

**Key semantic:** `firmwareValidationFailed:` is a passthrough to `_NSLog` with the formatted reason string (H6 `-firmwareValidationFailed:` decode above). **No `NSAlert`, no UI dialog, no delegate escalation.** The caller (`-deviceMatched:` at H2 log:170ish) only consults the return value `*(char*)(param_1 + 0x6a)` (set to `1` only on a *clean* validation); any corrupt path leaves it `0` and bypasses the update, falling through to `MAFirmwareStart`.

**Clone implication.** We only need to surface these four strings to the user if we want better-than-reference UX. Options:
- **Match reference:** forward `firmwareValidationFailed:` to `os_log` and move on. User sees nothing except in Console.app.
- **Improve:** attach the reason string to an `NSAlert` in the status-UI (H5 surface). Would need a 4th RPC verb (`validationFailed:<reason>`) in addition to the three H5 verbs.

The one unglamorous reason to decode this: the `"Year %d"` check means a BCD date before 2003-01-01 is treated as corruption, independent of CRC. This is the one soft signal of "bogus / forged firmware payload" the reference ships with.

## Follow-ups queued

- **H6a** — closed-by-H8 (sleep/wake/login hooks decoded in `H8-runtime-handoff.md §Sleep / wake / login`).
- **H6b** — resolved 2026-10-09 above (`kOSMetaClassHasInstances`).
- **H6c** — resolved 2026-10-09 above (4 reason strings).
