# Q2 — Decode `SetupCIP` + `CalcIsochPacketHeaders` — SYT interval / FDF per-rate ground truth

> **Correction (2026-10-15, M-B / R41):** `OutputDCLCallback` is at **`0x263c2`** (`0x26269` was a line number in `mafw.asm`), and the open "which SYT lives in which packet" question is answered in [`Q-MB-dcl-callbacks.md`](Q-MB-dcl-callbacks.md) §3: free-running mode uses the cadence accumulators `(1216, 1220)` for data/NODATA and `(1200, 1204)` to spread the SYT increment (`sytIncrement` or `+1`); external-sync mode replays the receiver's SYT table. `SetupCIP` header 1 uses the **local** node ID. `+0x4c8/+0x4cc` are ceilings of `(B<<11)/((B−A)·SYT)` and `(B<<12)/…` (48 k: 683 cycles of presentation offset).


**Status:** done.

**Scope:** `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` offsets `0x27102` (`FWDCLProgram::SetupCIP`), `0x272f6` (per-rate dispatch that calls SetupCIP), `0x251a0` (`FWDCLProgram::CalcIsochPacketHeaders`). Cross-checked against `openaudio-fw1814/kext/include/FW1814Rates.h`.

## Answer in one line

**The reference maintains a 7-entry per-rate table that maps sample-rate → `{SYTInterval, SYTIncrement, FDFValue}` and a 7-entry per-FDF table that maps FDF → `{sfc_quotient, sfc_divisor, cip_accum_num, cip_accum_denom}`.** Both tables are complete here. CIP headers are written big-endian via byte-swap on every packet; FDF/SYT fields come from the FDF-indexed table.

## Per-rate dispatch (@ 0x272f6, caller of SetupCIP)

Switch on `esi` = sample rate in Hz. Each case loads `(SYTInterval=edx, SYTIncrement=ecx, FDFIndex=r8d)` and calls `SetupCIP`.

| Rate (Hz) | hex | SYTInterval | SYTIncrement | FDF index |
|---|---|---|---|---|
| 32000  | 0x7D00  | **8** | **0x1800** | **0** |
| 44100  | 0xAC44  | **8** | **0x116A** | **1** (default fallthrough) |
| 48000  | 0xBB80  | **8** | **0x1000** | **2** |
| 88200  | 0x15888 | **0x10 (16)** | **0x116A** | **3** |
| 96000  | 0x17700 | **0x10 (16)** | **0x1000** | **4** |
| 176400 | 0x2B110 | **0x20 (32)** | **0x116A** | **5** |
| 192000 | 0x2EE00 | **0x20 (32)** | **0x1000** | **6** |

**Fallback behavior:** any rate not in the table falls through to the 44.1k path (`$0xac44` default at 0x2738b). The clone's `FW1814Rates.h` must mirror this fallback.

## `SetupCIP(numChannels, SYTInterval, SYTIncrement, FDFValue)` @ 0x27102

Field writes to `this` (= FWDCLProgram instance):

| Offset | Field (inferred) | Source arg | Semantic |
|---|---|---|---|
| +0x4a0 | ~~`numChannels`~~ **sample rate in Hz** | esi | **Correction (2026-10-14, from Q-DCL):** the dispatcher at `0x272f6` switches on `esi` = sample rate in Hz and passes it straight through, so `+0x4a0` stores the rate, not the AM824 channel count. |
| +0x4a4 | `fdfValue`    | ebx (r8d) | FDF index (0..6) |
| +0x4a8 | `sytIncrement`| ecx | per-packet SYT accumulator step |
| +0x4ac | `sytInterval` | r8d (edx) | packets-per-SYT-emit (8/16/32) |
| +0x4d0 | *bool flag*   | 0 (always) | likely "first packet / resync" |
| +0x4dc | `3E80000 / (rate_Hz*4) − 2` | computed | cycle budget low watermark. **Correction (2026-10-14):** the divisor is `esi*4` = sample rate × 4 (`0x27116 lea edi,[rsi*4]`), not SYTInterval; see Q-DCL |
| +0x4e0 | `3E80000 / (rate_Hz*4) + 2` | computed | high watermark = TX pkts per bank `P` (Q-DCL); same correction |
| +0x498 | `(hubID<<24) \| ((dbc_start<<16) & 0x3F0000)` | nodeID + packet dim | CIP header-1 template |
| +0x49c | `0x90000000` | const | CIP header-2 base (sync/qpc bits) |

`0x3E80000` = 65,536,000 — looks like cycle-count-per-second × 2 scaling factor used to derive packet-interval watermarks.

The constant `0x90000000` in header-2 is the standard CIP format `0b10010000_...` — tag/channel/tcode bits for AM824 encapsulation.

### Per-FDF table (7-entry switch indexed by `ebx` / FDFValue @ 0x271a0)

Each FDF index writes four moduli at `+0x4b0, 0x4b4, 0x4c0, 0x4c4`. These are the CIP-fractional-accumulator parameters (BeBoB-style SYT math).

Jump table at file offset `0x272d8`, raw-byte decoded (deltas relative to table base):

| FDF | Rate | Handler | +0x4c0 | +0x4c4 | +0x4b0 | +0x4b4 | eax (0x4b8) | ecx (0x4bc) |
|---|---|---|---|---|---|---|---|---|
| 0 | 32k    | A @ 0x271d1 | **1**     | **2**     | **1**     | **1**     | 1     | 2     |
| 1 | 44.1k  | B @ 0x27209 | **0xc7 (199)** | **0x280 (640)** | **0x153 (339)** | **0x1b9 (441)** | 0x153 | 0x280 |
| 2 | 48k    | C @ 0x27246 | **1**     | **4**     | **1**     | **1**     | 1     | 4     |
| 3 | 88.2k  | B @ 0x27209 | 0xc7 | 0x280 | 0x153 | 0x1b9 | 0x153 | 0x280 |
| 4 | 96k    | C @ 0x27246 | 1 | 4 | 1 | 1 | 1 | 4 |
| 5 | 176.4k | B @ 0x27209 | 0xc7 | 0x280 | 0x153 | 0x1b9 | 0x153 | 0x280 |
| 6 | 192k   | C @ 0x27246 | 1 | 4 | 1 | 1 | 1 | 4 |

**Three distinct handler blocks only** — the 44.1k family (FDF 1/3/5) shares constants `{199, 640, 339, 441}`, the 48k family (FDF 2/4/6) shares `{1, 4, 1, 1}`, and 32k gets its own `{1, 2, 1, 1}`. The 0x153/0x1b9 = 339/441 pair are rational approximants for 44.1 kHz SYT synthesis (continued-fraction convergent); 0xc7/0x280 = 199/640 is the complementary pair. The per-rate scaling (double for 88.2k, quadruple for 176.4k) is handled by SYTInterval (8/16/32), not by changing these accumulator constants.

### Post-FDF-switch computed fields

```c
int sfc_delta = ecx - edx;                 // e.g. 2 - 1 = 1 for 32k, 0x280 - 0xc7 = 0x1b9 for 48k
int divisor   = sfc_delta * sytInterval;   // CORRECTION 2026-10-14: multiplier is [+0x4ac] = SYTInterval (imul 0x4ac(%r14) at 0x27290), not channel count
uint32_t q    = (ecx * 0x800) / divisor;   // = (ecx << 11) / divisor
if ((ecx * 0x800) % divisor != 0) q++;     // round up
this->syt_quotient_a /* +0x4c8 */ = q;

q = (ecx * 0x1000) / divisor;              // = (ecx << 12) / divisor
if (q*divisor != ecx*0x1000) q++;
this->syt_quotient_b /* +0x4cc */ = q;
```

These are the per-packet SYT-advance / dbc-advance quotients used by the AM824 packer when producing successive packet headers.

## `CalcIsochPacketHeaders(uint32_t *outHdrs, uint32_t dbc, uint32_t sytOrFlags, bool isSyncPacket)` @ 0x251a0

```c
void FWDCLProgram::CalcIsochPacketHeaders(uint32_t *out, uint32_t dbc, uint32_t sytOrFlags, bool sync) {
    // Header 1: byte-swap (hdr1_template | sytOrFlags)
    uint32_t h1 = this->cip_header1_template /* +0x498 */ | sytOrFlags;
    out[0] = __builtin_bswap32(h1);

    uint32_t h2base = this->cip_header2_template /* +0x49c */;  // 0x90000000
    uint32_t fdf_hi = h2base >> 24;                             // low byte in big-endian = 0x90

    if (!sync) {
        // Non-sync packet: write 0xFFFFFF00 | fdf_hi (= NODATA packet)
        out[1] = 0xFFFFFF00 | fdf_hi;
        return;
    }
    // Sync packet: compose SYT + dbc into header-2 as big-endian
    uint32_t fdf_lo = this->fdf_lo /* +0x4a4 shifted */;
    uint32_t h2 = /* complex: swap16(syt_lo+dbc)<<16 | fdf_hi | fdf_lo */;
    out[1] = h2;
}
```

Two paths: **non-sync packet writes NODATA pattern `0xFFFFFF00`** in CIP header-2, **sync packet writes the real SYT**. This matches the AM824 "every N packets carries SYT, rest carry NODATA" pattern driven by `SYTInterval`.

## Clone comparison

`openaudio-fw1814/kext/include/FW1814Rates.h` currently defines the rate list but should mirror the exact table above. Any divergence here means CIP headers will be wrong and the device won't lock — this is a non-negotiable copy.

| Rate | Clone has it? (verify) | Reference values |
|---|---|---|
| 32k | ? | SYT_int=8, SYT_inc=0x1800, FDF=0 |
| 44.1k | yes | SYT_int=8, SYT_inc=0x116A, FDF=1 |
| 48k | yes | SYT_int=8, SYT_inc=0x1000, FDF=2 |
| 88.2k | yes | SYT_int=16, SYT_inc=0x116A, FDF=3 |
| 96k | yes | SYT_int=16, SYT_inc=0x1000, FDF=4 |
| 176.4k | yes | SYT_int=32, SYT_inc=0x116A, FDF=5 |
| 192k | yes | SYT_int=32, SYT_inc=0x1000, FDF=6 |

Clone action: open `FW1814Rates.h` and verify every row above. If any constant disagrees, mirror the reference exactly.

## Follow-up queue impact

- Q2a merged: full FDF jump table decoded above (three shared handlers for 32k / 44.1k-family / 48k-family).
- Follow-up for TX: `OutputDCLCallback` @ 0x26269 drives the "which SYT lives in which packet" decision. Needs its own question if TX stream kick behavior is unclear.

## Provenance

- Offsets: `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice`, `otool -tV` → `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- Rate hex ↔ decimal: verified against `cmpl` immediates at 0x2730b..0x2738b.
- CIP header template `0x90000000`: direct immediate at 0x27195. Matches CIP format: tag=0b01 (CIP), fmt=0b010000 (AM824) — i.e. upper byte `0b10010000`.
- FDF table scoping: 7-entry switch per `cmpl $0x6, %ebx` at 0x271a0.
