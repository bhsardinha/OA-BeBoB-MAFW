# Q-P1/P2 — Will 10.13.6 accept the clone's classic DCL chain, and what may the packet pool allocate?

**Status:** done 2026-10-14 (BUILD-PLAN tasks P1 and P2; static only, dev Mac; evidence entry R36). Bundles: kext 1.11.3 (`otool-exports/mafw.asm`); 10.13.6 = `_refs/macos-10.13.6-fw/` — `fam.asm` (IOFireWireFamily 4.7.2), `AppleFWOHCI.asm` (5.5.9), `kernel.asm`/`kernel.nm` (xnu-4570). Source for shape only: `_refs/IOFireWireFamily` is **4.8.3** (not what runs on 10.13.6). **This is a static reading: it shows what the code requires, not that an allocation or an `init` succeeds at run time** (that needs the first hardware load, M-B).

## 1. Answer in two lines

**P1.** The classic path `createLocalIsochPort → AppleFWOHCI::createDCLProgram → AppleFWOHCI_DCLProgram::init` exists in 10.13.6 and its acceptance test is `IODCLProgram::generateBufferMap`. The reference chain satisfies that test **by construction**, because every data buffer *and* every timestamp slot lives inside the single `TPacketPool` allocation. The clone must build its chain the same way.
**P2.** The pool is allocated with `inTaskWithPhysicalMask(kernel_task, 0, pages·4096, 0xFFFFF000)`; in the 10.13.6 kernel that is a **wired, virtually contiguous, 4 KiB-aligned block of pages below 4 GiB that is *not* physically contiguous**. The family source recommends exactly this recipe. Sizes are small (≈ 0.1–0.7 MB per pool), so there is no known size problem.

---

## 2. P1 — `generateBufferMap` (10.13.6, `fam.asm` @`0x17396`)

`createLocalIsochPort(talking, opcodes, info = NULL, 0, 0, 0)` → `fFWIM->createDCLProgram(...)` (source `IOFireWireController.cpp:6166`) → `AppleFWOHCI::createDCLProgram @0x75a4` allocates `AppleFWOHCI_DCLProgram` (or `_U` when `[+0x1b8] != 0`) and calls `init(OHCI&, talking, opcodes, info, startEvent, startState, startMask)` (`@0x89d0`):

1. `super::init(info)` (`IODCLProgram::init @0x175ce`: allocates the 16-byte expansion block; for `info == NULL` nothing else).
2. `if (getBufferMap() == NULL) generateBufferMap(opcodes)`; **if the map is still NULL, `init` returns false** (`0x8a20..0x8a2b` → `0x8b49`) → `createDCLProgram` releases and returns NULL → `CreateIsochChannelAndPorts` returns `0xE00002BE` (`kIOReturnNoResources`-class error, kext `0x25637`).
3. `IOLockAlloc` (NULL → fail); with `info == NULL` it takes the kernel-task branch: `[+0xe0] = 1`, creates `AppleFWOHCI_CallbackEventSource(staticHandleInterrupt)` and a "context died" event source and adds both to the OHCI workloop (`[+0xc0]`); failures log `"ERROR: couldn't create event source"` / `"…context died event source"` and return false.
4. `translateLegacyProgram(opcodes)` (`@0x8d5e`) converts the legacy chain into the internal DCL objects via the OHCI's DCL pool (`[ohci+0x58]->vt[0x960]`); it contains no validation errors or log strings — it returns NULL only if the pool is NULL, and a DCL-creation failure simply yields NULL objects (`init` then fails).

**`generateBufferMap` algorithm (binary, matches source structurally).** Walk the chain through `pNextDCLCommand` (offset +0): for each DCL with opcode in {1, 3, 5, 6} (`kDCLSendPacketStart`, `kDCLSendPacket`, `kDCLReceivePacketStart`, `kDCLReceivePacket`; mask `0x6a`) take `buffer = [dcl+0x18]`, `size = (u32)[dcl+0x20]`; for opcode 14 (`kDCLPtrTimeStamp`) take `timeStampPtr = [dcl+0x18]`, size 4. `low = min(trunc_page(addr))`, `high = max(round_page(addr+size))`. If `low == 0` → return (no map). Otherwise `IOMemoryDescriptor::withAddress(low, high−low, 3 /*InOut*/)`, `prepare()` (failure logs `"ERROR: couldn't prepare memory descriptor"`), then `IODMACommand::withSpecification(outputHost32, 32 address bits, maxSegment = len, options 0x40, maxTransfer = len, align 0, mapper NULL)` and `setMemoryDescriptor(desc, autoPrepare = 1)` (failure logs `"ERROR: couldn't prepare memory DMA command"`); on success `fExpansionData->fDMACommand = cmd` and `fBufferMem = desc->map()` (`[this+0x30]`).
*Differences from the 4.8.3 source:* the binary passes **mapper = NULL** (source uses `copyMapper()`), does not check the `map()` result, and the mapping-option constant is `0x40`.
*Cross-check of opcode numbering:* the binary's opcode values 1/5/14 are the same numbers the kext's builder emits (transfer 1 TX / 5 RX, PtrTimeStamp 14; Q1a), confirming Q1b's "builder numbering = IOKit numbering" for these three ops (labels/jumps/tag-sync remain R5's open SDK-header cross-check).

**Where the reference's pointers point (kext).** `FWDCLProgram::SetPacketSize @0x25e56`: panic if the stream is running; `ReleaseTalker`; `RebuildDCLProgram` (frees the old `TDataBuffer`, builds new DCL commands); `TPacketPool::Invalidate()` + `TPacketPool::Initialize(device, count, packetSize, audioQ, midiQ, NULL)`; then it **walks the new chain**: for opcodes 1/5 it sets `dcl->buffer = TPacketForwardIterator::NextPacket()` and `dcl->size = packetSize` (panics `"(dclPacket->buffer == NULL) at index %u"`/`"dclPacket == NULL (%u)"` otherwise) and calls `CalcIsochPacketHeaders`; for opcode 14 it sets `timeStampPtr = pool.base([+0x28]) + (pages([+0x3c]) << 12) − 0x1000 + idx·4` — i.e. **timestamp slots are 32-bit words in the last page of the pool allocation**. The DCL *command structures* (builder arena: 64 KiB RX, 80 KiB TX, `TDataBuffer`) are not buffers and are not part of the span. So `[low, high)` = the pool's pages exactly.

### P1 constraints for the clone's chain
1. Every transfer DCL must have a **non-NULL kernel virtual `buffer`** and a size; `low == 0` silently disables the map and `init` fails.
2. Every `kDCLPtrTimeStamp` must have a **non-NULL `timeStampPtr`** (4 bytes) in the same span.
3. All buffers and timestamp slots must lie inside **one contiguous kernel virtual range that is fully mapped** (the descriptor is built over `[trunc_page(min), round_page(max))`, and `prepare()` wires every page of it). Put them in one allocation — the packet pool, with the timestamp slots in its last page — and keep the DCL command arena outside it.
4. The span must be DMA-addressable with **32 address bits** (below 4 GiB or via a mapper) — satisfied by §3.
5. Chain: linked by `pNextDCLCommand` from the head passed as `opcodes`; `info == NULL`; classic 7-argument call exactly as the reference (Q1b). Node order per segment as C4 §3. Only the callback-carrying transfer DCL gets interrupt bits (Q-OHCI-interrupt), so the CallProc node must be present on the segments that need an interrupt.
6. Order of operations: allocate the pool **before** assigning buffers; `SetPacketSize` must not run while streaming; on a rate change the reference rebuilds the DCL program **and** re-initialises the pool (so the span changes) — keep that order.

**What P1 does not settle.** `translateLegacyProgram` was only structurally scanned (opcode jump table, flag setting `0x40000`/`0x80000`, branch/update-list handling); I did not decode each case. `allocateHW`/`compile` (the later steps that touch real hardware descriptors) were not read in full. Whether `IODMACommand::withSpecification(…, mapper NULL …)` succeeds on the build PC (an iMac11,2: VT-d/system-mapper presence not recorded) is a run-time fact. So: **acceptance is plausible and the clone's job is to reproduce the reference's layout; it remains unproven until M-B.**

---

## 3. P2 — what `inTaskWithPhysicalMask` does with the pool's arguments

**Kext call** (`TPacketPool::Initialize @0xa3e6`): `mask = (bus->[+0x88])->vt[0x140]() & ~0xFFF`; `inTaskWithPhysicalMask(kernel_task, /*options*/0, pages << 12, mask)`; then `prepare(0)` and `getBytesNoCopy()` (`vt[0x2e0]`) → `[pool+0x10]` (virtual base). The bus method at `vt[0x140]` is the controller's physical **buffer/address mask**; 10.13.6 returns `0xFFFFFFFF` for both `getFireWirePhysicalBufferMask` and `getFireWirePhysicalAddressMask` (`fam.asm` `0x28f6`, `0x28de`), so **mask = `0xFFFFF000`**. The family's own code does the same: `IOFWAsyncStreamReceiver.cpp:82` (`mask = getFireWirePhysicalBufferMask() & ~(PAGE_SIZE−1); inTaskWithPhysicalMask(kernel_task, 0, size, mask)`), and `IOFireWireBus.h:315-317` documents the call as intended for `inTaskWithPhysicalMask()` ("all current hardware is 32 bit").

**Kernel path** (xnu-4570, `kernel.asm`): `inTaskWithPhysicalMask @0xffffff800089fc40` allocates the 0xe8-byte descriptor and calls `initWithPhysicalMask(task, options, capacity, alignment = 1, mask)` (vtable `0x240`; `initWithPhysicalMask @0x89eb50`):
- `capacity == 0` → fail.
- Mapper flag: unless `options & 0x800` (`kIOMemoryMapperNone`), it waits for/uses `IOMapper::gSystem`; `mapped = (gSystem != NULL)` (the buffer is `bzero`'d when `mapped` or `options & 0xf0000`).
- With `alignment <= 1` and a non-zero mask: `alignment = (~mask) & (mask − 1)`, `highestMask = mask | alignment`, `alignment++`, minimum `page_size`. For `0xFFFFF000`: alignment `0x1000`, **`highestMask = 0xFFFFFFFF`**.
- Capacity is rounded up to a page; then `IOKernelAllocateWithPhysicalRestrict(round_page(capacity), highestMask, alignment, contiguous)` (`@0x85d4e0`) with `contiguous = 0` for options 0 (only `kIOMemoryPhysicallyContiguous`/host-contiguous options set it).
- In that allocator, with `maxPhys = 0xFFFFFFFF`, `alignment == page_size`, `contiguous == 0`: the contiguous branch is **not** taken (it requires `alignment > page_size` or `contiguous`); since `maxPhys >> 32 == 0` it calls **`kernel_memory_allocate(kernel_map, &addr, size, alignMask = 0xFFF, KMA_LOMEM, tag)`** (`r13d = 8`).
- `KMA_LOMEM` pages come from `vm_page_grablo @0xffffff80003296f0`: if `vm_lopage_needed == 0` it is a plain `vm_page_grab_options`; otherwise it pops `vm_lopage_queue_free` and, when that queue is empty, falls back to `cpm_allocate(0x1000, &page, max_pnum 0xfffff, …)` per page (failure increments `vm_lopages_allocated_cpm_failed` and returns no page). I did not decode `vm_lopage_needed`'s boot-time setting or any quota.

**Result:** a wired, **virtually contiguous** kernel allocation, 4 KiB-aligned, every page below 4 GiB, **not** physically contiguous (which is why `TPacketPool` never lets a packet straddle a page: `packetsPerPage = ⌊4096 / packetBytes⌋`, slack `[+0x34] = 4096 − packetsPerPage·packetBytes`, `GetIndexedPacket(i) = base + (i / packetsPerPage)·4096 + (i % packetsPerPage)·packetBytes`).

### 3.1 Pool geometry formulas (kext `Initialize @0xa3e6`, R8)
`[+0x18] count`, `[+0x20] packetBytes`, `[+0x28] = ⌊4096 / packetBytes⌋`, `pages [+0x24] = ⌊count / [+0x28]⌋ + 2` (integer division, `divl` at `0xa461`), `size = pages · 4096`. `count`: TX `4·P` (`P = ⌊65,536,000/(rate·4)⌋ + 2`), RX `256`. `packetBytes`: TX `8 + 4·DBS·SYTInterval`, RX `12 + 4·DBS·SYTInterval` (Q-CMP-connect §2). `Initialize` fails (and calls `ForceReleaseMemory`) if the audio-quadlet argument is 0, the index allocation fails, the allocation returns NULL, `prepare` fails or `getBytesNoCopy` is NULL.

> **Update (2026-10-14, P4/R38):** the DBS values and channel counts used below are now verified (`m = 1` at `0xed7a`; four `DeviceChannels` tables) and the full per-mode/per-rate table, including the S/PDIF-out cases, is in `Q-P4-cmp-per-rate-and-mixer-power-on.md` §A.2, which supersedes this table.

### 3.2 Resulting sizes — **computed**, not measured
Assumptions (each labelled): channel counts of the capture-72 configuration (`MAClockSource = 0x40000014`: out 12/8/4, in 10/10/2 at 44.1–48 / 88.2–96 / 176.4–192 kHz, C6 §1.1), **one MIDI quadlet per data block** (`m = 1`; consistent with capture 74's DBS 13 = 12 + 1 and macfw's `10+1`/`6+1`), SYT interval 8/16/32, and `P` from `SetupCIP` (Q-DCL). DBS at 176.4/192 kHz is therefore **inferred**. Pool size = `(⌊count/⌊4096/pb⌋⌋ + 2)` pages:

| Rate | TX: DBS / packet B / packets·page / count / pages / KiB | RX: DBS / packet B / packets·page / pages / KiB |
|---|---|---|
| 44.1 kHz | 13 / 424 / 9 / 1492 / 167 / 668 | 11 / 364 / 11 / 25 / 100 |
| 48 kHz | 13 / 424 / 9 / 1372 / 154 / 616 | 11 / 364 / 11 / 25 / 100 |
| 88.2 kHz | 9 / 584 / 7 / 748 / 108 / 432 | 11 / 716 / 5 / 53 / 212 |
| 96 kHz | 9 / 584 / 7 / 688 / 100 / 400 | 11 / 716 / 5 / 53 / 212 |
| 176.4 kHz | 5 / 648 / 6 / 376 / 64 / 256 | 3 / 396 / 10 / 27 / 108 |
| 192 kHz | 5 / 648 / 6 / 348 / 60 / 240 | 3 / 396 / 10 / 27 / 108 |

(These supersede Q-DCL's per-rate page table, which assumed a fixed 1568-byte packet and gave up to 1030 pages; Q-DCL carries a correction.) Capture 74 independently confirms the 48 kHz TX packet size (424 B). The timestamp slots need 4 bytes × (4 TX or 64 RX) inside the last page — no extra allocation.

### 3.3 P2 constraints for the clone
1. Call `inTaskWithPhysicalMask(kernel_task, 0, pages << 12, bus->getFireWirePhysicalBufferMask() & ~(PAGE_SIZE−1))` — **from the bus object, not a hard-coded constant** — and then `prepare()` and `getBytesNoCopy()`; handle every failure (NULL return, `prepare` error) by returning an error up the init path (no `delete(NULL)`, C1 §1).
2. Keep `options = 0` and `alignment = 1` (the mask supplies the 4 KiB alignment); adding contiguity flags would switch to `kmem_alloc_contig` and is **not** what the reference does.
3. Never let a packet cross a page (`packetsPerPage` and slack as §3.1) — the memory is not physically contiguous.
4. Keep the timestamp slots in the pool's last page (P1 constraint 3) and size the pool with the `+2` pages.
5. Sizes in §3.2 are the expected range (≤ ≈ 0.7 MB per pool); no larger request is needed.
6. Re-initialising the pool on a rate change (reference behaviour) frees and re-allocates ≈ 0.1–0.7 MB; an allocation that fails mid-change must leave the engine in a defined stopped state.

**Unverified / for the first load.** Whether `kernel_memory_allocate(KMA_LOMEM)` for these sizes succeeds on the build PC; `vm_lopage_needed` on that machine; whether the system IOMapper exists there (affects the `bzero` and the DMA mapping the `IODMACommand` sees); behaviour under memory pressure. Check: log the NULL/error paths in M-B and read `sysctl vm.*` / `ioreg -c IOMapper` on the build PC (read-only; needs the user's OK).

## Provenance
`fam.asm`: `generateBufferMap @0x17396`, `IODCLProgram::init @0x175ce`, `IODCLProgram::free @0x1768a`, `getFireWirePhysical{Buffer,Address}Mask @0x28f6/0x28de`. `AppleFWOHCI.asm`: `createDCLProgram @0x75a4`, `AppleFWOHCI_DCLProgram::init @0x89d0`, `translateLegacyProgram @0x8d5e`. `kernel.asm`: `inTaskWithPhysicalMask @0x89fc40`, `initWithPhysicalMask @0x89eb50`, `IOKernelAllocateWithPhysicalRestrict @0x85d4e0`, `vm_page_grablo @0x3296f0`. `mafw.asm`: `TPacketPool::Initialize @0xa3e6`, `GetIndexedPacket @0xa5bc`, `FWDCLProgram::SetPacketSize @0x25e56`, `BuildPacketDCLSegment @0x2587a`, `AddTimestampedTransferPackets @0x6ad8`. Sources: `_refs/IOFireWireFamily/IOFireWireFamily.kmodproj/IOFWDCLProgram.cpp`, `IOFireWireController.cpp:6166`, `IOFWAsyncStreamReceiver.cpp:82`, `IOFireWireBus.h:305-340` (all 4.8.3).
