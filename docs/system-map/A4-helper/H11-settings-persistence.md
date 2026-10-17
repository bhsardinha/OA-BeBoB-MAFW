# H11 — Settings persistence + replay (the helper's second job)

> **Correction (2026-10-14) — two instances per start** (evidence: [PROVENANCE-RESOLUTIONS.md §R16](../PROVENANCE-RESOLUTIONS.md)). Each helper start creates **two** `FWUserClient`s (SL ioreg: pids 463, 492, 534, 559 each have 2) and issues **two** 6570-byte prefs writes, i.e. two device instances per start; why `driverInstanceMatched:` delivers the same device twice is **unexplained** (hypothesis: overlapping class matchers). The stale clients of earlier helper pids confirm the kext's missing `terminate()` (see Q-UC-surface).


**Status:** done 2026-10-14. **Bundle:** v1.11.2 `_extracted/helper/M-Audio FireWire Device Helper` (i386). **Evidence:** Ghidra decompile `ghidra-exports/decomp/a4-prefs.log` (script `scripts/DecompA4Prefs.java`, run headless read-only against `ghidra-project/helper`), xref log `ghidra-exports/decomp/a4-xref-configureDCLLatency.log` (`scripts/XrefA4Sel.java`), SL v1.9 live capture 71, SL `defaults read`. This closes matrix item `A1-Q3b.3` and answers the 'who reapplies saved preferences?' question left open by H2/H8/B5.

## Finding — the helper, not the Panel, owns persistence

Each attached product gets a `MAFireWireDeviceInstance` (created by `-[MAFireWireLoader driverInstanceMatched:]`, @0x35d5, for each of the 8 product classes). `+deviceInstanceForService:` (@0x23a0):

1. `IOServiceOpen(service, mach_task_self(), type 0, &conn)` — **its own user client** per device (the helper is the second client next to the Panel; B5 §3).
2. `CanonicalGUIDForDevice` → settings key (`[+0x10]`, a hex string such as `D6C040074B9FB`).
3. `MAAsyncNotification userClientNotificationOfType:0 …` bound to `-port:sentAsyncMessage:ofSize:` → `-receivedPropertyChangeMessage:` (@0x26df): when a user is logged in it `IOCFUnserialize`s the notification body (at message offset `+0x24`) and **merges it into `fSettings` (`[+0xc]`)**, then `cancelPreviousPerformRequests…commitSettings:` and `performSelector:commitSettings: afterDelay: 4.0` (`0x40100000` = high word of the double 4.0). This is the kext's type-0 'dirty properties' port (Q-UC-surface §4) with the helper as subscriber (resolves AUD-C1-08).
4. `-service:changedProperties:` (@0x2c13) also merges IOKit interest-notification changes into `fSettings`.
5. If a console user exists: `-loadPrefsForUser:`; else `NSLog("No user logged in")`.

`-commitSettings:` (@0x2892): if the dirty flag `[+0x18]` is set, under `WithCurrentEffectiveUserAndGroup` (drops to the user's uid/gid) `CFPreferencesSetValue(key = GUID, value = fSettings, appID = "com.m-audio.device-settings.firewire", user, kCFPreferencesAnyHost)` + `CFPreferencesSynchronize`. So settings are persisted **per GUID, per user, 4 s after the last change** (coalesced).

`-loadPrefsForUser:` (@0x2afe): `CFPreferencesCopyValue(GUID, appID, user, AnyHost)`. If present: copy into a mutable dict (`[+0xc]`) and push the **whole dict** with one `IORegistryEntrySetCFProperties(service, dict)`. If absent: read the device's current `MAMetaAllProperties` (kext `FWPropertyDict::CopyProperty` special case) into `fSettings` as the seed — no write. Called at device-instance creation, and for all devices at user login (`userDidLogIn:` → `makeObjectsPerformSelector:loadPrefsForUser:`, H8).

## Live corroboration (SL v1.9, capture 71)
Restarting the helper produced three registry writes from the helper: **60 B**, **6570 B**, **6570 B**. The 60-byte one is exactly `<dict><key>MAFirmwareStart</key><string>YES</string></dict>` + NUL (computed length 60) = `deviceMatched:`'s 'firmware current → Starting <product>' write (H2). The 6570-byte ones are the `loadPrefsForUser:` replay (`captures/23-fw1814-current-prefs.xml` is also 6570 bytes — the stored per-GUID dict). **Why two** 6570-byte writes (one attached FW1814, GUID 0xD6C040074B9FB; prefs plist also holds a second GUID 0xD6C040012337F) is **unexplained** — hypothesis: a second `loadPrefsForUser:` from another entry point; check = trace `loadPrefsForUser:` callers with Ghidra xrefs.

## Dead code: `configureDCLLatency`
`-[MAFireWireDeviceInstance configureDCLLatency]` (@0x290a) reads `hw.cpufrequency`/`hw.cpusubtype` and writes the CFProperty `MAMetaConfigureDCLLatency` (a packet count: 128 for 'pre-G3' i.e. any cpusubtype < 9 — which includes Intel; 64 for G3; 32/16/8 for G4 by MHz; 8 for G5). The xref log shows its selector and implementation referenced **only from the ObjC method tables** (data `0xab38/0xab40/0xbab8`), **no code call site**, and the kext contains no `MAMetaConfigureDCLLatency` string (`strings`/`mafw.asm`: only `MAMetaAllProperties`, `MAMetaResetToFactorySettings`, `MAMetaVirtualRotateDataKnob`). Conclusion: a PowerPC-era remnant; the reference does **not** use it on Intel. (A dynamic call via `performSelector:` with a computed name is not excluded statically.)

## Clone implications (reasoning)
* Settings persistence and replay is part of 'full feature parity': a daemon must (a) subscribe to the kext's type-0 dirty-property port, (b) persist per GUID/user with ~4 s coalescing, (c) replay with one `setProperties` on attach and at login. The Panel need not write preferences.
* Do not implement `MAMetaConfigureDCLLatency` (no consumer).
