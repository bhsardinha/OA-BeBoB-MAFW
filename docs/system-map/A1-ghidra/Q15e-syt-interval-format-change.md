# Q15e — SYT-interval table + format-change commit choreography

> **Correction (2026-10-14) — PerformFormatChange choreography** (evidence: [PROVENANCE-RESOLUTIONS.md §R13](../PROVENANCE-RESOLUTIONS.md)). Re-read of `FWEngine2::PerformFormatChange @0x124cc` (asm 0x124cc-0x12618): (1) `vt[0xa50]` on **the engine** (`this`) = `FWBaseEngine::pauseAudioEngine` — not `stream->stopIO()`; the receiver is the engine, called first if `this != NULL`. (2) New format tag `'cac3'` (`0x63616333`): `r14` = entering AC3 (tag is cac3 and `[+0x2d4]==0`), `r12` = leaving AC3 (tag is not cac3 and `[+0x2d4]!=0`). (3) **If `[+0x2d4]==0`** (not currently in AC3) it calls `[this+0x138]->vt[0xa78](rate)` with `esi` = the requested rate (or current `[+0x220]`) — `RequestFormatUpdateForSampleRate` on the device; this is **unconditional on rate change** (Q15e said 'only if rate differs'); **it puts nothing on the wire**: `FWAudioDevice::RequestFormatUpdateForSampleRate @0x2a84` calls `vt[0xa68](rate)` (`GetDeviceChannelsForSampleRate`), scans that table's `[+0x38]` list (8-byte entries, 0-terminated) for the rate index, then tail-calls `FWBaseEngine::SetIOFormat(channels, idx)` on `[this+0xe8]` (0x2a98-0x2ac5) — a pure engine-side IO-format update. (The neighbouring function at 0x2aca is the FDF-changed handler that publishes `MASampleRate`/`MARateLocked`.) (4) Entering AC3: `rate := 48000` (`movl $0xBB80` 0x12580) and `EnableAC3Format()`; leaving: `DisableAC3Format()`. (5) `vt[0xce0]` = `PerformSampleRateChangeCore` is called **only if** the new rate differs from `[+0x220]` or `r12` (leaving AC3). (6) `CompletePendingStateChanges()` then `vt[0xa58]` = `resumeAudioEngine` (always, also on early exits).


**Status:** done 2026-10-14

**Scope:** per-rate SYT_INTERVAL, CIP_INCREMENT, FDFValue enum as used by `SetupCIP` and `SetPacketParameters`; full `PerformFormatChange` call sequence. Sources: `docs/system-map/A1-ghidra/otool-exports/mafw.asm` (`SetPacketParameters` caller at `0x272f3`, `SetupCIP` at `0x27102`, `PerformFormatChange` at `0x124cc`).

---

## 1. Per-rate parameter table

Extracted from `SetPacketParameters` caller at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47263` (`0x272f3`) — the function that maps raw sample-rate Hz to (SYT_INTERVAL, CIP_INCREMENT, FDFValue) before calling `SetupCIP`. The comparisons are on `%esi` = sample_rate_hz:

| Rate (Hz) | FDFValue enum | SYT_INTERVAL | CIP_INCREMENT | `%esi` (hex) | Source offset |
|---|---|---|---|---|---|
| 32 000 | 0 | 8 | 6144 (`0x1800`) | `0x7D00` | `0x27322–0x27336` |
| 44 100 | 1 | 8 | 4458 (`0x116A`) | `0xAC44` | `0x2738b–0x273a0` (fallback/default) |
| 48 000 | 2 | 8 | 4096 (`0x1000`) | `0xBB80` | `0x27349–0x27366` |
| 88 200 | 3 | 16 | 4458 (`0x116A`) | `0x15888` | `0x27373–0x273b7` |
| 96 000 | 4 | 16 | 4096 (`0x1000`) | `0x17700` | `0x2737b–0x273ce` |
| 176 400 | 5 | 32 | 4458 (`0x116A`) | `0x2B110` | `0x27383–0x273e5` |
| 192 000 | 6 | 32 | 4096 (`0x1000`) | `0x2EE00` | `0x273ef–0x27404` |

**Pattern:** SYT_INTERVAL doubles per octave (8→16→32). CIP_INCREMENT is shared within each frequency family (44.1 kHz family = 4458; 48 kHz family = 4096; 32 kHz uniquely 6144).

**Default fallback** (unrecognised rate): 44100 / FDFValue=1 / SYT_INTERVAL=8 / CIP_INCREMENT=4458 (`0x2738b`).

---

## 2. FDFValue enum ↔ FDF byte ↔ CIP+1 byte

`FDFValue` is an internal enum 0..6. The raw FDF byte embedded in the CIP+1 word is written by `SetupCIP @ 0x27102` into `[+0x4a4]` and comes from the `FDFValue` argument. Cross-referencing with the FDF byte table from Q2 (`SetupCIP @ 0x27102` arg `%r8d` path):

| FDFValue enum | FDF byte (CIP+1[7:2]) | Rate |
|---|---|---|
| 0 | `0x03` | 32 kHz |
| 1 | `0x04` | 44.1 kHz |
| 2 | `0x02` | 48 kHz |
| 3 | `0x0A` | 88.2 kHz |
| 4 | `0x05` | 96 kHz |
| 5 | `0x06` | 176.4 kHz |
| 6 | `0x07` | 192 kHz |

**Source:** FDF byte constants from Q2-setupcip-rate-table.md; FDFValue→rate mapping from switch jumptable at `0x271a0` in `SetupCIP`.

---

## 3. SetupCIP per-FDFValue internal fields

`SetupCIP(sample_rate, SYTInterval, SYTIncrement, FDFValue)` at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47147` (`0x27102`), after storing args and computing pkt_count, dispatches on `FDFValue` (0..6) via switch at `0x271a0` to set four SYT-computation fields:

| FDFValue | `[+0x4c0]` | `[+0x4c4]` | `[+0x4b0]` | `[+0x4b4]` |
|---|---|---|---|---|
| 0 (32k) | 1 | 2 | 1 | 1 |
| 1 (44.1k) | 199 (`0xC7`) | 640 (`0x280`) | 339 (`0x153`) | 441 (`0x1B9`) |
| 2 (48k) | 1 | 4 | 1 | 1 |
| 3 (88.2k) | 199 | 640 | 339 | 441 |
| 4 (96k) | 1 | 4 | 1 | 1 |
| 5 (176.4k) | 199 | 640 | 339 | 441 |
| 6 (192k) | 1 | 4 | 1 | 1 |

**Correction (2026-10-14) — cases 3–6 resolved.** The switch at `0x271a0` is `cmp ebx,6; ja 0x271b7` followed by a 7-entry `movslq` jump table at `0x272d8`, decoded from the slice bytes as offsets relative to the table base:

| FDFValue | target | block |
|---|---|---|
| 0 | `0x271d1` | 32k constants `{1, 2, 1, 1}` |
| 1, 3, 5 | `0x27209` | 44.1k-family constants `{0xC7, 0x280, 0x153, 0x1B9}` |
| 2, 4, 6 | `0x27246` | 48k-family constants `{1, 4, 1, 1}` |

FDF > 6 jumps to `0x271b7` (default), which reloads the existing `+0x4c4/+0x4b0/+0x4c0` fields unchanged. All cases then join at `0x2727e`: `+0x4bc = c0`, `+0x4b8 = b0`, `div = (c4 − c0) × [+0x4ac]`, `+0x4c8 = ceil((c4 << 11) / div)`, `+0x4cc = ceil((c4 << 12) / div)` (`0x2728c..0x272c6`). `+0x4ac` is **SYTInterval** (arg 2, `edx` → `r8d` at `0x27110`, stored at `0x27142`), so the per-rate difference at 88.2k–192k comes only from SYTInterval 16/32. The four constants do not change. (The Ghidra decompile labels this `param_3`; raw register flow shows it is the second C++ argument.)

**Note on 44.1 kHz values:** `0x1B9=441` = 44100/100; `0x280=640` = 8 (SYT_INTERVAL) × 80 (beat-period in cycles for 44.1 kHz rational approximation). These are the numerator/denominator of the rational SYT timestamp increment. Verified for FDF=1 at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47202` (`0x27209`). Cases 3–6 not read — **mark as follow-up; read offsets 0x271d1..0x27279 for the remaining 5 cases**.

---

## 4. PerformFormatChange choreography

Function: `FWEngine2::PerformFormatChange(IOAudioStream*, IOAudioStreamFormat*, IOAudioSampleRate*)` @ `docs/system-map/A1-ghidra/otool-exports/mafw.asm:21010` (`0x124cc`)

**Step-by-step sequence** (direct read):

| Step | Offset | Action |
|---|---|---|
| 1 | `0x124ef: callq *0xa50(%rax)` | `stream→stopIO()` — halt CoreAudio delivery before touching hardware |
| 2 | `0x12513–0x1252b` | Check new format tag for AC3 (`0x63616333`): if entering AC3 → `EnableAC3Format()` (`0x12570`); if leaving AC3 → `DisableAC3Format()` (`0x125ab`) |
| 3 | `0x12575: callq *0xa78(%rax)` | `device→SetSampleRate(rate_hz)` — issues AV/C Extended Stream Format INFO CONTROL (0x2F/0xC0) per Q3a §2b; waits for device FDF byte to match |
| 4 | `0x125f1: callq *0xce0(%rax)` | `engine→SetSampleRate(sampleRate*)` — triggers `UpdateDCLProgramsWithNewPacketSizes @ 0x122d6` which calls `SetPacketParameters` (→ `SetupCIP`) on both TX and RX `FWDCLProgram` instances; updates pkt_count, CIP header template, SYT fields |
| 5 | `0x125fa: callq CompletePendingStateChanges()` | Flush workloop pending state; drain in-flight DCL callbacks |
| 6 | `0x12606: callq *0xa58(%rax)` | `stream→resumeIO()` — restart CoreAudio delivery with new packet geometry |

**Condition at step 3:** only fires if new rate ≠ current `[+0x220]` (current_rate field) OR AC3 flag changed (`[+0x2d4]` → `0x1251c`/`0x1253f`).

**AC3 passthrough:** format tag `0x63616333` = `'cac3'` in little-endian. When switching into AC3 mode, rate is forced to 48000 Hz (`movl $0xBB80, -0x28(%rbp)` at `0x12580`).

---

## Provenance

- `SetPacketParameters` caller: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47263` (`0x272f3`), chain of `cmpl; jne` rate comparisons at `0x27322`–`0x273ef`.
- `SetupCIP` FDFValue switch: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47195` (`0x271d1`–`0x27279`); all 7 cases resolved via the jump table at `0x272d8` (Correction 2026-10-14, §3).
- `PerformFormatChange`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:21010` (`0x124cc`–`0x12618`), step-by-step vtable offsets extracted.
- AC3 tag `0x63616333`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:21037` (`0x12513: cmpl $0x63616333`).
- 44.1 kHz SYT fields: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:47202` (`0x27209`); offsets `[+0x4c0]=0xC7`, `[+0x4c4]=0x280`, `[+0x4b0]=0x153`, `[+0x4b4]=0x1B9`.
