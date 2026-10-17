# Q-followups batch 1 — Ghidra decompile pass over Q4/Q6/Q7/Q11/Q12/Q13/Q14/Q15 open items

Resolves the Ghidra-tractable follow-ups queued since Q1. Each entry: finding → corrects/supersedes which prior Q, clone implication.

Source: `scripts/DecompFollowups1.java` → `ghidra-exports/decomp/followups1.log` (20/20 hits).

---

## Q15 correction — mixer apply uses ONE vtable slot (0xba8), not three

**Supersedes Q15 claim that FW1814Device has `0xbb8` SetInputRouting / `0xbc0` SetInputPan / `0xbc8` SetAuxMonitor.** All three concrete methods call the **same** base vtable slot `0xba8`, which is the generic "AV/C vendor-dependent write" primitive. The *selectors are parameters*, not vtable offsets.

```c
// SetInputPan     @ 0xdd3e: vtable[0xba8](this, uVar2*2 + 0x3e, 1, &this[0x12b8 + i*2])
// SetInputRouting @ 0xdbea: vtable[0xba8](this, 0x90,            1, &this[0x1308])
// SetAuxMonitor   @ 0xcfc4: vtable[0xba8](this, 0x9c,            1, &this[0x1314])
```

Field map on FW1814Device:
- `+0x12b8..+0x12db` — 18-entry pan table (per-input), ushort each; big-endian value `((-v>>1)&0xff) | (-v<<15)` form.
- `+0x1308` — 32-bit input-routing bitmap (byteswapped on write). Branches:
  - inputs 1..4 → bits `1<<(i-1)` + `1<<(i+3)` (two bits per channel = L/R enable)
  - input 5 → fixed bits `0x100`/`0x200`
  - inputs 6..9 → bits `1<<(i+2)` + `1<<(i+6)`
- `+0x1314` — 32-bit aux-monitor bitmap, one bit per source.

**Opcode space** on AV/C vendor-dependent write (`vtable[0xba8]`):
- `0x3e..0x60` ≈ input-pan slots (ch1..18)
- `0x90` = input routing register
- `0x9c` = aux monitor register

**Clone impact:** M5 mixer can collapse to **one** virtual (`SendMixerRegister(opcode, len, buf)`) + per-field setters. The three-vtable surface in current notes is wrong — delete those slot numbers from clone's `FW1814Device.h`.

---

## Q15 refinement — AdjustedLevels is per-channel scalar array, not seriatim

`BaseChannelGroup::AdjustedLevels()` @ `0x1f300`:
- Dereferences `this[+8]` (parent group ref, default = `this`).
- Count = `vtable[+0x10](this)` (channel count).
- For each `i`, pulls `Channel* = vtable[+0x18](this, i)` then calls static `AdjustedLevelForChannel(this, ch, isSolo, this[0x11], this[0x10])` → `uint32_t`.
- Returns `new uint32_t[count]` (array-new, caller frees).

`this[0x10]`/`this[0x11]` are the group's "mute all" / "any-solo-active" flags; `Solo(parentGroup)` is a bool query.

So the "snapshot → OSArray" step in Q15 is **more granular**: scalar array first, then `SeriatimValuesToOSArray` converts to `OSArray<OSNumber>` later (see below).

---

## Q15d — OSArrayToSeriatimValues / SeriatimValuesToOSArray

Confirmed symmetric pair on `m_audio_b_FWBaseArrayProp`:

```c
// 0x80cc: for each OSNumber in incoming OSArray (clamped to param_2 cap),
//   vtable[0x160](this, i, osnumber->unsigned32BitValue())  // "setter at index i"
// 0x824e: for i in [0, cap): v = vtable[0x158](this, i); OSNumber::withNumber(v, 32); array->setObject(num);
```

Vtable slots on `FWBaseArrayProp`:
- `+0x158` — `GetAtIndex(i) -> uint32_t`
- `+0x160` — `SetAtIndex(i, uint32_t)`

The property class exposes `uint32_t[]` through these two slots; `setProperties("Levels" => OSArray)` deserializes via `0x80cc`.

**Clone impact:** if we build the typed-property framework in M5 (not recommended per Q15), each ArrayProp needs two virtuals (`GetAtIndex`/`SetAtIndex`). Current recommendation (plain OSDictionary + `setProperties` override) sidesteps this entirely.

---

## Q13b — DiscreteEngine sub-engine ABI (bitmask barrier)

`ClipSubEngineOutput` @ `0x18884`:
```c
uint32_t pending = engine[+0x12c];
engine[+0x12c] = pending |= incoming_subengine_bit;   // param_1
if ((pending & engine[+0x128]) != engine[+0x128]) return 0;   // not all pair-members in yet
engine[+0x12c] = 0;                                           // reset barrier
return vtable[0xc38](this, firstSample, numSamples, fmt, stream);  // shared ClipMixedOutput
```

Same pattern for `ConvertSubEngineInput` @ `0x188e4` — bits in `+0x104 (26)` barrier, mask in `+0x100 (25)`, dispatches via `vtable[0xc40]`.

`ActivateSubEngine` @ `0x18c44` + `DeactivateSubEngine` @ `0x18b1e`:
- Keep a bitmask of active sub-engines in `engine[+0x128]`.
- First activation: `FWDiscreteDevice::vtable[0xa10]()` (device-level open), vtable[+0xa38] (core-activate), vtable[+0xc18] (direction-specific).
- Non-first (param_1 != 1): also calls `device::vtable[0xa18]()` on error unwind.
- On transition out of state 2 (bitmask stayed empty), skip stop paths.

**Confirms Q13 model:** the four CoreAudio-facing sub-engines each publish their own IOAudioEngine but all three Clip/Convert/Activate hops serialize into one shared engine via `[+0x118]`. Our clone's single-engine design already matches the barrier-collapsed path.

---

## Q11b — ActionStartMIDI / ActionSendMIDI / CoreStartMIDI / CoreStopMIDI

`ActionStartMIDI(engine)` → thin forward to `CoreStartMIDI(engine)`.
`ActionSendMIDI(engine, MAMIDIPacket*, uint*)` → forward to `FWBaseEngine::SendMIDI(packet, len)`.

`CoreStartMIDI` @ `0x717c`:
```c
if (engine[+0x234] > 0 && engine[+0x230] == 0) {   // has MIDI endpoints, not already started
    engine[+0x230] = 1;
    return 0;
}
return vtable[0xcf0](engine);   // hardware MIDI engines override
```

`CoreStopMIDI` @ `0x714a`:
```c
if (engine[+0x298] == 0) {                         // no hw-MIDI
    if (engine[+0x234] > 0 && engine[+0x230]) engine[+0x230] = 0;
    return;
}
vtable[0xd00](engine);                             // hw-MIDI stop
```

Field summary on FWBaseEngine:
- `+0x230` — MIDI started flag
- `+0x234` — MIDI endpoint count
- `+0x298` — hw-MIDI capability flag

**Clone impact:** FW1814 isoch-embedded MIDI stays on the software path (`[+0x298]==0`), identical to the 2014 driver's AM824 MIDI-quadlet injection.

---

## Q11 fu — FWUserClient::start + clientMemoryForType

`FWUserClient::start` @ `0x76de`:
```c
if (!super::start(provider)) return false;
this[+0x290] = safeMetaCast(provider, m_audio_b_FWAudioDevice::metaClass);
return this[+0x290] != 0;
```

**UC is attached to `FWAudioDevice`, not `FW1814Device` directly** — matches the Carbon panel open path (IOServiceOpen on IOFireWireAudioDevice, not the subclass).

`clientMemoryForType` @ `0x7220` — three ranges:
| `type` | source | size | semantic |
|---|---|---|---|
| `1` | `audioDev[+0x100]` | `0x1000` | MIDI shared ring (single) |
| `0x400..0x400+N` | `engine[+0x2b0] + 0x20 + i*0x28` | `0x1000` | per-input ring buffer |
| `0x800..0x800+M` | `engine[+0x2a8] + 0x20 + i*0x28` | `0` | control/metadata descriptor (zero-sized → caller only reads kernel VA) |

`N = engine[+0x29c]`, `M = engine[+0x2a0]`. All three paths `retain()` the IOMD before return.

**Clone impact:** M5+ userclient surface needs memory types `1`, `0x400+`, `0x800+` — these are the handles that `/_extracted/panel` and MIDI plugin map in userland. If we're skipping Carbon panel the `0x800+` region can be dropped; MIDI plugin still needs `1`.

---

## Q6a — HSCIReadComplete

`m_audio_b_FWHSCI::HSCIReadComplete(int status)` @ `0x1d80e`:
```c
if (status != 0) {
    this->readCommand->setDeferred(1000);  // vtable[+0x1d0]  — retry after 50 ms? (unit TBD)
    return;
}
auto* f = new TFunctor<void>();
f->vtable = &PTR__TFunctor_...;
f->ownerThis = this;
f->payload   = this->bufferHandle;   // [+0xa8]
TFunctor::Init(f, this, &HSCISafelyComplete);
UKernelCallHelpers::InvokeOnNewThread(f);  // dispatch off DMA callback
```

On success: wraps the 50 ms poll result into a Functor and hands off to the "CallCenter new-thread" worker which calls `HSCISafelyComplete` — this is where the 12-byte envelope decode runs out of hard IRQ. Matches Q6's model but adds the error-retry edge (status!=0 → re-arm, no listener fire).

---

## Q7a — FirmwareCacheInfoRegisters

`FWMetaNub::FirmwareCacheInfoRegisters(IOService* nub)` @ `0x1cc9e`:
```c
if (nub[+0xe0] == NULL) return kIOReturnNoDevice;
void* buf = &nub[+0x90];
bzero(buf, 0x50);
addr = { 0x0000, 0xFFFF, 0xC8020000 };
status = FireWireReadCommand(nub, &addr, 0x50, buf);
nub[+0xf0] = (nub[+0x9c] != 0);                // some capability flag
OSData* d = OSData::withBytes(buf, 0x50);
nub->setProperty("MAFirmwareInfo", d);         // IORegistry snapshot
```

So the firmware-info block (80 bytes at CSR `0xFFFF:0xC8020000`, note offset `0xC8020000` ≠ Q7's bootloader mailbox at `0xC8021000`) gets published as the `MAFirmwareInfo` OSData property on the nub. One of those bytes (`+0xc` into the block, since `nub[+0x9c] = buf[+0xc]`) sets `nub[+0xf0]` which the clone currently ignores.

**Clone impact:** publish same property to let the panel/MIDI plugin version-check without an FCP command.

---

## Q14b — FWAVCAudioDevice::setProperties routing

Confirmed: **no FW1814Device override**. Base `m_audio_b_FWAVCAudioDevice::setProperties` @ `0x22e28` only intercepts **one key**:

```c
OSDictionary* dict = safeMetaCast(incoming, OSDictionary::metaClass);
OSNumber* rate = dict->getObject("MASampleRate");
if (rate && engine[+0x8c] != rate->unsigned32BitValue())
    vtable[0xa78](this, newRate);   // sample-rate change
// everything else falls through to:
IOAudioDevice::setProperties(this, incoming);
```

The *rest* of the control-dict surface (levels, pan, routing, mute, solo, aux send) **does not go through setProperties at all** — it is pushed via the Mach-port + externalMethod path on `FWUserClient` (Q11 selectors) and `FWKIPCUserClient` broadcast (Q14).

**Clone impact:** M5 can expose "MASampleRate" as the one public setProperties key; mixer stays on externalMethod. This matches our already-designed userclient surface.

---

## Q12 surprise — FW1814Device inherits SupportsAC3() returning 0

`com_m_audio_FW1814Device` has **no** `SupportsAC3` override. Base returns 0 → the AC3 passthrough path decoded in Q12 is **compiled in but never advertised** on FW1814. FW410 is the only enabler.

**Supersedes Q12 implication** that FW1814 ships S/PDIF-AC3 passthrough. The hardware can do it (S/PDIF out exists), but the 2014 driver gates it off. If we need AC3 we must opt in explicitly, overriding `SupportsAC3`.

`com_m_audio_FWEngine2::StartAC3Output` @ `0x1185e` just toggles `vtable[+0xcf8]` (if hw-MIDI engaged, pause it) then `vtable[+0xd18]` (install AC3 clip proc). Mirrors Q12's `ActivateAC3Output`.

---

## Open follow-ups (deferred — not Ghidra-only)

| Ref | Item | Why deferred |
|---|---|---|
| Q5a | FFADO cross-ref for HSCI payload bytes 10..15 | external source |
| Q11a | `MAMIDIPacket` struct layout | panel binary (not kext) |
| Q12b | AV/C vendor opcode byte values | FFADO cross-ref |
| Q13 | ioreg verification on SL Mac | hardware |
| Q14a | Panel binary decompile | different binary |
| Q15a | OSSymbol key strings (`"Level_0".."Level_17"`, etc.) | string-ref search, lower priority |
| Q16 | FWRev1 vs FWRev2 silicon-generation deltas | optional |
| Q17 | TPacketPool allocator decode | optional |

---

## Provenance

- All decompiles: `scripts/DecompFollowups1.java` against `ghidra-project/fw1814-kext` on 2026-10-06.
- Raw log: `ghidra-exports/decomp/followups1.log`.
- Function table: `ghidra-exports/functions.tsv`.
