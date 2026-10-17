# Q15f — Mixer control plane wire map

> **Correction (2026-10-15, Q-ME `Q-ME-mixer-property-framework.md` §2):** the table row `this+0x1b00 → 0x34 — functor [+0x1830] is CustomMixerWriteWithRotatedAuxChannels` is wrong. `InitializeUserSettableProperties @0xe6ca` stores `0x64` at `this+0x1838`, `0x80` at `this+0x183c` and the rotated writer at `this+0x1830`, which are the `[+0x40]`, `[+0x44]` and `[+0x38]` fields of the group at **`this+0x17f8` (aux send, 22 channels, register `0x64`, 11 quadlets)**; the group at `this+0x1b00` (aux output, 2 channels) is a normal `0x34` group with the ordinary `CustomMixerWriteQuads` writer. This also explains why the rotated block is 11 quadlets (= 22 channels / 2), resolving the "unverified n" note.

> **Correction (2026-10-14) — MA* crosswalk** (evidence: [Q-MAMAP-key-crosswalk.md](Q-MAMAP-key-crosswalk.md), [PROVENANCE-RESOLUTIONS.md §R20](../PROVENANCE-RESOLUTIONS.md)). For the FW1814 the mixer is **not** a single path: levels (SW return 0x00, output 0x08, input 0x10, headphone 0x38, aux output 0x34, aux send 0x64), input pan (0x40+), input routing (0x90), headphone selector (0x98) and aux monitor (0x9c) are vendor register writes via `CustomMixerWriteQuads` to `0xFFC7:0x00700000+offset`; **`MAAuxOutputPan2` (FWSettingsAuxSend::SetAuxPan → `AVCControlFeatureFunctionBlock`) and `MASWReturnRoutingV2` (base `FWSettingsLevels::SetSWReturnRouting` → `SetAVCRouting` → `AVCMixerConnectPair`) go over AV/C.** Mute/solo/dim are level operations (minimum / −10 dB). Level wire value = property level (−255..0, 0.5 dB) × 0x80, int16 big-endian, two channels per quadlet.


> **Correction (2026-10-14) — rotated aux** (evidence: [PROVENANCE-RESOLUTIONS.md §R14](../PROVENANCE-RESOLUTIONS.md)). The group size and the reason for the rotation are now explained. Binary: `CustomMixerWriteWithRotatedAuxChannels` sends **11 quadlets (44 bytes)** ordered `{in[9], in[10], in[0..8]}`. FFADO `src/bebob/maudio/special_mixer.cpp:224-236` (third-party, wire truth only) lists the device's aux-input register block `0x64..0x8c` as **Stream 1/2 (0x64), Stream 3/4 (0x68), Analog 1/2…7/8 (0x6c-0x78), S/PDIF (0x7c), ADAT 1/2…7/8 (0x80-0x8c)** = 2 stream entries first, then 9 hardware entries. So the kext's input buffer is ordered hardware-first (9) then stream-return (2) and the rotation moves the 2 stream entries to the front to match the device register order. **Inferred** (not read from the binary): that `in[9..10]` are the two software-return (stream) entries — check = the `FWSettingsAuxSend` channel-map in the kext or a live write capture on SL.


> **Correction (2026-10-14) — `0xBA9` / `0xBE9` are C++ virtual member-function pointers, not AV/C opcodes. Paths A and B are the same path.**
>
> - **Mechanism.** `SetupBatchChangeMethod @ 0xe184` allocates a 32-byte functor `{vtbl, object, 0xba9, 0}` (`0xe1c3..0xe1d9`). That is an Itanium-ABI pointer-to-member `{ptr, adj}`. An odd `ptr` means a *virtual* call through vtable byte offset `ptr − 1`. `com_m_audio_FW1814Device` vtable (`0x369b0` + 0x10) slot **`0xba8` = `m_audio_b_FWAVCAudioDevice::CustomMixerWriteQuads @ 0x22c7a`**. Slot **`0xbe8` = `com_m_audio_FW1814Device::AVCControlSetLEDStatus(unsigned int) @ 0xdaaa`**, so "Path C `0xBE9`" is the A/B-switch **LED status** call, not a mixer write.
> - **Group → register.** `SetupBatchChangeMethod` stores its `edx` argument at group `[+0x40]` (`0xe199`) and `0x80` at `[+0x44]`. The `edx` argument is the **register byte offset** in the `0xFFC7_0070_0000` window, not a sub-index. `BaseChannelGroup::SendLevelsToDevice` calls the functor as `(esi = [group+0x40], edx = Count() >> 1, buf = AdjustedAndMultipliedLevels())` (`0x1efc0..0x1efd1`). Two 16-bit channel values are packed per quadlet.
> - **`CustomMixerWriteQuads(offset, nQuads, buf)`** (`0x22c7a`) copies `nQuads × 4` bytes into an IOBMD and issues one async block write to **FireWire address `0xFFC7:0x00700000 + offset`**: `local_36 = 0xffc7` (high 16 bits), `local_34 = offset + 0x700000` (low 32). The command is created with `[this+0xe0]->vt[0x890]` and submitted with `vt[0x140]`. It is a no-op returning 0 if `[this+0xe0]` is NULL.
> - **FW1814 groups** (`InitializeUserSettableProperties @ 0xe6ca`). The offsets match FFADO `special_avdevice.h:55–69` exactly:
>
>   | Device field | `SetupBatchChangeMethod` offset | FFADO name |
>   |---|---|---|
>   | `this+0x1668` | `0x00` | `GAIN_STM_12_IN` (stream inputs) |
>   | `this+0x1318` | `0x08` | `GAIN_ANA_12_OUT` |
>   | `this+0x13e0` | `0x10` | `GAIN_ANA_12_IN` |
>   | `this+0x1730` | `0x38` | `GAIN_HP_12_OUT` |
>   | `this+0x1b00` | `0x34` | `GAIN_AUX_12_OUT` — functor `[+0x1830]` is `CustomMixerWriteWithRotatedAuxChannels @ 0xe982` instead of `0xba9` |
>
> - **Rotated aux.** `CustomMixerWriteWithRotatedAuxChannels(offset, n, buf)` builds `{buf[9], buf[10], buf[0..8]}` (44 bytes: `memcpy(+0x24 → front, 8)` then `memcpy(buf → +8, 0x24)`) and calls `vt[0xba8]` = `CustomMixerWriteQuads` with the same `offset` and `n`. **Unverified:** the exact `n` for the aux group, and why the last two quadlets are rotated to the front (the register order vs the panel's channel order). Needs `FWSettingsAuxSend::Init` and the aux group `Count()`.
> - **Implications.** "Path A — AV/C batch write opcode `0xBA9`" below does not exist as an AV/C command. All level/mute/pan group writes are direct async block writes to `0xFFC7:0x0070xxxx`, the same window FFADO, macfw and the Rust crate use (B1, B4, B2a). This resolves the "second mixer path" puzzle: there is one path. The open item "Path B opcode → register formula" is resolved by the table above. The open item "Path C key list" becomes: what `AVCControlSetLEDStatus` sends (A/B LEDs) — §1 Path C text needs re-reading in that light.

**Status:** done 2026-10-13

**Scope:** how Panel's MA* property writes reach hardware. Four write paths decoded from `docs/system-map/A1-ghidra/otool-exports/mafw.asm`. AVC batch opcodes `0xBA9` / `0xBE9`. Direct async FireWire write path (`CustomMixerWriteQuads`). HSCI clock-source path. `ResendCurrentMixerSettings` re-send sequence. Sources: `FW1814Device::InitializeUserSettableProperties @ 0xe6ca`, `SetupBatchChangeMethod @ 0xe184`, `FWPropertyDict::SetProperties @ 0xa0e0`, `CustomMixerWriteWithRotatedAuxChannels @ 0xe982`, `SetAuxMonitor @ 0xcfc4`, `CustomMixerWriteQuads @ 0x22c7a`, `ResendCurrentMixerSettings @ 0xd014`.

---

## 1. Four write paths

### Path A — AVC batch write (opcode `0xBA9`)

**Trigger:** Panel writes any MA* level/mute/pan/aux key via `IORegistryEntrySetCFProperties` →
device's `setProperties(OSDictionary*)` → `FWPropertyDict::SetProperties @ 0xa0e0` →
per-key `SetOneProperty` → `InjectModifiedProperty` → workloop `checkForWork` →
`ApplyModifiedProperties` → fires TFunctor with opcode `0xBA9`.

**TFunctor installation:** `BaseChannelGroup::SetupBatchChangeMethod @ 0xe184`:

```
000000000000e191	movl	%edx, 0x40(%r14)      // sub-index → channel group [+0x40]
000000000000e199	movl	$0x80, 0x44(%r14)      // max count = 128
...
000000000000e1d1	movq	$0xba9, 0x10(%rax)     // TFunctor opcode slot = 0xBA9
000000000000e1e1	movq	%rax, 0x38(%r14)       // store TFunctor in group [+0x38]
```

**Every `SetupBatchChangeMethod` call installs opcode `0xBA9` unconditionally.** The `%edx` argument is the **channel group sub-index** (stored at group `[+0x40]`), not an AVC opcode variant.

**Channel groups in FW1814Device (`InitializeUserSettableProperties @ 0xe6ca`):**

| Field offset | Sub-index (`%edx`) | Object |
|---|---|---|
| `[+0x1318]` | `0x08` (8) | InputChannelGroup |
| `[+0x13e0]` | `0x10` (16) | SW-return/output group |
| `[+0x1668]` | `0x00` (0) | First channel group (aux/main) |
| `[+0x1730]` | `0x38` (56) | Fourth channel group |
| `[+0x2d8]` | `0x34` (52) | FWSettingsABSwitch inner group |

**MA* keys handled by Path A** (all channel-group properties registered in `FWSettingsLevels::Init`, `FWSettingsAuxSend::Init`, `FWSettingsRotaryControl::Init`):

```
MAInputLevels       MAOutputLevels      MASWReturnLevels
MAAuxSendLevels     MAAuxOutputLevels   MAAuxOutputMute
MAAuxOutputLink     MAAuxOutputPan2     MAMonitorAuxOnOutput
MAHeadphoneLevels   MA%sLevels          MA%sMute
MA%sSolo
```

String-format keys `MA%sLevels` / `MA%sMute` / `MA%sSolo` registered at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:40023–40060`.

---

### Path B — direct async FireWire register write

**Function:** `CustomMixerWriteQuads @ 0x22c7a` (called via vtable slot `0xba8` on the device object).

**Signature:** `CustomMixerWriteQuads(device, opcode, count, data_ptr)`
- Target FireWire address: `0xFFC7:0x700000 + offset` (base = BeBoB vendor register window)
- Mechanism: `memcpy` quad data, then async async FireWire write via controller `vtable[0x890]`

**Path B call sites within FW1814Device:**

| Call site | Opcode | Count | What |
|---|---|---|---|
| `FW1814Device::SetAuxMonitor @ 0xcfc4` | `0x9c` (156) | 1 | Aux monitor enable bit |
| `CustomMixerWriteWithRotatedAuxChannels @ 0xe982` (via vtable[0xba8]) | caller-supplied | caller-supplied | Rotated aux channel data |

**`CustomMixerWriteWithRotatedAuxChannels @ 0xe982`** — FW1814-specific wrapper:

```
000000000000e99f	leaq	0x24(%rbx), %rsi      // source: data[+0x24] (8 bytes)
000000000000e9a7	movl	$0x8, %edx
000000000000e9af	callq	_memcpy               // copy 8 bytes → local buf
000000000000e9b8	movl	$0x24, %edx
000000000000e9c0	callq	_memcpy               // copy 36 bytes → second local buf
000000000000e9d5	callq	*0xba8(%rax)          // dispatch to CustomMixerWriteQuads
```

This TFunctor is stored at `FW1814Device[+0x1830]`, installed by `InitializeUserSettableProperties`:

```
000000000000e84f	leaq	CustomMixerWriteWithRotatedAuxChannels(%rip), %rcx
000000000000e856	movq	%rcx, 0x10(%rax)          // method ptr at TFunctor[+0x10]
000000000000e862	movq	%rax, 0x1830(%rbx)         // stored at device[+0x1830]
```

**MA* keys handled by Path B:**

```
MAAuxOutputLevels   (via CustomMixerWriteWithRotatedAuxChannels, sub-opcode unverified)
SetAuxMonitor → called by FWSettings1814AuxSend::SetAuxMonitor @ 0xd0b4
```

---

### Path C — FWSettingsABSwitch write (opcode `0xBE9`)

TFunctor at `FW1814Device[+0x2f0]` installed by `InitializeUserSettableProperties @ 0xe8bf–0xe90f`:

```
000000000000e8ff	movq	$0xbe9, 0x10(%rax)     // TFunctor opcode = 0xBE9
000000000000e90f	movq	%rax, 0x2f0(%rbx)      // stored at device[+0x2f0]
```

Different vtable pointer (`0x27f58(%rip)`) from the Path A TFunctor (`0x28646(%rip)`) — these are distinct TFunctor template specialisations.

Additionally, `FW1814Device[+0x300]` stores a TFunctor with `SetHeadphoneSelector` method pointer:

```
000000000000e94d	leaq	SetHeadphoneSelector(%rip), %rcx
000000000000e954	movq	%rcx, 0x10(%rax)
000000000000e960	movq	%rax, 0x300(%rbx)
```

**MA* keys handled by Path C** (**unverified — needs FWSettingsABSwitch::Init trace**):

```
MAHeadphoneSourceV2       MAMomentarySwitchRoutings
MAMomentarySwitchSource   MAHeadphoneSecondSource
MAMomentarySwitchLatent
```

Located at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:35883–35940` (property registration in FWSettingsABSwitch init area).

**`UpdateLEDs @ 0xe96a`** — called at end of ABSwitch init; fires TFunctor at `[+0x2d8]` + both `[+0x2f0]` and `[+0x300]` TFunctors to sync LED state to device.

---

### Path D — HSCI vendor-dependent command (clock source only)

**Function:** `FW1814Device::SetClockSourceInternal @ 0xe25c`

Full 16-byte command layout documented in Q15b. Summary:
- [0–2]: `0x00 0xFF 0x00` (ctype=CONTROL, unit addr, vendor-dependent opcode)
- [3–5]: `0x04 0x00 0x04` (company_id = `0x040004` M-Audio)
- [6]: clock source selector (0x03=Internal, 0x01=SPDIF, 0x02=ADAT, 0x00=WordClk)
- [7]: `(MAClockSource >> 3) & 1` (optical-IN ADAT, bit 3 of the B5 §10 bitfield; Correction 2026-10-14)
- [8]: `(MAClockSource >> 4) & 1` (optical output mode bit4)
- [9]: `(MAClockSource >> 5) & 1` (S/MUX enable bit5 — **NOT clk_lock**)
- [10–15]: `0x00 0x00 0x00 0x00 0x00 0x00`

**Corrects C3 hypothesis**: C3 had byte 9 = `clk_lock`. Disassembly shows `(>> 5) & 1` = S/MUX (bit 5 of `MAClockSource`). Source: `docs/system-map/A1-ghidra/otool-exports/mafw.asm` @ `0xe3e4`–`0xe3fd`.

**Dispatch:** `vtable[0xb60]` on device calls into `SetClockSourceInternal`; `SetBlankSlateClockSource @ 0xe4f0` also calls `vtable[0xb60]` after a `0x9c4` ms IOSleep.

**MA* key → Path D:**

```
MAClockSource  (composite bitfield — see memory/project_maclocksource_is_composite_bitfield.md)
```

---

## 2. `FWPropertyDict::SetProperties @ 0xa0e0` — generic dispatcher

```
000000000000a0e0	// entry point for Panel's IORegistryEntrySetCFProperties
000000000000a11a	callq  OSCollectionIterator::withCollection(...)  // iterate OSDictionary
000000000000a14b	callq  OSDictionary::getObject(symbol)
000000000000a15a	callq  FWPropertyDict::SetOneProperty(key, value)
// loops over all keys; calls SetOneProperty per key
```

`SetOneProperty` → `InjectModifiedProperty @ 0x83ac` → marks property dirty → workloop `checkForWork` → `ApplyModifiedProperties` fires the installed TFunctor for each dirty property.

**Important:** `MASampleRate` bypasses this dispatcher. It is handled by a dedicated `FWFunctionProp` that calls directly into `PerformFormatChange @ 0x124cc` (Q15e). The `SetProperties` path only handles the single generic dispatcher at `0xa0e0`; `MASampleRate` has its own vtable slot.

---

## 3. `ResendCurrentMixerSettings @ 0xd014` — full re-send sequence

Called on reconnect, firmware reload, or explicit panel request. Fires all four settings subsystems in order:

| Step | Offset | Action |
|---|---|---|
| 1 | `0xd02a: callq *0xb10(%rax)` | `FWRev1AudioDevice::ResendCurrentMixerSettings` (base class) |
| 2 | `0xd03e: callq *0x10(%rax)` on `[+0x2d8]` | `FWSettingsABSwitch::SendToDevice()` |
| 3 | `0xd04f: callq *0x10(%rax)` on `[+0x470]` | `FWSettingsRotaryControl::SendToDevice()` |
| 4 | `0xd059: callq FWSettingsAuxSend::SendToDevice()` on `[+0xf98]` | re-send aux settings |
| 5 | `0xd06e: jmp FWSettingsLevels::SendToDevice()` on `[+0x538]` | re-send all levels (tail-call) |

---

## 4. AVC opcode summary

| Opcode | Type | Used by |
|---|---|---|
| `0xBA9` | AVC batch channel-group write | All levels/mutes/pans/aux (Path A) |
| `0xBE9` | AVC ABSwitch write | FWSettingsABSwitch headphone A/B (Path C) |
| `0xBF1` | Bus-reset notification | `BusReset @ 0xe53a` (CallCenter TFunctor opcode) |
| `0x9c` | Direct register write | `SetAuxMonitor @ 0xcfc4` via vtable[0xba8] (Path B) |

`0xBF1` source: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:16348` (`0xe5ab: movq $0xbf1, 0x10(%rax)` in `BusReset` CallCenter init) — this is not a mixer-plane opcode; it triggers the bus-reset recovery sequence on the workloop.

---

## 5. Open items

- **Unverified**: exact MA* key → Path B opcode mapping for `CustomMixerWriteWithRotatedAuxChannels` — needs `FWSettingsAuxSend::Init` trace at `docs/system-map/A1-ghidra/otool-exports/mafw.asm` around `0xe763` to confirm which opcode is stored in the TFunctor at `[+0x1830]`.
- **Unverified**: FWSettingsABSwitch MA* key list (items in §1 Path C) — needs `FWSettingsABSwitch::Init` trace at `docs/system-map/A1-ghidra/otool-exports/mafw.asm:13413` for exact key registration.
- ~~**Unverified**: register-space offset formula `opcode → 0xFFC7:0x700000+?` for Path B~~ — **resolved 2026-10-14** (Correction at top): address = `0xFFC7:0x00700000 + [group+0x40]`.
- **SetupCIP cases 3–6** still unverified (separate — see Q15e §3).

---

## Provenance

- `FWPropertyDict::SetProperties`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:11267` (`0xa0e0`), iterator + `SetOneProperty` loop at `0xa117–0xa175`.
- `SetupBatchChangeMethod`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:16053` (`0xe184`); `movq $0xba9, 0x10(%rax)` at `0xe1d1`; sub-index stored at `0xe199`.
- `InitializeUserSettableProperties` channel group offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:16425` (`0xe6ca`–`0xe980`); `leaq 0x1318(%rbx)` at `0xe70f`, `leaq 0x13e0` at `0xe716`, `leaq 0x1668` at `0xe708`, `leaq 0x1730` at `0xe71d`.
- `SetupBatchChangeMethod` sub-indices: `movl $0x8, %edx` at `0xe7a6`; `$0x10` at `0xe7b3`; `xorl %edx` (=0) at `0xe7c6`; `$0x38` at `0xe7d1`; `$0x34` at `0xe869`.
- `CustomMixerWriteWithRotatedAuxChannels` TFunctor install: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:16514–16520` (`0xe84f–0xe862`).
- `0xBE9` TFunctor install: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:16553–16559` (`0xe8f1–0xe90f`).
- `SetHeadphoneSelector` TFunctor install: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:16569–16575` (`0xe93f–0xe960`).
- `ResendCurrentMixerSettings`: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:14787` (`0xd014–0xd06e`), step-by-step vtable offsets read directly.
- `SetAuxMonitor` vtable[0xba8] call + opcode 0x9c: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:14780–14786` (`0xcffa–0xd012`).
- `0xBF1` BusReset CallCenter: `docs/system-map/A1-ghidra/otool-exports/mafw.asm:16348` (`0xe5ab`).
- `0xBE9` / `0xBF1` grep: `grep -n "0xbe9\|0xbf1" docs/system-map/A1-ghidra/otool-exports/mafw.asm` → hits at lines 16556 and 16348 only.
- C3 correction (byte 9 = S/MUX not clk_lock): Q15b + session disassembly of `SetClockSourceInternal @ 0xe25c`.
