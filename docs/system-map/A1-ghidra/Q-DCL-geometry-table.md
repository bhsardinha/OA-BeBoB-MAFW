# Q-DCL — DCL geometry + packet-pool pages per rate

> **Correction (2026-10-15, M-B / R41):** (1) `AddSetTagSyncBits(1,0)` is emitted on the **first primer of every TX bank** (asm `0x25b50..0x25b59`: `isLast = (primer == 0)` for each of the four primer calls), not on a "TX last segment". (2) DCL transfer *k* receives pool packet `(k+1) mod N` (`TPacketIterator::CommonInit(start=1)`). (3) A bank always holds `P` packets but the callback sends only the last `n`: the previous jump is re-targeted past up to 4 primers (`AdjustOutputDCL`). Details and asm: [`Q-MB-dcl-callbacks.md`](Q-MB-dcl-callbacks.md) §3-§4.


> **Correction (2026-10-14, P5 consolidation):** the values `0x620` (TX) and `0x518` (RX) in `m_audio_b_FWDCL{Output,Input}Program::MetaClass` are the **class instance sizes** (the `OSMetaClass` size argument; `FWEngine2::initHardware @0x12622` does `OSObject::operator new(0x620)` / `(0x518)` for the programs), **not** fixed packet-buffer sizes. Therefore the "TX packet buffer size = 1568 bytes / RX = 1304 bytes" statement, the "cipDataQ / cipPadQ / bytes-per-packet" table, the "fixed packet buffer size is intentional slack" observation and the "Pool page allocation per rate" table below are **unsupported and withdrawn**. Packet size is rate/mode dependent: TX `4·(a+m)·SYTInterval + 8`, RX `+12` (Q-CMP-connect §2; confirmed by capture 74: 424-byte TX packets = 8 + 8 events × 13 quadlets). Pool pages follow `⌊[+0x18]/[+0x28]⌋ + 2` (R8). The ring topology, `P` formula and rate/CIP tables above remain valid. See C4 §1, §2.2, §4 and `Q-P1P2-dcl-acceptance-and-pool-memory.md` §3.2 (recomputed pool sizes).

**Status:** done 2026-10-13. Supersedes an earlier draft that had the TX ring topology wrong.

**Scope:** per-rate geometry of the TX + RX classic-DCL programs built by `m_audio_b_FWDCL{Output,Input}Program::BuildPacketDCLSegment`. Each cell cites either a Q-doc or a direct `docs/system-map/A1-ghidra/otool-exports/mafw.asm` offset. Addresses the Q-DCL + Q-POOL items in BUILD-MANUAL §2.B — unblocks M-B.

## Ring topology — the invariants

**TX ring** (`BuildTransmittingDCLProgram @ 0x25ae4`, caller `RebuildDCLProgramCore @ 0x25df4`):

- **4 banks** indexed by `refcon` 0..3. Each bank:
  - 4 × `BuildPacketDCLSegment(talking=0, builder, packetCount=1, &label, refcon=bank, callback=NULL)` — "primer" segs, each with **packetCount=1**.
  - 1 × `BuildPacketDCLSegment(talking=0, builder, packetCount=P−4, &label, refcon=bank, callback=OutputDCLStaticCallback)` — "big" seg, where `P = *(this+0x4e0)` is the rate-dependent pkt-count-per-bank (computed in `SetupCIP`, formula below).
  - 1 × `AddJump` after each bank.
- **Linker (@ 0x25d9d..25dde):** `jumps[k-1].pJumpDCLLabel = labels_from_bank_k` for k=1..4; wraps bank 3 → bank 0.
- **Total TX structure:** 20 segments (4 primers + 1 big per bank × 4 banks) + 4 jumps.
- **Arena size:** `SetBufferSize(0x14000) = 80 KiB` at `0x25b22`.

**RX ring** (`BuildReceivingDCLProgram @ 0x25962`, caller `RebuildDCLProgramCore @ 0x25a86`):

- **64 flat segments.** Each: `BuildPacketDCLSegment(talking=1, builder, packetCount=4, &labels[i], refcon=i, callback=InputDCLStaticCallback)` — `packetCount=4` is the immediate `$0x4, %edx` at `0x25a99`, **fixed at all rates**.
- 1 × `AddJump` per segment.
- Linker (@ 0x25a2f): ring-link each `jumps[i].pJumpDCLLabel = labels[(i+1) & 0x3f]`.
- **Total RX structure:** 64 segments + 64 jumps = 256 packets per ring at every rate.
- **Arena size:** `SetBufferSize(0x10000) = 64 KiB` per Q1:23.

**Per-segment node ordering** (both TX + RX; Q1a authoritative):

```
AddLabel → [TX-last only: AddSetTagSyncBits(1,0)] → N × DCLTransferPacket(opcode=1 TX / 5 RX)
         → DCLPtrTimeStamp (opcode 14) → DCLUpdateDCLList (opcode 12, enumerates N+1 refs)
         → DCLCallProc (opcode 8, if callback != NULL)
```

## TX pkt-count-per-bank formula

**Source:** `FWDCLProgram::SetupCIP @ 0x27102..0x27133`. Decoded instructions:

```
0x27113:  mov  rdi, r14              ; r14 = this
0x27116:  lea  edi, [rsi * 4]        ; edi = arg1 * 4
0x2711d:  mov  eax, 0x3E80000        ; eax = 65,536,000
0x27122:  xor  edx, edx
0x27124:  div  edi                   ; eax = 65_536_000 / (arg1 * 4)
0x27126:  lea  edx, [rax - 2]
0x27129:  mov  [r14 + 0x4dc], edx    ; +0x4dc = quotient − 2  (low watermark)
0x27130:  add  eax, 2
0x27133:  mov  [r14 + 0x4e0], eax    ; +0x4e0 = quotient + 2  ← passed to BuildTx as pkt_count
```

**Correction to Q2 (2026-10-13):** Q2 line 33 labels `rsi` as `numChannels` — **wrong**. The dispatcher at `0x272f6` switches on `rsi = sample rate in Hz` and passes the raw rate directly through to `SetupCIP` as `arg1`. The field at `+0x4a0` therefore stores the raw sample rate, not the AM824 `dbs` channel count. Pending edit to Q2 §Field writes to this.

The TX pkt-count-per-bank is thus:

```
P = ⌊65_536_000 / (rate_hz × 4)⌋ + 2
```

Evaluating over the 7-rate dispatch table (`Q2 §Per-rate dispatch`):

| Rate (Hz) | rate × 4 | 65_536_000 ÷ (rate × 4) | **P** (TX pkts per bank) | TX big-seg pkts (P − 4) | TX total pkts (4 × P) |
|---|---|---|---|---|---|
| 32000  | 128000 | 512.0 | **514** | 510 | 2056 |
| 44100  | 176400 | 371.4… → 371 | **373** | 369 | 1492 |
| 48000  | 192000 | 341.3… → 341 | **343** | 339 | 1372 |
| 88200  | 352800 | 185.7… → 185 | **187** | 183 | 748 |
| 96000  | 384000 | 170.6… → 170 | **172** | 168 | 688 |
| 176400 | 705600 | 92.8…  → 92  | **94**  | 90  | 376  |
| 192000 | 768000 | 85.3…  → 85  | **87**  | 83  | 348  |

Over 8000 isoch cycles/sec, 4 × P packets ≈ 42.8 ms (32k) down to 7.25 ms (192k) of audio-in-flight — matches typical low-latency isoch buffer depths. The sanity check corroborates the formula + decode.

## RX pkt-count (fixed)

RX `pkt_count_per_seg = 4` at all 7 rates (`0x25a99 : movl $0x4, %edx`). RX ring = 64 × 4 = **256 packets** regardless of rate.

## Per-rate CIP geometry (Q2 §Per-rate dispatch)

Mirrored here for the clone's `Fdf.h` + `Cip.h`:

| Rate (Hz) | FDF | SYT-interval | SYT-increment |
|---|---|---|---|
| 32000  | 0 | 8  | 0x1800 |
| 44100  | 1 | 8  | 0x116A |
| 48000  | 2 | 8  | 0x1000 |
| 88200  | 3 | 16 | 0x116A |
| 96000  | 4 | 16 | 0x1000 |
| 176400 | 5 | 32 | 0x116A |
| 192000 | 6 | 32 | 0x1000 |

Fallback for unrecognized rate: fall through to 44.1k (`$0xac44` default at `0x2738b`).

## cipDataQ / cipPadQ / bytes-per-packet

Not surfaced as per-rate immediates in the TX/RX builders because packet size is **fixed-size by DCL metaclass registration**, not by per-rate formula:

- `m_audio_b_FWDCLOutputProgram::MetaClass @ 0x27536`: `movl $0x620, …` — **TX packet buffer size = 0x620 = 1568 bytes**.
- `m_audio_b_FWDCLInputProgram::MetaClass @ 0x274c6`: `movl $0x518, …` — **RX packet buffer size = 0x518 = 1304 bytes**.

AM824 geometry breakdown (`bytes = 8 + 4 × data_blocks × (cipDataQ + cipPadQ)`):

| Rate (Hz) | data-blocks/pkt (rate ÷ 8000) | ch/block (A1-Q15c §channel table) | cipDataQ | cipPadQ | TX bytes/pkt | RX bytes/pkt |
|---|---|---|---|---|---|---|
| 32000  | 4  | 8  | 8  | 0 | 1568 (hdr 8 + 4×4×8×1 + slack) | 1304 |
| 44100  | 5  | 8  | 8  | 0 | 1568 | 1304 |
| 48000  | 6  | 8  | 8  | 0 | 1568 | 1304 |
| 88200  | 11 | 16 | 16 | 0 | 1568 | 1304 |
| 96000  | 12 | 16 | 16 | 0 | 1568 | 1304 |
| 176400 | 22 | 32 | 32 | 0 | 1568 | 1304 |
| 192000 | 24 | 32 | 32 | 0 | 1568 | 1304 |

**Observation — fixed packet buffer size is intentional slack.** At 192k with 24 data-blocks × 32 cipDataQ × 4 bytes = 3072 B payload — exceeds the 1568 B TX buffer. This means at ≥88k the reference **cannot be stuffing all data-blocks of a frame into one packet**; AM824 at high rates uses multi-packet-per-frame with incremental DBC. The reference's fixed 1568 B TX / 1304 B RX size is the per-isoch-packet buffer, with the DMA iterating sub-packets inside each 125 µs cycle. **Verify at M-B time** by observing OHCI IT descriptor count per DCL node — if >1, that's the sub-packet count. Flagged as follow-up.

## Pool page allocation per rate (Q-POOL)

**Source:** `TPacketPool::Initialize @ 0xa3e6` (Q17 §Pool allocation).

```
numPages = ⌈totalPackets / packetsPerPage⌉ + 2
packetsPerPage = ⌊4096 / packet_bytes⌋
packet_bytes = fixed 1568 TX / 1304 RX (from metaclass, above)
```

Both are **constant across all 7 rates** because `packet_bytes` is fixed:

- TX: `packetsPerPage = ⌊4096 / 1568⌋ = 2`. Allocates `⌈totalTx / 2⌉ + 2` pages.
- RX: `packetsPerPage = ⌊4096 / 1304⌋ = 3`. Allocates `⌈256 / 3⌉ + 2 = 86 + 2 = 88` pages (all rates).

| Rate (Hz) | TX total pkts | TX pool pages | RX total pkts | RX pool pages |
|---|---|---|---|---|
| 32000  | 2056 | 1030 | 256 | 88 |
| 44100  | 1492 | 748  | 256 | 88 |
| 48000  | 1372 | 688  | 256 | 88 |
| 88200  | 748  | 376  | 256 | 88 |
| 96000  | 688  | 346  | 256 | 88 |
| 176400 | 376  | 190  | 256 | 88 |
| 192000 | 348  | 176  | 256 | 88 |

(Clone implementations may choose to pre-allocate the 32k pool size for all rates, which handles rate-change without re-allocating — simpler but wastes memory. Reference re-allocates on rate change per Q15c `PerformSampleRateChangeCore`.)

## AM824 silence pre-fill (Q17 §ZeroElements)

Fixed, rate-independent:

- Data region: `0x000000CF` per quadlet — AM824 vendor label `0xCF` (data) + 24-bit zero sample (`Correction block` in Q17-tpacketpool.md).
- Pad region: `0x00000080` per quadlet — AM824 `NODATA` label `0x80` + 24-bit zero.
- **PLL idle-lock mechanism** per memory `project_fw1814_idle_lock_mechanism.md`: continuous `0xCF` silence TX is what the FW1814 PLL locks against. Clone must mirror.

## Non-blocking follow-ups

1. **High-rate sub-packet math (88k+).** Fixed 1568 TX / 1304 RX buffer vs. computed per-frame payload ≥ 3 KB at 192k means multi-sub-packet-per-DCL-node. Verify from OHCI IT descriptor granularity at M-B.
2. ~~**Correction propagation.**~~ **Done 2026-10-14** — Q2 line 33 and the §Post-FDF-switch formula now carry Correction notes.
3. **TX arena headroom check.** For 32k (worst case): 2056 DCLTransferPacket (40 B) + 20 DCLLabel (16 B) + 24 DCLUpdateDCLList (32 B) + 24 DCLPtrTimeStamp (32 B) + 24 DCLCallProc (32 B) + 4 DCLJump (24 B) ≈ 85 480 B — overshoots the 0x14000 = 81 920 B arena. **This may be a reference bug** triggering `TDataBuffer::GrowBufferAndReturnBlockPtr` to realloc, or an incorrect struct-size estimate on my side. Verify at M-B by instrumenting the clone's builder arena with `os_signpost` and watching for grow events on 32 kHz construction.

## Provenance

- TX ring 4-banks topology: `BuildTransmittingDCLProgram @ docs/system-map/A1-ghidra/otool-exports/mafw.asm:45624..45814` (0x25ae4..0x25df3), four 4-iter loops at `cmpl $0x4, %r13d` (0x25b8c, 0x25c20, 0x25cb7, 0x25d51) + four big-seg calls with `ebx = P−4` (0x25bb7, 0x25c4b, 0x25ce5, 0x25d7f) + four `AddJump` (0x25bd0, 0x25c64, 0x25cfe, 0x25d94) + four linker stores (0x25da8, 0x25dba, 0x25dcc, 0x25ddb).
- RX ring 64-flat topology: `BuildReceivingDCLProgram` + ring-linker — authoritative decode in Q1 §BuildReceivingDCLProgram (docs/system-map/A1-ghidra/otool-exports/mafw.asm:45509..0x25a53).
- RX pkt_count=4 constant: `movl $0x4, %edx` at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:45595` (0x25a99), just before the call to BuildReceivingDCLProgram at `0x25aa1`.
- TX pkt_count formula: SetupCIP @ `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47147..47163` (0x27102..0x27133).
- TX arena 0x14000: `SetBufferSize` immediate at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:45643` (0x25b22). RX arena 0x10000: Q1 §BuildReceivingDCLProgram line 23.
- TX/RX packet buffer size (0x620/0x518): metaclass constructors `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47536` + `0x274c6`.
- `AppendDCLCommandList` emits `count` consecutive DCLCommand nodes per element: inner loop `cmpl (%r15), %r10d; jb 0x69f8` at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:6547..6548` (0x6a37..0x6a3a).
- Pool page formula + packet_bytes semantics: `Q17-tpacketpool.md` §Pool allocation (`inTaskWithPhysicalMask(kernel_task, numPages × 4096, physMask & ~0xFFF)` at kext @ 0xa3e6).
- Per-rate CIP table: `Q2-setupcip-rate-table.md` §Per-rate dispatch (dispatch @ 0x272f6).
- Per-rate channels/block table: `Q15c-changerate-performformat.md` §channel-count derivation (0x479e..0x4194).
- AM824 silence constants (`0xCF` data, `0x80` pad): Q17 §Correction block (kext @ 0xa6da `movl $0xcf, (%r8)` + @ 0xa701 `movl $0x80, (%rdi)`).
