# Q15d — `MARateLocked` string xref

**Opened by:** B5 §13 (reclassified from B5 pending work on 2026-10-13).
**Goal:** decide whether `MARateLocked` rate-lock is a kext-side guard (prevents writes while locked) or a Panel-UX cosmetic.

**Verdict (static evidence from v1.11.2 `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice`):**

> `MARateLocked` has **two writers and zero readers** in the kext. It is a one-way broadcast of the rate-lock state to the Panel. The kext never gates any behavior on it; any UX enforcement is Panel-side only.
>
> This confirms the B5-era observation (B5 §9) that the SL kext layer accepts rate-change writes unconditionally — the Panel disables the control when locked, but a direct `IORegistryEntrySetCFProperties(MASampleRate=…)` bypasses the lock entirely.

## Provenance

String table: `otool -s __TEXT __cstring _extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` places `"MARateLocked"` at virtual address `0x2db07` inside the `__TEXT,__cstring` section.

Both xrefs are RIP-relative `leaq` into `%rsi`, immediately before (or inside) a tail-call / direct call to `m_audio_b_FWAudioDevice::SetStaticPropertyAndNotifyUserClients(char const*, unsigned int)`. That function is the kext → Panel broadcast primitive (A1-Q14 — `FWKIPCUserClient` Mach-port notification path).

### Xref #1 — `m_audio_b_FWAudioDevice::DetectExternalSyncChanges(bool, FDFValue)` @ `0x2bc3`

Enclosing symbol: `__ZN23m_audio_b_FWAudioDevice25DetectExternalSyncChangesEb8FDFValue` (asm lines 2009–2185; body `0x2aca..~0x2c?`).

Context (asm lines 2068–2081):

```
0x2bab   cmpb  0x25a(%r14), %bl        ; compare field_0x25a to new value
0x2bb2   jne   0x2bb9                   ; no change → skip
         ...retq                        ; (bail if unchanged)
0x2bb9   movb  %bl, 0x25a(%r14)         ; write field_0x25a = bl
0x2bc0   movzbl %bl, %edx               ; edx = (uint)bl
0x2bc3   leaq  [MARateLocked](%rip), %rsi
0x2bca   movq  %r14, %rdi               ; this
0x2bd1   jmp   SetStaticPropertyAndNotifyUserClients
```

**Semantics.** `DetectExternalSyncChanges(externalSync, fdf)` is called from the HSCI 50 ms polling timer (A1-Q6) whenever the device reports a new FDF value or sync-change event. The tail of the function recomputes a `bl` flag:

```
if (externalSync) {
  if (fEngine && fEngine->field_0x228 == this->field_0x25c)   // engine's expected rate == current FDF rate index
    bl = 1   // sync source is locked and rate-matched
  else
    bl = 0
}
```

If `bl` differs from the cached `field_0x25a`, the kext writes it back and broadcasts `("MARateLocked", (uint)bl)`.

### Xref #2 — `m_audio_b_FWAudioDevice::initHardware(IOService*)` @ `0x3644`

Enclosing symbol: `__ZN23m_audio_b_FWAudioDevice12initHardwareEP9IOService` (asm line 2678+).

Context (asm lines 2800–2804):

```
0x363c   movb  $0x1, 0x25a(%r14)        ; field_0x25a = 1
0x3644   leaq  [MARateLocked](%rip), %rsi
0x364b   movl  $0x1, %edx               ; edx = 1
0x3650   movq  %r14, %rdi               ; this
0x3653   callq SetStaticPropertyAndNotifyUserClients
```

**Semantics.** `initHardware` seeds `MARateLocked = 1` during device probe. Since nothing in the kext reads `0x25a`, this is purely a Panel-initialization hint — the Panel will see the property as `1` the first time it enumerates, matching the "I'm locked" default that `DetectExternalSyncChanges` then maintains.

## Reader scan

Full disassembly scan of accesses to `0x25a(%r14 | %rdi | %rax | …)` and the `MARateLocked` cstring:

```
grep -n '0x25a(%r1[0-9]\|0x25a(%rdi\|0x25a(%rax\|0x25a(%r[a-z]' docs/system-map/A1-ghidra/otool-exports/mafw.asm
```

Returns exactly three hits:

| line | offset | op | kind |
|---|---|---|---|
| 2068 | 0x2bab | `cmpb 0x25a(%r14), %bl` | **read** — but only for change-detection inside xref #1 itself; result is used solely to decide whether to broadcast |
| 2074 | 0x2bb9 | `movb %bl, 0x25a(%r14)` | write (xref #1) |
| 2800 | 0x363c | `movb $0x1, 0x25a(%r14)` | write (xref #2) |

The one read is self-referential — the `DetectExternalSyncChanges` broadcaster reads its own last-published value to avoid re-broadcasting. **No downstream code in the kext consults the field to decide whether to accept or reject a rate change, a routing write, a mixer write, or any other property write.**

## Implication for the clone

- **Rate-lock enforcement is Panel-side.** The clone's kext may freely accept `MASampleRate` writes regardless of the current `MARateLocked` state; the Panel is responsible for disabling the control.
- **Still publish `MARateLocked`.** The v1.9/v1.11.2 Panel listens for `MARateLocked` on the `FWKIPCUserClient` notification channel to grey out the sample-rate menu while externally synced. If the clone's Panel keeps parity with the reference UX, the kext must emit this property on sync-state changes and at init.
- **Emission rule.** `MARateLocked = (externalSyncSelected && externalSourceRate == deviceFDFRate)`. Emit on every HSCI sync-change event *and once at `initHardware`*.
- **Null-write OK.** The reference kext short-circuits when the state is unchanged (`cmpb 0x25a; jne` guard). The clone should do the same to avoid flooding the user-client with no-op notifications on every HSCI poll tick.

## Cross-links

- **A1-Q6** (HSCI polling timer) — the 50 ms `0xFFC7:00600000` poll is the trigger for `DetectExternalSyncChanges`.
- **A1-Q14** — `FWKIPCUserClient` is the broadcast channel; `MARateLocked` arrives at the Panel as a dirty-property OSDictionary entry.
- **A2-P3** — `MARateLocked` is one of 21 `MA*` keys the Panel subscribes to on the notification port.
- **B5 §9 + §13** — original SL hypothesis was "rate-locked guard is enforced somewhere"; this file rules out the kext.
- **A1-Q15b** — generic `SetProperties` dispatcher accepts writes regardless of `MARateLocked`; writes queue to the workloop → `ApplyModifiedProperties`.
