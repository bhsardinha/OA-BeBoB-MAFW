# Q3 — `SetupAudioEngine` + `ConnectAllPlugs` — channel counts, plug-connect order

> **Correction (2026-10-15, Q-MD2 §1 item 8):** lines 53/58 (`+0x300 = RearrangeSampleFrameLLRR6`, `ConvertLLRR4SkipMIDI1` "inherited by the FW1814") describe `FWAudiophileDevice::SetupAudioEngine`. The FW1814's `SetupAudioEngine` @0xed34 ends with `vt[0xaf8]` = `FW1814Device::ResetRearrangementProcs` @0xcad4, which installs `_RearrangeSampleFrameLRLR{4,6SkipMIDI…14SkipMIDI}` by output channel count and `_ConvertInputFramesOneStereoPairSkipMIDI` / `WorkaroundFor1814InputDelay` for input. The follow-up at line 144 (12-in/16-out procs) is answered by that table.


> **Correction (2026-10-14) — channel counts** (evidence: [PROVENANCE-RESOLUTIONS.md §R12](../PROVENANCE-RESOLUTIONS.md)). `SetupAudioEngine @0xed34`: bit 4 of `fMode` (optical-OUT ADAT) selects **output** channels `[+0x2e0]/[+0x2ec]` = 6 or 12; bit 3 (optical-IN ADAT) selects **input** channels `[+0x2e4]/[+0x2f0]` = 10 or 16. Older text ('6/12 in, 10/16 out'; 'in: bit 4, out: bit 3') is swapped. SL v1.9 `67a/67b` agree (48 kHz: out 12 / in 16; 96 kHz: out 8 / in 12).


**Status:** done.

**Scope:** offsets `0xbb9a` (`FWAudiophileDevice::SetupAudioEngine`), `0xed34` (`FW1814Device::SetupAudioEngine`), `0x12476` (`FWEngine2::ConnectAllPlugs`), `0x12476+N` (`ConnectAudioMIDIChannels`), `0x12622` (`FWEngine2::initHardware`).

## Answer in one line

**`SetupAudioEngine` fills engine-side channel counts and callback pointers (not plug connects); `ConnectAllPlugs` is a 3-step gate: `SufficientBandwidthAvailable` → `ConnectAudioMIDIChannels` → set `fConnected`.** `initHardware` is where the two `FWDCLProgram` instances (Output+Input) are allocated, `Initialize`'d with the shared `FWDCLSharedSyncData`, cross-linked (`input->fOutputProgram = output` at `+0x508`), both marked "talking" (`0x8e = 1`), and added to the engine's DCL program list.

## `FW1814Device::SetupAudioEngine(FWBaseEngine *engine)` @ 0xed34

```c
void FW1814Device::SetupAudioEngine(FWBaseEngine *e) {
    e->SetMIDIInputPorts (this, /*count*/ 1, /*names*/ NULL);
    e->SetMIDIOutputPorts(this, /*count*/ 1, /*names*/ NULL);
    e->[+0x2d5] = 1;                       // some bool flag — likely "has digital in"
    e->[+0x2d6] = 1;                       // "has digital out"
    e->[+0x348] = 1;                       // "supports AC3 passthrough"
    e->fNumMIDIChannels /* +0x2e8 */ = 1;

    // Dynamic in-channel count based on this->fMode[+0x144] bit 4 (ADAT-active)
    if (this->fMode & 0x10) {
        e->fInChannels      /* +0x2ec */ = 12;    // ADAT 8 + stereo analog pair + digital pair
        e->fInStreamChannels/* +0x2e0 */ = 12;
    } else {
        e->fInChannels      /* +0x2ec */ = 6;     // 4 analog + 2 digital
        e->fInStreamChannels/* +0x2e0 */ = 6;
    }
    // Dynamic out-channel count based on bit 3 (digital-mix / SMUX-active)
    if (this->fMode & 0x08) {
        e->fOutChannels      /* +0x2f0 */ = 16;
        e->fOutStreamChannels/* +0x2e4 */ = 16;
    } else {
        e->fOutChannels      /* +0x2f0 */ = 10;
        e->fOutStreamChannels/* +0x2e4 */ = 10;
    }

    // Tail-call parent (FWAudiophileDevice::SetupAudioEngine) for format-change callbacks
    this->vtable[0xaf8](this, e);
}
```

### `FWAudiophileDevice::SetupAudioEngine` @ 0xbb9a (parent)

Sets single-channel MIDI in/out, 6-in / 10-out defaults (no dynamic resize — the Audiophile only has one mode), and three function pointers on the engine:

| Offset | Function | Purpose |
|---|---|---|
| `+0x300` | `RearrangeSampleFrameLLRR6` | 6-ch frame rearrangement (TX pack) |
| `+0x308` | `SamplePeaks6` | 6-ch VU-meter sampler |
| `+0x310` | `ConvertLLRR4SkipMIDI1` | RX unpack: 4 LR pairs skipping 1 MIDI quadlet |
| `+0x318` | `this` (device pointer, callback context) | — |

**FW1814 inherits these.** Analog/digital unpacking is handled by the inherited `ConvertLLRR4SkipMIDI1`-family functions. For 12-in / 16-out modes the function pointers must change — look for `Convert*12*` / `Rearrange*16*` symbols in a follow-up (unresolved here).

### Channel-count conclusion

| Mode | fMode & 0x10 (ADAT) | fMode & 0x08 (digital-mix) | In ch | Out ch |
|---|---|---|---|---|
| Analog-only, 44.1/48k | 0 | 0 | 6  | 10 |
| ADAT in, no digital-mix | 1 | 0 | 12 | 10 |
| Analog, digital-mix out | 0 | 1 | 6  | 16 |
| Full digital (ADAT+SMUX) | 1 | 1 | 12 | 16 |

The 2014 FW1814 panel exposes exactly these four combinations.

## `FWEngine2::ConnectAllPlugs()` @ 0x12476

```c
IOReturn FWEngine2::ConnectAllPlugs() {
    if (this->fConnected /* +0x2d3 */) return kIOReturnBusy;    // 0xE00002BC
    if (!this->fDevice->SufficientBandwidthAvailable())          // vtable? direct call
        return kIOReturnNoResources;                             // 0xE00002BE
    IOReturn err = this->ConnectAudioMIDIChannels();
    this->fConnected = (err == 0);
    return err;
}
```

Three sequence points:
1. **Gate on already-connected** — early return `kIOReturnBusy` if the engine thinks it's already connected.
2. **Bandwidth probe** — `FWAudioDevice::SufficientBandwidthAvailable()` consults IRM state before attempting plug connect.
3. **Delegate** to `ConnectAudioMIDIChannels` for the real CMP/PCR work.

The `fConnected` flag at `+0x2d3` is also cleared by `DisconnectAllPlugs` elsewhere. Writing it to `1` only on success is important: a partially-connected device must not look "connected" to the engine.

## `FWEngine2::initHardware(IOService *provider)` @ 0x12622 — DCL program setup

```c
bool FWEngine2::initHardware(IOService *provider) {
    bool ok = FWBaseEngine::initHardware(provider);           // vtable[0x9f0]
    if (!ok) return false;

    // Allocate and initialize the Output DCL program (0x620 bytes)
    this->fOutputDCLProgram /* +0x350 */ = new m_audio_b_FWDCLOutputProgram();
    if (!this->fOutputDCLProgram) return false;
    FWDCLSharedSyncData *shared = &this->fSharedSync /* +0x240 */;
    if (this->fOutputDCLProgram->Initialize(this->fDevice /* +0x138 */, shared) != 0) {
        this->fOutputDCLProgram->release();
        return false;
    }

    // Allocate and initialize the Input DCL program (0x518 bytes)
    this->fInputDCLProgram /* +0x358 */ = new m_audio_b_FWDCLInputProgram();
    if (!this->fInputDCLProgram) {  this->fOutputDCLProgram->release(); return false; }
    if (this->fInputDCLProgram->Initialize(this->fDevice, shared) != 0) {
        this->fInputDCLProgram->release(); this->fOutputDCLProgram->release(); return false;
    }

    // Cross-link: input program's "paired output" pointer
    this->fInputDCLProgram->[+0x508] = this->fOutputDCLProgram;
    // Mark both programs active
    this->fInputDCLProgram->[+0x8e]  = 1;
    this->fOutputDCLProgram->[+0x8e] = 1;

    // Register both with the engine's DCL program list
    this->AddDCLProgram(this->fOutputDCLProgram);
    this->AddDCLProgram(this->fInputDCLProgram);
    return true;
}
```

Key cross-link: **the input program owns a pointer to the output program at `+0x508`.** This is how the input callback can read TX-side state (dbc counter, SYT history) when packing matching RX frames. Clone must mirror this — the Q1 ring-sync depends on it.

## Delta summary (clone vs reference)

| Aspect | Reference | Clone (expected) | Impact |
|---|---|---|---|
| Dynamic channel count | 6/12 in, 10/16 out based on `fMode` bits | Static 6-in / 10-out likely | **Important** — ADAT/SMUX modes will not work until clone wires `fMode` reading |
| MIDI port count | 1 in / 1 out | TBD | Needed for parity |
| Format-change callbacks (`+0x300/0x308/0x310/0x318`) | pointer trio installed | TBD | Required or TX/RX packer will crash |
| Plug-connect gate | `fConnected` + `SufficientBandwidthAvailable` + delegate | TBD | Simple to mirror |
| `fOutputDCLProgram` ↔ `fInputDCLProgram` cross-link at `+0x508` | yes | **must mirror** | Needed for Q1 ring sync |
| Both programs `[+0x8e] = 1` after init | yes | TBD | "program active" flag |
| Shared `FWDCLSharedSyncData` at engine `+0x240` | yes | TBD | Required object |

## Follow-up queue impact

- **Q3a (new):** decode `ConnectAudioMIDIChannels` — the actual CMP/PCR plug-connect sequence. Likely iterates `fDevice->oPlugs[]` and `fDevice->iPlugs[]`, calls `IOFireWirePCR::connectToListener` / `connectToTalker` per plug. Needed for the clone's stream-kick to not race.
- Follow-up: locate the 12-in / 16-out convert/rearrange function pointer sets used in ADAT/SMUX modes. Grep for `*12*` / `*16*` variants of `ConvertLLRR` and `RearrangeSampleFrame`.

## Provenance

- Offsets verified via `otool -tV` dump on 2026-10-06.
- `fMode` bit 4 vs 3 interpretation: inferred from channel-count split (ADAT = 8 extra inputs → +6 to in; digital-mix = 6 extra outs → +6 to out). Cross-check against FFADO maudio HSCI register map (B1).

## Channel-map vs manual §14 (added 2026-10-09)

Cross-reference against B5 §6b CoreAudio channel map + MA* key schema + `manual.pdf §14` defaults resolves what the "hardwired" terminology in the manual means at the IOKit layer. Written up in full in `../A2-panel/P-manual-crosscheck.md §Item #6`. Short form:

| Stream slot | Mixer-page processing | MA* control-plane entries | "Hardwired" per manual? |
|---|---|---|---|
| OutputChannel 1..4 (Analog 1..4) | yes — sum of DAW sw rtn + hardware input strips + aux return | `MAOutputLevels[0..3]`, `Link`, `Mute`, `Solo` | no |
| OutputChannel 5..12 (ADAT 1..8) | **no — direct DAW→DM1000→ADAT optical** | none | yes ("adat rtn bus hardwired to ADAT output bus") |
| S/PDIF out pair (when optical-out = S/PDIF) | **no — direct DAW→DM1000→coax+optical mirror** | none | yes ("spdif sw rtn hardwired to spdif out") |

The DM1000's internal DSP topology routes DAW→digital output channels straight to the digital output buses, bypassing the mixer. The kext publishes them as regular IOAudioStream channels (CoreAudio treats them identically to analog), but no `MA*RoutingV2` key names them because they aren't user-routable through the mixer page. This matches the manual's defaults table (`manual.pdf:303,304`).

## Correction (2026-10-09) — fInChannels/fOutChannels interpretation

The "6 vs 12 in / 10 vs 16 out" immediates read above do NOT match the hardware reality (per memory `project_fw1814_hardware_capabilities.md` + B5 §6b live observation):

| Digital In | Digital Out | True in | True out |
|------------|-------------|---------|----------|
| S/PDIF (2) | S/PDIF (2)  | 10      | 6        |
| S/PDIF (2) | ADAT (8)    | 10      | 12       |
| ADAT (8)   | S/PDIF (2)  | 16      | 6        |
| ADAT (8)   | ADAT (8)    | 16      | 12       |

The `6/12` and `10/16` immediates at the engine's `fInChannels`/`fOutChannels` offsets probably count **a subset** (e.g. just the "additional" channels beyond the engine's base 10/4, or the "slot indices used for packing") rather than the total device I/O channel count. B5 §6b observed 16 in / 12 out on SL at ADAT/ADAT mode — which matches "10 in base + 6 extra" and "4 out base + 8 extra" if the stored immediate is the extra-channel count. **Verification path:** re-decompile `FW1814Device::SetupAudioEngine @ 0xed34` with full struct annotations and inspect where `fInStreamChannels[+0x2e0]` is actually consumed — does it feed `IOAudioEngineChannelNames` population or only the AM824 packer? Not blocking Part A closure; item #6 answer stands.
