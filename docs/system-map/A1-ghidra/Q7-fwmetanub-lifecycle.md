# Q7 — `com_m_audio_FWMetaNub` lifecycle — bootloader ↔ runtime handoff

> **Correction (2026-10-15, M-FW build):** (1) `message()` (Ghidra `FWMetaNub__message.c`): on `0xE0000020` it copies `[+0xf1]` to `[+0xf2]`; then `vt[0x740](this,type,0,0)` is `IOService::messageClients(type,0,0)` and the second call, through the base-class vtable pointer, is `IOService::message(type,provider,arg)`. The 'global log hook' reading (line 65 and the 'two dispatches' paragraph) is therefore **wrong**: it is client forwarding plus the superclass handler. Implemented that way in `openaudio-fw1814/kext/src/FWMetaNub.cpp`. (2) **Live behaviour on 10.13.6 (build PC, 2026-10-15, FW1814 GUID d6c04…2337f):** powering the interface on enumerates a *bootloader* device (Model_ID 0x10070, GUID `…041…`, product `FW 1814 Bootloader`) and ~3 s later a separate *runtime* device (0x10071, GUID `…040…`); the bootloader auto-boots without any `Go`. A `MAFirmwareHalt` (Reset, start-mode 1) produced a new bootloader nub within ~3 s and the device returned to runtime on its own ~3 s later. So the bootloader window is ~3 s; the reference helper reacts within milliseconds to the nub notification. **Unverified:** whether the auto-boot timer is cancelled once the first download command arrives (matters for the update path: a 1,841-block download takes far longer than 3 s).

> **Correction (2026-10-14, M-A build):** `FWMetaNub` is **not** an `IOFireWireUnit` subclass (this file's "Answer in one line" and `CLAUDE.md` said so). Its constructor `__ZN21com_m_audio_FWMetaNubC1EPK11OSMetaClass` (`mafw.asm` 0x1d1b4..0x1d1c3) calls `IOService::IOService(OSMetaClass const*)` directly, then constructs the embedded `m_audio_b_FWKernelTimer` at `+0xf8`; the `MetaClass` ctor (0x1d318..0x1d33a) registers instance size `0x188`. The superclass is `IOService`; the provider is cast to `IOFireWireNub` (`safeMetaCast` then `IOFireWireNub::getController`, 0x1cfb2..0x1cfc6). The clone is `com_openaudio_FWMetaNub : IOService` (`openaudio-fw1814/kext/src/FWMetaNub.h`). Also read in the same pass: `kDeviceSpecs` @0x46450 = 8 entries {primary, alias, class}; the FW1814 entry is `"FW 1814"` / `"FW 1814 Bootloader"` / `com_m_audio_FW1814Device` (relocated pointers read from the Mach-O).

> **Correction (2026-10-14, P3/R37):** in the field table below `+0x98` (`fNodeHandle`, "written into `FireWireRequestHeader[0]`") is the cached **bootloader protocol version** (info-block word at offset 0x08, copied to `this+0x90` by `FirmwareCacheInfoRegisters`), `+0x9c` is the **bootloader version** (non-zero ⇒ `[+0xf0] = 1`), and `+0xf1` is the **update-in-progress** flag set by `FirmwareUpdate` (the `message()` handler copies it to `+0xf2`). Download state lives at `+0x138/+0x140/+0x148..+0x180` (Q-P3 §6). The kext-side firmware protocol is documented in `Q-P3-kext-firmware-path.md`.
> **Correction (2026-10-14) — state machine** (evidence: [PROVENANCE-RESOLUTIONS.md §R17](../PROVENANCE-RESOLUTIONS.md)). State machine verified in the asm: `fState [+0x88]` is written as 0 (StateString reset path, 0x1b89c), 1 in `start` (0x1d16d), 2 in `ReregisterDeviceAfterFirmwareReset` (0x1bbb6), 3 in `FirmwareStart` (0x1bebf), 4 in `FirmwareHalt` (0x1bffd, *before* the bootloader check at 0x1c007), 5 in `FirmwareUpdate` (0x1c924) — matching the six names. The 10 s timeout `0x989680` is set in **both** the write command (0x1bdea) and the read command (0x1c15b), not only writes. Lines 65 and 247 stay: 65 (`message` re-forwarded through the global at `0x14467c(%rip)`) is unresolved; 247 is refuted (see the earlier block).


> **Correction (2026-10-14) — firmware updater channel** (evidence: [PROVENANCE-RESOLUTIONS.md §R16](../PROVENANCE-RESOLUTIONS.md)). Line 247 ('Likely the user-space firmware updater talks via FWUserClient::externalMethod') is **refuted**: the helper drives firmware only with registry properties `MAFirmwareHalt`/`MAFirmwareUpgradeImage`/`MAFirmwareStart` via `setProperty:forKey:` (A4 H2 decompile) and reads `MAFirmwareInfo`; it uses no `externalMethod` for firmware.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - Line 65 ('presumably so the panel/prefpane can observe') and line 247 ('Likely the user-space firmware updater talks via FWUserClient::externalMethod') are **guesses — need verification**; line 247 is contradicted by A4-H2 (setProperty path). Check for 65 = resolve the global at `0x14467c(%rip)`.


**Status:** done.

**Scope:** symbols `__ZN21com_m_audio_FWMetaNub*` in `docs/system-map/A1-ghidra/otool-exports/mafw.asm` lines 32831–34060. Key methods: `StateString` @ 0x1b81e, `message` @ 0x1b9d6, `stop` @ 0x1ba84, `ReregisterDeviceAfterFirmwareReset` @ 0x1bba6, `ReregisterDeviceTimerFired` @ 0x1bc5e, `RegregisterDeviceThread` @ 0x1bcc2, `FireWireWriteCommand` @ 0x1bd3e, `FirmwareWriteCommand` @ 0x1be3c / 0x1be72, `FirmwareStart` @ 0x1beb4, `FirmwareReset` @ 0x1bf92, `FirmwareHalt` @ 0x1bff2, `FireWireReadCommand` @ 0x1c06e.

## Answer in one line

**`FWMetaNub` is a lightweight IOFireWireUnit-subclass that owns the device's firmware state machine.** It tracks six states (`nub start`, `initial-register`, `reregister`, `firmware-start`, `firmware-halt`, `firmware-update`) at `[+0x88]`, publishes IOKit, listens for `kIOFWMessageServiceIsRequestingClose = 0xE0000020` / `kIOFWMessageServiceIsResumed = 0xE0000030` messages, and after any firmware-reset write it arms a **2500 ms kernel timer** that spawns a worker thread which re-matches the device via `IOService::registerService`.

## State enum (`StateString` @ 0x1b81e)

| `[+0x88]` | Returned string | Meaning |
|---|---|---|
| 0 | `"nub start"` | just allocated, pre-`start()` |
| 1 | `"initial-register"` | first registration after probe |
| 2 | `"reregister"` | post-firmware-reset re-register (set by `ReregisterDeviceAfterFirmwareReset`) |
| 3 | `"firmware-start"` | `FirmwareStart` was invoked |
| 4 | `"firmware-halt"` | `FirmwareHalt` was invoked |
| 5 | `"firmware-update"` | mid-firmware-download |
| other | `"<unknown>"` | fallback |

## Field layout (`com_m_audio_FWMetaNub` instance, size `0x188`)

| Offset | Field | Semantic |
|---|---|---|
| `+0x88` | `uint32_t fState` | state enum (table above) |
| `+0x98` | `uint32_t fNodeHandle` | written into `FireWireRequestHeader[0]` on every FW write |
| `+0xe0` | `IOFireWireNub *fNub` | upstream nub (set by `start()`) |
| `+0xf0` | `bool fBootloaderActive` | true if device probed in bootloader mode (Model_ID 0x10070) |
| `+0xf1` | `bool fCached_IsRequestingClose` | copied into `[+0xf2]` on `0xE0000020` message |
| `+0xf2` | `bool fServiceClosing` | mirror of `+0xf1`, used by `message()` tail path |
| `+0xf3` | `bool fFirmwareMatchStarted` | sticky — set once `FirmwareStart` registered the metanub |
| `+0xf4` | `bool fStopped` | set in `stop()`; blocks `RegregisterDeviceThread` from acting |
| `+0xf8` | `m_audio_b_FWKernelTimer` (embedded) | 2500 ms re-register timer |
| `+0x108` | `IOWorkLoop *fWorkLoop` | timer's workloop |
| `+0x120` | `bool fTimerInitialized` | guards one-shot `Initialize()` call |
| `+0x128` | `TFunctor<void>` (embedded) | timer handler pointing at `ReregisterDeviceTimerFired` |

## `message(uint32_t sel, IOService *provider, void *arg)` @ 0x1b9d6

```c
IOReturn FWMetaNub::message(uint32_t sel, IOService *prov, void *arg) {
    bool stickyCached = this->fCached_IsRequestingClose /* [+0xf1] */;
    if (sel == 0xE0000020 /* kIOFWMessageServiceIsRequestingClose */) {
        this->fServiceClosing /* [+0xf2] */ = stickyCached;
        if (this->fNub) {
            uint32_t gen; uint16_t node;
            this->fNub->getNodeIDGeneration(gen, node, ...);
        }
    } else if (sel == 0xE0000030 /* kIOFWMessageServiceIsResumed */) {
        if (this->fNub) {
            uint32_t gen; uint16_t node;
            this->fNub->getNodeIDGeneration(gen, node, ...);
        }
    }
    // Dispatch twice: once via our own vtable[0x740] (parent::message),
    // then via the sibling vtable in global data (debug/log hook).
    (*this->vtable[0x740])(this, sel, 0, 0);
    g_logHook->vtable[0x740](this, sel, prov, arg);
    return eax;
}
```

The *two* dispatches are notable — the message is handled locally first (vtable[0x740] on the metanub) and then re-forwarded through a global service pointer (the "log hook" at `0x14467c(%rip)`), presumably so the panel/prefpane can observe FireWire-level service state changes.

## `stop(IOService *provider)` @ 0x1ba84

```c
void FWMetaNub::stop(IOService *prov) {
    OSObject *guidStr = this->CopyPropertyFrom(prov, "GUIDString");
    OSString *guid    = OSDynamicCast(OSString, guidStr);
    this->fStopped /* [+0xf4] */ = 1;
    IOService::stop(prov);              // via vtable[0x5d8]
    if (guid && sNubDictionary) {
        sNubDictionary->removeObject(guid);         // vtable[0x1e0]
        if (sNubDictionary->getCount() == 0) {      // vtable[0x130]
            sNubDictionary->release();              // vtable[0x28]
            sNubDictionary = NULL;
        }
    }
}
```

**A static `sNubDictionary` tracks every live metanub by GUIDString.** When the last one goes away the dictionary is torn down — classic class-level registry pattern. The `fStopped` flag prevents the re-register timer from firing after shutdown.

## `ReregisterDeviceAfterFirmwareReset()` @ 0x1bba6 — the handoff core

```c
IOReturn FWMetaNub::ReregisterDeviceAfterFirmwareReset() {
    this->fState /* [+0x88] */ = 2;         // "reregister"
    (*this->vtable[0x20])(this);            // retain
    IOWorkLoop *wl = (*this->vtable[0x688])(this);   // getWorkLoop()
    if (!this->fTimerInitialized /* [+0x120] */) {
        // First time: initialize the embedded kernel timer.
        TFunctor<void> *fn = &this->[0x128];
        if (!fn->Init(this, &FWMetaNub::ReregisterDeviceTimerFired)) goto out;
        if (!this->fTimer.Initialize(wl, fn))                        goto out;
    }
    // Arm/re-arm the 2500 ms single-shot timer:
    if (!this->fWorkLoop->vtable[0x160]()) this->fWorkLoop->vtable[0x150]();
    this->fWorkLoop->vtable[0x1d0](0x9c4);  // setTimeoutMS(2500)
out:
    return 0;
}
```

The `0x9c4 = 2500 ms` matches the "blank-slate" settle in `SetBlankSlateClockSource` (Q5). After any `FirmwareStart` / `FirmwareReset` / `FirmwareHalt` write to the device, the metanub waits 2.5 s for the hardware to drop + reappear on the bus, then runs `RegregisterDeviceThread` on a worker.

## `ReregisterDeviceTimerFired()` @ 0x1bc5e

Allocates a 32-byte `CallCenter { vtable, next=0, target=this, selector=&RegregisterDeviceThread }` and calls `UKernelCallHelpers::InvokeOnNewThread(center)`. Same off-workloop dispatch pattern as `BusReset` (Q4) — the actual re-match work happens on a dedicated kernel thread so the FireWire workloop stays free.

## `RegregisterDeviceThread()` @ 0x1bcc2 — runs on the worker

```c
void FWMetaNub::RegregisterDeviceThread() {
    if (this->fStopped /* [+0xf4] */)                                goto gone;
    if (this->fNub->isInactive())                                    goto gone;
    if (this->FirmwareCacheInfoRegisters(/*which*/NULL) != 0)        goto gone;
    IOLog("M-FW: Reregister: device still here, re-registering%c\n", '.');
    this->vtable[0x5b0](this, 0);        // IOService::registerService(0)
done:
    return (*this->vtable[0x28])(this);  // release — balances retain in ReregisterDeviceAfterFirmwareReset
gone:
    IOLog("M-FW: Reregister: device node vanished%c\n", '.');
    goto done;
}
```

Three go/no-go gates (`!fStopped`, `!isInactive`, `FirmwareCacheInfoRegisters == 0`), then `registerService(0)` — tells IOKit "re-run matching on me", which is how the kext transitions from bootloader (`com_openaudio_FW1814Bootloader`) to runtime (`com_openaudio_FW1814Nub`). The final `release` balances the `retain` in `ReregisterDeviceAfterFirmwareReset`.

## `FireWireWriteCommand(FWAddressStruct &addr, uint32_t len, void *data)` @ 0x1bd3e

```c
IOReturn FWMetaNub::FireWireWriteCommand(FWAddressStruct &addr, uint32_t len, void *data) {
    IOBufferMemoryDescriptor *buf =
        IOBufferMemoryDescriptor::withBytes(data, len, kIODirectionOutIn, false);
    if (!buf) return kIOReturnBusy;         // 0xE00002BC
    buf->prepare(kIODirectionNone);         // vtable[0x1f0]

    IOFWWriteCommand *cmd = new IOFWWriteCommand();
    if (!cmd) { buf->complete(); buf->release(); return kIOReturnBusy; }
    cmd->IOFWWriteCommand();

    FWAddressStruct hdr = { bus_hi=addr.bus_hi, node_hi=addr.node_hi, lo=addr.lo };
    bool ok = cmd->vtable[0x178/8 = initWithWriteGeneric](
        /*nub*/ this->fNub, /*gen*/ 0, /*&addr*/ &hdr,
        /*buf*/ buf, /*cb*/ NULL, /*ctx*/ NULL);
    if (!ok) { err = kIOReturnBusy; goto rel; }
    cmd->fTimeoutMicros /* [+0x48] */ = 0x989680;   // 10,000,000 µs = 10 s
    err = cmd->vtable[0x140](cmd, 0);               // submit(sync=0)
rel:
    cmd->release(); buf->complete(); buf->release();
    return err;
}
```

**10-second write timeout** is unusually long — matches the DM1000 bootloader's `0x989680` µs tolerance when the firmware image is being flashed. Note the write is submitted *async* (`submit(0)`), so the caller returns immediately and the firmware state machine advances through the download sequence driven by the response callbacks (`FirmwareReadResponse`).

## `FirmwareWriteCommand` wrapper variants

Two overloads build a 12-byte envelope and hand it to `FireWireWriteCommand`:

- **`FirmwareWriteCommand(MAFireWireRequestHeader *hdr, uint32_t len)` @ 0x1be3c:** overlays `hdr[0] = fNodeHandle[0x98]`, then builds `FWAddressStruct { bus_hi=0x0000, node_hi=0xFFFF, lo=0xC8021000 }` and calls FW-write.
- **`FirmwareWriteCommand(uint32_t a, uint32_t b)` @ 0x1be72:** 12-byte payload `[fNodeHandle, a, b]` to the same `0xFFFF:C8021000` address.

**The FireWire address `0xFFFF:0xC8021000` is the DM1000 bootloader request mailbox.** Every firmware-level command (start, reset, halt, download) is a quadlet-write to this register with a 12-byte header.

## `FirmwareStart()` @ 0x1beb4

```c
IOReturn FWMetaNub::FirmwareStart() {
    this->fState = 3;                        // "firmware-start"
    if (this->fBootloaderActive /* [+0xf0] */) {
        IOLog("M-FW: Bootloader active: starting firmware.\n");
        // Build the 12-byte "START" envelope:
        //   payload[0] = fNodeHandle, [1] = 0x01110000, [2] = 0
        //   addr = {bus=0, node=0xFFFF, lo=0xC8021000}
        IOReturn r = this->FireWireWriteCommand(addr, 12, &payload);
        this->ReregisterDeviceAfterFirmwareReset();   // arm 2500 ms timer
        return r;
    }
    if (this->fFirmwareMatchStarted /* [+0xf3] */) {
        IOLog("M-FW: Firnware active and we already started matching.\n");
        return 0;
    }
    IOLog("M-FW: Firmware active: registering metanub.\n");
    this->fFirmwareMatchStarted = 1;
    this->vtable[0x5b0](this, 0);            // registerService(0)
    return 0;
}
```

Two paths depending on `fBootloaderActive`: in bootloader mode we send the **START quadlet `0x01110000`** and wait for the device to reset; in firmware-active mode we just register the service once (and short-circuit on subsequent calls via the sticky flag). The log typo "Firnware" is a nice marker for recognising this build at runtime.

## `FirmwareReset(uint32_t subcode)` @ 0x1bf92

```c
IOReturn FWMetaNub::FirmwareReset(uint32_t subcode) {
    // 12-byte payload: [fNodeHandle, 0x01020000, subcode]
    //   addr = {0, 0xFFFF, 0xC8021000}
    IOReturn r = this->FireWireWriteCommand(addr, 12, &payload);
    this->ReregisterDeviceAfterFirmwareReset();
    return r;
}
```

Opcode `0x01020000` + subcode = soft reset. Caller supplies the subcode (0=cold, 1=warm, etc. — exact mapping TBD, needs panel-side cross-check).

## `FirmwareHalt()` @ 0x1bff2

```c
IOReturn FWMetaNub::FirmwareHalt() {
    this->fState = 4;                        // "firmware-halt"
    if (this->fBootloaderActive) return 0;   // already in bootloader
    // 12-byte payload: [fNodeHandle, 0x01020000, 0x00000001]
    IOReturn r = this->FireWireWriteCommand(addr, 12, &payload);
    this->ReregisterDeviceAfterFirmwareReset();
    return r;
}
```

Halt is `FirmwareReset` with subcode=1 — the "drop to bootloader" path. Fast-paths out if the device is already in bootloader mode.

## `FireWireReadCommand` @ 0x1c06e

Structurally symmetric to the write path: `IOBufferMemoryDescriptor::withCapacity(len)` → `new IOFWReadCommand` → async submit with 10-second timeout → copies response into caller buffer on completion. The read is used by `FirmwareReadResponse` to drain mailbox replies after each write (classic request/response bootloader protocol).

## Delta summary

| Aspect | Reference | Clone (expected) | Impact |
|---|---|---|---|
| State enum at `[+0x88]` with 6 named states | yes | TBD | Required for parity logging |
| 2500 ms re-register timer | `IOSleep`-equivalent via workloop setTimeout | TBD | Needed to let device drop + reappear |
| 10-second FW command timeout | `0x989680` µs on both read + write | TBD | Hardware-defined; firmware flash is slow |
| Bootloader mailbox address `0xFFFF:0xC8021000` | yes | TBD (clone has no fwhelper-side equivalent yet) | Required for FirmwareStart/Reset/Halt |
| START opcode `0x01110000` | yes | — | Hardware-defined |
| RESET opcode `0x01020000 \| subcode` | yes | — | Hardware-defined |
| Static `sNubDictionary` keyed by GUIDString | yes | TBD | Required if multiple FW1814s coexist |
| Two-stage message dispatch (local + global log hook) | yes | — | Likely optional for clone; worth noting |

## Follow-up queue impact

- **Q7a (new):** decode `FirmwareCacheInfoRegisters(IOService*)` — the gate function `RegregisterDeviceThread` uses to decide "is the device still here?". Likely does a quadlet read of config ROM; need the exact FW address + success criteria.
- **Q7b (new):** decode `MAFireWireRequestHeader` layout — the 12-byte envelope has `[fNodeHandle, opcode, operand]` structure but field names/semantics beyond that are hypothesis.
- Follow-up: trace who calls `FirmwareStart` / `FirmwareReset` / `FirmwareHalt`. Likely the user-space firmware updater talks via `FWUserClient::externalMethod` (Q11), which issues these on behalf of the panel/helper.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- `0x9c4 = 2500 ms`: immediate at 0x1bc45 in `ReregisterDeviceAfterFirmwareReset`.
- `0x989680 = 10,000,000 µs`: immediate at 0x1bdea in `FireWireWriteCommand`.
- Bootloader mailbox `0xFFFF:0xC8021000`: immediates at 0x1be4d/0x1be53 (and clones in Start/Reset/Halt).
- START `0x01110000`: immediate at 0x1befa; RESET `0x01020000`: immediates at 0x1bfa0 and 0x1c013.
- State strings `"nub start" / "initial-register" / "reregister" / "firmware-start" / "firmware-halt" / "firmware-update"` embedded in `StateString`'s literal pool (0x1b83e..0x1b872).
- Message selectors `0xE0000020` (RequestingClose) and `0xE0000030` (Resumed): immediates at 0x1b9e5 / 0x1b9ff — standard IOFireWire message constants.
