# Editions: Classic (macOS 10.13) and Modern (macOS 11 to 15)

Written 2026-10-21 (R129). Packages **0.3.0** are the first to come in two editions (published 2026-10-22, GitHub release v0.3.0); the latest release is **0.3.1** (driver 0.2.2, panel 1.5: external-rate follow, R131). **0.4.0** (driver 0.3.0, panel 1.6, 2026-10-27, not published yet) adds the FireWire 410: tried on Classic only (iMac, capture 106); the 0.4.0 Modern package is built but not tried. Everything below says whether it was checked, and where.

## At a glance

| | **Classic** | **Modern** |
|---|---|---|
| macOS | 10.13 High Sierra to 10.15 Catalina | 11 Big Sur to 15 Sequoia¹ |
| CPU | Intel | **Intel only** (no Apple Silicon) |
| Tested on | iMac11,2 (FireWire 800, 10.13.6) and MacBookAir5,1 + Apple's Thunderbolt-to-FireWire adapter (10.13.6) | MacBookAir7,2 + the same kind of adapter, **macOS 14.8.9** |
| Interfaces tested | FireWire 1814 and FireWire Audiophile, alone and together (up to 0.2.2); **FireWire 410** alone (0.4.0, 2026-10-26/27, capture 106) | **FireWire Audiophile**; FireWire 1814 for the panel and the external-rate follow only (0.3.1, 2026-10-22, capture 99); FireWire 410 not tried |
| Kernel extension built with | Xcode 9.4.1, 10.13 SDK, on the 10.13.6 build machine | Command Line Tools, **macOS 15.2 SDK**, on the Sonoma machine |
| Security setting | SIP disabled | SIP set so unsigned kexts load (kext signing off), **plus approval in Privacy & Security** and a restart |
| Package | `OA-BeBoB-MAFW-<version>-classic.pkg` / `.dmg` | `OA-BeBoB-MAFW-<version>-modern.pkg` / `.dmg` |

¹ Does not work on macOS 26 Tahoe.

The audio driver, the helper and the panel are **the same source code** in both editions. What differs is how the driver is compiled, how macOS loads it, and what has been tried on each.

## What is different in the Modern edition

1. **The kernel extension is built against a newer SDK.** Since IOAudioFamily 540 (macOS 14), two reserved slots of `IOAudioEngine` (15 and 16) are real methods (`calculateSampleTimeoutHiRes`, `useHiResSampleInterval`). A driver built with the 10.13 SDK imports the old placeholder names, which macOS 14 does not define: 775 of its 777 imports resolve, 2 do not. Built with the 15.2 SDK, all 778 resolve. This is the only reason for a separate build. *Checked: `kmutil libraries` on macOS 14.8.9 (R125).*
2. **The source is identical.** The Makefile passes `-DOA_BAND_MODERN`, but no source file tests it. No 10.13 workaround was removed for Modern, and none was added. The minimum OS stamped in the kext is still 10.13 (the 15.2 SDK's own start/stop objects make the linker warn about it; harmless, and it is the configuration that ran). *Checked: `grep` of the sources, R128/R129.*
3. **Loading is a different procedure.** Classic copies the kext to `/Library/Extensions`, rebuilds the cache and asks for a restart. On macOS 11 and later the kext goes into the **auxiliary kernel collection**: the installer stages it with `kmutil`, macOS refuses to load it until you press **Allow** in System Settings › Privacy & Security, and then a restart is needed. The Privacy & Security entry reads *unidentified developer*, because the driver is unsigned and stays unsigned. *Checked on the Sonoma Mac (R126: approval, restart, load).*
4. **The installer checks the driver against the Mac before installing it.** The Modern post-install script compares every kernel symbol the driver imports with what `kmutil libraries` finds on that Mac; if one is missing it removes the driver and fails the install instead of leaving something that cannot load. Classic runs `kextutil -t -n` as its check. *Checked: the gate gives 0 missing for the packaged kext on 14.8.9 and 1 missing when a fake symbol is added (R128). The installation of the package itself on the Sonoma Mac is still to be done by the user.*
5. **Uninstall differs on macOS 11 and later.** The uninstaller also runs `kmutil rebuild` to drop the driver from the auxiliary collection. *Not verified:* the confirmation prompt it pipes "y" to, and whether macOS asks for an extra authorization (R128).
6. **The panel names its edition.** Same panel, same code; the About tab shows *Modern Edition · macOS 11 to 15 (Intel) Driver* instead of the High Sierra caption, and an *Edition* row. The panel is still compiled on the High Sierra machine (Swift 4.1, runtime bundled), so it contains no Apple Silicon code. *Checked: demo mode on 14.8.9 (R124), the panel connected to the Audiophile on 14.8.9 (R126).*
7. **Each edition's installer states its own requirements** (macOS range, Macs, interfaces, security) on the welcome page and on the disk image, saying what has been tested.

## What is not different

- The classic DCL isochronous programs, which the 2014 driver uses and which this project keeps, **work from the kernel on macOS 14.8.9 Intel**: the Audiophile plays through the adapter with regular transmit callbacks (46 ms apart, none late, no jump in the clip position; R126). Nothing had to be rewritten for the newer kernel.
- No workaround for kernel symbol names was needed on 14 (the lock names, the fortify helpers and the reporting methods all resolve there).
- The deliberate deviations from the official driver (`VerifyStreamFormats` and the others in `CLAUDE.md`) are shipped unchanged; whether each one is still needed on macOS 14 has not been examined.
- The 0.3.0 changes that came with the editions (knob law only on the phones, console fader scale restored on the outputs, new status menu order) are in both editions; they are not a Modern feature.

## What is known to be open on Modern

| Item | State |
|---|---|
| **A dropout during real use** | After 2 min 43 s of playing, the adapter's FireWire controller left the PCI bus before any driver event (no bus reset, no error, no panic); the interface was bus-powered. Power-cycling the interface brought it back. Cause **unknown**: bad contact, adapter or bus power, or something the driver did. It never happened on the same adapter under 10.13.6. Raw logs: `BROAD_COMPATIBILITY/evidence/modern-kext-dropout-macos-14.8.9-1941.txt`. |
| Recording (input data), MIDI on the wire, other sample rates, S/PDIF, sleep/wake, unload | not tried on Modern |
| FireWire 410 on Modern, and the 0.4.0 Modern package with any interface | not tried (2026-10-27) |
| FireWire 1814 on Modern | **partly tried** (2026-10-22, 0.3.1, capture 99): installs, loads, the panel works and the S/PDIF rate follow works with a POD X3; playback, recording, MIDI, other rates and modes not tried |
| The 0.3.0 knob law on the hardware | not tried |
| Panel controls and meters against the real driver on 14 | the panel connected and ran; the controls were not exercised one by one |
| Other macOS versions of the range (11, 12, 13, 15) | not tried; the package allows them because the symbols exist there (static reading only) |
| **Apple Silicon** | not supported. The kernel side exists (the FireWire and IOAudio families ship for arm64e) but the driver has no arm64e build, M-Audio's MIDI plug-in has no arm64 code, and FireWire through the adapter chain has never been tried on such a Mac |
| macOS 26 Tahoe | no FireWire stack; out of reach |

## Classic: unchanged

Classic is what 0.2.2 was: Intel, macOS 10.13.6, built on the High Sierra machine, tested with both interfaces. Its package refuses to install on macOS 11 and later, and the Modern package refuses 10.x (checked with `installer -pkg … -volinfo` on both machines). There is no third edition: a planned "Legacy" edition for Mac OS X 10.6.8 to 10.8.5 was dropped on 2026-10-22, because the official M-Audio drivers work well there. The project's floor is 10.9.

## Where to read more

[`BROAD_COMPATIBILITY_PLAN_v4.md`](compatibility/BROAD_COMPATIBILITY_PLAN_v4.md) (plan, evidence and effort for the whole 10.9 to 15 range), [`../openaudio-mafw-bebob/CHANGELOG.md`](../openaudio-mafw-bebob/CHANGELOG.md), and resolutions R124 to R129 in [`system-map/PROVENANCE-RESOLUTIONS.md`](system-map/PROVENANCE-RESOLUTIONS.md).
