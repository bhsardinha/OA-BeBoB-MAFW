# H8 — Runtime handoff + sleep/login re-walk

> **Correction (2026-10-14) — settings persistence** (evidence: [A4 H11](H11-settings-persistence.md)). `loadPrefsForUser:` was never decompiled here; it is now (H11): `CFPreferencesCopyValue(GUID, "com.m-audio.device-settings.firewire", user)` then a single `IORegistryEntrySetCFProperties(service, dict)`; `commitSettings:` persists 4 s after the last type-0 dirty-property notification.


> **Correction (2026-10-14) — static audit** (full evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Steps 1-2 are inverted: `MAFirmwareInfo[0xc] != 0` is the **bootloader** state (upload); `== 0` is **runtime** (Halt). Blob = 80 bytes (kext `FirmwareCacheInfoRegisters @0x1cc9e`). The helper's pref push (60/6570/6570 B) was observed live on SL v1.9; payload undecoded.


**Status:** done 2026-10-09.
**Target:** `M-Audio FireWire Device Helper` i386 daemon.
**Log:** `ghidra-exports/decomp/a4-h8-runtime-handoff.log` (DecompA4H8.java).

## Headline

The helper watches **both** bootloader (`0x10070`) and runtime (`0x10071`) devices through a *single* class-name match on `com_m_audio_FWMetaNub` — not by Model_ID. The kext publishes `FWMetaNub` for both IDs, and `deviceMatched:` distinguishes bootloader vs runtime at runtime by reading `*(firmwareInfo + 0xc)` (the "firmware running" flag, per H6 log:91). Separately, the helper also registers **8 per-product driver-class watchers** (one per M-Audio FireWire product) via `driverInstanceMatched:` / `driverInstanceTerminated:` — these fire **only** when the full audio driver instance attaches to the runtime nub. That's the explicit "handoff complete" signal.

**No explicit bus-reset is kicked by the helper after upload.** The device self-resets (bootloader → runtime), which triggers a fresh IOKit match naturally. The helper's responsibility ends at `setProperty:@"MAFirmwareStart"` + `quitView`; the kext + IOKit handle the rest.

**Also resolves H6a / H7a** — sleep/login hooks fully decoded here.

## IOKit registration (init)

`-[MAFireWireLoader init]` @ `0x2c3d` (log:1-40):

1. `IOServiceMatching("com_m_audio_FWMetaNub")` (log:17) — the **catch-all nub class**, matched by both `0x10070` (bootloader) and `0x10071` (runtime) kext personalities.
2. `[MAServiceNotification observeServiceMatched:target:sel:]` with `sel = -deviceMatched:` — H2 showed this is the firmware-upload entry point.
3. Loop over an **8-entry class-name table** at `&sSystemSleeping - 0x20` (log:23-35). Each iteration:
   - `IOServiceMatching(classNameTable[i])`
   - Registers `-driverInstanceMatched:` on service-matched
   - Registers `-driverInstanceTerminated:` on service-terminated
4. `[MAPowerNotification observeSleepNotifications:]` — wires `-systemWillSleep` / `-systemWokeUp`.
5. `[MAConsoleUser startObservingLogins:]` — wires `-userDidLogIn:withUID:withGID:` / `-userDidLogOut:withUID:withGID:`.

**The 8-entry table is the per-product driver class list.** Resolves H10 directly (see §H10 below). Reading the table literally from the binary is pending — simple hex dump at the symbol address. Based on the H2 class landscape and the "all 8 M-Audio FireWire products" scope, the entries almost certainly correspond to: FW1814, FWAP (Audiophile), FW410, FW1200, FWOxyC (ProjectMix), FWSolo, FWMobilePre, FW1 (ProFire?) — **hypothesis**, verification queued as H8a.

## State machine the helper actually sees

Four distinct IOKit events feed one `MAFireWireLoader`:

| Event | Callback | Fires on | Role |
|---|---|---|---|
| `com_m_audio_FWMetaNub` matched | `deviceMatched:` | 0x10070 or 0x10071 kext nub appearance | triggers Halt/Upload/Start CFProperty dance (H2); also fires on wake/login re-walk |
| per-product driver class matched (8 classes) | `driverInstanceMatched:` | kext fully attached as runtime audio driver | lazily creates per-device `MAFireWireDeviceInstance`, adds to loader's device array |
| per-product driver class terminated | `driverInstanceTerminated:` | kext detaches (unplug / bus reset mid-attach / unload) | breaks retain cycles, removes device from array; no "resume" state |
| power / login state change | `systemWillSleep` / `systemWokeUp` / `userDidLogIn:` / `userDidLogOut:` | PM / SCDynamicStore | re-walks everything on wake and login |

**Observation:** these events are not strictly ordered — `deviceMatched:` can fire before or after `driverInstanceMatched:` depending on whether the kext matches the full product class before or after the catch-all nub. In practice: both fire for the same device during a single bus-reset storm. The helper doesn't try to correlate them explicitly — it keeps two independent sets (bootloader-mode CFProperty tasks in `deviceMatched:`, per-device audio-driver objects in `driverInstanceMatched:`).

## Bootloader → runtime handoff — the actual sequence

Starting from "FW1814 plugged in with no firmware yet uploaded":

1. Kext matches `0x10070` → publishes `com_m_audio_FWMetaNub` with `MAFirmwareInfo.firmware_running = 0`.
2. Helper's `deviceMatched:` fires. Reads `MAFirmwareInfo`, finds `firmware_running == 0`, takes the Halt-branch: writes `MAFirmwareHalt=YES` (H6 log:111-120).
3. **Device self-resets** — kext detaches, bus reset. `driverInstanceTerminated:` fires for any live product-driver (none, in bootloader case). Catch-all nub re-publishes.
4. Helper's `deviceMatched:` fires again. Reads `MAFirmwareInfo`, this time with the correct bootloader-mode marker. Falls through the Halt branch (no-op since already halted), proceeds to `firmwareFilesAtPath:forDevice:` + `validateFirmwareUpdate:` + `setProperty:@"MAFirmwareUpgradeImage"` (H2 log:94-99 → H6 log:97-107).
5. **If success**: `setProperty:@"MAFirmwareStart"=YES` (H6 log:106) + `[self quitView]` (H6 log:108).
6. **Device self-resets again** — now in runtime mode. Kext matches `0x10071`, publishes a new `FWMetaNub`, publishes the per-product driver class (e.g. `com_m_audio_fw1814`).
7. Helper's `deviceMatched:` fires a third time. `firmware_running != 0`, date matches, takes the "firmware is current" branch (H6 log:76-77). No action.
8. Helper's `driverInstanceMatched:` fires for the per-product class. Creates `MAFireWireDeviceInstance`, adds to array. `loadPrefsForUser:` is called from this path (or from `userDidLogIn:`, see below) to apply saved settings.

**No explicit "wait for runtime" step.** The helper is strictly event-driven; all handoff states are IOKit-notified. **This is the invariant our clone must preserve.**

## Sleep / wake / login — resolves H6a and H7a

### `-systemWillSleep` @ `0x36ca` (log:44-51)

```c
sSystemSleeping = 1;
```

That's the entire body. The flag is checked at the top of `deviceMatched:` (H6 log:23-25) — while sleeping, device-matched callbacks are swallowed. Avoids firing firmware uploads during a hibernate dance where the FireWire bus state is unpredictable.

### `-systemWokeUp` @ `0x36d6` (log:55-89)

```c
[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0]];  // 1-second settle
sSystemSleeping = 0;
```

- `0x3ff00000` is the high dword of IEEE 754 double `1.0` (so `[NSDate dateWithTimeIntervalSinceNow:1.0]`).
- The 1-second settle is **before** clearing `sSystemSleeping` — gives the FireWire bus a second to re-enumerate after wake.

Then:
- Re-walks every existing `com_m_audio_FWMetaNub` and calls `deviceMatched:` on each — synthetic re-match (log:71-76).
- `[self.devices removeAllObjects]` — nukes the per-device array (log:77).
- Re-walks every existing per-product driver class and calls `driverInstanceMatched:` on each (log:78-87) — rebuilds the per-device array from scratch.

**This is why the daemon doesn't miss devices plugged in during sleep.** On wake, it walks the entire ioreg state as if everything just appeared.

### `-userDidLogIn:withUID:withGID:` @ `0x37f8` (log:93-103)

```c
[self systemWokeUp];                                      // re-walk everything
[self.devices makeObjectsPerformSelector:@selector(loadPrefsForUser:)
                              withObject:gid];
```

**Confirms the H6/H7 "deviceMatched short-circuits until login" story.** The daemon runs from boot time but every `deviceMatched:` returns early via the `isUserLoggedIn` guard until the first login. At login:
1. `systemWokeUp` re-walks every ioreg entry → `deviceMatched:` now sees `isUserLoggedIn == YES` → the full firmware-update dance runs for every connected device.
2. `loadPrefsForUser:` applies the newly-logged-in user's per-device preferences.

Hilariously, the arg to `systemWokeUp` is the login-event. `systemWokeUp` doesn't inspect it, so the "sleep flag was already 0" case is benign — just a redundant re-walk, no state corruption. The `removeAllObjects` + rebuild is idempotent.

### `-userDidLogOut:withUID:withGID:` @ `0x383f` (log:107-116)

```c
[self.devices makeObjectsPerformSelector:@selector(commitSettings:)
                              withObject:gid];
```

Commits each device's prefs to the logging-out user's defaults. Does **not** tear down the device array. After logout, `deviceMatched:` callbacks keep arriving but short-circuit on `isUserLoggedIn == NO` — the device objects stay alive but go dormant.

**Not symmetric with login.** Login does *both* re-walk + loadPrefs; logout does *only* commitSettings. The daemon doesn't drop or re-fetch devices on logout.

## Cross-references

- **A1-Q7** — resolved in full. H2 answered question 1 (bootloader → runtime transition), H8 now answers question 2 (post-upload rediscover kick: *none*, device self-resets, IOKit re-matches naturally).
- **H2** (`H2-bootloader-detection.md`) — the `FWMetaNub` registration here is the same as H2 observed; H8 adds the companion 8-class driverInstance registration.
- **H6** (`H6-error-retry.md`) — `isUserLoggedIn` guard was H6's observation; H8 shows why it exists and how login re-triggers the walk.
- **H7** (`H7-launchd-lifecycle.md`) — resident-daemon rationale: must stay alive across boot → login window → first login to re-walk and apply prefs.
- **A1-Q11** (`A1-ghidra/Q11-userclient-selectors.md`) — the 8 per-product classes are user-client-candidate classes; whether any helper opens an `IOServiceOpen` on them is **unverified** here. Grepping `IOServiceOpen` call sites is queued as H8b. H6 class landscape lists `IOServiceOpen` as imported; its callers in the helper remain unidentified.

## Clone implications (for Part D)

1. **Our clone fwhelper watches by class name, not Model_ID.** The current M2 scaffold watches `0x10070` directly — this works for FW1814 only. Mirroring the reference means registering on our clone's nub class (whatever we name it) and letting the kext's own matching logic multiplex 0x10070/0x10071. For a single-product clone (FW1814-only, per project scope), direct Model_ID matching is **simpler and equivalent** — the only functional cost is losing the ability to add more products without re-wiring. **Clone decision: keep Model_ID matching** (we only ship FW1814).
2. **Mirror the sleep/wake re-walk.** Our Go-based fwhelper must subscribe to `IORegisterForSystemPower` and, on wake, re-walk the ioreg for the nub class and synthetically re-fire the device-matched handler. Without this, a device plugged in during sleep is invisible until the next unplug/replug.
3. **Mirror the login guard, but reconsider.** The guard exists because the status UI needs a user session. **Our clone has no mandatory status UI** (per H5 clone implication) — the firmware upload is a kext property write, which works at login window. **Drop the `isUserLoggedIn` guard** in our fwhelper. This is a *deliberate deviation* from the reference, enabled by dropping the status UI.
4. **Don't kick an explicit bus reset.** Reference doesn't; we don't either. Rely on the device's internal firmware → runtime self-reset.
5. **Post-upload success detection.** The reference assumes `setProperty:@"MAFirmwareStart"=YES` succeeds and moves on. If we want "upload confirmed by observing the runtime nub appear within N seconds" semantics, we add a `IOServiceAddMatchingNotification` on the runtime Model_ID with a timeout. **Nice-to-have, not reference-parity.**

## H8a resolved — the 8-entry per-product class name table

Raw read of helper's `__DATA.__const` at `0x97f4..0x9814` (file off 34804, 32 bytes = 8 × 4-byte pointers into `__TEXT.__cstring`):

```
0x97f4: 0x7709 → "com_m_audio_FW410Device"
0x97f8: 0x7721 → "com_m_audio_FWAudiophileDevice"
0x97fc: 0x7740 → "com_m_audio_FW1814Device"
0x9800: 0x7759 → "com_m_audio_FWSoloDevice"
0x9804: 0x7772 → "com_m_audio_FWOzonicDevice"
0x9808: 0x778d → "com_m_audio_FWNRV10Device"
0x980c: 0x77a7 → "com_m_audio_FWProjectMixDevice"
0x9810: 0x77c6 → "com_m_audio_FWLightbridgeDevice"
```

**Correction (2026-10-09)** to H8's hypothesis list. The actual 8 products are:

| # | Class name (verbatim from binary) | Product |
|---|---|---|
| 0 | `com_m_audio_FW410Device` | FireWire 410 |
| 1 | `com_m_audio_FWAudiophileDevice` | FireWire Audiophile |
| 2 | `com_m_audio_FW1814Device` | FireWire 1814 |
| 3 | `com_m_audio_FWSoloDevice` | FireWire Solo |
| 4 | `com_m_audio_FWOzonicDevice` | Ozonic |
| 5 | `com_m_audio_FWNRV10Device` | NRV10 |
| 6 | `com_m_audio_FWProjectMixDevice` | ProjectMix I/O |
| 7 | `com_m_audio_FWLightbridgeDevice` | FW Lightbridge |

The earlier hypothesis of `FW1200` / `FWMobilePre` / `FW1 ProFire` was **wrong** — those products either don't exist in the shipping v1.11.2 catalog or ship under different class names (FW1200 and MobilePre were USB, not FireWire; "ProFire" family postdates v1.11.2). The actual list contains `Ozonic`, `NRV10`, `ProjectMix` instead.

**Resolves H10a too.** H10's `FWLightbridge`/`FWNRV10` names are now confirmed as the literal class names in the helper binary — no FFADO cross-check is strictly required (though a passing nice-to-have under B1 would confirm the kext → user-visible product-name mapping is consistent across sources).

## H8b resolved — IOServiceOpen call sites in the helper

Two call sites to `_IOServiceOpen` in the helper (per raw `otool -tV`):

| Call site | Enclosing function | Purpose |
|---|---|---|
| `0x23fa` | `+[MAFireWireDeviceInstance deviceInstanceForService:]` (`0x23a0`, 418 bytes) | opens UC on the matched nub during device-instance construction |
| `0x405c` | `-[MAIOKitService openConnection]` (`0x4032`, 52 bytes) | generic open-UC wrapper (used by MAAsyncNotification path) |

**Only `IOConnectSetNotificationPort` is imported** on the IOConnect-family side. No `IOConnectCallMethod` / `IOConnectCallScalarMethod` / `IOConnectCallStructMethod` / `IOConnectMapMemory`. The `IOConnectSetNotificationPort` call site is `0x4ac4`, enclosed by `-[MAAsyncNotification initWithType:connection:runLoop:target:selector:]` (`0x4951`, 674 bytes).

**Conclusion:** the helper opens one user-client per matched nub purely to install an async notification port on it. The kext's property-change callbacks (e.g. `MAFirmwareInfo` arrival, upload-progress ticks) flow back through that port; no selectors are invoked. This fully confirms the "CFProperty-only control plane, UC used only as a notification sink" model established in H2.

**Clone implication (sharpened).** The clone's fwhelper can get away with the exact same pattern: open UC, install notification port, never call a selector. The clone's kext user-client class only needs:
- `clientClose` / `message` plumbing for the async-notification mach port
- *no* `externalMethod` dispatch table

This dramatically reduces the clone-kext surface for the fwhelper interaction. **Note:** the Panel's control-plane goes through `IORegistryEntrySetCFProperties` on the device registry entry (B5 §10) — also non-selector. So the entire reference `com_m_audio_FWUserClient` has no decoded `externalMethod` traffic at all between helper, Panel, and MIDIServer. Firmware upload, clock/mixer control, and panel notifications are all property-write + async-notification only.

## Follow-ups queued

- **H8a** — resolved 2026-10-09 above.
- **H8b** — resolved 2026-10-09 above.
