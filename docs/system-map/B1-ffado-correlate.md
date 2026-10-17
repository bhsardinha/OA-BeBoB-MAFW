# B1 — FFADO cross-reference against Part A1 findings

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - `vt[0xbe0]` = `FW1814Device::SwitchDigitalSignals @0xe5dc`, not an 'AV/C stream-kick after clock change'. Line ~132 ('probably the mechanism behind A1-Q6's 50 ms floor') is a **guess — needs verification** (Q6 re-arm is `0x32 + [+0xd4]` ms @0x1d50d-0x1d513).
> - **Stream kick** in the reference is TX start → 500 ms → INPUT 0x19 → 500 ms → RX start → OUTPUT 0x18 (`ConnectAudioMIDIChannels @0x12156`, asm 0x121c9..0x122bb); the 100 ms order is snd-bebob `bebob_maudio.c:312-337` only. Third-party/macfw statements that 'the stream-kick we locked is hardware truth' are **third-party-only** evidence and are contradicted by the kext order; hardware effect of the 500 ms gaps is **unverified** (check = SL-rig timestamped FCP capture).


> **Correction (2026-10-14) — `0xBA9` / `0xBE9` are not AV/C opcodes.** They are Itanium-ABI virtual member-function pointers (vtable byte offset + 1) stored in batch functors by `SetupBatchChangeMethod @ 0xe184`. `0xBA9` → FW1814 `vt[0xba8]` = `CustomMixerWriteQuads @ 0x22c7a`, a direct async block write to `0xFFC7:0x00700000 + offset`. `0xBE9` → `vt[0xbe8]` = `FW1814Device::AVCControlSetLEDStatus` (A/B-switch LEDs). There is **one** mixer write path (direct register writes, same window as FFADO/macfw/Rust). Group offsets `0x00/0x08/0x10/0x34/0x38` match FFADO `special_avdevice.h:55–69`. Full decode: `A1-ghidra/Q15f-mixer-control-plane-wire.md` Correction 2026-10-14. Any "AV/C batch opcode 0xBA9" or "two mixer paths" statement below is superseded.

**Source material:** `_refs/ffado/src/bebob/maudio/{special_avdevice,special_mixer}.{cpp,h}`, `_refs/ffado/src/bebob/bebob_avdevice.cpp`, `_refs/ffado/src/libstreaming/amdtp/AmdtpTransmitStreamProcessor.cpp`, `_refs/ffado/src/libstreaming/util/cip.h`, `_refs/ffado/src/libavc/avc_definitions.cpp`, `_refs/ffado/src/bebob/maudio/fwap.xml`, cross-checked against A1-Q2/Q5/Q6/Q15/Q15c.

**Scope:** for every A1 finding in {Q2 SetupCIP/FDF, Q5 clock-source + HSCI envelope, Q6 HSCI heartbeat, Q15 mixer typed-property model, Q15c format-change chain}, find or disprove an FFADO analogue, cite file:line, call out disagreements, and recommend `_extracted/` for the clone.

## Status / next step

Done 2026-10-13. **Headline:** FFADO's `MAudio::Special` driver (`is1814 = modelId == 0x00010071`, `special_avdevice.cpp:41`) corroborates three A1 claims with hard provenance — the Vendor_ID/Model_ID, the direct-register control-plane address, and the mixer offset layout — but FFADO is **deliberately thin** on three of our five topics: it ships no FDF/SYT table of its own (it inherits the generic libiec61883 one), it does not expose a clock-source selector (`updateClockSources` hard-codes "Controlled by ALSA" — `special_avdevice.cpp:131..140`), and it has no HSCI heartbeat (no code polls `0xFFC7:0x00600000` anywhere in `_refs/ffado/src`). **Biggest single value-add:** FFADO's `MAUDIO_SPECIFIC_ADDRESS + MAUDIO_CONTROL_OFFSET = 0xFFC700700000` (`special_avdevice.h:40, 47`) **exactly matches** the kext's mixer-write destination at `CustomMixerWriteQuads @ 0x22c7a` (`docs/system-map/A1-ghidra/otool-exports/mafw.asm:41853..41922`: `movw $0xffc7, -0x2e(%rbp); addl $0x700000, %r14d`). The 46-key `MA*` property surface decoded in A2-P3 ultimately resolves to writes against this exact FireWire address window — Q15/Q15b's AV/C-vendor-command story is only half the picture; direct async writes to `0xFFC7:00700000+off` are the other half.

Next: feed §2 into Part C's control-plane synthesis — the clone's mixer write path has two tracks (AV/C vendor-opcode 0xBA9 for typed properties; direct async-FireWire write to `0xFFC7:00700000+off` for Carbon-era legacy offsets). Both must be implemented; neither alone is complete.

## 1. Q2 — SetupCIP / FDF / SYT interval

### FFADO analogue

- `getSytInterval()` at `AmdtpTransmitStreamProcessor.cpp:414..431`
- `getFDF()` at `AmdtpTransmitStreamProcessor.cpp:433..447`
- Rate ↔ SFC mapping at `avc_definitions.cpp:195..216`
- SFC macro `IEC61883_FDF_SFC_MASK = 0x03` at `cip.h:110`; the specific `IEC61883_FDF_SFC_32KHZ` … `IEC61883_FDF_SFC_192KHZ` constants are **not defined in `_refs/ffado/src`** — they come from the system `libiec61883`/`iec61883-private.h` headers at build time. FFADO assumes the canonical values (0..6) without redefining them. **unverified** against FFADO source in-tree, but the canonical AMDTP standard values are 0=32k, 1=44.1k, 2=48k, 3=88.2k, 4=96k, 5=176.4k, 6=192k — identical to A1-Q2's "FDF index" column.

### FFADO vs. A1-Q2

| Fact | FFADO | A1-Q2 (`_extracted/kext/` @ 0x272f6, 0x27102) | Agree? |
|---|---|---|---|
| SYT interval 32k/44.1k/48k = 8 | `AmdtpTransmitStreamProcessor.cpp:417..420` returns 8 | asm 0x272f6 switch: `edx=8` for 32k/44.1k/48k | yes |
| SYT interval 88.2k/96k = 16 | `AmdtpTransmitStreamProcessor.cpp:421..423` returns 16 | asm 0x272f6: `edx=0x10` | yes |
| SYT interval 176.4k/192k = 32 | `AmdtpTransmitStreamProcessor.cpp:424..426` returns 32 | asm 0x272f6: `edx=0x20` | yes |
| FDF index 0..6 ↔ rate | `avc_definitions.cpp:197..203` (SFC → rate) + `210..216` (rate → SFC) | A1-Q2 table "FDF index" col | yes |
| CIP header-2 base `0x90000000` (tag=CIP+fmt=AM824) | derivable: `IEC61883_TAG_WITH_CIP=1` + `IEC61883_FDF_AM824=0x00` (`cip.h:101, 108`) → high byte `0x90` | asm 0x27195 writes `0x90000000` to `+0x49c` (A1-Q2 "CIP header-2 base") | yes |
| NODATA packet FDF `0xFF` | `IEC61883_FDF_NODATA 0xFF` at `cip.h:105` | A1-Q2 `CalcIsochPacketHeaders` writes `0xFFFFFF00 | fdf_hi` for non-sync | yes |

**Agreement is complete on the FDF/SYT invariants.** FFADO uses the canonical IEC-61883 numbers; A1-Q2 shows the kext hard-codes the same.

### Where FFADO is silent (not a disagreement — a gap in FFADO)

FFADO does **not** carry the per-FDF 4-entry accumulator table (`+0x4b0, +0x4b4, +0x4c0, +0x4c4`, i.e. the `{199, 640, 339, 441}` / `{1, 4, 1, 1}` / `{1, 2, 1, 1}` constants from A1-Q2 §Per-FDF table). That accumulator math is M-Audio-kext-specific; FFADO's AMDTP packer uses a different fractional-SYT algorithm (iec61883_fraction struct at `cip.h:134..154`). **This is fine** — the clone implements the kext's constants, not FFADO's — but it means A1-Q2's `{199,640,339,441}` quadruple has no FFADO corroboration and must stand on the Ghidra read alone. (Already cited in A1-Q2 provenance.)

### Recommendation for the clone

Ship the **A1-Q2 7-row table verbatim**. FFADO corroborates SYT interval + FDF index + 0x90000000 header-2 base + NODATA pattern. The per-FDF accumulator constants come from the kext only.

## 2. Q5 — Clock source + HSCI 16-byte envelope

### FFADO analogue — essentially absent

`special_avdevice.cpp:131..157` reduces the whole clock-source UX to:

```cpp
void Device::updateClockSources() {
    m_fixed_clksrc.type = FFADODevice::eCT_Internal;
    m_fixed_clksrc.active = true;
    m_fixed_clksrc.valid = true;
    m_fixed_clksrc.locked = true;
    m_fixed_clksrc.id = 0;
    m_fixed_clksrc.slipping = false;
    m_fixed_clksrc.description = "Controlled by ALSA";
}
```

- `getSupportedClockSources()` returns the single `m_fixed_clksrc` (`special_avdevice.cpp:141..147`).
- `setActiveClockSource(ClockSource)` is a no-op that returns true (`special_avdevice.cpp:153..157`).
- The comment at `special_avdevice.cpp:127..130` admits: *"Clock Source support: TODO: use ALSA control interface"*.

**FFADO never emits the 16-byte HSCI envelope `00 FF 00 04 00 04 <enum> 00 00 00 ...`** that A1-Q5 decoded from `com_m_audio_FW1814Device::SetClockSourceInternal @ 0xe25c`. Grepping the whole FFADO tree:

```
_refs/ffado/src   grep -rn "0x40004\|00 FF 00 04 00 04\|00ff000400" → no matches
```

(verified 2026-10-13 — no results in `_refs/ffado/src`).

**Nor does FFADO expose the Internal-muted / External-digital / Word Clock / Internal-unmuted bijection** that A1-Q5's final table locks (via `pulled/69-dtrace-syncradios-spdifport.txt`).

### FFADO vs. A1-Q5

| Fact | FFADO | A1-Q5 | Agree? |
|---|---|---|---|
| 4 Sync Source radios (Int-muted, Int-unmuted, External-digital, Word Clock) | **absent** (hardcoded Internal only) | 4 radios, 32-bit `MAClockSource` low bits `00/01/10/11` → wire byte `3/1/2/0` | n/a — FFADO does not implement |
| Composite bitfield (bit 4 = optical-out, bit 5 = S/MUX, bit 2 = S/PDIF-port) | **absent** | 4-way bitfield, decoded from capture 66 + Ghidra `SetClockSourceInternal` | n/a — FFADO does not implement |
| 16-byte HSCI payload header `00 FF 00 04 00 04` | **absent** | hardcoded in kext @ `0xe25c` | n/a |
| 300 ms post-HSCI settle | **absent** | `IOSleep(0x12c)` ×2 in `SetClockSourceInternal` | n/a |
| AV/C stream-kick after clock change (`vtable[0xbe0]`) | **absent** | `OUTPUT 0x18 + 100ms + INPUT 0x19` (memory `project_fw1814_stream_kick`) | n/a — FFADO delegates clock to ALSA, never kicks |

**No disagreement because FFADO opts out.** This is a known limitation acknowledged inline in FFADO source (`special_avdevice.cpp:127..130`).

### Why FFADO can opt out

FFADO relies on ALSA's `snd-bebob` (and later `snd-fireworks` / `snd-firewire-ctl-services`) for actual clock-source control; FFADO's responsibility is streaming + mixing. On M-Audio Special devices this split means **the clock-source wire protocol lives only in the ALSA kernel driver + the macOS kext**. B2 (snd-bebob cross-reference) should pick up whether the ALSA driver matches A1-Q5's envelope — this is the obvious next check.

### Recommendation for the clone

Ship A1-Q5 verbatim; FFADO provides zero corroboration on this path, so A1-Q5's authority rests on:
- Ghidra decompile of `0xe25c` (`docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log`),
- SL DTrace captures 66, 68, 69 (`pulled/`).

No FFADO disagreement, but no FFADO support either. Mark the A1-Q5 bijection as "unverified against ALSA" until B2 lands.

## 3. Q6 — HSCI 50 ms heartbeat (`m_audio_b_FWHSCI`)

### FFADO analogue — absent

Grep across all of `_refs/ffado/src`:

```
grep -rn "0xFFC7.*0x600000\|0xFFC700600000\|0x00600000" _refs/ffado/src → no matches
```

(verified 2026-10-13).

FFADO's `special_avdevice.cpp` performs **no asynchronous read polling** against the FW1814. All transactions are write-only (`writeReg`/`writeBlk`). The driver explicitly notes at `special_avdevice.cpp:174` that it cannot query parameters:

> *"FW1814/ProjectMix don't use AVC for control. The driver cannot refer to current parameters by asynchronous transaction. The driver is allowed to write transaction so MUST remember the current values."*

→ FFADO caches writes in a 160-byte `m_regs` array (`special_avdevice.h:181`) and never reads back from the device.

### FFADO vs. A1-Q6

| Fact | FFADO | A1-Q6 | Agree? |
|---|---|---|---|
| HSCI poll address `0xFFC7:0x00600000` | **absent** | asm `0x1d63b` (`movw $0xffc7`) + `0x1d641` (`$0x600000`) | n/a |
| 50 ms repeating timer | **absent** | `0x32` ms at `0x1d464`, back-off `0x3e8` ms at `0x1d776` | n/a |
| 12-byte envelope: 3 quadlets payload | **absent** | `HSCIReadComplete @ 0x1d81e` parses 3 quadlets | n/a |
| Pause/Resume nesting (`fPauseCount` at `+0x1c`) | **absent** | Pause @ `0x1d3fc`, Resume @ `0x1d424` | n/a |

**Not a disagreement.** FFADO deliberately avoids any read traffic to the Special devices to work around the firmware freeze (`special_avdevice.cpp:56..63, 161..168`), which is itself informative: the FW1814 firmware **can't tolerate arbitrary async reads**, which is why the kext confines its reads to the 50 ms HSCI heartbeat window at `0xFFC7:0x00600000` and nowhere else.

### Observation (implication for the clone)

The FFADO comment "firmware can't respond against frequent requests" (`special_avdevice.cpp:161..163`) is probably the mechanism behind A1-Q6's 50 ms **floor** on the heartbeat and the 1000 ms **back-off** on error (asm `0x1d776`). The clone must not poll faster than 50 ms, and must not add new read paths outside this window.

### Recommendation for the clone

Ship A1-Q6 verbatim. FFADO's silence here is corroboration-by-omission — the firmware is read-shy, so the kext's narrow 50 ms read window is a hardware requirement, not an implementation choice.

## 4. Q15 — Mixer typed-property / ChannelGroup framework

This is the richest cross-reference section. FFADO's `special_mixer.cpp` provides a **flat offset table** that gives us ground-truth hardware offsets independent of the kext's typed-property abstraction. The kext layers a `FWChannelGroupProperty` + `BaseChannelGroup::SetupBatchChangeMethod` framework on top; **the underlying hardware writes are the same**.

### 4.1 Register map agreement

FFADO's `special_avdevice.h:50..134` is a complete offset table into the device's control window at `0xFFC700700000 + offset`. A1-Q15 did **not** publish this offset table (it stayed inside the `FWChannelGroupProperty` abstraction). FFADO's table is therefore the first ground-truth source for which hardware offset carries which semantic:

| Offset | FFADO macro (`special_avdevice.h:L`) | Semantic (per FFADO comment at `L`) |
|---|---|---|
| `0x00` | `GAIN_STM_12_IN` (`:55`) | Stream 1/2 input gain |
| `0x04` | `GAIN_STM_34_IN` (`:56`) | Stream 3/4 input gain |
| `0x08` | `GAIN_ANA_12_OUT` (`:57`) | Analog 1/2 output gain |
| `0x0C` | `GAIN_ANA_34_OUT` (`:58`) | Analog 3/4 output gain |
| `0x10..0x1C` | `GAIN_ANA_{12,34,56,78}_IN` (`:59..62`) | Analog 1-8 input gain (4 stereo pairs) |
| `0x20` | `GAIN_SPDIF_12_IN` (`:63`) | S/PDIF 1/2 input gain |
| `0x24..0x30` | `GAIN_ADAT_{12,34,56,78}_IN` (`:64..67`) | ADAT 1-8 input gain (4 stereo pairs) |
| `0x34` | `GAIN_AUX_12_OUT` (`:68`) | AUX 1/2 output gain |
| `0x38..0x3C` | `GAIN_HP_{12,34}_OUT` (`:69..70`) | Headphone 1/2 and 3/4 output gain |
| `0x40..0x60` | `LR_ANA_{12,34,56,78}_IN`, `LR_SPDIF_12_IN`, `LR_ADAT_{12,34,56,78}_IN` (`:77..85`) | L/R balance for each stereo input |
| `0x64..0x8C` | `AUX_{STM,ANA,SPDIF,ADAT}_*_IN` (`:90..100`) | Aux-send levels per source |
| `0x90` | `MIX_ANA_DIG_IN` (`:119`) | Mixer routing: analog + digital inputs → Mix 1/2 or Mix 3/4 (bitflags) |
| `0x94` | `MIX_STM_IN` (`:120`) | Mixer routing: stream inputs → Mix 1/2 or Mix 3/4 (bitflags) |
| `0x98` | `SRC_HP_OUT` (`:133`) | Headphone source selector (bits for Mix12/Mix34/Aux12) |
| `0x9C` | `SRC_ANA_OUT` (`:134`) | Analog output source selector (bits for Mix/Aux per 1/2 or 3/4) |

Total controlled window: `0x00..0x9C` = **160 bytes = 40 quadlets** of mixer state.

### 4.2 Cross-check against A1-Q15 object layouts

A1-Q15's `FWSettings1814Levels` structure (`+0x010..+0x9f0`, ≥ 0xa50 bytes total) with "16 ChannelGroupProperty on Levels + 4 on AuxSend" maps onto the FFADO offset table as follows (my implication — label as **hypothesis** until Q15a extracts the OSSymbol keys):

| FFADO offset block | FFADO count | A1-Q15 counterpart (hypothesis) | Verification path |
|---|---|---|---|
| `0x10..0x30` (ANA/SPDIF/ADAT inputs, 9 × quadlet gain) | 9 stereo-pair input gains | 1 of the 4 ChannelGroupProperty rows on `FWSettings1814Levels[+0x130]..+0x6d0` | grep OSSymbol strings near `0xeec0` ctor (Q15a) |
| `0x00..0x08` (STM/ANA output gains, 4 quadlets) | 4 stereo-pair output gains | ArrayProp #1 at `[+0x010]` or Group umbrella | Q15a |
| `0x34..0x3c` (AUX/HP outputs, 3 quadlets) | 3 stereo-pair master gains | `FW32BitNumberProp #1/#2/#3` at `[+0x070/0x0b0/0x0f0]` | Q15a |
| `0x40..0x60` (L/R balance, 9 quadlets) | 9 pan values | 1 of the 4 ChannelGroupProperty rows | Q15a |
| `0x64..0x8c` (aux-send, 11 quadlets) | aux-send per source | 4 ChannelGroupProperty on `FWSettings1814AuxSend` + 2 ArrayProp | Q15a |
| `0x90, 0x94` (mix routing, 2 bitfield quadlets) | 2 routing bitmaps | `SetInputRouting` vtable slot (A1-Q15 §SetInputRouting @ `0xd15c`) | agrees in semantic, byte-level TBD |
| `0x98, 0x9c` (output source select, 2 bitfield quadlets) | 2 selector bitmaps | `SetAuxMonitor` + `SetInputPan` vtable slots? | **hypothesis — verify** by decompiling `com_m_audio_FW1814Device::SetAuxMonitor` (opened as Q15b-mix-selector) |

### 4.3 Where FFADO and A1-Q15 agree

- **Both use direct writes to `0xFFC700700000 + offset`.** FFADO: `writeReg/writeBlk` in `special_avdevice.cpp:200..250`. Kext: `CustomMixerWriteQuads @ 0x22c7a` writes `movw $0xffc7, -0x2e(%rbp); addl $0x700000, %r14d; movl %r14d, -0x2c(%rbp)` (`docs/system-map/A1-ghidra/otool-exports/mafw.asm:41907..41916`) then dispatches via FireWire write command vtable slot `0x890` on the controller.
- **Both encode gain as upper-16-bit L + lower-16-bit R per quadlet.** FFADO: `special_avdevice.h:52..54` ("upper 16bit for left channel and lower 16bit for right ... 0x8000 (low) to 0x0000 (high) ... same as 10.3.1 Volume Control in AV/C Audio Subunit Specification 1.0"). Kext: A1-Q15 `OSArrayToSeriatimValues` unpacks per-channel values into slot-pairs; the quadlet packing is implicit in the AV/C vendor-command payload constructed inside `BaseChannelGroup::ApplyAdjustedLevels(before)`. **unverified** at exact byte level in the kext — Q15 noted it as "AV/C vendor-command payload" without decoding the quadlet packing; FFADO's "upper16=L, lower16=R" from `special_avdevice.h:52..54` is the authoritative decode.
- **The 0x8000 fully-attenuated / 0x0000 fully-open semantic is AV/C-standard.** `special_avdevice.h:54` cites 1394TA 1999008 §10.3.1; FFADO just passes the AV/C scalar straight to the register. A1-Q15 did not re-decode this scale (left as "per-channel level values"); the clone adopts the AV/C-standard scale.

### 4.4 Where FFADO and A1-Q15 disagree

**The frameworks differ, but the hardware substrate is the same.** FFADO exposes a flat `/Mixer/Feature_Volume_{1..27}` namespace (`special_mixer.cpp:202..237`, 27 feature-volumes + 9 LR-balances + 4 selectors + 2 processing = 42 Control::Elements). The kext organizes the same 42 controls as 16 ChannelGroupProperty + 4 ArrayProp + batching infrastructure on `FWSettings1814Levels` plus 4 ChannelGroupProperty + 2 ArrayProp on `FWSettings1814AuxSend`.

| Aspect | FFADO | A1-Q15 kext | Winner for the macOS clone |
|---|---|---|---|
| Framework shape | 42 flat Control::Elements on `/Mixer/*` | typed-property dict + ChannelGroupProperty + BatchChangeMethod | **kext** (Panel compatibility; CoreAudio HAL needs IORegistry+setProperties, not an ALSA-style control tree) |
| Hardware write path | Direct async FireWire write to `0xFFC700700000+off` | **BOTH**: direct async write via `CustomMixerWriteQuads @ 0x22c7a` (same address) AND AV/C vendor-opcode `0xBA9` via `SetupBatchChangeMethod` (A1-Q15 §Batch-apply) | **kext** — but FFADO reveals that there is a direct-write-only path in the kext too, which is a Part A gap (A1-Q15 did not surface `CustomMixerWriteQuads` at all) |
| Hardware offset documentation | **FFADO WINS** — 160-byte offset table with per-offset comments at `special_avdevice.h:50..134` | A1-Q15 skipped the offset-to-semantic mapping | **FFADO offsets** (promote into Q15a/Q15b decode of `com_m_audio_FW1814Device` vtable slots) |
| Gain scaling 0x0000..0x8000 | documented at `special_avdevice.h:52..54` | implicit | **FFADO decoding** |
| Mix routing bitfield at `0x90`/`0x94` | documented at `special_avdevice.h:102..120` | A1-Q15 says `SetInputRouting` at vtable `[+0xbb8]` but byte-level unknown | **FFADO decoding** for the register bits; kext source-of-truth confirms the register is reached via `vtable[+0xbb8]` *or* via `CustomMixerWriteQuads(ch=?, offset=0x90, data=…)` — need Q15 follow-up to establish which path the kext actually takes |

**Resolution rule applies:** where FFADO and the kext diverge on **framework**, the kext wins (`_extracted/` is the macOS target). Where FFADO decodes a **register bit** that the kext opaquely treats as a vendor-command payload, FFADO's decoding wins **as documentation** — but the clone must mirror the kext's **transport** (which may use AV/C vendor-opcode 0xBA9 wrapping the same bit layout; this is unverified byte-for-byte against FFADO's offset table).

### 4.5 Candidate A1 gap — the `CustomMixerWriteQuads` path

A1-Q15 asserts that mixer writes flow through `BaseChannelGroup::SetupBatchChangeMethod` → AV/C vendor-opcode `0xBA9`. But `com_m_audio_FWAVCAudioDevice::CustomMixerWriteQuads @ 0x22c7a` (`docs/system-map/A1-ghidra/otool-exports/mafw.asm:41853..41922`) performs a direct async FireWire write to `0xFFC700700000+off` — **not** an AV/C transaction. The call chain:

```
CustomMixerWriteQuads(this, chan /*ch?*/, numQuads, void *src)
  → AllocateCustomMixerWriteBuffer(numQuads, &buf)       [0x22cbd]
  → memcpy(buf.data, src, numQuads*4)                     [0x22ced]
  → vtable[0x890](controller, {addrHi=0xFFC7, addrLo=0x00700000+chan*4}, buf, …) [0x22d29]
  → ReleaseCustomMixerWriteBuffer(buf)
```

This is the direct-async-write path FFADO uses. It **coexists** with the AV/C vendor-opcode 0xBA9 batch path. **Which path is used for which panel event is a Part A gap** — opens Q15f.

**Guess — needs verification by decompiling `com_m_audio_FW1814Device::SetInputRouting @ vtable[+0xbb8]`:** typed-property writes from the Panel ("set a volume slider") route through the ChannelGroup → AV/C-0xBA9 path for *change notification + delta commit*; low-latency direct writes (e.g. real-time mixer-routing bitfield flips) route through `CustomMixerWriteQuads` for *per-quadlet pokes*. **Verification path:** grep call sites of `CustomMixerWriteQuads` (`docs/system-map/A1-ghidra/otool-exports/mafw.asm` → `__ZN26m_audio_b_FWAVCAudioDevice21CustomMixerWriteQuads*`) and tie each call site to a specific Panel-facing semantic.

### Recommendation for the clone

- Ship the FFADO offset table **as documentation** inside the clone's `include/FW1814MixerMap.h` — this is independent ground truth for which quadlet carries which semantic.
- Mirror the kext's dual-path architecture: implement **both** the typed-property + AV/C-0xBA9 batch path (for Panel-visible controls, change notifications, delta commits) and the direct-async-write path (`CustomMixerWriteQuads` equivalent at `0xFFC700700000+off`).
- Open **Q15f** to establish which Panel events use which path.

## 5. Q15c — Format-change chain (`performFormatChange` → `RequestFormatUpdateForSampleRate` → `SetIOFormat` → `PerformSampleRateChangeCore`)

### FFADO analogue — absent / stubbed

`special_avdevice.cpp:111..124`:

```cpp
std::vector<int> Device::getSupportedSamplingFrequencies() {
    std::vector<int> freqs;
    freqs.push_back(0);                     // "0" sentinel
    return freqs;
}
bool Device::supportsSamplingFrequency(int s) { return true; }
int  Device::getSamplingFrequency() { return 0; }
```

**FFADO cannot change the sample rate on Special devices.** It returns a single sentinel entry `{0}` for supported rates, lies by returning true for any rate, and returns 0 for the current rate — all because, per the file comment at `special_avdevice.cpp:106..108`:

> *"Sampling frequency support: set: Input/Output Signal Format command; get: Input Signal Format command (or metering information); TODO"*

FFADO leaves this as a TODO. The AV/C `INPUT/OUTPUT SIGNAL FORMAT` commands that A1-Q15c identifies as the kext's commit path (via `vtable[+0xce0]`) are **never issued** by FFADO's Special driver.

### FFADO vs. A1-Q15c

| Fact | FFADO | A1-Q15c | Agree? |
|---|---|---|---|
| 7-rate ladder {32k, 44.1k, 48k, 88.2k, 96k, 176.4k, 192k} | stubbed to `{0}` | explicit `switch` at asm 4138–4194 | n/a |
| Per-rate channel count 8/8/8/16/16/32/32 | **absent** | engine `[+0x224]` field set by `PerformSampleRateChangeCore` | n/a |
| Workloop + `IOCommandGate::runAction` on format change | n/a (not kernel-side) | `FWBaseEngine::performFormatChange @ 0x3e54` runs on gate | n/a |
| AV/C commit via `vtable[+0xce0]` | **absent (TODO)** | subclass override at asm 21098 | n/a |
| HSCI envelope emit on format change (`vtable[+0xaf8]`) | **absent** | asm 3830 | n/a |
| DCL program per-rate reload | **absent (userspace streaming)** | iterate `[+0x178]` collection, call `vt[+0x128]` on each | n/a |
| ResetLatency unconditional on rate change | **absent** | asm 4210 | n/a |
| AC3 fork (`'cac3'` tag → EnableAC3Format) | **absent** | A1-Q15c §AC3 fork at asm 21037–21101 | n/a |

**No disagreement because FFADO opts out.** Rate-change on FW1814 under FFADO is simply not implemented.

### Recommendation for the clone

Ship A1-Q15c verbatim. FFADO provides zero corroboration on the format-change chain. B2 (snd-bebob) is the next cross-reference opportunity — if snd-bebob implements rate-change on `0x00010071`, that would be an independent third voice.

## 6. What FFADO tells us that A1 missed — candidate A1 gaps

### 6.1 (Major) `CustomMixerWriteQuads` is a parallel mixer write path A1-Q15 did not surface

Already discussed in §4.5. **This is a confirmed A1 gap.** A1-Q15 asserts the mixer flows exclusively through `BaseChannelGroup::SetupBatchChangeMethod` + AV/C vendor-opcode 0xBA9. The disassembly of `0x22c7a..0x22d74` (`docs/system-map/A1-ghidra/otool-exports/mafw.asm:41853..41922`) shows a direct async FireWire write path to `0xFFC700700000+off` that bypasses the AV/C stack entirely — matching FFADO's `writeReg/writeBlk` to the identical address window. **Action:** open **Q15f** (new Qn) — "`CustomMixerWriteQuads` call sites + which Panel events route through this vs. AV/C-0xBA9."

### 6.2 (Major) Gain quadlet encoding is AV/C-standard 0x8000..0x0000, not kext-opaque

Already discussed in §4.3. **Action:** promote FFADO's `special_avdevice.h:52..54` scale note into the clone's `FW1814MixerMap.h`; cite as "quadlet packing per AV/C Audio Subunit Specification 1.0 §10.3.1 (1394TA 1999008)".

### 6.3 (Medium) Mix routing bitfields at `0x90/0x94` and selector bitfields at `0x98/0x9C` are documented register layouts

A1-Q15 named the vtable slot (`SetInputRouting @ +0xbb8`) but didn't decode the bitfield at offset `0x90`. FFADO's `special_avdevice.h:102..134` and `special_mixer.cpp:376..480` give the exact bit layout:
- `0x90` upper 16 bits = **digital inputs routing** (S/PDIF 2 bits + ADAT 8 bits), lower 16 bits = **analog inputs routing** (4 stereo pairs × 2 destinations).
- `0x94` lower 4 bits = **stream inputs** (2 streams × 2 destinations).
- `0x98` lower 3 bits = **HP source** ("from Aux12", "from Mix34", "from Mix12").
- `0x9C` lower 2 bits = **analog output source** ("to Ana34", "to Ana12", each bit: 1 = from Aux12, 0 = from Mix).

**Action:** copy this decoding into Q15a/Q15b supplemental notes; label "FFADO-sourced register bit layout, cross-check against `_extracted/kext/` at Q15f when `CustomMixerWriteQuads` call sites are decoded."

### 6.4 (Medium) FFADO's `is1814` guard rails out AV/C `ExtendedPlugInfo` on `0x00010071` and `0x00010091`

`bebob_avdevice.cpp:87..92`:

```cpp
/* M-Audio Special Devices don't support followed commands */
if ((vendorId == FW_VENDORID_MAUDIO) &&
    ((modelId == 0x00010071) || (modelId == 0x00010091)))
    return true;
```

→ FFADO **skips** the `ExtendedPlugInfoCmd` probe for FW1814 (0x10071) and ProjectMix I/O (0x10091) because the firmware freezes on it.

**Cross-check with A1-Q3** (`SetupAudioEngine + ConnectAllPlugs`): A1-Q3 says the kext does a "3-step plug gate" + `initHardware` cross-links. **unverified** whether the kext issues `ExtendedPlugInfo` — if it does, that's an FFADO disagreement; if it does not (same as FFADO), that's silent agreement. **Verification path:** grep `ExtendedPlug` / `0x40 0x01 0xc8` (AV/C EXTENDED PLUG INFO byte sequence) in `docs/system-map/A1-ghidra/otool-exports/mafw.asm`. **Action:** open **Q3a** — "does the kext issue AV/C EXTENDED PLUG INFO?"

### 6.5 (Minor) FFADO's `getStreamingState()` returns `eSS_Both` unconditionally for Special devices

`special_avdevice.cpp:164..168`:

```cpp
enum FFADODevice::eStreamingState Device::getStreamingState() {
    return eSS_Both;
}
```

With the comment "firmware can't respond against frequent requests. TODO: how to void frequent transaction?" (`special_avdevice.cpp:161..163`).

**Cross-check with A1-Q4** (`ReconnectP2P + BusReset`): the kext's bus-reset path (`InvokeOnNewThread` + CallCenter list) tracks streaming state internally. FFADO's workaround — assume always-streaming — is a reasonable Linux-side bodge but **does not inform the kext**. No action.

### 6.6 (Minor) `writeBlk` initializes 160 bytes of state with specific defaults

`special_mixer.cpp:74..106` `Mixer::initialize`:

```cpp
/* inputs volumes */        data[0..15]  = 0x00000000;    // 16 gain quadlets
/* inputs panning */        data[16..24] = 0x7ffe8000;    // 9 LR-balance quadlets, centered
/* aux inputs volume */     data[25..35] = 0x00000000;    // 11 aux-send quadlets
/* analog inputs to mixer */data[36]     = 0x00000000;    // 0x90
/* stream inputs to mixer */data[37]     = 0x00000000;    // 0x94
/* headphone sources */     data[38]     = 0x00000000;    // 0x98
/* output sources */        data[39]     = 0x00000000;    // 0x9c
```

→ **40 quadlets × 4 bytes = 160 bytes = `0x9c + 4` bytes**, pushed in a single `writeBlk` transaction.

**Cross-check with A1-Q3** `initHardware` + A1-Q15 `FWSettings1814Levels::ctor @ 0xeec0`: Q15 noted ArrayProp and 32BitNumberProp slots but did not state the initial values. **unverified** whether the kext issues an equivalent single-shot 160-byte initialization write. **Verification path:** grep for `0x7ffe8000` or `0x7FFE8000` in `docs/system-map/A1-ghidra/otool-exports/mafw.asm`. **Action:** open **Q3b** — "kext mixer power-on default values, cross-check against FFADO's `0x7ffe8000` center-pan default."

### 6.7 (Minor) FFADO's AC3-related code is **absent** — i.e. no FFADO corroboration for A1-Q12

FFADO's `MAudio::Special::Device` has no AC3 passthrough support (no 'cac3' format, no `ActivateAC3Output` equivalent). This is **not a gap** — just a confirmation that AC3 passthrough is an advanced Mac-only feature the kext implements in `_extracted/kext/` @ A1-Q12.

### 6.8 (Not a gap, cross-check for A1-Q11) FW1814 MIDI stream in FFADO's XML descriptor

`fwap.xml` is for `FW Audiophile` (`<GUID>0x000D6C031088FA68</GUID>`), **not** FW1814 (there's no `fw1814.xml` in `_refs/ffado/src/bebob/maudio/`). The descriptor documents an **Audiophile** stream map with 1 MidiPort per direction (Format=13, Type=10, Position=6/4 in the stream). A1-Q11 says FW1814 has a MIDI RPC path through `FWUserClient` selectors 500/501/502/503 + a shared ring at `clientMemoryForType(1)`. **No conflict** — Audiophile and FW1814 are separate hardware with separate stream maps. **Observation:** FFADO's lack of a `fw1814.xml` is additional evidence that FFADO never fully modeled FW1814 at the stream-config level (consistent with the FFADO stubs in `special_avdevice.cpp` throughout).

## 7. Summary deltas table

| A1 topic | FFADO corroborates? | FFADO disagrees? | FFADO adds new ground truth? |
|---|---|---|---|
| Q2 SYT interval + FDF index | yes (`AmdtpTransmitStreamProcessor.cpp:414..447`) | no | no |
| Q2 CIP header-2 base `0x90000000` | yes (`cip.h:101, 108`) | no | no |
| Q2 NODATA pattern | yes (`cip.h:105`) | no | no |
| Q2 per-FDF accumulator constants (339/441/640/199) | no (not in FFADO) | no | no |
| Q5 clock-source bijection (4 radios) | no (stubbed) | no | no |
| Q5 16-byte HSCI envelope | no | no | no |
| Q6 HSCI 50 ms heartbeat at `0xFFC7:0x00600000` | no (FFADO is write-only) | no | implies firmware is read-shy (`special_avdevice.cpp:161..163`) |
| Q15 typed-property framework | no (FFADO uses flat Control::Elements) | **framework-level disagreement, kext wins for macOS** | **yes** — mixer offset table at `special_avdevice.h:50..134`, gain packing at `:52..54`, bitfield decodings at `:102..134` |
| Q15 direct-write path at `0xFFC700700000+off` | **yes** — FFADO's `writeReg` matches kext's `CustomMixerWriteQuads @ 0x22c7a` | no | **yes** — `CustomMixerWriteQuads` is a parallel path A1-Q15 missed (open Q15f) |
| Q15c format-change chain | no (FFADO stubs sample-rate) | no | no |
| Q15c AC3 fork | no | no | no |

## 8. Implications for Part C / D

1. **Part C mixer section must enumerate TWO write paths**, not one. (a) AV/C vendor-opcode 0xBA9 batch commit via `SetupBatchChangeMethod` (A1-Q15) and (b) direct async FireWire write to `0xFFC700700000+off` via `CustomMixerWriteQuads @ 0x22c7a` (FFADO cross-ref). Open **Q15f** to decode which path each Panel event uses.
2. **Part C mixer-offset documentation adopts FFADO's `special_avdevice.h:50..134` table as independent ground truth** for the 40-quadlet register window. Cross-cite A1-Q15 for the framework; FFADO for the register semantics.
3. **A1-Q5 and A1-Q6 stand on `_extracted/kext/` alone** — FFADO contributes nothing to clock-source control or HSCI heartbeat. Mark these findings as "requires B2 (snd-bebob) corroboration" in Part C.
4. **A1-Q15c (format-change chain) stands on `_extracted/kext/` alone** — FFADO explicitly stubs rate-change for Special devices. Mark as "requires B2 corroboration."
5. **Pre-code-work actions in Part D-scope but discovered here:**
   - Open **Q15f** — `CustomMixerWriteQuads` call-site enumeration vs. AV/C-0xBA9 batch path.
   - Open **Q3a** — does the kext issue AV/C `ExtendedPlugInfo` against FW1814? (FFADO explicitly skips it.)
   - Open **Q3b** — kext mixer power-on default values, cross-check against FFADO's center-pan `0x7ffe8000`.
6. **The clone's `include/FW1814MixerMap.h`** should contain FFADO's complete offset table — it is the most compact ground-truth control-plane documentation we have, better than our current Q15 object-layout dump.
7. **FFADO's `is1814 = modelId == 0x00010071`** at `special_avdevice.cpp:41` is a third-party corroboration of the runtime Model_ID in `memory/project_fw1814_ids.md`. Independent of the kext's `IOKitPersonalities` plist entries. No action needed — Model_ID was never in doubt — but worth noting for Part C provenance.
8. **The firmware-freezes-on-reads constraint** from `special_avdevice.cpp:161..163` explains both A1-Q6's narrow 50 ms heartbeat window and FFADO's write-only policy. Part C should document this as a hardware-level invariant: "no asynchronous reads outside the 50 ms HSCI window at `0xFFC7:0x00600000`."

## 9. Cross-references

- **A1-Q2** — SYT interval + FDF table (fully corroborated by FFADO AmdtpTransmitStreamProcessor + cip.h).
- **A1-Q5** — clock-source bijection + 16-byte HSCI envelope (no FFADO corroboration; stands alone).
- **A1-Q6** — HSCI 50 ms heartbeat at `0xFFC7:0x00600000` (no FFADO corroboration; stands alone).
- **A1-Q15 / Q15b** — mixer typed-property + ChannelGroup framework (framework differs from FFADO; FFADO adds the hardware offset table and gain packing).
- **A1-Q15c** — format-change chain (no FFADO corroboration; stands alone).
- **A2-P3** — Panel 46 `MA*` property keys (`MAClockSource` was already decoded as composite bitfield in A1-Q5 Addendum; FFADO contributes nothing further).
- **B3** — Sierra kextload-hack delta (orthogonal to this cross-reference; loader politics, not protocol).
- **B5 §10, §13** — SL DTrace captures (FFADO irrelevant to macOS runtime; B5 remains the primary ground truth for the live wire layer on 10.6.8).

## 10. Reproducing the cross-reference

```sh
# FFADO FW1814 code
ls _refs/ffado/src/bebob/maudio/
# → fw410.xml fwap.xml normal_avdevice.cpp normal_avdevice.h refdesign.xml
#   special_avdevice.cpp special_avdevice.h special_mixer.cpp special_mixer.h

# Confirm Model_ID match
grep -n "0x00010071\|is1814" _refs/ffado/src/bebob/maudio/*.cpp
# → special_avdevice.cpp:41:    is1814 = (getConfigRom().getModelId() == 0x00010071);

# Confirm MAUDIO_SPECIFIC_ADDRESS matches kext
grep -n "0xffc700\|MAUDIO_SPECIFIC\|MAUDIO_CONTROL" _refs/ffado/src/bebob/maudio/*.h
# → special_avdevice.h:40: #define MAUDIO_SPECIFIC_ADDRESS 0xffc700000000
# → special_avdevice.h:47: #define MAUDIO_CONTROL_OFFSET   0x00700000

# Confirm no HSCI poll address in FFADO
grep -rn "0xFFC7.*00600000\|0x00600000" _refs/ffado/src
# → no matches

# Confirm no clock-source wire envelope in FFADO
grep -rn "00 FF 00 04 00 04\|0x40004\|SetClockSource" _refs/ffado/src/bebob/maudio/
# → no matches (stubbed; see special_avdevice.cpp:131..157)

# Confirm the kext CustomMixerWriteQuads path writes to 0xFFC700700000+off
grep -n "0x700000\|CustomMixerWriteQuads" docs/system-map/A1-ghidra/otool-exports/mafw.asm | head
# → 41853: __ZN26m_audio_b_FWAVCAudioDevice21CustomMixerWriteQuadsEjjPv:
# → 41894: 0000000000022d0f addl $0x700000, %r14d  ## imm = 0x700000
# → 41895: 0000000000022d09 movw $0xffc7, -0x2e(%rbp)  ## imm = 0xFFC7
```
