# Q1a — Decode `AddTimestampedTransferPackets(stride, N)` + `AddTransferPackets(stride, N)`

> **Correction (2026-10-15, M-B / R41):** `AddSetTagSyncBits(1,0)` is called for the **first primer of every TX bank** (`isLast = (primer == 0)`, asm `0x25b50`), not for a single last segment; see [`Q-MB-dcl-callbacks.md`](Q-MB-dcl-callbacks.md) §7. Builder arrays: the builder records every label and jump in creation order (used by `ModifyJumpLabel`).


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - Opcode numbering: `FWDCLBuilder::AddLabel` emits `$0x9` (@0x6bdd) and `AddSetTagSyncBits` emits `$0xb` (@0x6d34); line 85's '9 | SetTagSyncBits' is swapped. SDK-header cross-check **unverified**.


**Status:** done. Opcode-numbering question partially resolved — mapping M-Audio-internal → IOKit DCL opcode is deferred to Q1b (the hand-off step).

**Scope:** `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` offsets `0x6ad8` (`AddTimestampedTransferPackets`), `0x6b5a` (`AddTransferPackets`), `0x68ae` (`AppendDCLCommandList`), `0x665c` (`OpcodeToStructSize`). Cross-checked against `openaudio-fw1814/kext/src/FW1814DCLBuilder.cpp` and `.../src/FW1814DCLInputProgram.cpp`.

## Answer in one line

**`stride` is not a stride — it's the DCL opcode for the packet-body command.** `AddTimestampedTransferPackets(opcode, N)` emits `N × (opcode) + 1 × (opcode 14) + 1 × (opcode 12)` DCL commands and back-patches opcode-12's operands to point at the full group. `AddTransferPackets(opcode, N)` is the same minus the opcode-14 PtrTimeStamp. **The clone's three-step emission (`addTransferPacket × N` → `addPtrTimeStamp` → `addUpdateDCLList(N+1)`) is structurally equivalent** — modulo the opcode value handed to the transfer-packet slot (Q1b resolves).

## What the reference does

### `AddTimestampedTransferPackets(opcode_x, N)` @ 0x6ad8

Pseudocode:

```c
bool AddTimestampedTransferPackets(this, uint32_t opcode_x, uint32_t N) {
    FWDCLCommandListElement elems[3] = {
        { .opcode = opcode_x, .count = N },   // transfer-packet slots
        { .opcode = 14,        .count = 1 },  // PtrTimeStamp (M-Audio internal #14)
        { .opcode = 12,        .count = 1 },  // UpdateDCLList (M-Audio internal #12)
    };
    DCLCommandStruct **list_head = NULL;
    bool ok = AppendDCLCommandList(this, elems, /*count*/ 3, &list_head);
    if (!ok) return false;

    // Walk the returned chain looking for the opcode-12 node and back-patch:
    for (DCLCommandStruct *n = list_head; n; n = n->pNextDCLCommand) {
        if (n->opcode == 12) {
            n->operand_0x18 = list_head;            // dclCommandList pointer
            n->operand_0x20 = N + 1;                // numDCLCommands = N transfers + 1 ts
            return true;
        }
    }
    return false;
}
```

### `AddTransferPackets(opcode_x, N)` @ 0x6b5a

Identical structure, two elements:

```c
bool AddTransferPackets(this, uint32_t opcode_x, uint32_t N) {
    FWDCLCommandListElement elems[2] = {
        { .opcode = opcode_x, .count = N },
        { .opcode = 12,        .count = 1 },  // UpdateDCLList
    };
    // AppendDCLCommandList + back-patch opcode-12 node:
    //   node->operand_0x18 = list_head
    //   node->operand_0x20 = N      (no +1 — no timestamp slot)
    ...
}
```

### `AppendDCLCommandList(elems[], count, &list_head)` @ 0x68ae

Two-pass:
1. **Size-up pass.** For each element, look up `OpcodeToStructSize(opcode)` and total `N_i × size_i` bytes needed. Also tracks the opcode-12 node's byte offset within the arena (that's the offset used by callers to back-patch).
2. **Allocate** a contiguous slab via `TDataBuffer::GrowBufferAndReturnBlockPtr(total_bytes, &base)`.
3. **Fill pass.** For each element, for each count, write:
   - `cmd->pNextDCLCommand = 0` (set to next in next iteration)
   - `cmd->opcode = elem.opcode`
   - Link into the running tail; `cmd->pNextDCLCommand = next_cmd`.
   - Advance `base += struct_size` (per-opcode size).
4. **Write back** `*list_head = base` (first command).

Key: **one element produces `count` consecutive DCLCommand structs of that opcode**, all chained through `pNextDCLCommand`. The clone emits each DCLCommand via a separate builder call; the reference emits a batch.

### `OpcodeToStructSize(opcode)` @ 0x665c — the authoritative size table

Jump-table decoded from the binary at offset `0x6690` (14 int32 entries, each a `rip + offset` branch to one of four handlers):

| opcode | size (bytes) | semantic (from BuildPacketDCLSegment + back-patch usage) |
|---|---|---|
| 1 | 0x28 (40) | TX transfer packet (used with `stride=1` from TX builder) |
| 2 | 0 — invalid | — |
| 3 | 0x28 (40) | TX transfer packet variant (unused by FW1814) |
| 4 | 0 — invalid | — |
| 5 | 0x28 (40) | RX transfer packet (used with `stride=5` from RX builder) |
| 6 | 0x28 (40) | *unused by FW1814 — probably another packet variant* |
| 7 | 0 — invalid | — |
| 8 | 0x28 (40) | **CallProc** — writes `.proc = cb@0x18, .procData = ctx@0x20` (see Q1 §BuildPacketDCLSegment) |
| 9 | 0x18 (24) | **SetTagSyncBits** — see `AddSetTagSyncBits(1, 0)` call in TX bank-0 primer |
| 10 | 0x20 (32) | *unused in observed call sites* |
| 11 | 0x18 (24) | *unused in observed call sites* |
| 12 | 0x28 (40) | **UpdateDCLList** — back-patched: `.dclCommandList@0x18 = head, .numDCLCommands@0x20 = count` |
| 13 | 0x18 (24) | *unused in observed call sites* |
| 14 | 0x20 (32) | **PtrTimeStamp** — operand `.timeStampPtr@0x18` written later |

Four distinct handler bodies, each returning a constant:
- `0x6677` → `0x28` (40)
- `0x667e` → `0x18` (24)
- `0x6685` → `0x20` (32)
- `0x668c` → `0` (invalid)

**M-Audio uses its own internal opcode numbering 1..14 — NOT the standard IOKit DCL opcode enum.** 10.4-era IOKit numbered DCL opcodes 1..12 (`kDCLSendPacketStartOp=1, …, kDCLSkipCycleOp=12`), but the sizes don't match: `kDCLCallProcOp` would be opcode 6 (not 8), `kDCLJumpOp` would be opcode 8 (not 12), etc. The write to `operand_0x18/0x20` at BuildPacketDCLSegment's opcode-8 slot is unambiguous DCLCallProc layout, which means opcode 8 ≠ `kDCLJumpOp` in this numbering. **The builder's internal opcodes are translated to IOKit DCL opcodes at some later stage (candidate: `Disassemble`, `CheckDCLOverrun`, or at `createLocalIsochPort` hand-off — Q1b).**

## What the clone does

`openaudio-fw1814/kext/src/FW1814DCLInputProgram.cpp` lines 73–87 emits:

```c
for (UInt32 k = 0; k < slotsPerSeg; ++k) {
    segCmds[k] = (DCLCommand *)builder.addTransferPacket(buf, bytes, /*talking*/ false);
}
segCmds[slotsPerSeg] = (DCLCommand *)builder.addPtrTimeStamp();
builder.addUpdateDCLList(segCmds, slotsPerSeg + 1);
```

Where `builder.addTransferPacket(_, _, false)` sets `p->opcode = kDCLReceivePacketStartOp` directly on each node (`FW1814DCLBuilder.cpp:54`).

## Delta summary

| Aspect | Reference | Clone | Impact |
|---|---|---|---|
| Packet-group emission shape | N transfer-packet + 1 PtrTimeStamp + 1 UpdateDCLList | N transfer-packet + 1 PtrTimeStamp + 1 UpdateDCLList | **Match** |
| Builder entry point | One call `AddTimestampedTransferPackets(5, N)` | Three calls (`addTransferPacket × N`, `addPtrTimeStamp`, `addUpdateDCLList`) | Cosmetic |
| Transfer-packet opcode on RX | M-Audio internal **5** | IOKit `kDCLReceivePacketStartOp` | **Unresolved — Q1b** |
| Transfer-packet opcode on TX | M-Audio internal **1** | IOKit `kDCLSendPacketStartOp` | **Unresolved — Q1b** |
| PtrTimeStamp opcode | M-Audio internal **14** | IOKit `kDCLPtrTimeStampOp` | **Unresolved — Q1b** |
| UpdateDCLList opcode | M-Audio internal **12** | IOKit `kDCLUpdateDCLListOp` | **Unresolved — Q1b** |
| UpdateDCLList count | `N + 1` (transfers + timestamp) | `slotsPerSeg + 1` (same N + 1) | Match |

The structural shape is correct in the clone. The remaining question is **whether M-Audio's internal opcodes are 1:1 remapped to IOKit opcodes at hand-off, or whether a different wire-format is produced for `IOFireWireController::createLocalIsochPort`.** That hand-off is `CreateIsochChannelAndPorts` and `IOSynchronize` — Q1b territory.

## Why Q1a does not change the Q1 answer

Q1's primary findings stand:
1. 4 segments vs 64 — still the dominant structural bug.
2. 1 jump vs 64 jumps — still the structural bug.
3. `procData = this` vs `procData = {this, segIdx}` tuple — still the resync-ambiguity issue.

Q1a adds a nuance: the clone's intra-segment packet-group emission is architecturally equivalent to the reference's. **Fixing Q1 (ring shape) does NOT require also changing the clone's `addTransferPacket/addPtrTimeStamp/addUpdateDCLList` trio.** That's a positive — one less thing to port.

## Follow-up queue impact

- **Q1b (next):** must also resolve the M-Audio-internal → IOKit opcode mapping. One hypothesis to test when reading `CreateIsochChannelAndPorts` / `IOSynchronize` / `Disassemble`:
  - If M-Audio opcodes are translated inside the builder itself (e.g. `Disassemble` rewrites the arena to the IOKit numbering before pointer hand-off), the clone is already correct — just emits a different internal scaffold.
  - If M-Audio hands its internal opcodes to IOKit directly, the clone would be incompatible — but that's unlikely because IOKit wouldn't accept them.
  - Most likely: M-Audio stores its own numbering in `DCLCommandStruct.opcode` during build, then calls into IOKit which only reads structurally (not by opcode). The real DCL opcode interpretation happens via vtable dispatch (`pNextDCLCommand`, `compilerData`, `opcode` are the only fields IOKit touches generically; the rest is opcode-specific). **Test this by looking for `cmd->opcode = ...` writes anywhere in the hand-off path — if none, M-Audio is treating opcodes opaquely.**
- The 14-slot opcode table rules out M-Audio using the 10.7+ IOKit numbering (which starts at 20). Confirms 10.4-era base, with M-Audio-local additions in slots 13/14.

## Provenance

- Offsets: `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` disassembly captured 2026-10-06 via `otool -tV`.
- Jump-table decoded via Python read of raw bytes at file offset 0x6690.
- Clone comparison: `openaudio-fw1814/kext/src/FW1814DCLBuilder.cpp`, `.../src/FW1814DCLInputProgram.cpp` as of 2026-10-06.
