# H1 — Entry + argv dispatch

> **Correction (2026-10-14) — static audit** (full evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Verified against the i386 asm: argc≥2 and non-nil server guard (0x22fe-0x2305), `strcmp` 'start'/'stop' (0x68ae/0x68c4). DO server name `MAudioFireWireHelper`.


**Target:** `_extracted/helper/M-Audio FireWire Device Helper` (i386).

**Decomp log:** `ghidra-exports/decomp/a4-h1-entry-dispatch.log:1-71` (`_main` @ `0x000021ee`, `entry` @ `0x000021b0`).

## Finding — the helper is an NSConnection "server" singleton; argv is lifecycle control, not a work selector

### `entry` @ `0x000021b0` (log:54-71)

Standard C-startup trampoline. Walks past the environ pointer, calls `_main`, `_exit(status)`. Nothing custom.

### `_main` @ `0x000021ee` (log:1-50)

Pseudocode in prose:

```
_main(argc, argv):
    pool = [[NSAutoreleasePool alloc] init]
    server = [MAFireWireCommunicator getServer:kMAFireWireServerName]
    if (server == nil):
        pool2 = [[NSAutoreleasePool alloc] init]
        tmp = [[MAFireWireCommunicator alloc] init]
        server = tmp
        [pool2 release]
        [server runServer:kMAFireWireServerName]       // registers DO vendor; blocks until stopped
    pool3 = [[NSAutoreleasePool alloc] init]
    if (argc > 1 && server != nil):
        NSLog("Server running")
        if (strcmp(argv[1], "start") == 0):
            NSLog("Starting server")
            [server start]
        else if (strcmp(argv[1], "stop") == 0):
            NSLog("Stopping server")
            [server stop]
        // else: silently fall through
    [pool3 release]; [pool release]; return 0
```

### What this proves (and what it rules out)

1. **IPC is NSConnection / Distributed Objects**, not Mach ports directly, not XPC. Cited strings: `0x7837 "could not register server. Is one already running?"`, `0x78da "could not find server. You need to start one on this machine first."`, `0x7934 "could not get proxy."`, and the Obj-C class reference `0xdfba .objc_class_name_NSConnection` (`strings-M-Audio_FireWire_Device_Helper.txt`). The `getServer:` / `runServer:` naming pattern on `MAFireWireCommunicator` is the classic DO vendor/consumer idiom. Verified-by-use further in `-[MAFireWireCommunicator start]` / `stop` once we decompile them (deferred to H1a).
2. **The daemon is a long-lived singleton.** The *first* invocation (launchd's `argv={..., "start"}`) hits the `server == nil` branch, calls `runServer:` which **blocks forever** registering + servicing the DO vendor. The subsequent `[server start]` is sent in-process to the just-created instance; from the DO side, every later CLI invocation is a *client* that uses `getServer:` → non-nil → skips the `runServer:` block and just sends `[server start]` or `[server stop]` over DO.
3. **`"start"` and `"stop"` are lifecycle verbs on the server object, not firmware actions.** No `upload`, no `install`, no `verify`, no product-name subcommand surfaces here. Any argv token other than `start`/`stop` silently falls through (no usage string, no exit code). All firmware work happens *inside* the running server, driven by IOKit matching (`MAFireWireLoader.deviceMatched:` — H2).
4. **No root check at main() entry.** The daemon runs as whatever launchd gives it (root per `com.m-audio.firewire.helper.plist` → implicit root for pre-sandbox LaunchDaemons on 10.7–10.13). A sibling class `WithCurrentEffectiveUserAndGroup` (classes-M-Audio_FireWire_Device_Helper.md:102-106) exists as an RAII guard for *temporarily* dropping euid/egid when touching user-owned files — not for gating privileged work.
5. **No `getopt`.** Dispatch is a two-prong `strcmp` chain. The clone's fwhelper doesn't need to grow a flag parser.

### Open sub-questions (decoded in H1a / H1b)

- **H1a — `-[MAFireWireCommunicator start]` body.** What does it actually do? Expectation (**hypothesis**): kick off IOKit matching notifications + register the sleep/wake handler. Verify by decomp — my first-pass search missed the Communicator methods because the symbol strings are `MAFireWireCommunicator_start` (underscore) and my matcher didn't normalize `[Cls method]` → `Cls_method`. Follow-up decomp will target `MAFireWireCommunicator_start` / `_stop` / `_init` / `_runServer:` / `_getServer:`.
- **H1b — DO server name constant.** `_kMAFireWireServerName` is an exported symbol (`strings-M-Audio_FireWire_Device_Helper.txt:0xdbc9`); its string value is the DO port name the panel / UI must know. Grab the literal via a targeted dump of the dataseg at `_kMAFireWireServerName`'s address so the clone's control-panel + status-UI can bind to it.
- **(deferred to H5)** The status-UI binary (`FirmwareStatusWindow`) — does it also open an NSConnection to this port, or receive callbacks over distributed notifications? H5 decides.

## Clone implication

- **Keep the daemon-singleton pattern** for the clone's `openaudio-fw1814-fwhelper`, but drop NSConnection: it's deprecated post-10.7 and will fight sandboxing on 10.14+. Replacement path:
    - **10.13.6 shipping target:** XPC service (`NSXPCConnection` + `NSXPCInterface`) with a Mach service name registered in the LaunchDaemon plist. Same singleton semantics, modern IPC.
    - **Alternate (simpler for one-shot):** no IPC at all. Make the helper one-shot — exit after a successful upload — and let launchd re-run on bootloader-mode reappearance via `KeepAlive = { SuccessfulExit = NO }` + `OnDemand` IOKit matching. **H7 decides** between resident-singleton vs one-shot; both are compatible with the kext.
- **Argv surface is `start` and `stop` only.** The clone's `make load` / `make unload` already map to `launchctl load/unload` which calls the plist's `ProgramArguments` as-is — no new subcommands needed. If we go one-shot, argv dispatch disappears entirely.
- **StartupItem path in the reference plist is a 2007-era holdover.** Our LaunchDaemon plist should live in `/Library/LaunchDaemons/` directly (where it already is per `openaudio-fw1814/firmware-helper/`), with no StartupItem bundle.

## Cross-references

- **A3-M3** (`A3-midi/M3-async-notifications.md`) — CFMachPort + runloop-source IPC pattern on the plugin side. Different mechanism (CFMachPort direct from an IOKit async notification port) than this binary's NSConnection; cite the contrast when C3 (comm path) is written.
- **A2-P2** (`A2-panel/P2-notification-receive.md`) — ditto, panel-side.
- **A1-Q7** (`A1-ghidra/Q7-fwmetanub-lifecycle.md`) — the kext side of the bootloader↔runtime transition. H2 (next) is the userland complement that wakes this daemon on `0x10070` appearance.

## H1b resolved — DO vendor port name

Literal value of `_kMAFireWireServerName` (helper binary, symbol @ `0x97c0`, points to CFString @ `0x915c` → cstring @ `0x6852`, length 20):

```
"MAudioFireWireHelper"
```

This is the NSConnection DO port name the daemon registers at `-[MAFireWireCommunicator start]` and that any client (status-UI, panel, uninstaller) must pass to `+[NSConnection connectionWithRegisteredName:host:]` to reach it. Verified by raw read of `__DATA.__cstring` (file offset 22610 = 0x5852).

Note the asymmetry with the UI binary's `_kServerName` = `"MAudioFireWireFirmwareStatus"` (H5b) — the two components register two different DO port names, and each is both a server (publishing its name) and a transient client (looking up the other's name) as decoded in H5.

**Clone implication.** If we keep the NSConnection IPC pattern (we won't — H1 clone-plan replaces it with XPC per `_kOpenAudioHelperXPCService`), the clone's helper port name should namespace to `com.openaudio.firewire.helper` and avoid the historical `MAudioFireWire…` prefix.

## Status

Done 2026-10-09. H1a closed-by-H2 (see H2 §Finding 4). H1b resolved 2026-10-09 (above). Cited against `ghidra-exports/decomp/a4-h1-entry-dispatch.log` (lines 1-71), `ghidra-exports/strings-M-Audio_FireWire_Device_Helper.txt`, and raw `__DATA.__cstring` read at file offset 22610.
