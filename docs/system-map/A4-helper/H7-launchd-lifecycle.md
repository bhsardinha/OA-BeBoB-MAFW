# H7 — launchd lifecycle

> **Correction (2026-10-14) — static audit** (full evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Confirmed: no `signal`/`sigaction`/`atexit` imports; plist has only Label/RunAtLoad/ProgramArguments. SL v1.9: restarting the helper via `launchctl unload/load` makes it push three property writes (capture 71).


**Status:** done 2026-10-09.
**Target:** `M-Audio FireWire Device Helper` i386 daemon + `com.m-audio.firewire.helper.plist`.
**Log:** `ghidra-exports/decomp/a4-h7-launchd-lifecycle.log` (DecompA4H7.java).

## Headline

The daemon is a **resident NSConnection Distributed-Objects server**, not a one-shot upload worker. launchd starts it at boot via `RunAtLoad=true` with argv `[path, "start"]`; the process blocks forever in `NSRunLoop runMode:beforeDate:distantFuture` serving a DO proxy named `kMAFireWireServerName`. The firmware-upload logic (per H2/H6) is driven by IOKit notifications from inside the resident daemon's runloop, not by the argv `start` subcommand. **`start` and `stop` are DO methods invoked by *subsequent* CLI invocations against the already-running daemon — they do not themselves start/exit the process.**

**Clone implication:** our LaunchDaemon stays loaded. One `/Library/LaunchDaemons/*.plist` entry, `RunAtLoad=true`, no `KeepAlive`, no custom signal handling. SIGTERM from launchd kills the process bluntly on logout/shutdown; the daemon has no `-applicationWillTerminate:`-style hook. If we need "upload done, exit" semantics we must *add* them; the reference does not do it.

## plist (dumped 2026-10-09)

```
Label               com.m-audio.firewire.helper
ProgramArguments    "/Library/StartupItems/M-Audio FireWire Audio Helper/M-Audio FireWire Device Helper"
                    "start"
RunAtLoad           true
```

No other keys. Missing: `KeepAlive`, `UserName`, `ThrottleInterval`, `WatchPaths`, `StartInterval`, `WorkingDirectory`, `StandardOutPath`, `StandardErrorPath`, `MachServices`, `Sockets`, `LowPriorityIO`.

**Reading each absence:**

- `KeepAlive` absent → launchd does **not** restart the daemon if it crashes or exits. Combined with "the firmware upload has no retry story" (H6), a crash during upload is a terminal failure until next boot. **Clone follow-up:** decide whether to add `KeepAlive=true` or `KeepAlive={SuccessfulExit=false}`. Probably yes.
- `UserName` absent → runs as root (launchd default under `/Library/LaunchDaemons`). The reference exploits this to call `IOCatalogueTerminate` (per H6) and `setProperty:` on IOKit nubs. Our clone needs the same.
- `MachServices` absent → the IPC port is **not** a launchd-managed Mach port. It's an NSConnection-registered name via NSPort/mach_port registration directly (see `[NSConnection registerName:]` at `+[MACommunicator serve:forName:]` log:137). This is pre-XPC Obj-C IPC; on a modern port we would move to XPC with a MachServices entry.
- `StandardOutPath`/`StandardErrorPath` absent → `NSLog` output goes to the launchd-captured stderr → ASL/`system.log` on 10.13. That's how we observe the helper's error strings (H6 inventory).

**Lifecycle quirk — StartupItem path:** the plist points at `/Library/StartupItems/M-Audio FireWire Audio Helper/M-Audio FireWire Device Helper` rather than `/Library/LaunchDaemons/...`. StartupItems were deprecated in 10.4 but still honored by launchd through 10.13. The installer evidently ships both the StartupItem bundle and the launchd plist, with the plist re-pointing into the StartupItem's executable. **Our clone drops the StartupItem bundle entirely** — install directly under `/Library/LaunchDaemons/`.

## Lifecycle walk

### Boot (launchd starts the daemon)

```
launchd → fork+exec("...FireWire Device Helper", "start")
   argc = 2, argv[1] = "start"
```

`_main` (log:3-50):

1. Alloc outer autoreleasepool.
2. `iVar3 = [MAFireWireCommunicator getServer:kMAFireWireServerName]` (log:17-18)
   → tries `[NSConnection connectionWithRegisteredName:host:]` with nil host, i.e. local machine (`+getServer:` log:189-190).
   → on **first boot** no server registered → returns 0.
3. `if (iVar3 == 0)` branch (log:19-27):
   - Alloc inner pool.
   - `uVar5 = [MAFireWireCommunicator alloc]` → `[uVar5 init]`.
   - **`-[MAFireWireCommunicator init]` auto-calls `[self start]`** (log:65) — this is the real startup trigger, not argv.
   - `[communicator runServer:kMAFireWireServerName]` — **blocks forever** (see below).
4. Argv dispatch (log:30-45) is guarded by `(argc > 1) && (iVar3 != 0)` — on first boot `iVar3 == 0` so **`argv[1] = "start"` is never looked at**.

**So on the launchd boot path, `argv[1]="start"` is purely cosmetic.** The real init is `[MAFireWireCommunicator init]` auto-calling `start`, and the real serve is `runServer:`. The `start` argv string exists so the *second* CLI invocation path (below) works symmetrically.

### `runServer:` → `+serve:forName:` → `-serve` (the eternal loop)

`-[MACommunicator runServer:]` (log:152-162) is a thin forwarder to the class method `+serve:forName:`.

`+[MACommunicator serve:forName:]` (log:123-148):
1. `[[NSConnection alloc] init]`.
2. `[conn setRootObject:self]` — the `MAFireWireCommunicator` instance is the DO root object; anyone calling `+getServer:` from another process gets a proxy to this instance.
3. `[conn registerName:kMAFireWireServerName]` — publishes to the Mach port namespace.
   - **Failure path:** `NSLog("%@ could not register server. Is one already running?")` and return — the daemon then falls back to the outer autoreleasepool release and exits. **Means launchd running the daemon twice (e.g. after a crash + manual relaunch) results in silent exit of the second instance.** No retry.
4. `NSLog("%@: started server")`.
5. `[self serve:...]` — enter runloop.
6. On exit from serve, `NSLog("%@: stopped server")` and release the connection.

`-[MACommunicator serve]` (log:102-119):
```c
self->serving = 1;        // ivar at +4
do {
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]];
} while (self->serving != 0);
```

Pumps the main runloop in the default mode with no timeout. This is where IOKit notification callbacks (per H2's `IOServiceAddMatchingNotification` on `com_m_audio_FWMetaNub`) fire — the daemon's runloop is the single serialization point for both DO method dispatch *and* device-match callbacks.

**Exits only when `self->serving` is set to 0**, which happens via `-stopServer` (log:166-173). `stopServer` is **not called from argv** (not from `-stop`, which releases the loader but leaves `serving` flag alone). The only in-process paths that could set it are reflection or an explicit `-applicationWillTerminate:` — **neither exists in this binary**. So `-serve` effectively never returns during normal operation; the only way the daemon exits is `exit()`-style termination from launchd (SIGTERM/SIGKILL on logout, shutdown, or `launchctl unload`).

### Subsequent `start` / `stop` CLI invocation (while daemon running)

A later invocation of the helper binary with argv `start` or `stop` takes a different path:

```c
iVar3 = [MAFireWireCommunicator getServer:kMAFireWireServerName];  // returns proxy
// iVar3 != 0, so skip the runServer: branch
if (argc > 1 && iVar3 != 0) {
    if (strcmp(argv[1], "start") == 0) [proxy start];
    else if (strcmp(argv[1], "stop") == 0) [proxy stop];
}
// release pools, return 0
```

The second-invoker's process does **not** become a server; it's a transient DO client that sends one message and exits. The resident daemon receives the message in its runloop (via `-serve`) and:

- `-[MAFireWireCommunicator start]` (log:71-85): guard `if (self->loader == 0)` + `self->loader = [[MAFireWireLoader alloc] init]`. **Idempotent** — a `start` sent to an already-started daemon is a no-op (just the `NSLog("starting")`).
- `-[MAFireWireCommunicator stop]` (log:89-98): `[self->loader release]; self->loader = nil`. **Does not exit the process.** The DO server keeps running; the device-matching loader is torn down. A subsequent `start` can bring it back without respawning the daemon.

**Why this matters for the clone:** `stop` is a *pause*, not an uninstall. The uninstaller must `launchctl unload` the plist in addition to (or instead of) sending `stop`. See H1a follow-up — the installer/uninstaller scripts are external to the daemon binary.

### Shutdown / logout / `launchctl unload`

launchd sends SIGTERM. **No signal handler is installed** — `grep signal/sigaction/_atexit` on the function listing returns zero matches. So:

- Any in-flight firmware upload (`setProperty:@"MAFirmwareUpgradeImage"` currently blocked in-kext) is interrupted at the Mach-message boundary. The kext may be left in an undefined state until the next bus reset or kextcache rebuild.
- No `[self stop]`, no `[loader unloadTheDriver]`, no connection unregister. launchd cleans up the Mach port on process death.

**Clone implication:** add a SIGTERM handler if we want graceful shutdown — specifically to let an in-flight upload finish or to drive `unloadTheDriver` before process exit. For MVP parity we can skip it (the reference doesn't), but **mark it as a known gap**.

### Interaction with `RunAtLoad`

`RunAtLoad=true` means launchd starts the daemon at `launchd` boot time (i.e. before any user login window). The guard inside `deviceMatched:` of `[MAConsoleUser isUserLoggedIn] == NO → return` (per H6) means:

- At boot, before any user logs in: daemon runs, DO server up, IOKit callbacks fire, but every `deviceMatched:` **short-circuits** at the `isUserLoggedIn` check. No firmware update happens at the login window.
- First user login triggers `userDidLogIn:` (not yet decoded — H6a) which must re-walk the ioreg and re-fire `deviceMatched:` for already-present devices. **Hypothesis confirmed by log:4-7 in H6** that this is the mechanism, but the actual code path is pending.

**Clone implication, continued from H6:** this is why the reference binary is a long-running daemon rather than a one-shot. It must survive from boot through first login and beyond, because IOKit matches at boot time and users log in later. A `KeepAlive`-less, one-shot replacement would silently miss any device plugged in before login.

## Resident daemon vs one-shot — the decision

The reference ships **resident daemon**. Reasons (observed, not inferred):

1. **DO server has no shutdown path** (see `-serve` analysis above). The binary is architecturally one-shot-hostile.
2. **IOKit callbacks are runloop-driven** (H2) and must stay alive for the lifetime of the device being watchable.
3. **`isUserLoggedIn` guard** means the daemon must outlive the login window to handle devices plugged in before login.
4. **No KeepAlive in plist** means the daemon trusts launchd to only start it once; it's not a worker that re-exits per job.

**Our clone must match this.** M2's current `exit-after-upload` assumption (noted in H2's `0e8fb01` → M2 scaffolded commit; `openaudio-fw1814/firmware-helper/` is a Go process) is **invalid**. The clone's fwhelper must either:

- (a) stay resident with an IOKit notification source — like the reference.
- (b) stay resident with a launchd `WatchPaths` or `MachServices` wake trigger — modernized but equivalent.
- (c) accept the limitation that devices plugged in before daemon-launch will be missed, and compensate via `KeepAlive={SuccessfulExit=false}` or `MachServices`-on-demand.

**Recommendation (to be lifted into Part D):** option (a). Minimal deviation from the reference, keeps the "every bus event re-fires `deviceMatched:`" invariant.

## Cross-references

- **H1** (`H1-entry-dispatch.md`) — argv `start`/`stop` dispatch; H7 explains that argv is cosmetic on the launchd path and only meaningful for CLI client invocations.
- **H2** (`H2-bootloader-detection.md`) — IOKit notifications fire inside the `-serve` runloop. The resident runloop is why `IOServiceAddMatchingNotification` + CFRunLoopSource work.
- **H5** (`H5-status-ui-ipc.md`) — status app's own NSConnection is a separate, per-launch DO server (different name, `_kServerName`). The helper daemon's DO is `kMAFireWireServerName`. Two independent name registrations.
- **H6** (`H6-error-retry.md`) — `-serve`'s runloop is also where `unloadTheDriver`'s `[NSRunLoop runUntilDate:]` retry runs (log:218-221 in H6 log) — same runloop, same mode.
- **A1-Q7 #2** — "does the helper exit after upload" question is now answered: **no.** The daemon is resident; successful upload returns control to the runloop and waits for the next re-match.

## Clone implications (for Part D)

1. **LaunchDaemon stays loaded.** `RunAtLoad=true`, no `KeepAlive` needed if the daemon doesn't crash; add `KeepAlive={SuccessfulExit=false}` for defensive restart.
2. **Install under `/Library/LaunchDaemons/`**, not `/Library/StartupItems/`. Our current clone fwhelper layout already does this per CLAUDE.md.
3. **Pick a single serialization primitive** for both the IPC server and IOKit callbacks — reference uses the NSRunLoop. Our Go-based fwhelper doesn't have NSRunLoop; the equivalent is a single goroutine draining `IOIterator` from `IOServiceAddMatchingNotification`. **This is the biggest architectural translation step for the clone fwhelper.**
4. **Expose a `stop` IPC verb** (not just SIGTERM) so an uninstaller can tear down the loader without killing the daemon. Current clone fwhelper has no IPC surface — **follow-up:** add one (XPC listener or a Unix-domain socket).
5. **Add a SIGTERM handler** that drives graceful teardown (release loader, finalize any in-flight upload). Reference skips this; we should not.
6. **DO replacement: XPC.** NSConnection DO is deprecated on 10.13 and gone from AppKit on 10.14. If we ever stay on this architecture we rewrite the IPC layer in XPC. **For MVP, we don't need DO at all** — the clone's control panel can speak directly to the kext via a user client (per A1-Q11 / `12-USERCLIENT-DESIGN.md`), and the fwhelper's IPC surface can be local-only.

## Follow-ups queued

- **H7a** — decompile `-[MAFireWireLoader systemWillSleep]` / `systemWokeUp` / `userDidLogIn:` / `userDidLogOut:` to confirm the "wake / login re-walks ioreg" hypothesis (shared with H6a). 15 min.
- **H7b** — check for `-[NSApplication delegate]`-style termination hooks on `MAFireWireCommunicator` (unlikely given no NSApplication is instantiated, but verify). 5 min — grep strings for `applicationWillTerminate` → already shown absent, but `atexit` was also absent per grep so this is **closed**.
- **H7c** — read the actual installer/uninstaller shell scripts for `start`/`stop`/`launchctl load`/`launchctl unload` sequencing — belongs to a future A4-installer sub-pass, not in-scope here. **Deferred to A6 or later.**
