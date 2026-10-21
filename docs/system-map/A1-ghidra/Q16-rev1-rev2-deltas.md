# Q16 — FWRev1AudioDevice vs FWRev2AudioDevice silicon-generation deltas

> **Correction (2026-10-19) — Audiophile phase 0** (evidence: [PROVENANCE-RESOLUTIONS.md §R77](../PROVENANCE-RESOLUTIONS.md), [Q-AP0-nub-and-rev2.md](Q-AP0-nub-and-rev2.md) §2).
> - **SyncStream phases (lines 73-74) are inverted.** ctype `0x01` = STATUS, `0x00` = CONTROL: the first frame `01 FF 1A 0F FF FE 60 <plug>` is the STATUS query, then `GetAddressAndPlugForSyncType` (vt[0xbc0]) writes the source into bytes 4/5 and the CONTROL `00 FF 1A 0F <addr> <srcplug> 60 <plug>` is sent (asm `0x12c60-0x12e1d`).
> - **Sync-type names (lines 44, 82) have no source in the binary.** The numbers are right (type 0 → `60`/plug, 1 → `FF 81`, 2 → `FF 00`, 3 → `FF 83`, 4 → `FF 82`), but `0x60` is the music-subunit *address*, and on the Audiophile **S/PDIF is type 4 (`FF 82`)**, not "Aux" (`SetClockSourceInternal @0xb9d4`; `ctl-services` `normal.rs:483-495`).
> - **Rev1 latency (line 39)**: asm `0x136da`: FDF1/2 (44.1/48) → 186, FDF3 (88.2) → 262, FDF4-6 (96/176.4/192) → 292, FDF0 → 0.
> - **"Solo = Rev1" (line 80)** is wrong: `FWSoloDevice` derives from `FWRev2AudioDevice` (table in the 2026-10-14 Correction above).
> - The Rev2 PlugSignalFormat layout (lines 68-69) is confirmed and completed in Q-AP0 §2.1 (AV/C `0x2F`, FFADO layout).


> **Correction (2026-10-14) — opcode 0x1A** (evidence: [PROVENANCE-RESOLUTIONS.md §R16](../PROVENANCE-RESOLUTIONS.md)). Opcode `0x1A` is the AV/C **SIGNAL SOURCE** command (FFADO `src/libavc/ccm/avc_signal_source.cpp:33` `AVC1394_CMD_SIGNAL_SOURCE 0x1A`), not a 'stream-input-connect'. It appears in `FWRev2AudioDevice::AVCSetupSyncStream @0x12c7e`: the frame is built `01 FF 1A 0F FF FE 60 <sync-type>` (asm 0x12ca9-0x12d23). Rev2-only; the FW1814 (Rev1) does not use it.


> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - 'AVC opcode 0x1A is the AV/C stream-input-connect' (lines 39, 75) is **unverified**: AV/C General spec lists 0x1A as SIGNAL SOURCE, and the path is Rev2-only. Check = AV/C General Specification opcode table.


> **Correction (2026-10-14) — the FW1814 is a Rev1 class, not Rev2.** OSMetaClass registrations in the slice (each `*::MetaClass::MetaClass` passes its superclass `gMetaClass` to `OSMetaClass::OSMetaClass`):
>
> | Class | Superclass | Source |
> |---|---|---|
> | `com_m_audio_FW1814Device` | **`FWRev1AudioDevice`** | `0xf473` (instance size `0x1d50`) |
> | `com_m_audio_FWProjectMixDevice` | `com_m_audio_FW1814Device` | metaclass ctor |
> | Audiophile, FW410, Ozonic, Solo, NRV10, Lightbridge | `FWRev2AudioDevice` | metaclass ctors |
> | `FWRev1AudioDevice`, `FWRev2AudioDevice` | `m_audio_b_FWAVCAudioDevice` | metaclass ctors |
>
> The C++ constructor agrees: `FW1814Device::FW1814Device @ 0xf04a` calls `FWRev1AudioDevice::FWRev1AudioDevice(OSMetaClass const*)` at `0xf060`. The FW1814 vtable slot `0xb50` resolves to `FWRev1AudioDevice::AVCControlPlugSignalFormat` (Q3a §3 Correction).
>
> **Consequences:**
> - The "FW1814 is Rev2" claim below is wrong.
> - The silicon-generation interpretation (Rev1 = pre-DM1000 without HSCI, Rev2 = DM1000) is **unsupported**. The FW1814 is DM1000 (snd-firewire-ctl-services `special.rs`) and uses HSCI through `FW1814Device::SetClockSourceInternal` (`[+0x268]->vt[0x8b8]`, Q5) while deriving from Rev1. "Rev1/Rev2" is better read as two AV/C stream-format strategies: plain `0x18/0x19` vs extended `0x2F`. Any claim about which Rev2 overrides the FW1814 "needs" must be re-checked against Rev1 + FW1814Device overrides. **Unverified:** the `[+0x2c8]` dependency argument in the original text. Check who writes `[+0x2c8]` on an FW1814Device instance.

**Status:** done (optional).

**Scope:** `FWRev1AudioDevice` (6 overrides) vs `FWRev2AudioDevice` (16 overrides) — the two silicon-generation subclasses of `m_audio_b_FWAVCAudioDevice`. Determines which generation the FW1814 belongs to and what the generation actually gates.

## Answer in one line

**FWRev2 = DM1000-class silicon with hardware HSCI control-packet reception, peak-meter DMA path, and AV/C stream-setup opcode `0x1A`. FWRev1 = pre-DM1000 chips with no HSCI and a simpler 8-byte PlugSignalFormat.** FW1814 inherits from the FW1814Device → FWRev2 line (indirectly via `FWBaseEngine`/`FWAVCAudioDevice`). Only `GetRoundTripLatencyForFDF` and `AVCControlPlugSignalFormat` are present on *both* generations; everything else is Rev2-only.

## Override surface

| Method | Rev1 | Rev2 |
|---|---|---|
| `AllocateAudioEngine` | returns `0` (stub — subclass's job) | `new com_m_audio_FWEngine2` |
| `stop` | release `[+0x268]` AVC unit then super::stop | release `[+0x2c8]` HSCI then `[+0x268]` AVC unit then super::stop |
| `GetRoundTripLatencyForFDF` | 3-value: `44.1k/88.2k/96k→0xba(186)`, `48k→0x106(262)`, `176.4k/192k→0x124(292)` | 5-value: `44.1k→0xe2(226)`, `48k→0xd2(210)`, `88.2k→0x1c9(457)`, `96k→0x181(385)`, `176.4k/192k→0x18d(397)` |
| `AVCControlPlugSignalFormat` | 8-byte vendor-dep: `[00 FF (0x18|dir) rate 0x90 fmt FF FF]` | 14-byte vendor-dep `0x2F / 0xC1 / 0xC0` — two-pass (probe plug for `cVar13` match, then full command with payload) |
| `initHardware` | *(inherits)* | thin wrapper around super |
| `IsReceptionOfControlPacketsEnabled` | — | `(this[+0x2c8] != 0)` |
| `UpdatePeakMeters` | — | big-endian swap + `>> 16` into `[+0x108] + i*4` |
| `GetAddressAndPlugForSyncType` | — | 5-way enum: {Internal:plug=0x60, S/PDIF:plug=0x81, ADAT:plug=0, WordClock:plug=0x83, Aux:plug=0x82} |
| `performPowerStateChange` | — | wrapper around super |
| `AVCSetupSyncStream` | — | 8-byte AV/C control `01 FF 1A 0F FF FE 60 <plug>` then if OK call `vtable[0xbc0]` then same cmd with byte0 flipped to 0 (SET phase) |
| `StopReceptionOfControlPackets` | — | `HSCI::Stop` + `ClearPeakMeters` |
| `PauseReceptionOfControlPackets` | — | `HSCI::Pause` + `ClearPeakMeters` |
| `ResumeReceptionOfControlPackets` | — | `HSCI::Resume` |
| `StartReceptionOfControlPackets` | — | `new FWHSCI` → attach workloop → `HSCI::Start(workloop)`; also stores a TFunctor callback with tag `0xBB9` into HSCI `[+0xb8]` |

## Field map

`FWRev2AudioDevice`:
- `+0x108` — `uint32_t[]` peak-meter buffer base (one quadlet per channel, big-endian store)
- `+0x268` — AVC unit nub (shared with Rev1)
- `+0x2c8` — `m_audio_b_FWHSCI*` control-packet receiver (Rev2 only)

## AVC opcodes decoded

**Rev1 PlugSignalFormat** (8 bytes):
```
[0]=0x00 (CONTROL)  [1]=0xFF  [2]=0x18|(dir!=0)  [3]=rate(FDF)
[4]=0x90            [5]=format [6]=0xFF  [7]=0xFF
```

**Rev2 PlugSignalFormat** (14 bytes, two-phase):
- Phase 1 — probe: `01 FF 2F C1 (dir==0) 0 0 plug FF FF ...` → read reply byte[13] until it matches generation-specific marker (`cVar13` from switch on param_4: 0→2, 2→4, 3→10, 4→5, 5→6, 6→7, else 3).
- Phase 2 — set: `00 FF 2F C0 (dir==0) 0 0 plug ... FF FF 90 40 cVar13 [n_bytes] <memmove from probe reply>`

**Rev2 SetupSyncStream** (8 bytes, two-phase):
```
Phase 1: 01 FF 1A 0F FF FE 60 <plug>   // CONTROL stream-connect
Phase 2: 00 FF 1A 0F FF FE 60 <plug>   // GET-STATUS confirm
```

## Why this matters

- ~~**FW1814 is Rev2.**~~ (**wrong — see Correction 2026-10-14 at top**) The HSCI envelope decoded in Q6 and the StartExternalSyncUnmuteTimer path (part of Q5) require `[+0x2c8]` to be non-null — Rev1 can't source these at all.
- The clone's HSCI subsystem (currently open-coded as `FW1814Device::StartHSCI` in M4) is actually the Rev2 base-class state machine. If we ever add more M-Audio devices (Audiophile = Rev2 too, Solo = Rev1), the clone can't share HSCI across both without replicating this generation gate.
- `GetAddressAndPlugForSyncType` is the authoritative plug→addr map for the sync-stream code path. S/PDIF/ADAT/WordClock all go to plug `0xFF` with distinct sync-type bytes (`0x81/0x00/0x83`) — matches the Q5 clock-source enum but keyed on *destination plug*, not source enum.
- **AVC opcode `0x1A`** is the AV/C stream-input-connect (two-phase CONTROL + GET-STATUS) — not vendor-dependent. Clone's current `ConnectAllPlugs` builds this opcode manually; it should instead call `AVCSetupSyncStream` so the GET-STATUS confirmation is kept.

## Clone impact

- Keep the clone's `FW1814Device` single-class design — don't split into Rev2 base + FW1814 concrete unless we support more M-Audio devices.
- Mirror the two-phase SyncStream protocol in `ConnectAllPlugs` (currently fire-and-forget).
- Publish `UpdatePeakMeters` behavior to the clone's user-client (big-endian swap + `>>16` saturation) — current `FW1814Device::GetPeakMeters` returns raw HSCI payload values which the panel then has to swap client-side.
- `GetAddressAndPlugForSyncType` enum maps directly onto our `FW1814ClockSource` enum from auto-memory — same numbers, same destinations.

## Provenance

- All decompiles: `scripts/DecompQ16Q17.java` → `ghidra-exports/decomp/q16q17.log` lines 411–1047.
- Addresses: Rev1 ctor `0x13904`, Rev2 ctor `0x13574`; Rev1 AllocateAudioEngine `0x136aa`; Rev2 StartReceptionOfControlPackets `0x13138`.
