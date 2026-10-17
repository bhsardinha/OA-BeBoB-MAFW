# Q17 — `m_audio_FWb::TPacketPool` — DCL packet-pool allocator

> **Correction (2026-10-15, M-B / R41):** (1) `FWTPacketIndex` is now decoded (`SetValidPackets @0x10504`, `GetIndexOfActivePacket @0x1055e`, `init @0x105f2`: per-group active-offset + list of `count/4` entries) and `InitGroupCIPsForOutput` semantics are exact (header 0 always to the group's packet 0, then header *i* to `base + activeOffset + i`). (2) Buffers are assigned to the DCL chain **offset by one**: iterator start index 1, so DCL *k* ↔ pool packet `(k+1) mod N`. (3) `ZeroElements` has a raw path (`bzero`) for non-audio pools (`[+0x51] == 0`). (4) `MarkGroupReadForInput @0xa738` and `SyncCurrentInputPacketIndex @0xa86c` have no callers. See [`Q-MB-dcl-callbacks.md`](Q-MB-dcl-callbacks.md) §4, §6.


> **Correction (2026-10-14) — live TX buffer attempt (SL)** (evidence: [capture 74](../B5-sl-dtrace-truth/captures/74-sl-tx-buffer-attempt.txt), [PROVENANCE-RESOLUTIONS.md §R28](../PROVENANCE-RESOLUTIONS.md)). Partial live check on the SL (v1.9, idle): the TX OHCI program really sends **424-byte packets = 8-byte CIP header + 8 events × 13 quadlets** (CIP template `01 0d 00 00 | 90 ff ff ff`, DBS 13 = 12 audio + 1 MIDI at the current clock bits), and only a repeating subset of the descriptors carries the extra interrupt bits (`0x183c` vs `0x100c`), as Q-OHCI-interrupt predicts. The **label bytes of the live data blocks could not be read** (data buffers are only reachable by physical address on a 32-bit kernel with no `/dev/mem`; the pool allocation never recurred, even across an interface power cycle). So R10 (`0x40` audio / `0x80` MIDI / `0xCF` unrewritten) is still supported by the decompile only, and 'the device requires 0xCF' remains a **hypothesis**; it does not block the clone, which mirrors the reference.


> **Correction (2026-10-14) — firmware lock-detection strings** (evidence: [PROVENANCE-RESOLUTIONS.md §R23](../PROVENANCE-RESOLUTIONS.md)). Firmware-side hint (strings of `fw1814.bcd`, no disassembly): the DM1000 image configures `/cfg/dev/isodrv/...` with `inLockDetection`, `LOCK detection is PLL_LOCK / FIX_LOCK`, `pllInLock`, `playOutSytOffset`, `dbcOffsetCode`, `EmptyPktGen` ('DM1000 MDB generates empty packets'), `ISO_NOT_INLOCK`; none of these strings mentions AM824 labels or the value 0xCF. This suggests lock detection is driven by the iso/SYT cadence rather than label contents, but the AM824 label decode is done by the isochronous hardware block and **the effect of label 0xCF on the audio output stays unverified** (check = isochronous wire capture of the host's idle TX plus device output, which needs the FW1814 on a machine with a bus analyser; not possible on the SL rig, where only `mach_kernel` `fbt` exists, nor on the High Sierra machine, where the device is not attached).


> **Correction (2026-10-14) — 0xCF / ZeroElements** (evidence: [PROVENANCE-RESOLUTIONS.md §R10](../PROVENANCE-RESOLUTIONS.md)). `TPacketPool::ZeroElements(uint)` (`mafw.asm` 11694; stores at 0xa6da `movl $0xcf` and 0xa701 `movl $0x80`) is not only an init-time pre-fill: it is called from `FWDCLOutputProgram::OutputDCLCallback` (0x26874 for the primary pool, 0x268b8 for the secondary), i.e. **every TX callback re-clears the next group** before `SetValidPackets`/`InitGroupCIPsForOutput`. The live output converters then overwrite the slots they handle with label `0x40` (MBLA, e.g. `LocalRearrangeSampleFrame16Bit2` stores `((s<<16)+s)&0xFFFF00 + 0x40`, 0x1197f-0x11986; silent slots `movl $0x40`, 0x1195d) and MIDI/other slots with `0x80` (0x119bd). So `0xCF` survives only in slots no converter rewrites. What the DM1000 firmware does with label `0xCF` is **unverified** (check = ARM-code analysis of `fw1814.bcd` or a wire capture of idle TX); 'AM824 silence = 0xCF' in older text is not supported — the silent audio quadlet written by live code is `0x40` with zero data.


> **Correction (2026-10-14) — Part A completion batch 1** (evidence: [PROVENANCE-RESOLUTIONS.md §R8](../PROVENANCE-RESOLUTIONS.md)). Pool size: pages = `floor([+0x18]/[+0x28]) + 2` with `[+0x28] = floor(4096/packetSize)`; alloc = `pages<<12`, physical mask = `bus->vt[0x140]() & ~0xFFF`, options 0 (`TPacketPool::Initialize 0xa3e6..0xa49e`). Meaning of `[+0x18]` **unverified**; the earlier 'ceil(N/floor(4096/S))+2' wording (C4:455) is superseded.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - 'The DM1000 firmware is tuned against 0xCF' (line 114) is a **hypothesis**; no capture shows it. Check = SL-rig isoch capture comparing idle behaviour with 0xCF vs zero fill.


**Status:** done (optional).

**Scope:** The templated packet-pool used by both input and output DCL programs for AM824 packet backing storage. Covers `Initialize`, memory layout, index arithmetic, `ZeroElements` (packet silence fill), and CIP template seeding.

## Answer in one line

**TPacketPool allocates N packets of size `S` across `floor([+0x18] / [+0x28]) + 2` (2026-10-14 Correction R8; the earlier `ceil(N/floor(4096/S))+2` wording is superseded) physical 4 KiB pages via `IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task)`, with `floor(4096/S)` packets per page and `4096 - floor(4096/S)*S` bytes of per-page slack unused** — this is the page-aligned backing memory that the DCL program's `DCLTransferPacket` entries point into.

## Object layout (56 bytes)

| Offset | Type | Semantic |
|---|---|---|
| `+0x00` | `IOBufferMemoryDescriptor*` | DMA-mapped backing pages (kernel task) |
| `+0x08` | `m_audio_b_FWTPacketIndex*` | packet-index table (lazily `withElementCount` if ctor arg is NULL) |
| `+0x10` | `void*` | kernel VA of mapped buffer (from `IOMD::getBytesNoCopy`) |
| `+0x18` | `uint32_t` | `totalPackets` |
| `+0x1c` | `uint32_t` | `packetsPerGroup = totalPackets / 4` |
| `+0x20` | `uint32_t` | `packetBytes` (per-packet size) |
| `+0x24` | `uint32_t` | `numPages = totalPackets / packetsPerPage + 2` |
| `+0x28` | `uint32_t` | `packetsPerPage = floor(4096 / packetBytes)` |
| `+0x2c` | `uint32_t` | `cipDataQuadlets` (ctor param 4) |
| `+0x30` | `uint32_t` | `cipPadQuadlets` (ctor param 5) |
| `+0x34` | `uint32_t` | `slackBytes = 4096 - packetBytes * packetsPerPage` |
| `+0x38` | `uint8_t` | initialized flag |
| `+0x39` | `uint8_t` | audio-pool flag (ZeroElements picks CIP vs raw path on this bit) |

## Initialize sequence

```c
TPacketPool::Initialize(fwDev, numPkts, pktBytes, cipDataQ, cipPadQ, packetIndex)
{
    this->packetBytes      = pktBytes;
    this->totalPackets     = numPkts;
    this->packetsPerGroup  = numPkts / 4;
    this->cipDataQuadlets  = cipDataQ;
    this->cipPadQuadlets   = cipPadQ;
    this->packetsPerPage   = 4096 / pktBytes;
    this->slackBytes       = 4096 - pktBytes * (4096 / pktBytes);
    this->packetIndex      = packetIndex ?: FWTPacketIndex::withElementCount(numPkts, slack);
    this->numPages         = numPkts / packetsPerPage + 2;

    uint64_t physMask = fwDev->getBus()->getDMAPhysicalMask();  // vtable[+0x140]
    this->iobmd        = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(
                             kernel_task, 0,
                             numPages * 4096,
                             physMask & ~0xFFFULL);              // page-aligned
    if (!iobmd || iobmd->prepare(0) != 0) goto fail;             // vtable[+0x1f0]
    this->kernelVA     = iobmd->getBytesNoCopy();                // vtable[+0x2e0]
    this->initialized  = (kernelVA != NULL);
    return initialized;
fail:
    ForceReleaseMemory(this);
    return 0;
}
```

Observations:
- Allocation is **kernel-task** (not userspace-mapped). The DCL program uses the physical pages for DMA; userspace only sees derived views via `clientMemoryForType` (Q11 fu).
- The `+2` page overallocation is intentional padding — probably one head page for CIP state + one trailing wrap page.
- `physMask & ~0xFFFULL` forces page-aligned physical addresses — matches BeBoB DMA alignment requirement.

## Index arithmetic

`GetIndexedPacket(i)`:
```
addr = kernelVA + (i / packetsPerPage) * 4096 + (i % packetsPerPage) * packetBytes
```
Zero-index returns `kernelVA` (sentinel/head). Out-of-range (`i >= totalPackets`) also returns `kernelVA` silently (**no bounds error** — callers must validate).

`GetPacketFrameContainingSampleIndex(sampleIdx, samplesPerPkt, outPktCount, outPkt, outFrameInPkt)`:
```
pktIdx       = packetIndex->GetIndexOfActivePacket(
                   sampleIdx >> 11,                             // group index (2048-sample blocks)
                   sampleIdx / samplesPerPkt - (0x800/samplesPerPkt) * (sampleIdx >> 11));
frameOffset  = sampleIdx % samplesPerPkt;
pktBase      = GetIndexedPacket(pktIdx);
*outPktCount = samplesPerPkt - frameOffset;
return pktBase + (cipDataQuadlets + cipPadQuadlets) * frameOffset + 2;  // +2 = skip 2 CIP quadlets
```

So AM824 **payload starts at `packetBase + 2 quadlets`** (= 8 bytes), after the pair of CIP headers. Each sample frame occupies `(cipDataQuadlets + cipPadQuadlets) * 4` bytes within the payload. This is the ground truth for the DCL payload stride our clone needs to match the 2014 driver.

## Packet silence fill — `ZeroElements(group)`

Walks `packetsPerGroup` packets starting at `group * packetsPerGroup` via `TPacketForwardIterator::NextPacket`.

Two paths based on `this[+0x39]`:

**Raw path** (`+0x39 == 0`, non-audio pool):
```
for each pkt in group:
    bzero(pkt, packetBytes);
```

**Audio path** (`+0x39 != 0`, AM824 pool):
```
for each pkt in group:
    groupsPerPacket = (packetBytes >> 2) / (cipDataQ + cipPadQ);
    for g in 0..groupsPerPacket:
        base = pkt + 8 + g*(cipDataQ + cipPadQ)*4;
        // data region: fill with 0x000000CF — on the big-endian FireWire wire this
        // emits bytes `CF 00 00 00`, i.e. AM824 label = 0xCF (vendor-specific idle),
        // 24-bit sample = 0x000000.
        for q in 0..cipDataQ: base[q] = 0x000000CF;
        // pad region: fill with 0x00000080 — on wire `80 00 00 00`, i.e. AM824
        // label = 0x80 (NODATA / no-info), data = 0x000000.
        for q in cipDataQ..cipDataQ+cipPadQ: base[q] = 0x00000080;
```

**What this fill is (consolidated 2026-10-14, evidence R10 + asm).** `ZeroElements` runs at pool init **and again from every `FWDCLOutputProgram::OutputDCLCallback`** (asm 0x26874 / 0x268b8) for the next group, before the live output converters run. The converters then overwrite the slots they serve: audio slots get label `0x40` (`((s<<16)+s)&0xFFFF00 + 0x40`, 0x1197f-0x11986; silent slots `movl $0x40` @0x1195d), MIDI/other slots `0x80` (`movl $0x80` @0x119bd). Therefore `0xCF` is visible on the wire only for slots no converter rewrites (and in the packets sent before the first callback); the label of a silent *handled* audio slot is **`0x40`** with zero data - the same as snd-bebob's `0x40000000`.

**What is NOT established:** that this fill is "the mechanism" of click-free idle (older wording), or that the DM1000 firmware is tuned against `0xCF`. Supported facts: the reference TX ring keeps sending data-bearing packets at idle (capture 74: 424-byte packets = 8-byte CIP + 8 events x 13 quadlets on the SL, v1.9); macfw reports that a host->device stream of NODATA-only packets does not let the INPUT rate command succeed (macfw `special-firmware-notes.md`, `first-duplex-capture-success.md`, third-party hardware on macOS). Unsupported: any statement that `0xCF` (vs `0x40`) matters, or that a zero-filled ring was the cause of the pre-Ghidra clicks (no capture ties them; the DCL-callback hypothesis is also still unproven - Q-OHCI-interrupt). Open check: Q17.2 (bus capture of idle TX label bytes, or ARM analysis of `fw1814.bcd`).

> **Changelog:** 2026-10-13 B2 flagged "label 0x40" vs "0xCF" wording (label byte is `0xCF` in the pool fill; confirmed `movl $0xcf` @0xa6da, `movl $0x80` @0xa701). 2026-10-14 R10 added the per-callback refill and the converter overwrite; "firmware tuned against 0xCF" demoted to hypothesis (provenance audit AUD-A1-04, status Rotulado).

## CIP template seeding — `InitGroupCIPsForOutput(group, extraCount, cipTemplate*)`

Writes the first qword (two 32-bit CIP headers) of each group's base packet:
```
basePktIdx = packetsPerGroup * group;
offset = packetIndex->activePacketOffset[group];   // from FWTPacketIndex +0x18 + group*4
if (offset + extraCount <= packetsPerGroup) {
    *(uint64_t*)GetIndexedPacket(basePktIdx) = cipTemplate[0];
    for (i = 0; i < extraCount; i++)
        *(uint64_t*)GetIndexedPacket(basePktIdx + offset + i) = cipTemplate[1 + i];
}
```

This is called at stream start to install the CIP header templates (h1/h2 captured in Q2) into each DCL group's packet-head.

## Lifecycle

- Ctor zeros all 4 pointer fields and `+0x38`. 
- `ForceReleaseMemory`: releases packetIndex (`vtable[+0x28]`), unprepares IOMD (`vtable[+0x1f8]`), releases IOMD (`vtable[+0x28]`), nulls `kernelVA`.
- `Invalidate`: marks un-init but calls ForceReleaseMemory anyway (same effect).
- No reinit path — pool is destroy-and-reallocate.

## Clone impact

The clone currently open-codes DCL buffer allocation inside `FW1814DCLBuilder::Build*` using plain `IOBufferMemoryDescriptor` with implicit sizes. Switching to a TPacketPool-equivalent gives us:

1. **Mirror the fill + overwrite order.** Pool init and every TX callback call `ZeroElements` (data quadlets `0x000000CF`, pad quadlets `0x00000080`), then the converters write `0x40` audio / `0x80` MIDI slots. A zero-filled (`bzero`) ring would differ from the reference in the not-rewritten slots and in the packets before the first callback; whether that matters for the FW1814 is **unverified** (it was suspected as the cause of pre-Ghidra idle clicks - no capture confirms). Mirroring costs nothing, so do it, but do not cite it as the proven click fix.
2. **Correct page alignment** — the clone's IOMD doesn't pass `physMask & ~0xFFF`, which can cross-page a packet on hardware with stricter DMA alignment.
3. **Correct CIP header seeding** — the clone writes CIP headers at build time; the reference re-seeds them from a template at `StartIsochStreams` time via `InitGroupCIPsForOutput`. Matters for sample-rate changes without rebuilding the ring.

**Recommendation:** add a minimal `FW1814PacketPool` helper mirroring the field map above (no templates needed, just a C++ class with the same layout + methods). Priority LOW - does not change the Part D decision. *(2026-10-14: the earlier phrase 'the single biggest fix for TX silence' is withdrawn; the TX-silence root cause is a hypothesis, see CLAUDE.md in-flight notes and Q-OHCI-interrupt.)*

## Provenance

- All decompiles: `scripts/DecompQ16Q17.java` → `ghidra-exports/decomp/q16q17.log` lines 49–408.
- Addresses: `Initialize` `0xa3e6`, `ctor` `0xa564`, `ForceReleaseMemory` `0xa4e8`, `GetIndexedPacket` `0xa5bc`, `ZeroElements` `0xa5e6`, `GetPacketFrameContainingSampleIndex` `0xa7aa`, `InitGroupCIPsForOutput` `0xa8b0`.
