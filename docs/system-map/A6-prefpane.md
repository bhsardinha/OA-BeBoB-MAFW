# A6 — Prefpane (trampoline-only)

> **Correction (2026-10-14) — NIB contents** (evidence: [PROVENANCE-RESOLUTIONS.md §R19](PROVENANCE-RESOLUTIONS.md)). The 'probably an empty placeholder view' guess is verified: `PreferencesPanePref.nib/objects.nib` (NeXT typedstream) contains only `NSCustomObject` (File's Owner = `NSPreferencePane`), one `NSWindowTemplate`/`NSWindow`, one `NSView` named `View`, and one `NSNibOutletConnector` (`_window`); `strings` finds **0** control classes (`NSButton`, `NSTextField`, `NSImageView`, `NSPopUp…`, `NSBox`, `NSMatrix`, `NSControl`, `NSCell`). Info.plist facts (bundle id, `MALaunchApplication`/`MALaunchBundleID`, principal class `PreferencesPanePref`, universal i386+x86_64) and the three error/`didSelect` strings were re-read and match. (`ibtool` was unavailable; the typedstream was read with `strings`.)


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - 'The NIB probably has an empty placeholder view' (line 76) is a **guess — needs verification** (check = `ibtool --print-hierarchy` on the NIB).


**Status:** done 2026-10-09.
**Target:** `_extracted/prefpane/M-Audio FWPanel.prefPane/`.

## Headline

**It's a trampoline, nothing more.** The `.prefPane` bundle contains one `NSPreferencePane` subclass whose only job is to launch the Carbon panel app (`M-Audio FireWire.app`, bundle id `com.m-audio.appc.MAfw`) via `[NSWorkspace launchAppWithBundleIdentifier:…]`, then dismiss itself. No settings UI of its own, no IPC, no FireWire, no IOKit. Deepest useful insight for the clone: **the prefpane is not a reference for anything** — it's a UX convenience that offers a System Preferences entry point for the real panel. If we don't ship a System Preferences entry, the prefpane doesn't need a counterpart. If we do, it's a 50-line Swift/Obj-C bundle.

**This is why A6 was listed as "lowest C-value" in the top-level index.** Confirmed.

## Bundle facts

```
Bundle ID         com.m-audio.prefspanes.FireWirePanelTrampoline
Executable        Contents/MacOS/PreferencesPane  (universal: i386 + x86_64)
Version           1.3.4
Principal Class   PreferencesPanePref  (subclass of NSPreferencePane)
Main NIB          PreferencesPanePref.nib  (classes/info/objects — Carbon-era NIB)
Icon              PreferencesPanePref.tiff
NSPrefPaneIconLabel
                  "M-Audio FireWire\nFireWire 410"
Trampoline keys
  MALaunchApplication  "M-Audio FireWire.app"
  MALaunchBundleID     "com.m-audio.appc.MAfw"
```

**Universal binary quirk:** this is the only `_extracted/` component shipped as universal (i386 + x86_64). Rationale: a prefpane runs inside `System Preferences.app`, whose architecture varies with host OS (10.6/10.7+ shipped 64-bit System Preferences; 10.4/10.5 was 32-bit). The *real* panel app (`M-Audio FireWire.app`) is Carbon and therefore i386-only — the prefpane deliberately avoids inheriting that limitation by being a thin universal shim.

**Icon label mentions "FireWire 410":** the shipped label is hardcoded to the FW410 marketing name even though the trampoline is generic across the full product family. **Hypothesis — not worth verifying:** different `.prefPane` installer variants exist per product with only this string differing; or (more likely) this was simply the shipped default label that nobody updated post-FW410.

## Behavior — reconstructed from strings alone

Classes are not fully exposed by `otool -ov` on this older bundle (Objective-C metadata lives in a non-standard section for i386 bundles pre-10.5 ABI), but strings-grep gives the complete control flow of `PreferencesPanePref`:

```
Imports/strings used:
  NSPreferencePane
  NSObject
  NSWorkspace, +sharedWorkspace
  -launchAppWithBundleIdentifier:options:additionalEventParamDescriptor:launchIdentifier:
  -bundle, -infoDictionary, -objectForKey:
  -showAll:, -tryToPerform:with:
  "MALaunchApplication"
  "MALaunchBundleID"
  "Cannot open control panel"
  "The %@ control panel seems to be missing. Please try reinstalling the software."
  "didSelect"
```

**Reconstructed `-didSelect`** (the only non-trivial method; called by System Preferences when the user clicks the pane icon):

```objc
- (void)didSelect {
    NSString *bundleID = [[[self bundle] infoDictionary]
                           objectForKey:@"MALaunchBundleID"];
    BOOL ok = [[NSWorkspace sharedWorkspace]
               launchAppWithBundleIdentifier:bundleID
                                      options:0
               additionalEventParamDescriptor:nil
                             launchIdentifier:NULL];
    if (!ok) {
        NSString *appName = [[[self bundle] infoDictionary]
                             objectForKey:@"MALaunchApplication"];
        // show alert: "Cannot open control panel" / "The %@ control panel seems
        //              to be missing. Please try reinstalling the software."
        //              (format arg = appName)
    }
    // Return to the top-level System Preferences grid regardless of success:
    [NSApp tryToPerform:@selector(showAll:) with:nil];
}
```

**Observations:**

1. **No `-mainViewDidLoad` logic shown in strings** — the NIB probably has an empty placeholder view, and the real work is deferred until `didSelect` fires. System Preferences typical pattern.
2. **No IOKit, no CFMessagePort, no NSConnection, no FireWire.** Confirmed by the imports list: strings-grep for `IOKit`, `FireWire`, `FCP`, `AVC`, `HSCI`, `IOService` all return nothing. The prefpane is user-space-only and device-ignorant.
3. **`-showAll:` is sent to `NSApp` (= System Preferences)** — this closes the current pane and returns to the pane grid. So after clicking the M-Audio FireWire pane icon, the real panel app opens as a separate window and System Preferences bounces the user back to its main view. No "preferences pane window with embedded panel" illusion.
4. **The error alert is informative but passive** — "please reinstall." No attempt to find/relaunch; no retry; no fallback path to a stripped-down inline UI. Matches the overall "trampoline" character.

## What the prefpane does *not* do

Interesting negative-space observations:

- **It does not read device properties.** The prefpane doesn't know what FireWire device is plugged in; it couldn't display per-device status if it tried.
- **It does not persist settings.** No `CFPreferences` writes; no `.plist` in `~/Library/Preferences/`. All settings live in the panel app (A2).
- **It does not survive the launch.** Once the panel app opens, the prefpane process (System Preferences) and the prefpane instance stop participating in the UX. No live link, no update.
- **It does not require the kext.** Launching the panel app works regardless of whether `OpenAudioFW1814.kext` is loaded. The panel app then fails gracefully if no device is attached (per A2).

## Cross-references

- **A2-panel** (`A2-panel/README.md`) — the real UI target; `com.m-audio.appc.MAfw` is what gets launched. The prefpane is strictly upstream of A2 and does nothing A2 doesn't already handle better.
- **A4-helper** — unrelated. The helper is a LaunchDaemon that watches for devices and uploads firmware. The prefpane never talks to the helper.
- **A1-ghidra Q11** (user-client selectors) — the prefpane does not open a user-client. The panel app does (per A2).
- **CLAUDE.md "component split" paragraph** — mentions "a future Cocoa panel + CoreMIDI plugin drive control and MIDI." Does **not** mention a prefpane. Clone correctly omits it as optional UX surface.

## Clone implications (for Part D)

1. **Skip the prefpane in the clone.** Zero functional loss. Modern macOS's System Preferences / Settings has shifted away from third-party `.prefPane` bundles anyway (10.14+ discourages them; 13+ Settings uses a different plug-in model entirely). Install the control panel as a regular `.app` under `/Applications`; drop a Launchpad / Finder entry; done.
2. **If we ever want a System Preferences entry on 10.13 specifically** (where the `.prefPane` ABI still works), the trampoline pattern is trivial to clone: one Swift `NSPreferencePane` subclass with a `didSelect` identical to the reconstruction above, Info.plist with `MALaunchBundleID` pointing at our clone's panel bundle ID. 1 hour of work, no reverse engineering needed.
3. **The universal-binary trick is still correct in 2026-era builds** if we did ship a prefpane — build the trampoline as a modern universal bundle (`arm64 + x86_64`) even though the panel app is single-arch. The two components upgrade independently.
4. **The hardcoded "FireWire 410" in `NSPrefPaneIconLabel`** is a bug in the reference. If we ever ship this, use `"M-Audio FW 1814"` (or the clone's branding) and stop pretending we handle 8 products.

## No follow-ups

The prefpane is architecturally trivial. There is nothing further to decode. If an unexpected question comes up later (e.g. "does the prefpane set an environment variable before launching?"), re-grep strings and read the Info.plist — the whole bundle fits in one sitting.

## A6 closes Part A

With A6 done, **Part A — static disassembly of `_extracted/` — is complete.** All six components have primary decode notes:

| Component | File | Primary findings |
|---|---|---|
| Kext | `A1-ghidra/` | Class chain, probe/start/stop, DCL, HSCI, clock, mixer, user-client (Q1–Q17) |
| Panel | `A2-panel/` | Carbon NIB + binary; every control → kext call (P1–P10) |
| MIDI plugin | `A3-midi/` | CoreMIDI CFPlugIn, wire format, endpoint model, sleep (M1–M9) |
| Helper daemon + status UI | `A4-helper/` | LaunchDaemon lifecycle, firmware upload protocol, bootloader detection, retry, multi-product dispatch (H1–H10) |
| Firmware payload | `A5-firmware.md` | `bCoD` layout, both CRCs verified, BridgeCo attribute namespace enumerated |
| Prefpane | `A6-prefpane.md` | Trampoline only — launches panel app via NSWorkspace. Not a reference for anything |

**Next: Part B — cross-reference with FFADO, snd-bebob, Sierra kextload hack, macfw, and live SL Mac Pro DTrace.** B5 (live DTrace against the running v1.9 driver on `fw1814-sl` @ 192.168.100.17) is the only hardware-input step in the entire system map and is the gate for the sync/comm paths in Part C.
