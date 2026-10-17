# Q3b — mixer / control power-on defaults (factory reset)

> **Update (2026-10-14, P4/R38) — Q3b closed.** (1) The factory state **is** sent to the device at init: `FWAVCAudioDevice::initHardware` runs `SetBlankSlateClockSource` → `ResendCurrentMixerSettings` (parent: throttled clock-source resend; FW1814: ABSwitch/Rotary/AuxSend/Levels `SendToDevice`) → `StartReceptionOfControlPackets` (starts the HSCI poll); the same resend runs on wake, on `MAMetaResetToFactorySettings` and after a non-blank clock change. (2) The property→key mapping is in Q-MAMAP and the default wire words are tabulated in `Q-P4-cmp-per-rate-and-mixer-power-on.md` Part B (levels zero / aux send `0x8080` verified; input-pan half-word order, headphone-selector initial image and the two AV/C-routed frames stay open). The "Open (Q-MAMAP)" §3 below is superseded.
> **Correction (2026-10-14) — MA* crosswalk** (evidence: [Q-MAMAP-key-crosswalk.md](Q-MAMAP-key-crosswalk.md), [PROVENANCE-RESOLUTIONS.md §R20](../PROVENANCE-RESOLUTIONS.md)). For the FW1814 the mixer is **not** a single path: levels (SW return 0x00, output 0x08, input 0x10, headphone 0x38, aux output 0x34, aux send 0x64), input pan (0x40+), input routing (0x90), headphone selector (0x98) and aux monitor (0x9c) are vendor register writes via `CustomMixerWriteQuads` to `0xFFC7:0x00700000+offset`; **`MAAuxOutputPan2` (FWSettingsAuxSend::SetAuxPan → `AVCControlFeatureFunctionBlock`) and `MASWReturnRoutingV2` (base `FWSettingsLevels::SetSWReturnRouting` → `SetAVCRouting` → `AVCMixerConnectPair`) go over AV/C.** Mute/solo/dim are level operations (minimum / −10 dB). Level wire value = property level (−255..0, 0.5 dB) × 0x80, int16 big-endian, two channels per quadlet.


> **Correction (2026-10-14) — settings persistence** (evidence: [A4 H11](../A4-helper/H11-settings-persistence.md)). Resolved: the **helper** replays saved settings (CFPreferences key = GUID, suite `com.m-audio.device-settings.firewire`) with one `setProperties` at device attach and user login; the Panel does not (SL capture 71). Open: why two 6570-byte writes were seen.


> **Correction (2026-10-14) — Part A completion batch 1** (evidence: [PROVENANCE-RESOLUTIONS.md §R8](../PROVENANCE-RESOLUTIONS.md)). Saved-pref reapplication at runtime is done by the helper daemon (SL v1.9 capture 71), not by the Panel launch; payload decode still open.


**Status:** partial 2026-10-14. The property-level defaults are decoded from raw disassembly. Still open: **which MA\* key** each settings array backs, and how a property value **encodes into register bits** — both belong to Q-MAMAP. Answers BUILD-MANUAL §2.E Q3b ("decompile `initHardware`").

**Sources:** raw `otool-exports/mafw.asm`, FW1814 vtable `0x369b0` (+0x10 address point), Ghidra 12.1.4 read-only decompiles (scratchpad, 2026-10-14). Bundle: `_extracted/` v1.11.x.

## 1. Where defaults come from

`m_audio_b_FWAudioDevice::initHardware @ 0x342e`. Calls through the FW1814 vtable resolve as follows (in order):

1. `setProperty("IOAudioDeviceConfigurationApplication", "com.m-audio.appc.MAfw")`, then `IOAudioDevice::initHardware`. Also sets names, `MAIconName` and `MAMaxBytesPerCycle`.
2. `[+0xf0] = vt[0xa28]()` = **`FWAVCAudioDevice::AllocateConnectionManager` @ `0x23c72`** (fail → cleanup).
3. `vt[0xa30]()` = **`FWAudioDevice::createAudioEngine` @ `0x2fd8`** (fail → cleanup).
4. `vt[0xa78](engine[+0x220])` = `RequestFormatUpdateForSampleRate`, then `FWBaseEngine::CompletePendingStateChanges` (≠0 → cleanup).
5. `vt[0xb28]()` = **`FW1814Device::InitializeUserSettableProperties` @ `0xe6ca`** (≠0 → cleanup). This builds the settings objects and channel groups (Q15f Correction).
6. Meter page: `[+0x100]`/`[+0x108]`/`[+0x110]` (Q-UC-surface §3), `MARateLocked = 1`, peaks zeroed.
7. **`vt[0xb08]()` = `FW1814Device::ResetToFactorySettings` @ `0xd190`.** These are the power-on defaults.
8. `AddToDeviceList()`, then `engine->vt[0xc70]()`; return 1.

There is no device read-back of mixer state. The kext seeds every control from constants. **Unverified:** whether the Panel or helper then re-applies saved user settings (`com.m-audio.device-settings.firewire.plist`, B5 capture `23-fw1814-current-prefs.xml`) over these defaults. Check: a DTrace `setProperties` trace at Panel launch on the SL rig.

> **Correction (2026-10-14, Q-SDK.5 / R31):** the panicking path does **not** run `initHardware` at all; the candidate is rejected by `FWAVCAudioDevice::probe` (first matching round) and freed (`Q-SDK5-first-round-probe-reject.md`). The "one of steps 2–5" reading below is withdrawn; steps 2–5 are untested on 10.13.6, not failing.
>
> **Panic link (observation, superseded above):** `[+0x110]` is only allocated in step 6 (`0x3624..0x3635`). A failure in steps 2–5 (or before `initHardware` runs) leaves it NULL, and `FWAudioDevice::free()` then deletes it at `0x23b7`. That is the 10.13 panic site (Q-SDK §5). So the "unknown earlier failure" is one of steps 2–5 or a start-path failure before step 1. **Unverified which.**

## 2. `FW1814Device::ResetToFactorySettings @ 0xd190`

```
FWRev1→…→FWAudioDevice::ResetToFactorySettings(this)   @0x1db4   [+0x144] = 0x40000000
FWSettingsABSwitch::ResetToFactorySettings(this+0x2d8)   @0x1df88
FWSettingsRotaryControl::ResetToFactorySettings(this+0x470) @0x21060
FWSettingsAuxSend::ResetToFactorySettings(this+0xf98)    @0x20b9c
FWSettingsLevels::ResetToFactorySettings(this+0x538)     @0x21bc4
[this+0x144] = 0x40000003                                 (FW1814 override, last write wins)
```

| Object | Default (raw) | Source |
|---|---|---|
| **`MAClockSource`** (`[+0x144]`) | **`0x40000003`** = gate bit 30 + sync-source bits[1:0] = 3 → HSCI wire byte 6 = `0x00` ("Internal — digital inputs muted", C2 §2.4 table); bits 2–5 = 0 (coax S/PDIF, optical in/out S/PDIF, S/MUX off) | `0xd190` tail; `0x1db4` writes `0x40000000` first |
| A/B switch | `+0x104 = 0`, `+0xd0 = 0`, array `+0x70 = {1, 2, 3, 4}`, `+0x144 = 1`, `+0x184 = 1` | `0x1df88` |
| Rotary control | `+0x68` (u64) = `0x0000ffff00000000` (low u32 = 0, high u32 = `0xffff`) | `0x21060` |
| Aux send | every entry of channel group `[+0x10]` = `0xffffff01`; array `+0x260 = {0xffffff01, 0xff}`; array `+0x200` filled with 0 | `0x20b9c` |
| Levels | `+0x11c = 1`; `[+0x8c0]->+0x11 = 0`; groups `+0x930` (if `+0x942`) and `+0x8d0` (if `+0x8e2`): every channel `level = 0, balance = 0, mute = 0, solo = 0, linked = 1`; array `+0x10`: pairs `{0xffffff01, 0xff}` for `i = 0, 2, … < [+0x3c]`; array `+0x990[i] = 0`; array `+0x9f0[i] = 1 << i` (identity routing mask) | `0x21bc4..0x21c8e` + helper bodies below |

**Channel defaults** (raw asm). `BaseChannelGroup::SetAll*` ignore their argument and store constants into each `Channel`:

| Helper | Address | Store |
|---|---|---|
| `SetAllLevels(int)` | `0x215b0` | `movl $0x0, (%rax)` at `0x215df` → level = 0 |
| `SetAllBalance(int)` | — | `+0x4 = 0` (`0x21627`) |
| `SetAllMute(bool)` | — | `+0x8 = 0` (u64, `0x21671`) |
| `SetAllSolo(bool)` | — | `+0x10 = 0` (`0x216bb`) |
| `SetAllLinked(bool)` | — | `+0x18 = 1` (`0x21705`) |

(Ghidra drops the call arguments here; the callers never load `esi` before these calls, `0x21bf2..0x21c54`.)

## 3. Open (Q-MAMAP)

- **Property → MA\* key.** Which `FWArrayProp`/`FW32BitNumberProp` at the offsets above is `MAInputRouting`, `MAAuxMonitor`, `MAHeadphoneSource`, etc. Check: the `Init` functions (`FWSettingsLevels::Init`, `FWSettingsAuxSend::Init`, `FWSettingsABSwitch::Init`) where property names are registered.
- **Property value → register word.** For example, what level `0` and the `0xffffff01`/`0xff` pairs become on the wire. `BaseChannelGroup::AdjustedLevels` / `AdjustedAndMultipliedLevels` (`~0x1f…`) compute the quadlet values written by `CustomMixerWriteQuads` (Q15f). FFADO's register semantics (`0x0000` = max, `0x8000` = min; `special_avdevice.h:50–54`) are a decoder, not the spec.
- **Is the factory state what the device actually gets at power-on?** It depends on whether `ResendCurrentMixerSettings` / `SendAllToDevice` runs after the reset. **Unverified.**

## Provenance

`mafw.asm`: `0x342e..0x36cf` (initHardware), `0xd190`, `0x1db4`, `0x1df88`, `0x20b9c`, `0x21060`, `0x21bc4..0x21cb0`, `0x215b0..0x215f7`, `0x21621..0x21705`. Vtable slots `0xa28/0xa30/0xa78/0xb08/0xb28/0xaf0` resolved from FW1814 vtable bytes.
