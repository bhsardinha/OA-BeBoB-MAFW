# BROAD_COMPATIBILITY_PLAN v4: one OpenAudio BeBoB MAFW from OS X 10.9 Mavericks to macOS 15 Sequoia

**Status:** v4, 2026-10-22 (R130). v3 plus the user's scope decision: **the floor is now macOS 10.9; Mac OS X 10.6.8 to 10.8.5 (the Legacy edition) is dropped**, because the official M-Audio drivers already work well on those systems (user, 2026-10-22). Everything about Legacy in §1 to §12 is **out of scope**; §14 lists exactly what that removes. v1 to v3 are kept.

**Base:** package 0.2.2 (driver 0.2.0, panel 1.3), tree `openaudio-mafw-bebob/`.

**Tracker:** `BROAD_COMPATIBILITY_PLAN_v4.xlsx`, next to this file (task and decision statuses brought up to date; same IDs: E-n evidence, U-n unverified, DEC-n decisions, BC-n.m tasks, RK-n risks). It uses the same IDs: E-n evidence, U-n unverified, DEC-n decisions, BC-n.m tasks, RK-n risks.

**Raw evidence:** `evidence/`.

**Rules from CLAUDE.md that apply throughout:**
* Mirror the 2014 reference (v1.11.2). A deviation is allowed only when an OS mechanically forces it, and its comment must name what forced it.
* Every claim cites its source; anything unverified is labelled.
* No signing and no notarisation (user, 2026-10-17).
* No formal testing (R123). The on-device checks in §8 run only if the user asks for them.

**Effort unit:** one working **session**, as in `docs/BUILD_AUDIOPHILE_PLAN_v2.md` §8. Every figure is an estimate.

**Related:** `docs/MACOS12_PORT_PERSPECTIVE.md` (2026-10-19). That document assessed macOS 12+ only. This plan covers the whole range and updates two of its "not verified" points (E-5, E-10).

---

---

## 0. Where we are (2026-10-21, R129)

| Edition | OS | Status | Evidence |
|---|---|---|---|
| **Classic** | 10.13 (package allows 10.13 to <11; 10.14/10.15 allowed, untested) | **Shipped** as 0.2.2; 0.3.0 package built | the project's own tests on the iMac and the MacBook Air, R117-R123 |
| **Modern** | 11 to 15, **Intel only** | **Works on macOS 14.8.9** with the FW Audiophile: links 778/778, loads (kmutil, approval, restart), attaches, plays; 0.3.0 package built with an install gate | E-17, E-18, R125, R126, R128 |
| ~~Legacy~~ | ~~10.6.8 to 10.8.5~~ | **dropped (user, 2026-10-22): the official drivers work there** | none |
| Apple Silicon | 11 to 15 | not started; kernel families exist (E-5); no hardware | E-5, E-6 |
| Classic extension | 10.9 to 10.12 | **in scope, not done** (the installer starts at 10.13): BC-3.2, BC-4.2, BC-6.2 | none |

**What the Modern edition turned out to be.** The question of the plan (one source, how many builds?) is answered for Intel 10.13 to 14: **one source, two builds**. Modern differs from Classic only in the SDK it is compiled with (15.2, because IOAudioFamily 540 turned the reserved `IOAudioEngine` slots 15/16 into real methods), in how macOS loads it (auxiliary kernel collection: `kmutil`, Allow in Privacy & Security, restart) and in the installer around it. The `-DOA_BAND_MODERN` define exists but **no source file uses it**. The full list, with what is and is not verified, is `docs/EDITIONS.md`.

**What did not happen.** None of the work v2 expected for the Main band was needed on 14: no lock-name binding (BC-3.2), no `_FORTIFY_SOURCE` fallback, no vtable patching (BC-3.3 became "build with the 15.x SDK"), no Swift-panel changes (BC-4.2: the 0.2.2 panel runs on 14.8.9), no change to classic DCL (RK-4 answered on Intel, transmit side). The cost of the Modern edition was therefore far below the S1 estimate for its part.

**What the plan now says that v2 did not:**
* **Classic means 10.13 to 10.15 in the package, not 10.9 to 10.15.** The installer's minimum is 10.13 (the SDK and the only OS the project has run). 10.9 to 10.12 stays a possible extension (BC-3.2, BC-4.2, BC-6.2) and is not offered.
* **Modern is 11 to 15 on paper and 14.8.9 in fact.** 11, 12, 13 and 15 are allowed by the package because the symbols resolve there by static reading; nobody has loaded the driver on them.
* **Modern's user cost is higher than Classic's:** the Allow step and a second restart, and "unidentified developer" in Privacy & Security (signing is out of scope, R73).
* **A new open risk, RK-6 (adapter chain), became real:** one dropout, cause unknown (R126).
* **Order of work from here (all on request, R123):** (1) Modern on the other tested interface and more of the checks that were not done (FW1814, record, MIDI, rates); (2) the other macOS versions of 11 to 15 if a Mac is available; (3) publish 0.3.0 with the two editions; (4) the Classic extension to 10.9-10.12 if a Mac of that age is available; (5) Apple Silicon (S2) only with hardware and DEC-3. Legacy is dropped (§14).

**Effort, revised.** The Intel part of Modern needed none of BC-3.2, BC-3.3 (as a patch), BC-4.2 and the multi-slice merge of BC-2.4, so it cost less than the S1 estimate for it (no session count was kept). The estimates for the Classic extension to 10.9-10.12, Apple Silicon and Legacy are unchanged; they have no new evidence.

## 1. Short answer

| Question | Answer |
|---|---|
| One source tree, one installer package and one version number for 10.6.8 to 15? | **Yes, feasible.** No checked fact forbids it. |
| One binary per component for the whole range? | **No.** Three checked facts force separate builds; see below. |
| Minimum number of builds inside that one package | **2 kernel-extension builds.** **Legacy** (i386 + x86_64, old SDK) runs on 10.6.8 to 10.8.5. **Main** is a single Mach-O holding x86_64 for 10.9 to 15 on Intel and arm64e for 11 to 15 on Apple Silicon. It drops to **1** if probe P-1 shows that the Legacy x86_64 slice also runs on 10.9 to 15. The **panel needs 1 or 2 builds** (DEC-4). The **helper needs 1** (universal). The **MIDI plug-in needs 1 or 2**: M-Audio's original on Intel, plus a clone for Apple Silicon. |
| If you want separate downloads instead | **3 editions:** Legacy (10.6.8 to 10.8.5), Classic (10.9 to 10.15) and Modern (11 to 15, Intel and Apple Silicon). The differences are in §5.3. |
| What older systems lose | **No audio, mixer, digital-mode or MIDI function is expected to be lost.** The reference v1.9 delivers the full feature set on 10.6.8 (the SL rig is the ground truth). What 10.6 to 10.8 lose is panel cosmetics and system integration (§6). They lose the **whole panel** only if DEC-4 rejects a port of the panel. |
| Effort (estimates, §11) | **S1**, Intel 10.9 to 15: **13 to 18.5** sessions. **S2**, adding Apple Silicon 11 to 15: **+9 to 17.5**. **S3**, adding 10.6.8 to 10.8.5: **+11.5 to 21.5**. The whole range comes to **33.5 to 57.5**. |
| Recommendation | Stage the work: **S1 first**, then **S2** if an Apple Silicon Mac and the adapters become available, then **S3 only if wanted**. The official M-Audio drivers already run on 10.6.8 (v1.9 on the SL rig) and, according to B3, up to 10.11, so S3 mainly adds our panel to systems that already have a working driver. Most of S3's cost is the panel port (BC-4.4) and an old toolchain (BC-2.2). |

The three facts that force separate builds:

1. **Kernel symbol names changed.**
   * Any SDK of 10.10 or later renames `IOLockSleep`/`IOLockWakeup` to `_IOLockSleep_darwin14`/`_IOLockWakeup_darwin14` on x86_64. This happens unconditionally (E-2).
   * The IOService vtable of newer SDKs references `IOService::configureReport`/`updateReport`.
   * The shipped 0.2.2 kext imports all four (E-1). A kernel that predates those names cannot link it.
   * Consequence: the 10.6 to 10.8 kext must be built against an old SDK. A Mach-O fat file holds only one x86_64 slice.
2. **Apple Silicon.** An arm64e kext needs a macOS 11 or later SDK, and it must implement the ARM-only pure virtual `IOAudioEngine::driverDesiresHiResSampleIntervals()` (E-6).
3. **Swift's floor is 10.9.** The panel is 5,925 lines of Swift, and the Uninstaller is Swift too (E-11).

---

## 2. Facts checked today (2026-10-21)

All of these come from static reads. Nothing was loaded or installed on any machine. Raw outputs are in `evidence/`.

| ID | Fact | How it was checked |
|---|---|---|
| E-1 | The 0.2.2 kext imports **777** symbols. **28** of them are imported by no reference build (v1.9 SL x86_64, v1.10.5 x86_64, v1.11.3), including `_IOLockSleep_darwin14`, `_IOLockWakeup_darwin14`, `IOService::configureReport`/`updateReport`, `___memmove_chk` and `___strlcat_chk` | `nm -u` on the payload of `dist/OA-BeBoB-MAFW-0.2.2.pkg` and on the three reference binaries; `evidence/kext-0.2.2-imports-not-in-reference-builds.txt` |
| E-2 | On x86_64, a modern SDK aliases `IOLockSleep`, `IOLockSleepDeadline` and `IOLockWakeup` to `_darwin14` names whatever the deployment target | `MacOSX13.3.sdk` Kernel.framework `sys/cdefs.h:514-519` (`__DARWIN14_ALIAS`), `IOKit/IOLocks.h:129,132,134` |
| E-3 | 10.13.6 exports **both** `_IOLockSleep` and `_IOLockSleep_darwin14` (and the same for Wakeup and SleepDeadline). It also exports `___memmove_chk`, `___strlcat_chk`, `_version_major`, `_kernel_thread_start`, `_thread_call_allocate` and `_IOCreateThread` | read-only `nm` of `System.kext/PlugIns/{IOKit,Libkern,Mach}` on the build PC; `evidence/buildpc-10.13.6-kpi-exports.txt` |
| E-4 | On **macOS 14.8.4 Intel**, all but 2 of the 777 imports are defined in the kernel and the families. The two missing are `IOAudioEngine::_RESERVEDIOAudioEngine15` and `16`; in the 15.2 SDK those slots hold `calculateSampleTimeoutHiRes` and `useHiResSampleInterval`. "Defined" is necessary but not sufficient (exports are what count) | KDK 14.8.4 (23J319) kernel and dSYMs on the dev Mac; `IOAudioEngine.h:389,403,431-432`; `evidence/kext-0.2.2-vs-macos-14.8.4-x86_64.txt` |
| E-5 | **The FireWire and IOAudio kernel families ship on Apple Silicon** (macOS 14.8.4). The dSYMs of IOFireWireFamily, IOFireWireAVC, IOAudioFamily and IOFireWireSBP2 are x86_64 + arm64e. The arm64e kernel-cache manifests (t8103, t6000, t8112, t6020, t8122, t6031, t6030) list `IOFireWireFamily`, `IOFireWireAVC`, `IOAudioFamily` and `AppleFWOHCI`. The arm64e IOFireWireFamily defines the classic-DCL entry point `IOFireWireController::createLocalIsochPort(bool, DCLCommandStruct*, …)`. (Apple's published IOFireWireFamily-492 project sets `VALID_ARCHS = x86_64`; it is not what Apple ships.) | KDK 14.8.4; `_refs/IOFireWireFamily` `project.pbxproj:1280,1330,1376`; `evidence/kdk-14.8.4-families-arm64e.txt` |
| E-6 | On `TARGET_OS_OSX && TARGET_CPU_ARM64`, `IOAudioEngine` adds the pure virtual `driverDesiresHiResSampleIntervals()`. The header says "a driver subclass which runs on ARM must implement this method" | `MacOSX15.2.sdk` `IOKit/audio/IOAudioEngine.h:928-943` |
| E-7 | macOS 14.8.9 (dev Mac): `IOFireWireLib.plugin`, `IOFireWireAVCLib.plugin` and `MIDIServer` are x86_64 + arm64e. **`AppleFWAudio.kext` is absent.** IOAudioFamily is 540.3, IOFireWireFamily 4.8.3, IOFireWireAVC 4.2.8 | `lipo -archs`, `Info.plist` |
| E-8 | 10.13.6 (build PC): IOAudioFamily 206.5, IOFireWireFamily 4.7.2, IOFireWireAVC 4.2.6; **`AppleFWAudio.kext` is present** | `evidence/buildpc-10.13.6-kpi-exports.txt` |
| E-9 | Every 0.2.2 binary is x86_64 only. The helper and panel carry `LC_VERSION_MIN_MACOSX` 10.13 / SDK 10.13. The kext is built with `-mmacosx-version-min=10.13` (`kext/Makefile`). The installer says `allowed-os-versions min="10.13"` and `hostArchitectures="x86_64"` (`installer/distribution.xml.in:6-7`) | `lipo`, `otool -l` |
| E-10 | **The 0.2.2 panel (Swift 4.1, bundled runtime) starts and renders all 8 demo snapshots on Intel macOS 14.8.9.** Demo mode only: no driver and no IOKit path was exercised | `evidence/panel-0.2.2-demo-macos-14.8.9.txt` |
| E-11 | The panel uses five APIs newer than 10.9:<br>• `NSTextField(labelWithString:)` 10.12 (`Widgets.swift:11`)<br>• `monospacedDigitSystemFont` 10.11 (`Theme.swift:62`)<br>• `NSAppearance(named: .vibrantDark)` 10.10 (`Theme.swift:173`)<br>• `NSStatusItem.button` 10.10 (`StatusMenu.swift:105,114`)<br>• `.labelColor` 10.10 (`StatusMenu.swift:35`)<br>It also uses `AudioObjectAddPropertyListenerBlock`, available from 10.7. The Uninstaller uses `NSButton(checkboxWithTitle:)` (10.12), `labelWithString` and `.secondaryLabelColor` (10.10). Swift itself deploys to 10.9 at the earliest | `API_AVAILABLE` annotations in the `MacOSX15.2.sdk` AppKit and CoreAudio headers (`NSTextField.h:78`, `NSButton.h:58`, `NSFont.h:62`, `NSAppearance.h:70`, `NSStatusItem.h:51`, `NSColor.h:190-191`, `AudioHardware.h:397`) |
| E-12 | Reference builds:<br>• The v1.11.3 kext is x86_64 only, with both `OSBundleLibraries` and `OSBundleLibraries_x86_64`.<br>• The v1.10.5 kext is x86_64 + i386 + ppc (`_vendor/README.md`).<br>• v1.9 on the SL rig is i386 + x86_64 (`_refs/sl-v1.9`).<br>• **The 1.11.2 MIDI plug-in is x86_64 + i386 with `LC_VERSION_MIN` 10.7 / SDK 10.7.** | `lipo`, `otool -l`, `PlistBuddy` |
| E-13 | The dev Mac has Command Line Tools with SDKs 13 to 15.2 (no Xcode). `libkmod.a` and `libkmodc++.a` in the 15.2 SDK are x86_64, x86_64h and arm64e. The CLT's `libclang_rt.cc_kext.a` covers i386, x86_64, arm64 and arm64e. So arm64e kexts can be built with the CLT; modern SDKs have no i386 `libkmod` | `lipo -archs` |
| E-14 | The helper uses only IOKit, CoreFoundation and SystemConfiguration APIs of the 10.0 to 10.4 era: IONotificationPort, IORegisterForSystemPower, SCDynamicStore console user and CFPreferences | grep of `firmware-helper/*.c` (457 lines) |
| E-15 | The release disk image is UDZO + HFS+, which opens on 10.6 | `installer/dmg-settings.py:7-8` |
| E-16 | Code size today: panel 5,925 lines of Swift (+ the Uninstaller, about 120 lines); helper 457 lines of C | `wc -l` |

| E-17 | **(2026-10-21, R125)** On the live macOS 14.8.9 Intel kernel collections, the shipped 0.2.2 kext resolves 775 of 777 imports (missing: `_RESERVEDIOAudioEngine15/16`); the same sources built with `MacOSX15.2.sdk` (CLT) compile and resolve 778 of 778. The `_darwin14`, `*_chk` and IOReporting symbols resolve on 14 | `kmutil libraries -p <kext> -a x86_64 -z`; `evidence/kext-0.2.2-kmutil-libraries-macos-14.8.9.txt` |
| E-18 | **(2026-10-21, R126)** The `BAND=modern` kext (UUID 20D9014D…) loads on macOS 14.8.9 Intel (kmutil, approval, restart), attaches the FW Audiophile through the Thunderbolt-to-FireWire adapter (AV/C trace all `r=0x0`) and plays: the user heard it; TX callbacks 46/46 ms, no late gap, no CLIP jump. One dropout after 2 min 43 s of real use: the adapter's FireWire controller left PCI first; cause open (the user suspects a bad contact; bus-powered) | dev Mac run; `evidence/modern-kext-first-run-audiophile-macos-14.8.9.txt`, `evidence/modern-kext-dropout-macos-14.8.9-1941.txt` |

## 3. Not verified (each line says what would settle it)

| ID | Claim | Settle by |
|---|---|---|
| U-1 | On 10.6 most Macs boot the 32-bit kernel (K32) by default. On 10.7 many still do. Macs with 32-bit EFI cannot boot K64 at all | `uname -v` (RELEASE_I386 / RELEASE_X86_64) on each target; Apple's 10.6/10.7 kernel notes (BC-1.2) |
| U-2 | Loading policy for unsigned kexts by OS:<br>• 10.6 to 10.8: no check.<br>• 10.9: a warning, and `/Library/Extensions` requires a signature.<br>• 10.10: the `kext-dev-mode=1` boot-arg is needed.<br>• 10.11 to 10.15: SIP off, or `csrutil enable --without kext`.<br>• 11 to 15 Intel: SIP off, then the auxiliary kernel collection (`kmutil`), approval and a restart.<br>• Apple Silicon: Reduced and then Permissive Security, user-managed kexts, approval and a restart. | Apple documentation and `man kmutil`/`kextutil` on each OS (BC-1.2). **The 10.13.6 row is verified by the project itself (0.2.2 loads with SIP off).** |
| U-3 | `/Library/Extensions` is scanned only from 10.9; earlier systems load from `/System/Library/Extensions` | BC-1.2 |
| U-4 | IOReporting (`configureReport`/`updateReport`) exists from 10.9. This sets the floor of the Main build | 10.9 KPI symbol set or KDK (BC-1.1) |
| U-5 | Whether `___memmove_chk`/`___strlcat_chk` are exported on 10.9 to 10.12 (they are on 10.13.6, E-3) | BC-1.1; or build with `-D_FORTIFY_SOURCE=0` (the `string.h` path exists) |
| U-6 | macOS 26 Tahoe has no FireWire stack, so 15 is the ceiling | macfw's statement (`docs/MACOS12_PORT_PERSPECTIVE.md` §1-2); a Tahoe KDK |
| U-7 | Whether the 1.11.2 MIDI plug-in (min 10.7, E-12) runs on 10.6.8 | Its imports against the 10.6.8 CoreMIDI/CoreFoundation exports (BC-4.7); the SL rig runs v1.9's own plug-in |
| U-8 | Whether the x86_64 panel works against the real kext on 11 to 15 (user client, shared meter page, CoreAudio rate changes). Only demo mode was checked (E-10) | BC-6.1 |
| U-9 | Whether CoreMIDI on Apple Silicon can host an x86_64-only driver plug-in (the arm64e MIDIServer cannot load it in-process) | One run on an Apple Silicon Mac, or Apple documentation |
| U-10 | Whether FireWire works on Apple Silicon through Apple's Thunderbolt 3 → Thunderbolt 2 and Thunderbolt → FireWire 800 adapters. E-5 proves only that the software is there | BC-6.4 |
| U-11 | Whether the kernel linker (kxld on 10.x, `kmutil` on 11+) patches the 0.2.2 vtable entries `_RESERVEDIOAudioEngine15/16` (E-4) | P-1 = BC-6.1 (`kextutil -n -t`, then a load on one Intel 11+ Mac) |
| U-12 | Whether the Legacy x86_64 slice (old SDK) loads and runs on 10.9 to 15. If yes, a single kext bundle covers every band | P-1 extension (BC-6.1 with the Legacy build) |
| U-13 | How the 10.6 HAL treats our 10.13-driven deviations, such as `VerifyStreamFormats` and rate changes from the panel through CoreAudio. Known from the SL rig: the 10.6 HAL applies a rate request only while the client's run loop spins | BC-6.3 |
| U-14 | Whether the classic DCL ring, `TPacketPool` (4 KiB packing, 32-bit physical mask) and the 4 KiB meter page behave on Apple Silicon (16 KiB pages, DART IOMMU, 24 MHz timebase, pointer authentication) | BC-1.4 (static), BC-6.4 |

## 4. OS bands

The kext reads `version_major`, which is exported (E-3), so behaviour can branch per OS at run time. The Darwin to OS mapping is: 10 = 10.6, 11 = 10.7, 12 = 10.8, 13 = 10.9, 14 = 10.10, 15 = 10.11, 16 = 10.12, 17 = 10.13, 18 = 10.14, 19 = 10.15, 20 = 11, 21 = 12, 22 = 13, 23 = 14, 24 = 15.

| Band | OS | Kernel | Kext build | Kext goes to | Prerequisite on the user's Mac (U-2) | Cache / load tool | Panel | MIDI plug-in | Hosts the project has |
|---|---|---|---|---|---|---|---|---|---|
| **L** Legacy | 10.6.8 to 10.8.5 | i386 (K32) or x86_64 on 10.6/10.7; x86_64 on 10.8 | Legacy: i386 + x86_64, 10.6 SDK | `/System/Library/Extensions` (U-3) | none | `touch /System/Library/Extensions` + `kextcache` | Obj-C port (DEC-4) or none | original 1.11.2 (if U-7 holds) or the clone | SL rig (10.6.8, but it must keep its v1.9 state, so a separate volume is needed); iMac11,2 (Apple's support range 10.6.4 to 10.13.6, recalled); MacBookAir5,1 (10.7.4 to 10.15, recalled) |
| **M** Main | 10.9 to 10.15 | x86_64 | Main x86_64, 10.13 SDK (as today) + BC-3.2 | `/Library/Extensions` | 10.9 unverified (U-2); 10.10 `kext-dev-mode=1`; 10.11 to 10.15 SIP off (or `--without kext`) | `kextcache -system-caches` / `-i /` | Swift (as today) | original | build PC and Air on 10.13.6 (today); the Air can run 10.14/10.15 |
| **M11** Main on 11+ Intel | 11 to 15 | x86_64 | same Main x86_64 slice (U-11) | `/Library/Extensions` → auxiliary kernel collection | SIP off; approval + restart | `kmutil` (`kextutil -n -t` still exists for the gate) | Swift (E-10) | original (U-8) | dev Mac Air7,2 (14.8.9; Apple's support for this model ends at 12, recalled, so 14.8.9 is a non-standard install) |
| **AS** Apple Silicon | 11 to 15 | arm64e | arm64e slice of the Main file, macOS 15 SDK | `/Library/Extensions` → auxiliary kernel collection | recoveryOS: Permissive Security + user-managed kexts; approval + restart | `kmutil` | arm64 slice (Swift 5 toolchain) | **clone** (U-9) | none in the inventory; also needs the two Apple adapters (U-10) |

The ceiling is 15 (U-6). FireWire hardware: the iMac11,2 and Mac Pro have native FireWire 800. The Airs need Apple's Thunderbolt to FireWire adapter; the Air already runs the 1814 that way on 10.13.6.

## 5. How many versions: the options

### 5.1 Option A (not chosen, DEC-5 = b): one package, two kext builds

```
OA-BeBoB-MAFW-<version>.pkg   (one version number, one download)
├─ kext "Legacy"  OpenAudioBeBoBMAFW.kext  i386 + x86_64 (10.6 SDK)    → /System/Library/Extensions   chosen on 10.6.8–10.8.5
├─ kext "Main"    OpenAudioBeBoBMAFW.kext  x86_64 (10.13 SDK) + arm64e (15 SDK) in one Mach-O → /Library/Extensions   chosen on 10.9–15
├─ helper         universal: i386 + x86_64 (min 10.6) + arm64 (min 11)
├─ panel          one universal Obj-C binary (DEC-4 a)  — or Swift x86_64 + arm64 for 10.9–15 (DEC-4 b/c)
├─ MIDI           M-Audio original (Intel, unmodified) + OpenAudio clone (Apple Silicon; DEC-8)
└─ scripts        OS-conditional choices in distribution.xml; postinstall branches per band (§7 BC-5)
```

* The installer picks the bundle and its location from `system.version.ProductVersion`, using the Installer JavaScript in `distribution.xml`. Both kext builds keep the same bundle id and name, so the uninstall lists stay simple.
* Why two builds: E-1 and E-2. The Main build cannot link on kernels without the `_darwin14` names or IOReporting. The Legacy build cannot share the Main file's single x86_64 slice.

### 5.2 Option A′: one package, one kext bundle

Possible only if U-12 holds, that is, if an old-SDK x86_64 slice loads and runs on 10.9 to 15. The single bundle would then hold Legacy i386 + x86_64 plus Main's arm64e.

* **Gain:** one kext file.
* **Cost:** the 10.9 to 15 Intel kernels run a kext compiled against 10.6 headers. Every IOAudio/IOFireWire behaviour change since then would have to be absorbed by the kernel linker's vtable patching (U-11).
* **Verdict:** keep it as an outcome of P-1, not as the plan.

### 5.3 Option B (CHOSEN, DEC-5): three editions (three downloads)

| | **Legacy edition** | **Classic edition** | **Modern edition** |
|---|---|---|---|
| OS | 10.6.8 to 10.8.5 | 10.9 to 10.15 | 11 to 15 |
| CPUs | Intel (K32 + K64; i386 user space only if 32-bit CPUs are in scope, DEC-2) | Intel | Intel + Apple Silicon |
| Kext | i386 + x86_64, 10.6 SDK, `/System/Library/Extensions` | x86_64, 10.13 SDK, `/Library/Extensions` | x86_64 + arm64e, `kmutil` flow |
| Panel | Obj-C (or none) | Swift 4.1 + bundled runtime (as today) | Swift 4.1 x86_64 + Swift 5 arm64, or Obj-C universal |
| MIDI | original (U-7) or clone | original | original (Intel) + clone (arm64) |
| User prerequisites | none | 10.10 boot-arg; 10.11+ SIP off | SIP off; Apple Silicon: Permissive Security |
| Toolchain | Xcode 9.4.1 + 10.6 SDK (or Xcode 4.x) | Xcode 9.4.1 (build PC) | CLT 15.2 or Xcode 12+ (arm64e/arm64) + Xcode 9.4.1 (x86_64) |

The code base is the same; only the build flags and the installer differ. The editions buy simpler installer logic and per-edition release notes, at the price of three artefacts per release. **Option A gives the user the same result with one download.**

### 5.4 Option C: no Legacy band (floor 10.9)

One package and one kext bundle (Main x86_64 + arm64e), with the Swift panel kept. This is S1 + S2 in §11. It is the cheapest broad version and covers every system the official M-Audio driver does not reach (10.12+, with B3's Sierra hack aside).

## 6. What each band loses compared with 10.13.6 today

The features are those of 0.2.2. Legend:
* "=": unchanged.
* "fallback": same function, older API.
* "lost": not available.
* "unverified": expected but unchecked.

| Feature | 10.6.8–10.8.5 (L) | 10.9–10.12 (M) | 10.14–10.15 (M) | 11–15 Intel (M11) | 11–15 Apple Silicon (AS) |
|---|---|---|---|---|---|
| Audio I/O, every rate 44.1–192 kHz, S/PDIF / ADAT / S/MUX modes | = (v1.9 does it on 10.6.8; ours unverified, U-13) | = unverified | = unverified | = unverified (U-11) | unverified (U-10, U-14) |
| 32-bit kernel (K32) Macs | needs the i386 slice (DEC-2); without it, only Macs booted K64 work | n/a | n/a | n/a | n/a |
| Mixer, routing, levels, front-panel encoders, settings replay by the helper | = | = | = | = | unverified |
| Meters (shared 4 KiB page) | = | = | = | = | needs a 16 KiB page review (U-14) |
| MIDI in/out | = only if the 1.11.2 plug-in runs on 10.6 (U-7); otherwise the clone | = | = | = (U-8) | **lost** unless the clone is written (DEC-8, U-9) |
| Control panel | **lost** if DEC-4 = c; = with the Obj-C port | = (fallbacks for 5 APIs, E-11) | = | = (E-10, demo) | = with an arm64 slice; under Rosetta 2 unverified |
| Panel: dark vibrant title bar | **lost** (10.10+); content stays dark (custom drawing) | lost on 10.9 only | = | = | = |
| Panel: tabular-digit system font | fallback (fixed-pitch font) | fallback on 10.9–10.10 | = | = | = |
| Panel: GPU-only meter updates (`wantsUpdateLayer`) | fallback on 10.6–10.7 (CPU drawing, more CPU) | = | = | = | = |
| Panel: Retina | lost on 10.6 (10.7.3+) | = | = | = | = |
| Panel: Big Meter full screen | lost on 10.6 (10.7+) | = | = | = | = |
| Status menu, launch at login | fallback (legacy `NSStatusItem` API; LaunchAgent works) | fallback on 10.9 (no `.button`) | = | = | = |
| CoreAudio rate/device listeners | fallback on 10.6 (C callbacks instead of blocks) | = | = | = | = |
| Firmware check window (read-only) | = (with the port) | = | = | = | = |
| Uninstall app | needs a port (Swift today) | fallbacks (E-11) | = | = | arm64 slice or Rosetta |
| Installer | kext in `/System/Library/Extensions`; no prerequisites | 10.10 boot-arg; 10.11+ SIP off | SIP off | SIP off + approval + restart | Permissive Security + approval + restart |

Newer systems gain nothing functional over 10.13.6. They differ only in install steps. **Apple Silicon is the only band with a possible functional loss (MIDI), and the only one where FireWire itself is unproven on hardware.**

## 7. Work plan

Each task has an ID and a band: C = common, M = Main, AS = Apple Silicon, L = Legacy. The effort range is in sessions. Acceptance criteria are in the workbook's `Tasks` tab. Nothing starts before the decisions in §10.

### BC-0 Decisions (user)

DEC-1 to DEC-10, listed in §10. These gate everything else.

### BC-1 Static evidence (no hardware, nothing loaded)

| ID | Band | Task | Sessions |
|---|---|---|---|
| BC-1.1 | C | **Symbol audit, one row per build and OS.** Compare the imports of each kext build with the KPI symbol sets and family exports of the oldest and newest OS of its band. Sources: the SL rig (10.6.8, read-only copy), the build PC (10.13.6; partly done, E-3), and KDKs for 10.9, 11, 15 and others (Apple developer downloads). *Partly done: E-1, E-3, E-4.* | 1–2 |
| BC-1.2 | C | Confirm U-1, U-2, U-3 and U-6 from Apple documentation, man pages and KDKs, and fill the `OS_Matrix` source column | 0.5–1 |
| BC-1.3 | L | i386 (ILP32) audit of the kext: pointer/`UInt32` casts, `mach_vm_*` sizes, user-client struct layouts for 32- and 64-bit tasks, format strings | 0.5–1 |
| BC-1.4 | AS | Apple Silicon audit of the kext:<br>• 16 KiB pages: `TPacketPool` packs 4096-byte pages (`TPacketPool.cpp:15-34`), the meter page is 4 KiB, `FWSharedSync` aligns to 0x1000.<br>• AbsoluteTime is not nanoseconds (`FWBaseEngine.cpp:179`, `FWDCLProgram.cpp:284-477`, `FWAVCAudioDevice.cpp:212`).<br>• Pointer authentication on DCL `CallProc` and `FWFunctor` function pointers.<br>• `IOCreateThread` (deprecated) → `kernel_thread_start`.<br>• `inTaskWithPhysicalMask` under the DART.<br>• `driverDesiresHiResSampleIntervals`. | 1–1.5 |
| BC-1.5 | C | Compile the kext headers per SDK (`-fsyntax-only`): 10.6 SDK (DEC-6), 10.13 SDK, and 15.2 SDK with `-arch arm64e`. List every error | 0.5–1 |
| BC-1.6 | M | Panel and Uninstaller API audit. **Done (E-11).** | 0 |
| BC-1.7 | C | MIDI plug-in audit. **Done (static, E-12).** What remains is U-7 (BC-4.7) and U-9 | 0 |

### BC-2 Build system: one source, band builds

| ID | Band | Task | Sessions |
|---|---|---|---|
| BC-2.1 | C | Band-aware build files:<br>• `kext/Makefile` gets `BAND=legacy\|main\|silicon`, which sets the SDK, archs, min version and `OA_BAND_*` defines.<br>• The helper Makefile builds universal.<br>• The panel Makefile builds per slice. | 1 |
| BC-2.2 | L | Legacy toolchain probe (P-2): Xcode 9.4.1 clang/ld with `MacOSX10.6.sdk` (its i386 `libkmod`) for an i386 + x86_64 kext on the build PC. Fallback: Xcode 4.6.3 on a 10.8 volume (C++11 support limited) | 1–3 |
| BC-2.3 | AS | Silicon toolchain: the dev Mac's CLT 15.2 can build arm64e kexts (E-13). Building there needs DEC-10; the alternative is Xcode on another 13+ Mac | 0.5–1 |
| BC-2.4 | C | Merge and guards:<br>• `lipo` per component.<br>• A per-slice guard (`LC_VERSION_MIN` / `LC_BUILD_VERSION` must match the band).<br>• A symbol-audit gate in the build: fail if a band's kext imports a name its oldest OS does not export.<br>• `check-self-contained.sh` extended to universal bundles. | 1–1.5 |

### BC-3 Kernel extension (driver 0.3.0)

| ID | Band | Task | Sessions |
|---|---|---|---|
| BC-3.1 | C | One OS gate in `base/` (`version_major`). Re-label each deviation forced by 10.13 (`VerifyStreamFormats`, R68 and the others in CLAUDE.md) with the OS range that forces it. Keep the behaviour generic where it is harmless | 0.5–1 |
| BC-3.2 | M | Bind `IOLockSleep`/`IOLockWakeup` to their plain exported names with an `__asm` label (10.13.6 exports both, E-3), or use `IOCommandGate::commandSleep`. Add `-D_FORTIFY_SOURCE=0` if U-5 fails | 0.5 |
| BC-3.3 | M | Reserved slots 15/16 (E-4, U-11): confirm that `kmutil` patches them on 11 to 15. If it does not, build the 11+ x86_64 slice with a new SDK. That would mean a third kext bundle (Main ≤10.15 and Main 11+) | 0.5–1 |
| BC-3.4 | L | Legacy build: fix what BC-1.5 and BC-1.3 found. Info.plist with KPI library names; per-arch keys only if needed (the reference has `OSBundleLibraries_x86_64`, E-12) | 2–4 |
| BC-3.5 | AS | Silicon build: implement `driverDesiresHiResSampleIntervals` (with `useHiResSampleInterval`/`calculateSampleTimeoutHiRes` as the header documents) and the BC-1.4 fixes. Each deviation's comment says "forced by the arm64 IOAudioEngine header" | 2–4 |
| BC-3.6 | C | Bundle layout per band: the same bundle id and personalities; the Main file holds x86_64 + arm64e | 0.5 |

### BC-4 User space

| ID | Band | Task | Sessions |
|---|---|---|---|
| BC-4.1 | C | Helper universal: i386 + x86_64 (Xcode 9.4.1, min 10.6) + arm64 (CLT, min 11), merged with `lipo`. No API changes (E-14) | 0.5 |
| BC-4.2 | M | Swift panel to 10.9: `#available` fallbacks for the five APIs (E-11), deployment target 10.9; Uninstaller likewise | 1–2 |
| BC-4.3 | AS | Panel and Uninstaller arm64 slice: the same sources built with a Swift 5 compiler (`-swift-version 4.2` mode if needed), `lipo` with the Swift 4.1 x86_64 slice. `bundle-swift-runtime.sh` made universal-aware (the bundled 4.1 runtime serves only the x86_64 slice) | 0.5–1 |
| BC-4.4 | L | Panel port to Objective-C (DEC-4 a): file by file (Theme, Widgets, MixerView, …), with the host tests ported. It then **replaces** the Swift panel everywhere: one universal binary, no bundled Swift runtime | 6–10 |
| BC-4.5 | L | Uninstaller for 10.6 to 10.8: Obj-C (with BC-4.4) or an AppleScript applet | 0.5 |
| BC-4.6 | AS | MIDI clone plug-in. It mirrors the M-Audio plug-in (A3 decompiles, user-client selectors 500–507), is universal, and keeps the original's behaviour (DEC-8) | 4–8 |
| BC-4.7 | L | MIDI on 10.6: check the 1.11.2 plug-in's imports against the 10.6.8 frameworks (U-7). Otherwise ship the clone on 10.6 | 0.5–1 |

### BC-5 Installer (one package)

| ID | Band | Task | Sessions |
|---|---|---|---|
| BC-5.1 | C | `distribution.xml`: `allowed-os-versions min="10.6.8"`, `hostArchitectures="i386,x86_64,arm64"`, and OS-conditional choices (Installer JavaScript) for the kext bundle and location and for the MIDI plug-in | 0.5–1 |
| BC-5.2 | C | `postinstall` per band:<br>• Cache rebuild: `touch /System/Library/Extensions` + `kextcache` on L; `kextcache -system-caches` on M; the `kmutil` flow with an approval and restart message on M11/AS.<br>• Keep the `kextutil -t -n` gate on every band.<br>• Restart `MIDIServer`. | 1–1.5 |
| BC-5.3 | C | `uninstall.sh`: add `/System/Library/Extensions/OpenAudioBeBoBMAFW.kext` and every new path. The rule that "every path ever installed stays in the list" is kept | 0.5 |
| BC-5.4 | C | Installer pages and README: per-band prerequisites (10.10 boot-arg, 10.11+ SIP, 11+ approval, Apple Silicon Permissive Security). These are documentation only, not signing | 0.5 |

### BC-6 On-device checks: only if the user asks (R123)

| ID | Band | Task | Sessions |
|---|---|---|---|
| BC-6.1 | M | **P-1:** the Main kext on one Intel 11 to 15 Mac. Covers `kextutil -n -t`, load, CoreAudio, play/record, panel, and U-8, U-11 and U-12 (the latter with the Legacy build too). The dev Mac is a candidate (DEC-7); a panic reboots it | 1 |
| BC-6.2 | M | The Main kext on one of 10.9 to 10.12 | 1 |
| BC-6.3 | L | The Legacy kext on 10.6.8, K32 and K64, on a volume that is not the SL rig's reference install | 1–2 |
| BC-6.4 | AS | Apple Silicon, once the hardware and adapters exist (U-10, U-14) | 1–2 |

### BC-7 Release

| ID | Band | Task | Sessions |
|---|---|---|---|
| BC-7.1 | C | README compatibility table: each band "verified on <machine>, <date>" or "untested". Also NOTICE, CHANGELOG, VERSION (package 0.3.0 proposed), `docs/ARCHITECTURE_MAFW_v2.md` (build bands), and `/close-item` | 1 |

## 8. On-device checks and safety

* **None of BC-6 is scheduled.** The no-formal-testing rule (R123) stands. A band that nobody has loaded ships labelled **untested** in the README.
* The postinstall `kextutil -t -n` gate stops link failures from turning into boot loops on every band. It cannot catch runtime panics.
* A kext panic at boot on 11 and later needs Safe Mode or recoveryOS to remove the kext. On Apple Silicon it needs recoveryOS. Say so before any such load (hardware etiquette).
* Never disturb the SL rig's v1.9 reference install. Use a separate volume for 10.6.8 work.

## 9. Risks

| ID | Risk | Bands | Mitigation |
|---|---|---|---|
| RK-1 | The 10.6 HAL and IOAudioFamily behave differently from 10.13 (U-13) | L | Decompile-and-diff first (standing method lesson); BC-6.3 |
| RK-2 | A 10.13-forced deviation misbehaves on another IOAudioFamily version (206.5 on 10.13.6 vs 540.3 on 14.8.9) | all | BC-3.1 gates each deviation by OS range |
| RK-3 | `kmutil` does not patch the reserved slots 15/16 | M11 | BC-3.3: a separate 11+ x86_64 build (a third bundle) |
| RK-4 | Classic DCL from the kernel fails on 11 to 15 (unverified since `MACOS12_PORT_PERSPECTIVE` §1), although the symbol exists on x86_64 and arm64e (E-4, E-5) | M11, AS | BC-6.1 decides it early |
| RK-5 | 16 KiB pages, DART and the timebase break the DMA ring or the meter page | AS | BC-1.4, BC-3.5 |
| RK-6 | FireWire through the adapter chain is unreliable, or bus power is not enough for the Audiophile | AS | BC-6.4 |
| RK-7 | A panic reboots the test host; an Apple Silicon panic needs recoveryOS | all | §8 |
| RK-8 | Every release needs three toolchains (Xcode 9.4.1, the 10.6 SDK, CLT/Xcode 12+) | all | BC-2 scripts; record toolchain versions in CHANGELOG |
| RK-9 | Permissive Security lowers Apple Silicon security for the whole Mac | AS | the user's choice, stated in the installer pages |
| RK-10 | The panel port (BC-4.4) introduces UI regressions | L | port file by file against the `--demo --snapshot` PNGs (panel quality rule) |
| RK-11 | The original MIDI plug-in fails on 10.6 (min 10.7) | L | BC-4.7 → the clone |
| RK-12 | Most bands ship untested (R123) | all | README marks each band; on-device checks only on request |

## 10. Decisions for the user

| ID | Question | Options | Recommendation |
|---|---|---|---|
| DEC-1 | Lowest OS | a) 10.6.8 (as asked; adds band L)<br>b) 10.9 (no old toolchain, Swift panel kept)<br>c) keep 10.13 and extend upward only | **b now, a later if wanted.** Band L costs 11.5 to 21.5 sessions, and the official drivers already run there |
| DEC-2 | The 32-bit kernel (i386 slice) on 10.6/10.7 | yes / no | yes, if DEC-1 = a. Without it, 10.6 Macs must boot K64 (U-1) |
| DEC-3 | Apple Silicon in scope | yes (needs an Apple Silicon Mac, the TB3→TB2 and TB→FW800 adapters, Permissive Security and the MIDI clone) / no | yes as stage S2 when the hardware exists; the kernel families are there (E-5) |
| DEC-4 | Panel for 10.6 to 10.8 | a) Obj-C port: one panel everywhere<br>b) Swift 10.9+ plus a separate Obj-C legacy panel<br>c) no panel on 10.6 to 10.8 | a if DEC-1 = a (b doubles the UI code; c loses the whole UI) |
| DEC-5 | Packaging | a) one package with OS-conditional choices<br>b) three editions | **DECIDED 2026-10-21 (user): b, three editions.** The recommendation of v1 (a) is superseded |
| DEC-6 | Source of the 10.6 SDK (band L) | from Apple's developer downloads (Xcode 3.2.6 / 4.x, the user's Apple ID) | user action |
| DEC-7 | Test hosts per band, and whether any BC-6 check happens | candidates in §4 | the user's call; README marks untested bands |
| DEC-8 | MIDI on Apple Silicon | a) clone plug-in<br>b) no MIDI there<br>And: should the clone also replace the original on Intel? | a, Apple Silicon only (keeps Intel unchanged) |
| DEC-9 | Where the work lives, and versions | a branch of `openaudio-mafw-bebob` (band builds in the same tree) / a new folder; package 0.3.0, driver 0.3.0 | the branch; 0.3.0 |
| DEC-10 | Building on the dev Mac | allow CLT builds of the arm64e/arm64 slices there (CLAUDE.md keeps it edit-only) / use another Mac | the user's call |

## 11. Effort by scenario (sessions; the same sums are formulas in the workbook)

| Scenario | Bands | Tasks | Min | Max |
|---|---|---|---|---|
| Common work (in every scenario) | C | BC-1.1, 1.2, 1.5, 2.1, 2.4, 3.1, 3.6, 4.1, 5.1–5.4, 7.1 | 9 | 13 |
| **S1** Intel 10.9 to 15 | C + M | + BC-3.2, 3.3, 4.2, 6.1, 6.2 (BC-6 only on request: 2) | **13** | **18.5** |
| **S2** = S1 + Apple Silicon 11 to 15 | + AS | + BC-1.4, 2.3, 3.5, 4.3, 4.6, 6.4 | **22** | **36** |
| **S3** = S2 + 10.6.8 to 10.8.5 (the full request) | + L | + BC-1.3, 2.2, 3.4, 4.4, 4.5, 4.7, 6.3 | **33.5** | **57.5** |
| S1 + L (all Intel, no Apple Silicon) | C + M + L | | 24.5 | 40 |

For comparison, `MACOS12_PORT_PERSPECTIVE.md` estimated route R1 (Intel 12 to 15 only) at 5 to 11 sessions. S1 adds 10.9 to 10.12, the band-aware build and installer, and the symbol-audit gate.

## 11a. Editions: names, builds, where they live (v2, decided)

| Edition | OS | `make BAND=` | Build output | Package | Status |
|---|---|---|---|---|---|
| **Classic** | 10.9 to 10.15 | `classic` (default; today's 0.2.2 build, 10.13 SDK, build PC) | `openaudio-mafw-bebob/build/` (unchanged, so existing scripts keep working) | `dist/OA-BeBoB-MAFW-<ver>-classic.pkg` | exists (0.2.2 is this edition); 10.9 to 10.12 and 10.14/10.15 unverified |
| **Modern** | 11 to 15, Intel first, Apple Silicon later | `modern` (15.x SDK) | `openaudio-mafw-bebob/build/modern/` | `dist/OA-BeBoB-MAFW-<ver>-modern.pkg` | **in progress on the dev Mac (14.8.9)**; links 778/778 (E-17); loads and plays with the FW Audiophile (E-18, R126; one dropout, cause open); panel names the edition (R127) |
| **Legacy** | 10.6.8 to 10.8.5 | `legacy` (10.6 SDK, i386 + x86_64) | `openaudio-mafw-bebob/build/legacy/` | `dist/OA-BeBoB-MAFW-<ver>-legacy.pkg` | not started (S3: BC-1.3, 2.2, 3.4, 4.4, 4.5, 4.7) |

* One source tree and one version number per release; each edition has its own installer, `distribution.xml` (`allowed-os-versions` per edition), `postinstall`, README section and release notes. The OS-conditional installer JavaScript of BC-5.1 is not needed; BC-5.2 is split per edition.
* Order: Modern (this Mac, S1 Intel part) -> Classic re-verified with the shared code (build PC) -> Modern Apple Silicon (S2) -> Legacy (S3). Every shared-code change must keep the Classic build byte-for-byte behaviour on 10.13.6 (the build PC stays the Classic reference).
* Effort is unchanged (§11); the editions add three release artefacts instead of one.

## 12. Where things live

* **This plan:** `BROAD_COMPATIBILITY/` holds the plan, the workbook and the evidence. The code would stay in `openaudio-mafw-bebob/` (DEC-9); `openaudio-fw1814/` stays frozen.
* **Versioning:** a new plan version is a new file, `BROAD_COMPATIBILITY_PLAN_v2.md` and `.xlsx`.
* **Publication:** `scripts/sync-to-public.sh` publishes this folder too (it contains no binaries, so its audit passes). Exclude it there if the proposal should stay private.

## Correction block (2026-10-21, R125)
* **U-11 (linking part) settled by E-17:** building against the 15.x SDK resolves the two reserved slots; no linker patching is needed. Whether it loads and runs is still open (BC-6.1).
* **BC-3.2 is not needed on 14:** the `_darwin14` lock names, `___memmove_chk` and `___strlcat_chk` resolve there. It stays relevant for 10.9 to 10.12 only.
* **BC-3.3 becomes:** build the Main band with `make BAND=main` against the 15.x SDK (Makefile change done). RK-3 is lowered, not closed.
* **Work started on this Mac** (macOS 14.8.9, user decision: build here, Xcode 16.2 being installed). Panel: the 0.2.2 build is expected to be reusable unchanged (E-10, bundled Swift runtime, x86_64); real-driver behaviour unverified.

## Correction block (2026-10-21, R126 + R127)
* **BC-6.1 partly done (E-18):** the Modern kext loads, attaches and plays on macOS 14.8.9 Intel. **RK-4 for Intel is answered on the transmit side:** classic DCL from the kernel runs on 14. Still open: record/RX data, the panel's controls against the real kext (U-8), MIDI, other rates, the FW1814, unload, sleep/wake.
* **U-11 settled for the Modern route:** the 15.x SDK build loads. Whether `kmutil` patches the reserved slots of a 10.13-SDK build was never tried and no longer matters.
* **Name fix:** the correction block above says `make BAND=main` and R125 says `-DOA_BAND_MAIN`. The real names are `make BAND=modern` and `-DOA_BAND_MODERN` (kext/Makefile).
* **New risk seen (RK-6, adapter chain):** one dropout during real use. The adapter's FireWire controller left PCI before any driver event; the interface was bus-powered through the adapter. Cause open: the user suspects a bad contact, and it is on demand (R123).
* **Panel per edition (R127):** the panel binary is shared, but its About tab reads `OAEdition` (`make -C control-panel app BAND=…`). Modern builds go to `control-panel/build/modern/`; `scripts/install-modern.sh` installs that panel when present. The Modern panel is still the Swift 4.1 build from the build PC, with its bundled runtime.
* **Decisions:** DEC-5 is answered (three editions, §11a). DEC-7 is answered for the Modern Intel band only (the dev Mac).

## 13. Correction block (2026-10-21, R129)
* **§1, "Minimum number of builds":** for Intel 10.13 to 15 it is **2 builds from one source** (Classic: 10.13 SDK; Modern: 15.2 SDK), delivered as two packages (DEC-5 = b). The "one Mach-O for 10.9 to 15" idea is not used; whether the 10.13-SDK build would also load on 11+ was never tried and is moot (R126).
* **§4, band M11:** "same Main x86_64 slice (U-11)" is wrong; Modern has its own build. U-11 is settled by loading (E-18).
* **§7, BC-3.2 and BC-4.2:** not needed on 14; relevant only if 10.9 to 10.12 is ever offered.
* **§7, BC-5.1:** the OS-conditional installer JavaScript is not used. Each edition has its own `distribution.xml`; Classic `min 10.13 before 11.0`, Modern `min 11.0 before 16.0`; both limits are honoured (checked with `installer -volinfo` on 14.8.9 and 10.13.6, R128).
* **§11a:** Classic row: the package range is 10.13 to <11, and 0.3.0 is built. Modern row: built, loaded and playing on 14.8.9; packaged with a symbol gate; uninstall on 11+ unverified.
* **Names:** `make BAND=modern`, `-DOA_BAND_MODERN` (unused by the sources).
* **New evidence:** the editions document `docs/EDITIONS.md`; R129 in `PROVENANCE-RESOLUTIONS.md`.

## 14. Scope change (2026-10-22, R130): floor 10.9, Legacy dropped
**User decision:** *"change the scope of the whole plan and documentation on legacy to 10.9 onwards. the older ones work well on official drivers."* The official M-Audio drivers run on 10.6.8 to 10.8.5 (v1.9 on the SL rig is the reference, B5), so our driver would add nothing there except our panel, and that panel would cost a rewrite (see below).
* **Dropped:** the Legacy edition and band L; tasks BC-1.3, BC-2.2, BC-3.4, BC-4.4, BC-4.5, BC-4.7 and BC-6.3; decisions DEC-2 (32-bit kernel), DEC-4 (Obj-C panel for 10.6-10.8) and DEC-6 (10.6 SDK) become moot; U-1, U-3 and the 10.6/10.7 rows of the OS matrix are no longer needed; scenario S3 and "S1 + L" disappear. RK-1, RK-8 (the 10.6 SDK part), RK-10 and RK-11 no longer apply. The i386 slice, the Obj-C panel port (6 to 10 sessions) and the old toolchain (1 to 3) are not needed.
* **DEC-1 is answered: option b** (lowest OS 10.9; the Swift panel is kept, no old toolchain).
* **Kept in scope:** Classic extended to 10.9-10.12 (BC-3.2 lock-name binding, BC-4.2 `#available` fallbacks for five panel APIs, BC-6.2 one run on such a Mac), Modern 11-15 Intel, and Apple Silicon as stage S2 (DEC-3 still open).
* **Effort now:** S1 (Intel 10.9 to 15) **13 to 18.5 sessions** as in §11, of which the Modern Intel part is done (§0); S2 (S1 + Apple Silicon) **22 to 36**. The S3 figure of 33.5 to 57.5 is withdrawn.
* **Editions:** two, not three: Classic (10.9 to 10.15 as a target; the installer still starts at 10.13 until BC-6.2) and Modern (11 to 15). The `legacy` value in the Makefiles and in `Edition.swift` is unused and stays as dead code until someone wants to remove it.
* **The SL rig stays**, as the ground truth for captures (v1.9 on 10.6.8); it is no longer a build target.
