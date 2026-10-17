# Q-SDK — 10.13 SDK reserved vtable slots vs the reference kext

> **Correction (2026-10-14) — real kextload WITH the device** (evidence: [75-imac-kextload-with-device-panic.txt](../B5-sl-dtrace-truth/captures/75-imac-kextload-with-device-panic.txt), [audiodevice-free-2372.asm](otool-exports/audiodevice-free-2372.asm), [PROVENANCE-RESOLUTIONS.md §R29](../PROVENANCE-RESOLUTIONS.md)). Loading the reference 1.11.3 kext with the FW1814 powered on in the High Sierra iMac reproduces the panic: `probeCandidates+0x770` -> `m_audio_b_FWAudioDevice::free()` (`0x2372`, `__ZdlPv` at `0x23b7`, `[+0x110]` NULL) -> `kalloc_size` panic. This confirms the "operator delete(NULL) in teardown" cause; the failing step before `initHardware` ends is still **unverified**.

> **Correction (2026-10-14) — real kextload without the device** (evidence: [kextload-real-no-device-1.11.3.txt](otool-exports/kextload-real-no-device-1.11.3.txt), [PROVENANCE-RESOLUTIONS.md §R27](../PROVENANCE-RESOLUTIONS.md)). A **real `kextload`** of the same 1.11.3 kext (staged in `/tmp`, user-authorized) on the build PC with **no FW1814 attached** (bus = `IOFireWireLocalNode` only) **loaded successfully and stayed stable**: `kextstat` listed `com.m-audio.driver.firewire (1.11.3)`, `ioreg` showed **zero** `com_m_audio_*`/`m_audio_b_*` objects (no `FWMetaNub`, hence no product candidate), no `M-FW` log lines, no new panic report, uptime continuous; `kextunload` then succeeded cleanly. So the `probeCandidates → free() → delete(NULL)` panic **needs a matching provider (a device) — it is not triggered by loading the kext alone**. Consequence for Q-SDK.4: the earlier panics were taken with a device present; the candidate-release reason can only be observed by repeating the load with the FW1814 on this machine. Related: the `FWMetaNub` personality has no `Model_ID`/vendor filter (`IOProviderClass = IOFireWireDevice`), so *any* remote FireWire device would create a nub; the product personalities then match on `FireWire Product Name`, so only an FW1814 yields a `FW1814Device` candidate — **unverified** what a non-M-Audio device would do.


> **Correction (2026-10-14) — kextutil dry run** (evidence: [kextutil-n-dryrun-1.11.3.txt](otool-exports/kextutil-n-dryrun-1.11.3.txt), [PROVENANCE-RESOLUTIONS.md §R26](../PROVENANCE-RESOLUTIONS.md)). User-authorized `kextutil -n -t -v 6` on the 10.13.6 build PC (kext staged in `/tmp`, not loaded): **"appears to be loadable (including linkage for on-disk libraries)"**, exit 0 — every symbol resolves against the on-disk IOAudioFamily 206.5 / IOFireWireFamily 4.7.2 / IOFireWireAVC 4.2.6, so there is no link-time breakage (consistent with the no-slot-drift result). The only complaint is the signature: `Kext with invalid signatured (-67007) allowed … Code Signing Failure: code signature is invalid` (the 2014 SHA-1 / version-1 signature; tolerated because SIP is disabled on this machine). The panic is therefore a **runtime** failure (matching → `free()` → `delete(NULL)`), not a link failure. A real `kextload` was not run (hard rule; the user expects it to crash and may authorize it separately).


> **Correction (2026-10-14) — panic symbolication** (evidence: [PROVENANCE-RESOLUTIONS.md §R22](../PROVENANCE-RESOLUTIONS.md)). **Symbolication of all six build-PC panics** against the 10.13.6 kernel (`/System/Library/Kernels/kernel`, sha256 `4d51fac0…`, kernel slide from each report; `_refs/macos-10.13.6-fw/kernel.nm`): four panics (2026-09-29 15:59, 10-01 10:54, 10-01 11:01, 10-05 19:49) share the same stack `_IOConfigThread::main → IOService::doServiceMatch+711 → IOService::probeCandidates+1904 → [kext free()] → operator delete (__ZdlPv+14) → kalloc_size+80/+201 → panic` — i.e. the release of a matching **candidate instance** inside `probeCandidates` (call `*0x28(%rax)` = `release()` of the instance at the end of the candidate loop, kernel asm 0x…871d1d). `__ZdlPv` has **no null check** (`callq _kalloc_size` immediately), so `delete [+0x110]` with a NULL member panics. The other two panics (09-29 21:39, 23:03) are `iokit_user_client_trap` crashes inside earlier builds. Bundle note: the loaded kext is bundle **1.11.3** (identical sha256 to `_extracted/kext` and `legacy-mod`; its source tree string is `FireWireBeBoB_01.11.002`); panel 1.5.2 / helper UI 1.2.2 / MIDI 1.11.2 belong to the 1.11.2 installer. **Why the candidate was released (init/probe/attach/start failure vs losing candidate) is still unknown** and cannot be read statically; and the FW1814 is **not attached to the High Sierra machine** (user, 2026-10-14), so no on-device test is possible there.


**Status:** done 2026-10-14. Answers BUILD-MANUAL §2.A Q-SDK. **Contradicts B3 breaker #3** (see §4). That breaker is also evidence D-decision relies on.

**Sources:**
- **SDK:** build PC `192.168.100.20` — macOS 10.13.6 (17G14042), Xcode 9.4.1, `MacOSX10.13.sdk`. Headers are under `…/Kernel.framework/Headers/IOKit/{audio,firewire,}/`. Read over SSH on 2026-10-14.
- **Reference:** `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` (v1.11.x shipping kext). Read with `nm -um`.

## 1. Method

1. **Reference side.** Run `nm -um` on the x86_64 slice and keep the undefined `_RESERVED<Class><n>` symbols (265 in total). If the reference imports `_RESERVED<Class><n>`, slot `n` of `<Class>` was **unused** in the SDK the reference was built against. That is because the compiler only emits a `_RESERVED` symbol into a subclass vtable for a pad slot. So the lowest imported `n` gives the number of slots that were already used when the reference was built.
2. **10.13 side.** Grep the SDK headers for `OSMetaClassDeclareReservedUsed(<Class>, n)`. Then read the `#if __LP64__` branches by hand, because the core-class headers declare different slot usage for 32-bit and 64-bit builds.

## 2. Result — every parent class the reference kext imports pad slots from

| Parent class | Ref: first unused slot (pad slots imported) | 10.13 SDK slots used (LP64) | Drift? |
|---|---|---|---|
| `IOAudioDevice` | 6 (26) | 6 | **none** |
| `IOAudioEngine` | 15 (33) | 15 | **none** |
| `IOAudioEngineUserClient` | 12 (20) | 12 | **none** |
| `IOAudioStream` | 12 (36) | 12 | **none** |
| `IOFWIsochChannel` | 0 (4) | 0 | none |
| `IOFWIsochPort` | 0 (2) | 0 | none |
| `IOFireWireAVCNub` | 1 (3) | 1 | none |
| `IOFireWireAVCUnit` | 0 (4) | 0 | none |
| `IOFireWireAVCAsynchronousCommand` | 0 (4) | 0 | none |
| `IORegistryEntry` | 0 (32) | 0 — slots 0–5 are `Used` only in the `#else` (32-bit) branch, `IORegistryEntry.h:181–195` | none |
| `IOUserClient` | 0 (16) | 0 — 32-bit branch only, `IOUserClient.h:215–221` | none |
| `IOService` | 0 (48) | **2** — `IOService.h:435–437` (`#if __LP64__`: slots 0, 1 `Used`) | **slots 0–1 consumed** |
| `IOEventSource`, `OSObject`, `OSMetaClass`, `OSMetaClassBase` | — | 0 | none |

The `IOService` slots 0 and 1 hold `configureReport(...)` and `updateReport(...)`, the IOReporting API declared at `IOService.h:415` and `:429`. They sit immediately before the reserved-slot block. IOReporting is a 10.9-era addition. **Hypothesis:** the reference was therefore built with a pre-10.9 SDK. The slice can't confirm it: `otool -l` shows no `LC_VERSION_MIN_MACOSX` or `LC_BUILD_VERSION` load command (checked 2026-10-14).

## 3. Observations

- **The IOAudioFamily and IOFireWireFamily vtables are slot-for-slot identical** between the reference's build SDK and the 10.13 SDK, for every class the reference subclasses.
- **The only slots consumed since are `IOService` LP64 slots 0–1.** The reference's subclasses do not override `configureReport` or `updateReport`; neither symbol appears in `nm -m` of the slice. They inherit the pad slots (the vtables point at `_RESERVEDIOService0/1`).

## 4. Implications (reasoning — not yet a hardware capture)

- **B3 breaker #3 is not supported for IOAudioFamily or IOFireWireFamily.** B3 said: *"Apple consumed `_RESERVEDIOAudioEngine*` / `_RESERVEDIOFWIsochPort*` slots between 10.11 and 10.13; subclass virtual overrides shift."* The SDK shows no `IOAudioEngine` or `IOFWIsochPort` slot consumed relative to the reference.
- **The `IOService` 0–1 consumption is the case reserved slots exist for.** **Unverified — needs a check:** xnu's kext linker (kxld) should patch a subclass vtable entry that points at a now-used parent pad symbol so that it resolves to the parent's new method. Overrides in other slots do not move. A link-only dry run on the build PC would confirm this: `kextutil -n -t` with the reference bundle staged in `/tmp`, **never** in `/Library/Extensions`. That run counts as an "experiment" under CLAUDE.md, so it needs the user's go-ahead.
- **Consequence:** the reported 10.13 autoload panic of the reference (memory `feedback_reference_kext_panic_risk.md`) is **not explained** by reserved-slot drift. §5 decodes the actual panics.
- **For a D2 clone** built against this SDK, the question does not arise. For **D1/D3**, breakers #1 (approval for unsigned kexts on 10.13) and #2 (SHA-256 code-signature requirement) still stand per B3. Breaker #3 is downgraded to "unexplained panic, cause unknown".

## 5. Panic forensics — what actually panicked on 10.13 (build-PC logs)

Six kernel panic reports from the build PC (`/Library/Logs/DiagnosticReports/Kernel_2026-*.panic`) were copied verbatim to `docs/system-map/build-pc-panics/` on 2026-10-14. Kext frames are symbolicated as *return address − kext load address* against `otool-exports/mafw.asm`. The reference `__TEXT` vmaddr is `0x0`, so the offset is the file address.

| Panic | Kext | Panic string | Kext frame(s) → function |
|---|---|---|---|
| 2026-09-29 15:59 | `com.m-audio.driver.firewire.dice` 2.7 | `kfree`: *"Attempting to lookup/free an address not allocated via kalloc! … addr: 0"* | not symbolicated (DICE binary not in repo) |
| 2026-09-29 21:39, 23:03 | `com.openaudio.driver.fw1814` 0.1 (pre-Ghidra clone) | page fault, type 14 | our old clone; not relevant to the reference |
| **2026-10-01 10:54** | `com.m-audio.driver.firewire` **1.11.3** | `kfree … addr: 0` | `0x23bc` → `m_audio_b_FWAudioDevice::free()`: `0x23b0 movq 0x110(%rbx),%rdi` / `0x23b7 callq __ZdlPv` |
| **2026-10-01 11:01** | 1.11.3 (called from IOAudioFamily) | `kfree … addr: 0` | `0x13ab8` → `com_m_audio_FW410Device::$_663::~$_663()`: `0x13aac movq 0x6a0(%rbx),%rdi` / `0x13ab3 callq __ZdlPv`; caller `0x15896` → `~com_m_audio_FW410Device()` |
| **2026-10-05 19:49** | 1.11.3 | `kfree … addr: 0` | `0x23bc` → `m_audio_b_FWAudioDevice::free()` (same site as 10-01 10:54) |

**Observations (cited above):**
- Every reference-kext panic is `operator delete` (`__ZdlPv`) on a member pointer during **teardown** (`free()` or a destructor). The panic string reports `addr: 0`, so the pointer was NULL.
- The DICE kext panics with the identical string, which suggests this is a shared M-Audio code pattern (**hypothesis**).
- `FW1814Device`'s constructor (`0xf04a`) calls `FWRev1AudioDevice::FWRev1AudioDevice`, not `FW410Device`. So the 11:01 panic involved a real `FW410Device` instance. **Unverified why:** the `FW410Device` personality matches only `FireWire Product Name = "FW 410"`. The conditions of the 2026-10-01 experiment (which plist/bundle was loaded) were not recorded.

**Update (2026-10-14):** `[+0x110]` (the pointer deleted at `0x23b7`) is only allocated at `0x3635`, near the end of `FWAudioDevice::initHardware`, after `AllocateConnectionManager`, `createAudioEngine`, `RequestFormatUpdateForSampleRate`/`CompletePendingStateChanges` and `InitializeUserSettableProperties`. A failure in any of them, or a start failure before `initHardware`, produces exactly this panic (Q3b §1).

**Implications (reasoning):**
- **The panic is a teardown bug, not an ABI mismatch.** On this kernel, `operator delete(NULL)` reaches `kfree` and panics. **Hypothesis:** older kernels tolerated this. It needs checking, either from the xnu `kern_os_free` source for 10.6–10.13 or from the SL rig's behaviour.
- **Teardown means something failed earlier.** The object was being freed, so `init`, `probe` or `start` failed (or the driver was rejected) before the panic. **That earlier failure is the real unknown** for any D1/D3 path. It is not visible in the panic log.
- **For D2:** the clone must not `delete` or `IOFree` NULL members in `free()` or destructors. Every reference `free()` path needs NULL guards — a 10.13-forced deviation that we must comment in the code.
- **For D1/D3:** the panic site can be patched (two NULL checks). The unknown earlier failure, plus breakers #1 and #2, remain.

> **Correction (2026-10-14, Q-SDK.5 / R31):** the "earlier init/probe/start failure" above is resolved: it is the reference's own first-round `probe` reject (`FWAVCAudioDevice::probe @0x22f78` returns NULL until the helper sets `MAFirmwareStart`), after which the never-started candidate is freed and `operator delete(NULL)` panics on 10.13.6. See `Q-SDK5-first-round-probe-reject.md`. The statement "for D1/D3 the panic site can be patched (two NULL checks)" is also corrected: there are ~218 `__ZdlPv` call sites; a NULL-safe delete wrapper is the narrow fix.

## Provenance

- Reference imports: `nm -um _extracted/kext/M-AudioFireWireBeBoB.x86_64-slice | grep -i reserved` → 265 symbols, 2026-10-14.
- SDK: `grep OSMetaClassDeclareReservedUsed` over `MacOSX10.13.sdk/System/Library/Frameworks/Kernel.framework/Headers`, plus a manual read of the `__LP64__` branches in `IOService.h:435–453`, `IORegistryEntry.h:181–198`, `IOUserClient.h:215–228`. Done on `192.168.100.20`, 2026-10-14.
