
> **Correction (2026-10-14) — MIDI ring layout** (evidence: [PROVENANCE-RESOLUTIONS.md §R11](../PROVENANCE-RESOLUTIONS.md)). **Also:** 'FW1814's 2in/2out → 4 mappings' is wrong — SL ioreg shows one MIDI in + one out (`MAMIDIInputPorts=("1")`), i.e. 2 mappings (types 0x400 and 0x800). Kernel side, `m_audio_FWb::TKernelMIDIPort::Init @0x1e64a`: one `IOBufferMemoryDescriptor::withOptions(0x10003, 0x2000)` per port (8 KiB, kernel/user shared), stored at `port[+0x20]` (the descriptor `clientMemoryForType` returns). Its base `B` (`getBytesNoCopy`, vt+0x2e0): `port[+0x00] = B` with `*(u32*)B = 0`; `port[+0x10] = B` with `*(u32*)B = 0`, `*(u32*)(B+4) = 0` (ring header {head, tail}); `port[+0x18] = B + 0x808` with `*(u32*)(B+0x808) = *(u32*)(B+0x80c) = 0` (second ring header). So the **timestamped ring is at mapping offset +0x000 and the realtime ring at +0x808**; the `0x10` in `TUserMIDIPort[0x10]`/`TKernelMIDIPort[0x10]` is the *object field index* that stores the base pointer, not a byte offset (C5:269 '+0x000' is right; C3:194 '+0x010' was wrong). Memory types: `clientMemoryForType` (0x7220) serves `0x400+n` from `[eng+0x2b0]` (input ports, `SetMIDIInputPorts @0x5226`, count `[+0x29c]`) with `*options = 0x1000` (kIOMapReadOnly) and `0x800+n` from `[eng+0x2a8]` (output ports, `SetMIDIOutputPorts @0x5160`, count capped at 8 `[+0x2a0]`, writable). Port objects are 40 bytes (`(n<<3)*5`). Default port names are `"1"`, `"2"`, … when none are passed (0x50ae-0x50ca). The panel's meter map option `0x1001` = `kIOMapAnywhere|kIOMapReadOnly` (not 'DefaultCache').


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - Ring offset: lines 23/42 say '+0x010' while C5:269 says '+0x000'; `this[0x10]=ptr` suggests the pointer is stored at +0x10 of the *object*, and the mapping begins at offset 0. Layout of the mapped region is **unverified** (check = decode `clientMemoryForType` consumer in the MIDI plugin).

---
title: A3-M4 — DigiFWMIDIPacket layout + shared-memory ring geometry
part: A3
status: done
date: 2026-10-06
---

# M4 — DigiFWMIDIPacket wire format and the kext↔plugin shared ring

**Headline:** each MIDI port maps a single contiguous shared-memory region via `IOConnectMapMemory`. That region holds **two 128-entry rings of 16-byte `DigiFWMIDIPacket` records** — a *timestamped* ring at ring-base `+0x10` and a *realtime* ring at ring-base `+0x18`, plus a leading byte-stream header at `+0x00`. Memory type selector is `(0x400 for source | 0x800 for destination) + portIndex`.

## 1 — Mapping call

`TUserMIDIPort::Init(TUserMIDIPort *this, uint connection, bool isSource, uint portIdx)` (`a3-m345.log:572–603`, `0x00004d70`):

```c
int memTypeBase = isSource ? 0x400 : 0x800;
IOConnectMapMemory(connection, memTypeBase + portIdx,
                   mach_task_self(), &ptr, &size,
                   kIOMapAnywhere /* =1 */);
this[0x00] = ptr;              // byte-stream base (used by CheckForNewBytes)
this[0x08] = *(uint32_t*)ptr;  // snapshot of initial read-tail
this[0x10] = ptr;              // timestamped-ring base
this[0x18] = ptr + 0x202 * 4;  // realtime-ring base   (+2056 bytes = +0x808)
```

- **`memType` = `(dir<<10) + portIdx`** with `dir=1` for source (RX from device to host) and `dir=2` for destination (TX from host to device). Same encoding family used by panel's meter mapping (A2-P1).
- `kIOMapAnywhere=1` → kernel picks the user VA.
- Only one `IOConnectMapMemory` call per port — the byte stream and both packet rings share one page / mapping.

**Clone implication:** the kext `IOUserClient::clientMemoryForType()` returns four distinct `IOMemoryDescriptor`s per `(dir, portIdx)` tuple, keyed by `0x400|idx` (RX) and `0x800|idx` (TX). For FW1814's 2in/2out, that's 4 mappings total.

## 2 — Region layout

Addresses are offsets from the mapped base pointer `ptr`.

```
+0x000  uint32_t  byteWriteIdx       // producer (kext writes, plugin reads)
+0x004  uint8_t   bytes[0x200]       // 512-byte byte-stream ring (RX only)
            (0x200 = 512 is the max span CheckForNewBytes will drain
             in one call — see a3-m345.log:636 "if (0x200 < uVar5)")
+0x010  <timestamped ring base>      // same as ptr? No — this is an alias stored
            in TUserMIDIPort[0x10], used by the packet APIs. In the mapping,
            the ring **also starts at ptr+0 overlapping**, but the plugin treats
            [0x00]/[0x10]/[0x18] as three handles into one shared page.
            (See §3 for the per-entry decode; the exact base-offset distinction
            between byte-stream and timestamped ring is **not yet confirmed** —
            **hypothesis**: byte ring and timestamped-packet ring are two
            independent regions mapped by two different memTypes, and the
            plugin only stashes both pointers for convenience. Verification:
            trace IOConnectMapMemory call-sites and memType values once we
            instrument the kext side.)
+0x808  <realtime ring base>         // ptr + 0x202*4
```

> **Note on the overlap:** `TUserMIDIPort::Init` assigns `this[0] = this[0x10] = ptr` and `this[0x18] = ptr + 0x202*4`. That strongly suggests **one `IOConnectMapMemory` per port covers both the byte-stream (used by RX only) and the timestamped-packet ring**, with the realtime ring at a fixed offset `+0x808` inside the same mapping. The TX side (`isDest=true`, memType `0x800+idx`) would then alias just the packet rings (byte stream unused). This is **consistent but not fully proven**; see M6 for RX decode cross-check.

## 3 — Ring geometry — common to both rings

Both rings are 128 slots of 16 bytes each = 2048 bytes = 0x800. The ring header (first slot) is used as `{head, tail}` cursors, so the usable data starts at slot index 2 (offset `+2*4 = +8` into the ring).

Each slot (16 bytes) decoded from `send:viaPort:` writes (`a3-m345.log:175–180` for realtime, `log:200–205` for timestamped):

```c
struct DigiFWMIDIPacket {
    uint64_t hostTimeNanos;   // offset +0x00  (8 bytes) — AudioConvertHostTimeToNanos result
    uint32_t length;          // offset +0x08  (4 bytes) — byte count, max 3
    uint8_t  data[4];         // offset +0x0C  (4 bytes) — up to 3 MIDI bytes + 1 pad
};  // sizeof = 0x10
```

And the ring itself:

```c
struct DigiFWMIDIRing {
    uint32_t writeIdx;        // +0x00 — producer increments mod 0x80
    uint32_t readIdx;         // +0x04 — consumer increments mod 0x80
    DigiFWMIDIPacket slot[128];  // +0x08 .. +0x808
};  // sizeof = 0x808
```

- **Mod-128 arithmetic:** `(idx+1) - ((idx+1 + sign>>25) & 0xFFFFFF80)` (`a3-m345.log:179–180`, `log:204–205`, `log:687–688`, `log:717–719`) — all 4 sites do this same wrap, confirming the 128-slot ring size.
- **Full-ring threshold** (`a3-m345.log:162` and `log:187`): if `(write - read) % 0x80 > 0x7D` (= 125 entries queued), stall. So effective capacity is 125 slots; last 3 slots are headroom.

## 4 — Realtime vs timestamped — selection rule (TX side)

From `send:viaPort:` (`a3-m345.log:157`):

```c
uint64_t nowNs = AudioConvertHostTimeToNanos(pkt->timestamp);
if (nowNs < fDestPortCurrentTimestamp[portIdx])
    → push into realtime ring  (TUserMIDIPort[0x18])
else
    → push into timestamped ring  (TUserMIDIPort[0x10])
       and update fDestPortCurrentTimestamp[portIdx] = nowNs
```

Reading paths mirror this:

- `TMIDIPort::CheckForNewRealtimePacket(DigiFWMIDIPacket*&)` (`a3-m345.log:665–691`, `0x00004e8e`): pop from ring `[0x18]`, no timestamp check — delivered as soon as present.
- `TMIDIPort::CheckForNewTimestampedPacket(DigiFWMIDIPacket*&, uint64 base, uint64 window)` (`log:695–722`, `0x00004ed4`): pop from ring `[0x10]`, but only consume if `pkt.hostTimeNanos - base < window`. Otherwise leave it for a later tick.

Both pop methods advance the ring's `readIdx` field (`piVar2 + (long)iVar1 * 4 + 2` = ring-slot pointer; `*(ring+4) = next readIdx mod 128`).

## 5 — Byte-stream ring (RX only)

`TMIDIPort::CheckForNewBytes(TMIDIPort*, MIDIMemVector*, uint32 maxBytes)` (`a3-m345.log:607–661`, `0x00004de8`):

- Reads `writeIdx = *((uint32_t*)this[0])`, compares to local tail `this[8]`.
- Computes `avail = (writeIdx + 1 + ~tail) mod 0x200` (ring of 512 bytes).
- Clamps to 0x200 and to `maxBytes` arg.
- Emits a `MIDIMemVector` with 1 or 2 spans (handles ring wrap).
- Advances tail: `this[8] = writeIdx`.

So **the RX byte stream** is a 512-byte circular buffer of raw MIDI bytes (producer: kext DMA → MIDI parser → push to byte ring; consumer: `TMIDIParser::ReceiveMIDIByte` in M6). **Timestamped and realtime rings on the RX side are separate** — they are built *in the plugin* by `TMIDIParser` as it parses the byte stream, then stuffed into `TMIDIPort[+0x24]`/`[+0x130]`-local MIDIPackets (not the shared-memory rings). The shared rings are TX-only on the output side; RX uses only the byte ring.

**Clone implication for the kext:**
- RX side: expose one `IOMemoryDescriptor` per source port (type `0x400+idx`) laid out as `{uint32 writeIdx; uint8 bytes[512]}` — total 516 bytes rounded up (likely a full page).
- TX side: expose one `IOMemoryDescriptor` per dest port (type `0x800+idx`) laid out as two `DigiFWMIDIRing` structs at offsets +0x000 (timestamped) and +0x808 (realtime). Total ~4 KB — fits one page.

## 6 — Timer interval (why 1500 Hz)

`DAT_00007470 = 0x3f45d867c3ece2a5` → `Double.longBitsToDouble` → `6.666666666666666E-4` seconds = **666.666 µs = 1500 Hz** (`a3-m345b.log:1–2`).

That cadence is tuned to a 24-tick-per-quarter-note MIDI clock at ~250 BPM max, or more simply: at 1500 Hz polling, the worst-case latency from kext RX to CoreMIDI delivery is **~667 µs**, which is below one audio buffer at 512 frames @ 44.1 kHz (≈11.6 ms) by a factor of 17 — well below jitter tolerance.

**No `IOFireWireIsoch`-style timer in the kext for MIDI**: because polling at 1500 Hz is cheap and the shared-memory handoff needs no interrupt, the kext can just stage completed MIDI frames into the ring from the FCP response context and the plugin picks them up on its next tick.

## 7 — Clone implications

1. Kext user client declares four `clientMemoryForType` memTypes per port direction:
   - RX byte ring: `0x400 | portIdx`
   - TX combined (timestamped+realtime): `0x800 | portIdx`
2. The TX region layout is **fixed-size** (two 128-slot × 16-byte rings), so we can allocate `IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, kIOMemoryKernelUserShared, 0x1000)` per TX port.
3. The RX region is **small (~516 B)** but keep it page-aligned (one page per port) for mapping sanity.
4. Mod-128 arithmetic matches the plugin's `(x + sign>>25) & ~0x7F` idiom — kext must use the identical formula so wraparound at index 127→0 matches bit-for-bit.
5. Full-ring threshold at 125 slots must be respected by the kext when it writes into the RX ring (not applicable here since RX writes bytes, not packets) and by whichever TX-side kext consumer reads the ring — if the kext drains at ≥1500 Hz, the plugin's TX backpressure (`usleep(10 000) × 100`) never fires.

## 8 — Open items

- **Hypothesis — needs verification:** the overlap between `this[0]` (byte-stream base) and `this[0x10]` (timestamped-ring base) in `TUserMIDIPort::Init` is intentional aliasing within one mapping — the two live in the *same* memType region, offset `+0x000` and offset `+0x010`? Or does `TUserMIDIPort::Init` only map one region and the "timestamped ring" simply isn't used on RX? Verification: either (a) decode the kext's `clientMemoryForType` once we have ours, or (b) run DTrace on `IOConnectMapMemory` from the plugin on the SL reference Mac.
- **Hypothesis:** 128-slot rings per direction may be insufficient at ~1 Mbit/s MIDI (31.25 kbaud × 32 MIDI inputs = ~1 Mbit/s theoretical). At 1500 Hz drain, 125 slots × 3 bytes = 375 B per 666 µs = ~4.5 Mbit/s drain capacity — comfortable. No concern.
