# Q-SDK.5 — What the "earlier failure" before the 10.13 panic actually is

> **Correction (2026-10-15, live evidence, R40):** the statement in §2 that round two "has never been exercised" no longer holds for the **clone**: on the 10.13.6 build PC the clone's two-round design ran end to end (round 1 rejected quietly, `MAFirmwareStart` -> round 2 -> `com_openaudio_FW1814Device` shell started, no panic over ≥ 6.5 min, also after install + reboot). Still untested on 10.13.6: everything past `initHardware` (engines, DCL, CMP, mixer) — the shell sets names only. Log: [`build-pc-runs/2026-10-15-M-A-M-FW-install.md`](../build-pc-runs/2026-10-15-M-A-M-FW-install.md).


**Status:** done 2026-10-14 (static, dev Mac only; no hardware, no build-PC access). Evidence entry: `PROVENANCE-RESOLUTIONS.md` R31. Bundle: `_extracted/` kext, bundle version 1.11.3 (same binary as `legacy-mod`, R24). 10.13.6 side: `_refs/macos-10.13.6-fw/` (`audio.asm`, `kernel.asm`, `kernel.nm`).

## Answer in one paragraph

**There is no failing init step.** The candidate that is freed in the panic (`FW1814Device`, a `FWAudioDevice` subclass) is **deliberately rejected by `FWAVCAudioDevice::probe` on the first matching round** and released without ever reaching `start()`/`initHardware`. That ordinary release calls `FWAudioDevice::free()` (and the destructor chain) on an object whose members were never allocated, and those functions call `operator delete(NULL)` without a NULL test. The reference's 2009–2014 kernels tolerated that; the 10.13.6 `operator delete` does not (§4). So on 10.13.6 the reference panics on **every** device attach, regardless of device state, firmware or user action.

This replaces the "init/probe/start failure that is not yet identified" wording in Q-SDK §5, Q3b §1 and R29. Steps 2–5 of `initHardware` (Q3b §1) are **not** the cause — they never run in the panicking path.

## 1. Observations (all from raw disassembly; addresses are `_extracted/` kext file addresses unless stated)

1. `FWMetaNub::start @0x1cf68` copies the device properties, calls `FirmwareCacheInfoRegisters`, sets `fState [+0x88] = 1` (`0x1d16d`, "initial-register") and then calls `registerService(0)` (`vt[0x5b0]` @`0x1d17b`..`0x1d180`). `registerService` starts matching of the per-product personalities (`IOProviderClass = com_m_audio_FWMetaNub`, matched on `FireWire Product Name`; `plutil -p` of the plist).
2. `FWMetaNub::FirmwareCacheInfoRegisters` (asm 34356) reads 0x50 bytes at `0xFFFF:0xC8020000` into `[+0x90..]` and sets `[+0xf0] = ([+0x9c] != 0)` (`0x1cd09`/`0x1cd11`) — the "bootloader active" flag.
3. `[+0xf3]` ("firmware match started") is **only** written at `0x1bf7a`, inside `FWMetaNub::FirmwareStart @0x1beb4` (Ghidra `FWMetaNub__FirmwareStart.c`): if `[+0xf0] == 0` and `[+0xf3] == 0` it logs "Firmware active: registering metanub", sets `[+0xf3] = 1` and calls `registerService(0)` **a second time**. In bootloader mode it instead writes the boot-from-flash cue (`0xC8021000`) and re-registers after the reset (`ReregisterDeviceAfterFirmwareReset`).
4. `FirmwareStart` has a single caller: `FWMetaNub::SetOneProperty` at `0x1caf8`, reached when the property key `MAFirmwareStart` is set on the nub (`0x1cae1` literal, `0x1caf3` branch). In other words **only a user process (the helper) can trigger the second registration** — the reference helper does it for every device state, runtime included (A4 H2 pseudocode lines 61/65/75; H11: a 60-byte registry write `MAFirmwareStart=YES` at helper start, capture 71).
5. `m_audio_b_FWAVCAudioDevice::probe(IOService*, int*) @0x22f78` (the only `probe` override in the device chain; `FWAudioDevice`, `FWRev1AudioDevice` and `FW1814Device` have none — `grep` of the symbol table):
   - `*score = 0` at entry (`0x22fc1`); provider must cast to `FWMetaNub` and have `FireWire Product Name`; the provider's name must `isEqualTo` the personality's name (`0x22fd1..0x22ff6`); if both provider and personality carry `Model_ID` they must be equal (`0x22ffe..0x23038`); mismatch → return NULL.
   - Then `cmpb $0, 0xf0(%r13); jne -> NULL` (`0x23050`: bootloader active → reject) and `cmpb $0, 0xf3(%r13); je -> NULL` (`0x2305a`: firmware match not yet started → reject).
   - Only if both pass: `*score = 0x7FFFFF` (`0x23068`) and return `this`.
6. Combining 1, 3 and 5: during the **first** round (triggered from `FWMetaNub::start`), `[+0xf3]` is 0 by construction, so the product personality's candidate **always** returns NULL from `probe`. The second round (after `MAFirmwareStart`) is the one that accepts it.
7. A rejected candidate is an already-constructed object: IOKit allocates and `init`s the class before calling `probe`. The kernel then releases it. 10.13.6 `IOService::probeCandidates @0xffffff80008715b0` (`kernel.nm`): the return address `+0x770` = `0xffffff8000871d20` follows `callq *0x28(%rax)` on `%r12` (`0xffffff8000871d1d`), i.e. a virtual `release()` (slot 0x28; same slot the kext uses for `release`, e.g. `0x23a2`; `registerService` is the same `vt[0x5b0]` that 10.13.6 `IOAudioDevice::start` calls at `0x410f`). That is exactly the frame in R22/R29 (`probeCandidates+0x770` → `free`).
8. `m_audio_b_FWAudioDevice::free() @0x2372` (`otool-exports/audiodevice-free-2372.asm`): `[+0x100]` is NULL-guarded, then `movq 0x110(%rbx),%rdi; callq __ZdlPv` at `0x23b0..0x23b7` **without a test**. `[+0x110]` is only allocated at `0x3624..0x3635` near the end of `initHardware` (Q3b §1), so it is NULL in a never-started object.
9. The destructor chain has many more unguarded deletes: `FW1814Device::$_667::~$_667 @0xc410` deletes `[+0x820]`, `[+0x810]`, `[+0x518]`… with no test (`0xc43e..0xc48f`); `TFunctor<…>::~TFunctor @0xf5bc` deletes `[+8]` (`0xf5d2..0xf5d6`); `FW410Device::$_663` is the same pattern (`0x13ab3`, panic 2026-10-01 11:01). The whole binary contains **218** `callq __ZdlPv` sites (`grep -c "callq\t__ZdlPv" mafw.asm`) and 273 mentions of the symbol.
10. 10.13.6 `operator delete(void*)` = `__ZdlPv @0xffffff8000830c60` (`kernel.asm`): `callq _kalloc_size` → `lock addl` on `_debug_iomalloc_size` → `jmp _kfree_addr`. **No NULL test.** `kalloc_size(NULL)` returns 0 and `kfree_addr(NULL)` panics with the captured string `Attempting to lookup/free an address not allocated via kalloc! … addr: 0` (R29, panic report).

## 2. What this explains (and what it does not)

| Fact | Explained? |
|---|---|
| R27: clean `kextload` with no device, no panic | yes — no nub, no candidate |
| R29 / capture 75: panic in the matching thread with no user action, immediately after the interface powers on | yes — first-round reject → release → `delete(NULL)` |
| R22: four of six build-PC panics share `probeCandidates → free() → delete(NULL)` | yes |
| 2026-10-01 11:01 panic from `~FW410Device` (member-dtor delete(NULL), called from IOAudioFamily) | consistent — same pattern in a different class; **why a `FW410Device` was instantiated is still unexplained** (conditions of that experiment unrecorded; Q-SDK §5) |
| Older kernels (SL 10.6.8, v1.9 rig) run the same driver without panic | **hypothesis (unverified)**: their `operator delete`/`kern_os_free` tolerated NULL. Check: disassemble the SL kernel's `__ZdlPv`, or read xnu-1504 `libkern/c++/OSRuntime.cpp` — neither is in the repo |

**Not verified:** that the build-PC device was in bootloader mode at the time of the panic (irrelevant to the first-round reject, but relevant to what happens in round two); the Model_ID/state at capture 75 was never recorded. What *would* happen **after** the second registration (accept → `start` → `initHardware` steps 2–7 on 10.13.6) has **never been exercised**, because every run died in round one. Steps 2–5 remain untested on 10.13.6, not proven good.

> **Correction (2026-10-14):** the user chose **D2 (full clone)**; the D1-specific bullets in §3 (patch mechanisms P-A/P-B/P-C, retargeting `__ZdlPv`, re-signing the reference) are void. What carries over to the clone: every `free()`/destructor must be NULL-safe, the two-round lifecycle must be mirrored, the helper must send `MAFirmwareStart`, and round-two risks (classic-DCL acceptance, IOBMD) apply to the clone's own DCL code.

## 3. Consequence for D (reasoning — not a measurement)

- The unknown "earlier failure" that made D1 look unbounded is **resolved**: the first fault is a systematic NULL-delete, with one root mechanism (`operator delete(NULL)`), not a scatter of init failures.
- The 10.13-forced modification for D1 is therefore **one narrow change**: make every `operator delete` the kext executes NULL-safe. Candidate mechanisms, none tried (code/binary freeze): (a) retarget the kext's external relocations for `__ZdlPv` to a tiny local NULL-checking wrapper placed in unused `__TEXT` padding (one wrapper, ~218 call sites follow automatically); (b) patch `free()` + the identified destructors individually (many sites). Both are binary edits and need re-signing (B3.2); neither is authorised yet.
- D1 additionally needs the **second-round trigger**: something must set `MAFirmwareStart` on the nub after attach (kext point 4). The reference helper does; the project's own `openaudio-fw1814/firmware-helper` does not reference `MAFirmwareStart` at all (grep, 2026-10-14) — **for D1 the helper's registry-write protocol (A4 H2/H11) must be reproduced**, or the second registration is never reached and the device gets no driver. **Unverified**: how the project helper uploads firmware today (A4 says it uses the UC only for notification), so check its upload path before assuming a gap.
- Residual D1 risk: round two is untested. Candidate 10.13-sensitive spots to examine statically next, in order: `FWAVCAudioDevice::initHardware @0x22ed2` (provider cast, `[+0xe0]`), `AllocateConnectionManager @0x23c72`, `createAudioEngine @0x2fd8`, `FWEngine2::initHardware @0x12622` → `FWDCLProgram::Initialize @0x27458` → `RebuildDCLProgram @0x25586` → virtual `BuildTransmitting/ReceivingDCLProgram`, `InitializeUserSettableProperties @0xe6ca`. Not analysed here.

## 4. How this was checked / how to falsify

- Falsify the central claim: if a `kextload` on the build PC with `operator delete` made NULL-safe still panics in `probeCandidates`, point 7/8 are incomplete. That test needs the frozen binary patch.
- Cheaper falsifier (read-only on the build PC, needs the user's OK): the persisted log around the R29 panic should show `M-FW: metanub 0x… GUID:…` (`IOLog` at `0x1d159`) **and no** `M-FW: Firmware active: registering metanub` (`FirmwareStart`'s log) — both would fit this account. **Unverified** whether those lines survive.

## Provenance

`otool-exports/mafw.asm` lines 34570 (`FWMetaNub::start`), 34356 (`FirmwareCacheInfoRegisters`), 34180–34258 (`SetOneProperty`), 42088 (`FWAVCAudioDevice::probe`), 42035 (`FWAVCAudioDevice::initHardware`), 13949 / 17020 / 17235 / 47358 / 45217 (`$_667` dtor, `FW1814Device` D1/D2, `FWDCLProgram::Initialize`, `RebuildDCLProgram`); `otool-exports/audiodevice-free-2372.asm`; Ghidra `ghidra-exports/decomp/FWMetaNub__FirmwareStart.c`; `_refs/macos-10.13.6-fw/kernel.asm` (`__ZdlPv`, `probeCandidates`), `kernel.nm`, `audio.asm` (`IOAudioDevice::start`, 10.13.6); `docs/system-map/build-pc-panics/*`; R22, R27, R29; A4 `H2-bootloader-detection.md`, `H11-settings-persistence.md`.
