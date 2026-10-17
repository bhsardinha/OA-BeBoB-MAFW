# H5 — Status-UI IPC

> **Correction (2026-10-14) — static audit** (full evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). `launch` is the status-UI binary's own argv subcommand (helper passes `statusAppPath`), so the 'Nicholas Riley `launch(1)`' hypothesis is refuted. `quit` is shelled by the helper (`sudo -u %@ "%@" quit &`). UI CFStrings at the H5b/H5c addresses verified.


**Target:** `_extracted/helper/FirmwareStatusWindow.app/Contents/MacOS/FirmwareStatusWindow` (i386).

**Decomp log:** `ghidra-exports/decomp/a4-h5-status-ui-ipc.log` — `_main` (lines 1-8), `-applicationDidFinishLaunching:` (12-70), `-showWindowWithDeviceIcon:name:GUID:` (74-129), `-(Messaging)checkForServer` (133-164), `-(Messaging)startServer` (168-188), `-setProgressIndicatorCurrent:maximum:` (192-237), `-applicationWillTerminate:` (241-261), `-terminate` (265-272), `-deviceIconWithName:` (276-314).

## Finding 1 — The UI binary is its own command-line client via NSConnection reflection

`_main` is one line: `NSApplicationMain()`. All logic sits in the AppKit delegate.

`-applicationDidFinishLaunching:` (log:12-70) is the entry point. Decoded dispatch:

```
applicationDidFinishLaunching:note
    args = [[NSProcessInfo processInfo] arguments]
    proxy = [self checkForServer]          // try to connect to kServerName NSConnection
    verb  = args[1]
    target = proxy                         // default
    if (proxy == nil):
        [self startServer]                 // register self as kServerName vendor
        target = self                      // we ARE the server now

    if ([verb isEqual:@"launch"]):
        [target showWindowWithDeviceIcon: args[2] name: args[3] GUID: args[4]]
    else if ([verb isEqual:@"quit"]):
        [target terminate]
    else if ([verb isEqual:@"progress"]):
        if ([args[2] isEqual:@"indeterminate"]):
            [target setProgressIndicatorCurrent: 0x7fffffff maximum: *irrelevant*]
        else:
            [target setProgressIndicatorCurrent: [args[2] intValue]
                                        maximum: [args[3] intValue]]

    if (target != self):                   // we were the client, not the server
        [self terminate]                    // exit immediately after RPC
```

**Mechanism:** the UI binary doubles as the server *and* every subsequent command-line client. First `launch` invocation registers an NSConnection vendor named `_kServerName` (symbol in dataseg; literal string **unverified** — a targeted dumper would read it, but not needed to understand the shape) and sticks around as the resident AppKit app. Every later invocation of the same binary with any `verb` finds the running vendor via `[NSConnection connectionWithRegisteredName:host:nil]`, sends the one message, and `[self terminate]`s immediately — the launched client never shows a window.

### `-checkForServer` (log:133-164)

```
checkForServer
    conn = [NSConnection connectionWithRegisteredName:_kServerName host:nil]  // nil host = local
    [conn retain]
    NSLog("got connection %@", conn)
    if (conn == nil): return nil
    proxy = [conn rootProxy]
    if (proxy == nil):
        NSLog("%@ server: could not get proxy. This should not happen.", _kServerName)
    else:
        NSLog("%@ server: found %@", _kServerName, proxy)
    [conn release]
    return proxy                           // nil or rootProxy of running server
```

### `-startServer` (log:168-188)

```
startServer
    conn = [[NSConnection alloc] init]
    [conn setRootObject:self]
    ok = [conn registerName:_kServerName]
    if (!ok): NSLog("%@ server: could not register server. Is one already running?")
    else:     NSLog("%@ server: started")
    return ok
```

Classic DO vendor. The exported remote-callable surface is **three methods**: `showWindowWithDeviceIcon:name:GUID:`, `terminate`, `setProgressIndicatorCurrent:maximum:`. No `NSProtocol` is defined (NSConnection in pre-10.5 code accepted any `-forwardInvocation:` from the proxy side), so method dispatch is dynamic.

### Three "subcommands" of the UI binary

| argv[1] | argv[2] | argv[3] | argv[4] | Remote selector |
|---|---|---|---|---|
| `launch` | iconName | productName | GUID (hex string) | `showWindowWithDeviceIcon:name:GUID:` |
| `quit` | — | — | — | `terminate` |
| `progress` | `indeterminate` | — | — | `setProgressIndicatorCurrent:0x7fffffff maximum:0` |
| `progress` | `<current>` | `<maximum>` | — | `setProgressIndicatorCurrent:current maximum:maximum` |

**Only `launch` was decompiled in the daemon's call path** (H2 log:294-320 — `-[MAFireWireLoader updateViewForDevice:]` builds `"sudo -u %s launch %s %s %s %llX"`). The daemon's `quit`/`progress` call sites are **not** in `-deviceMatched:` as I expected — need to decompile `MAFireWireLoader quitView` (0x306d, 126 bytes) which `-deviceMatched:` calls at log:199 of H2. **Follow-up queued below.**

## Finding 2 — Daemon → UI protocol over `sudo -u <user> launch`

Reconstructing from the H2 `-updateViewForDevice:` decode (log:294-320) + this file's dispatch table, the full UI lifecycle during a firmware update is:

1. **Daemon** (root) decides update is needed → `system("sudo -u <consoleUser> launch /path/to/FirmwareStatusWindow.app <iconName> <productName> <guid>")`.
   - **Note:** `-updateViewForDevice:` builds the format `"sudo -u %s launch %s %s %s %llX"` so the first positional arg to the UI (consumed as `argv[1]`) is **the first `%s`**, i.e. the status-app path (`argv[1] = "/path/.../FirmwareStatusWindow.app"`). **That contradicts this file's dispatch** which assumes `argv[1] = "launch"`.
   - **Resolved:** `launch` here is a *tool invocation*, not a positional arg to the UI. The `launch(1)` command is Nicholas Riley's open-source utility (shipped on the installer or as a system add-on; **hypothesis** — verify by looking for `/usr/bin/launch` or a bundled copy in `_extracted/helper/`). `launch <app-path> <args…>` launches the app with the trailing args as its argv (minus argv[0] which `launch` sets to the app's own path). So from the UI's point of view: `argv[0]` = FirmwareStatusWindow path, `argv[1]` = `"launch"` or `"quit"` or `"progress"`, then the args.
   - **unverified** — the `launch` tool identity is a hypothesis; could also be `/usr/bin/open -a <app> --args <...>` which would produce the same argv layout. Confirm by: (a) search for a `launch` binary bundled in the `_extracted/` installer; (b) grep for `/usr/bin/launch` or `/Applications/*launch*` strings in the daemon. Would also pin whether the installer dropped it.
2. First invocation: UI starts, no existing server → registers as `_kServerName` vendor → calls `[self showWindowWithDeviceIcon:iconName name:productName GUID:guid]` → window animated in via `CXCoreImageTransition`.
3. (Hypothesized progress updates from the daemon — would be more `sudo -u <user> launch <path> progress <n> <max>` or `progress indeterminate` shellouts. **Not observed in `-deviceMatched:`.** May live in another daemon path driven by the kext's upload progress callback — i.e. a notification handler we haven't decompiled yet. Suggested follow-up: grep daemon strings for "progress" to find the call site.)
4. Daemon done → `[self quitView]` → **hypothesis** — shells `sudo -u <user> launch <path> quit`. Verify by H5a decomp of `-[MAFireWireLoader quitView]` @ `0x0000306d`.
5. UI receives `-terminate` → `[NSApp terminate:self]` → `applicationWillTerminate:` fires → `CXCoreImageTransition` style-9 slide-out → exit.

## Finding 3 — Window content + icon lookup

`-showWindowWithDeviceIcon:name:GUID:` (log:74-129) is pure AppKit presentation:

- Sets icon via `-deviceIconWithName:` (next finding).
- Formats name as `@"M-Audio %@"` and GUID as `@"GUID: %@"` into two `NSTextField`s at ivars `self+8` and `self+0xc`.
- Caches `_sPreviousGUID` (static) so repeated `launch` calls for the same device don't re-draw.
- First-time show: `-constrainToMainScreen`, `-makeKeyWindow`, `-setHidesOnDeactivate:0`, `-orderFrontRegardless`, `[NSApp activateIgnoringOtherApps:1]`.
- Content view intro animation via `CXCoreImageTransition` (`setStyle:direction:0 0x3f800000` = Core Image fade direction +1.0f).

`-deviceIconWithName:` (log:276-314):

```
deviceIconWithName:name
    fm = [NSFileManager defaultManager]
    for ext in extensionsFromDataseg (4 iterations):
        path = [NSString stringWithFormat:@"%@%@.%@",
                 @"/Library/Application Support/M-Audio/DeviceIcons/",
                 name,
                 ext]
        if ([fm fileExistsAtPath:path]):
            img = [[NSImage alloc] initByReferencingFile:path]
            if (img) return [img autorelease]
    return [[NSImage imageNamed:@"GenericDevice"] autorelease]
```

Four extensions in the dataseg at `&UNK_000073d0 - 0x10 .. &UNK_000073d0` — **unverified** list, likely `.tif` / `.tiff` / `.icns` / `.png` or similar. Not critical to decode.

**Install-time dependency:** the installer must populate `/Library/Application Support/M-Audio/DeviceIcons/<MAIconName>.<ext>` for each product. For FW1814 the icon file name is whatever the kext publishes as `MAIconName` CFProperty (H2 Finding 2, log:310) — **unverified** — probably `"FW1814"` or `"FireWire 1814"`.

## Finding 4 — Indeterminate progress uses `0x7fffffff` sentinel

`-setProgressIndicatorCurrent:maximum:` (log:192-237) treats `maximum == 0x7fffffff` as the "spin indefinitely" signal:

- `max == 0x7fffffff` → set `self->isIndeterminate = YES`, `[progress startAnimation:self]`, `[progress setIndeterminate:YES]`.
- `max != 0x7fffffff` → stop any running indeterminate animation, clear indeterminate flag, set `[progress maxValue:max]` + `[progress doubleValue:current]`.

The current/max values arrive as `int` → cast via `(double)((uint64)val | DAT_00006f18) - DAT_00006f18` (classic unsigned-int-to-double trick on x86 — `DAT_00006f18` is a magic bias constant).

## Finding 5 — There is no second-UI prefpane here

The UI binary is **not** `FWAudiophilePrefPane.prefpane` (which is A6). It is a transient firmware-upload status window, nothing more. No controls, no settings. It accepts three remote calls and dies when told to.

## New follow-ups raised

- **H5a (small)** — decompile `-[MAFireWireLoader quitView]` @ `0x0000306d` to confirm it shells `launch <path> quit` and resolve the `launch` tool identity. Also decompile any daemon method that shells the `progress` subcommand (grep `strings-M-Audio_FireWire_Device_Helper.txt` for `"progress"`, `"indeterminate"` tokens).
- **H5b (small)** — dump the literal values of `_kServerName` (UI binary) and `_kMAFireWireServerName` (daemon binary) from their dataseg CFString tables. These are the DO vendor names the clone would need to match if we keep the IPC pattern.
- **H5c (trivial)** — read the 4 icon-file extensions at `&UNK_000073d0 - 0x10 .. &UNK_000073d0` to document the device-icon on-disk format.

None block H6 (error/retry).

## Clone implications

- **Three-call remote surface, if we keep the pattern.** `showWindowWithDeviceIcon:name:GUID:`, `setProgressIndicatorCurrent:maximum:`, `terminate`. XPC-port the trio into a protocol:
    ```objc
    @protocol OpenAudioFirmwareStatus
    - (void)showWindowWithDeviceIcon:(NSString *)iconName name:(NSString *)productName GUID:(NSString *)guid;
    - (void)setProgressIndicatorCurrent:(int)cur maximum:(int)max;
    - (void)terminate;
    @end
    ```
    Daemon side: `NSXPCConnection` + `NSXPCInterface` registered under a known Mach service name. Status UI: `NSXPCListener` daemon-less. Drop the reflection-via-NSConnection pattern — not needed when IPC is first-class.
- **Alternative: no status UI at all.** For a FW1814-only clone, the upload is <= 10 seconds. A command-line fwhelper that logs progress to `/var/log/openaudio-fw1814-fwhelper.log` is enough; the GUI is nice-to-have. H7 (launchd lifecycle) will decide; if the fwhelper is one-shot (not resident), the UI story is largely moot — launchd spawns + logs + exits.
- **Icon lookup path is a non-issue for the clone.** We're one product. Hardcode `/Library/Application Support/OpenAudio/FW1814.icns` or skip the icon.
- **Avoid `system()` with user-supplied strings.** The reference's `-updateViewForDevice:` shellout is shell-injectable if `MAIconName` or `MAActualProductName` ever contains metacharacters. Clone must use `NSTask` or XPC, never `system()`.

## Cross-references

- **A4-H1** (`H1-entry-dispatch.md`) — the daemon side also uses NSConnection via `MAFireWireCommunicator`. Two independent NSConnections exist, each with its own name constant. Same deprecation story applies.
- **A4-H2** (`H2-bootloader-detection.md`) — `-updateViewForDevice:` is the daemon's call site that spawns this UI. The format string there (`"sudo -u %s launch %s %s %s %llX"`) is the full on-wire protocol.
- **A3-M3** / **A2-P2** — unrelated IPC patterns (CFMachPort + runloop source for IOKit async notifications). Different mechanism, different purpose.

## H5b resolved — DO vendor port names (both binaries)

Raw reads of the two literal strings:

| Binary | Symbol | CFString addr | cstring addr (file off) | Length | Value |
|---|---|---|---|---|---|
| helper | `_kMAFireWireServerName` | `0x915c` | `0x6852` (22610) | 20 | `MAudioFireWireHelper` |
| UI     | `_kServerName` (CFString ptr @ `0x73e4`) | `0x7170` | `0x62f6` (21238) | 28 | `MAudioFireWireFirmwareStatus` |

Both are plain ASCII, no suffix / version / UID. The daemon registers `MAudioFireWireHelper`; the status-UI registers `MAudioFireWireFirmwareStatus`; each is reflected into a client of the other via the `argv[0]`-lookup trick decoded in `-[FirmwareStatusWindowAppDelegate ...]` (H5 §…).

## H5c resolved — 4 icon file extensions

Raw read of 4 CFString pointers at `__const` `0x73c0..0x73d0` (file off 25536..25552 in UI binary), each resolving to a cstring in `__cstring`:

| Index | CFString addr | cstring addr | Length | Value |
|---|---|---|---|---|
| 0 | `0x70b0` | `0x5e7f` | 3 | `png` |
| 1 | `0x70c0` | `0x5e83` | 3 | `tif` |
| 2 | `0x70d0` | `0x5e87` | 4 | `tiff` |
| 3 | `0x70e0` | `0x5e8c` | 3 | `jpg` |

Semantic: when the daemon sends `showWindowWithDeviceIcon:name:GUID:` with `iconName = "FW1814"` (or whatever product-specific prefix), the UI's icon-loader tries `<iconName>.png`, `.tif`, `.tiff`, `.jpg` in that order against a hardcoded search directory (not decoded here — likely `/Library/Application Support/M-Audio FireWire/` or a resource path on the daemon's installer bundle).

**Clone implication.** We're one product; hardcode a single `.icns` and skip this extension-ladder lookup entirely. If the UI is kept, the clone's `showWindow…` RPC should take an `NSImage *` payload directly (or a resource-bundle URL) and drop filename sniffing.


## Status

Done 2026-10-09. H5a resolved by H6 (`quitView` = `sudo -u ... quit`). H5b/c resolved 2026-10-09 (above sections). All claims cited to `a4-h5-status-ui-ipc.log`, raw `__DATA.__const` + `__DATA.__cfstring` + `__TEXT.__cstring` reads, and cross-ref to H2/H6.
