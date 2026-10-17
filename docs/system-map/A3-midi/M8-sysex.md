
> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). Parser constants (0xdd mask, 0x100 threshold, 0xffffff sentinel, running status at +0x21c) verified at 0x4bcb/0x4c11/0x4ce8/0x4d5b. Kext use of `fDestPortInSysex` and 0xF4/0xF5 behaviour remain hypotheses.

---
title: A3-M8 — Sysex handling (reassembly on both sides)
part: A3
status: done
date: 2026-10-06
---

# M8 — Sysex: parser state, flush, and TX bookkeeping

**Headline:** Sysex is tracked by **two independent bits of state**, one per direction. On TX, `fDestPortInSysex[portIdx]` is a per-port flag the plugin maintains for symmetry — it does **not** change ring-selection or framing. On RX, the sysex state is a `uint32_t` sentinel inside `TMIDIParser` (`remaining = 0xFFFFFF`), which forces every subsequent data byte to accumulate into the `tsScratch` MIDIPacket until either the 256-byte scratch overflows (synthetic `0xF7` flush) or a `0xF7` arrives naturally. **Running-status is suppressed** during sysex. **Realtime bytes (0xF8–0xFF)** interleave freely without disturbing sysex state.

## 1 — Why sysex is special

Standard MIDI message framing:

| Message | Status byte | Data bytes | Max length |
|---|---|---|---|
| Channel voice | `0x8n..0xEn` | 1–2 | 3 |
| System common | `0xF1..0xF6` | 0–2 | 3 |
| **Sysex** | `0xF0 .. 0xF7` | **arbitrary** | unbounded |
| System realtime | `0xF8..0xFF` | 0 | 1 |

Only sysex has unbounded length and only sysex can be interrupted mid-stream by realtime bytes. Any correct MIDI parser needs:
- A sysex-active state bit.
- A buffer with explicit overflow handling.
- A rule for whether realtime bytes break sysex (they must not).
- A rule for whether channel-voice status bytes break sysex (they do, in MIDI 1.0 — but the parser here handles it as "truncate with synthetic 0xF7 and restart").

This plugin's parser handles all four correctly.

## 2 — RX side: parser state

Reference `com_m_audio::TMIDIParser::ParseMIDIByte` (`a3-m345.log:452–546`, address `0x00004b9c`). The relevant fields in `TMIDIParser`:

```
+0x000  uint32_t remaining     // normal messages: 0, 1, 2
                                // sysex active:    0xFFFFFF (per log:530)
                                // mid-sysex flush: >0x100 marker (per log:505)
+0x110  MIDIPacket tsScratch   // 264-byte scratch — can hold up to 256 sysex bytes
+0x21C  uint8_t runStatus      // running-status byte (0 during sysex — see log:527)
```

### 2.1 — Sysex entry

```c
case 0xF0:                          // log:528–531
    *runStatus = 0;                 // kill running-status
    *remaining = 0xFFFFFF;          // sysex sentinel
    tsScratch.length = 1;
    tsScratch.data[0] = 0xF0;
    break;
```

Setting `remaining = 0xFFFFFF` is both an "active" bit (any positive value) and a count-remainder that the data-byte branch happily decrements without ever reaching 0 (as long as sysex continues).

### 2.2 — Sysex continuation (data bytes)

Data-byte branch (`log:479–487`):

```c
tsScratch.data[tsScratch.length] = byte;
tsScratch.length++;
(*remaining)--;
if (*remaining != 0 && tsScratch.length < 0x100)
    return 0;   // keep accumulating
*outTs = true;  // emit when either exhausted or 256-byte full
```

Max sysex burst **before an emission** is 255 data bytes plus the leading `0xF0` = 256 bytes = `MIDIPacket.data` capacity. The length check `< 0x100` is what forces the mid-sysex flush at the scratch-buffer limit.

### 2.3 — Sysex overflow — synthetic `0xF7` flush

If the parser is mid-sysex (`*remaining > 0x100` — the `0xFFFFFF` sentinel surrounds the normal data-byte decrement path) **and** a non-realtime status byte arrives (`a3-m345.log:504–513`):

```c
if (0x100 < *remaining) {
    tsScratch.data[tsScratch.length++] = 0xF7;  // synthetic end-of-sysex
    *outTs = true;
    *remaining = 0;
    if (byte == 0xF7)
        return 0;   // natural EOX, consumed
}
```

Two reasons this fires:
1. A real `0xF7` arrives after a 256-byte-flushed burst — the parser already flushed with a synthetic `0xF7`, so the real one is just consumed and discarded.
2. A status byte arrives mid-sysex (malformed stream per MIDI 1.0 but allowed by the plugin) — the current sysex is finalized with synthetic `0xF7`, then the function returns 1 to re-process the status byte (`log:515–519`).

**Downstream contract:** a receiver of the emitted packet sequence sees `0xF0 <data...> 0xF7` as the terminator of **every** sysex emission — the synthetic flush is transparent.

### 2.4 — Natural `0xF7`

If `0xF7` arrives inside the data-byte accumulation path (no — `0xF7` has the high bit set so it falls into the status branch), it's handled by the §2.3 code: the mid-sysex check fires, emits with synthetic `0xF7` (which equals the natural one at that length), returns 0.

### 2.5 — Realtime interleave during sysex

Realtime branch (`log:490–502`):

```c
if (0xF7 < byte) {
    if (!valid_realtime(byte))  // 0xDD mask: F8/FA/FB/FC/FE/FF
        return 0;
    rtScratch.length = 1;
    rtScratch.data[0] = byte;
    *outRt = true;
    return 0;   // does NOT touch *remaining, runStatus, or tsScratch
}
```

Sysex state is **completely preserved**. The realtime byte is reported via the `outRt` channel and the next data byte continues the sysex buffer. Correct per MIDI 1.0.

## 3 — RX side: `MIDIPacket` emission

When the parser emits a sysex packet via `*outTs = true`, `checkForMIDIInput` (`a3-m345.log:293–298`) does:

```c
if (gotTimestamped) {
    port->tsPacket.timestamp = AudioGetCurrentHostTime();
    TMIDIPacketList::AddPacket(packetList, &port->tsPacket);
    port->tsPacket.length = 0;   // reset for next message
}
```

The `tsPacket` scratch (which is the parser's internal `tsScratch` — see M6 §6) gets its `length` zeroed so the next message starts fresh. The `data[]` buffer is not cleared — length is the only cursor.

**Clone implication:** a sysex longer than 256 bytes arrives at the CoreMIDI client as **multiple back-to-back MIDIPacket records**, each `0xF0 ... 0xF7`, each 256 bytes. The client is responsible for reassembling. This matches the behavior of Apple's own built-in MIDI drivers and is the standard CoreMIDI contract.

## 4 — TX side: `fDestPortInSysex` is advisory

Full TX decode: M5 §2 + §5. The ivar `fDestPortInSysex` is a per-destination-port `uint8_t`:

```c
// On every incoming MIDIPacket (log:140–145):
if (fDestPortInSysex[portIdx] && (firstByte & 0x80))
    fDestPortInSysex[portIdx] = 0;   // mid-sysex interrupt
if (firstByte == 0xF0)
    fDestPortInSysex[portIdx] = 1;   // sysex start

// After the packet is chunked (log:214–222):
if (fDestPortInSysex[portIdx] && lastByte == 0xF7)
    fDestPortInSysex[portIdx] = 0;   // natural EOX
```

**Observation:** the flag is **set and cleared but never read** anywhere else in `send:viaPort:`. The chunk-size (3 bytes), ring selection (timestamp compare), backpressure (125/128), and timestamp update are all independent of the flag.

**Hypothesis — unverified:** the flag might be consumed by the kext selector-500 handler or by a FireWire-side scheduling decision — e.g. "do not fragment sysex across isoch cycles." The plugin binary has no further reader. **Verification path:** decode the kext's MIDI consumer in A1 Q-queue (deferred — kext selector 500/501 handlers not yet mapped), or DTrace the sysex ivar's value during a known sysex burst on the SL reference Mac.

**Clone implication:** if §4 verification shows the flag is truly unused on the FireWire wire side, our clone can skip the TX-side sysex bookkeeping entirely. For safety and bit-for-bit parity, keep it until kext decode confirms. **Marking the "drop the flag" option as deferred until A1 Q11-MIDI selector 500 lands.**

## 5 — Running-status interaction

Entry into sysex clears `runStatus` to 0 (`log:527`). Exit from sysex (either natural or synthetic `0xF7`) does **not** restore running-status. The MIDI 1.0 spec forbids running-status across sysex, so this is correct.

Entry/exit around realtime bytes does **not** touch `runStatus` — correct, realtime bytes don't break running-status.

Non-sysex status bytes with `(status & 0xF0) == 0xF0` (System Common, F1/F3/F6) also clear `runStatus` to 0 (`log:527`). Running-status is only preserved for channel-voice messages (0x80–0xEF).

## 6 — Relational diagram

```
                            Device  →  Kext (RX ingest)
                                       │
                                       ▼
                             (append raw bytes to port RX byte ring; no framing)
                                       │
    ============================= mapped memory boundary ====================
                                       │
                                       ▼
              Plugin tick ──▶ CheckForNewBytes ──▶ byte stream
                                                        │
                                           per-byte ─── ▼ ──────┐
                                           ParseMIDIByte  ◂─────┤  loop with
                                                        │        │  do/while
                                                        ▼        │  on return==1
                                          { outTs?, outRt? }     │
                                                        │        │
                                      ┌─ outRt=true ──▶ rtScratch → AddPacket (RT)
                                      │                           │
                                      ├─ outTs=true ──▶ tsScratch → AddPacket (NORMAL/SYSEX)
                                      │                           │
                                      └─ remaining=0xFFFFFF ──────┘  (sysex-active state)
                                                                 │
                                       MIDIReceived(ep, packetList)
                                                                 │
                                        ┌────────────────────────┘
                                        ▼
                                 CoreMIDI client
                              (may see multiple
                               packets per sysex)
```

```
                           CoreMIDI client ──▶ MIDISend
                                                 │
                                                 ▼
                                        MIDIDriverSend(ep, pktList)
                                                 │
                                                 ▼
                                      -[send:viaPort:] (M5)
                                                 │
                               per MIDIPacket    │
                                                 │
                                     ┌───────────┼───────────┐
                                     │           │           │
                              fDestPortInSysex   │   chunk & write ring
                                 set/clear  ─────┤   (ring choice NOT
                              (unused readers)   │    affected by flag)
                                                 ▼
                                   DigiFWMIDI ring in shared memory
                                                 │
    ============================= mapped memory boundary ====================
                                                 ▼
                                   Kext TX consumer → AV/C → device
```

## 7 — Clone implications

1. **Parser is a direct line-for-line port.** The `0xFFFFFF` sentinel, the `0x100` scratch-overflow threshold, the synthetic `0xF7` flush, the `0xDD` realtime mask, and the running-status suppression are mechanical. Reproduce exactly.
2. **TX sysex flag is a nop in the plugin.** Keep it until kext consumer is decoded; delete it if A1 confirms the kext does not read it.
3. **Sysex chunking at 3 bytes per shared-ring slot is identical to normal chunking** — no sysex-specific framing byte in the slot. The kext demarshaller must identify sysex by looking at the `0xF0`/`0xF7` sentinels in the byte stream it reconstructs from `data[length]` fields.
4. **CoreMIDI client API contract:** sysex > 256 bytes arrives as multiple MIDIPackets. Our plugin must preserve this. Any client that relies on single-packet sysex is already broken against Apple's own drivers.

## 8 — Open items

- **TX `fDestPortInSysex` reader on the kext side** — see §4. Hypothesis is "unused by the wire layer, maintained for symmetry." Deferred to A1 Q11-MIDI.
- **Sysex ≥ 65535 bytes:** `remaining` is 32-bit, starts at `0xFFFFFF` = ~16 MB — comfortably larger than any real sysex. No overflow concern.
- **System Common `0xF4`, `0xF5`:** undefined per MIDI 1.0; the parser `switch` at `log:528` falls through to the default (no action). **Hypothesis — unverified**: those bytes may be silently dropped or treated as 0-length status. Low-priority — undefined behavior at the input stage.
