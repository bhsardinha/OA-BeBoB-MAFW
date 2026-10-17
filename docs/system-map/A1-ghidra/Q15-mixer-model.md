# Q15 — Mixer / typed-property / ChannelGroup framework

> **Correction (2026-10-14) — MA* crosswalk** (evidence: [Q-MAMAP-key-crosswalk.md](Q-MAMAP-key-crosswalk.md), [PROVENANCE-RESOLUTIONS.md §R20](../PROVENANCE-RESOLUTIONS.md)). For the FW1814 the mixer is **not** a single path: levels (SW return 0x00, output 0x08, input 0x10, headphone 0x38, aux output 0x34, aux send 0x64), input pan (0x40+), input routing (0x90), headphone selector (0x98) and aux monitor (0x9c) are vendor register writes via `CustomMixerWriteQuads` to `0xFFC7:0x00700000+offset`; **`MAAuxOutputPan2` (FWSettingsAuxSend::SetAuxPan → `AVCControlFeatureFunctionBlock`) and `MASWReturnRoutingV2` (base `FWSettingsLevels::SetSWReturnRouting` → `SetAVCRouting` → `AVCMixerConnectPair`) go over AV/C.** Mute/solo/dim are level operations (minimum / −10 dB). Level wire value = property level (−255..0, 0.5 dB) × 0x80, int16 big-endian, two channels per quadlet.


> **Correction (2026-10-14) — `0xBA9` / `0xBE9` are not AV/C opcodes.** They are Itanium-ABI virtual member-function pointers (vtable byte offset + 1) stored in batch functors by `SetupBatchChangeMethod @ 0xe184`. `0xBA9` → FW1814 `vt[0xba8]` = `CustomMixerWriteQuads @ 0x22c7a`, a direct async block write to `0xFFC7:0x00700000 + offset`. `0xBE9` → `vt[0xbe8]` = `FW1814Device::AVCControlSetLEDStatus` (A/B-switch LEDs). There is **one** mixer write path (direct register writes, same window as FFADO/macfw/Rust). Group offsets `0x00/0x08/0x10/0x34/0x38` match FFADO `special_avdevice.h:55–69`. Full decode: `A1-ghidra/Q15f-mixer-control-plane-wire.md` Correction 2026-10-14. Any "AV/C batch opcode 0xBA9" or "two mixer paths" statement below is superseded.

**Status:** done.

> **Correction (2026-10-06, batch1 follow-up):** the "three distinct vtable slots `0xbb8/0xbc0/0xbc8`" claim below is **wrong**. All three mixer concretes (`SetInputPan`, `SetInputRouting`, `SetAuxMonitor`) call the **same** FW1814Device vtable slot — `0xba8` — which is the generic AV/C vendor-dependent write primitive. The *selector byte* is a parameter (0x90 routing, 0x9c aux, 0x3e..0x60 pan), not an index. The call shape is `vtable[0xba8](this, selector, 1, &register_field)`. See `Q-followups-batch1.md` for the authoritative decode. Treat the slot numbers in sections below as superseded.


**Scope:** `FWSettings1814Levels`, `FWSettings1814AuxSend`, `FWSettingsLevels` (base), `m_audio_b_FWChannelGroupProperty`, `m_audio_FWb::BaseChannelGroup`, `m_audio_FWb::ConcreteChannelGroup<N>` for N∈{2,4,6,10,14,18,22}, `m_audio_FWb::GroupChannelGroup`.

## Answer in one line

**The mixer is a typed-property model where the FW1814 device publishes 1 "Levels" dict (4 channel-groups × 4 property-types = 16 ChannelGroupProperty objects, plus 2 group-umbrellas and 4 ArrayProps) and 1 "AuxSend" dict (4 ChannelGroupProperty + 2 ArrayProp).** Each `FWChannelGroupProperty` binds `(key, channelGroup, channelMask, optionalSecondGroup)`; when the panel writes an OSArray into the property, `CallSetter` snapshots current levels, unpacks the array into per-channel slots on the group(s), and fires the group's bound hardware-apply TFunctor — which goes through a `FWAVCAudioDevice::*` member-function pointer stored at `FW1814Device[+0x38]` to issue the actual AV/C vendor command batch.

## Object sizes & layouts

### `FWSettings1814Levels` — ≥ 0xa50 bytes

```text
+0x000  vtable (final: PTR_SendToDevice_000366c0; ctor transient: _00048e60)
+0x008  OSMetaClass* self-ref (used by safeMetaCast)
+0x010  m_audio_b_FWArrayProp  #1       (96 B)   — per-channel array A
+0x070  m_audio_b_FW32BitNumberProp #1  (64 B)   — scalar A
+0x0b0  m_audio_b_FW32BitNumberProp #2  (64 B)   — scalar B
+0x0f0  m_audio_b_FW32BitNumberProp #3  (64 B)   — scalar C
+0x130  ChannelGroupProperties #1 (480 B = 4×FWChannelGroupProperty @ 0x78 ea.)
+0x310  ChannelGroupProperties #2
+0x4f0  ChannelGroupProperties #3
+0x6d0  ChannelGroupProperties #4
+0x8d0  GroupChannelGroup "all A"  (0x30 B incl. 2× TFunctor + enabled-flag @ +0x8e2)
+0x930  GroupChannelGroup "all B"  (0x30 B incl. 2× TFunctor + enabled-flag @ +0x942)
+0x990  m_audio_b_FWArrayProp #2        (96 B)
+0x9f0  m_audio_b_FWArrayProp #3        (96 B)
```

**4 channel-groups × 4 properties each = 16 ChannelGroupProperty** instances. Plus 2 `GroupChannelGroup` "umbrella" collections and 4 raw `FWArrayProp` for simple per-channel scalar arrays (likely input gain trim + headphone + output trim + something).

Each `ChannelGroupProperties` holds exactly 4 `FWChannelGroupProperty` sub-objects at offsets `[0, 0x78, 0xf0, 0x168]` — so each `FWChannelGroupProperty` is **0x78 = 120 bytes**. Likely layout: Volume / Pan / Mute / Routing per group.

### `FWSettings1814AuxSend` — ≥ 0x2c0 bytes

```text
+0x000  vtable (final: PTR_SendToDevice_00036670)
+0x020  FWChannelGroupProperty #1 (120 B)   — Aux1 send
+0x098  FWChannelGroupProperty #2            — Aux1 pan
+0x110  FWChannelGroupProperty #3            — Aux2 send
+0x188  FWChannelGroupProperty #4            — Aux2 pan
+0x200  m_audio_b_FWArrayProp #1  (96 B)    — aux master A
+0x260  m_audio_b_FWArrayProp #2  (96 B)    — aux master B
```

Simpler, flat structure. 2 aux buses × (send + pan) + 2 master arrays.

### `m_audio_b_FWChannelGroupProperty` — 0x78 bytes

```text
+0x00  vtable
+0x08  OSSymbol* key (property name as seen by panel)
+0x2c  numChannels (uint, used by OSArrayToSeriatimValues)
+0x60  m_audio_FWb::BaseChannelGroup* primary group
+0x68  uint channelMask (which channels this property applies to within the group)
+0x70  m_audio_FWb::BaseChannelGroup* optional second group (NULL if single-group)
```

### `m_audio_FWb::ConcreteChannelGroup<N>` — 0x48 + N × 0x20 bytes

```text
+0x00  vtable (differs per N — e.g. PTR__ConcreteChannelGroup_00035390 for N=2,
                                     _000354d0/_00035430 for N=6)
+0x08  parent pointer (NULL in bare ctor)
+0x10  bool flag A
+0x11  bool flag B
+0x12  bool enabled (set to 1 at end of ctor)
+0x20  TFunctor<get>  (vtable=_000354d0 or _000366c0 depending on parent)
+0x30  TFunctor<set>  (vtable=_00035500 or _00036780)
+0x48 + i*0x20   per-channel slot i (for i=0..N-1), each 0x20 bytes
```

- `ChannelAtIndex(i)` is inlined: `return this + 0x48 + i*0x20` (with a `< N` clamp, else returns slot 0).
- Each channel slot is 32 bytes; of these, 24 are populated by the ctor loop (`+0, +8, +16` zeroed) — so the layout per slot is likely `{currentValue:u64, adjustedValue:u64, metadata:u64, padding:u64}`.
- N instances observed: **2, 4, 6, 10, 14, 18, 22**. These match:
  - N=2: a stereo pair (any aux send, S/PDIF, Audiophile mains)
  - N=4: Audiophile analog ins
  - N=6: Audiophile outs
  - N=10: FW1814 analog outs
  - N=14: FW1814 inputs with ADAT-out-only mode
  - N=18: FW1814 inputs with ADAT in (= 8 analog + 8 ADAT + 2 S/PDIF)
  - N=22: FW1814 outputs with ADAT out (= 2 S/PDIF + 2 phones + 10 analog + 8 ADAT) → wait, more likely 2+10+8 = 20 and the extra 2 are AuxMonitor sends, so 22 total surfaces

### `m_audio_FWb::GroupChannelGroup` — 0x60 bytes

Union-of-channel-groups abstraction. Holds 2 TFunctors (`+0x20`, `+0x30`) + flag at `+0x12` + vtable at `+0x30` (`PTR__GroupChannelGroup_00047fc0`). Lets a single property apply across multiple ConcreteChannelGroups (e.g. "set all inputs"). Used twice at `[+0x8d0]` and `[+0x930]` in `FWSettings1814Levels`.

## The hardware-apply path

### `FWSettings1814Levels::SetInputPan(ch, val)` @ 0xd128

```c
void FWSettings1814Levels::SetInputPan(FWSettings1814Levels *this, uint ch, uint val) {
    FW1814Device *dev = (FW1814Device*)OSMetaClassBase::safeMetaCast(
                            this[+0x08], &com_m_audio_FW1814Device::gMetaClass);
    (*dev->vtable[0xbc0])(dev, ch, val);     // vtable slot 0xbc0 / 8 = index 376
}
```

### `FWSettings1814Levels::SetInputRouting(ch, val)` @ 0xd15c

```c
    (*dev->vtable[0xbb8])(dev, ch, val);     // slot 0xbb8 (= 3000)
```

### `FWSettings1814AuxSend::SetAuxMonitor(ch, val)` @ 0xd0b4

```c
    (*dev->vtable[0xbc8])(dev, ch, val);     // slot 0xbc8
```

**FW1814Device vtable slots used by the mixer:**

| Slot (byte offset) | Method | Signature |
|---|---|---|
| `0xbb8` | `SetInputRouting` | `(FW1814Device*, uint ch, uint route) → int` |
| `0xbc0` | `SetInputPan` | `(FW1814Device*, uint ch, int16 pan) → int` |
| `0xbc8` | `SetAuxMonitor` | `(FW1814Device*, uint auxBus, uint level) → int` |

These are concrete per-control pokes. The property model uses them for **individual-channel writes**; the batch path (below) uses a different route for whole-group apply.

## Property setter path — `FWChannelGroupProperty::CallSetter`

```c
int FWChannelGroupProperty::CallSetter(FWChannelGroupProperty *this,
                                        OSSymbol *key, OSObject *valueArray) {
    void *beforeA = BaseChannelGroup::AdjustedLevels(this[+0x60]);
    if (!beforeA) return 0;                 // group not ready yet
    void *beforeB = this[+0x70] ? BaseChannelGroup::AdjustedLevels(this[+0x70]) : NULL;

    // Unpack the OSArray values into per-channel slots on this (array-prop base).
    int err = FWBaseArrayProp::OSArrayToSeriatimValues(this, valueArray, this[+0x2c] /*nCh*/);
    if (err == 0) {
        // Virtual slot +0x28 on BaseChannelGroup = "ApplyAdjustedLevels(before)" → fires hardware.
        (*this[+0x60]->vtable[0x28])(this[+0x60], beforeA);
        delete[] beforeA;
        if (this[+0x70]) {
            (*this[+0x70]->vtable[0x28])(this[+0x70], beforeB);
            if (beforeB) delete[] beforeB;
        }
    }
    return err;
}
```

**The two-group pattern:** some controls (notably **pan**) are inherently stereo-paired — the left and right channels live in different ConcreteChannelGroups, so one property write has to touch both. The second-group field at `[+0x70]` is the "paired-stereo other side."

**`BaseChannelGroup::AdjustedLevels`** returns a heap-allocated snapshot of current per-channel values (adjusted = post-pan post-mute computed values ready for the HW). The snapshot is passed back into `vtable[0x28]` so the apply method can compute **deltas** and only issue AV/C writes for channels that actually changed. This matches the AV/C-command budget of the FW1814 (~1 ms per vendor command) — a 22-channel mixer with 4 properties = 88 potential writes per panel event; delta-batching keeps it tractable.

## Batch-apply binding — `BaseChannelGroup::SetupBatchChangeMethod<Dev, PMF>`

```c
template<class Dev, class PMF>
void BaseChannelGroup::SetupBatchChangeMethod(Dev *dev, PMF method,
                                               uint mask, int flag) {
    dev[+0x44] = 0x80;              // "batch method installed" marker
    dev[+0x40] = mask;              // channel-selector mask for this batch
    if (dev[+0x38]) delete dev[+0x38];
    void *tf = NULL;
    if (method) {
        tf = operator new(0x20);
        tf[0]  = &PTR_Call_00036810;    // TFunctor vtable
        tf[8]  = method;                // pointer-to-member-function (16 bytes on x86_64)
        tf[16] = 0xBA9;                 // tag/opcode (M-Audio AV/C vendor opcode?)
        tf[24] = 0;
    }
    dev[+0x38] = tf;
}
```

- Instantiated for `com_m_audio_FW1814Device` with method type `int (FWAVCAudioDevice::*)(uint, uint, void*)` — matches `AVCVendorCommand(subunit, selector, payload)`.
- Constant `0xBA9` goes into the TFunctor's data slot; this is almost certainly the M-Audio vendor-opcode or vendor-attribute tag that distinguishes "mixer batch commit" from other vendor commands.
- `dev[+0x40]` holds the active channel mask so when the batch commits it knows which rows to push.
- Marker byte `0x80` at `dev[+0x44]` lets `com_m_audio_FW1814Device` check "is a batch method installed" cheaply.

**So the FW1814 holds, at `[+0x38..+0x44]`, a single bound hardware-apply handle:**

| Offset | Field |
|---|---|
| `+0x38` | TFunctor<batch-apply> (new'd 32 B) |
| `+0x40` | uint channelMask |
| `+0x44` | uint8 flag (0x80 when installed) |

## The complete property surface

Below is the enumeration implied by the ctor + layout analysis. Property keys (OSSymbol strings) live in the ctors we haven't fully decompiled — a Q15a follow-up can grep for `OSSymbol::withCString` calls near each ctor to extract them. **Guessed names in italics**, based on the Carbon panel's observed controls on the Snow Leopard reference Mac:

### `FWSettings1814Levels` (dict exposed on FW1814Device as `"InputLevels"` or similar)

| Field | Property | Channels | Guessed key |
|---|---|---|---|
| `+0x10` ArrayProp | per-channel scalar | 10 (or 16) | *AnalogInGain* |
| `+0x70` 32bitProp | scalar | 1 | *MasterOutVolume* |
| `+0xb0` 32bitProp | scalar | 1 | *HeadphoneVolume* |
| `+0xf0` 32bitProp | scalar | 1 | *MasterInGain* |
| `+0x130` CGP #1a | ChannelGroup<N> | per-channel | *Volume* (group A) |
| `+0x1a8` CGP #1b | | | *Pan* |
| `+0x220` CGP #1c | | | *Mute* |
| `+0x298` CGP #1d | | | *Routing* |
| `+0x310` CGP #2a-d | group B | | same 4 keys, different group |
| `+0x4f0` CGP #3a-d | group C | | |
| `+0x6d0` CGP #4a-d | group D | | |
| `+0x8d0` GroupCG | all-A union | | *AllInputs* |
| `+0x930` GroupCG | all-B union | | *AllOutputs* |
| `+0x990` ArrayProp | per-channel scalar | 10 or 16 | *OutputTrim* |
| `+0x9f0` ArrayProp | per-channel scalar | 10 or 16 | *ClipIndicators* |

### `FWSettings1814AuxSend` (dict exposed on FW1814Device as `"AuxSend"`)

| Field | Property | Channels | Guessed key |
|---|---|---|---|
| `+0x20` CGP #1 | Aux1 send | 10/16 inputs | *Aux1Send* |
| `+0x98` CGP #2 | Aux1 pan | 10/16 inputs | *Aux1Pan* |
| `+0x110` CGP #3 | Aux2 send | 10/16 inputs | *Aux2Send* |
| `+0x188` CGP #4 | Aux2 pan | 10/16 inputs | *Aux2Pan* |
| `+0x200` ArrayProp | | 2 | *AuxMasters* |
| `+0x260` ArrayProp | | 2 | *AuxPanMasters* |

## The 4-group structure — hypothesis

Groups A/B/C/D on `FWSettings1814Levels` are probably:
- **A = analog inputs** (ConcreteChannelGroup<10> or <6>)
- **B = ADAT inputs** (<8>, lazy-created when ADAT-in mode active — but 8 is not in the observed N set, so probably rolled into A via GroupChannelGroup)
- **C = main outputs** (<10> or <6>)
- **D = aux outputs** (<2>, S/PDIF)

But the observed N set is {2,4,6,10,14,18,22}. The 14/18/22 variants are the "with-ADAT" totals we see in Q13's channel-count branches. So groups are probably:
- A = inputs full (ConcreteChannelGroup<14> or <18>)
- B = outputs full (<10> or <22>)
- C = headphone (<2>)
- D = aux (<2>)

Confirmation via Q15a (grep OSSymbol strings) will nail this down.

## Dependency: how a panel edit flows end-to-end

1. Panel calls `IORegistryEntrySetCFProperties(fw1814Device, { "InputLevels": { "Volume": [0.5, 0.7, …] }})`
2. `com_m_audio_FW1814Device::setProperties` → `m_audio_b_FWPropertyDict::setProperties` → finds the `FWSettings1814Levels` sub-object, calls its vtable method for sub-key "Volume".
3. The property's `CallSetter` fires with the OSArray.
4. `AdjustedLevels` snapshots group A's current state.
5. `OSArrayToSeriatimValues` unpacks `[0.5, 0.7, …]` into group A's per-channel slots.
6. `group.vtable[0x28]` (= `BaseChannelGroup::ApplyAdjustedLevels(before)`) fires.
7. Inside ApplyAdjustedLevels, deltas vs `before` are computed; for each changed channel, `TFunctor::Call(dev, channelIndex, newLevel)` fires, which calls `FWAVCAudioDevice::AVCVendorCommand(subunit=stashed, selector=0xBA9, payload)`.
8. AV/C transaction completes; the kext optionally writes the new values back into the property dict so panel re-reads are consistent.
9. `AddModifiedProperties({"Volume": [...]})` + `SendChangedPropertiesMessage` broadcasts the change to all other open panels (Q14).

## Clone implications

### M5 mixer scaffold

The clone needs:
1. **A typed-property framework** — the clone's current `FW1814Device` doesn't have one. Options:
   - **Full mirror**: port `FWBaseProp`/`FWArrayProp`/`FW32BitNumberProp`/`FWChannelGroupProperty`/`FWPropertyDict`. ~2000 LOC. Preserves 1:1 property surface compatibility with the Carbon panel.
   - **Lighter model**: expose a flat `OSDictionary` tree directly on the FW1814Device's IORegistry entry; override `setProperties` with per-key handlers. ~500 LOC. Clean break from the Carbon panel's wire format, but perfectly viable for an AppKit-only panel.
2. **A ConcreteChannelGroup<N> equivalent** — per-channel slot ring with `AdjustedLevels` snapshotting and a `BatchApply` method.
3. **Vtable slots on `FW1814Device`** at indices equivalent to `0xbb8/0xbc0/0xbc8` — our existing `FW1814Device` can simply have:
   ```c++
   virtual IOReturn SetInputRouting(uint ch, uint route);
   virtual IOReturn SetInputPan   (uint ch, uint pan);
   virtual IOReturn SetAuxMonitor (uint bus, uint level);
   ```
4. **The `0xBA9` vendor opcode** — stash as a constant; this is one of the two "mixer-commit AV/C vendor opcodes" (the other being the AC3 mode-switch opcode found in Q12). FFADO's `maudio.cpp` likely documents it.

### Recommendation

- **For M5-first-light**: do the lighter model. Build the FW1814Device's three virtuals, use plain `OSDictionary` on the registry, override `setProperties` with a 3-branch handler, publish per-channel OSArrays for levels.
- **Defer the full property framework** until M7+ or until we attempt panel binary compatibility.
- Match the `[+0x40..+0x44]` state-mask + marker layout so a future Carbon-panel binary (if we ever rebuild one for testing) can be glued on without rewriting the hardware-apply path.

## Follow-ups

- **Q15a:** extract OSSymbol key strings for each ChannelGroupProperty via `grep OSSymbol::withCString` in Ghidra; confirms the actual property keys the panel reads.
- **Q15b:** decompile `com_m_audio_FW1814Device::SetInputPan` (vtable[0xbc0] implementation) to see exactly what AV/C vendor command it emits — this tells us the subunit + opcode of the "per-channel pan" write, which + 0xBA9 bounds the FW1814 mixer protocol.
- **Q15c:** `BaseChannelGroup::AdjustedLevels` + `BaseChannelGroup::ApplyAdjustedLevels` — the delta-apply guts. One-off to decode.
- **Q15d:** `FWBaseArrayProp::OSArrayToSeriatimValues` — the OSArray→typed-slot unpacking logic.

## Provenance

- Addresses (2026-10-06): `FWSettings1814Levels::FWSettings1814Levels` @ `0xeec0`; `FWSettings1814AuxSend::FWSettings1814AuxSend` @ `0xefda`; `SetInputPan` @ `0xd128`; `SetInputRouting` @ `0xd15c`; `SetAuxMonitor` @ `0xd0b4`; `FWChannelGroupProperty::init` @ `0x93a4`; `CallSetter` @ `0x940e`; `BaseChannelGroup::SetupBatchChangeMethod<FW1814,…>` @ `0xe184`; `ConcreteChannelGroup<N>::ctor` @ `0xb3b4` (N=6), `0xb432` (N=2); `ChannelAtIndex` @ `0xac9e`.
- Vtable constants: `PTR_SendToDevice_000366c0` (Levels final), `PTR_SendToDevice_00036670` (AuxSend final), `PTR__GroupChannelGroup_00047fc0`.
- Vendor tag constant `0xBA9`: immediate at `0xe1cc` inside `SetupBatchChangeMethod`.
- Vtable slot offsets `0xbb8/0xbc0/0xbc8` on FW1814Device: immediates in the three `Set*` methods above.
- Field offsets on FWChannelGroupProperty (`[+0x60]`, `[+0x68]`, `[+0x70]`): writes in `init` @ `0x93d7..0x93e5`.
- Decompiled bodies in `docs/system-map/A1-ghidra/ghidra-exports/decomp/q15.c` (regenerable via `scripts/DecompQ15.java`).
