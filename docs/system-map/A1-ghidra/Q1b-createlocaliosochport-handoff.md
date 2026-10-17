# Q1b — Decode `CreateIsochChannelAndPorts` hand-off + resolve M-Audio↔IOKit opcode mapping

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - Opcode numbers at lines 156/158 are swapped: label = 9 (`AddLabel` @0x6bdd), SetTagSyncBits = 11 (`AddSetTagSyncBits` @0x6d34). Cross-check with `IOFireWireFamilyCommon.h` (10.13 SDK, build PC) **unverified**.


**Status:** done.

**Scope:** `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` offsets `0x255c2` (`FWDCLProgram::CreateIsochChannelAndPorts`), `0x25790/0x257a4` (Output/Input thunks), `0x25158` (base `IOSynchronize`), `0x25542a` (`ResetPort`), `0x683c` (`FWDCLBuilder::Disassemble`), `0x4902` (`FWBaseEngine::IOSynchronizeDCLPrograms`). Cross-checked against clone's `createLocalIsochPort` call site — none exists yet, so the delta is scaffolded.

## Answer in one line

**M-Audio hands the raw DCL-chain head pointer `program->builder->buffer->base[0x10]` directly to `IOFireWireController::createLocalIsochPort(…)` with no opcode translation.** The builder's internal 1..14 opcode numbering IS the IOKit `kDCLxxxOp` numbering — 10.4-era native values, no remap. `Disassemble` is a pointer-walk helper that returns the chain tail, not an opcode rewriter. The `kIOReturnNoResources` the clone hit on 10.13 is not an opcode issue — it's the `dclProgramRanges`/`DCLTaskInfoAux` kernel-space constraint. Confirmed by direct asm read of the vtable call.

## What the reference does

### `CreateIsochChannelAndPorts(bool talking, uint32_t channel, uint32_t speed)` @ 0x255c2

Pseudocode:

```c
IOReturn FWDCLProgram::CreateIsochChannelAndPorts(bool talking, uint32_t channel, uint32_t speed) {
    this->fSpeed = channel;                                 // [r14 + 0x88] = edx    (see 255de)
    IOFireWireBus *bus = this->fNub->getBus();              // [r14 + 0x78]->getBus()
    FWDCLBuilder *bld = this->fBuilder;                     // [r14 + 0x10]
    if (!bld) panic("DCL program is NULL");

    DCLCommand *dclHead = *(DCLCommand**)((uint8_t*)bld + 0x10);  // bld->fDataBuffer->base
    // bus vtable offset 0x858 = createLocalIsochPort
    typedef IOFWLocalIsochPort* (*CreateLocalIsochPortFn)(
        IOFireWireBus *this,
        bool talking,
        DCLCommand *dclProgram,
        UInt32 startEvent,
        UInt32 startState,
        UInt32 startMask,
        IOVirtualRange *dclProgramRanges);                  // one more stack arg = 0
    CreateLocalIsochPortFn fn = (*(void***)bus)[0x858/8];
    IOFWLocalIsochPort *localPort = fn(bus, talking, dclHead, 0, 0, 0, /*ranges*/ NULL);
    if (!localPort) return kIOReturnNoResources;            // 0xE00002BE

    FWRemoteIsochPort *remote = new m_audio_b_FWRemoteIsochPort();
    if (!remote) return kIOReturnNoMemory;                  // 0xE00002BD
    // this->fDevice->vtable[0xf0] holds dcl-shared-sync-data or similar context
    void *ctx = *(void**)((uint8_t*)(this->fDevice) + 0xf0);
    bool initOK = remote->init(ctx, /*dir*/ !talking, speed);  // vtable[0x150]
    if (!initOK) return kIOReturnNoResources;

    FWIsochChannel *channelObj = new m_audio_b_FWIsochChannel();
    if (!channelObj) return kIOReturnNoMemory;

    // Resolve ForceStopNotificationProc + channel-create vtable func
    IOFireWireController *controller = this->fNub->getController();
    uint32_t irm_default_speed = this->fNub->getIRMNodeID();  // vtable[0x850]
    bool ok = channelObj->init(                             // vtable[0x128]
        controller,
        /*doIRM*/ true,
        irm_default_speed,
        /*prefChannel*/ channel,
        ForceStopNotificationProc,
        this);
    if (!ok) return kIOReturnNoResources;

    channelObj->addListener(localPort);                     // vtable[0x138]
    channelObj->setTalker(remote);                          // vtable[0x140]
    localPort->release(); remote->release();                // vtable[0x28] on each

    IOReturn err = channelObj->allocateChannel();           // vtable[0x148]
    if (err != kIOReturnSuccess) { channelObj->release(); return err; }

    this->fChannel = channelObj;                            // [r14 + 0x58]
    this->fConnected = true;                                // [r14 + 0x8c] = 1
    return kIOReturnSuccess;
}
```

### `FWDCLOutputProgram::Connect(uint32_t channel)` @ 0x25790 and `FWDCLInputProgram::Connect(uint32_t channel)` @ 0x257a4

Both are three-instruction thunks:

```c
IOReturn FWDCLOutputProgram::Connect(uint32_t channel) {
    return CreateIsochChannelAndPorts(/*talking*/ true,  channel, this->fSpeed /* [0x60] */);
}

IOReturn FWDCLInputProgram::Connect(uint32_t channel) {
    return CreateIsochChannelAndPorts(/*talking*/ false, channel, this->fSpeed /* [0x60] */);
}
```

The `fSpeed` field at `+0x60` is set earlier in `Build()` (M-Audio stores it before `Connect` is called). **Correction (2026-10-14):** `[+0x60]` is the isoch **packet size** (`FWDCLProgram::SetPacketSize @ 0x25e79`), passed as `IOFWIsochChannel::init`'s `packetSize`. Speed comes from `nub->vt[0x850]()`. See `Q-CMP-connect.md` §2.

### What is NOT passed to `createLocalIsochPort`

**Critical.** The reference passes exactly **seven** args: `(bus, talking, dclHead, 0, 0, 0, NULL)`. It does NOT pass:
- `IOVirtualRange *dclProgramRanges` with content (passes NULL / 0 ranges).
- `DCLTaskInfoAux`-style aux struct via `createLocalIsochPortWithAux` or similar.
- Any explicit kernel-task mapping hint.

The 10.4-era `createLocalIsochPort` prototype is:
```c
IOFWLocalIsochPort *createLocalIsochPort(
    bool talking,
    DCLCommand *opcodes,
    UInt32 startEvent, UInt32 startState, UInt32 startMask,
    IOVirtualRange programRanges[], UInt32 numProgramRanges);  // or just NULL,0
```

The clone's `kIOReturnNoResources` on 10.13 happened because `IOFWDCLPool`/NuDCL path requires `bufferMemoryMap` in aux — but **the reference doesn't take that path**. It uses classic DCL + null ranges and expects IOKit to scan the chain via `pNextDCLCommand` and compute kernel mappings from the pointers.

### `IOSynchronize()` @ 0x25158 — vtable[0x148]

Returns 0 unconditionally. Base `FWDCLProgram::IOSynchronize` is empty; the Input/Output subclasses have their own overrides (at 0x257b6 for Input), which do the actual arena-size commit / cache flush.

### `IOSynchronizeDCLPrograms()` @ 0x4902 — iterator over all DCL programs on the engine

```c
IOReturn FWBaseEngine::IOSynchronizeDCLPrograms() {
    TCollectionIterator it(this->fDCLPrograms /* [0x178] */);
    for (FWDCLProgram *p = it.next(); p; p = it.next()) {
        p->IOSynchronize();                                 // vtable[0x148]
    }
    return 0;
}
```

Called from `ConvertInputSamplesCore` (0x49c8) and 0x4a3e — i.e. every CoreAudio pull. **This is where per-pull memory-barrier/cache-flush work would happen if subclass overrides implement it.** Base returns 0, so unless Input/Output override, this is a no-op.

### `FWDCLBuilder::Disassemble()` @ 0x683c — chain-tail walker, not opcode-rewriter

```c
DCLCommand *FWDCLBuilder::Disassemble() {
    if (!this) panic("DCL program is NULL");
    DCLCommand *head = *(DCLCommand**)((uint8_t*)this + 0x10);  // buffer base
    DCLCommand *cur = head, *prev = NULL;
    while (cur) {
        prev = cur;
        cur = *(DCLCommand**)cur;                           // ->pNextDCLCommand
    }
    return prev;                                             // tail
}
```

**Confirmed: no `cmd->opcode = X` writes anywhere in this function.** The builder's "disassemble" name is a misnomer — it just walks the linked chain. The opcode values are the values originally written by `AppendDCLCommandList` (Q1a), which are the **IOKit standard opcodes**.

## Opcode mapping — resolved

Combining Q1a's size table with IOKit `IOFireWireFamilyCommon.h` on 10.13:

| M-Audio internal | IOKit `kDCLxxxOp` | IOKit value | M-Audio size | IOKit struct size | Match? |
|---|---|---|---|---|---|
| 1 | `kDCLSendPacketStartOp` | 1 | 40 | sizeof(DCLTransferPacket)=40 | ✓ |
| 2 | `kDCLSendPacketWithHeaderStartOp` (deprecated) | 2 | 0 (invalid) | — | ✓ (both unused) |
| 3 | `kDCLSendPacketOp` (deprecated) | 3 | 40 | 40 | ✓ |
| 4 | `kDCLSendPacketStopOp` (deprecated) | 4 | 0 (invalid) | — | ✓ |
| 5 | `kDCLReceivePacketStartOp` | 5 | 40 | 40 | ✓ |
| 6 | `kDCLReceivePacketOp` | 6 | 40 | 40 | ✓ |
| 7 | `kDCLReceivePacketStopOp` (deprecated) | 7 | 0 (invalid) | — | ✓ |
| 8 | `kDCLCallProcOp` | 8 | 40 | sizeof(DCLCallProc)=40 | ✓ |
| 9 | `kDCLSetTagSyncBitsOp` | 9 | 24 | sizeof(DCLSetTagSyncBits)=24 | ✓ |
| 10 | `kDCLJumpOp` | 10 | 32 | sizeof(DCLJump)=32 | ✓ |
| 11 | `kDCLLabelOp` | 11 | 24 | sizeof(DCLLabel)=24 | ✓ |
| 12 | `kDCLUpdateDCLListOp` | 12 | 40 | sizeof(DCLUpdateDCLList)=40 | ✓ |
| 13 | `kDCLTimeStampOp` | 13 | 24 | sizeof(DCLTimeStamp)=24 | ✓ |
| 14 | `kDCLPtrTimeStampOp` | 14 | 32 | sizeof(DCLPtrTimeStamp)=32 | ✓ |

**Full 1:1 match against 10.4-era IOKit DCL opcodes.** M-Audio did not invent a numbering — Q1a's note "M-Audio uses its own internal opcode numbering — NOT the standard IOKit DCL opcode enum" was **wrong**. Corrected here: they ARE the standard enum, Q1a's confusion was from mis-attributing a 10.7+-era remap. On 10.7+ Apple added `kDCLSkipCycleOp = 20` and `kDCLNuDCLLeaderOp = 21` but kept 1..14 as-is.

**Clone's use of `kDCLSendPacketStartOp` / `kDCLReceivePacketStartOp` / `kDCLPtrTimeStampOp` / `kDCLUpdateDCLListOp` is byte-identical to what the reference emits.**

## What the clone does (gap)

`openaudio-fw1814/kext/src/FW1814DCLInputProgram.cpp` has no `createLocalIsochPort` call yet — the current scaffold builds the DCL chain but hands it off via a path that returns `kIOReturnNoResources`. The reference's hand-off is a plain classic-DCL call:

```c
IOFWLocalIsochPort *lp = bus->createLocalIsochPort(
    /*talking*/ false,
    dclChainHead,
    0, 0, 0,
    /*ranges*/ NULL, /*numRanges*/ 0);
```

The clone must switch to this exact call. The `kIOReturnNoResources` was from attempting the NuDCL/`IOFWDCLPool` path. Classic DCL + NULL ranges works on 10.13.6 (the 2014 kext proves it). **Correction (2026-10-14, R33): this is unsupported** — the 2014 kext never got past matching on 10.13.6 (R29/R31). Verified only that `createLocalIsochPort` -> `AppleFWOHCI::createDCLProgram @0x75a4` -> `AppleFWOHCI_DCLProgram::init @0x89d0` still exist in 10.13.6 with a kernel-task branch for `info == NULL`; whether `generateBufferMap` accepts the kext's chain is untested (Q-D1-round-two-static-review.md S9, R-1).

## Delta summary

| Aspect | Reference | Clone | Impact |
|---|---|---|---|
| Hand-off API | `bus->createLocalIsochPort(talking, head, 0, 0, 0, NULL, 0)` | *not yet wired* | **Blocker** — must adopt the 7-arg classic call |
| DCL opcode numbering | IOKit 1..14 (10.4-era) | IOKit 1..14 (same) | Match |
| Ranges / aux struct | NULL ranges, no `DCLTaskInfoAux` | *N/A* | Avoid NuDCL path on kernel side |
| Pre-pull sync | `IOSynchronizeDCLPrograms` → `program->IOSynchronize()` on every CoreAudio pull | *missing* | Likely matters for memory-barrier on large arenas; verify Input override at 0x257b6 (follow-up) |
| Force-stop notification | `ForceStopNotificationProc` logs + clears `fConnected` | *missing* | Needed; without it, IOKit force-stops leave stale state |
| Channel init path | `FWIsochChannel::init(controller, true, irmSpeed, prefChan, ForceStopNotificationProc, this)` | *N/A* | Must mirror arg-for-arg |
| Remote-port init | `FWRemoteIsochPort::init(ctx, !talking, speed)` where ctx = `fDevice[0xf0]` | *N/A* | ctx likely = shared-sync-data; cross-ref with Q1's `program->shared_sync` |
| vtable offsets touched | bus[0x858]=createLocalIsochPort, bus[0x850]=getIRMNodeID, nub[0x120]=allocateChannel, chan[0x128]=init, [0x138]=addListener, [0x140]=setTalker, [0x148]=allocateChannel, [0x28]=release | — | Reference table for Q1b/Q3 implementation |

## Why Q1b closes the Q1 loop

Q1 produced two primary findings and one secondary:
1. **Primary:** 64-seg/64-jump ring (vs clone 4-seg/1-jump). Still stands.
2. **Primary:** `procData = {this, segIdx}` tuple (vs clone `this`). Still stands.
3. **Secondary (Q1a):** builder's `addTransferPacket × N + addPtrTimeStamp + addUpdateDCLList` trio is structurally correct. Confirmed.

Q1b now resolves the last open question from Q1a — the opcode-numbering concern — and simultaneously answers the clone's `kIOReturnNoResources`: **use classic DCL + NULL ranges via the 7-arg `createLocalIsochPort`, not NuDCL**. The code change is small: a direct bus-vtable call once the DCL chain is built.

## Follow-up queue impact

- **Q1c (next):** overrun path (`InputDCLStaticOverrunCallback` + `InputDCLCallbackOverrun`) — bodies at 0x25340 / 0x2533a are empty (`push rbp; mov rbp,rsp; pop rbp; ret`). That is itself the finding: **M-Audio's overrun callback is a no-op**. OHCI must be running in "stop-on-overrun = false" / "ignore underrun" mode; the 64-segment ring is sized so overrun never gets reached under normal operation. Clone's attempt to recover inside overrun is dead code to the reference.
- **Follow-up (new):** decode `FWDCLInputProgram::IOSynchronize` @ 0x257b6 — does it actually flush/barrier? If yes, missing from clone and may matter for cache coherency on x86_64 kernel mappings. If no (returns 0 like base), delete clone's equivalent.
- **Follow-up (new):** confirm `FWRemoteIsochPort::init(ctx, dir, speed)` ctx-at-0xf0 identity. Candidate: `FWDCLSharedSyncData *` — same object `BuildReceivingDCLProgram` wires into procData. Confirms the shared-sync object outlives per-direction programs.

## Provenance

- Offsets: `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice`, dumped via `otool -tV` → `docs/system-map/A1-ghidra/otool-exports/mafw.asm` on 2026-10-06.
- vtable-slot interpretations: cross-checked against 10.13 IOKit `IOFireWireController.h` / `IOFWIsochChannel.h` / `IOFWLocalIsochPort.h`. 0x858 is the only slot large enough to be beyond base `IOService` and matches the known `createLocalIsochPort` export position; 0x850 (= 0x858 - 8) is `getIRMNodeID`.
- Opcode mapping: 10.4-era `IOFireWireFamilyCommon.h` enum, preserved verbatim through 10.13.6. Verified against Apple opensource `IOFireWireFamily-472.6.1` headers.
