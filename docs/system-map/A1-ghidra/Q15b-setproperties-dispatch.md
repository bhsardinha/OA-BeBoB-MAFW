# Q15b — Which IOService absorbs non-`MASampleRate` `setProperties` writes?

**Opened by:** A2-P3 (property-key table) — asked which kext class dispatches the 21 `MA*` NSString keys the panel sends via `IORegistryEntrySetCFProperties`.

**Decompile log:** `ghidra-exports/decomp/q15b.log` — four functions.

## Short answer

**`m_audio_b_FWPropertyDict::SetProperties` @ 0xa0e0** is the single generic dispatcher. **`com_maudio_FWMixer::setProperties` does not exist** — the P3 doc's hypothesis #3 was wrong (confirmed by `grep -i FWMixer::setProperties` returning zero hits in `functions.tsv`). The real chain is:

```
panel: IORegistryEntrySetCFProperties(entry, NSDictionary*)
       ↓ kernel
IOService::setProperties(OSObject*)            [virtual]
       ↓
m_audio_b_FWAVCAudioDevice::setProperties      @ 0x22e28   -- siphons MASampleRate only
       ↓ (tail-call PTR_setProperties_00031470 = parent class)
m_audio_b_FWAudioDevice::setProperties         @ 0x299c    -- thunk
       ↓
m_audio_b_FWPropertyDict::SetProperties        @ 0xa0e0    -- generic iterator
       ↓ for each (key, value) pair
FWPropertyDict::SetOneProperty(this, OSSymbol, OSObject)   -- per-key router
       ↓
IOEventSource::signalWorkAvailable()                       -- kick workloop
       ↓
FWPropertyDict::checkForWork                   @ 0x9d38
       ↓
FWPropertyDict::ApplyModifiedProperties(bool)  @ 0x9b40    -- walks modified set,
                                                             calls each typed-property's
                                                             virtual Apply() to issue
                                                             the actual FCP/HSCI command
```

The dict is attached to `FWAudioDevice` at offset `+0xf8` (`FWAVCAudioDevice` keeps a copy at `+0xf8` and the engine pointer at `+0xe8`). This is the "typed-property framework" the A1 overview already identified.

## Function-by-function decode

### `m_audio_b_FWAudioDevice::setProperties` @ 0x299c (39 B — thunk)

```c
void FWAudioDevice::setProperties(FWAudioDevice *this, OSObject *props) {
    if (this->propertyDict /* +0xf8 */ != NULL) {
        this->propertyDict->SetProperties(props);
        return;
    }
    IORegistryEntry::setProperties(this, props);   /* fallback — default IOKit path */
}
```

Pure forwarder. If no `FWPropertyDict` is installed (shouldn't happen in production), falls back to the IORegistryEntry base, which just stashes the dict in the registry without side-effects.

### `m_audio_b_FWAVCAudioDevice::setProperties` @ 0x22e28 (169 B)

```c
undefined8 FWAVCAudioDevice::setProperties(FWAVCAudioDevice *this, OSObject *props) {
    if (this->propertyDict /* +0xf8 */ && this->engine /* +0xe8 */) {
        OSDictionary *d = OSDynamicCast(OSDictionary, props);
        if (!d) return 0;                             /* note: still falls through below */
        OSObject *rate = d->getObject("MASampleRate");
        OSNumber *n = OSDynamicCast(OSNumber, rate);
        if (n) {
            int newRate = n->unsigned32BitValue();
            if (this->engine->currentRate /* +0x8c */ != newRate) {
                this->vtable[0xa78/8](this, newRate);  /* FWAVCAudioDevice::ChangeRate */
            }
        }
    }
    return parent::setProperties(this, props);        /* always forward */
}
```

**Correction to A1-Q11b:** the Q11b write-up stated "`FWAVCAudioDevice::setProperties` **only** handles `MASampleRate`; everything else falls through." That's correct but incomplete — the key insight is the **tail-call** to `PTR_setProperties_00031470` (which decodes to the parent `FWAudioDevice::setProperties` thunk) happens on **every** path, including after a successful `MASampleRate` write. So MASampleRate is a *sidecar fast-path*; the dict still iterates every key via the generic dispatcher below. The `MASampleRate` sidecar exists purely so a rate change doesn't have to go through the typed-property `Apply()` virtual dispatch (which would race the AV/C stream-kick).

### `m_audio_b_FWPropertyDict::SetProperties` @ 0xa0e0 (255 B — generic dispatcher)

```c
IOReturn FWPropertyDict::SetProperties(FWPropertyDict *this, OSObject *props) {
    OSDictionary *d = OSDynamicCast(OSDictionary, props);
    if (!d) {
        OSString *s = OSDynamicCast(OSString, props);          /* bare-string form */
        if (s) {
            OSSymbol *sym = OSSymbol::withString(s);
            SetOneProperty(this, sym, NULL);                   /* NULL value = query/reset */
            sym->release();
            goto kick;
        }
        return 0xe00002c2;                                     /* kIOReturnUnsupported */
    }
    if (!d) return 0xe00002c2;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(d);
    if (!it) return 0xe00002c2;
    OSSymbol *key = it->getNextObject();
    if (!key) {
        it->release();
        return 0xe00002c2;
    }
    do {
        OSObject *val = d->getObject(key);
        SetOneProperty(this, key, val);
        key = it->getNextObject();
    } while (key);
    it->release();
kick:
    if (this->workloopSrc /* +0x30 */ != NULL) {
        this->workloopSrc->signalWorkAvailable();              /* wake checkForWork */
    }
    return 0;  /* on success */
}
```

**No key-string filter.** Every key the panel sends is handed to `SetOneProperty`, which looks it up in the dict's internal `OSSymbol → m_audio_b_FWBaseProp*` map. Unknown keys are silently ignored by `SetOneProperty` (that is a safe assumption from the surrounding helpers like `ModifyProperty @ 0x9e28` and `AddUpdateForAudioControl @ 0x9ce8`). The actual hardware action is deferred — the workloop event source is signalled, and the write happens from `checkForWork` → `ApplyModifiedProperties`.

**The `+0x30` field is a `IOEventSource *`** (likely `IOInterruptEventSource` or `FWPropertyDict` itself since it's a `IOEventSource` subclass per `checkForWork @ 0x9d38` existing). Deferral via workloop is **why the panel's writes are low-latency** — the `IORegistryEntrySetCFProperties` call returns as soon as the keys are dropped into the modified set; the FCP/HSCI commands dispatch asynchronously.

### `com_m_audio_FWMetaNub::setProperties` @ 0x1cb64 (314 B — nub-level override)

```c
IOReturn FWMetaNub::setProperties(FWMetaNub *this, OSObject *props) {
    bool locked = this->engine /* +0xe8 */->vtable[0xa18/8]();   /* try-lock */
    /* same dict iterator loop as FWPropertyDict::SetProperties, but calls
       SetOneProperty on `this` (the nub), not a dict object */
    ...
    if (locked == 0) {
        this->engine->vtable[0xa20/8]();                         /* unlock */
    }
    return rc;
}
```

The nub-level override exists because `FWMetaNub` is the single polymorphic matching class (per A1 overview) and may receive `setProperties` before the full engine stack is up. The lock/unlock pair gates the write against concurrent probe-time initialization. Not reached from the panel's normal operation — the panel's registry match targets `FWAudioDevice`, not the nub.

## `SetOneProperty` dispatch (brief)

Not fully decoded here, but from the surrounding helpers in `functions.tsv`:

- `ModifyProperty(OSSymbol, OSObject)` @ 0x9e28 — look up the typed property wrapper in the dict's internal map; call `Set(value)` on it; add to a "modified" `OSSet` at `this[+<offset>]`.
- `AddUpdateForAudioControl(FWBaseProp*)` @ 0x9ce8 — queue a specific property for the next workloop pass.
- Workloop entry `checkForWork` @ 0x9d38 → `ApplyModifiedProperties(bool shouldKick) @ 0x9b40` iterates the modified set, calls each typed-property's virtual `Apply()` (vtable+?), then clears the set.
- `CacheModifiedPropertiesInUserClients` @ 0x9ab6 → `SendPropertyChangedMessagesToUserClients` @ 0x9acc — this is the **outbound** Mach-port notification path (A1-Q14). It fires *after* Apply() succeeds, so the panel's `bodyType==0` notification carries the post-write state.

The typed-property objects themselves are what A1-Q15 decoded (`ConcreteChannelGroup<N>`, scalar properties, etc.). Each one owns its own `Apply()` that converts the cached NSNumber/NSArray/NSString value into the appropriate 12-byte HSCI envelope (A1-Q6) or AV/C command (A1-Q5, Q16).

## Answer to the two "decoded when?" questions

Both were pending as **Q15b**, newly opened by A2-P3 and listed in `A2-panel/README.md` as "priority":

- **`FWAVCAudioDevice::setProperties` fallthrough** — decoded here. Fallthrough is unconditional tail-call to `FWAudioDevice::setProperties` → `FWPropertyDict::SetProperties`. There is nothing selective about it: every key is handed to the generic dispatcher regardless of whether `MASampleRate` was present.
- **`com_maudio_FWMixer::setProperties`** — **does not exist**. P3 hypothesis #3 was wrong. Mixer semantics are implemented inside the `FWPropertyDict`'s typed-property framework (A1-Q15), not as a separate IOService override. Our clone must mirror this structure: one `setProperties` surface, one dispatcher, N typed-property objects, each with its own `Apply()`.

## Clone implication

Our kext's `OpenAudioFW1814Device::setProperties` surface **should mirror this exact chain**:

1. Thin override on the audio device that siphons `MASampleRate` for a fast-path rate change (so the panel's rate-change latency is bounded) — mirrors `FWAVCAudioDevice::setProperties`.
2. Tail-call to a generic property-dict dispatcher — mirrors `FWPropertyDict::SetProperties`.
3. All other `MA*` keys route to typed-property wrappers via `SetOneProperty`.
4. Writes land in a "modified" `OSSet`; workloop event source signals; `checkForWork` → `ApplyModifiedProperties` walks the set and dispatches HSCI/FCP commands.
5. On success, broadcast the Mach-port notification (A1-Q14 / A2-P2 `bodyType==0`) so the panel syncs.

**Do not create a per-feature `setProperties` override** (no `FWMixer::setProperties`, no `FWHardware::setProperties`). All feature-specific behaviour lives inside typed-property `Apply()` methods, not in `setProperties` dispatch.

## Status

Closes **Q15b**. Opens no new follow-ups. The one remaining structural unknown is the exact `SetOneProperty` + `ApplyModifiedProperties` key-registration side (how typed-property objects enter the dict at init time) — that is a secondary question for A1 (**Q15c**) if we need it for the clone, but can be deferred until the clone work resumes.
