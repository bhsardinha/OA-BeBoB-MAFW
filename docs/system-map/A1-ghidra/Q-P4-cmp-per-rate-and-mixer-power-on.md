# Q-P4 — Per-rate CMP/packet/bandwidth parameters and the mixer power-on state

> **Correction (2026-10-15, Q-ME §7b):** three of the four "open" wire items of Part B are closed. (1) The AV/C frames for aux pan and SW-return routing are decoded (feature block `00 08 B8 81 09 10 02 <ch> 03 02 <hi> <lo>`; mixer connect pairs `00 08 B8 82 01 10 04 00 <src> <dst> 03 02 <00|80> 00 00 00`). (2) The `0x98` headphone-selector register is **not written at init** (only the ABSwitch setters reach `SetHeadphoneSelector`), so there is no initial image to derive. (3) The pan half-word order follows from the raw-memory send of host-order words (Q-ME §7b); whether the device agrees is verified at the integrated run.

**Status:** done 2026-10-14 (BUILD-PLAN task P4; static, dev Mac; evidence entry R38). Closes the open parts of **Q-CMP-CONNECT** (per-rate parameters) and **Q3b** (mixer power-on defaults). Bundle: kext 1.11.3 (`_extracted/kext/…x86_64-slice`, read with `otool` and `llvm-objdump`, which symbolises internal calls). Live cross-checks: SL v1.9 captures 12/40/50 (ioreg), 72 (per-rate stream channels), 74 (TX packet size). Nothing here has run on 10.13.6.

## Part A — CMP / packet / bandwidth per rate and mode

### A.1 Where the numbers come from (all read in the binary)

1. **Channel counts** — `FW1814Device::GetDeviceChannelsForSampleRate(rate) const @0xc998` copies one of four static `DeviceChannels` tables into `sDeviceChannels` (`0x4ac60`) and returns it. Selection: `rate ≥ 176400` → `s176_192DeviceChannels` (`0x4ab80`, **mode-independent**); `88200/96000` → `sADAT88_96` (`0x36580`) or `sSPDIF44_96` (`0x363c0`); every other rate (44.1/48 kHz, and the 32 kHz fall-through) → `sADAT44_48` (`0x364a0`) or `sSPDIF44_96`. The **input** fields (`+0x0`, `+0x4`) come from the table chosen by `MAClockSource` bit 3 (`fMode [+0x144] & 8`, 1 → ADAT), the **output** fields (`+0x8`, `+0xc`) from the table chosen by bit 4 (`& 0x10`). Table contents (raw words, little-endian):

   | Table | in `+0x0/+0x4` | out `+0x8/+0xc` | `+0x10` | `+0x30` / `+0x38` | `+0x40..+0x6c` |
   |---|---|---|---|---|---|
   | `sSPDIF44_96` | 10 / 10 | 6 / 6 | **1** | `0x2f890` each | `1 1 4 0 1 5 5 1 0 1 2 0` |
   | `sADAT44_48` | 16 / 16 | 12 / 12 | **1** | `0x2f890` | `1 1 4 0 1 5 8 2 0 1 2 0` |
   | `sADAT88_96` | 12 / 12 | 8 / 8 | **1** | `0x2f890` | `1 1 4 0 1 5 6 2 0 1 2 0` |
   | `s176_192` | 2 / 2 | 4 / 4 | **1** | `0x2f890` | `1 1 1 0 0 1 2 0 0 0 0 3` |

   `+0x10 = 1` is the MIDI quadlet count per data block (also confirmed by `FW1814Device::SetupAudioEngine` storing `[+0x2e8] = 1` at `0xed7a`, which `UpdateDCLProgramsWithNewPacketSizes @0x122d6` passes as `m`). The meaning of `+0x30/+0x38` (`0x2f890 = 194704`) and of `+0x40..+0x6c` was not decoded. `SetupAudioEngine` also stores `[+0x2d6] = 1` (`0xed6c`), so for the FW1814 the RX packet parameters are refreshed at every rate (the high-rate skip in `UpdateDCLProgramsWithNewPacketSizes` applies only to products that do not set it).
2. **Rate row** — `FWDCLProgram::SetPacketParameters(rate, a, m, …) @0x272f4` stores `[+0x64] = a` (channels) and `[+0x68] = m`, selects the 7-row rate table (rate → SYT interval 8/8/8/16/16/32/32, SYT increment `0x1800/0x116A/0x1000/0x116A/0x1000/0x116A/0x1000`, FDF index 0..6; default 44.1 kHz, same as C2 §5), calls `SetupCIP` and then the virtual `SetPacketSize(a, m)` (`vt[0x118]`). `UpdateDCLProgramsWithNewPacketSizes` calls it for the output program with `a = [+0x2e0]` (output channels) and the input program with `a = [+0x2e4]` (input channels).
3. **Packet bytes** — `FWDCLOutputProgram::SetPacketSize @0x260d2`: `4·(a+m)·SYT + 8`, count `4·[+0x4e0]`; `FWDCLInputProgram::SetPacketSize @0x260a8`: `4·(a+m)·SYT + 12`, count `0x100` (Q-CMP-connect §2). **Pool** — `TPacketPool::Initialize`: `packets/page = ⌊4096/bytes⌋`, `pages = ⌊count/(packets/page)⌋ + 2` (Q-P1P2 §3.1).
4. **Bandwidth** — `FWAudioDevice::GetBandwidthBytesAtSampleRate(rate) @0x1ee2`: `ch = vt[0xa68](rate)` (the table above); `ecx = engine ? ([engine+0x224] << 2) & 0x3fc : 0x20` (= SYT·4); **bytes per cycle = (in + out + 2·m)·(SYT·4) + 16**, with `in = ch[0]`, `out = ch[8]`, `m = ch[0x10]`. `SufficientBandwidthAvailable @0x22e8` returns true iff that value is `< 0x186b` (6251): it also calls `MaximumBandwidthBytesSupported @0x2234` but **ignores its result**. `FW1814Device::GetMaxBandwidthBytesPerCycle` (`vt[0xb30]`, `@0xc364`) returns `0x928` = **2344**; `MaximumBandwidthBytesSupported` is the minimum of `0x186a` (6250) and every attached audio device's maximum. Every value in the table below is ≤ 1424, below both limits.
5. **Speed** — `FWRemoteIsochPort`/`FWIsochChannel` take `nub->FWSpeed()` (`vt[0x850]`, R8). The SL ioreg shows `"FireWire Speed" = 2` on the FW1814's nodes (`12-ioreg-full.txt:3866, 3970`, same block as `Vendor_ID = 3436`, `Model_ID = 65649`), i.e. **S400** (code 2; the line-3774 value 3 belongs to a different node). **Channel** — chosen by the IRM (`allocateChannel`), no fixed value (Q-CMP-connect §1). **IRM bandwidth units** — computed inside IOFireWireFamily from `packetSize` and speed; the kext passes none (the 4.7.2 allocation arithmetic was not read).

### A.2 Table (computed from A.1; every input is a verified word or formula)

`P = ⌊65,536,000/(rate·4)⌋ + 2` (TX packets per bank, `SetupCIP @0x27113`); TX count `4·P`; RX count 256. "mode" = `MAClockSource` bit 3 (input) / bit 4 (output). Other mode letters: S = S/PDIF, A = ADAT.

| Rate | mode (in/out) | in ch | out ch | SYT | P | TX count | TX B/pkt | TX pk/page | TX pages | RX B/pkt | RX pk/page | RX pages | BW B/cycle |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 44.1 kHz | S / S | 10 | 6 | 8 | 373 | 1492 | 232 | 17 | 89 | 364 | 11 | 25 | 592 |
| 44.1 kHz | A / A | 16 | 12 | 8 | 373 | 1492 | 424 | 9 | 167 | 556 | 7 | 38 | 976 |
| 44.1 kHz | S in / A out | 10 | 12 | 8 | 373 | 1492 | 424 | 9 | 167 | 364 | 11 | 25 | 784 |
| 44.1 kHz | A in / S out | 16 | 6 | 8 | 373 | 1492 | 232 | 17 | 89 | 556 | 7 | 38 | 784 |
| 48 kHz | S / S | 10 | 6 | 8 | 343 | 1372 | 232 | 17 | 82 | 364 | 11 | 25 | 592 |
| 48 kHz | A / A | 16 | 12 | 8 | 343 | 1372 | 424 | 9 | 154 | 556 | 7 | 38 | 976 |
| 48 kHz | S in / A out | 10 | 12 | 8 | 343 | 1372 | 424 | 9 | 154 | 364 | 11 | 25 | 784 |
| 48 kHz | A in / S out | 16 | 6 | 8 | 343 | 1372 | 232 | 17 | 82 | 556 | 7 | 38 | 784 |
| 88.2 kHz | S / S | 10 | 6 | 16 | 187 | 748 | 456 | 8 | 95 | 716 | 5 | 53 | 1168 |
| 88.2 kHz | A / A | 12 | 8 | 16 | 187 | 748 | 584 | 7 | 108 | 844 | 4 | 66 | 1424 |
| 88.2 kHz | S in / A out | 10 | 8 | 16 | 187 | 748 | 584 | 7 | 108 | 716 | 5 | 53 | 1296 |
| 88.2 kHz | A in / S out | 12 | 6 | 16 | 187 | 748 | 456 | 8 | 95 | 844 | 4 | 66 | 1296 |
| 96 kHz | S / S | 10 | 6 | 16 | 172 | 688 | 456 | 8 | 88 | 716 | 5 | 53 | 1168 |
| 96 kHz | A / A | 12 | 8 | 16 | 172 | 688 | 584 | 7 | 100 | 844 | 4 | 66 | 1424 |
| 96 kHz | S in / A out | 10 | 8 | 16 | 172 | 688 | 584 | 7 | 100 | 716 | 5 | 53 | 1296 |
| 96 kHz | A in / S out | 12 | 6 | 16 | 172 | 688 | 456 | 8 | 88 | 844 | 4 | 66 | 1296 |
| 176.4 kHz | any | 2 | 4 | 32 | 94 | 376 | 648 | 6 | 64 | 396 | 10 | 27 | 1040 |
| 192 kHz | any | 2 | 4 | 32 | 87 | 348 | 648 | 6 | 60 | 396 | 10 | 27 | 1040 |

(32 kHz falls into the 44.1/48 channel tables with SYT 8, `P = 514`; not offered by the hardware list.) Pool sizes are `pages × 4 KiB` (≈ 0.1–0.4 MB each); this table **supersedes the table in Q-P1P2 §3.2**, which had assumed only the capture-72 clock configuration and now carries the verified channel counts and the S/PDIF-out cases.

### A.3 Cross-checks against live data
- **Capture 72** (SL v1.9, `MAClockSource = 0x40000014`: bit 4 set, bit 3 clear → S in / A out): stream channels out/in = 12/10 at 44.1/48 kHz, 8/10 at 88.2/96 kHz, 4/2 at 176.4/192 kHz — identical to the table rows.
- **Capture 74** (48 kHz, 12 audio + MIDI out): TX packet 424 B = table row "48 kHz A out".
- **`MABytesPerCycle = 976`** (all three SL ioreg captures 12/40/50, 44.1 and 48 kHz): matches **only** the "A / A, SYT 8" combination `(16 + 12 + 2)·32 + 16 = 976` — so those captures ran with both optical ports in ADAT mode (consistent with `MAClockSource = 0x4000001c`, bits 2–4 set, used in the cap-61 clock-source tests). The property is described as "stale" in B5 because the Panel/engine writes it once (`RecalculateBandwidth @0x1235a` → `SetStaticPropertyAndNotifyUserClients` with `GetBandwidthBytesAtSampleRate([+0x220])`).

### A.4 What the clone must do (CMP side)
Reproduce A.1 exactly: the four `DeviceChannels` tables and the selector (bit 3 → inputs, bit 4 → outputs, rate class), `m = 1`, the packet-byte and pool formulas, the bandwidth formula and the 6250-byte constant check, and pass `FWSpeed()` and an IRM-allocated channel as the reference does (C4 §2). `RecalculateBandwidth` (when `[+0x348] != 0`) also sends the INPUT then OUTPUT plug-signal-format commands (`vt[0xb50]` of the AV/C device with `(direction, plug 0, FDF index [+0x228], channels, 100)`), in addition to the sequence in `ConnectAudioMIDIChannels` (C2 §7).

---

## Part B — Mixer power-on state (closes Q3b)

### B.1 When the factory state reaches the device (this was the open question)
`FWAVCAudioDevice::initHardware @0x22ed2` runs, after `FWAudioDevice::initHardware` (which ends with `ResetToFactorySettings`, Q3b §1 — RAM only), three device virtuals **in this order**:
1. `vt[0xb58]` = `FW1814Device::SetBlankSlateClockSource @0xe4f0` — HSCI clock frame (C2 §2.7; sleeps 300 + 300 + 2500 ms).
2. `vt[0xb00]` = `FW1814Device::ResendCurrentMixerSettings @0xd014` — see below.
3. `vt[0xb78]` = `FW1814Device::StartReceptionOfControlPackets @0xe064` — creates the `FWHSCI` object (0xd8 bytes), sets its read length from `GetControlPacketSize()` (`vt[0xba0]`, `@0xc962`), registers `ReadHSCIData(unsigned*)` (PMF `0xbf9` → `vt[0xbf8]`, `@0xd952`) as the listener and **starts the poll** (`FWHSCI::Start(workloop)`); `initHardware` succeeds only if this returns true. **Listener chain (verified):** `ReadHSCIData` reads the packet word at byte offset `0x50` (`bit 24` = external sync present, `bits 18:16` = FDF index), calls `DetectExternalSyncChanges(bit24, fdf)`, then tail-calls device `vt[0xbd8]` = `ReceiveControlPacket(FW1814ControlPacketWithoutNodeID*) @0xe9ea` (the peak-meter/rotary decoder, Q-UC-surface §3). A sibling `ReceiveControlPacket(FWControlPacket*) @0xd98e` (`vt[0xbd0]`) also calls `DetectExternalSyncChanges`; its caller was not found.

`ResendCurrentMixerSettings` (FW1814 override @0xd014) = parent (`FWAVCAudioDevice::ResendCurrentMixerSettings @0x23af8`) + four sends: **parent:** if `HasSoftwareControls()` (`@0xc32c`, returns 1 for the FW1814) — throttle: if less than 1 s (`≤ 999,999,999 ns`) since the previous resend (`[+0x270]`) it logs `"M-FW: Trying to resend mixer settings too often (within %d second of previous resend)"` and `IOSleep(1000)`; then it re-applies the clock source through the `SetClockSource(fMode)` functor (`[+0x150]`); stamps `[+0x270]`. **FW1814:** `FWSettingsABSwitch::SendToDevice` (`@0x1df6c`: only normalises `[+0x104]` to 0/1; no wire), `FWSettingsRotaryControl::SendToDevice` (base no-op `@0x21058`), `FWSettingsAuxSend::SendToDevice @0x20af4`, `FWSettingsLevels::SendToDevice @0x21a36`. **So yes: the factory state is pushed to the device on every init** (this matches capture 73's cold-start log: clock sets 300 ms ×2 and 2500 ms, then "Trying to resend mixer settings too often", then the stream-start chain).
Other triggers of the same `vt[0xb00]` (callers found by slot): `FWAudioDevice::ResetMixer @0x2660` (property `MAMetaResetToFactorySettings`), `FWAVCAudioDevice::SystemWokeUp @0x22d76` (after sleep), and `SetClockSourceInternal` when `!blankSlate` (`FWSettingsLevels::SendToDevice` at `0xe4ad`).

### B.2 What `SendToDevice` sends (structure; exact word order not captured)
- **Levels** (`@0x21a36`): the combined groups at `+0x8d0` and `+0x930` (when present: `GroupChannelGroup::SendLevelsToDevice`), the object at `+0x8c8`, then the pan array (`+0x10`, via functor `[+0x48]`) and the routing arrays (`+0x990` input routing, `+0x9f0` SW-return routing) through their functors.
- **Aux send** (`@0x20af4`): the aux-send group (`[+0x10]->vt[0x28](0)` → rotated register-`0x64` write), the aux-monitor array (`+0x200`, functor `[+0x238]`, one call per element), and the aux-pan array (`+0x260`, functor `[+0x298]`, elements 0 and 1 → AV/C Feature function block).
- Sends are `CustomMixerWriteQuads` register writes (C3 §3) except the AV/C-routed keys. The relative order of the individual register writes inside `Levels::SendToDevice` was not fully traced (**open**); it does not change the final device state.

### B.3 Defaults and their wire words (after `ResetToFactorySettings @0xd190`)
Encoding rule (Q-MAMAP §1.4): level (−255..0, 0.5 dB) × `0x80`, int16, byte-swapped to big-endian, two channels per quadlet; mute/solo/dim have no register.

| State | Default (RAM) | Wire at init | Status |
|---|---|---|---|
| `MAClockSource` | `0x40000003` | HSCI frame `00 FF 00 04 00 04 00 00 00 00 …` (wire byte 6 = 0; bytes 7–9 = 0), bytes 10–15 unverified | verified (C2 §2.4) |
| SW return (4 ch), Output (4), Input (18), Headphone (4), Aux output (2) levels | 0 (0 dB), balance 0, mute 0, solo 0, linked 1 | register blocks `0x00` (2 quads), `0x08` (2), `0x10` (9), `0x38` (2), `0x34` (1): **all zero** | **verified** (zero is invariant under byte order) |
| Aux send (22) levels | −255 each | register block `0x64` (11 quads): every int16 = `0x8080` → quads `80 80 80 80` (rotated order irrelevant: all equal) | **verified** |
| `MAInputPan2` (18) | pairs `{−255, +255}` | register `0x40 + 4·((ch−1)/2)`, 9 quads from the half-word formula `((−p>>1)&0xff) + p·(−0x8000)` | formula verified; **wire half-word order not derived (open)** |
| `MAInputRoutingV2` (9) | 0 | register `0x90`: **0** | verified |
| `MAMonitorAuxOnOutput` | 0 (`Fill(0)`) | register `0x9c`: **0** | verified |
| Headphone source / second source | `1` / `1` (`[+0x144]`, `[+0x184]`), momentary routings `{1,2,3,4}`, switch mode 0 | register `0x98` from the image `[+0x1310]` (bits 8/9/10 or 24/25/26 by A/B position and phone index) | bit layout from Q-MAMAP; **initial image value not derived (open)** |
| `MAAuxOutputPan2` (2) | `{−255, +255}` | **AV/C** Feature function block, selector 3, value<<7 | frame **not derived (open)** |
| `MASWReturnRoutingV2` (2) | element i = `1<<i` | **AV/C** mixer connect pair per set bit | frame **not derived (open)** |
| `MARotaryControlV2` | `0xFFFF00000000` | none (RAM only) | verified |
| `MADirectMonitoringActive` | `[+0x11c] = 1`, input mute flag cleared | none directly (value ignored) | verified |
| `MAEnableStereoPairEngines` | 0 (SL prefs) | none | observation (SL) |

### B.4 What the clone must do (mixer power-on)
- Run, at init, in this order: blank-slate clock source → resend (clock source via `SetClockSource`, with the 1 s throttle and its log/sleep, then ABSwitch/Rotary/AuxSend/Levels sends) → start the HSCI poll; repeat the resend on wake, on `MAMetaResetToFactorySettings`, and after every non-blank clock-source change.
- Use the verified words above (zeros; `0x8080` for aux sends). For the four **open** wire items (input pan half-word order, headphone-selector initial image, the AV/C aux-pan and SW-return-routing frames) the reference's bytes are not recoverable from the SL rig (`fbt` reaches only `mach_kernel`). Options, in order: (1) take the **register semantics from FFADO/macfw/Rust as a decoder** (allowed for wire bytes; C4 §8) and validate by ear/behaviour at M-E against the SL; (2) capture the reference's writes with a bus analyser (not available); (3) leave the item at the device's own power-on state and verify the audible result. Whichever is chosen must be labelled in code as third-party-derived until a capture exists.

## Open / unverified
- Wire half-word order of `MAInputPan2`; initial `0x98` image; AV/C frames for aux pan and SW-return routing (Part B above).
- Meaning of `DeviceChannels +0x30/+0x38` (`0x2f890`) and `+0x40..+0x6c`.
- IRM bandwidth-unit arithmetic inside IOFireWireFamily 4.7.2 (not read).
- Order of individual register writes inside `FWSettingsLevels::SendToDevice` (final state unaffected).
- Everything on 10.13.6.

## Provenance
`mafw.asm`/`llvm-objdump`: `GetDeviceChannelsForSampleRate @0xc998`, tables `0x363c0/0x364a0/0x36580/0x4ab80`, `SetupAudioEngine @0xed34` (`[+0x2e8]` `0xed7a`, `[+0x2d6]` `0xed6c`), `UpdateDCLProgramsWithNewPacketSizes @0x122d6`, `SetPacketParameters @0x272f4`, `Output/Input::SetPacketSize @0x260d2/0x260a8`, `GetBandwidthBytesAtSampleRate @0x1ee2`, `SufficientBandwidthAvailable @0x22e8`, `MaximumBandwidthBytesSupported @0x2234`, `GetMaxBandwidthBytesPerCycle @0xc364`, `RecalculateBandwidth @0x1235a`, `FWAVCAudioDevice::initHardware @0x22ed2`, `ResendCurrentMixerSettings @0xd014/@0x23af8`, `StartReceptionOfControlPackets @0xe064`, `FWSettings{Levels,AuxSend,ABSwitch}::SendToDevice @0x21a36/@0x20af4/@0x1df6c`, vtable `0x369b0` (address point `0x369c0`, slot = entry − `0x369c0`; entries from `otool-exports/mafw-data-const.txt`). Captures: `B5-sl-dtrace-truth/captures/12-ioreg-full.txt:3866,3970,…`, `40-…`, `50-…`, 72, 73, 74.
