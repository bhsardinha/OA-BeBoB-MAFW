---
title: A3-M5 — MIDI TX path (MIDIDriverSend → shared-memory ring)
part: A3
status: done
date: 2026-10-06
---

# M5 — TX path: `MIDIDriverSend` → `send:viaPort:` → DigiFWMIDI ring

**Headline:** CoreMIDI calls `MIDIDriverSend(ep, pktList)` → C++ glue thunks to `_MAFireWireMIDIDICE_Send` → finds device via `refCon1`/`refCon2` set at `MIDIEndpointSetRefCons` time → invokes Obj-C `-[MAFWMIDIDeviceBeBoB send:viaPort:]`. The method chunks each `MIDIPacket` into ≤3-byte groups, timestamp-compares against `fDestPortCurrentTimestamp[portIdx]`, writes into either the **realtime** ring (`port+0x18`) or the **timestamped** ring (`port+0x10`), and backpressures on ring-full. Device-side start/stop are IOKit scalar selectors **500** (start) and **501 = 0x1F5** (stop).

## 1 — start / stop selectors to the kext

`-[MAFWMIDIDeviceBeBoB start]` (`a3-m345.log:5–66`, `0x00002d47`):

```c
conn = [fKitDevice connection];
err = IOConnectCallScalarMethod(conn, 500, NULL, 0, NULL, NULL);
// If kIOReturnBusy (0xE00002D8), sleep(2s) and retry up to 30 times.
if (err == 0) {
    fStatusRunning = 1;
    now = CFAbsoluteTimeGetCurrent();
    timer = CFRunLoopTimerCreate(NULL, now, DAT_00007470 /* 666µs */, 0, 0, 0,
                                 HandleMIDIInputCallout, &ctx);
    fDataInputTimer = timer;
    CFRunLoopAddTimer(MIDIGetDriverIORunLoop(), timer, kCFRunLoopDefaultMode);
    CFRelease(timer);
    [fAsyncNotification retain];  // nil in production (see M3 §2)
    return 0;
}
```

`-[MAFWMIDIDeviceBeBoB stop]` (`a3-m345b.log:217–232`, `0x00002f00`):

```c
if (fStatusRunning) {
    fStatusRunning = 0;
    CFRunLoopTimerInvalidate(fDataInputTimer);
    [fAsyncNotification release];  // nil in production
    conn = [fKitDevice connection];
    IOConnectCallScalarMethod(conn, 0x1F5 /* =501 */, NULL, 0, NULL, NULL);
}
```

**Clone implication:** our kext `MAMIDIUserClient` (name **hypothesis** — see A3-M2 §4) must expose:
- Selector **500** = `startMIDI` — scalar method, 0 inputs / 0 outputs. Returns `kIOReturnBusy` if the FireWire isoch streams aren't up yet (plugin retries up to 60 s with 2 s sleep).
- Selector **501** = `stopMIDI` — scalar method, 0 inputs / 0 outputs.

Both are **synchronous**. They do not stand up or tear down shared memory — the mapping persists; these selectors only enable/disable the MIDI data flow through the already-mapped rings.

## 2 — `send:viaPort:` — per-packet dispatch

Signature (`a3-m345.log:102–229`, `0x00002f7d`):

```c
int -[MAFWMIDIDeviceBeBoB send:(const MIDIPacketList*)pktList viaPort:(uint64)portIdx];
```

`portIdx` is the destination port index that was baked into the endpoint via `MIDIEndpointSetRefCons(ep, self, portIdx)` in `rebuildEndpointArray` (`a3-m345b.log:148,151` for dest, `log:126,129` for source). CoreMIDI invokes the driver with `endpoint=self` → the glue reads `refCon2 = portIdx` and passes it here.

Per-`MIDIPacket` loop (`pktList->numPackets` iterations):

```c
for (uint16_t p = 0; p < pktList->numPackets; p++) {
    MIDIPacket *pkt = /* advance from pktList->packet[0] */;
    uint16_t nBytes = pkt->length;
    uint8_t  firstByte = pkt->data[0];

    // Sysex-state entry/exit on 0xF0 / 0xF7 — see §5
    if (fDestPortInSysex[portIdx] && (firstByte & 0x80))
        fDestPortInSysex[portIdx] = 0;
    if (pkt->data[0] == 0xF0)
        fDestPortInSysex[portIdx] = 1;

    // Chunk the packet into ≤3-byte groups
    for (int off = 0; off < nBytes; off += 3) {
        uint32_t chunkLen = min(nBytes - off, 3);
        uint64_t hostNs = AudioConvertHostTimeToNanos(pkt->timestamp);
        uint32_t chunkData;
        memcpy(&chunkData, pkt->data + off, chunkLen);

        // Choose ring by timestamp order relative to port's last-seen ts
        DigiFWMIDIRing *ring;
        if (hostNs < fDestPortCurrentTimestamp[portIdx]) {
            ring = (DigiFWMIDIRing*)(fDestinationPorts + portIdx*0x240 + 0x18);
            // realtime ring — do not advance fDestPortCurrentTimestamp
        } else {
            ring = (DigiFWMIDIRing*)(fDestinationPorts + portIdx*0x240 + 0x10);
            // timestamped ring — AND advance fDestPortCurrentTimestamp
        }

        // Backpressure (see §3)
        if ((ring->writeIdx - ring->readIdx) & 0x7F > 0x7D) {
            for (int tries = 100; tries > 0; tries--) {
                usleep(10000);
                if (!fStatusRunning) return TX_ABORT;
                if ((ring->writeIdx - ring->readIdx) & 0x7F <= 0x7D) break;
            }
        }

        // Write slot
        DigiFWMIDIPacket *slot = &ring->slot[ring->writeIdx];
        slot->hostTimeNanos = hostNs;
        slot->length        = chunkLen;
        memcpy(slot->data, &chunkData, 4);
        ring->writeIdx = (ring->writeIdx + 1) % 128;

        if (ring == timestamped_ring)
            fDestPortCurrentTimestamp[portIdx] = hostNs;
    }

    // Sysex-exit on packet last-byte == 0xF7
    if (fDestPortInSysex[portIdx] && pkt->data[nBytes-1] == 0xF7)
        fDestPortInSysex[portIdx] = 0;
}
return 0;
```

### Observations tied to log lines
- 3-byte chunk clamp: `log:152–154` — `if (3 < uVar14) uVar5 = 3;`.
- `AudioConvertHostTimeToNanos` call: `log:150`.
- Ring selection by `uVar6 < fDestPortCurrentTimestamp[portIdx]`: `log:157`.
- Slot write `*(piVar11+iVar13*4+2) = hostNs` and `*(piVar11+iVar13*4+4) = (chunkLen|chunkData<<32)`: `log:175–176` (realtime), `log:200–201` (timestamped). Note the `CONCAT44(uStack_34, uVar5)` packs `{length, data}` into the 8 bytes at slot offset `+0x08..+0x10`.
- `fDestPortCurrentTimestamp[portIdx] = hostNs` only on timestamped-ring path: `log:206`.

## 3 — Backpressure

Threshold: `(writeIdx - readIdx) & 0x7F > 0x7D` (`log:162`, `log:187`) — i.e. **≥126 pending slots** (so effective ring capacity = 125 of 128).

Loop: up to **100 iterations of `usleep(10000)`** = up to 1 second total wait per chunk.

Abort: if `fStatusRunning == 0` any time during the wait (`log:170`, `log:195`), set `uVar9 = 0xFFFFFF` (nonzero error flag, swallowed on return) and break. The function returns 0 regardless (`log:228`) — **the plugin never surfaces TX failure to CoreMIDI**.

**Clone implication:** our kext consumer must drain the TX ring at ≥1500 Hz. Given 125-slot capacity, we must consume ≥188 k slots/sec to avoid starvation under worst-case sustained MIDI stream — easily satisfied.

## 4 — Timestamp semantics

`fDestPortCurrentTimestamp[portIdx]` is a per-port watermark of "the latest host-nanos ever written to the timestamped ring." A new chunk goes to:
- **Realtime ring** if its timestamp is *earlier* than the watermark (i.e. would arrive out-of-order in a serialized stream). Realtime ring is drained *immediately* by the device-side consumer, regardless of time.
- **Timestamped ring** if its timestamp is *at or after* the watermark. Device consumer honors the timestamp and schedules playout.

This matches the two kext consumer helpers in M4 §4:
- `CheckForNewRealtimePacket` — pop & deliver always.
- `CheckForNewTimestampedPacket(base, window)` — pop only if `pkt.ts - base < window`.

**Why two rings?** MIDI Clock, Active Sensing, and Real-time System messages (0xF8–0xFF) are allowed by MIDI spec to interleave inside running-status messages — they must bypass the scheduled queue. The plugin lets any out-of-order chunk take the fast lane.

## 5 — Sysex state

`fDestPortInSysex[portIdx]` is a per-port `uint8_t` flag:
- Set to **1** when a `MIDIPacket` begins with `0xF0` (`log:143–145`).
- Cleared to **0** either at next packet start when `pkt->data[0] & 0x80` (indicates a non-sysex status byte entered mid-sysex, which is spec-compliant for Real-time messages) (`log:140–141`), **or** at packet end when last byte is `0xF7` (`log:220–221`).

The ring-write loop does **not** short-circuit based on sysex state — sysex continuation bytes are chunked into 3-byte groups and written just like any other MIDI data. State flag is used solely by **RX side** (per M8 — pending) for packet reassembly on the receive path, not here.

**Reality check:** the TX-side sysex flag has no effect on which ring is chosen or how bytes are chunked. It's maintained so that the RX reassembler can mirror it symmetrically on the receive side. **Hypothesis** — needs verification in M8.

## 6 — Call graph

```
CoreMIDI client            CoreMIDI framework       Plugin
  MIDISend(...)
    │
    ▼
  MIDIDriverSend(ep, pktList)
    │
    ▼  (vtable slot 5 on MIDIDriverInterface2 — see A3-M1 §4)
  MIDIDriverSend_stub ─▶ MAFWMIDIPluginGlueBeBoB::Send(ep, pktList, refCon2)
                                   │
                                   ▼
                            _MAFireWireMIDIDICE_Send(ep, pktList, refCon2)
                                   │
                                   ▼
                            device = refCon1  (set in rebuildEndpointArray)
                            portIdx = refCon2
                                   │
                                   ▼
                            objc_msgSend(device, @selector(send:viaPort:),
                                         pktList, portIdx)
                                   │
                                   ▼
                            -[MAFWMIDIDeviceBeBoB send:viaPort:]
                                   │
                                   ▼
                            per-chunk writes to shared-memory ring
```

Device & portIdx recovery uses `MIDIEndpointSetRefCons`-stored values (`a3-m345b.log:126,148`): `MIDIEndpointSetRefCons(ep, device, portIdx)` → `MIDIEndpointGetRefCons(ep, &refCon1, &refCon2)` → `refCon1 = device`, `refCon2 = portIdx`.

**Clone implication:** `MAFireWireMIDIDICE_Send` is the demarshal point from CoreMIDI-world (endpoint handle) to our Obj-C device (`refCon1`) and port (`refCon2`). Not optional — the glue cannot resolve `device` from `endpoint` any other way given the CFPlugIn boundary.

## 7 — Clone implications summary

1. **Kext user client selectors:** `500 = startMIDI`, `501 = stopMIDI`. Both scalar, 0-in/0-out, synchronous. `startMIDI` returns `kIOReturnBusy` while isoch streams are not yet established — plugin tolerates ~60 s of busy retries.
2. **TX ring consumer in kext:** drain at ≥1500 Hz from both rings per TX port. Prefer realtime ring first (unordered delivery to AV/C), then timestamped ring up to a scheduling window.
3. **AV/C bridge:** kext receives `{hostTimeNanos, length ≤3, data[4]}` tuples; must serialize onto the MIDI subunit's AV/C data channel at the configured sample rate.
4. **No TX async channel needed:** TX failures are not reported back to CoreMIDI; the plugin's own backpressure is the only surface. We can keep the kext TX path 100% buffer-based.
5. **Timestamp units:** nanoseconds, host-clock epoch (`AudioConvertHostTimeToNanos`). Kext must convert to its own FireWire cycle-time clock for AV/C scheduling.

## 8 — Open items / unverified

- **Selector 500 "busy" semantics:** plugin treats `kIOReturnBusy` as "wait and retry up to 30× 2 s" (`log:35–45`). **Hypothesis:** the kext returns busy if the isoch streams haven't come up yet, or if `fMIDIEnabled == 0` because the panel hasn't opened the audio side. Verification: trace selector 500 in the SL reference kext with DTrace, confirm return value gating condition.
- **Chunk-length cap at 3 bytes:** this matches the max length of a non-sysex MIDI message. **Hypothesis:** multi-byte sysex continuations are also chunked to 3 bytes per slot, each carrying a sub-chunk of the sysex payload. The kext just streams them in order. No evidence of a sysex-specific framing byte in the slot layout. Confirm by inspecting kext consumer in A1 Q11 selector mapping (deferred — selector 500 handler not yet decoded on the kext side).
- **4th byte of slot `data[4]`:** always written via `CONCAT44(uStack_34, uVar5)` where `uVar5=length`, so the top 32 bits of the 8-byte word are `length` and the bottom 32 are the raw packed data. Reader side (kext) interprets as `{length:u32, data[4]:u8*4}`.
