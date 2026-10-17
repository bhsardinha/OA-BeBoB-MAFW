---
title: A3-M6 — MIDI RX path (shared ring → TMIDIParser → MIDIReceived)
part: A3
status: done
date: 2026-10-06
---

# M6 — RX path: 1500 Hz tick → byte ring → parser → CoreMIDI

**Headline:** The RX path is **strictly polled** — no interrupts, no async port. Every 666 µs the `CFRunLoopTimer` fires `HandleMIDIInputCallout` → `-checkForMIDIInput`. That method loops over every source endpoint, drains the shared-memory **byte ring** via `TMIDIPort::CheckForNewBytes`, feeds each byte to `TMIDIParser::ReceiveMIDIByte` (which emits either a timestamped packet or a realtime packet via two output `MIDIPacket` pointers), and batches them into a single `MIDIPacketList` delivered via `MIDIReceived`. The parser rebuilds **both** timestamp and realtime streams from a **single input byte stream** — the kext does **not** have to pre-separate realtime bytes.

## 1 — Structural relationship to the TX side

| Direction | Shared-memory shape | Who writes | Who reads | Separation of realtime vs. timestamped |
|---|---|---|---|---|
| TX (host → device) | Two 128-slot packet rings per port (`+0x10` timestamped, `+0x18` realtime) | Plugin `send:viaPort:` | Kext consumer → AV/C | **Decided by sender** before write (host-time compare) |
| RX (device → host) | One 512-byte **byte** ring per port (`+0x00..+0x204`) | Kext FCP response handler | Plugin `TMIDIPort::CheckForNewBytes` | **Decided by receiver's parser** — in-stream recognition of `0xF8..0xFF` |

**Why asymmetric?** TX must preserve the host's intent (CoreMIDI already classifies realtime vs. scheduled). RX has no such classification upstream — the device emits a serial MIDI byte stream and the plugin reconstructs messages. The parser's state machine (`TMIDIParser::ParseMIDIByte`) is the single point of truth.

**Clone implication:** the kext RX code is simpler than TX code. It only has to append raw received bytes to a 512-entry ring and advance `writeIdx`. No message framing, no sysex tracking, no timestamping at ingest.

## 2 — Timer tick dispatcher

`HandleMIDIInputCallout(CFRunLoopTimer*, void *ctx)` (`a3-m345b.log:175–182`, `0x00002ee8`):

```c
objc_msgSend(ctx /* = MAFWMIDIDeviceBeBoB* */,
             @selector(checkForMIDIInput));
```

Timer context pointer is the device self; the timer is created in `-start` with `&local_58` → `ctx->target = self` baked at creation (`a3-m345.log:49`). Only indirection — ensures retain/release ordering matches the runloop lifetime.

**Runloop placement:** `MIDIGetDriverIORunLoop()` — the CoreMIDI framework's dedicated driver I/O thread. **Not** the main thread, not the matching thread. This is the same runloop used for `IOConnectSetNotificationPort` registrations by other BeBoB classes (A2-P2 panel path). So **all driver-side CoreMIDI activity for this plugin is serialized on one runloop** — no locking needed inside `checkForMIDIInput`, `send:viaPort:`, `rebuildEndpointArray`, or the delegate callbacks from `MAFWBeBoBServiceNotification`.

## 3 — `checkForMIDIInput` — the drain loop

Signature (`a3-m345.log:233–325`, `0x00002b1b`):

```c
void -[MAFWMIDIDeviceBeBoB checkForMIDIInput];
```

Pseudocode:

```c
TMIDIPacketList list[2048];   // 2 KB stack buffer (sizeof TMIDIPacketList = 1)
for (uint32_t sourceIdx = 0; sourceIdx < fSourceEndpointCount; sourceIdx++) {
    TMIDIPort *port    = &fSourcePorts[sourceIdx];   // stride 0x240
    MIDIEndpointRef ep = fSourceEndpointArray[sourceIdx];
    MIDIPacketListInit(list);

    MIDIMemVector vec;
    if (!port->CheckForNewBytes(&vec, 0xFFFFFFFF))   // 0xFFFFFFFF = no cap
        continue;                                     // nothing new

    // vec has 1 or 2 spans (ring wrap)
    for (int span = 0; span < vec.numSpans; span++) {
        for (uint32_t b = 0; b < vec.span[span].length; b++) {
            uint8_t byte = vec.span[span].ptr[b];
            bool gotTimestamped = false, gotRealtime = false;

            do {
                // Parser may emit BOTH outputs for one input byte if a
                // realtime byte (0xF8..0xFF) is embedded in a running-status
                // channel message. The outer do/while re-invokes the parser
                // once with the SAME byte to drain the second output.
                parser.ReceiveMIDIByte(byte, &gotTimestamped, &gotRealtime);

                if (gotTimestamped) {
                    port->tsPacket.timestamp = AudioGetCurrentHostTime();
                    TMIDIPacketList::AddPacket(list, &port->tsPacket);
                    port->tsPacket.length = 0;
                }
                if (gotRealtime) {
                    port->rtPacket.timestamp = AudioGetCurrentHostTime();
                    TMIDIPacketList::AddPacket(list, &port->rtPacket);
                    port->rtPacket.length = 0;
                }
            } while (parserReturnedOne);  // see §4 — parser signals re-invoke
        }
    }
    MIDIReceived(ep, list);
}
```

### Decoded field layout of `TMIDIPort` (= per-port state, 0x240 bytes)

Cross-reference `checkForMIDIInput` field offsets against `TUserMIDIPort::Init` writes:

| Offset | Field | Set by | Used by |
|---|---|---|---|
| `+0x00` | `uint8_t *byteRingBase` | `Init` (= mapped ptr) | `CheckForNewBytes` producer-idx read |
| `+0x08` | `uint32_t byteRingTail` | `Init` (= initial writeIdx snapshot) | `CheckForNewBytes` as consumer cursor |
| `+0x10` | `uint32_t *timestampedRingBase` | `Init` (= mapped ptr alias) | `CheckForNewTimestampedPacket` |
| `+0x18` | `uint32_t *realtimeRingBase` | `Init` (= ptr + 0x202*4) | `CheckForNewRealtimePacket` |
| `+0x20..+0x23F` | `TMIDIParser parser` | ctor | `ReceiveMIDIByte` state |
| `+0x24..+0x12F` | **`MIDIPacket rtPacket`** (realtime scratch, 264 B) | parser output | `MIDIReceived` |
| `+0x130..+0x23B` | **`MIDIPacket tsPacket`** (timestamped scratch, 264 B) | parser output | `MIDIReceived` |

Evidence for the scratch-packet offsets:
- `log:277` → `pMVar2 = (MIDIPacket*)(lVar4 + 0x130 + lVar12)` — timestamped packet lives at port+0x130.
- `log:278` → `pMVar1 = (MIDIPacket*)(lVar4 + 0x24 + lVar12)` — realtime packet lives at port+0x24.
- `log:297` → length-zero clear at `port+0x138 = tsPacket.length` after AddPacket (`MIDIPacket.length` field is at `+0x08`, so `+0x130+0x08 = +0x138` ✓).
- `log:303` → length-zero clear at `port+0x2c = rtPacket.length` (`+0x24+0x08 = +0x2c` ✓).

The parser (`+0x20`) is where `TMIDIParser::ReceiveMIDIByte` writes its state — `this = &port->parser`.

**Clone implication:** `TMIDIPort` is a 576-byte PODO (plain old data object) that fully encapsulates per-port state. No pointers to heap except the shared-memory base. One parser per port — stateful across calls, must not be shared.

## 4 — `TMIDIParser::ParseMIDIByte` — the byte-level state machine

Signature (`a3-m345.log:452–546`, `0x00004b9c`):

```c
int ParseMIDIByte(uint8_t byte,
                  uint32_t  *remaining,  // bytes still needed for current msg
                  uint8_t   *runStatus,  // last channel-voice status byte
                  bool      *outTs,      // set true if timestamped pkt ready
                  bool      *outRt,      // set true if realtime pkt ready
                  MIDIPacket *tsPkt,
                  MIDIPacket *rtPkt);
return value: 1 if the SAME byte must be re-processed (status interrupted),
              0 otherwise.
```

Behavior branches:

**Branch A — data byte (`byte & 0x80 == 0`):**
- If `*remaining == 0`: either running-status re-use (`*runStatus != 0`, construct a 2-byte Program Change / Channel Pressure, emit as ts), or ignore stray data.
- Else: append to `tsPkt.data[tsPkt.length++]`, decrement `*remaining`. If `remaining` reaches 0, emit ts packet.
- Running-status for Program Change / Channel Pressure sets `remaining = 1`; everything else sets `remaining = 2`. See the `switch` at `log:528` and the `(status & 0xE0) != 0xC0` ternary at `log:524`.

**Branch B — realtime byte (`0xF8 <= byte <= 0xFF`):**
- Only the bits set in `0xDD` (= `0b11011101`) are considered valid realtime status (System Reset `0xFF`, Active Sense `0xFE`, Clock `0xF8`, Start `0xFA`, Continue `0xFB`, Stop `0xFC`). `0xF9` and `0xFD` are undefined and dropped (`log:496`).
- Emit `rtPkt = {length=1, data[0]=byte}`, `*outRt = true`. Return 0.
- Crucially: this is **re-entrant inside sysex and running-status** — does not touch `remaining`, `runStatus`, or `tsPkt`. Sysex and running-status survive realtime interleaving.

**Branch C — status byte (`0x80 <= byte < 0xF8`):**
- If we're mid-sysex (`*remaining > 0x100`, see §5): close out current sysex with a trailing `0xF7` byte (`log:506`), emit ts, then re-process (`return 1`) so the status byte starts a fresh message.
- If current message partial (`tsPkt.length != 0`): discard partial, flag ts anyway, return 1 to re-process. Partial-message flush — safeguard against lost bytes.
- Channel-voice status (`< 0xF0`): save to `*runStatus`, set `remaining = 1 or 2`.
- System common `0xF0`: start sysex, `remaining = 0xFFFFFF` sentinel.
- System common `0xF1, 0xF3`: `remaining = 1`.
- System common `0xF2`: `remaining = 2`.
- System common `0xF6` (Tune Request): emit immediately as ts (0-arg status), `remaining = 0`.

### Why `0xFFFFFF` for sysex

The upper bits of `remaining` are used as a sysex-active indicator. Checks like `if (0x100 < *remaining)` (`log:505`) distinguish "sysex in progress, over 256 data bytes buffered" from "normal multi-byte message." When the sysex-growing branch overruns the 256-byte `MIDIPacket.data`, the parser forces a mid-sysex flush with a synthesized `0xF7` sentinel, letting the next chunk be a fresh continuation. See M8 for sysex reassembly rules.

## 5 — Parser interleaving — the `do { } while (ReceiveMIDIByte returned 1)` loop

`checkForMIDIInput` **does not** have an outer re-feed loop on the same byte. Looking at `log:289` closely:

```c
do {
    parser.ReceiveMIDIByte(byte, &gotTs, &gotRt);
    if (gotTs) { ... AddPacket(ts); ... }
    if (gotRt) { ... AddPacket(rt); ... }
} while (cVar5 != '\0');
```

The `cVar5` return comes from `ReceiveMIDIByte` → `ParseMIDIByte` → return 1 if a status-byte interrupt forced a flush-and-reparse. The do/while re-invokes with **the same byte** until the parser consumes it cleanly. This handles the "status byte lands mid-message" case: first iteration flushes the pending partial message as ts (returns 1), second iteration consumes the status byte as a fresh message start (returns 0).

**Clone implication:** faithful parser must honor the return-1-to-reparse contract. Our clone's `TMIDIParser::ReceiveMIDIByte` cannot be lossy on malformed input — the plugin's test corpus likely includes synthetic status-interrupt sequences.

## 6 — `TMIDIParser::ReceiveMIDIByte` — the thin wrapper

Signature (`a3-m345.log:550–568`, `0x00004d20`):

```c
void TMIDIParser::ReceiveMIDIByte(uint8_t byte, bool *outTs, bool *outRt) {
    *outTs = false;
    *outRt = false;
    ParseMIDIByte(byte,
                  (uint32_t*)this,         // this[0]       = remaining
                  (uint8_t *)(this+0x21c), // this[0x21c]   = runStatus
                  outTs, outRt,
                  (MIDIPacket*)(this+0x110),  // this[0x110] = tsPkt scratch
                  (MIDIPacket*)(this+4));     // this[0x04]  = rtPkt scratch
}
```

Parser state structure (`sizeof TMIDIParser = 0x220`, confirmed by `TMIDIPort` field spacing from `+0x20` to `+0x240`):

| Offset | Field |
|---|---|
| `+0x000` | `uint32_t remaining` |
| `+0x004..+0x10F` | `MIDIPacket rtScratch` (264 B) |
| `+0x110..+0x21B` | `MIDIPacket tsScratch` (264 B) |
| `+0x21C` | `uint8_t runStatus` |
| `+0x21D..+0x21F` | `uint8_t pad[3]` (align to 4) |

Wait — this conflicts with `TMIDIPort` layout where `port+0x24 = rtPacket`. Let me re-check: `port+0x20 = parser base`, `parser+0x04 = rtScratch`, so `port+0x24 = rtScratch` ✓. And `parser+0x110 = tsScratch` → `port+0x130` ✓. The `TMIDIPort` "scratch packets at `+0x24` and `+0x130`" are **the parser's internal scratch**, exposed as members only for the purpose of `AddPacket` reading them. Not two separate buffers; **shared with the parser**.

So the structural picture is tighter than §3 suggested:

```
TMIDIPort {
  +0x00  uint8_t  *byteRingBase;
  +0x08  uint32_t  byteRingTail;
  +0x10  uint32_t *timestampedRingBase;  // TX-oriented API, unused on RX source
  +0x18  uint32_t *realtimeRingBase;     // TX-oriented API, unused on RX source
  +0x20  TMIDIParser parser;   // 0x220 bytes, embeds the two scratch MIDIPackets
};  // total = 0x240
```

**Clone implication:** the clone's `TMIDIPort` is one object with the parser embedded by value, not pointer. One allocation per port.

## 7 — `MIDIReceived` and endpoint identity

The receiving endpoint handle is read from `fSourceEndpointArray[sourceIdx]` (`log:269`), which was stored at `MIDIEndpointSetRefCons` time in `rebuildEndpointArray` (`a3-m345b.log:128`):

```c
*(uint32_t*)(fSourceEndpointArray + sourceIdx*4) = sourceEndpointRef;
MIDIEndpointSetRefCons(sourceEndpointRef, self, sourceIdx);
```

So CoreMIDI needs:
1. The **endpoint handle** (`MIDIEndpointRef`) to route the packet list to subscribers → stored in `fSourceEndpointArray`.
2. The **port index** (`sourceIdx`) to look up the shared-memory ring → recovered either by iteration here, or by `MIDIEndpointGetRefCons` elsewhere.

The split lets `checkForMIDIInput` iterate directly over `fSourcePorts[sourceIdx]` without needing a reverse map from `MIDIEndpointRef` → `sourceIdx`.

## 8 — Relational summary across M1–M6

```
      ┌───────────────────────────────────────────────────────────────┐
      │ CoreMIDI framework (MIDIServer process)                       │
      │                                                                │
      │  Client A --MIDISend--┐                 ┌--MIDIReceived--┐    │
      │                       │                 │                │    │
      └─ MIDIDriverSend ──────┘                 └── delivers ────┘    │
                 │                                        ▲            │
   ====== CFPlugIn boundary ===========================================
                 │                                        │
                 ▼                                        │
   MAFWMIDIPluginGlueBeBoB::Send                MIDIReceived(ep, list)
                 │                                        ▲
                 ▼                                        │
   _MAFireWireMIDIDICE_Send(ep, pktList, refCon)          │
                 │                                        │
                 ▼                                        │
   send:viaPort:  (M5)                 checkForMIDIInput  (M6)
                 │                                        ▲
                 ▼                                        │
      per-port TX ring (M4)              per-port RX byte ring (M4)
       (0x800|idx mapping)                  (0x400|idx mapping)
                 │                                        │
   ========= IOConnectMapMemory boundary ============================
                 │                                        │
                 ▼                                        │
   kext IOUserClient::clientMemoryForType           (ditto)
                 │                                        │
                 ▼                                        │
   kext MIDI AV/C encoder                   kext MIDI AV/C decoder
                 │                                        │
   =============== FireWire isoch bus ================================
                 │                                        │
                 ▼                                        │
             device MIDI subunit                    device sends
            (plays scheduled bytes)                  byte stream
```

**Where the complexity lives:** entirely in the plugin. The kext's role for MIDI is three things: (1) selector 500/501 start/stop, (2) RX byte-ring producer, (3) TX ring consumer with timestamp-aware scheduling. All the state machine work (parser, ring dispatch, running status) is userspace.

## 9 — Clone implications

1. **Kext RX producer:** on each FCP MIDI event, append the byte(s) to the RX byte ring at `writeIdx`, then `writeIdx = (writeIdx + nBytes) & 0x1FF`. No parsing.
2. **Timer thread model:** our plugin re-uses `MIDIGetDriverIORunLoop()`. All serialization is one-thread; no locks needed in `MAFWMIDIDeviceBeBoB`.
3. **Parser is a direct line-for-line port.** The `0xDD` mask (`0b11011101`) and the running-status 1-vs-2 remaining-byte selection by `(status & 0xE0) == 0xC0` are mechanical — reproduce exactly.
4. **Buffer sizes:** TMIDIPort = 0x240 bytes, TMIDIParser = 0x220 bytes, byte ring = 516 bytes (one page). These are not negotiable — they are part of the ABI between parser output and `MIDIReceived` consumer.
5. **The `do { } while (parserReturnedOne)` re-invoke** must be preserved on both sides — kext does not need to care, but plugin must.

## 10 — Open items / unverified

- **Hypothesis:** `MIDIReceived` is called **once per source endpoint per tick** even if nothing was received, because `MIDIReceived(ep, emptyList)` is cheap. Looking at `log:275`: `if (cVar5 != '\0') { ... _MIDIReceived(...); }` — **no**, `MIDIReceived` is only called if `CheckForNewBytes` returned nonzero. Confirmed, not a hypothesis.
- **Timestamping on RX:** every delivered packet carries `AudioGetCurrentHostTime()` from the plugin tick (`log:294`, `log:300`), **not** the device's FireWire cycle time. So RX timestamps have ≤666 µs jitter from the actual byte arrival. **Clone implication:** we may be able to improve latency by having the kext stash a `mach_absolute_time()` at RX ingest and emit it in a sidecar byte, but that would change the shared-memory shape — **do not** without a corresponding plugin change. For parity, keep plugin timestamping.
- **Hypothesis (unverified):** the `TMIDIPort::CheckForNewRealtimePacket` and `TMIDIPort::CheckForNewTimestampedPacket` APIs (the ring-popping helpers decoded in M4) are **never called on the RX path** — RX uses only `CheckForNewBytes`. Those two methods are used by the **kext** (via a symmetric type-2 memType mapping) to drain TX, or by test tooling. Verification: xref callers of `CheckForNewRealtimePacket` within the plugin binary. (Deferred.)
