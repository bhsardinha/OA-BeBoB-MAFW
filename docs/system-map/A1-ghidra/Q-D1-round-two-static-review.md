# Q-D1.1 — Round-two static review: what happens on 10.13.6 after the nub registers a second time

**Status:** done 2026-10-14 (static only, dev Mac; no build-PC access, no patch, no load). Evidence entry: `PROVENANCE-RESOLUTIONS.md` R33. Bundle: `_extracted/` kext 1.11.3. 10.13.6 side: `_refs/macos-10.13.6-fw/` (`audio.asm` = IOAudioFamily 206.5, `AppleFWOHCI.asm` = AppleFWOHCI 5.5.9, `kernel.asm`); family source `_refs/IOFireWireFamily` is **4.8.3** (10.13.6 runs 4.7.2 per the panic reports) — version skew, used for shape only.

> **Correction (2026-10-14):** the user chose **D2**; the file keeps its name for citation stability. Stage table S1-S10 and risks R-1, R-2, R-4, R-5, R-6, R-7 still apply to the clone (the clone re-implements the same stages); the D1-only risk R-3 (re-signing the reference) becomes "signing the clone", and the "patch" parts of the test ladder become "build and stage the clone".

> **Update (2026-10-14, R36):** S6 (pool allocation) and S9 (classic-DCL acceptance) are now analysed in `Q-P1P2-dcl-acceptance-and-pool-memory.md`: acceptance depends on `generateBufferMap`, which the reference's layout satisfies by construction; the pool allocation is `KMA_LOMEM` (not contiguous). Both remain **unverified at run time**.

**Purpose.** D1 (approved) = the official kext + minimal 10.13-forced changes. Every build-PC run so far died in *round one* (R31). This review lists what round two (accept → `start` → `initHardware` → engine → streaming) will exercise, what is verified, and what is not, so the plan's acceptance tests target the real risks. **No claim here says round two works.**

## 1. Stage table

| # | Stage (reference code) | What I checked on 10.13.6 | Status |
|---|---|---|---|
| S1 | `MAFirmwareStart` → `FirmwareStart @0x1beb4`: `[+0xf3]=1`, `registerService` again | kext-internal logic; log strings `M-FW: Firmware active: registering metanub` | **verified in kext** (R31); needs the helper (H12) |
| S2 | Matching: personality `FireWire Product Name = "FW 1814"` → `FWAVCAudioDevice::probe @0x22f78` accepts (score `0x7FFFFF`) | probe logic read in full (R31). `Model_ID` is only compared if the personality has one (FW1814's does not) | **verified in kext**. Competing driver **unverified** (see R-4) |
| S3 | `IOAudioDevice::start` (Apple) → `IOService::start`, `IOCommandGate`, `initHardware` through `vt[0x950]` | 10.13.6 `audio.asm` `IOAudioDevice::start @0x3fa2`: same order; Q-SDK shows IOAudioDevice vtable slots identical to the reference's SDK | **verified structurally** |
| S4 | `FWAVCAudioDevice::initHardware @0x22ed2`: provider → `FWMetaNub` → its upstream device (`vt[0x680]`, slot name not resolved) → cast to `IOFireWireNub` into `[+0xe0]`; then `FWAudioDevice::initHardware`; then `vt[0xb58]`, `vt[0xb00]`, `vt[0xb78]` | read in full | kext logic; **unverified** at runtime |
| S5 | `AllocateConnectionManager @0x23c72`: `new m_audio_b_FWAVCUnit` (0xf8 B, subclass of `IOFireWireAVCUnit`), `init`, `attach`, vt[0x5c0], then `new FWAVCConnectionManager` | `IOFireWireAVCUnit` has no slot drift (Q-SDK §2). `OSObject::operator new` on 10.13.6 zero-fills (`kern_os_malloc` calls `_bzero`, `kernel.asm`), so "never-set members are NULL" holds | structure **verified**; behaviour **unverified** |
| S6 | `createAudioEngine @0x2fd8` → `FWEngine2::initHardware @0x12622` → `FWDCLProgram::Initialize @0x27458` → `SetupCIP` + `RebuildDCLProgram @0x25586` → virtual `BuildTransmitting/ReceivingDCLProgram` (TX 20 seg / RX 64×4, `TPacketPool` via `IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task)`, Q17) | `RebuildDCLProgram` itself only frees the old `TDataBuffer` and calls `vt[0x150]`; **the Build functions and the IOBMD physical-mask allocation were not re-read here** | **unverified**; a failure here fails `start` |
| S7 | `InitializeUserSettableProperties @0xe6ca`, meter page, `ResetToFactorySettings @0xd190` (Q3b) | pure OSDictionary/OSNumber logic | low risk, **unverified** |
| S8 | `registerService` → IOAudioFamily publishes engines; CoreAudio sees the device | IOAudioFamily stays supported on 10.13 (general knowledge, **no repo evidence**) | **unverified** |
| S9 | Stream start → `CreateIsochChannelAndPorts @0x255c2` → `bus->vt[0x858]` = `IOFireWireController::createLocalIsochPort(talking, head, info=NULL, 0, 0, 0)` | Source (4.8.3): `createLocalIsochPort` → `fFWIM->createDCLProgram(...)`. 10.13.6 `AppleFWOHCI::createDCLProgram @0x75a4` still exists: allocates `AppleFWOHCI_DCLProgram` (or `_U` when `[+0x1b8] != 0`) and calls `init`. `AppleFWOHCI_DCLProgram::init @0x89d0` (asm line 8047): `super::init`; if no buffer map it calls `generateBufferMap(head)` and **fails (returns false → NULL port → `kIOReturnNoResources`) if the map is still NULL**; with `info == NULL` it takes the kernel-task branch (creates `AppleFWOHCI_CallbackEventSource`, adds it to the OHCI workloop) | classic path **exists and is plausible**; whether `generateBufferMap` accepts the kext's chain on 10.13.6 is **unverified** |
| S10 | Segment interrupts, CMP lock, FCP/AV/C, HSCI | OHCI interrupt from the CallProc callback (Q-OHCI-interrupt, 5.5.9); wire protocol decoded (Q3a, Q-CMP-connect, Q5/Q6) | decoded; **never run on 10.13.6** |

## 2. Findings

1. **Correction to Q1b (applied):** Q1b states "Classic DCL + NULL ranges works on 10.13.6 (the 2014 kext proves it)". That proof does not exist: the 2014 kext never got past matching on 10.13.6 (R29, R31). What *is* verified is that the classic entry points (`createLocalIsochPort` → `createDCLProgram` → `AppleFWOHCI_DCLProgram::init`) still exist and have a kernel-task branch in 10.13.6 (S9). Whether the chain is accepted is untested.
2. **NULL-safe `operator delete` is also the safety net for every failed `start`.** If any of S4–S7 fails, `start` returns false, the candidate is released and `free()`/destructors `delete(NULL)` — the same panic as round one (R31). With a NULL-safe delete a failure becomes a clean "no driver" instead of a kernel panic, which is what makes the staged hardware test survivable.
3. **A real extra D1 risk: `AppleFWAudio`.** `MakeAppleFWAudioGoAway` matches only `FireWire Product Name == "FW Solo"` in the reference (Q8), so on 10.13.6 Apple's driver may also try to attach to the FW1814's AV/C audio sub-unit. **Unverified** — nothing in the repo shows whether `AppleFWAudio` matches this device on 10.13. If it does, D1 needs a plist change (add a FW1814 "go away" personality). Any plist edit invalidates the signature anyway (B3.2), so it costs nothing extra once we re-sign. Check (read-only, build PC, **needs the user's OK**): `kextstat | grep -i fwaudio` and `ioreg -l` with the interface attached but *without* our kext.
4. **Order of the first hardware test is forced by the risks above.** S2→S3→S4/S5 can be observed with `IOLog` lines already in the binary (`M-FW: M-Audio FireWire device found`, `M-FW: metanub …`, `M-FW: Firmware active: registering metanub`), without new instrumentation.

## 3. Risk register (ranked) → acceptance tests for the build plan

| ID | Risk | Likelihood (my reading) | Check |
|---|---|---|---|
| R-1 | `AppleFWOHCI_DCLProgram::init` rejects the kext's classic chain (`generateBufferMap`) → no streaming | medium, **unverified** | T5 below; static follow-up: read `IODCLProgram::generateBufferMap` (10.13.6 IOFireWireFamily.bin) |
| R-2 | `TPacketPool` / `inTaskWithPhysicalMask` allocation fails on 10.13.6 → `start` fails | low–medium, **unverified** | T3; static follow-up: Build*DCLProgram + Q17 |
| R-3 | Re-signed kext not loadable with SIP on (B3.2) | **unknown** | backlog #12 (user: Recovery) |
| R-4 | `AppleFWAudio` also claims the device | **unknown** | read-only `ioreg`/`kextstat` check (needs OK) |
| R-5 | Helper never sends `MAFirmwareStart` (H12) | certain with the current helper | build the `MAFireWireLoader`-style helper first |
| R-6 | `FirmwareUpdate` (BeBoB251/BeBoB32) path untested on 10.13.6 | **unverified** | only needed when the bundled `.bcd` is newer |
| R-7 | Panel/MIDI/status UI bitness (panel Carbon 32-bit, status window i386-only; MIDI plug-in has an x86_64 slice) | certain for panel/status window | new AppKit panel; MIDI plug-in check on 10.13 (**unverified**) |

**Staged test ladder (all in `/tmp` on the build PC, never `/Library/Extensions`; each step is a go/no-go for the next; none authorised until the build plan is approved):**
- **T1** patched+re-signed bundle passes `kextutil -n -t` (link, signature allowed).
- **T2** `kextload` with the interface **off**: clean load, no panic (as R27).
- **T3** interface on, helper running: no panic; log shows `metanub …` then `Firmware active: registering metanub`; `FW1814Device` instance appears in `ioreg`.
- **T4** CoreAudio lists the device (S8) — if not, inspect `AppleFWAudio` (R-4).
- **T5** start a stream at 48 kHz/512: `createLocalIsochPort` returns non-NULL, packets flow (R-1).
- **Fallback trigger (to D2):** T3–T5 fail in a way a small, justified patch cannot fix. Define "small" in the plan before testing.

## 4. Not verified / not done here

Build functions (S6), `IODCLProgram::generateBufferMap` behaviour, IOAudioFamily runtime behaviour on 10.13.6 (S8), anything on hardware. This review is a map of risk, not a go-ahead.
