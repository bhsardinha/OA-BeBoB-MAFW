# B2 — snd-bebob cross-reference: isoch shape, AM824 silence, CMP connect, plug map

> **Correction (2026-10-14) — OHCI interrupt.** Row 'Timestamp mechanism': on 10.13.6 `DCLPtrTimeStamp` only attaches a timestamp pointer to the preceding DCL; the interrupt comes from the CallProc-derived callback (A1-ghidra/Q-OHCI-interrupt.md). The snd-bebob `interrupt = true` analogue is the CallProc, not the timestamp.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - **Stream kick** in the reference is TX start → 500 ms → INPUT 0x19 → 500 ms → RX start → OUTPUT 0x18 (`ConnectAudioMIDIChannels @0x12156`, asm 0x121c9..0x122bb); the 100 ms order is snd-bebob `bebob_maudio.c:312-337` only. Third-party/macfw statements that 'the stream-kick we locked is hardware truth' are **third-party-only** evidence and are contradicted by the kext order; hardware effect of the 500 ms gaps is **unverified** (check = SL-rig timestamped FCP capture).
> - 'FWHSCI::ChangeRate (A1-Q15c)' names no real symbol (Q15c has no such symbol; the engine path is `PerformSampleRateChangeCore @0x117e6`).


> **Correction (2026-10-14) — DCL ring shape.** Per `A1-ghidra/Q-DCL-geometry-table.md` (raw disassembly): **TX** = 4 banks × (4 one-packet primer segments + 1 large segment of `P−4` packets that carries the callback) + 4 jumps = **20 segments** (`BuildTransmittingDCLProgram @ 0x25ae4`; `P = *(this+0x4e0)`, per rate). **RX** = **64 segments × 4 packets** + 64 jumps (`BuildReceivingDCLProgram @ 0x25962`; `packetCount` immediate `$0x4` at `0x25a99`, fixed at all rates). Any "64-segment TX" statement below is wrong; RX 64-segment statements stand. Timing derived from "64 segments × ~1 ms" must be recomputed: 4 packets = 4 isoch cycles ≈ 500 µs per RX segment.

> **Correction (2026-10-14, B2.2 audit) - the two snd-bebob gaps vs A1 are closed from the kext binary** (`otool-exports/mafw.asm`; snd-bebob `_refs/linux-snd-bebob/sound/firewire/`).
> - **`CIP_EMPTY_HAS_WRONG_DBC` (bebob.c:170-175, bebob_stream.c:449-452, amdtp-stream.c:774-776) - Resolved: not needed in the kext's RX path.** `FWDCLInputProgram::InputDCLCallbackCopyPacketGroupToSamples @0x26e3e` reads, per packet, the first CIP quadlet at packet `+4` (`movl 0x4(%rax)` @0x26f7a; DBC = top byte after `shrl $0x18` = wire byte 7; bit 7 of wire byte 4 = EOH set -> packet skipped, @0x26f84) and the SYT half-word at `+0xa` (@0x26f9b). If SYT == `0xFFFF` (empty/NODATA packet) it increments `[+0x510]` and jumps over the packet **before** the DBC is used (`cmpw $-0x1` @0x26fc1, `jne` @0x26fc5, `incl 0x510(%rbx)` @0x26fc7). For data packets the DBC is only compared with a stored expected value `[+0x505]` (flag `[+0x504]`); a mismatch just re-seeds `[+0x505]` (@0x27016-0x2702f), then `[+0x505] += SYTInterval([+0x4ac])` (@0x27041). There is no lost-packet error path. So a `dbc += 8` on empty packets is invisible to the kext. **Clone implication (reasoning):** do not add strict DBC-continuity checking on RX that the reference lacks. **Unverified:** that the FW1814's wrong-DBC empties carry SYT `0xFFFF` (a bus capture would show it); if they did not, the kext would simply re-seed.
> - **Post-registration bus reset (bebob.c:280-290) - Resolved with a different shape.** The kext has no bus-reset call in its import table (`nm -u`: no `resetBus`/`BusReset` command symbol); the one virtual call that lands on `IOFireWireBus::resetBus` is `bus->vt[0x880]` at `0x1c9ea`, inside `com_m_audio_FWMetaNub::FirmwareUpdate()` — **on its error path only** (after `FirmwareUpdateBeBoB251`/`BeBoB32` returned non-zero; `0x1c9cf`): it calls `[this+0xe8]->vt[0x880]()` and then `IOSleep(0xdac)` = **3500 ms** (corrected 2026-10-14 by Q-P3/R37; the success path returns without a bus reset). Slot derivation: `createLocalIsochPort` is `bus->vt[0x858]` (Q1b; load at 0x254bc), and in `_refs/IOFireWireFamily/IOFireWireFamily.kmodproj/IOFireWireBus.h` `resetBus()` (line 258) is the 5th virtual after `createLocalIsochPort` (line 246; between them `getCycleTime`, `getBusCycleTime`, `AddUnitDirectory`, `RemoveUnitDirectory`) -> `0x858 + 5*8 = 0x880`; no other `callq *0x880(` exists in the binary. That identification is by header slot order (**reasoning, medium confidence**; a runtime check would be a `bus_reset` event ~3.5 s after firmware start in the SL capture of a bootloader->runtime transition). The kext issues **no equivalent of Linux's gap-count fix for a device that is already in runtime mode**; whether the FW1814 has the gap-count problem under IOFireWireFamily is **unverified**.
> - **0xCF vs 0x40 (section 2).** The "firmware tuned against 0xCF" and "do not substitute 0x40" statements in 2.3 and 2.5 are superseded: the live output converters overwrite the pool fill with label `0x40` on every callback (A1 Q17 Correction R10; asm `movl $0x40` @0x1195d, `((s<<16)+s)&0xFFFF00 + 0x40` @0x1197f-0x11986, MIDI slots `movl $0x80` @0x119bd). So for converter-handled slots the reference puts **`0x40`, the same label snd-bebob writes**; the "DISAGREE" row only holds for the initial pool fill and for slots no converter rewrites. The device effect of `0xCF` stays a **hypothesis** (check: bus-analyser capture of idle TX, or ARM analysis of `fw1814.bcd`; firmware strings give no hint, R23).
> - **4.6 byte 9 / 4.7 stream kick:** see the two in-place corrections below; the 4.7 statement that `special_set_rate` "cross-validates" the stream-kick memory is withdrawn (the reference order is TX start -> 500 ms -> INPUT 0x19 -> 500 ms -> RX start -> OUTPUT 0x18; 100 ms between OUTPUT and INPUT is snd-bebob/macfw only).

**Source material:** Linux kernel `sound/firewire/bebob/` and `sound/firewire/` support code at `_refs/linux-snd-bebob/`. Cross-checked against Part A1 Ghidra decode of `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice`.

**Scope:** confirm or contradict Part A findings on four topics — (1) the TX+RX DCL ring shape (A1-Q1/Q1a), (2) AM824 silence pre-fill constants (A1-Q17), (3) FW1814 plug map + CMP connect order (A1-Q3), (4) product discovery / IDs. Flag deltas; prefer `_extracted/` on disagreement.

## Status / next step

Done 2026-10-13. **Headline:** snd-bebob is a parallel re-implementation against the same BeBoB hardware, so it is useful for AV/C-wire truth and for the maudio-special vendor envelope — but it does **not** touch the OHCI DCL layer (Linux uses NuDCL via `fw_iso_context_queue`, not classic DCL). So the Q1/Q1a DCL ring questions **cannot be answered from snd-bebob at all**; A1 is the only ground truth for that layer. On AM824 silence (Q17), the two drivers disagree about the exact per-quadlet silence constant — `_extracted/` writes `0xCF…` label + `0x80` NODATA label; snd-bebob writes `0x40000000` as the MBLA zero PCM sample. Both are legal AM824. For the macOS clone we mirror `_extracted/`. On CMP connect (Q3), snd-bebob confirms Input-plug-rate-mirrors-Output, confirms iPCR/oPCR layout, confirms the FW1814 vendor-specific 12-byte HSCI envelope byte-for-byte, and adds two previously-unmemo'd FW1814-specific details (`SND_BEBOB_QUIRK_WRONG_DBC` and the post-register bus-reset workaround).

Next: feed §4.3 (CIP-DBC quirk) and §4.4 (post-attach bus-reset) into Part C before any clone rate-change or probe path is specified.

## 1. Scope boundary — what snd-bebob does and does not model

Linux's FireWire audio stack is split into three independent layers:

| Layer | Linux file | What it does | Our equivalent |
|---|---|---|---|
| OHCI iso contexts | kernel `drivers/firewire/*` (not here) | NuDCL-style packet scatter-gather kernel API (`fw_iso_context_create`, `fw_iso_context_queue`) | IOFireWireFamily `IODCLProgram` + `createLocalIsochPort` (classic DCL on 10.13) |
| AMDTP packer | `_refs/linux-snd-bebob/sound/firewire/amdtp-stream.c` + `amdtp-am824.c` | CIP header build, AM824 label write, PCM silence fill | `_extracted/kext/` `BuildReceivingDCLProgram` + `TPacketPool::ZeroElements` + `InitGroupCIPsForOutput` |
| BeBoB device driver | `_refs/linux-snd-bebob/sound/firewire/bebob/*.c` | AV/C discovery, PCR/MPR connect, M-Audio vendor envelope | `_extracted/kext/` `FW1814Device`, `FWEngine2`, `ConnectAllPlugs`, `FWHSCI` |

**Key consequence:** snd-bebob does **not** build DCL command chains. It calls into the FireWire core driver, which hides the OHCI-descriptor layout entirely. Any question of the form "how many segments / how many jumps / where does the OHCI IR context fire its interrupt" (Q1, Q1a, Q1b) has no snd-bebob answer — Linux delegates all of that to `fw_iso_context_queue(ctx, pkt, buffer, offset)` on a per-packet basis (`amdtp-stream.c:646`). snd-bebob is parallel reverse-eng on protocol/wire semantics only.

This document is therefore scoped to AMDTP packer (Q17 overlap) and BeBoB device driver (Q3, Q7, Q6 overlap). Q1/Q1a stay A1-only.

## 2. AM824 silence constants (Q17 cross-check)

### 2.1 What A1-Q17 captured in the reference kext

Decompile `TPacketPool::ZeroElements` @ `_extracted/kext/.../M-AudioFireWireBeBoB.x86_64` offset `0xa5e6` (Ghidra log `ghidra-exports/decomp/q16q17.log:49–408`):

- Data region (per quadlet): `0x000000CF`
- Pad region (per quadlet): `0x00000080`

Interpretation per A1-Q17 lines 102–109: `0xCF` is the AM824 label byte with data value 0 (label=0x40 "MBLA 24-bit"? no — label 0xCF ≠ 0x40; the memo there was wrong — see §2.4). `0x80` is the AM824 NODATA label used in pad slots and MIDI quadlets. **The byte is big-endian aligned to the low byte of each 32-bit quadlet**, i.e. the quadlet's label is at `(quadlet >> 0) & 0xff` when read as host-endian from a little-endian x86 kernel. That matters because it means the on-wire packet (which is big-endian) sees `0xCF 00 00 00` as the first quadlet in the data region — label byte first on the wire — and `0x80 00 00 00` for pad.

### 2.2 What snd-bebob writes for silence

`amdtp-am824.c:209–220` — `write_pcm_silence`:

```c
static void write_pcm_silence(struct amdtp_stream *s,
                              __be32 *buffer, unsigned int frames)
{
    struct amdtp_am824 *p = s->protocol;
    unsigned int i, c, channels = p->pcm_channels;

    for (i = 0; i < frames; ++i) {
        for (c = 0; c < channels; ++c)
            buffer[p->pcm_positions[c]] = cpu_to_be32(0x40000000);
        buffer += s->data_block_quadlets;
    }
}
```

And `amdtp-am824.c:169–171` — `write_pcm_s32` (live PCM path):

```c
buffer[p->pcm_positions[c]] = cpu_to_be32((*src >> 8) | 0x40000000);
```

MIDI/pad quadlets `amdtp-am824.c:311–317`:

```c
b[0] = 0x80;      // NODATA label (no MIDI byte available)
b[1] = 0;
b[2] = 0;
b[3] = 0;
```

So snd-bebob writes:
- Live PCM: label `0x40` + 24-bit sample in the low 3 bytes (`0x40 ss ss ss` on wire). This is the standard "AM824 MBLA 24-bit" encoding per IEC 61883-6.
- Silence PCM: `0x40000000` → `0x40 00 00 00` on wire — i.e. same label, sample value zero.
- MIDI pad: `0x80 00 00 00` on wire — matches `_extracted/`.

### 2.3 Delta

| Region | `_extracted/` (A1-Q17 at `0xa5e6`) | snd-bebob (`amdtp-am824.c:217` / `:313`) | Agree? |
|---|---|---|---|
| MIDI/pad quadlet silence | `0x00000080` host-endian → `0x80 00 00 00` wire | `b[0] = 0x80; b[1..3] = 0` → `0x80 00 00 00` wire | **MATCH** |
| PCM data quadlet silence | `0x000000CF` host-endian → `0xCF 00 00 00` wire | `0x40000000` host-endian → `0x40 00 00 00` wire | **DISAGREE on label byte** |

**Both are valid AM824.** Label `0x40` is "multi-bit linear audio, 24-bit, L=0 (channel is active)". Label `0xCF` — bit decomposition `0b11001111` — is also legal (label-group 0xC = "ancillary" or extension; `0xCF` specifically is in the vendor-extensible range per IEC 61883-6 Table 11). The exact semantics of `0xCF` are **unverified** — needs IEC 61883-6:2005 Table 11 cross-check, or a 2026 DTrace capture of the SL reference Mac Pro's TX packet dump to confirm the on-wire label the DM1000 firmware actually expects at idle (B5 captures planned but not yet run for this slice).

**hypothesis — needs verification:** The DM1000's AMDTP sync detector likely accepts any MBLA label (including `0xCF`) as "data present" for the purpose of PLL lock. The reference 2014 driver chose `0xCF` as a vendor-specific idle marker to let the device distinguish host-silence from live-silence in its own metering path (B5 peak-meter path reads label bytes). snd-bebob uses the standard `0x40` because it doesn't care about the vendor meter — it only cares about PLL lock. **Verification path:** DTrace the SL reference driver's TX FIFO during idle and compare label-byte distribution against `0x40` and `0xCF`.

### 2.4 Correction to A1-Q17 line 103

A1-Q17 line 103 asserts that `0xCF` is "AM824 silence marker — label=0x40, data=0x00". That's internally inconsistent — label=0x40 would put `0x40` in the byte, not `0xCF`. The correct statement is "`0xCF` is the vendor-chosen silence label for the DM1000; `0x40` is the IEC-standard MBLA label". The data byte is `0x00` regardless.

**Correction (2026-10-13):** A1-Q17 §Audio path "fill with 0xCF (AM824 silence marker — label=0x40, data=0x00)" is contradictory — the label is `0xCF`, not `0x40`. See B2 §2.4 above. Does not change the clone decision (we mirror `_extracted/` either way), but the memo string is wrong.

### 2.5 Clone recommendation

> **Correction (2026-10-14):** the recommendation below was written before R10. Mirror the reference's *code*, which fills the pool with `0xCF`/`0x80` at start and re-fills each callback, then overwrites handled slots with `0x40`/`0x80`. A clone that mirrors that code gets `0x40` on the wire for handled slots. "The 2014 driver's firmware was tuned against the 0xCF idle signature" is a **hypothesis** with no support (neither capture nor firmware strings).

Mirror `_extracted/` exactly: data-region quadlets `0x000000CF`, pad/MIDI-region quadlets `0x00000080`, using the TPacketPool `ZeroElements` audio path (A1-Q17 §Audio path). **Do not substitute `0x40000000`** on the grounds that snd-bebob uses it — snd-bebob doesn't ship on the DM1000 vendor path; the 2014 driver does, and the 2014 driver's firmware was tuned against the `0xCF` idle signature.

The AMDTP CIP-FMT value, FDF formatting, and SFC-based rate index are shared between the two drivers (CIP_FMT_AM = `0x10` per `amdtp-am824.c:13`; FDF = `AMDTP_FDF_AM824 | sfc` per `amdtp-am824.c:94`) — those are IEC 61883-6 wire format and agree trivially.

## 3. DCL ring shape (Q1 / Q1a) — not addressable from snd-bebob

### 3.1 Why Linux cannot answer the DCL questions

snd-bebob allocates isoch context via `fw_iso_context_create_with_header_storage_size` (`amdtp-stream.c:1714`) with `type = FW_ISO_CONTEXT_TRANSMIT` or `FW_ISO_CONTEXT_RECEIVE`. The queue surface is:

```c
err = fw_iso_context_queue(s->context, params, &s->buffer.iso_buffer,
                           ...);                                 // amdtp-stream.c:646
```

The kernel core takes one `fw_iso_packet` struct per logical packet and compiles its own OHCI descriptor chain internally. Linux's compiler is a per-packet descriptor emitter — it is **not** classic DCL. On x86 Linux (and ppc, arm, etc.), the OHCI driver (`drivers/firewire/ohci.c` and `core-iso.c`) uses a flat descriptor array; the "segment ring + jump" shape from A1-Q1 is a classic-DCL macOS artifact of the IOFireWireFamily pre-NuDCL era.

### 3.2 Queue size — one weak hint

`amdtp-stream.c:2102–2114`:

```c
queue_size = DIV_ROUND_UP(CYCLES_PER_SECOND * events_per_buffer,
                          amdtp_rate_table[d->irq_target->sfc]);
...
err = amdtp_stream_start(s, s->channel, s->speed, queue_size, idle_irq_interval);
```

Linux sizes its packet queue dynamically from the ALSA period/buffer in frames at the current rate. There is **no fixed "64 segments / 64 jumps"** constant on the Linux side. The A1-Q1 finding (64-segment ring) is binary-authoritative for the macOS kext regardless.

### 3.3 Delta — none to resolve; keep A1 answer

| Topic | `_extracted/` (A1-Q1/Q1a) | snd-bebob | Resolution |
|---|---|---|---|
| DCL command set | M-Audio internal opcodes 1,5,8,9,12,14 over classic DCL (A1-Q1a §OpcodeToStructSize) | n/a — kernel core builds OHCI descriptors from `fw_iso_packet` structs | A1 is sole truth |
| Ring shape | 64 segments × (label, timestamped-xfer, callproc, jump), 64 jumps | n/a — flat per-packet queue, dynamic size | A1 is sole truth |
| Timestamp mechanism | `DCLPtrTimeStamp` (op 14) emits OHCI IRQ at descriptor completion | `fw_iso_packet.interrupt = true` on the irq_target stream's last packet of a period (`amdtp-stream.c:1250` et al.) | Different mechanism, same purpose |

**The Q1 TX-silence root cause (clone has 4-segment ring, must be 64) is unaffected by snd-bebob review.** No action.

## 4. CMP connect + plug map (Q3 cross-check)

### 4.1 What A1-Q3 says

- FW1814 has 2 oPCRs + 3 iPCRs. Plug 0 (both directions) carries PCM.
- `FWEngine2::ConnectAllPlugs` @ `0x12476` is: `fConnected` gate → `SufficientBandwidthAvailable` → `ConnectAudioMIDIChannels` → set `fConnected`.
- `FWEngine2::initHardware` @ `0x12622` allocates two DCL programs (output `0x620` bytes, input `0x518` bytes), cross-links `input->fOutputProgram = output` at `+0x508`, marks both active (`+0x8e = 1`), registers both with `AddDCLProgram`.
- Channel counts flex with `fMode`:
  - ADAT-active (bit 4): 12-in vs 6-in
  - digital-mix (bit 3): 16-out vs 10-out

### 4.2 What snd-bebob does

**Plug discovery is dynamic**, not hardcoded. `bebob_stream.c:908 snd_bebob_stream_discover`:

```c
err = avc_general_get_plug_info(bebob->unit, 0x1f, 0x07, 0x00, plugs);
...
err = fill_stream_formations(bebob, addr, AVC_BRIDGECO_PLUG_DIR_IN, 0,
                             bebob->rx_stream_formations);        // uses plug 0
err = fill_stream_formations(bebob, addr, AVC_BRIDGECO_PLUG_DIR_OUT, 0,
                             bebob->tx_stream_formations);        // uses plug 0
```

For FW1814 the `maudio_special_spec` quirk path replaces this: `bebob_maudio.c:258 snd_bebob_maudio_special_discover` sets the formations from a hardcoded `ch_table[2][2][3]` indexed by `[direction][dig_in_fmt or dig_out_fmt][rate_index/2]`:

```c
static const unsigned int ch_table[2][2][3] = {
    /* AMDTP_OUT_STREAM */ (rx = device-input / host→device)
    { {  6,  6,  4 },     /* SPDIF out:    6ch @ 44/48, 6ch @ 88/96, 4ch @ 176/192 */
      { 12,  8,  4 } },   /* ADAT out:    12ch @ 44/48, 8ch @ 88/96, 4ch @ 176/192 */
    /* AMDTP_IN_STREAM  */ (tx = device-output / device→host)
    { { 10, 10,  2 },     /* SPDIF in:    10ch @ 44/48, 10ch @ 88/96, 2ch @ 176/192 */
      { 16, 12,  2 } }    /* ADAT in:     16ch @ 44/48, 12ch @ 88/96, 2ch @ 176/192 */
};
```

(`bebob_maudio.c:230–237`, note snd-bebob's "AMDTP_OUT_STREAM" is host→device = our rx_stream from device perspective; see `bebob_stream.c:439–447` for the direction mapping.)

Point-to-point connect: `bebob_stream.c:406 start_stream` → `cmp_connection_establish` → `cmp.c:294 cmp_connection_establish` which modifies the iPCR or oPCR register on the target via `TCODE_LOCK_COMPARE_SWAP` to the CSR address `CSR_REGISTER_BASE + CSR_IPCR(idx)` / `CSR_OPCR(idx)` (`cmp.c:65–71`). PCR bitfields (`cmp.c:25–38`):

```
PCR_ONLINE             0x80000000
PCR_BCAST_CONN         0x40000000
PCR_P2P_CONN_MASK      0x3f000000
PCR_CHANNEL_MASK       0x003f0000
OPCR_XSPEED_MASK       0x00C00000
OPCR_SPEED_MASK        0x0000C000
OPCR_OVERHEAD_ID_MASK  0x00003C00
```

Rate must be mirrored input ← output before CMP, per `bebob_stream.c:70–90 snd_bebob_stream_get_rate`:

```c
err = avc_general_get_sig_fmt(unit, &tx_rate, AVC_GENERAL_PLUG_DIR_OUT, 0);
err = avc_general_get_sig_fmt(unit, &rx_rate, AVC_GENERAL_PLUG_DIR_IN,  0);
if (rx_rate != tx_rate)
    err = avc_general_set_sig_fmt(unit, rx_rate, AVC_GENERAL_PLUG_DIR_IN, 0);
```

And when setting a new rate (`bebob_stream.c:95–117`):

```c
err = avc_general_set_sig_fmt(unit, rate, AVC_GENERAL_PLUG_DIR_OUT, 0);
err = avc_general_set_sig_fmt(unit, rate, AVC_GENERAL_PLUG_DIR_IN, 0);
msleep(300);   // 300 ms transition
```

Note snd-bebob's 300 ms wait **after** both commands, vs memory `project_fw1814_stream_kick`'s 100 ms wait **between** the two commands. See §4.5.

### 4.3 FW1814-specific CIP-DBC quirk (not in A1)

`bebob.c:170–175`:

```c
case MODEL_MAUDIO_FW1814:
case MODEL_MAUDIO_PROJECTMIX:
    // At high sampling rate, M-Audio special firmware transmits empty packet
    // with the value of dbc incremented by 8.
    bebob->quirks |= SND_BEBOB_QUIRK_WRONG_DBC;
    break;
```

And `bebob_stream.c:449–452`:

```c
if (stream == &bebob->tx_stream) {
    if (bebob->quirks & SND_BEBOB_QUIRK_WRONG_DBC)
        flags |= CIP_EMPTY_HAS_WRONG_DBC;
}
```

**Translation for us:** at 88.2 / 96 / 176.4 / 192 kHz, the DM1000 firmware emits empty (NODATA) packets whose `dbc` field is **incremented by 8** (standard behavior: empty packets keep dbc unchanged). The macOS RX callback must tolerate this when parsing CIP continuity — otherwise the `InputDCLCallbackCopyPacketGroupToSamples` dbc-continuity check (A1-Q1 §prologue, references `expected_next` logic) will falsely report a dropped packet.

**Verification path for the kext:** re-decompile `InputDCLCallbackCopyPacketGroupToSamples` at `0x26e3e` and check whether it has a code path that accepts `dbc += 8` for `data_blocks == 0` packets. If not — and the kext still works at high rates — then **hypothesis**: the kext uses a different continuity check (e.g. based on SYT presence rather than DBC monotonicity) or ignores dbc on empty packets. This matters only if we re-implement RX dbc validation in the clone.

### 4.4 FW1814-specific post-registration bus reset (not in A1)

`bebob.c:280–290`:

```c
if (entry->vendor_id == VEN_MAUDIO &&
    (entry->model_id == MODEL_MAUDIO_FW1814 ||
     entry->model_id == MODEL_MAUDIO_PROJECTMIX)) {
    // This is a workaround. This bus reset seems to have an effect to make devices
    // correctly handling transactions. Without this, the devices have gap_count
    // mismatch. This causes much failure of transaction.
    fw_schedule_bus_reset(fw_parent_device(bebob->unit)->card, false, true);
}
```

**Translation:** after `snd_card_register`, Linux forces an IEEE-1394 bus reset to fix a gap-count mismatch on FW1814/ProjectMix. Without this, subsequent transactions (AV/C, CSR reads/writes, PCR connect) fail intermittently.

**Does the macOS kext do this?** A1 didn't look. **Verification path (Q-new):** grep the reference kext for `resetBus` / `generateBusReset` / `FWController::resetBus` calls near `start`/`attach`/`willTerminate`/post-firmware-upload paths. If present, A1 missed a lifecycle step. If absent, the Snow Leopard IOFireWireFamily may generate the reset implicitly on vendor-ID registration — needs B5 ioreg capture to confirm (planned).

If the macOS kext does **not** issue a post-attach bus reset but the Linux driver does, the clone must do it. Otherwise initial AV/C discovery can fail intermittently.

### 4.5 Vendor-specific HSCI envelope (Q6 cross-check)

`bebob_maudio.c:170–226` `avc_maudio_set_special_clk` builds and sends:

| byte | value | meaning |
|---|---|---|
| 0 | `0x00` | CONTROL ctype |
| 1 | `0xff` | UNIT subunit addressing |
| 2 | `0x00` | vendor-dependent opcode |
| 3–5 | `0x04 0x00 0x04` | company_id = 0x040004 |
| 6 | `clk_src` | clock source: `0x00=Internal+DigMute`, `0x01=Digital`, `0x02=WordClock`, `0x03=Internal` (`bebob_maudio.c:341–346`) |
| 7 | `dig_in_fmt` | `0x00=S/PDIF`, `0x01=ADAT` |
| 8 | `dig_out_fmt` | `0x00=S/PDIF`, `0x01=ADAT` |
| 9 | `clk_lock` | `0x00=unlock`, `0x01=lock` |
| 10–11 | `0x00 0x00` | padding |

Sent via `fcp_avc_transaction(bebob->unit, buf, 12, buf, 12, ...)` — AV/C over FCP registers, response status check at `buf[0]` (`0x08`=NOT_IMPLEMENTED, `0x0a`=REJECTED, else accept).

### 4.6 Comparison vs auto-memory `project_fw1814_hsci_envelope` + A1-Q6

| Byte | Our memory | snd-bebob | Agree? |
|---|---|---|---|
| 0 (ctype) | `0x00` CONTROL | `0x00` | yes |
| 1 (subunit) | `0xff` UNIT | `0xff` | yes |
| 2 (opcode) | `0x00` vendor-dep | `0x00` | yes |
| 3–5 (company_id) | `0x040004` | `0x04 0x00 0x04` | yes |
| 6 (clk_src) | TBD / hw-specific | 0=IntDigMute, 1=Digital, 2=WordClock, 3=Internal | **new mapping** — our memory `project_fw1814_clock_sources` lists Internal/S/PDIF/ADAT/Word Clock as four UI entries; snd-bebob confirms the device-level clock-src is only 4 values with Digital collapsing S/PDIF+ADAT (the digital kind is picked separately by byte 7) |
| 7 (dig_in_fmt) | `0x00=S/PDIF, 0x01=ADAT` hypothesis per memory | **confirmed** `0x00=S/PDIF, 0x01=ADAT` | yes |
| 8 (dig_out_fmt) | `0x00=S/PDIF, 0x01=ADAT` hypothesis per memory | **confirmed** `0x00=S/PDIF, 0x01=ADAT` | yes |
| 9 (clk_lock) | not in memory | `0x00=unlock, 0x01=lock` — "lock these settings" | **Correction (2026-10-14):** in the macOS kext byte 9 is `(MAClockSource >> 5) & 1` = S/MUX (asm `0xe41a..0xe425`), and the frame is 16 bytes (`0x10` @0xe441); the `clk_lock` name and the 12-byte length are snd-bebob's. The "device-side latch" reading is unsupported for the kext. |
| 10–11 (padding) | not in memory | `0x00 0x00` | yes |

**Three new facts for the clone:**
1. Clock source byte 6 is a 2-bit enum, not a 4-way UI enum. The macOS panel's four UI entries (Internal, S/PDIF, ADAT, Word Clock) **must map** to `{byte6, byte7}` tuples:
   - Internal → `{0x03, dontcare}` (or `0x00` for int+dig-mute)
   - S/PDIF   → `{0x01, 0x00}`
   - ADAT     → `{0x01, 0x01}`
   - Word Clock → `{0x02, dontcare}`
2. Byte 9 (`clk_lock`) exists and is a device-side setting latch. Memory `project_fw1814_hsci_envelope` does not mention it. **Verification path:** re-decompile the kext's `FWHSCI::SetClockSource` and look for a 4th parameter being written at buffer offset 9. If present, update memory.
3. Four `MAClockSource` panel values is a UI abstraction over device byte-tuples — confirms the composite-bitfield memory `project_maclocksource_is_composite_bitfield.md` (which says one 32-bit key packs clock-src + optical-out + S/MUX + gate). B5 §10 shows the macOS panel maps a single 32-bit key into these fields; snd-bebob models it as four separate kcontrols (`special_clk_ctl`, `special_dig_in_iface_ctl`, `special_dig_out_iface_ctl`, implicit `clk_lock`). **Same underlying device bytes, different host-side packaging.**

### 4.7 Rate-change timing (memory `project_fw1814_stream_kick`)

> **Correction (2026-10-14):** the closing sentence of this section ("This cross-validates `project_fw1814_stream_kick` as the correct FW1814 path") is **withdrawn**. snd-bebob and macfw are third-party hardware evidence for *their* order; the reference kext does INPUT 0x19 before OUTPUT 0x18, 500 ms apart, with the DCL program starts in between (Q3a section 3 Correction, asm 0x121c9-0x122bb). macfw's hardware finding that does carry over: the INPUT rate command fails unless the host->device stream is running and data-bearing (macfw `special-firmware-notes.md` experiments 3-4 and `first-duplex-capture-success.md`); the kext likewise starts TX *before* sending INPUT 0x19.

- Memory says: OUTPUT set 0x18 → 100 ms → INPUT set 0x19.
- snd-bebob `bebob_stream.c:95–117`: OUTPUT set → INPUT set → 300 ms.
- snd-bebob `bebob_maudio.c:312–337 special_set_rate`: OUTPUT set → 100 ms → INPUT set.

**The `special_set_rate` path matches our memory exactly** (100 ms **between** the two sets, not after both). The generic BeBoB path differs (both-then-300ms). FW1814 is on the `maudio_special_spec` → uses `special_set_rate`. **This cross-validates `project_fw1814_stream_kick` as the correct FW1814 path.**

**Clone note:** when rate-changing, use the 100 ms *between* pattern, not the 300 ms *after* pattern. The `_extracted/` kext's `FWHSCI::ChangeRate` (A1-Q15c) should be checked for the same inter-command delay — see Q15c, which already captured a `scaledTickToNanos` / `IOSleep` in the format-performer path.

## 5. Clock-source topology (Q5 cross-check)

### 5.1 Standard BeBoB clock discovery

`bebob_stream.c:119–258 snd_bebob_stream_get_clock_src` picks between three paths in order:

1. **Device-specific `clock->get`** callback — used by `maudio_special_spec` (FW1814 → `special_clk_get` in `bebob_maudio.c:347–352`, which just returns `params->clk_src` cached from last set).
2. **MSU (Music Sub Unit) sync-input plug** — queried via `avc_bridgeco_get_plug_input` on plug type `AVC_BRIDGECO_PLUG_TYPE_SYNC`. If no sync-input plug, assume internal clock.
3. **Default internal.**

For FW1814 the device-specific path wins; snd-bebob never actually walks MSU plugs for it.

### 5.2 Delta vs A1-Q5

A1-Q5 captures the kext's `MAClockSource` property-dict mechanism (via `IORegistryEntrySetCFProperties` from the panel; see B5 §10). The underlying wire protocol is the 12-byte HSCI envelope — same bytes snd-bebob sends. **No disagreement.** snd-bebob adds the "digital-mute at internal" option (byte 6 = `0x00`) which the macOS panel may or may not expose as a separate radio button — needs panel decode (A2-P3 MA* key schema has `MAClockSource` with 32-bit composite values; bit 30 "gate" may be the digital-mute equivalent).

**Resolved (2026-10-14) — hypothesis refuted:** byte 9 = `MAClockSource` bit 5 (S/MUX), raw disassembly `0xe41a..0xe425`; bit 30 is the gate (Q5). ~~**Hypothesis — needs verification:** `MAClockSource` bit 30 = `clk_lock` byte 9~~, not digital-mute. **Verification path:** compare the exact 32-bit values the panel sends for "Internal" vs "Internal with Digital Mute" (if the latter exists) against the resulting byte-6 value in the HSCI frame; a B5 FCP capture across the two UI states resolves it.

## 6. Product IDs (sanity check vs memory `project_fw1814_ids`)

`bebob.c:63`:

```c
#define MODEL_MAUDIO_FW1814    0x00010071
```

`bebob.c:459–461`:

```c
SND_BEBOB_DEV_ENTRY(VEN_MAUDIO, 0x00010070, NULL),              /* bootloader */
SND_BEBOB_DEV_ENTRY(VEN_MAUDIO, MODEL_MAUDIO_FW1814, &maudio_special_spec),  /* runtime */
```

**Confirms memory `project_fw1814_ids`:** runtime `0x10071`, bootloader `0x10070`. snd-bebob and the kext (A1-Q7) and FFADO agree. No `0x10051`.

The bootloader-mode entry has `spec = NULL`; `bebob_probe` falls through to `snd_bebob_maudio_load_firmware` (`bebob.c:202–207`) which sends the three-cue magic sequence to address `BEBOB_ADDR_REG_REQ` to tell the device to load its on-flash firmware. **This is a parallel to our userland firmware-helper** (A4 wire protocol); snd-bebob's cues are `0x00000001`, `0x01110000`, `0x00000000` (`bebob_maudio.c:40–49, 119–126`). Our firmware-helper's cue sequence is different (A4 captured the full binary upload, not a flash-cached cue trigger) — because our FW1814 doesn't have v5058+ firmware stored on flash, so we upload the full `.bcd`. Not a conflict; different scenarios.

## 7. Agreement / disagreement summary

| Topic | A1 answer | snd-bebob answer | Winner for macOS clone |
|---|---|---|---|
| DCL ring 64 segs / 64 jumps / timestamped-xfer | yes (Q1, Q1a) | n/a (NuDCL kernel) | **A1** |
| AM824 label at idle: `0xCF`/`0x80` | yes (Q17) | `0x40`/`0x80` | **A1** (vendor firmware tuned to `0xCF`) |
| AM824 pad quadlet = `0x80 00 00 00` on wire | yes (Q17) | yes (`amdtp-am824.c:313`) | match |
| CMP connect via P2P iPCR/oPCR set | yes (Q3) | yes (`cmp.c:294` → compare-swap) | match |
| FW1814 plug 0 = PCM both dirs | yes (Q3) | yes (`bebob_stream.c:932,937`) | match |
| HSCI envelope 12 bytes, company_id `0x040004` | yes (Q6) | yes (`bebob_maudio.c:187–198`) | match |
| HSCI byte 7 S/PDIF=0x00 / ADAT=0x01 | hypothesis in memory | **confirmed** | memory promoted |
| HSCI byte 9 = `clk_lock` latch | not in A1 | yes (`bebob_maudio.c:196`) | **rechecked 2026-10-14: macOS kext writes S/MUX bit 5 here, not a lock latch** |
| Rate-change inter-command delay = 100 ms | memory | yes (`bebob_maudio.c:326`) | match |
| CIP-DBC quirk: empty pkt dbc += 8 at ≥88 kHz | not in A1 | yes (`bebob.c:174, bebob_stream.c:451`) | **new — needs A1 RX path recheck** |
| Post-attach bus reset to fix gap-count | not in A1 | yes (`bebob.c:289`) | **new — needs A1 lifecycle recheck** |
| Model_ID 0x10071 runtime / 0x10070 bootloader | memory | yes (`bebob.c:63, 460`) | match |
| Firmware upload mechanism | A4 full-binary | 3-cue trigger (v5058+ on-flash) | different scenarios; both valid |

## 8. What snd-bebob tells us that A1 missed

Three concrete gaps worth re-checking in Ghidra:

### 8.1 CIP empty-packet DBC+=8 at high sample rates (§4.3)

**Re-decompile target:** `InputDCLCallbackCopyPacketGroupToSamples` @ `_extracted/.../M-AudioFireWireBeBoB.x86_64:0x26e3e` and its DBC-continuity check (prologue at A1-Q1 lines 131–139 shows `and $0x3f` for segment index but doesn't show the dbc path yet). Specifically look for:
- `cmp` of received `dbc` against `(expected_dbc + 8) & 0xff` when `data_blocks == 0`
- Or a flag check that disables dbc validation on empty packets
- Or an `sfc >= 3` (88.2k and up) special case

**Why it matters:** if the clone's RX callback ever enforces strict dbc monotonicity (dbc[i+1] == dbc[i] + data_blocks[i]), it will spuriously stop streaming at ≥88 kHz. Linux handles this via `CIP_EMPTY_HAS_WRONG_DBC` flag; the clone needs the equivalent.

### 8.2 Post-registration / post-attach bus reset (§4.4)

**Re-decompile target:** `FWMetaNub::start` + `FW1814Device::start` + any `FWController::resetBus` call reachable from the attach path. In A1-Q7 (`FWMetaNub` 6-state machine) we captured the bootloader→runtime transition but didn't check whether runtime-attach then kicks a bus reset.

**Why it matters:** if the kext does kick a bus reset post-attach, our clone must too. If it doesn't, our clone's AV/C discovery may need an explicit reset the kext didn't bother with. Either way: current A1 is silent, and snd-bebob is explicit.

### 8.3 HSCI clk_lock byte (§4.6)

**Re-decompile target:** every call site of `FWHSCI::SendControlFrame` / vendor-dependent frame builder, inspecting whether byte 9 of the outgoing buffer is written and from which source. A1-Q6 captured the envelope framing but didn't check byte 9 vs 10/11 explicitly.

**Why it matters:** the panel's `MAClockSource` composite 32-bit key may be packing clk_lock into a bit we currently think is "unused padding" — losing it would mean our clone can't lock-latch the clock source against external disturbance, which may be the mechanism behind memory `project_v19_rate_change_while_streaming_panics` (clock-change while streaming → firmware confusion → upstream realloc page-fault).

### 8.4 Minor nice-to-haves

- `bebob_stream.c:70–90` shows that the device **can** report different rates on OUT vs IN plugs transiently, and the host **must mirror input to output** on read. Our clone's rate-get path (A1-Q5) should assert this invariant or at least log when it fails.
- `bebob_maudio.c:139–164 check_clk_sync` reads a 84-byte meter block at address `0xffc700000000 + 0x00600000` and interprets byte `[size - 2]` as the sync-SFC indicator (`0xff` = not synced). The macOS panel's "Clock Status" indicator is likely driven from the same meter block via `clientMemoryForType(1)` shared-ring (see memory `project_meter_path_shared_ring`). **Candidate verification:** the ring's trailing bytes carry sync status, matching snd-bebob's trailer-byte scheme. Worth a B5 shared-ring dump to confirm offset `84 - 2 = 82` holds the sync-SFC byte in our pulled frame.

## 9. Implications for Part C / D

### 9.1 For Part C synthesis

- **Lifecycle state machine (NEW_PLAN Part C §1):** add a mandatory post-attach bus-reset node to the lifecycle if A1 re-check per §8.2 doesn't find one in the kext. If it's absent in the kext, keep the Linux workaround in the clone — low cost, high robustness.
- **Comm path (NEW_PLAN Part C §3):** HSCI envelope documentation can now cite snd-bebob byte-for-byte, strengthening the provenance of memory `project_fw1814_hsci_envelope`. Add byte 9 (clk_lock) explicitly.
- **Transport/control (NEW_PLAN Part C §4):** rate-change ordering = OUTPUT → 100 ms → INPUT, confirmed from two sources (memory + snd-bebob). Lock in that specific delay as spec, not folklore.
- **Sync path (NEW_PLAN Part C §2):** AM824 label `0xCF` is the vendor idle signature; the clone must emit it for PLL lock. Section this away from the standard IEC 61883-6 `0x40` to prevent future "modernizing" edits.

### 9.2 For Part D decision

- B2 does not shift the clone-vs-modified-reference calculus set up by B3. The DCL ring (Q1) is the only hard macOS-specific question left and B2 can't speak to it — A1 remains the sole source and already told us the answer.
- B2 **adds** three dbc/bus-reset/clk-lock corner cases to the clone's bring-up checklist. Each is a few lines of code; none is architecturally expensive.
- B2 **validates** the maudio-special-firmware wire protocol against a second independent reverse-engineering effort, which raises confidence in A1-Q6 and B5 §10 to "two parallel sources agree".

### 9.3 Open follow-ups for B1 / B5

- **B1 (FFADO):** cross-check `libffado/src/bebob/maudio/` for the same 12-byte HSCI envelope and for any dbc/bus-reset quirks snd-bebob doesn't mention.
- **B5 (DTrace):** capture one FCP exchange on clock-source change on the SL reference Mac to verify byte 9 live. Confirm meter block layout matches snd-bebob's 84-byte / trailer-sync-byte scheme.

## 10. Provenance of this document

Every file:line referenced:

- `_refs/linux-snd-bebob/sound/firewire/bebob/bebob.c` — lines 63 (FW1814 model ID), 150–182 (quirks table), 184–296 (probe path, post-attach reset), 450–481 (device ID table).
- `_refs/linux-snd-bebob/sound/firewire/bebob/bebob_maudio.c` — lines 40–49 (bootloader cue constants), 78–85 (special_params struct), 119–126 (firmware-load cue send), 139–164 (meter + sync-check), 170–226 (12-byte HSCI envelope), 230–254 (ch_table + stream_formation_set), 258–297 (special_discover with is1814 flag), 300–337 (special_set_rate inter-command delay), 341–394 (clock-source control), 436–520 (digital-in iface control), 522–575 (digital-out iface control).
- `_refs/linux-snd-bebob/sound/firewire/bebob/bebob_stream.c` — lines 24–32 (rate table), 62–117 (get/set rate), 119–258 (clock-src discovery), 376–404 (break_both_connections), 406–465 (start_stream / init_stream), 500–591 (stream_reserve_duplex), 593–674 (stream_start_duplex), 870–906 (seek_msu_sync_input_plug), 908–957 (snd_bebob_stream_discover).
- `_refs/linux-snd-bebob/sound/firewire/cmp.c` — lines 25–38 (PCR bitfield constants), 65–71 (PCR address), 120–153 (cmp_connection_init via MPR read), 211–220 (ipcr_set_modify), 241–267 (opcr_set_modify), 294–321 (cmp_connection_establish via lock-compare-swap), 323–351 (cmp_connection_break).
- `_refs/linux-snd-bebob/sound/firewire/amdtp-am824.c` — lines 13 (CIP_FMT_AM), 16 (AMDTP_FDF_AM824), 93–94 (fdf set in set_parameters), 148–177 (write_pcm_s32, label `0x40` + sample), 209–220 (write_pcm_silence `0x40000000`), 295–321 (write_midi_messages, label `0x80` for NODATA).
- `_refs/linux-snd-bebob/sound/firewire/amdtp-stream.c` — line 646 (fw_iso_context_queue), 1200–1225 (RX packet processing), 1714 (fw_iso_context_create), 2080–2124 (amdtp_domain_start, dynamic queue sizing).
- `/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A1-ghidra/Q1-rx-dcl-callbacks.md` — DCL ring shape and OpcodeToStructSize.
- `/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A1-ghidra/Q1a-addtimestampedtransferpackets.md` — opcode numbering 1–14.
- `/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A1-ghidra/Q3-setupaudioengine-connectallplugs.md` — plug map + ConnectAllPlugs gate.
- `/Users/bernardosardinha/Documents/firewireDriver/docs/system-map/A1-ghidra/Q17-tpacketpool.md` — TPacketPool silence pre-fill (`0xCF`/`0x80`).
