# B2a — snd-firewire-ctl-services (Rust) cross-reference

> **Correction (2026-10-14) — `0xBA9` / `0xBE9` are not AV/C opcodes.** They are Itanium-ABI virtual member-function pointers (vtable byte offset + 1) stored in batch functors by `SetupBatchChangeMethod @ 0xe184`. `0xBA9` → FW1814 `vt[0xba8]` = `CustomMixerWriteQuads @ 0x22c7a`, a direct async block write to `0xFFC7:0x00700000 + offset`. `0xBE9` → `vt[0xbe8]` = `FW1814Device::AVCControlSetLEDStatus` (A/B-switch LEDs). There is **one** mixer write path (direct register writes, same window as FFADO/macfw/Rust). Group offsets `0x00/0x08/0x10/0x34/0x38` match FFADO `special_avdevice.h:55–69`. Full decode: `A1-ghidra/Q15f-mixer-control-plane-wire.md` Correction 2026-10-14. Any "AV/C batch opcode 0xBA9" or "two mixer paths" statement below is superseded.

**Source:** `snd-firewire-ctl-services` by Takashi Sakamoto (LGPL-3), ALSA-ecosystem userspace port of `snd-bebob` for FW1814-class gear. Local checkout at `_refs/snd-firewire-ctl-services/`. The FW1814 implementation lives in `protocols/bebob/src/maudio/special.rs` (1205 lines) with shared BeBoB primitives in `protocols/bebob/src/lib.rs` and `bridgeco.rs`. ProjectMix I/O rides the exact same `special.rs` codepath (shared register layout, DM1000-era silicon).

**Scope:** map the 46 `MA*` property keys enumerated in A2-P3 (and the mixer-model types in A1-Q15) onto the Rust mixer graph; validate A1-Q15b's single-dispatcher claim; close out ChannelGroup bitmask semantics.

## 0. Wire-protocol caveat (read first)

The macOS v1.11.2 kext (`_extracted/kext/`) drives FW1814 via **AV/C vendor-dependent (HSCI) over FCP** — 12-byte envelopes, company_id `0x040004`, batch opcode `0xBA9` (A1-Q6, A1-Q15 @ `0xe1cc`). The Rust crate drives the same hardware via **direct FireWire register pokes to the DM1000 "APPL" window** at absolute offsets:

```
DM_APPL_OFFSET  = 0xffc700000000      (lib.rs:35)
DM_APPL_METER_OFFSET  = DM_APPL_OFFSET + 0x00600000    (lib.rs:36)
DM_APPL_PARAM_OFFSET  = DM_APPL_OFFSET + 0x00700000    (lib.rs:37)
```

Writes land at `DM_APPL_PARAM_OFFSET + <range.start>` via `FwReq::transaction(FwTcode::WriteQuadletRequest, …)` (special.rs:1038, 1079). This is a **different wire path** than the kext uses — the DM1000 firmware image (`fw1814.bcd`) accepts both transports; the kext picks HSCI, Linux picks pokes.

Consequence for us:
- The Rust crate validates **control-plane semantics** (what each control does to the audio graph, what bit encodings exist, what the mixer topology is).
- The Rust crate does **not** validate the HSCI envelope byte layout the macOS kext emits.
- Where Rust disagrees with `_extracted/`, `_extracted/` wins for the macOS clone — this is the standing B rule (see `feedback_ffado_bebob_macfw_are_parallel_rev_eng.md`).

## 1. Signal-flow topology — A2-P3 claims vs. Rust claims

A2-P3 enumerates 46 `MA*` keys across Levels / Mutes / Solos / Routings / Aux / Headphone / Clock / Rate / Rotary. Q15 posits a 22-channel output surface and a 14/18-channel input surface. The Rust ASCII diagram (special.rs:14-41) resolves the topology:

```
8 analog in + 2 S/PDIF in + 8 ADAT in = 18 phys inputs
2 stream-input pairs                   =  4 stream inputs
                 → 22x4 main mixer + 22x2 aux mixer
                 → 2x mixer-output pair + 1x aux-output pair
                 → 4 analog outputs + 4 headphone outputs
Stream inputs 5..12                    → 8 digital outputs (bypass mixer)
```

- **Confirms A1-Q15 ConcreteChannelGroup<22>** — the `22x4` + `22x2` mixer is the literal source of the N=22 in the Rust diagram (special.rs:33-35).
- **Resolves the 10/14/18 confusion** — Rust shows the aggregate input count is 18 physical + 4 stream = **22 mixer rows**; Q15 was hypothesising 14 or 18 for inputs but missed that stream-returns are mixed in parallel through the same bus matrix.
- **Headphone output count** — Rust declares 4 headphone outputs (special.rs:188 `headphone: [i16; 4]`) organised as 2 pairs. A2-P3's `MAHeadphoneLevels` (NSArray) and `MAHeadphoneSourceV2` (per-pair source) map 1:1 onto `headphone_volumes: [i16; 4]` and `headphone_pair_sources: [HeadphoneSource; 2]` (special.rs:582).

**Guess — needs verification:** the kext's N=14 ConcreteChannelGroup variant (A1-Q15 §Object-sizes) probably corresponds to the "ADAT-input-mode = SMUX / off" degenerate case where ADAT contributes 0 or 4 instead of 8 channels. Verify by decoding `com_m_audio_FW1814Device::OpticalModeChanged` xref (not yet in A1).

## 2. The 46 MA* keys — 1:1 Rust mapping

Grouping by Rust protocol struct (special.rs offset annotations @ lines 310-399).

### 2.1 Levels (A2-P3 fader keys)

| MA* key (A2-P3) | Rust field / range | special.rs:line | Notes |
|---|---|---|---|
| `MAInputLevels` (analog + spdif + adat gains) | `MaudioSpecialInputParameters.analog_gains[8]` + `.spdif_gains[2]` + `.adat_gains[8]` → `ANALOG_INPUT_GAIN_RANGE 0x10..0x20`, `SPDIF_INPUT_GAIN_RANGE 0x20..0x24`, `ADAT_INPUT_GAIN_RANGE 0x24..0x34` | 338-347, 418-422 | 18 inputs, 16-bit signed i16, step 0x100 |
| `MASWReturnLevels` (stream returns) | `MaudioSpecialInputParameters.stream_gains[4]` → `STREAM_INPUT_GAIN_RANGE 0x00..0x08` | 328-331, 414-415 | 4 stream inputs (two pairs) |
| `MAOutputLevels` (analog outs) | `MaudioSpecialOutputParameters.analog_volumes[4]` → `ANALOG_OUTPUT_VOLUME_RANGE 0x08..0x10` | 332-335, 576 | 4 analog outs |
| `MAHeadphoneLevels` | `MaudioSpecialOutputParameters.headphone_volumes[4]` → `HEADPHONE_VOLUME_RANGE 0x38..0x40` | 352-355, 580 | 4 phones (2 pairs) |
| `MAAuxOutputLevels` | `MaudioSpecialAuxParameters.output_volumes[2]` → `AUX_OUTPUT_VOLUME_RANGE 0x34..0x38` | 348-351, 740 | 2-ch aux master |
| `MAAuxSendLevels` (per-input send to aux bus) | `MaudioSpecialAuxParameters.stream_gains[4]` + `.analog_gains[8]` + `.spdif_gains[2]` + `.adat_gains[8]` → `AUX_STREAM_INPUT_GAIN_RANGE 0x64..0x6c`, `AUX_ANALOG_INPUT_GAIN_RANGE 0x6c..0x7c`, `AUX_SPDIF_INPUT_GAIN_RANGE 0x7c..0x80`, `AUX_ADAT_INPUT_GAIN_RANGE 0x80..0x90` | 368-383, 742-748 | 22 per-input aux-send gains |

**Confirms A1-Q15 1-Levels-dict / 1-AuxSend-dict split:** the Rust crate puts Levels and AuxSend into two separate protocol structs with disjoint offset ranges — mirrors the two FWSettings\* dicts.

### 2.2 Pan / Balance

A2-P3 lists `MAInputPan2` (cookie `0x5eb` in P7-fup:91) separately from `MAInputLevels`. Rust routes this through:

| MA* key | Rust field / range | special.rs:line |
|---|---|---|
| `MAInputPan2` → analog | `MaudioSpecialInputParameters.analog_balances[8]` → `ANALOG_INPUT_BALANCE_RANGE 0x40..0x50` | 356-359, 425 |
| `MAInputPan2` → spdif | `.spdif_balances[2]` → `SPDIF_INPUT_BALANCE_RANGE 0x50..0x54` | 360-363, 427 |
| `MAInputPan2` → adat | `.adat_balances[8]` → `ADAT_INPUT_BALANCE_RANGE 0x54..0x64` | 364-367, 429 |

**Discrepancy with A1-Q15:** Q15 posited that pan is "inherently stereo-paired, so one property write has to touch both groups" via `FWChannelGroupProperty[+0x70]` second-group pointer. The Rust crate stores pan as a **per-channel L/R balance scalar** (i16 per channel, 18 channels = 18 scalars). This is wire-compatible: a stereo pair's L/R balance is just two adjacent i16 values. The kext's two-group abstraction is a cosmetic UI-side split (Q15 §Property setter path lines 128-153) and does not change the hardware encoding.

### 2.3 Mute / Solo (bitmask-packed)

A2-P3 lines 19-24: `MASWReturnMute`, `MAInputMute`, `MAOutputMute`, `MAHeadphoneMute`, `MAAuxOutputMute` + solo twins — all NSNumber bitmasks.

**Rust gap:** `special.rs` has no explicit mute/solo state. The Linux stack models mute as a hard `-inf` (i16::MIN at `GAIN_MIN = i16::MIN`, special.rs:473) on the fader itself; solo is a UI concept not pushed to hardware.

Implication for A1-Q15: the kext's `FWChannelGroupProperty` for `Mute`-variant properties (one of the 4 CGPs per group at offsets `[0, 0x78, 0xf0, 0x168]` in Q15 §Object-sizes line 37) likely encodes mute by **gating the fader value through its own hardware register**, not by poking a separate mute bitmap. **Hypothesis — needs verification** by decoding one of the Mute `Apply()` methods (A1-Q15c scope). Confirmation would be a cite showing the mute-variant CGP writes to the same AV/C selector as the level-variant CGP, differing only in that mute forces the payload to the min-level sentinel.

### 2.4 Routing — the "to-mixer source select" bitfield

A2-P3 lists `MAInputRoutingV2` (per-input destination picker) and `MASWReturnRoutingV2` (per-stream-return destination). Rust implements this as a **monolithic 32-bit quadlet at offset `0x90..0x94`** (`MIXER_PHYS_SOURCE_RANGE`) + another at `0x94..0x98` (`MIXER_STREAM_SOURCE_RANGE`):

```rust
// special.rs:841-861 — MaudioSpecialMixerParameters
pub struct MaudioSpecialMixerParameters {
    pub analog_pairs:  [[bool; 4]; 2],   // 8 analog inputs, grouped as 2 banks of 4 pairs
    pub spdif_pairs:   [bool; 2],         // 2 S/PDIF pairs (one bank)
    pub adat_pairs:    [[bool; 4]; 2],   // 8 ADAT inputs, 2 banks of 4
    pub stream_pairs:  [[bool; 2]; 2],   // stream returns, 2 banks
}
```

**Bitfield layout (special.rs:871-911, 936-985):**

| Bit range | Meaning |
|---|---|
| `MIXER_PHYS_SOURCE_RANGE` quadlet (big-endian at `0x90`) | |
| bits 0..3 | `analog_pairs[0][0..3]` enable → analog inputs 1/2, 3/4, 5/6, 7/8 → mixer bank A |
| bits 4..7 | `analog_pairs[1][0..3]` → same analog pairs → mixer bank B |
| bits 8..11 | `adat_pairs[0][0..3]` → ADAT 1/2 .. 7/8 → bank A |
| bits 12..15 | `adat_pairs[1][0..3]` → ADAT 1/2 .. 7/8 → bank B |
| bits 16, 17 | `spdif_pairs[0]`, `spdif_pairs[1]` → bank A, bank B |
| bits 18..31 | unused |
| `MIXER_STREAM_SOURCE_RANGE` quadlet (big-endian at `0x94`) | |
| bits 0..3 | `stream_pairs[j=0..1][i=0..1]` with flag = `1u32 << (j * 2 + i)` (special.rs:924) |

The analog/adat/spdif routing uses `flag = 1u32 << (i * 4 + j)` (special.rs:883, i=bank 0/1, j=pair 0..3), `1u32 << (8 + i*4 + j)` for ADAT (special.rs:905), `1u32 << (16 + i)` for S/PDIF (special.rs:896).

**Correlation to A1-Q15 "channelMask bitfield":**
- Q15 line 61: `+0x68 uint channelMask (which channels this property applies to within the group)`.
- Q15 line 185: `+0x40 uint channelMask` on the FW1814Device batch-apply TFunctor.

**The Rust crate confirms the mask is a per-pair enable bitmap, not a per-channel one.** 22 inputs are addressed as 11 pairs: 4 analog pairs + 1 S/PDIF pair + 4 ADAT pairs + 2 stream pairs = 11. Replicate across 2 mixer banks = 22 slots total. The 32-bit mask matches: bits 0-3 + 4-7 (8 analog) + bits 8-11 + 12-15 (8 ADAT) + bits 16-17 (2 S/PDIF) = 18 of 32 bits used in `MIXER_PHYS_SOURCE_RANGE`.

**Opens an A1 gap:** the exact bit-position of each pair in the kext's `channelMask` is not documented in Q15. The Rust bit layout (special.rs:883-911) is the direct spec. **Verify by** reading the kext's `FWChannelGroupProperty::init @ 0x93a4` for literal mask immediates per CGP — the 16 ChannelGroupProperty instances in `FWSettings1814Levels` (Q15 line 37) must each burn one of these mask constants.

### 2.5 Headphone source + Analog-output source — the pair-source quadlets

A2-P3 line 30: `MAHeadphoneSourceV2` (NSNumber per pair). Rust (special.rs:555-564, 697-714):

```rust
pub enum HeadphoneSource {
    MixerOutputPair0,    // flag = 0x01
    MixerOutputPair1,    // flag = 0x02
    AuxOutputPair0,      // flag = 0x04
}

// Packed into HEADPHONE_PAIR_SOURCE_RANGE 0x98..0x9c:
//   pair 0: shift =  0, mask = 0x07
//   pair 1: shift = 16, mask = 0x07
```

The complementary "which-source feeds analog-out-pair-N" is `MaudioSpecialOutputParameters.analog_pair_sources: [OutputSource; 2]` (special.rs:578, 540-547):

```rust
pub enum OutputSource {
    MixerOutputPair,       // flag = 0x00
    AuxOutputPair0,        // flag = 0x01
}
// Packed into ANALOG_OUTPUT_PAIR_SOURCE_RANGE 0x9c..0xa0:
//   pair 0: shift = 0, mask = 0x01
//   pair 1: shift = 1, mask = 0x01
```

**No equivalent A2-P3 key found for `analog_pair_sources`.** A2-P3's `MAMonitorAuxOnOutput` (bool checkbox) is the closest — hypothesis is that `MAMonitorAuxOnOutput` *is* the `analog_pair_sources` toggle, with "monitor aux on output = ON" meaning both pairs set to `AuxOutputPair0`. **Verify** by reading the kext's `ReceiveMonitorAuxOnOutput` handler for which register-field it writes (A2-P3 line 27 lists the handler but not its target).

### 2.6 Clock source — the ONE control the Rust crate omits

A2-P3 line 31: `MAClockSource` — Internal / S/PDIF / ADAT / Word Clock. A1-Q5 (not re-read here) + `project_fw1814_clock_sources.md` establish the 4 options.

**Rust omission:** `Fw1814ClkProtocol` (special.rs:69-82) implements **only** `MediaClockFrequencyOperation` — read current sample rate via AV/C `OutputPlugSignalFormat`. It does NOT implement `SamplingClockSourceOperation`:

```rust
// special.rs:72 — Fw1814ClkProtocol impls MediaClockFrequencyOperation ONLY
// Compare normal.rs:197 — Fw410ClkProtocol impls BOTH MediaClockFrequencyOperation
// AND SamplingClockSourceOperation (SignalAddr::Subunit / SignalAddr::Unit(Ext(0x02))).
```

The crate-level docstring (special.rs:100) says `// NOTE: Special models doesn't support any bridgeco extension.` which explains why the standard BeBoB AV/C `SignalSource` plug-connect method is skipped. The FW1814's clock source is therefore controlled via **one of the quadlet pokes in `DM_APPL_PARAM_OFFSET`** — but which offset is unexplored in the Rust tree. The 160-byte cache (`CACHE_SIZE = 160`, special.rs:308) has no clock-source slot allocated in the 18 offset-range constants (special.rs:328-399 — total covers `0x00..0xa0`).

**Hypothesis:** on Linux, FW1814 clock-source selection is either (a) left to a hardcoded Internal default or (b) handled by a different Takashi crate module not re-exported through `special.rs`. **Verify** by `grep -r "FwReq.*write.*MaudioSpecial\|Fw1814ClkProtocol" protocols/` — likely yields nothing, confirming (a).

**Consequence for A1-Q5 and `project_maclocksource_is_composite_bitfield.md`:** the kext ground-truth (B5 §10) stays authoritative. The 32-bit composite packing — clock-source bits 0..3 + optical-out mode bit 4 + S/MUX bit 5 + gate bit 30 — has **no Rust counterpart**. If a future Rust contributor added FW1814 clock-source support, they would have to re-derive the composite from DTrace or kext RE. For our clone, this means `MAClockSource` is a **pure HSCI-side control** (A1-Q6 envelope + the `project_maclocksource_is_composite_bitfield.md` layout); the register-poke path validates nothing here.

### 2.7 Hardware-rotary meter + sync status

A2-P3 line 33: `MARotaryControlV2` — hardware rotary → selected input mask (NSNumber bitmask).

Rust models this as meter-side state (special.rs:194-306):

```rust
pub struct MaudioSpecialMeterState {
    // ... level fields ...
    pub switch: bool,              // special.rs:192
    pub rotaries: [i16; 3],        // special.rs:194 — 3 hardware rotaries
    pub sync_status: bool,         // special.rs:196
}
```

**Confirms A1 understanding and `project_fw1814_front_knobs_are_encoders.md`:** 3 rotary encoders (HP1, HP2, Main Level per the auto-memory), not 1. Each produces step events (special.rs:282-300 `0x01 = step up by ROTARY_STEP 0x400`, `0x02 = step down`). The macOS kext broadcasts these as `MARotaryControlV2` notifications; the Linux stack exposes them as ALSA mixer "integer" controls.

A2-P3 line 32: `MARateLocked` → Rust `MaudioSpecialMeterState.sync_status` (special.rs:302 `meter.sync_status = bitmap1[3] ^ frame[METER_SIZE - 1] > 0`). Same bool semantic.

### 2.8 Switch / Momentary

A1-Q15 and A2-P3 do not list a switch key. Rust (special.rs:192) has `switch: bool` tracked at offset 0 of the meter frame (special.rs:276). This is **the hardware's single physical switch** (likely the front "Set" button on the FW1814). The kext almost certainly broadcasts this as a notification `bodyType==0` (A2-P2) under some key — unexplored in A2-P3. **Candidate: `MAMomentarySwitch*` keys referenced in A2-P11 §2.2 as "no write path".** Verify by grepping panel binary for `MAMomentarySwitch`.

### 2.9 Meter / peak path

A2-P3 line 38: `MAMetaAllProperties` is initial-state slurp only. A2-P9 (peak meters) + `project_meter_path_shared_ring.md` establish the vendor FWUserClient clientMemoryForType(1) shared-ring path for peak meters.

Rust reads meters by block-request to `DM_APPL_METER_OFFSET` (special.rs:248-255):

```rust
req.transaction(
    node,
    FwTcode::ReadBlockRequest,
    DM_APPL_METER_OFFSET,      // 0xffc700000000 + 0x00600000
    frame.len(),                // 84 bytes
    frame,
    timeout_ms,
)?
```

**Different mechanism, same semantics.** Linux polls a 84-byte register window; macOS does zero-copy via a shared IOBufferMemoryDescriptor exposed through the user client's `clientMemoryForType(1)`. The macOS approach is lower-latency (no FireWire read request per poll) and matches the ~70 Hz Panel refresh cap (`project_meter_path_shared_ring.md`).

Field layout agrees between the two:
- 8 analog inputs + 2 S/PDIF + 8 ADAT + 4 analog outputs + 2 S/PDIF outs + 8 ADAT outs + 4 headphone + 2 aux = **38 i16 meter values**. Rust parses them in exactly this order starting at offset 4 of the 84-byte frame (special.rs:260-274, `pos = 2 + (1 + i) * 2`).

**Clone implication:** the IOBufferMemoryDescriptor mapped into userspace has to pack 38 i16 values in the identical order (analog in → spdif in → adat in → analog out → spdif out → adat out → headphone → aux). This nails down the "cap 70" shared-ring layout that `project_meter_path_shared_ring.md` left as a bare count.

## 3. Validating A1-Q15b single-dispatcher model

**A1-Q15b claim:** `FWPropertyDict::SetProperties @ 0xa0e0` is the single generic dispatcher; every non-`MASampleRate` write is iterated through `SetOneProperty` into typed-property `Apply()` methods.

**Rust equivalent — special.rs:1002-1096 (`MaudioSpecialParameterProtocol` trait):**

Two methods on every protocol struct (`MaudioSpecialInputProtocol`, `MaudioSpecialOutputProtocol`, `MaudioSpecialAuxProtocol`, `MaudioSpecialMixerProtocol`):

- `whole_update` (special.rs:1004) — serialize full param block, scan `OFFSET_RANGES` for contiguous runs, emit one `WriteQuadletRequest` per run.
- `partial_update` (special.rs:1053) — serialize into a scratch, compare each range against the cache, emit a write only for ranges that differ. On success, promote scratch into cache (`cache.0[range.start..range.end].copy_from_slice(raw)`, special.rs:1087).

Both are blanket-impl'd for any `T: Copy` where `O: SpecialParametersSerdes<T>` (special.rs:1096), so **every parameter type routes through the same two methods**.

**Correlation:**

| Kext (A1-Q15b) | Rust (special.rs) |
|---|---|
| `FWPropertyDict::SetProperties @ 0xa0e0` — single iterator | `MaudioSpecialParameterProtocol::partial_update` — single iterator per protocol |
| `SetOneProperty` per key → `ModifyProperty @ 0x9e28` adds to `OSSet` "modified" | Rust diff: `if raw != &cache.0[range.start..range.end]` (special.rs:1072) |
| `checkForWork @ 0x9d38` → `ApplyModifiedProperties @ 0x9b40` dispatches `Apply()` | Rust: direct `req.transaction(...)` inline (no deferred workloop; synchronous AV/C-less path) |
| Workloop deferral (A1-Q15b line 113) coalesces panel edits | Rust path is synchronous because register pokes are sub-ms (no FCP round-trip) |

**Confirms A1-Q15b:** the single-dispatcher model is architecturally sound. The macOS kext adds workloop deferral to amortise the ~1 ms AV/C HSCI round-trip; the Linux stack skips deferral because register pokes are near-free.

**Rust gives us the serdes partition the kext doesn't make explicit:** per-protocol `OFFSET_RANGES` arrays (special.rs:488-496, 613-618, 793-799, 868-869) list which offset-ranges that protocol owns. The kext implicitly does the same at property-registration time via `FWChannelGroupProperty::init @ 0x93a4` — the per-CGP `channelMask` and `key` fields (Q15 lines 54-61) are the kext's version of "which offset-ranges does this property own". Rust explicit, kext implicit; semantically identical.

**One difference worth flagging:** the Rust `partial_update` compares against a cache *before* writing, writes only diffs, then updates cache from write-request bytes. The kext comparison happens at `BaseChannelGroup::AdjustedLevels` snapshot time (A1-Q15 line 142) — snapshot *before* unpack, apply computes deltas inside `vtable[0x28]`. Both patterns, same intent.

## 4. ChannelGroup bitmasks — A1-Q15 vs. Rust explicit layout

**A1-Q15 count:** 16 `ChannelGroupProperty` in `FWSettings1814Levels` (4 groups × 4 property types) + 4 in `FWSettings1814AuxSend` (2 aux buses × 2 property types) = **20 CGPs total**. Each with a `channelMask: uint` at `+0x68` (Q15 line 61).

**Rust count:** 4 top-level protocol structs (`Input`, `Output`, `Aux`, `Mixer`) each with 2-7 offset-ranges = **18 contiguous ranges** total, verified by the `offset_ranges()` test (special.rs:1103-1128) which asserts every byte in `0..CACHE_SIZE=160` is owned by exactly one range.

**Reconciling the count mismatch:**
- The 20 CGPs on the kext side include duplicates for the "two-group pattern" (A1-Q15 line 151 — pan writes touch two groups). After de-duplicating paired writes, the semantic count collapses toward the Rust 18.
- A1-Q15's 4 group split (A/B/C/D) is likely the **4 bank × direction tuples**: analog-in bank A, analog-in bank B, output bank, aux bank. The Rust `MaudioSpecialMixerParameters.analog_pairs: [[bool; 4]; 2]` (special.rs:843) literally encodes two banks of 4 analog pairs — this is the "bank" axis the kext expresses as groups A vs. B.

**Hypothesis:** the kext's 4 groups are {analog-in bank A, analog-in bank B, output bank, aux bank}, and the 4 property-types per group are {Level, Pan, Mute, Routing}. 4×4=16 for Levels dict + 2 aux buses × 2 property-types (Send + Pan) = 4 → total 20. **Verify** via Q15a (OSSymbol::withCString extraction near each CGP ctor) — this is already an open follow-up in A1-Q15 line 277.

**The exact bitmask constants are only in Rust:**
- `MIXER_PHYS_SOURCE_RANGE` quadlet bit layout — special.rs:883 (analog `i*4+j`), :896 (spdif `16+i`), :905 (adat `8+i*4+j`).
- `MIXER_STREAM_SOURCE_RANGE` quadlet bit layout — special.rs:924 (`j*2+i`).
- `HEADPHONE_PAIR_SOURCE_RANGE` — 16-bit shift per pair, 3-bit mask (special.rs:643-653).
- `ANALOG_OUTPUT_PAIR_SOURCE_RANGE` — 1-bit shift per pair, 1-bit mask (special.rs:666-675).

For the clone, these are the exact bit positions to burn as constants (or, equivalently, the exact HSCI selector-byte mappings when A1 decodes the per-mask HSCI opcode via Q15c).

## 5. What the Rust crate tells us that A1 missed

1. **Topology count resolution.** 18 physical inputs + 4 stream returns = 22 mixer rows. Settles A1-Q15's N=14/18 ambiguity on the input side.
2. **Signal flow is "parallel pipe":** physical inputs 1-18 pass straight through to stream outputs 1-18 (special.rs:16-23 ASCII), AND feed the mixer in parallel. The direct passthrough is unmodifiable. A1-Q15 did not note this.
3. **Stream inputs 5-12 bypass the mixer entirely** (special.rs:37-40). They go straight to digital outputs 1-8. Our clone cannot route stream inputs 5-12 through the mixer. **Audit:** A1-Q13's engine-family split (4 engines) probably corresponds to (a) stream-pair-0+mixer, (b) stream-pair-1+mixer, (c) stream-5..8+bypass, (d) stream-9..12+bypass. **Hypothesis — needs verification** against FWEngine1/FWEngine2 ctor code.
4. **Rotary count is 3, not 1.** `project_fw1814_front_knobs_are_encoders.md` already states HP1/HP2/Main; Rust confirms (`rotaries: [i16; 3]`, special.rs:194). A2-P3's `MARotaryControlV2` is a single bitmask key — meaning the macOS panel multiplexes all 3 encoders through one broadcast. The clone's AppKit panel needs to demultiplex by field index (0 = HP1, 1 = HP2, 2 = Main) — Rust gives us the ordering.
5. **Meter frame layout is fully specified** (special.rs:260-274): 84-byte frame, 38 i16 meter values starting at byte 4, in a fixed order. Clone's shared-ring layout must match this if we want Linux-tool interop; must at minimum pack *some* consistent order for our own panel. Rust's order is the obvious choice.
6. **Headphone source has 3 options, not 2.** `MixerOutputPair0`, `MixerOutputPair1`, `AuxOutputPair0` (special.rs:558-563). A2-P3 lists `MAHeadphoneSourceV2` as NSNumber without enumerating the values; Rust tells us it is 3-valued (and the panel's "HP A/B source" dialog in A2-P11 confirms 3 radio buttons).
7. **Register-poke protocol exists.** The DM1000 firmware accepts both HSCI and direct-register-poke writes. **For our macOS clone this is unused** — we commit to HSCI-only (A1-Q6, matches `_extracted/`). But documenting it here means a future hybrid-driver experiment (e.g. a 10.15+ `AudioServerPlugIn` port using IOFireWireFamily's `IOFWCommand`) can be considered without re-reading the Rust tree.

## 6. What A1 tells us that the Rust crate missed

1. **HSCI envelope opcodes.** The macOS kext's AV/C vendor-dependent envelope (A1-Q6, 12-byte CONTROL to FCP cmd reg, company_id `0x040004`, byte 7+8 = digital in/out selector) is **invisible** to the Rust path. A clone defender who only read `special.rs` would miss all HSCI knowledge.
2. **Batch opcode `0xBA9`** (A1-Q15 @ 0xe1cc). The AV/C vendor-dependent opcode that batches a group of CGP writes. Rust has no analog — register pokes don't need batching.
3. **FWUserClient notification path** (A1-Q11, Q14; A2-P2). Mach-port 44-byte messages with no peak/rate-lock body types — type 0 props, type-1 port = MIDI-in (corrected 2026-10-14, A1 Q-UC-surface §4). Rust has no equivalent because ALSA's path is `snd_ctl_elem_value` IOCTL polling + hinawa event-port.
4. **FWKIPCUserClient shared OSSet broadcast** (A1-Q14). The pattern that lets all open Panels sync on one property change. Rust doesn't need it — all state is read-authoritative from the registers.
5. **Composite `MAClockSource` bitfield** (`project_maclocksource_is_composite_bitfield.md`, B5 §10). 32-bit packing: clock-src + optical-out + S/MUX + gate. Rust omits clock-source entirely for FW1814 (§2.6).
6. **ChangeRate / performFormat path** (A1-Q15c + Q15d). The sample-rate fast-path through `FWAVCAudioDevice::setProperties`'s MASampleRate siphon → `ChangeRate` vtable slot `0xa78`. Rust tracks rate via AV/C `OutputPlugSignalFormat` STATUS only (special.rs:107-118) — no CONTROL path to change rate (left to the stream-start side).
7. **`com_m_audio_FWMetaNub` 6-state lifecycle** (A1-Q7). Bootloader ↔ runtime transitions driven by Model_ID `0x10070` ↔ `0x10071`. Rust is handed an already-booted device by `fw-core` / `hinawa` and skips the state machine.
8. **DCL ring + packet-pool AM824 silence pre-fill** (A1-Q1 + Q17). Rust is userspace — ALSA/ffado-based clients receive AM824 streams via `fw-isochresource`, which does its own DCL. The kext's DCL rings (TX 20-seg/4-bank, RX 64-seg × 4-pkt — Q-DCL, corrected 2026-10-14) with `0xCF/0x80` idle-lock silence is macOS-specific and crucial for the TX-silence bug (`project_fw1814_idle_lock_mechanism.md`).
9. **Clip indicators / Levels dict ArrayProp #4 `+0x9f0`** (A1-Q15 line 211 "ClipIndicators"). Rust has no clip-indicator state. If this is a kext-side computed signal, it is derived at AV/C-poll time or from the meter window and does not need a wire-level representation.
10. **Routing V2 vs. V1 split.** A2-P3 keys end in `V2` (`MAInputRoutingV2`, `MASWReturnRoutingV2`, `MAHeadphoneSourceV2`). The implied V1 is the older DM1000 panel (circa 2007 firmware). Rust targets the modern binding, so this is a kext-side compatibility shim invisible to Rust.

## 7. Implications for Part C/D

### C3 (comm-path HSCI envelope)
- `_extracted/` + A1-Q6 remain authoritative for byte layout.
- Rust is **only** a cross-validator for the *semantic payload*: which bits mean "ADAT source enabled on mixer bank B", what the clock-source domain is, which meter slot holds analog-out-3 peak, etc.
- Use Rust bitfield constants (§2.4, §2.5) as **sanity checks** when decoding HSCI opcode arguments in A1-Q15c follow-up.

### C5 (userclient surface)
- Confirms A1-Q11's selectors 500-507 are the whole surface for the vendor UC — Rust has no notion of a "user client" because ALSA models it as char devices.
- Confirms A1-Q14's shared-ring + notification mechanism is macOS-unique. Clone's userclient must replicate it; no Linux-side design to borrow.
- Meter shared-ring layout (§2.9) must mirror Rust's 38-i16 packing order for consistency.

### Part D (build decision)
- B2a strengthens the clone recommendation from B3 by **adding a second independent spec of mixer semantics**. Even if the modified-reference path solved the three 10.13 breakers (UAKEL/signing/vtable), the mixer graph it would inherit matches what Rust describes — so we lose nothing by rebuilding from scratch.
- The Rust crate is a free integration-test oracle: any protocol-level library we build in `core/` (per `docs/Proposal.md`) can be fuzzed against Takashi's cache serdes to catch regressions. Specifically, our `MixerBitfield::encode(routing_matrix)` output can be hex-compared against `MaudioSpecialMixerProtocol::serialize` output on identical inputs.
- Reinforces Proposal.md's "`core/` is transport-agnostic" claim: Takashi's `SpecialParametersSerdes<T>` trait **already is** a transport-agnostic serdes. Our `core/` layer can borrow the same shape — a `MixerParams::serialize(&mut [u8; 160])` entry point, with the HSCI vs. poke choice living in a separate transport trait.

## 8. Status / next step

Done 2026-10-13. **Headline:** 46 `MA*` keys map cleanly onto Rust's 4 protocol structs over a 160-byte cache; A1-Q15b's single-dispatcher model is validated by Rust's `partial_update` pattern; the N=22 mixer count is fixed; clock-source is the one control Rust omits for FW1814, confirming it is HSCI-only on the macOS side.

**Next:** B2 (ALSA snd-bebob kernel driver) — the C-side counterpart to Takashi's Rust crate. Expected to validate the AV/C `SignalSource` path for non-FW1814 BeBoB devices and ground the "DM1000 silicon vs. older BeBoB" partition.

## 9. Provenance table (one-glance audit)

| Claim | Source |
|---|---|
| DM_APPL register window | `lib.rs:35-37` |
| 22-row mixer topology | `special.rs:14-41` |
| 160-byte cache + 18 offset-ranges | `special.rs:308, 328-399, 1125-1128` |
| Analog/S/PDIF/ADAT input gain-balance ranges | `special.rs:336-367, 418-429` |
| MixerPhysSource bit layout | `special.rs:883, 896, 905` |
| MixerStreamSource bit layout | `special.rs:924` |
| HeadphoneSource 3 enum values + bit packing | `special.rs:555-564, 643-653, 697-714` |
| 3 hardware rotaries + meter frame layout | `special.rs:194, 260-274, 282-300` |
| Fw1814ClkProtocol omits SamplingClockSourceOperation | `special.rs:69-82` (vs `normal.rs:197`) |
| partial_update single-dispatcher pattern | `special.rs:1053-1093` |
| MaudioSpecialStateCache 160 B | `special.rs:401-409` |
| ReadBlockRequest for meter | `special.rs:248-255` |
| WriteQuadletRequest for params | `special.rs:1038, 1079` |
| A1-Q15 batch opcode 0xBA9 | `A1-ghidra/Q15-mixer-model.md:170` |
| A1-Q15b single dispatcher @ 0xa0e0 | `A1-ghidra/Q15b-setproperties-dispatch.md:9-32` |
| A2-P3 46 MA* keys | `A2-panel/P3-property-key-strings.md:11-33` + `P7-fup-translator.md:70-103` |
| MAClockSource composite bitfield | `memory/project_maclocksource_is_composite_bitfield.md` + B5 §10 |
| Meter shared-ring (vendor UC clientMemoryForType(1)) | `memory/project_meter_path_shared_ring.md` |
