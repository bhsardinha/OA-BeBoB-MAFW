# Q1c — Decode RX overrun path (`InputDCLStaticOverrunCallback` + `InputDCLCallbackOverrun`)

> **Correction (2026-10-14) — OHCI interrupt.** Q1's 'RX callbacks never fire' causes: the interrupt is produced only by a transfer DCL that carries a callback (folded CallProc), see Q-OHCI-interrupt.md. A CallProc with no preceding transfer DCL is silently dropped (`translateLegacyProgram @0x8f69`).


**Status:** done.

**Scope:** `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` offsets `0x25340` (`InputDCLStaticOverrunCallback`), `0x2533a` (`InputDCLCallbackOverrun`), `0x67b0` (`FWDCLBuilder::CheckDCLOverrun`), `0x6734` (`FWDCLBuilder::ModifyJumpLabel`), and the per-segment CallProc body (Q1 §`InputDCLStaticCallback` @ 0x25abe).

## Answer in one line

**The reference has no DMA-stall recovery path. `InputDCLStaticOverrunCallback` and `InputDCLCallbackOverrun` are empty stubs.** `CheckDCLOverrun` exists in the builder but **has zero callers in the binary** — it is leftover dead code from an earlier rotating-jump design. The 64-segment ring with 64 pre-wired jumps is self-sustaining: each segment's jump is set at build time to point at its successor, and no runtime jump re-pointing happens. If the DMA ever overran, the overrun callbacks would absorb it silently and the hardware would stop — the reference accepts that as "shouldn't happen, don't recover."

## What the reference does

### `InputDCLStaticOverrunCallback(DCLCommandStruct*)` @ 0x25340

```c
void FWDCLInputProgram::InputDCLStaticOverrunCallback(DCLCommandStruct *) {
    // Empty stub — push rbp / mov rbp,rsp / pop rbp / ret
}
```

Three instructions: prologue, epilogue, return. No body.

### `InputDCLCallbackOverrun()` @ 0x2533a

```c
void FWDCLInputProgram::InputDCLCallbackOverrun() {
    // Empty stub — same three instructions
}
```

### `FWDCLBuilder::CheckDCLOverrun(IOFWLocalIsochPort *port)` @ 0x67b0

```c
void FWDCLBuilder::CheckDCLOverrun(IOFWLocalIsochPort *port) {
    uint32_t total      = this->ring_slot_count;  // [this + 0x1a0] = 64
    uint32_t cursorA_jmp = this->jump_cursor_A;    // [this + 0x1a4]
    uint32_t cursorA_lbl = this->label_cursor_A;   // [this + 0x1a8]
    uint32_t cursorB_lbl = this->label_cursor_B;   // [this + 0x1ac]

    ModifyJumpLabel(port, /*jumpIdx*/ cursorA_jmp, /*labelIdx*/ cursorA_lbl);
    ModifyJumpLabel(port, /*jumpIdx*/ cursorA_lbl, /*labelIdx*/ cursorB_lbl);

    // round-robin advance (modulo total):
    uint32_t next = cursorA_lbl + 1;
    this->label_cursor_A = (next >= total) ? 0 : next;
    next = cursorB_lbl + 1;
    this->label_cursor_B = (next >= total) ? 0 : next;
    next = cursorA_jmp + 1;
    this->jump_cursor_A  = (next >= total) ? 0 : next;
}
```

Decoded fields at builder+0x1a0..0x1ac:
- `+0x1a0`: ring slot count (likely 64 — matches `BuildReceivingDCLProgram`'s loop in Q1).
- `+0x1a4`: jump-cursor advancing one step per call.
- `+0x1a8`: label-cursor advancing one step per call.
- `+0x1ac`: label-cursor advancing one step per call (offset from +0x1a8).

### `FWDCLBuilder::ModifyJumpLabel(port, jumpIdx, labelIdx)` @ 0x6734

```c
void FWDCLBuilder::ModifyJumpLabel(IOFWLocalIsochPort *port, uint32_t jumpIdx, uint32_t labelIdx) {
    TStaticArray<DCLJump*>  &jumps  = this->jumps_array;    // ptr @ +0x138, count @ +0x140
    TStaticArray<DCLLabel*> &labels = this->labels_array;   // ptr @ +0x28,  count @ +0x30
    DCLJump  *jump  = jumps[jumpIdx];     // bounds-checked (returns array[0] if OOB)
    DCLLabel *label = labels[labelIdx];
    jump->operand_0x18 = label;           // DCLJump::pJumpDCLLabel
    if (port) {
        // IOFWLocalIsochPort::notify(kFWDCLModifyNotification=2, &jump, 1)
        port->notify(/*type*/ 2, /*list*/ &jump, /*count*/ 1);  // vtable[0x158]
    }
}
```

Standard 10.4 IOFWLocalIsochPort jump-notify path: patch the DCL node in place, then tell IOKit so OHCI picks up the change on the next DMA fetch.

### Who calls `CheckDCLOverrun`?

```sh
$ grep -c "15CheckDCLOverrun" docs/system-map/A1-ghidra/otool-exports/mafw.asm
1   # only the symbol definition — zero callers
```

**No callers in the entire binary.** Not from `InputDCLCallback`, not from `OutputDCLCallback`, not from `Start`, not from the overrun stubs. The function exists but is dead code.

### How the ring stays running without `CheckDCLOverrun`

The 64 segments each terminate with a jump. `BuildReceivingDCLProgram` (Q1) wires those jumps at build time:
- `jump[i]->pJumpDCLLabel = label[(i+1) & 0x3f]`

The ring is **statically wired** to loop seg 0 → 1 → … → 63 → 0 → … forever. No runtime re-pointing is necessary because the ring is topologically closed from the moment IOKit starts the DMA.

This is distinct from the "two-bank primed, rotate as consumed" pattern used on the TX side (bank-0/bank-1 primers with periodic rotation). RX is single-ring; TX is double-buffered. Only TX would need jump rotation — but even TX's rotation appears to live in `OutputDCLCallback` directly (0x26269) rather than through `CheckDCLOverrun`.

## What the clone does

`openaudio-fw1814/kext/src/FW1814DCLInputProgram.cpp`:
- 4-segment ring (Q1).
- 1 jump total (seg 3 → seg 0).
- No `InputDCLCallbackOverrun` equivalent (likely a stub if present).
- No `CheckDCLOverrun` equivalent (nothing to rotate, by construction).

The clone's RX ring would, like the reference, be self-sustaining — if its CallProc fired. The real bug is the callback-cadence failure from Q1, not an overrun-recovery gap.

## Delta summary

| Aspect | Reference | Clone | Impact |
|---|---|---|---|
| `InputDCLCallbackOverrun` body | empty stub | *not implemented* | **Match** — both no-op |
| Ring topology | 64 segs, pre-wired forward-jumps | 4 segs, 1 pre-wired forward-jump | Q1 (segment count), not overrun |
| Runtime jump rotation | **none** (`CheckDCLOverrun` is dead code) | **none** | Match — neither does rotation |
| TX bank rotation | inside `OutputDCLCallback` (not via builder) | TBD — different question (Q-TX) | Out of scope |
| Overrun recovery strategy | "shouldn't happen; absorb silently" | implicitly same | Match |

## Why Q1c closes the Q1 investigation

Q1 postulated three possible causes for "RX callbacks never fire":
1. Ring too small (confirmed primary — 4 vs 64 segments).
2. `procData` tuple missing (confirmed secondary — `{this, segIdx}` not `this`).
3. Overrun recovery gap (**ruled out here** — reference also has no overrun recovery).

The clone does not need an overrun-recovery port. Fixing (1)+(2) is sufficient. **Overrun stubs can stay empty in the clone**, matching the reference exactly.

## Follow-up queue impact

- **No new follow-ups.** Q1 is now fully closed. The remaining open question — "when exactly does TX bank rotation fire?" — belongs to a separate TX investigation, not Q1.
- **Note on dead code:** `CheckDCLOverrun` and `ModifyJumpLabel` are reachable only from a vtable we haven't found; worth flagging if a future question touches runtime DCL mutation. Clone can ignore both.

## Provenance

- Offsets: `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` via `otool -tV` → `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- Caller count verified: `grep -c "15CheckDCLOverrun" docs/system-map/A1-ghidra/otool-exports/mafw.asm` → `1` (symbol def only).
- vtable[0x158] = `IOFWLocalIsochPort::notify` on 10.13 IOKit — matches 10.4-era signature `(uint32_t type, void **list, uint32_t count)`.
