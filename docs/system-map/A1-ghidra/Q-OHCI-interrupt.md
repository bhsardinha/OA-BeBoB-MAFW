# Q-OHCI-interrupt — what makes OHCI raise the per-segment interrupt on 10.13.6

**Status:** done 2026-10-14 (closes audit flags AUD-C2-08, AUD-C4-08, AUD-Q1-01, AUD-D-07; matrix item `A1-Q1.2`).
**Bundle under test:** `AppleFWOHCI` **5.5.9** inside `IOFireWireFamily` 4.7.2 on the build PC (10.13.6), read-only `scp` of `/System/Library/Extensions/IOFireWireFamily.kext/Contents/PlugIns/AppleFWOHCI.kext/Contents/MacOS/AppleFWOHCI` (sha256 `3d66e5db6d1b5ab051fd9c5bafad1cc66ecca63495e335c38d61c74f37bca243`; `IOFireWireFamily` binary sha256 `23594cd566d180973bdbf3b3dc9fb9ba5c3fd87e246ac0196ba8aae22339d168`). Local copy under git-ignored `_refs/macos-10.13.6-fw/` (recipe in `_refs/README.md`). Nothing was loaded or run on the build PC. Apple's public source has **no** `AppleFWOHCI` (GitHub `apple-oss-distributions/AppleFWOHCI` = 404), so the binary is the only primary source; `IOFireWireFamily` source (`_refs/IOFireWireFamily`, commit `ad9f879`) gives the `IOFWDCL` field layout.
**Evidence file:** `otool-exports/AppleFWOHCI-5.5.9-dcl-excerpts.asm` (translateLegacyProgram, SendDCL/ReceiveDCL compile, interrupt).

## Answer

**The per-segment interrupt comes from a *callback* on the transfer DCL, not from the timestamp node.** On 10.13.6 the kernel translates the legacy (classic) DCL program the kext hands to `createLocalIsochPort` into `IOFWDCL` objects (`AppleFWOHCI_DCLProgram::translateLegacyProgram @0x8d5e`). In that translation:

| Legacy opcode (IOFireWireFamilyCommon.h: `kDCL*Op`) | Case target (jump table @0x9284) | Effect (asm) |
|---|---|---|
| 8 `CallProc` | `0x8f69` | if a previous transfer DCL exists (`r12 != 0`) and it has **no callback yet** (`getCallback()==0`): `setCallback(op+0x18)`, `setRefcon(op)`; the CallProc is **folded into the preceding transfer DCL** — no node of its own. With no preceding transfer DCL it is dropped. |
| 12 `UpdateDCLList` | `0x8fe3` | if previous DCL exists and has no callback: `flags |= 0x4` (`kUpdateBeforeCallback`, `IOFWDCL.h`), update list built at `0x9180..0x9212` |
| 13 `TimeStamp` / 14 `PtrTimeStamp` | `0x900f` / `0x901d` | `setTimeStampPtr()` on the **previous transfer DCL** (folded; no interrupt effect) |
| 9 `Label` / 11 `SetTagSync` | `0x8fa2` / `0x8fbf` | label bookkeeping / `setCurrentTagAndSync(sync, tag)` — confirms Label = 9, SetTagSync = 11 in the 10.13.6 binary itself |

Then the descriptor compile decides the OHCI interrupt control field:

* `AppleFWOHCI_SendDCL::compile @0xd3da`: for the `OUTPUT_LAST_IMMEDIATE`-style header word (`0x120c0008` / `0x1a0c0008`) the `0x300000` interrupt bits are OR-ed in **iff `[dcl+0x18]` (fCallback) ≠ 0** (`0xd53f..0xd575`). For the `OUTPUT_LAST` word (`0x180c0000`), `0x300000` is OR-ed when `cl != 0`; `cl` starts 1 and is cleared only when the DCL has **no** callback (`+0x18`), update list (`+0x38`), skip-callback (`+0x80`) or skip-branch (`+0x78`) **and** has a timestamp pointer (`+0x20`) or status pointer (`+0x48`) (`0xd5dd..0xd60e`). So a timestamp pointer alone *suppresses* the interrupt bits; a callback sets them.
* `AppleFWOHCI_ReceiveDCL::compile @0xcc2c`: `r10b = (fCallback != 0)`; `0x300000` is OR-ed iff `r10b` (`0xcce7..0xceac`). Update list / status ptr only select the `0x380c0000` vs `0x300c0000` key.
* Runtime side: `SendDCL::interrupt @0xd89a` runs the update list first when `flags & 4`, then calls the callback `[+0x18](refcon [+0x50])` (`0xd8c2..0xd902`); `checkForInterrupt @0xd85c` polls the descriptor status word.

`IOFWDCL` field offsets used above are from `_refs/IOFireWireFamily/.../IOFWDCL.h` (fBranch 0x10, fCallback 0x18, fTimeStampPtr 0x20, fUpdateList 0x38, fUserStatusPtr 0x48, fRefcon 0x50, fFlags 0x58, fLoLevel 0x60) and match `interrupt()`'s accesses.

## Corrections this forces

* C2:242,333 / C4:133-135,478 / D:175 ("the `DCLPtrTimeStamp` node makes OHCI raise the interrupt") is **wrong** for 10.13.6: the timestamp op only attaches `fTimeStampPtr` to the preceding transfer DCL; the interrupt bits come from the **CallProc-derived callback**.
* Q1:177,187 ("sets the OHCI `s` bit on `kDCLCallProcOp` nodes") is right in substance (CallProc → interrupt) but names the wrong mechanism/bit: the descriptor control field is `0x300000` (bits 21:20 set) via the folded callback. The exact OHCI-spec name of that field (`i`, interrupt control) is **unverified** here (check = OHCI 1.1 §7 descriptor layout, not in the repo).
* "Without the timestamp node, end-of-segment callbacks never fire" (CLAUDE.md, Q-DCL root-cause phrasing) is **not supported**. **Hypothesis (needs hardware test):** the pre-Ghidra clone's TX silence may instead come from a CallProc placed where no transfer DCL precedes it (dropped silently, `0x8f69`) or from the interrupt bits being suppressed. Cannot be settled without running a program on 10.13.6 (blocked by the hard rule).

## Implications for the clone (reasoning, not observation)

* Keep the reference segment shape (…transfers → PtrTimeStamp → UpdateDCLList → CallProc → Jump, Q1/Q1a) so that on 10.13.6 the CallProc folds into the last transfer DCL and `UpdateDCLList` sets `kUpdateBeforeCallback` first. Order matters: `UpdateDCLList` must precede `CallProc` or the flag `0x4` is not set (both handlers test `getCallback()==0`). *Node order in the reference segment is from Q1/Q1a and was not re-derived here.*
* Never emit a CallProc without a preceding transfer DCL.

## Still open after this item

* Whether SL (10.6.8) `AppleFWOHCI` translates identically — **unverified** (the SL rig has the binary; same analysis possible). Irrelevant to the 10.13.6 target.
* Live confirmation (interrupt cadence) — blocked by "no experiments on the build PC".
