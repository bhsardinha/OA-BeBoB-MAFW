# H12 — The project's `firmware-helper` vs. what the reference kext requires

> **Correction (2026-10-15):** the new helper (`openaudio-fw1814/firmware-helper/main.c`, C, LaunchDaemon `com.openaudio.fw1814.helper`) exists and mirrors only the start branch of `MAFireWireLoader.deviceMatched:`; see the H2 correction for the flash cut. Sleep/login gating (H.3), settings replay (H.4) and packaging polish (H.6) are skeleton/in-progress tasks in the sheet.


> **Correction (2026-10-14):** the user chose D2; "D1" below means "the reference kext's protocol". The finding stands for the clone: it mirrors the reference firmware-state design, so the new helper writes `MAFirmwareStart/Halt/UpgradeImage` and reads `MAFirmwareInfo`, and the firmware protocol lives in the clone's kext (BUILD-PLAN M-FW).

**Status:** done 2026-10-14 (static read of `openaudio-fw1814/firmware-helper/src/*` + H2/H11 + Q-SDK.5). Evidence entry: `PROVENANCE-RESOLUTIONS.md` R32.

## Observations (cited)

1. **Our helper is a pre-pivot, userspace-only tool.** `main.cpp:1-10` header; matching dict at `main.cpp:420-437`: `IOServiceMatching("IOFireWireUnit")` + `Vendor_ID = 0x000D6C` + `Model_ID = 0x00010070` (**bootloader only**).
2. **It talks to the device directly over IOFireWireLib**, not through the kext: `IOCreatePlugInInterfaceForService(... kIOFireWireLibTypeID ...)`, `FireWireDeviceInterface_v9->Open/Read/Write` (`main.cpp:52-120`). Default action `doGo` writes the BeBoB bootloader "Go(application)" command (`main.cpp:182-205`); `--flash PATH` runs the full BCD download (`doFlash`, `main.cpp:210-330`).
3. **It never touches the kext nub:** a grep of `src/` and `include/` finds no `MAFirmwareStart`, `MAFirmwareHalt`, `MAFirmwareUpgradeImage`, `MAFirmwareInfo`, `IORegistryEntrySetCFProperty(ies)` or `IOServiceOpen`. It does nothing for a device that is already in runtime mode (`main.cpp:394`: "device reports app already running — nothing to do").
4. **Stale constant:** the comments say the runtime Model_ID is `0x00010051` (`main.cpp:5`, `main.cpp:199`). The canonical value is `0x10071` (memory `project_fw1814_ids`; the nub copies `Model_ID` as-is).
5. **The reference helper works the other way round** (A4 H2, `a4-h2-bootloader-detection.log`): it watches `com_m_audio_FWMetaNub` (both bootloader and runtime), reads the kext-published `MAFirmwareInfo` blob (80 B, `[0xc]` = state, `[0x20..0x2f]` = firmware date), and drives everything with three `setProperty` writes on the nub: `MAFirmwareHalt`, `MAFirmwareUpgradeImage` (the `.bcd` bytes), `MAFirmwareStart`. The physical bootloader protocol lives **in the kext** (`FWMetaNub::FirmwareUpdate*`, `FirmwareStart`'s `0xC8021000` boot cue). When no update is needed it still writes `MAFirmwareStart=YES` ("Starting <product>").
6. From Q-SDK.5 (R31): `MAFirmwareStart` is the **only** thing that sets `[+0xf3]` and makes the nub register a second time, which is what lets `FWAVCAudioDevice::probe` accept the product personality.

## Consequence for D1 (reasoning)

- With the reference kext in place, **our current helper would never produce a working driver**: its `Go` flips a bootloader device to runtime outside the kext, the device re-enumerates, the nub does its first (always-rejected) registration, and nothing ever sends `MAFirmwareStart` — so no `FW1814Device` is ever started. It would also compete with the kext for the device (`FWDevice::open` takes the device exclusively).
- **D1 therefore needs a helper that mirrors `MAFireWireLoader`**: match `com_m_audio_FWMetaNub`; on each match read `MAFirmwareInfo`; if no update is needed write `MAFirmwareStart=YES`; if the bundled `.bcd` is newer (or one-shot file present) follow the Halt → Upgrade → Start sequence. Settings re-apply (H11) belongs to the same helper.
- What survives from the current helper: the CF/IOKit plumbing, the LaunchDaemon plist, and `BCD.cpp` (header/CRC parse, A5) for the "is the bundled firmware newer" check. The direct bootloader protocol (`BootloaderProtocol.cpp`, `doFlash`) is **not needed** in D1 because the kext implements it; keep it only as an offline recovery tool (**decision for the user**).
- The status-UI hand-off (`sudo -u … launch`, H2 Finding 3) and the `MAFirmwareInfo` byte layout (Q19: only `[0xc]` and `[0x20..0x2f]` are known) are open design items for the helper plan.

## Not verified

- That the reference kext's `FirmwareUpdate` path (BeBoB251/BeBoB32) works on 10.13.6 — untested, same status as round two.
- Whether a runtime-mode device needs `MAFirmwareStart` on *every* attach or only the first after boot: the reference helper writes it on every `deviceMatched:` (H2 pseudocode), which is the safe mirror.
