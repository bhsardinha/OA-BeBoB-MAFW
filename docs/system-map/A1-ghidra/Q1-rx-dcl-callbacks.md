# Q1 — Why do reference RX DCL callbacks fire where ours don't?

> **Correction (2026-10-15, M-B / R41):** the RX segment callback (`InputDCLCallbackCopyPacketGroupToSamples @0x26e3e`) is decoded in [`Q-MB-dcl-callbacks.md`](Q-MB-dcl-callbacks.md) §5: catch-up over missed segments, SYT table (2048 entries) written by the primary program, consumed marker `quad1 = 0x80`, payload copy from `pkt+12` into the shared byte ring, `ResyncInputBuffer`, MIDI hooks. It reads pool packets `4·seg … 4·seg+3` while the segment's own packets are pool `4·seg+1 … 4·seg+4` (pool offset by one), i.e. it never touches the packet in flight.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - **Resolved 2026-10-14 (Q-OHCI-interrupt.md):** Q1:177,187 is right in substance — a `kDCLCallProcOp` is what causes the interrupt — but the mechanism is the kernel's legacy-DCL translation (CallProc → callback on the preceding transfer DCL → `0x300000` interrupt bits in the compiled descriptor), not an 's' bit set by `createLocalIsochPort`. The C2/C4 'timestamp node raises the interrupt' counter-claim is wrong.


> **Correction (2026-10-14) — DCL ring shape.** Per `A1-ghidra/Q-DCL-geometry-table.md` (raw disassembly): **TX** = 4 banks × (4 one-packet primer segments + 1 large segment of `P−4` packets that carries the callback) + 4 jumps = **20 segments** (`BuildTransmittingDCLProgram @ 0x25ae4`; `P = *(this+0x4e0)`, per rate). **RX** = **64 segments × 4 packets** + 64 jumps (`BuildReceivingDCLProgram @ 0x25962`; `packetCount` immediate `$0x4` at `0x25a99`, fixed at all rates). Any "64-segment TX" statement below is wrong; RX 64-segment statements stand. Timing derived from "64 segments × ~1 ms" must be recomputed: 4 packets = 4 isoch cycles ≈ 500 µs per RX segment.

**Status:** done (first-pass answer with concrete deltas). Follow-up Q1a queued for `AddTimestampedTransferPackets` opcode decode.

**Method:** `otool -tV` x86_64 disassembly of `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice`, cross-referenced against `openaudio-fw1814/kext/src/FW1814DCLInputProgram.cpp`.

**Scope:** `m_audio_b_FWDCLInputProgram::BuildReceivingDCLProgram` (0x25962), `Start` (0x2500e), `InputDCLStaticCallback` (0x25abe), `InputDCLCallbackCopyPacketGroupToSamples` (0x26e3e), and the shared helper `m_audio_b_FWDCLProgram::BuildPacketDCLSegment` (0x2587a).

## Answer in one line

**The clone builds a 4-segment / 1-jump DCL ring. The reference builds a 64-segment / 64-jump ring.** OHCI's isoch-receive DMA engine fires callbacks at `kDCLCallProcOp` nodes, but it does so once per *ring-wrap* when the ring is tiny; the clone's ring is coarse enough that callbacks either never fire within useful latency windows or get coalesced to one-per-DMA-buffer. Several secondary deltas listed below compound the problem.

## What the reference actually does

### `BuildReceivingDCLProgram(callback, packet_count)` @ 0x25962

Pseudocode:

```c
IOReturn BuildReceivingDCLProgram(this, void (*cb)(DCLCommandStruct*), uint32_t pkt_count_per_seg) {
    FWDCLBuilder *b = new FWDCLBuilder();   // operator new (sizeof = 0x1b0 = 432 bytes)
    b->SetBufferSize(0x10000);              // 64 KB arena for DCL commands

    DCLLabelStruct *labels[64];
    DCLJumpStruct  *jumps[64];

    for (long i = 0; i < 64; ++i) {
        if (!this->BuildPacketDCLSegment(
                /*isReceive*/ true,         // arg1 == 1
                b,
                pkt_count_per_seg,          // arg3 — count passed in from the caller
                labels[i],
                i,                          // segment index
                cb,                         // callback for EVERY segment
                /*lastSegFlag*/ false)) {   // arg7 == 0
            goto fail;
        }
        if (!b->AddJump(jumps[i])) goto fail;
    }

    // Ring-link: jump[k-1].next = label[k mod 64]
    for (int k = 1; k <= 64; ++k) {
        jumps[k-1]->field_0x18 = labels[k & 0x3f];
    }
    return b;
}
```

Confirmed values:
- `0x428 = 1064` bytes stack frame holds `labels[64]*8 + jumps[64]*8 + slack`.
- Loop bound `cmpl $0x40, %r15d` → 64 iterations.
- Linker loop `cmpq $0x41, %rcx` → iterates `k = 1..64`, wrapping `k=64` back to `labels[0]` via `k & 0x3f`.
- Callback is passed on **every** segment (not just one).

### `BuildPacketDCLSegment(isReceive, builder, pkt_count, label_out, i, cb, lastSegFlag)` @ 0x2587a

What each segment actually contains:

```c
bool BuildPacketDCLSegment(this, bool isReceive, FWDCLBuilder *b, uint32_t pkt_count,
                           DCLLabelStruct *&label_out, long i,
                           void (*cb)(DCLCommandStruct*), bool lastSegFlag)
{
    if (!b->AddLabel(label_out)) return false;

    // SetTagSync only fires on TX bank-0 first primer:
    if (lastSegFlag && !isReceive) {
        b->AddSetTagSyncBits(/*tag*/ 1, /*sync*/ 0);
    }

    uint32_t stride = isReceive ? 5 : 1;          // (!isReceive ? 1 : 5) via leal(1, al*4)

    if (cb != NULL) {
        // RX path — AND TX "big" packet path when callback is wired
        if (!b->AddTimestampedTransferPackets(stride, pkt_count)) return false;
        DCLCommandStruct *dcl = b->AppendDCLCommand(/*op*/ 8, /*size*/ 1);  // op 8 = kDCLCallProcOp
        if (!dcl) return false;

        // Per-segment context lives INLINE in the FWDCLProgram object,
        // at offset 0x98 + i*0x10 (16 bytes: {this, i}):
        *(void**)((char*)this + 0x98 + i*0x10) = this;   // ctx->field_0  = this
        *(long*)((char*)this + 0xa0 + i*0x10)  = i;      // ctx->field_0x8 = segment index

        dcl->proc     = cb;                              // DCLCallProc.proc
        dcl->procData = (void*)((char*)this + 0x98 + i*0x10);  // ptr to {this, i}
    } else {
        // Primer / no-callback path:
        if (!b->AddTransferPackets(stride, pkt_count)) return false;
    }
    return true;
}
```

Key structural facts:
- The callback-site is a *plain* `kDCLCallProcOp` (op 8, size 1 slot) appended **after** the transfer group.
- `procData` is **not** `this` — it's a per-segment `{this, i}` tuple embedded inside the `FWDCLProgram` object. The segment index is carried authoritatively through the callback.
- `AddTimestampedTransferPackets(stride, N)` is used only when a callback is attached. When no callback, `AddTransferPackets(stride, N)` runs instead. **The `stride` arg (RX=5, TX=1) is the same slot-arg both paths use** — it's not an isReceive-vs-isTransmit flag; it looks like a DCL slot stride / UpdateDCLList group size, but exact semantics need Q1a.

### `InputDCLStaticCallback(cmd)` @ 0x25abe

```c
void InputDCLStaticCallback(DCLCommandStruct *cmd) {
    if (cmd == NULL) return;
    void *ctx = cmd->procData;             // = &program[0x98 + i*0x10]
    if (ctx == NULL) return;
    FWDCLInputProgram *self = *(FWDCLInputProgram**)ctx;    // ctx->field_0
    if (self == NULL) return;
    uint32_t segIdx = *(uint32_t*)((char*)ctx + 8);         // ctx->field_0x8
    return self->InputDCLCallbackCopyPacketGroupToSamples(segIdx);  // tailcall
}
```

The segment index is authoritative — supplied by the DMA engine via `procData` at build time, not by any mutable `fReadSegment` member.

### `Start()` @ 0x2500e

```c
IOReturn Start() {
    this->field_0x510 = 0;                    // sample counter reset (int32)
    this->field_0x504 = 0;                    // byte flag reset
    *(uint32_t*)(this->field_0x80 + 0x88) = 0x3f;   // shared-sync->current_seg = 63
    return FWDCLProgram::Start();             // tailcall into base
}
```

Starting value `63` matters: `InputDCLCallbackCopyPacketGroupToSamples` reads this value, computes `expected_next = (current + 1) & 0x3f`, and compares it against the passed-in `segIdx`. If equal → normal advance; if ahead → resync path. Starting at 63 means the *first* callback is expected at `segIdx == 0` — the pre-wrap sentinel pattern.

### `InputDCLCallbackCopyPacketGroupToSamples(segIdx)` @ 0x26e3e (prologue only — full decode is Q4-queue)

```asm
mov 0x80(%rdi), %rax         ; shared_sync = this->field_0x80
mov %rax, -0x38(%rbp)
mov 0x88(%rax), %eax         ; current_seg = shared_sync->current_seg
lea 0x1(%rax), %ecx
and $0x3f, %ecx              ;  ← confirms 64-segment ring (mask = 0x3f)
mov %ecx, -0x5c(%rbp)        ; expected_next = (current + 1) & 63
cmp %esi, %ecx
jne 0x26e75                  ; if (expected != received) resync branch
```

The `and $0x3f` is a tombstone: **this file-authoritative decode confirms the reference uses a 64-segment ring**, matching the `BuildReceivingDCLProgram` loop bound.

## What the clone does (openaudio-fw1814/kext/src/FW1814DCLInputProgram.cpp, lines 40–104)

```c
IOReturn buildSegments(DCLBuilder &builder) {
    const UInt32 slotsPerSeg = fPacketCount / kSegmentCount;    // kSegmentCount = 4
    DCLLabel *segLabel[kSegmentCount];

    UInt32 pkt = 0;
    for (UInt32 s = 0; s < kSegmentCount; ++s) {
        segLabel[s] = builder.addLabel();

        // slotsPerSeg transfer packets
        DCLCommand **segCmds = builder.allocCommandPtrArray(slotsPerSeg + 1);
        for (UInt32 k = 0; k < slotsPerSeg; ++k) {
            segCmds[k] = (DCLCommand*)builder.addTransferPacket(base + pkt*fPacketBytes, fPacketBytes, false);
            ++pkt;
        }
        segCmds[slotsPerSeg] = (DCLCommand*)builder.addPtrTimeStamp();
        builder.addUpdateDCLList(segCmds, slotsPerSeg + 1);

        builder.addCallProc(&InputDCLStaticCallback, /*procData*/ this);
    }

    builder.addJump(segLabel[0]);   // ONE jump total, closing the whole ring
    return kIOReturnSuccess;
}
```

## Delta table — ranked by causal impact on "callbacks never fire"

| # | Reference | Clone | Impact |
|---|---|---|---|
| 1 | **64 segments** | **4 segments** | **Primary.** OHCI IR context issues one callback-interrupt per `kDCLCallProcOp` reached. With 4 segments the DMA engine processes slotsPerSeg (often 256+) packets between callbacks; typical frame budgets never close. Worse: the IOFireWire controller's DCL compiler may coalesce or re-order segment work with so few jumps, so the "callback per segment" invariant breaks. |
| 2 | **64 jumps, one per segment, ring-linked `jump[k-1] → label[(k+1) mod 64]`** | **1 jump total, closing seg[-1] → seg[0]** | **Primary.** Each `kDCLJumpOp` in classic DCL is also a DMA-fetch resync point — without a jump per segment the IR engine runs the whole arena contiguously and only re-fetches at ring wrap. This likely explains why callbacks fire *at best* once per full buffer traversal. |
| 3 | `AddTimestampedTransferPackets(5, N)` as a **single builder op** when callback is present | `addTransferPacket × N → addPtrTimeStamp → addUpdateDCLList(segCmds, N+1)` as **three separate ops** | **Suspected secondary.** May be equivalent but the stride arg `5` (vs implicit alignment in the clone) and the single-op packing suggest the reference's packet group has a different OHCI descriptor shape. Queued as **Q1a**. |
| 4 | `DCLCallProc.procData = &program->(0x98+i*0x10)` (points to `{this, segIdx}`) | `DCLCallProc.procData = this` only; `segIdx` derived from mutable `fReadSegment` member | **Not the "no callbacks" bug** but degrades resync: the clone cannot tell *which* segment fired; the reference can. |
| 5 | `Start()` writes `shared_sync->current_seg = 63` so the first callback expects `seg==0` | Clone resets `fReadSegment = 0`, so the first callback expects `seg==1` — off by one at startup | Cosmetic/edge-case: shows up as a one-time resync trip on the first ring. Not the primary bug. |
| 6 | Signature `BuildReceivingDCLProgram(callback, pkt_count_per_seg)` — callback and packet-count are **parameters** | Hardcoded `InputDCLStaticCallback` and computes `slotsPerSeg = fPacketCount / 4` internally | Cosmetic. Loses the reference's ability to inject a diagnostic callback for M3 bringup. |
| 7 | `AddSetTagSyncBits(1, 0)` is only emitted on TX bank-0 first primer; **RX never emits it** (condition `arg7 && !arg1` false for all RX segs) | Clone's RX also doesn't emit `AddSetTagSyncBits` | **Match.** No action needed. Confirms the OHCI IR context control register alone sets tag/sync for RX. |

## Why "classic DCL + 64 segments" is specifically what High Sierra needs

`IOFireWireController::createLocalIsochPort()` on 10.13 constructs an OHCI IR DMA chain by **walking the DCL program linearly, emitting one OHCI descriptor per DCL command, and setting the OHCI `s` (status) bit on `kDCLCallProcOp` nodes**. The IR context fires an interrupt on each `s`-bit descriptor completion. Fewer `kDCLCallProcOp` nodes → fewer interrupts; too few jumps between them → the OHCI DMA fetches ahead of the kernel, and the IR context stops when its branch pointer falls off the compiled program.

The reference's `64 × (label, timestamped xfers, callproc, jump)` is the shape OHCI IR is designed to run: ~1 ms of audio per segment at 48 kHz, 64 segments ≈ 64 ms ring, callback cadence tracks the OHCI cycle rate. The clone's 4 × (large group + ptrtimestamp + updatelist + callproc) + 1 jump is a very different DMA chain that doesn't match that cadence.

## Recommendations (do NOT implement yet — Part D gate)

1. **Rebuild the RX ring to 64 segments × 64 jumps**, each segment holding `fPacketCount / 64` packets. Match the reference's structural shape literally.
2. **Switch the per-segment callback to a `{this, segIdx}` procData payload** stored inside the DCL program object.
3. **Initialize the shared-sync `current_seg` field to 63 on `Start()`** so the first callback matches.
4. **Resolve Q1a** before trusting the per-segment transfer group — the `AddTimestampedTransferPackets(stride=5, N)` builder op may expand differently than the clone's `addTransferPacket × N / addPtrTimeStamp / addUpdateDCLList(N+1)` triplet.

## Follow-up queue

- **Q1a:** Decompile `m_audio_b_FWDCLBuilder::AddTimestampedTransferPackets(uint stride, uint count)` and `AddTransferPackets(uint, uint)`. Confirm the exact DCL opcode sequence each emits and the meaning of `stride=5` on RX vs `stride=1` on TX.
- **Q1b:** Decompile `m_audio_b_FWDCLProgram::CreateIsochChannelAndPorts(bool, uint, uint)` to confirm how the compiled 64-seg DCL program is handed to `IOFireWireController::createLocalIsochPort` — specifically whether a `DCLTaskInfoAux::bufferMemoryMap` is attached (and if so, how it's built). This is where our clone saw `kIOReturnNoResources`; may be unrelated to Q1 but blocks shipping the fix.
- **Q1c:** Decompile the overrun path (`InputDCLStaticOverrunCallback` + `InputDCLCallbackOverrun`) to understand how the reference recovers from DMA stall. Clone's `fReadSegment` resync is a weaker proxy.

## Provenance

- Binary offsets cited above are from `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` (SHA — pending; captured via `otool -tV` on 2026-10-06).
- Clone comparison file: `openaudio-fw1814/kext/src/FW1814DCLInputProgram.cpp` (status as of 2026-10-06; see git).
- Supersedes the auto-memory entry `reference_dcl_structure_authoritative.md` for the RX side: that memory said "64 single-pkt segs + 64 jumps" — correct on segment/jump count, but conflated "single-pkt" with the actual "packet_count-per-seg = caller-chosen, emitted as a timestamped-transfer-group" layout. Memory to be updated after Q1a resolves `stride=5`.
