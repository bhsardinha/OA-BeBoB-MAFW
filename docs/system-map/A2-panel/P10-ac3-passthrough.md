# A2-P10 — AC3 passthrough UI toggle

> **Correction (2026-10-14) — static audit** (evidence: [AUDIT-2026-10-14.md](AUDIT-2026-10-14.md)). **§2-§3 are wrong:** `FW1814Device::SupportsAC3` (vtable slot 0xaa8 → 0xc318) **returns 1**; base `@0x1c40` also returns 1; only Ozonic (0x16960) and NRV10 (0x1a162) return 0. The FW1814 already supports AC3 in the reference; the clone mirrors it. The 'cheap win' recommendation is withdrawn.


**Scope:** Does the panel expose a user-facing AC3-passthrough toggle for FW1814? And if we wanted to enable AC3 on FW1814, what would it cost?

## 1. Panel-side — nothing exists

A full-tree grep of the panel binary for `AC3`, `Passthrough`, `Passthru`, `NonAudio`, `Dolby`, `IEC958`, `SupportsAC3`, `MAAC3`, `MASupportsAC3` returns **zero hits**. No class, no method, no NSString, no NIB resource ID in `_INDEX.md`, nothing in `functions.tsv` or `classes.md`.

So **the panel has no AC3 toggle for any device, not just FW1814.** It is not hidden, not conditional, not greyed-out — the panel is unaware of AC3 as a concept.

That matches the Carbon-era design: AC3 passthrough on CoreAudio is advertised as a non-mixable stream format on the IOAudioStream. HAL activates it automatically when a client (DVD player, media app) asks for the non-mixable Dolby-Digital-wrapped IEC 61937 format. No user toggle needed, no panel involvement.

Closes P10 as "no panel UI, as expected after A1-Q12 (SupportsAC3=0 for FW1814) and Q-followups-batch1."

## 2. Kext-side — fully wired, just not advertised

From A1-Q12 and `Q-followups-batch1.md:208`:

- `com_m_audio_FWEngine2` has the complete AC3 path: `ActivateAC3Output` @ `0x11fee`, `DeactivateAC3Output` @ `0x11fc6`, `SetAC3OutputState` @ `0x11f28`, `ClipAC3Output` @ `0x11c44`, `StartAC3Output` @ `0x1185e`.
- `com_m_audio_FWBaseEngine` has `AddAC3Format` @ `0x39a0`, `EnableAC3Format` @ `0x445e`, `DisableAC3Format` @ `0x596a`.
- The AV/C vendor+selector handshake that toggles non-mixable S/PDIF is already implemented.
- `IndexForAC3OutputChannels` is defined on all five engines in the family.

The **only** thing missing is a `SupportsAC3` override on `com_m_audio_FW1814Device`. Base returns `0`; FW410 overrides to `1`; FW1814 inherits the `0`. Set that one virtual to `1` and the entire path lights up.

## 3. Should we enable it on our FW1814 clone?

**Short answer: yes, trivially, when we get to kext work in Part D — but note the caveats.**

### 3.1 Cost

- One virtual override on the FW1814 device class: `virtual uint32_t SupportsAC3() const override { return 1; }`.
- `SetupAudioEngine` branch at Q3:19 (`e->[+0x348] = 1`) activates automatically from that flag — no further code changes needed.
- `AddAC3Format` runs during engine setup, registering an additional non-mixable 2-ch 16-bit S/PDIF format alongside the regular PCM formats. HAL ignores it until a client requests it.

### 3.2 Risks (all low)

- **HAL stream multiplexing:** AC3 and PCM share the same S/PDIF physical output. The AV/C handshake switches the port between mixable (PCM) and non-mixable (AC3) modes. The 2014 driver handles this correctly for FW410 — the FW1814 shares the same engine family (FWEngine2), so the same code should work.
- **Hardware readiness:** FW1814 hardware has S/PDIF out; whether the BeBoB firmware accepts the "enter AC3 mode" vendor opcode is confirmed by FFADO docs for the DM1000 family in general. We should still verify on-device: send the opcode, confirm `ActivateAC3Output` returns 0, and check that a Dolby Digital trailer plays through DVD Player or Audio MIDI Setup's non-mixable test.
- **No panel surface:** since no panel toggle exists, we won't be able to UI-test; the test is in Audio MIDI Setup → Format dropdown showing "Encoded Digital Audio" as an alternative to PCM.
- **No regression risk on normal PCM:** `AddAC3Format` registers an additional format; it doesn't modify the PCM path. If AC3 fails at activation time, PCM continues to work.

### 3.3 Decision for Part D

Enable `SupportsAC3=1` on the FW1814 device class as a **1-line feature-parity-plus** when kext work resumes. Treat it as low-priority — not a Part D gating item, just a cheap win to include. If activation turns out to be flaky on real hardware, revert the override; there is no downside to disabling.

**Verification plan** (for whenever hardware testing resumes):
1. `kextload` the modified kext.
2. Open Audio MIDI Setup, check that the FW1814's S/PDIF output stream advertises both `Linear PCM` and `Encoded Digital Audio` formats.
3. Play a known-good AC3 WAV through DVD Player or `ffplay -af aresample=48000 -f s16le test.ac3`.
4. Confirm a hardware AC3 decoder on the receiver end locks (receiver LED/display shows "Dolby Digital").
5. Confirm PCM playback still works after switching back to the PCM format.

## 4. Clone implications

- **Panel (AppKit rewrite):** no change — no toggle to port.
- **Kext (Part D clone or patch):** add one virtual override `SupportsAC3` returning `1` on the FW1814 device class.
- **CoreMIDI / firmware-helper:** unaffected.

## 5. Status

Closes **P10**. Panel surface is empty by design — AC3 is a CoreAudio HAL format advertisement, not a UI control. Kext-side enablement is a trivial 1-line change deferred to Part D kext work.
