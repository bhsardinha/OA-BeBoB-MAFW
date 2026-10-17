# Q-MB — TX/RX DCL callbacks, packet-index table, CIP/SYT cadence, sample ring (closes the "OutputDCLCallback" gap of Q2 and the `takeTimeStamp` site of C2 §6.1)

> **Correction (2026-10-15, Q-START / R43):** the hypothesis below is closed: channel start is `FWEngine2::StartAudio` -> `StartPCMAudio` -> `ActivatePCMInput` (RX `Start`) then `ActivatePCMOutput` (TX `Start`, primes 4 banks first); `vt[0x120]` is `Connect`. Evidence: [`Q-START-channel-start-order.md`](Q-START-channel-start-order.md).

**Status:** done 2026-10-15 (static; BUILD-PLAN task M-B, evidence entry R41). Bundle: kext 1.11.3 (`_extracted/`, sha256 `5532c6e8…`). Method: Ghidra 12.1.4 headless, read-only, over `A1-ghidra/ghidra-project` (script `DecompList.java`: decompile the function containing an address) **plus** raw `otool` assembly for every place where the decompiler lost call arguments (`Add/SubtractFWCycleTimeFromFWCycleTime`, stack arrays). Raw output: `ghidra-exports/decomp/q-mb-dcl-callbacks.c` (34 functions + the `TRingBuffer`/`FWTPacketIndex`/iterator appendix); assembly `otool-exports/mafw.asm` lines 46269-46606 (`OutputDCLCallback`), 45217 (`RebuildDCLProgram`), 45588/45815 (`RebuildDCLProgramCore`), 46047 (`ResyncInputBuffer`). Nothing here has run on 10.13.6: the clone code that implements it (`openaudio-fw1814/kext/src/FWDCLProgram.*`, `FWCip.*`, `TPacketPool.*`, `FWSharedSync.*`) is unfinished and untested (see "Clone mapping").

> **Address note.** Q2 cites `OutputDCLCallback @0x26269`; that number is the *line* in `mafw.asm`. The function is at file address **`0x263c2`** (`functions.tsv`). The same line-vs-address slip affects `AdjustOutputDCL` (`0x2636e`, not `0x26237`).

## 1. Answer in one paragraph

Each of the four TX banks is a **group of exactly `P` DCL packets** (`P = ⌊65,536,000/(rate·4)⌋ + 2`) of which the ring sends only the last `n` per lap. When the callback after the big segment of bank *b* fires (OHCI wrote the bank's end time into a slot in the last page of the pool), `OutputDCLCallback(b)` **re-generates the group for its next lap**: it decides, packet by packet, whether the cycle carries data or is a NODATA cycle (cadence accumulator or the receiver's SYT table), advances the running presentation time (`SYT`) and the data block counter (`DBC`), builds the two CIP header quadlets of every packet, stops after `n` packets or **2048 samples**, and then **skips the unused leading primers** by re-pointing the previous bank's jump (`AdjustOutputDCL`, up to 4 one-packet primer segments). Finally it clears the group (`ZeroElements`, `0xCF`/`0x80` fill), records which packets are data packets (`SetValidPackets`, the table the sample converters use) and writes the headers (`InitGroupCIPsForOutput`). The same callback of **bank 3** is where the engine's `takeTimeStamp` is fed. The RX side mirrors this: each 4-packet segment callback drains the received packets (CIP checks, SYT table, MIDI, payload copy into a byte ring) and marks them consumed.

## 2. Field map (reference object offsets → meaning → evidence)

`FWDCLProgram` (base), offsets from the object start:

| Offset | Meaning | Evidence |
|---|---|---|
| `+0x10` | `FWDCLBuilder*` (node arena + label/jump arrays) | `RebuildDCLProgram` asm `0x25590` |
| `+0x18..+0x57` | embedded `TPacketPool` (see Q17; pool `+0x08`→`FWTPacketIndex*` at program `+0x20`, base VA `+0x28`, count `+0x30`, per-group `+0x34`, packet bytes `+0x38`, pages `+0x3c`, per-page `+0x40`, audio Q `+0x44`, MIDI Q `+0x48`, slack `+0x4c`, init `+0x50`, **audio-pool flag `+0x51`**) | decomp `SetPacketSize` (`this[0x51]=1`) |
| `+0x58` | `FWIsochChannel*` (its `+0x28` = the local isoch port the program re-targets jumps on) | `AdjustOutputDCL`: `*(chan+0x28)` |
| `+0x60` | packet bytes (`4·(a+m)·SYT + 8` TX / `+12` RX) | `SetPacketSize` stores `param_1` |
| `+0x64` / `+0x68` | audio quadlets *a* / MIDI quadlets *m* per data block | `SetPacketSize`, `SetupCIP` header 1 |
| `+0x70` | owning `FWAudioDevice*`; `+0x78` = its `IOFireWireNub*` (`[dev+0xe0]`) | `Initialize @0x27458` |
| `+0x80` | `FWDCLSharedSyncData*` (section 3) | `Initialize` |
| `+0x8c` / `+0x8d` / `+0x8e` | connected / running / **primary program** (set to 1 on both programs by `FWEngine2::initHardware`, asm `0x1271a`/`0x12721`) | `Disconnect`, `Start`, callbacks |
| `+0x90` | secondary program (stereo-pair engines; the TX callback prepares its pool too) | `OutputDCLCallback` |
| `+0x498` `+0x49c` | CIP header templates (`SID<<24 | DBS<<16`, `0x90000000`) | `SetupCIP` |
| `+0x4a0` `+0x4a4` `+0x4a8` `+0x4ac` | rate Hz, FDF index, SYT increment (ticks), SYT interval (blocks) | `SetupCIP` |
| `+0x4b0 +0x4b4 +0x4b8` | fractional SYT spreader: numerator C, denominator D, running sum | `SetupCIP`, callback |
| `+0x4bc +0x4c0 +0x4c4` | cadence: running sum, A, B (`{1,2}` 32 k, `{199,640}` 44.1 family, `{1,4}` 48 family) | `SetupCIP`, callback |
| `+0x4c8` `+0x4cc` | `⌈(B<<11)/((B−A)·SYT)⌉`, `⌈(B<<12)/((B−A)·SYT)⌉` (48 k: 683) = presentation offset **in cycles** | `SetupCIP` |
| `+0x4d0` | DBC (byte); `+= SYT interval` after **every** packet, data or NODATA | callback asm `0x6808` |
| `+0x4d4` / `+0x4d8` | running presentation time (cycle-time format) / last consumed input SYT | callback |
| `+0x4dc` / `+0x4e0` | `⌊65,536,000/(rate·4)⌋ − 2` / **`P`** | `SetupCIP` |
| Output only: `+0x518` lap counter; `+0x520/+0x528/+0x530` and `+0x538…` history {bank, host ns, lap}; `+0x550` history index; `+0x558…` MIDI schedule; `+0x618` **external-sync flag** (`FWEngine2::SetExternalSyncEnabled`, asm `0x11bed` copies engine `+0x2d0`) | callback, `Start`, asm `0x11bc0` |
| Input only: `+0x504` DBC-valid, `+0x505` expected DBC, `+0x508` TX program, `+0x510` consecutive-NODATA counter | `InputDCLCallbackCopyPacketGroupToSamples` |

`FWDCLSharedSyncData` (shared by TX, RX, engine): `+0x00` running-output counter; `+0x0c` resync cycle time; `+0x11` resync pending; `+0x14` last TX bank; `+0x18` `uint16*` SYT table (2048 entries, mask `0x7ff`); `+0x20` RX write index; `+0x24` TX read index; `+0x28` `TRingBuffer` (RX payload); `+0x40` input byte position; `+0x58` MIDI-in enabled; `+0x5c` MIDI-in port count; `+0x60` MIDI-out port count; `+0x68` MIDI-out ports (`TMIDIPort`, 0x28 stride); `+0x70` MIDI-in ports; `+0x78` the engine (receives `takeTimeStamp`); `+0x80` start time ns; `+0x88` last RX segment (`0x3f` at `Start`). Names are ours; each offset is read at the cited decompile/asm site.

## 3. `OutputDCLCallback(bank, prime)` (asm `0x263c2..0x68fe`; decomp lines 593-995)

```
if !primary(+0x8e) or bank>3: return
ts = slot[bank]                       // *(pool_base + pages*4096 - 0x1000 + bank*4), written by the OHCI PtrTimeStamp DCL
if !prime {                           // hardware callback
    if bank==3: lap++
    ns = CycleTimeToHostNanos(ts); hist[idx^1] = {bank, ns, lap}; shared.lastBank = bank
    if bank==3: if lap>=3: engine->takeTimeStamp(true, ns->abs)      // vt[0xae0] on shared+0x78  [inferred: signature (1,&abstime)]
                else if lap==2: shared.resyncCycleTime = ts; shared.resyncPending = true
}
if !running and !prime: return
if !prime {                           // keep the presentation clock ahead of the transmit clock
    c = ((ts>>12)&0x1fff) + q12; d = ((sytBase>>12)&0x1fff) - c; if d < -4000: d += 8000
    if (d-3) as unsigned >= 3:        // i.e. d not in {3,4,5}
        sytBase = (ts & 0xfe000000) + (((c<<12) + 0x4000) & 0x1fff000) + (sytBase & 0xfff)
}
useQueue = false
if externalSync(+0x618): avail = shared.sytWrite - shared.sytRead
    if avail >= P+4:  if avail > 3P: shared.sytRead = sytWrite - 4 - P
        base = (q12 + (ts>>12)) << 12
        e = table[read & 0x7ff]; if e==0xffff or (uint16)(e-base)-0x1000 < 0x2001: useQueue = true
        else repeat { read++; e = table[read&0x7ff]; if e==0xffff continue; if ((e-base)&0xf000)-0x1000 <= 0x1000 {useQueue = true; break} }
n = 0; samples = 0
if P != 0: do {
    e = table[read & 0x7ff]; i = n++
    if useQueue {
        if e == 0xffff { read++; sync = false }                       // NODATA cycle
        else { delta = (sytLast > e) ? Add(Subtract(0xfc00, sytLast), e) : Subtract(e, sytLast)
               sytBase = Add(sytBase, delta); sytLast = e; read++; sync = true }
    } else {
        cadAcc += A; if cadAcc > B { cadAcc -= B; sync = false } else sync = true
        if sync { fracAcc += C; inc = (fracAcc <= D) ? sytIncrement + 1 : (fracAcc -= D, sytIncrement)
                  sytBase = Add(Add(Add(sytBase, inc>>1), inc>>1), inc&1) }
    }
    if sync: samples += sytInterval
    valid[i] = sync; CalcIsochPacketHeaders(&hdr[i], (uint16)sytBase, dbc, sync); [+ secondary]; dbc += sytInterval
} while n < P and samples < 0x800
shared.sytRead = read
unused = P - n
AdjustOutputDCL(bank, unused) [+ secondary]
pool.ZeroElements(bank); pool.index.SetValidPackets(bank, unused, n, valid); pool.InitGroupCIPsForOutput(bank, n, hdr)  [+ secondary]
```

Notes. (1) `Add/Subtract…` argument order was fixed from the assembly (`0x66c1`, `0x6703`, `0x6779`): `Subtract(a,b)` is `a−b`. (2) The cadence reproduces the blocking-mode pattern: 48 k `{1,4}` gives D D D N (6000 data packets/s × 8 = 48 000 samples/s); 44.1 k `{199,640}` gives 68.9 % data (44 100). (3) SYT increments are `0x1000` ticks for 8 samples at 48 k (166.67 µs at 24.576 MHz), `0x116A` (+1 spread) at 44.1 k. (4) The DBC advances after NODATA packets too — the reference does not follow IEC 61883-6's "empty packet keeps the DBC"; this driver's NODATA packets are full-size (424 B in capture 74; the only size seen), so every packet counts its 8 blocks. Mirror it; do not "fix". (5) The 2048-sample window is why a group is `P ≈ 343` packets at 48 k yet only ≈ 341 are needed (256 data + 85 NODATA): `unused` is usually 2.

`AdjustOutputDCL(bank, unused)` (decomp 557-590): if `unused < 5`: `ModifyJumpLabel(jumpIdx, labelIdx)` with `(bank,jump,label) = (1,0,unused+5) (2,1,unused+10) (3,2,unused+15) (0,3,unused)`: the jump that **precedes** bank *b* is re-pointed to label `5·b + unused`, i.e. into bank *b* after skipping `unused` one-packet primers. `ModifyJumpLabel` (decomp `FWDCLBuilder__ModifyJumpLabel.c`) writes `jump->pJumpDCLLabel` in place (offset `0x18`) and calls `port->notify(kFWDCLModifyNotification = 2, &jump, 1)`. For `unused ≥ 5` nothing is changed (a jump left from an earlier lap stays — reference behaviour, mirrored).

`OutputProgram::Start` (decomp 998-1040): reset history/lap/MIDI schedule, run `OutputDCLCallback(0..3, prime=true)` to prime all four banks, then `FWDCLProgram::Start()` (= `channel->start()`, vt `0x158` on the `FWIsochChannel`); on success `shared.outputRunning++`. `Stop`: `channel->stop()` (vt `0x160`), `running=0`, `outputRunning--`.

## 4. The pool is offset by one packet relative to the DCL chain (reference quirk, mirrored)

`FWDCLProgram::SetPacketSize @0x25e56` (decomp 258-377) assigns buffers with `TPacketForwardIterator(pool, 1, 0xffffffff)`. `TPacketIterator::CommonInit(start=1, count=−1)` sets the first address to **packet 1** and `NextPacket` returns the *current* address then advances (wrapping to packet 0 after the last), so **DCL transfer *k* gets pool packet `(k+1) mod N`**; pool packet 0 belongs to the very last DCL. Consequences, all consistent with each other (reasoning from the decompile; **not** exercised on hardware): `InitGroupCIPsForOutput` writes the group's header 0 into pool packet 0 of the group (sent by the previous bank's last DCL) and active headers at `base + unused + i`; `SetValidPackets` maps the first valid packet to index 0 (`entry = (j == 0) ? 0 : unused + j`); the RX callback reads pool packets `4·seg … 4·seg+3` which lag the segment's own packets by one (so the packet still in flight is never read). Whether the first-valid-packet mapping to index 0 is intended or a latent reference bug is **unverified**; the clone mirrors it exactly because the sample converters (M-D/M-J) read through the same table.

## 5. RX callback `InputDCLCallbackCopyPacketGroupToSamples(segment)` (decomp 1246-1370)

Catch-up: `next = (lastSeg+1)&0x3f`; if `next != segment`, `d = ((segment − lastSeg) + 0x3f) & 0x3f`, `if d > 0x20: d |= 0xffffffc0`, negative → return; process `next … next+d`. Per segment: `t = Subtract(slot[seg&0x3f], 0x3000)` (time of the segment's first packet); primary: `lastSeg = seg&0x3f`. Per packet *j* (pool packet `(4·seg)&0xff + j`): skip if `quad1 & 0x80` (already consumed / EOH set); `SYT = bswap16(*(u16*)(pkt+10))`; primary: `table[wr++ & 0x7ff] = SYT` (also for `0xFFFF`); `SYT == 0xFFFF` → `nodata++`; otherwise `nodata = 0`, MIDI-in parse, DBC tracking (`expected` re-seeded on mismatch; `+= SYT interval`), `CopySamplesFromPacket((a+m)·SYT interval quadlets from pkt+12 → inputRing)`, **`quad1 = 0x80`** (consumed marker), and if `resyncPending`: `ResyncInputBuffer(SYT + (t & 0xffff0000))`, pending cleared when it returns true; `t = Add(t, 0x1000)`. After 4 packets: primary writes back `sytWrite`; every even segment calls the TX program's `ProcessCompoundMIDIOutput(3)` when MIDI is on. Payload offset is **`pkt + 12`** (4-byte isoch header + 8-byte CIP) — `TPacketPool::GetPacketFrameContainingSampleIndex` (`+2` quadlets) is the *TX* layout.

`ResyncInputBuffer` (decomp 434-478; asm arg order checked): if primary, `Δ = cycles(arg) − cycles(shared.resyncCycleTime)` mod 8000 (nearest, `<0x7d1` direct); if `Δ > 0`: `samples = ((sytIncrement/2 + (cycles·0xc00 + seconds·0x1770000 + ticks) · sytInterval) / sytIncrement) & 0x1fff`; `bytesPerFrame = 4·(a+m)`; sets `shared.inputReadByte = bytesPerFrame · samples` unless equal; returns true.

`CycleTimeToHostNanos(ct)` (decomp 510-554): brackets `bus->getCycleTime` between two `clock_get_uptime` until the bracket is < 150 µs, then `host_ns = t0 + (t1−t0)/2 + 125,000 · Δcycles` with `Δcycles` the nearest value modulo 8000 (direct if `|Δ| < 0x7d1`).

## 6. Other decoded pieces

* `FWDCLProgram::Initialize @0x27458`: stores device/nub/shared, `audioQ = 2`, `SetupCIP(44100, 8, 0x116A, 1)`, `RebuildDCLProgram`. `SetPacketParameters @0x272f4`: `(rate, audioQ, midiQ, &syt, &fdf)`; table switch as Q2; unknown rate → 44.1 kHz; calls `vt[0x118]` (= `SetPacketSize(audioQ, midiQ)`), failure `0xE00002BC`. `Output::SetPacketSize @0x260d2` = base with `(a+m)·SYT·4 + 8`, count `P<<2`; `Input::SetPacketSize @0x260a8` = `… + 12`, count `0x100`. `RebuildDCLProgram @0x25586` frees the builder (`~TDataBuffer`, `operator delete` after a NULL test) and tail-calls `vt[0x150]`; `RebuildDCLProgramCore` builds with `OutputDCLStaticCallback` + `P` (TX) / `InputDCLStaticCallback` + 4 (RX); NULL → `0xE00002BD`.
* `SetupCIP`: header 1 uses the **local node ID** (the 3-argument `getNodeIDGeneration` third output, `localID & 0x3f`), not the device's node ID: this host is the CIP source.
* `CalcIsochPacketHeaders`: `out[0] = bswap32(dbc | hdr1)`; NODATA `out[1] = 0xFFFFFF90`; data `out[1]` = bytes `{0x90, FDF, SYT hi, SYT lo}` (FDF = SFC index 0-6 as in IEC 61883-6: 0 = 32 k … 6 = 192 k).
* `ResetPort(talking, cycleTime) @0x2541a`: `sytBase = cycleTime`; releases the old listener, **re-creates the local port with cycle-match start** `createLocalIsochPort(talking, head, NULL, startEvent=1, startState = talking ? cycleTime>>12 : cycleTime, startMask = talking ? 0x7fff : 0x7fff000)`, `channel->addListener(port)` (vt `0x138`) and `port->allocatePort(speed, channel)` (vt `0x120`). Who calls it (engine start) is **unverified**; `Connect(channel)` (`CreateIsochChannelAndPorts`, Q1b) uses start `(0,0,0)`.
* `ConnectAudioMIDIChannels` (decomp `FWEngine2__ConnectAudioMIDIChannels.c`) calls **`vt[0x120]`** of the TX/RX programs with argument `0`: that slot is `Connect(channel)`, not `Start`. See "Corrections" — R1's "TX program start" wording must be re-verified in M-C.
* `TRingBuffer`, `FWTPacketIndex`, `TPacketIterator` decoded (appendix of the decomp file); `TPacketPool::MarkGroupReadForInput @0xa738` (clears bit 7 of packet byte 11) and `SyncCurrentInputPacketIndex @0xa86c` have **no callers found**; not ported.

## 7. Corrections this item forces (applied 2026-10-15)

1. **`SetTagSyncBits(1,0)` is emitted on the FIRST primer of every TX bank**, not on "the last segment" (Q-DCL, C4 §3, Q1a, memory `reference_dcl_structure_authoritative`): asm `0x25b50..0x25b59` computes `isLast = (primer == 0)` for each of the four primer calls per bank.
2. **`takeTimeStamp` is fed from the TX callback of bank 3** from the third lap on (C2 §6.1 / C4 §11 "open" is closed); the RX callback does not call it.
3. **Q2: `OutputDCLCallback` is at `0x263c2`** and its cadence/SYT/DBC logic is decoded here (the "needs its own question" note is resolved).
4. **Q17 / Q-DCL: pool packet ↔ DCL offset by one** (section 4).
5. **R1 / C2 §7 wording:** `ConnectAudioMIDIChannels` invokes `vt[0x120](…, 0)` on the programs = `Connect`, so "TX program start → 500 ms → INPUT" is *Connect* (channel/IRM allocation + port wiring), and the actual `channel->start()` happens elsewhere (probably `StartIsochStreams`, vt `0xd08/0xd10` in `FWEngine2::StartAudio`). **Hypothesis — needs verification at M-C** (decompile `FWBaseEngine` slots `0xd08`, `0xd10`, `0x128` of the program).
6. **Q7 (also applied):** `FWMetaNub` derives from `IOService`; `message()` = `messageClients` + `IOService::message`.

## 8. Open / unverified

* Meaning of the constant `0xfc00` in `Subtract(0xfc00, last)` (16 cycles × 0x1000 ticks = one SYT wrap) — arithmetic reading only.
* `vt[0xae0]` on the engine = `IOAudioEngine::takeTimeStamp(bool, AbsoluteTime*)` — identified from the argument pattern `(1, &abstime)`; slot not cross-checked against the 10.13 SDK vtable.
* Intent of "first valid packet → index 0" and of the one-packet offset (section 4).
* Which function starts the channel after `Connect` (item 5 above) and who calls `ResetPort`.
* MIDI scheduling (`ScheduleMidiPacket @0x26ab2`, `ProcessCompoundMIDIOutput @0x26be6`) is decoded in the decomp file but not documented here; it belongs to M-H (A3 M5/M6).
* Behaviour on 10.13.6: classic-DCL acceptance by `generateBufferMap` (P1) remains unproven until the integrated run.

## 9. Clone mapping (work in progress, 2026-10-15)

| Reference | Clone file | State |
|---|---|---|
| `TRingBuffer`, `FWDCLSharedSyncData` | `kext/src/FWSharedSync.{h,cpp}` | written, compiles |
| `SetupCIP`, `CalcIsochPacketHeaders` | `kext/src/FWCip.{h,cpp}` | written |
| `TPacketPool` + `FWTPacketIndex` | `kext/src/TPacketPool.{h,cpp}` | written (offset-by-one assignment still lives in `FWDCLProgram`) |
| `FWDCLBuilder` + `ModifyJumpLabel` | `kext/src/FWDCLBuilder.{h,cpp}` | written |
| `FWDCLProgram/Output/Input` | `kext/src/FWDCLProgram.{h,cpp}` | **ported 2026-10-15**: builds on the build PC (10.13 SDK), `kextutil -t -n` from `/tmp` OK, never loaded. Run-time behaviour unverified (integrated test). `Connect`/`ResetPort` and the RX-then-TX start order are M-C |
| Engine, channel/IRM, CMP, AV/C, MIDI | skeleton files with plan headers | not started (M-C next) |

## Provenance

`ghidra-exports/decomp/q-mb-dcl-callbacks.c` (function headers `=== name @ addr ===`): `OutputDCLCallback @0x263c2` (l.593), `AdjustOutputDCL @0x2636e` (l.557), `Output::Start @0x26900` (l.998), `InputDCLCallbackCopyPacketGroupToSamples @0x26e3e` (l.1246), `ResyncInputBuffer @0x2610e` (l.434), `CycleTimeToHostNanos @0x26284` (l.510), `SetPacketSize @0x25e56` (l.258), `SetPacketParameters @0x272f4` (l.1373), `Initialize @0x27458` (l.1463), `ResetPort @0x2541a` (l.1640), `SetValidPackets @0x10504` (l.153), `InitGroupCIPsForOutput @0xa8b0` (l.113), `CommonInit @0x11484` and the `TRingBuffer`/iterator appendix. Assembly: `mafw.asm` 46269-46606 (callback), 45624 ff. (`BuildTransmittingDCLProgram`, isLast at 45624+`0x25b50`), 11bc0 (`SetExternalSyncEnabled`), 21150-21190 (`FWEngine2::initHardware` primary flags). SDK (build PC, 10.13): `IOFireWireFamilyCommon.h` (DCL structs, `kDCL*Op`), `IOFWLocalIsochPort.h` (`notify`), `IOFWUtils.h` (`AddFWCycleTimeToFWCycleTime`, `SubtractFWCycleTimeFromFWCycleTime`), `IOFireWireBus.h` (`createLocalIsochPort`, `getCycleTime`).
