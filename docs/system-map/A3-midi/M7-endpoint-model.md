---
title: A3-M7 — Endpoint model (entities, endpoints, port arrays)
part: A3
status: done
date: 2026-10-06
---

# M7 — Endpoint model: how FW1814's MIDI jacks surface as CoreMIDI endpoints

**Headline:** One `MIDIDevice` per hardware unit, with **exactly one `MIDIEntity`** per device. The entity's **source and destination endpoint counts come from the kext**, published as the IORegistry properties `MAMIDIInputPorts` and `MAMIDIOutputPorts`. Those two numbers determine the per-port arrays — endpoint-ref table, port-state table, sysex flags, timestamp watermarks. Count mismatch between kext and plugin is impossible by construction: the plugin always allocates to the kext's published numbers.

## 1 — Structural overview

```
MIDIDevice  "MAFWxxxx (GUID)"       ← persisted in CoreMIDI setup via MIDISetupAddDevice
     │                                 keyed by com.m-audio.FW.GUID property
     │
     └── MIDIEntity  (exactly one — see §3)
            │
            ├── fSourceEndpointCount source endpoints
            │       │
            │       └── each one has:
            │             • MIDIEndpointRef in fSourceEndpointArray[idx]
            │             • TMIDIPort       in fSourcePorts[idx]  (stride 0x240)
            │             • mapped region   via IOConnectMapMemory(0x400|idx)
            │
            └── fDestinationEndpointCount destination endpoints
                    │
                    └── each one has:
                          • MIDIEndpointRef in fDestinationEndpointArray[idx]
                          • TMIDIPort       in fDestinationPorts[idx]
                          • uint8_t         in fDestPortInSysex[idx]
                          • uint64_t        in fDestPortCurrentTimestamp[idx]
                          • mapped region   via IOConnectMapMemory(0x800|idx)
```

All six per-port parallel arrays are allocated by `rebuildEndpointArray` from the single pair of entity counts (`a3-m345b.log:71–117`).

## 2 — Count derivation chain

```
kext property               plugin consumer                CoreMIDI primitive
──────────────              ───────────────                 ─────────────────
"MAMIDIInputPorts"  (uint)   driverInstanceMatched:         MIDIDeviceAddEntity(…, numSources, …)
                     │
                     ▼       (via IORegistryEntryCreateCFProperties)
"MAMIDIOutputPorts" (uint)                                  MIDIDeviceAddEntity(…, …, numDestinations)
                     │
                     ▼
                   Entity is **persisted** in setup.  On subsequent plug-in,
                   fetchMIDIDeviceForService: finds the saved entity (see M2 §2)
                   and the entity's count is **not re-read from the kext** —
                   it's whatever was recorded at first-pairing time.
```

**Reality check:** this means if the kext ships one plug-count and later a firmware update changes it, the saved CoreMIDI entity will have stale counts until the user deletes the device from Audio MIDI Setup. **Hypothesis** — needs verification: the `err + 0x2A54U < 2` migration branch in `fetchMIDIDeviceForService:` (A3-M2 §2) might force a rebuild in that case. For FW1814 the count is hardware-fixed (2 in / 2 out — see §4), so this is not a live concern.

## 3 — Why **one** entity per device

FFADO's BeBoB stack models the FW1814 as a single MIDI subunit with internal routing. The plugin follows suit: `rebuildEndpointArray` calls `MIDIDeviceGetEntity(fDevice, 0)` (`a3-m345b.log:58`) — index 0, no iteration over entity count. If a second entity existed on a BeBoB device, this plugin would silently ignore it.

**Clone implication:** our kext must publish `MAMIDIInputPorts` + `MAMIDIOutputPorts` on the top-level `com_openaudio_FW1814Device` IOService node (not on child services), because that is the node CoreMIDI's `driverInstanceMatched:` matches against. Multi-entity models (e.g. splitting MIDI subsystems across virtual entities) are not supported by this plugin's shape.

## 4 — FW1814 endpoint count — expected values

From the FW1814 user guide and hardware: **1 × 5-pin MIDI In + 1 × 5-pin MIDI Out on the front panel**. Looking at auto-memory `project_fw1814_hardware_capabilities.md`: the FW1814 has only the physical MIDI In/Out pair on the breakout. So expected:

```
MAMIDIInputPorts  = 1
MAMIDIOutputPorts = 1
```

**Correction note vs. A3 README P-queue (2026-10-06):** the README M7 row speculates "expect **2 in / 2 out** for FW1814." That is likely **wrong** — the FW1814 has one physical DIN pair. The 2in/2out figure may have come from confusion with the FW410 (which does have 2in/2out). **Verification path:** read `MAMIDIInputPorts`/`MAMIDIOutputPorts` on the live SL reference Mac via `ioreg -l | grep -i mamidi` while the device is attached, OR grep the reference kext binary for defaults published when the properties are absent. **Marking unverified** until either check lands.

## 5 — `rebuildEndpointArray` — the per-session allocation

Full decode in `a3-m345b.log:4–167`, address `0x00002728`. Key relational facts:

**Lifecycle:** called once per `start`, and whenever endpoint counts change (none observed in practice). First thing it does is free all six per-port arrays (`log:36–57`), then re-reads counts from the entity. So it's safe to call repeatedly as a reset.

**Fatal failure mode (`log:80–82`):** if `calloc(4, numSources)` fails for `fSourceEndpointArray`, the function returns `-108` (= `0xFFFFFF94`) **without** having touched `fDestinationEndpointArray` (which was already written at `log:78`). Caller `-start` surfaces this via `NSLog("CannotstartMIDI (%lX) '%s'", err, mach_error_string(err))` (`a3-m345.log:63–64`). Clean enough.

**Zero-count fast path:** if `fDestinationEndpointArray` ended up NULL (either count was zero or calloc failed), the function skips the port-array allocation at `log:83` and returns 0 — endpoint arrays stay empty but no error. This is how 0-port entities are tolerated.

**Per-port Init call (sources):**
```c
MIDIEndpointSetRefCons(sourceEp, self, portIdx);         // log:126
fSourceEndpointArray[portIdx] = sourceEp;                // log:128
TUserMIDIPort::Init(&fSourcePorts[portIdx], conn,
                    /*isSource=*/true, portIdx);         // log:129
```

**Per-port Init call (destinations):**
```c
MIDIEndpointSetRefCons(destEp, self, portIdx);           // log:148
fDestinationEndpointArray[portIdx] = destEp;             // log:150
TUserMIDIPort::Init(&fDestinationPorts[portIdx], conn,
                    /*isSource=*/false, portIdx);        // log:151
fDestPortInSysex[portIdx] = 0;                           // log:154
fDestPortCurrentTimestamp[portIdx] = 0;                  // log:155
```

**Note the symmetry:** `refCon1 = self`, `refCon2 = portIdx` on *both* sides. The glue function (`_MAFireWireMIDIDICE_Send`) uses `refCon2` to recover `portIdx` directly (M5 §6). On the RX side, the plugin iterates `fSourceEndpointArray` by index so it already knows `portIdx` — the refCon is set for symmetry/debugging rather than necessity.

## 6 — Zeroing of per-port scratch

The port-state object (`TMIDIPort`, 0x240 bytes) is `operator_new__()`-allocated as an array (`log:86`), then explicitly zeroed in a loop:

```c
puVar13 = pvVar12 + 0x20;    // = &port->parser
do {
    *puVar13 = 0;                               // parser.remaining = 0
    *(uint8_t *)(puVar13 + 0x87) = 0;           // runStatus = 0  (0x20 + 0x21C = 0x23C offset into TMIDIPort)
    *(uint16_t *)(puVar13 + 0x46) = 0;          // tsScratch.length = 0  (0x20+0x118 = 0x138, = port+0x138)
    *(uint16_t *)(puVar13 + 0x03) = 0;          // rtScratch.length = 0  (0x20+0x0C = 0x2C, = port+0x2C)
    puVar13 += 0x90;                            // next port (0x240/4 = 0x90)
    bytesLeft -= 0x240;
} while (bytesLeft != 0);
```

(`log:88–96` for sources, `log:102–112` for destinations.)

Only four fields are zeroed — not the whole 0x240 bytes. This confirms that **`TMIDIPort` is a POD without a constructor** that gets a partial reset — the parser's remaining-byte counter, the running-status byte, and the two scratch packet lengths are the only fields that affect correctness; the rest are overwritten before use.

**Clone implication:** our clone can either (a) mirror the partial-reset shape, or (b) use `memset(ports, 0, numPorts * sizeof(TMIDIPort))` which is simpler and strictly correct. Prefer (b) — the partial-reset is a 2014 micro-optimization not worth preserving.

## 7 — Relational interface to M4 (shared-memory) and M5/M6 (data paths)

Endpoint model binds the four sibling arrays:
- `fSource/DestinationEndpointArray` ↔ **CoreMIDI handle** (consumed by `MIDIReceived` + glue refCon lookup).
- `fSource/DestinationPorts` ↔ **in-process port state** (parser + scratch packets + ring base pointers).
- `fDestPortInSysex` / `fDestPortCurrentTimestamp` ↔ **TX-only state** (consumed by `send:viaPort:`).

The kext-side counterpart: the user client's `clientMemoryForType` must honor both `0x400|idx` (RX byte ring per source) and `0x800|idx` (TX combined-ring per dest), for every `idx` in `[0, MAMIDIInputPorts)` and `[0, MAMIDIOutputPorts)` respectively. **If the kext changes those counts between a `stop` and a `start` call, `rebuildEndpointArray` is NOT called again** — the ring maps are stale until the plugin is reloaded. This is a design constraint: counts are a boot-time fact.

## 8 — Clone implications

1. **Publish `MAMIDIInputPorts` and `MAMIDIOutputPorts`** on `com_openaudio_FW1814Device` at `start()` time, before CoreMIDI matching. Values depend on §4 verification.
2. **One entity per device**, no multi-entity split.
3. **Per-port memType mappings must match** the port indexes implied by the counts. Mapping failure in `TUserMIDIPort::Init` aborts the whole `rebuildEndpointArray`, which the plugin surfaces as a NSLog but doesn't retry — kext must get it right first time.
4. **`refCon2 = portIdx` is load-bearing** for TX only — do not change the glue layer's refCon encoding without updating `send:viaPort:` accordingly.

## 9 — Open items

- **FW1814 port counts: 1in/1out vs 2in/2out** — unverified (see §4). Need either a live `ioreg` or kext-side decode.
- **Multi-entity BeBoB devices** (if any shipped) are silently truncated to entity 0. Not relevant for FW1814 but worth noting if we ever stub the plugin to drive FW410 or Ozonic.
- **Property stability across firmware reload** — the fwhelper reloads the device as "runtime" after bootloader upload (per CLAUDE.md M2). The resulting IOService must publish `MAMIDIInputPorts`/`MAMIDIOutputPorts` **before** CoreMIDI's `driverInstanceMatched:` fires. If there is a race, `fetchMIDIDeviceForService:` sees empty properties and the migration branch (A3-M2 §2) kicks in. **Hypothesis — unverified**: the migration branch is specifically designed for this race.
