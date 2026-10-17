# Q12 — `ActivateAC3Output` + `ClipAC3Output` — IEC 61937 passthrough path

> **Correction (2026-10-15, Q-MI `Q-MI-ac3.md`):** the addresses and vtable slots in this file are stale and parts of the pseudocode are wrong. Authoritative: `StartAC3Output @0x1185e` (stops MIDI, then `ActivateAC3Output`), `ActivateAC3Output @0x11fee` = `SetAC3OutputState(true)` then `ActivatePCMOutput` (vt `0xd20`), `DeactivateAC3Output @0x11fc6` = `DeactivatePCMOutput` (vt `0xd38`) then `SetAC3OutputState(false)`; on the FW1814 `WantsAC3MixerBypass()` is 0, so `SetAC3OutputState` sends nothing (no AV/C vendor toggle); the S/PDIF pair is channels (4, 5) and the 16-bit words are packed as `40 hi lo 00`.

> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](../A2-panel/AUDIT-2026-10-14.md)). Line 147 / 157: `m_audio_b_FWAudioDevice::SupportsAC3` returns **1** (`@0x1c40`), and the FW1814 override (slot 0xaa8 → 0xc318) returns 1; only Ozonic and NRV10 return 0. AC3 is live on the FW1814.


> **Correction (2026-10-14) — Part A completion batch 1** (evidence: [PROVENANCE-RESOLUTIONS.md §R8](../PROVENANCE-RESOLUTIONS.md)). `[+0xa5]` is `IOAudioEngine::configurationChangeInProgress` (10.13.6 IOAudioFamily `beginConfigurationChange` writes it, `0xa3d8`), **not** an AC3 flag; AC3 is `[+0x2d4]`. Earlier 'fIsAC3 = 0xa5' text is wrong.


**Status:** done.

**Scope:** `FWEngine2::ActivateAC3Output` @ 0x11fee (docs/system-map/A1-ghidra/otool-exports/mafw.asm:20668); `DeactivateAC3Output` @ 0x11fc6; `SetAC3OutputState(bool)` @ 0x11f28; `ClipAC3Output` @ 0x11c44 (:20385); `StartAC3Output` @ 0x11b14 (:20073); `AddAC3Format` @ 0x3082 base engine; `EnableAC3Format` / `DisableAC3Format` @ 0x3898 / 0x5407.

## Answer in one line

**AC3 passthrough is a side-channel on the output stream: AV/C vendor-command toggles the device into "non-mixable S/PDIF out" mode, then CoreAudio's clip hook `ClipAC3Output` byte-reshuffles 16-bit stereo IEC 61937 frames and `memcpy`'s them directly into the DCL packet buffer, bypassing the normal 32-bit AM824 mixer path entirely.**

## `ActivateAC3Output()` @ 0x11fee

```c
IOReturn FWEngine2::ActivateAC3Output() {
    IOReturn r = this->SetAC3OutputState(true);
    if (r != 0) return r;
    return (*this->vtable[0xd20])(this);   // StartAC3Output()
}
```

## `DeactivateAC3Output()` @ 0x11fc6

```c
IOReturn FWEngine2::DeactivateAC3Output() {
    (*this->vtable[0xd38])(this);          // StopAC3Output()
    return this->SetAC3OutputState(false);
}
```

Both wrap `SetAC3OutputState` with the DCL-side start/stop sequence.

## `SetAC3OutputState(bool enable)` @ 0x11f28 — AV/C handshake

```c
IOReturn FWEngine2::SetAC3OutputState(bool enable) {
    FWAVCAudioDevice *avc = OSDynamicCast(
        m_audio_b_FWAVCAudioDevice, this->fAVCDevice /* [+0x138] */);
    if (!avc) return 0;
    if (!avc->vtable[0xab0]()) return 0;    // IsAC3Supported()

    // Build a 9-byte AV/C descriptor on the stack:
    uint8_t descr[9] = { 0x09, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0 };
    // Vendor-set: AV/C selector 1 → set vendor mode = enable
    avc->AVCVendorCommand(/*subunit*/ 0, /*attr*/ 1, /*value*/ enable ? 1 : 0);
    // Read-back / validate via vtable[0xb48] (likely GetVendorStatus)
    IOReturn err = avc->vtable[0xb48](avc, &descr);
    if (err) return err;
    // Then flip the hardware stream selector:
    //   enable=1 → AVCSelectorCommand(src=descr.subunit, select=0)
    //   enable=0 → AVCSelectorCommand(src=descr.subunit, select=1)
    avc->AVCSelectorCommand(descr[4] /* subunit */, !enable);
    return 0;
}
```

Three AV/C transactions in sequence: **vendor-set → readback → selector-set**. The selector command re-routes the digital-out stream formatter: `select=0` → IEC 61937 passthrough (AC3), `select=1` → normal PCM. The vendor command's semantics are M-Audio-specific — likely a mixer-bypass flag that disables the hardware's upsampling path.

## `ClipAC3Output(float *src, int *dst, uint firstFrame, uint numFrames, IOAudioStreamFormat *fmt)` @ 0x11c44

```c
IOReturn FWEngine2::ClipAC3Output(
    float *mix, int *dst, uint32_t firstFrame, uint32_t numFrames,
    const IOAudioStreamFormat *fmt)
{
    uint32_t sampleRate = this->vtable[0xc68]();   // GetCurrentSampleRate()

    // Validate buffer bounds: (first+count)*4 must fit in output buffer size [+0x2d8].
    if ((firstFrame + numFrames) * 4 > this->fOutBufferSize) return kIOReturnBusy;
    // AC3 format gate: must be non-mixable, 2 channels, ≤ 0x2000 frames total.
    if (fmt->fIsMixable        /* [+0x10] */    != 0 ) return kIOReturnBusy;
    if (fmt->fNumChannels      /* [+0x00] */    != 2 ) return kIOReturnBusy;
    if (firstFrame + numFrames > 0x2000)                return kIOReturnBusy;
    if (!mix || !dst)                                   return kIOReturnBusy;

    // Interleave + 16-bit pack (passthrough path: no float conversion).
    bool haveMIDI = (this->fNumMIDIChannels /* [+0x2e8] */ != 0);
    sampleRate += this->fNumMIDIChannels;         // adjust sample stride for MIDI quadlet
    this->LocalRearrangeSampleFrame16Bit2(
        /*src*/ (uint16_t*)mix,
        /*srcOff*/ (uint16_t*)(mix + numFrames * 2),
        /*dst*/ dst,
        /*nFrames*/ numFrames,
        /*unused*/ sampleRate,
        /*haveMIDI*/ haveMIDI);

    if (numFrames == 0) return 0;

    // Now copy from the host output buffer into the DCL packet buffer.
    uint32_t written = 0;
    while (written < numFrames) {
        uint32_t packetBase;
        int *packetBuf;
        this->fOutputDCLProgram /* [+0x350] */
            ->GetPacketFrameContainingSampleIndex(
                /*rate*/ this->fRate /* [+0x224] */,
                /*sampleIdx*/ written + firstFrame,
                /*outPacketBase*/ packetBase,
                /*outBuffer*/ &packetBuf,
                /*outQuadletsAvailable*/ NULL,
                /*rsvd*/ 0);
        uint32_t take = packetBase;
        if (written + take > numFrames) take = numFrames - written;
        memcpy(packetBuf,
               dst + written * sampleRate * 4,
               take * sampleRate * 4);
        written += take;
    }
    return 0;
}
```

**The write path bypasses the mixer entirely** — AC3 frames go: userland CoreAudio buffer → `LocalRearrangeSampleFrame16Bit2` (interleaves/packs) → `memcpy` into the DCL output ring. No level/pan/matrix stage. This is required for IEC 61937 — any mix-engine processing on an encoded AC3 stream destroys the data.

### Validation constants

| Gate | Value | Semantic |
|---|---|---|
| `(first+count)*4 ≤ [+0x2d8]` | buffer-size check | 4 bytes per IEC 61937 frame (16-bit stereo) |
| `fmt->fIsMixable == 0` | mandatory | AC3 must not be mixable (CoreAudio marks encoded streams this way) |
| `fmt->fNumChannels == 2` | mandatory | IEC 61937 is always 2-channel |
| `first+count ≤ 0x2000` | 8192-frame cap | DCL output buffer limit |

## `StartAC3Output()` @ 0x11b14 (skimmed)

Transitions the DCL output program to AC3-fill mode by calling `FWDCLOutputProgram::ReconfigureForAC3()` (likely vtable[0x1a0] or similar on the DCL program), re-sets the output stream's active format, enables AC3-mode packet rearrange, and starts the stream if not already running. Full decode deferred (120-ish lines) — not blocking because the clone's AC3 story is "don't ship AC3 passthrough until PCM path is stable."

## `AddAC3Format(IOAudioStream *stream)` @ 0x3082 (skimmed)

Called once at engine init to publish an IEC 61937 non-mixable format to CoreAudio alongside the normal AM824 PCM format. CoreAudio surfaces this as an additional option in Audio MIDI Setup; the OS selects it when a passthrough-capable player (DVD Player, VLC) requests encoded output.

## Delta summary

| Aspect | Reference | Clone (expected) | Impact |
|---|---|---|---|
| AV/C vendor-cmd + selector handshake for mode switch | yes | TBD | Required for AC3 to work at all |
| AC3 format = non-mixable, 2-channel, 16-bit | yes | TBD | CoreAudio-level flag |
| Separate clip hook `ClipAC3Output` writing directly to DCL | yes | TBD | **Critical — no mixer processing** |
| 8192-frame cap on single clip call | `0x2000` | TBD | Hardware-side DCL program bound |
| MIDI stride adjustment inside clip | yes | required if MIDI-enabled | Same quadlet-skip as PCM path |

## Clone implications

**AC3 is explicitly out-of-scope for near-term milestones** per CLAUDE.md ("feature parity first, then extensions"). The clone can:
- Ship a `SupportsAC3()` returning false initially (base-class default returns false — see `m_audio_b_FWAudioDevice::SupportsAC3` @ 0xaff6, which just does `xor %eax, %eax; ret`).
- Add `AC3Activated(bool)` as a no-op (what the base class does @ 0xb0b4).
- Skip `AddAC3Format` in the engine's init path.

This yields a PCM-only driver that CoreAudio won't try to send IEC 61937 to. Later, when the mixer is proven, AC3 can be added as a drop-in.

## Follow-up queue impact

- **Q12a (new):** decode `StartAC3Output` / `StopAC3Output` to confirm the DCL reconfigure sequence — only needed if the clone adds AC3.
- **Q12b (new):** identify AV/C vendor-command byte values. `AVCVendorCommand(0, 1, enable)` encodes a specific M-Audio vendor opcode; FFADO's maudio.cpp likely documents it.
- Follow-up: observe whether `FW1814Device::SupportsAC3` returns true (the FW1814 has S/PDIF out, so yes expected). Confirm via subclass override — probably at a higher offset than the base.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm`, 2026-10-06.
- AV/C descriptor opcode `0x09`: immediate at 0x11f67.
- Buffer-size field `[+0x2d8]`: at 0x11c88 (`cmpl %ecx, 0x2d8(%r15)`).
- Mixable + channel gates: `cmpb $0x0, 0x10(%rcx)` at 0x11ca9 and `cmpl $0x2, (%rcx)` at 0x11caf.
- 8192-frame cap `0x2000`: immediate at 0x11cb4.
- 16-bit stereo rearrange `LocalRearrangeSampleFrame16Bit2`: tail-call at 0x11cf9 — only used by AC3 path (PCM path uses the `LocalRearrangeSampleFrame` family at offset `+0x300` on engine, decoded in Q3).
- `GetPacketFrameContainingSampleIndex` on DCL output program: direct call at 0x11d2b, confirming the clip hook writes straight to the DCL ring, not to a CoreAudio-side intermediate.
