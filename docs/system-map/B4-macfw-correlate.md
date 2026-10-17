# B4 — macfw cross-reference: subsystem coverage sanity check

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](PROVENANCE-RESOLUTIONS.md)).
> - **Stream kick** in the reference is TX start → 500 ms → INPUT 0x19 → 500 ms → RX start → OUTPUT 0x18 (`ConnectAudioMIDIChannels @0x12156`, asm 0x121c9..0x122bb); the 100 ms order is snd-bebob `bebob_maudio.c:312-337` only. Third-party/macfw statements that 'the stream-kick we locked is hardware truth' are **third-party-only** evidence and are contradicted by the kext order; hardware effect of the 500 ms gaps is **unverified** (check = SL-rig timestamped FCP capture).


> **Correction (2026-10-14) — `0xBA9` / `0xBE9` are not AV/C opcodes.** They are Itanium-ABI virtual member-function pointers (vtable byte offset + 1) stored in batch functors by `SetupBatchChangeMethod @ 0xe184`. `0xBA9` → FW1814 `vt[0xba8]` = `CustomMixerWriteQuads @ 0x22c7a`, a direct async block write to `0xFFC7:0x00700000 + offset`. `0xBE9` → `vt[0xbe8]` = `FW1814Device::AVCControlSetLEDStatus` (A/B-switch LEDs). There is **one** mixer write path (direct register writes, same window as FFADO/macfw/Rust). Group offsets `0x00/0x08/0x10/0x34/0x38` match FFADO `special_avdevice.h:55–69`. Full decode: `A1-ghidra/Q15f-mixer-control-plane-wire.md` Correction 2026-10-14. Any "AV/C batch opcode 0xBA9" or "two mixer paths" statement below is superseded.

> **Correction (2026-10-14) — DCL ring shape.** Per `A1-ghidra/Q-DCL-geometry-table.md` (raw disassembly): **TX** = 4 banks × (4 one-packet primer segments + 1 large segment of `P−4` packets that carries the callback) + 4 jumps = **20 segments** (`BuildTransmittingDCLProgram @ 0x25ae4`; `P = *(this+0x4e0)`, per rate). **RX** = **64 segments × 4 packets** + 64 jumps (`BuildReceivingDCLProgram @ 0x25962`; `packetCount` immediate `$0x4` at `0x25a99`, fixed at all rates). Any "64-segment TX" statement below is wrong; RX 64-segment statements stand. Timing derived from "64 segments × ~1 ms" must be recomputed: 4 packets = 4 isoch cycles ≈ 500 µs per RX segment.

**Source material:** `_refs/macfw/` (clone at `0.05.000` alpha, user-space Intel macOS 12–15 target) + Part A docs under `docs/system-map/A1-ghidra/`, `A2-panel/`, `A3-midi/`, `A4-helper/`, `A5-firmware.md`, `A6-prefpane.md`.

**Scope:** subsystem-by-subsystem check — does macfw have an equivalent of each `_extracted/` kext subsystem, and does its model agree with our Ghidra decode? Where they disagree, `_extracted/` wins for our 10.13.6 clone (per CLAUDE.md "mirror `_extracted/`, don't invent" and `memory/feedback_ffado_bebob_macfw_are_parallel_rev_eng.md`).

**Fundamental caveat.** macfw is a **greenfield user-space rewrite targeting macOS 12.7.6+ (Monterey) through 15.x (Sequoia)**, built on `IOFireWireLib` NuDCL + `AudioServerPlugIn` HAL (`_refs/macfw/README.md:63-78`, `COMPATIBILITY.md:19-27`). It explicitly does **not** port the 2014 M-Audio kext (`README.md:64`: "The project does not port the original M-Audio kernel extension"). Our target is 10.13.6 only, with a kext architecture that the 2014 driver already uses. So many "disagreements" are platform divergences, not evidence errors. The useful signal is: **does macfw agree with `_extracted/` on the wire-level protocol facts?** Where it does, that's a cross-check. Where it doesn't, the question is which side we trust for which claim.

## Status / next step

Scaffolded + populated 2026-10-13. Highest-signal findings:

1. **macfw bypasses the HSCI vendor envelope entirely.** The entire control plane uses direct FFADO-documented memory-mapped register writes at `0xffc7:0x00700000+` (see §5). Our A1-Q6 locked HSCI as `0xffc7:0x00600000` poll + 12-byte envelope through FCP (`A1-ghidra/Q6-fwhsci.md:9`). These are **two different control planes on the same device**, both FFADO-cited. See §3 for implications.
2. **macfw uses NuDCL / IOFireWireLib userspace isoch.** Explicitly the thing CLAUDE.md says we cannot use from kernel on 10.13.6 (NuDCL `createLocalIsochPort` returns `kIOReturnNoResources` from kernel per `A1-ghidra/Q1b-createlocaliosochport-handoff.md:9`). macfw's architecture is thus the *inverse* of ours — correct for its 2026 user-space HAL target, not applicable to our 10.13.6 kext target.
3. **macfw does not upload firmware.** It only sends the boot-from-flash cue (`_refs/macfw/devices/fw1814/tools/fwboot1814.cpp:245-259`). The actual `fw1814.bcd` upload path belongs to the 2014 M-Audio installer flow that we reproduce in `openaudio-fw1814/firmware-helper/` (A4-complete). See §7.
4. **macfw's wire protocol facts (rate reassert, blocking CIP, S/PDIF channel map, boot cue bytes) all agree with FFADO/snd-bebob — i.e. with our A4/A5 and the stream-kick memory `project_fw1814_stream_kick.md`.**
5. **Mixer gap between A1-Q15 and macfw is explainable:** macfw writes raw FFADO registers directly; the 2014 kext wraps the same registers inside the `FWChannelGroupProperty` + `TFunctor` opcode `0xBA9` AV/C vendor-dependent batch (A1-Q15 §Correction + Q15b). The register map underneath is almost certainly the same.

Next: feed §3, §5, §6 into Part C — in particular, decide whether the clone's control plane goes through the AV/C vendor envelope (reference-faithful, mirrors `_extracted/`) or raw FFADO register writes (what every modern driver does). The reference-faithful route is clearly §5's recommendation given CLAUDE.md's "mirror `_extracted/`" rule.

---

## 1. Isoch stack (vs A1-Q1, Q1a, Q1b)

**macfw:** `IOFireWireLib` NuDCL from userspace via `(*pool)->AllocateReceivePacket` / `AllocateSendPacket` / `(*device)->CreateLocalIsochPort`:

- `_refs/macfw/devices/fw410/lib/src/amdtp_receive_ring.cpp:84` — `AllocateReceivePacket(ring.pool_, updateSet, 4, 2, ranges)`.
- `_refs/macfw/devices/fw410/lib/src/amdtp_receive_ring.cpp:119` — `CreateLocalIsochPort(native, false, program, ...)`.
- `_refs/macfw/devices/fw410/lib/src/amdtp_transmit_ring.cpp:83,100` — same, talking=true.
- `_refs/macfw/devices/fw410/lib/src/amdtp_nodata_transmit_ring.cpp:80,96` — NODATA transmit path.

FW1814 reuses the FW410 lib: `_refs/macfw/devices/fw1814/transport/duplex_lifecycle.h:52-58` builds `IsochAllocation`s and attaches them. The RX side feeds a configurable NuDCL depth; the TX side runs rolling-TX or fixed half-ring refill.

**`_extracted/` kext (A1-Q1):** classic DCL `IODCLProgram` with **20-segment (4-bank) TX + 64-segment × 4-packet RX (Q-DCL)**, each segment emitting `AddTimestampedTransferPackets` (small timestamped xfer runs + bulk xfer + explicit `DCLPtrTimeStamp` node + `callproc` + `jump`) — `A1-ghidra/Q1-rx-dcl-callbacks.md:11,20-39`. Q1b @ `A1-ghidra/Q1b-createlocaliosochport-handoff.md:9` confirms the kext calls `IOFireWireController::createLocalIsochPort` directly from kernel with raw DCL head pointer.

**Agree / disagree:** **structural disagree, by platform constraint.** macfw NuDCL vs reference classic DCL. CLAUDE.md: "Do not switch to `IOFWDCLPool` / NuDCL — on 10.13.6 `createLocalIsochPort` returns `kIOReturnNoResources` from kernel space." macfw's NuDCL depth (`captureMaxPacket=360`, `playbackMaxPacket=232` at 48 kHz S/PDIF — `_refs/macfw/devices/fw1814/transport/analog48_main.cpp:34-35` and `special-firmware-notes.md:109-114`) is a userspace-driven packet-ring size, not comparable to our 64-segment DCL ring shape. **Recommend: `_extracted/`** — mirror classic DCL per A1-Q1 + auto-memory `reference_dcl_structure_authoritative.md`.

**Useful cross-check:** macfw's blocking-CIP packet sizes (`232/360` bytes at 48 kHz / S/PDIF — 11 PCM+1 MIDI capture, 7 PCM+1 MIDI playback, 8 events per data-bearing packet) are protocol-level facts and must match our kext's packet-pool sizing. Cross-reference against A1-Q17 `TPacketPool::Initialize` buffer math.

---

## 2. CMP / PCR plug map (vs A1-Q3)

**macfw:** reads oPCR[0] + iPCR[0] exclusively — `_refs/macfw/devices/fw1814/transport/duplex_lifecycle.h:43-45` (`cmp::readOpcr0` + `cmp::readIpcr0`). The special-firmware notes confirm oMPR plugs=2, iMPR plugs=3 as the hardware ground truth: `_refs/macfw/devices/fw1814/analysis/special-firmware-notes.md:16-20`:

```
oMPR:    0xbfff0002  plugs=2
iMPR:    0x80ff0003  plugs=3
```

Lifecycle order: `playback.Start()` (host→device, iPCR[0]) **before** `capture.Start()` (device→host, oPCR[0]) — `_refs/macfw/devices/fw1814/transport/duplex_lifecycle.h:106-116`. Comment at `:106`: "Hardware-proven FW1814 start order: host->device playback first, device->host capture second."

**`_extracted/` kext (A1-Q3):** `ConnectAllPlugs` 3-step gate: `SufficientBandwidthAvailable` → `ConnectAudioMIDIChannels` → set `fConnected` (`A1-ghidra/Q3-setupaudioengine-connectallplugs.md:9`). Plug map is 2 oPCRs + 3 iPCRs with plug 0 = PCM — matches auto-memory `project_fw1814_plugs.md`.

**Agree / disagree:** **agree.** oMPR plugs=2, iMPR plugs=3, PCM on plug 0 — cross-verified against hardware by macfw on 2026-09-06 (`profile.h:14`). Start order (playback-before-capture) agrees with Linux `snd_bebob_stream_start_duplex` (`special-firmware-notes.md:69-88`). macfw additionally confirms the **AV/C stream-kick after CMP connect** that our auto-memory `project_fw1814_stream_kick.md` already records: `special-firmware-notes.md:85-87`: "Linux programs device OUTPUT first, waits 100 ms, then programs device INPUT because the second command is otherwise prone to failure" + "after the CMP connections and AMDTP domain are started, Linux re-applies the current sample rate."

**No `_extracted/` disagreement.** macfw is a clean independent confirmation that the plug map + stream-kick we locked are hardware truth.

---

## 3. HSCI vendor envelope (vs A1-Q6)

**macfw:** **no HSCI envelope anywhere.** `grep -r 'HSCI\|00600000'` across `_refs/macfw/devices/fw1814/` returns zero hits. Rate control is standard AV/C INPUT/OUTPUT PLUG SIGNAL FORMAT via `_refs/macfw/devices/fw1814/fcp_control.h:90-110` (`setSignalRate(rate, opcode=0x18|0x19)`), identical to Linux `avc_general_set_sig_fmt`. The 8-byte CONTROL block is:

```c
const UInt8 command[8] = {
    0x00, 0xff, opcode, 0x00, 0x90,
    static_cast<UInt8>(sfc), 0xff, 0xff
};
```

Clock-source / digital-mode / monitoring state are kept in **software** on the macfw side with no device readback — confirmed by `special-firmware-notes.md:76-83` quoting Linux `snd-bebob`:
> "Digital input/output format and clock-source state are maintained in software because the driver is not allowed to query these parameters safely."

**`_extracted/` kext (A1-Q6):** `m_audio_b_FWHSCI` is a **repeating-timer heartbeat** polling `0xFFC7_00600000` for N quadlets every ~50 ms (`A1-ghidra/Q6-fwhsci.md:9,54-60`). Response gets parsed into a 12-byte status envelope. The envelope's `company_id = 0x040004` is used elsewhere by the kext to issue **vendor-dependent AV/C writes** carrying things like clock-source select — see A1-Q5 §Dispatcher + mute-bit decode and auto-memory `project_fw1814_hsci_envelope.md`.

**Agree / disagree:** **both approaches describe the same device from different angles.** macfw does not touch HSCI at all; the 2014 kext uses HSCI as a periodic status poll (meter / rate-lock readback — `A1-ghidra/Q6-fwhsci.md:11` "ships the 12-byte status envelope to `HSCIReadComplete`"). The two coexist because:

- macfw reads peak meters out-of-band via a different mechanism (the 84-byte meter frame — see §5).
- macfw never reads back clock-source — it tracks it in software, same as Linux.
- Clock-source *writes* in the 2014 kext go through `SetClockSource @ 0x23858` → `SetClockSourceInternal @ 0xe25c` → HSCI envelope + vendor AV/C (A1-Q5 §Dispatcher + mute-bit decode). macfw doesn't change clock source yet — digital I/O is explicitly deferred (`_refs/macfw/README.md:131` and `devices/fw1814/README.md:84-86` "S/PDIF, ADAT and MIDI are intentionally deferred").

**No direct contradiction.** `_extracted/` wins for the clone — our control plane must drive clock source via the composite-bitfield MAClockSource key through `IORegistryEntrySetCFProperties` (A1-Q15b + auto-memory `project_maclocksource_is_composite_bitfield.md`). macfw's "write FFADO register directly" shortcut skips the kext's `FWPropertyDict` dispatcher entirely because macfw *is* the driver with no kext in the picture.

**Caveat.** If we ever discover the HSCI polling isn't strictly needed on 10.13.6 (e.g. because the Panel-side peak-meter path doesn't require it — see B5 §10 meter path), we can trim it. But A1-Q6 shows the heartbeat is wired into the engine's workloop and status reports; defer any such trim until Part C analyzes the full status readback graph.

---

## 4. Clock sources (vs A1-Q5)

**macfw:** **not implemented.** The code establishes "the known-safe M-Audio baseline explicitly: internal clock + S/PDIF input/output + unlocked clock controls" (`special-firmware-notes.md:131`) and never changes it. Digital-mode selection would be a Linux `snd-bebob` vendor-dependent CONTROL (`special-firmware-notes.md:78-82`):

> clock source `0x03` = internal; digital input format `0x00` = S/PDIF; digital output format `0x00` = S/PDIF; clock lock `0x00` = unlocked.

Note the Linux encoding here: **clock source `0x03` = internal.** This matches our A1-Q5 locked bijection: `HSCI wire byte 6 = 3` → "Internal — digital inputs unmuted" (`A1-ghidra/Q5-clock-source.md:12-14`). Independent confirmation.

**`_extracted/` kext (A1-Q5):** full 4-radio + composite-bitfield decode — Internal unmuted / External digital / Word Clock / Internal muted, with bit 4 (optical-OUT), bit 5 (S/MUX), bit 2 (S/PDIF-port), bit 30 (gate). All wire-byte / HSCI-envelope mappings locked.

**Agree / disagree:** **agree on the one point macfw commits to.** macfw's "0x03 = internal" is byte 6 of a vendor-dependent CONTROL, exactly our A1-Q5 wire-byte position. macfw does not touch the other three Sync Source radios, bit 4/5/30, or the mute bit. **`_extracted/` wins** on everything beyond the Internal/S/PDIF default — macfw simply doesn't have it yet.

**Useful implication for Part C:** A1-Q5's bijection is cross-validated at one point by macfw and by Linux `snd-bebob`. The remaining 3 Sync Source radios are only in `_extracted/` + Panel P7-fup-translator + B5 §10 dtrace. That's three independent sources agreeing on the one byte macfw touches — good enough to treat the full bijection as locked.

---

## 5. Mixer model (vs A1-Q15 + A2-P3)

This is the **highest-signal section.** macfw's mixer code is the most complete independent FW1814 mixer decode outside FFADO itself.

**macfw register map** (`_refs/macfw/devices/fw1814/special_mixer.h:14-39`) — all at `0xffc7:0x00700000+`, **write-only**:

| Lo address | Symbol | Role |
|---|---|---|
| `0x00700000` | `GAIN_STM_12_IN` | SW-return 1/2 level |
| `0x00700004` | `GAIN_STM_34_IN` | SW-return 3/4 level |
| `0x00700008` | `GAIN_ANA_12_OUT` | analog out 1/2 level |
| `0x0070000c` | `GAIN_ANA_34_OUT` | analog out 3/4 level |
| `0x00700010..0x0070001c` | `GAIN_ANA_{12,34,56,78}_IN` | analog in monitor levels |
| `0x00700034` | `GAIN_AUX_OUT` | aux master |
| `0x00700038..0x0070003c` | `GAIN_HP_{1,2}_OUT` | headphone 1/2 level |
| `0x00700040..0x0070004c` | `LR_ANA_{12,34,56,78}_IN` | per-pair pan/balance |
| `0x00700064..0x00700078` | `AUX_{STM,ANA}_*` | aux send levels |
| `0x00700090` | `MIX_ANA_DIG_IN` | 8-bit route-mask (analog→mixer) |
| `0x00700094` | `MIX_STM_IN` | 4-bit route-mask (streams→mixer) |
| `0x00700098` | `SRC_HP_OUT` | HP source selector (Mixer12/Mixer34/Aux12 per HP) |
| `0x0070009c` | `SRC_ANA_OUT` | analog out source selector (Mixer vs Aux) |

Level encoding: AV/C signed 8.8 dB stereo-packed `(L << 16) | R`; `0x8000` = mute, `0x0000` = unity (`special_mixer_model.h:16-62`). This is **FFADO's documented M-Audio special-firmware mixer area** (`special_mixer.h:13`).

**`_extracted/` kext (A1-Q15 + Q15b + A2-P3):** completely different *shape* on top of what is almost certainly the same underlying device. The 2014 kext wraps all of these in a C++ class graph:

- `FWSettings1814Levels` (≥ 0xa50 bytes) and `FWSettings1814AuxSend` (≥ 0x2c0 bytes) — typed-property dicts (`A1-ghidra/Q15-mixer-model.md:14-50`).
- `m_audio_b_FWChannelGroupProperty` (0x78 bytes each) binds `(key, channelGroup, channelMask, optionalSecondGroup)` to a per-group `TFunctor` apply hook.
- Writes route via the generic AV/C vendor-dependent writer `FW1814Device.vtable[0xba8](this, selector, 1, &register_field)` with selector bytes `0x90` (routing), `0x9c` (aux), `0x3e..0x60` (pan) — A1-Q15 §Correction @ `Q15-mixer-model.md:5`.
- Panel-facing surface is the 46 MA* property keys the panel sets through `IORegistryEntrySetCFProperties` (`A2-panel/P3-property-key-strings.md:11-33`), which the kext coalesces + batches via `FWPropertyDict::SetProperties @ 0xa0e0` (A1-Q15b) + opcode `0xBA9` TFunctor delta apply.

**Agree / disagree — this needs care:**

- **Register addresses:** macfw says `0xffc7:0x00700000+`. Our A1-Q15 decode gives selector bytes (`0x90`, `0x9c`) and opcode `0xBA9` for the vendor-dependent AV/C envelope. These are **different control planes on the same hardware** per the Linux snd-bebob + FFADO precedent. **unverified** — I have not read the specific decompile of the generic AV/C vendor-dependent writer at `vtable[0xba8]` to confirm whether, under the AV/C vendor-dependent envelope, the kext ultimately targets the same `0xffc7:0x00700000+` physical register space, OR a disjoint register space on `0xffc7:0x00500000..0x00600000`. **Verification path:** decompile `FW1814Device.vtable[0xba8]` + the opcode `0xBA9` TFunctor delta applier (`Q15c-changerate-performformat.md` as the entry point), and compare the resulting FireWire write address against macfw's `0x00700000+` map. If the kext's final write hits `0x00700000+`, A1-Q15's AV/C envelope is a protocol wrapper around the same registers macfw writes raw. If it hits elsewhere, there are two parallel control planes and §3 generalizes.
- **Shape:** macfw's register-level direct writes + locally cached mixer state (because registers are write-only — `special_mixer.h:13`, `devices/fw1814/README.md:126` "cannot be queried safely") is **compatible** with the kext's cache + delta apply. Both sides maintain software truth and only write deltas. The difference is the kext embeds that cache + delta in `FWPropertyDict`, while macfw embeds it in its transport-process routing-control API (`devices/fw1814/README.md:124-183`).
- **Panel surface:** macfw exposes 44 CLI verbs (`devices/fw1814/README.md:128-183`) which correspond fairly well to the 46 MA* panel property keys in A2-P3 — e.g. `mixer-route set`, `output-source set`, `output-volume set`, `headphone-source set`, `aux-send-level set` map onto `MASWReturnRoutingV2`, `MAOutputRoutingV2`, `MAOutputLevels`, `MAHeadphoneSourceV2`, `MAAuxSendLevels` (A2-P3 table). The property model survives translation to a different IPC transport.

**Recommend:** `_extracted/` wins for the shipping clone — use the `FWPropertyDict` + `MA*` keys surface (reference-faithful; mirrors the 2014 Panel RPC exactly). Treat macfw as **high-value cross-check for the register map** when the time comes to validate what the kext's AV/C vendor-dependent writer actually emits on the wire. The B5 §10 DTrace of the Panel setProperties flow should produce the raw bytes we can compare against macfw's register writes.

**Specific gaps where macfw is more complete than our current A1 docs:**

1. **Explicit MIX_STM_IN bitmask layout** — `special_mixer_model.h:150-161`:
   ```
   bit 3: Stream 1/2 -> Mixer 3/4
   bit 2: Stream 1/2 -> Mixer 1/2
   bit 1: Stream 3/4 -> Mixer 3/4
   bit 0: Stream 3/4 -> Mixer 1/2
   ```
2. **MIX_ANA_DIG_IN bit layout** — `special_mixer_model.h:163-174` (bits 0..3 = analog pair → Mixer 1/2; bits 4..7 = analog pair → Mixer 3/4).
3. **Pan encoding** — `special_mixer_model.h:20-24`: `0x7ffe` = hard left, `0x0000` = center, `0x8000` = hard right (reserves `+32767`; hard-left endpoint is `+32766`).
4. **Headphone source** — `special_mixer_model.h:97-108`: Mixer12 = `0x01`, Mixer34 = `0x02`, Aux12 = `0x04`, packed as `(hp1 << 0) | (hp2 << 16)`.
5. **Analog-out source** — same file §analogOutputSource: bit 0 = pair 1/2 Mixer vs Aux, bit 1 = pair 3/4.

A1-Q15b should borrow these when the Panel→kext bytes-on-the-wire are decoded.

---

## 6. Userclient surface (vs A1-Q11 + Q14)

**macfw:** no user-client at all — macfw's architecture is a userland transport daemon with a Unix socket at `/tmp/macfw-fw1814-control.sock` (`devices/fw1814/README.md:124`) + CoreAudio shared-memory ring (`macfw_fw1814_hal_shm.h:9` → `/macfw_fw1814_pcm_v1`). All device access is via `IOFireWireLib` directly from the daemon.

**`_extracted/` kext (A1-Q11 + Q14):**

- `FWUserClient` — 8 selectors 500..507 (MIDI start/stop/stop-alias/send, DiceSet/Get, ProToolsOpen no-op, ProToolsErrorReport no-op). ProTools entries are vestigial 2008-era stubs that return 0 (`A1-ghidra/Q11-userclient-selectors.md:9`).
- `FWKIPCUserClient` — Mach-port async notification-only, 44-byte message; type-0 port carries props (body 0) + IOKit message codes, type-1 port carries MIDI-in (≤320 B); no peak/rate-lock types (corrected 2026-10-14, A1 Q-UC-surface §4). No external method dispatch. Pure "poke the panel, come re-read" (`A1-ghidra/Q14-fwkipcuserclient.md:9`).

**Agree / disagree:** **structurally disjoint** — macfw has no kext → no user-client. There is nothing in macfw to cross-check A1-Q11/Q14 against. **`_extracted/` wins by default.**

**Transferable observation:** macfw's control-socket + shared-memory split (control via Unix socket, audio via SHM) is architecturally parallel to our kext's `FWUserClient` (RPC) + `FWKIPCUserClient` (notifications) + `com_m_audio_FWUserClient clientMemoryForType(1)` shared ring for peak meters (`project_meter_path_shared_ring.md`). The parallel suggests the IPC split is a natural shape for this device class, not a 2014 artifact.

---

## 7. Firmware upload (vs A4)

**macfw:** does **not** upload `fw1814.bcd`. The entire firmware handling is `fwboot1814.cpp` — a one-shot "send the documented boot-from-flash cue":

`_refs/macfw/devices/fw1814/tools/fwboot1814.cpp:245-259`:

```c
const UInt8 cue[12] = {
    0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x01,
    0x00, 0x00, 0x00, 0x00,
};
// write to 0xffff:0xc8021000
```

Preflight at `:195-220` reads the 0x68-byte bootloader info block at `0xffff:0xc8020000` and matches:

- `manufacturer == "bridgeCo"`
- `protocol version == 1`
- `bootloader version == 0x00002805`
- `hardware model == 0x00000083`
- `hardware revision == 0x00000001`
- `software date == "20070713"` (ASCII — matches our A5 BCD date parsing finding at `A4-helper/H9-bcd-reader.md:6`)
- `software id == 0`
- `image base == 0x20080000`
- `image max == 0x00180000`
- `bootloader date == "20040330"`

Then it sends the cue. macfw explicitly **assumes the device has already been flashed with operational firmware by the vendor driver or by someone else.** From `_refs/macfw/README.md:149-156`: upload is not a project goal; the project assumes an already-flashed interface.

**`_extracted/` kext + A4 helper:** the 2014 M-Audio installer actually uploads `fw1814.bcd` via a 3-step `MAFirmwareHalt` → `MAFirmwareUpgradeImage` → `MAFirmwareStart` CFProperty handshake (`A4-helper/H2-bootloader-detection.md`), reading only header date + CRC range + expected CRC32 from the file (`A4-helper/H9-bcd-reader.md:14-15`). The payload is mmap'd and handed opaque to the kext via `setProperty:@"MAFirmwareUpgradeImage"`. Our `openaudio-fw1814/firmware-helper/` reproduces this A4-complete and survives Part D (per CLAUDE.md).

**Agree / disagree:** **agree on the handshake endpoint** — macfw's info-block read at `0xffff:0xc8020000` and cue write at `0xffff:0xc8021000` are both inside the same bootloader CSR window the 2014 helper uses. macfw stops where the 2014 helper starts (chunk-wise upload). The one byte-level cross-check: macfw's cue bytes `01 00 00 00 00 00 11 01 00 00 00 00` must appear somewhere in the 2014 kext's bootloader-mode handling. **Verification path:** grep the `_extracted/kext` Ghidra export for the 12-byte sequence or for the `0xc8021000` address literal.

**Observation.** The macfw `fwboot1814.cpp` preflight gives us **free ground-truth values for the 10-field bootloader info block**: image base `0x20080000`, image max `0x00180000` = 1.5 MB. These should match whatever our A4 helper computes from `fw1814.bcd` header offsets `0x30/0x38`. **Verification path:** load `_extracted/firmware/fw1814.bcd` and verify header `0x30` → `0x20080000` and header `0x38 - 0x30` ≤ `0x00180000`. If matches, A5 header decode is cross-validated against hardware.

---

## 8. Control-panel UX scope (vs A2)

**macfw:** AppKit control panel with 7 tabs — Mixer / Outputs / Headphones / AUX / Inputs / Device / Info (`_refs/macfw/README.md:112-123`). FW1814 scope specifically covers: software returns 1/2 + 3/4, four analog input pairs, two analog output pairs, two digital-volume headphone outputs, aux sends/master, persistent state, 44.1/48/88.2/96 kHz selection (+ experimental 176.4/192), performance-profile selector (Aggressive 250 µs / Balanced 375 µs / Conservative 500 µs — `devices/fw1814/README.md:192-199`). Digital I/O (S/PDIF, ADAT) and MIDI deferred.

The panel invokes `fw1814ctl` subprocesses as its backend (`_refs/macfw/README.md:85` "The GUI currently invokes `fw410ctl` subprocesses as its proven backend boundary" — same architecture used for FW1814).

**`_extracted/` panel (A2):** Carbon-era Nib-driven panel. Hardware/Mixer/Device-Settings/Rate-and-Buffer/HPAB-Source/AC3/Peak-Meters tabs (`A2-panel/P5..P11`). Control plane is 46 MA* keys via `IORegistryEntrySetCFProperties` + peak meters via vendor UC shared ring (`A2-panel/P3-property-key-strings.md` + auto-memory `project_meter_path_shared_ring.md`). Full digital I/O support: S/PDIF, ADAT, Word Clock routing, SMUX, direct-monitor.

**Agree / disagree:** **structurally disagree, by scope.**

- macfw ships **analog-only** FW1814 and defers digital — our `_extracted/` panel is a full-featured product-era GUI.
- macfw's performance-profile selector (250/375/500 µs service period) is a **macfw-specific concept with no equivalent in A2** — it's a user-space Mach-paced service thread knob irrelevant to a kext whose DCL ring IRQs are driven by OHCI segment callbacks.
- macfw's control-socket-based IPC is architecturally different from the 2014 panel's setProperties-based control plane (A2-P3 + B5 §10).
- Peak meters: both have them; macfw uses the 84-byte Linux-documented meter frame (`transport/headphone_rotaries.h:7-9` "bytes 1 and 2 of the 84-byte meter block"); A2-P9 + B5 §10 use the vendor UC shared ring with cap 70.

**Recommend:** `_extracted/` wins. For the clone, the mandatory panel features are the 46 MA* key set and the S/PDIF/ADAT/Word Clock/SMUX surface that macfw deliberately omits. The macfw UX is a useful **UX reference** for a future simplified panel (e.g. the "two headphones, aux, four analog pairs" baseline), but it is a subset of what we must ship for feature parity.

**Transferable detail.** Headphone encoders: macfw's `headphone_rotaries.h:7-27` confirms the two headphone encoders expose +1/-1 steps in bytes 1 and 2 of the meter frame — this matches auto-memory `project_fw1814_front_knobs_are_encoders.md` and gives us the exact byte offsets.

---

## 9. Build artifacts, bundle identifiers, platform gating (vs A6 + 10.13 kextutil path)

**macfw** build artifacts:

- LaunchDaemon plist label `com.mbprado.macfw.fw1814.transport` (`service/com.mbprado.macfw.fw1814.transport.plist:5`); binary at `/Library/Application Support/macfw/fw1814/bin/fw1814supervisor`; `KeepAlive`+`RunAtLoad`+`ThrottleInterval=5`; `ProcessType=Interactive`.
- CoreAudio device UID `com.mbprado.macfw.fw1814.device` (`control-panel/Sources/main.mm:9`).
- Playback SHM name `/macfw_fw1814_pcm_v1` with magic `M18O` (`hal/include/macfw_fw1814_hal_shm.h:9-13`).
- No kext; `AudioServerPlugIn` HAL. No signing, no notarization in current alpha (`KNOWN-LIMITATIONS.md:100-102`).

**Platform gating.** macfw explicitly targets Monterey (12.7.6) + Ventura (13.7.8) + Sonoma (14.8.9) + Sequoia (15.x) — `COMPATIBILITY.md:19-27`. Not validated on 10.13 at all. Not supported on Tahoe 26 due to removed FireWire stack (`README.md:55-57`). Not supported on Apple Silicon.

Code inspection: no `__MAC_OS_X_VERSION_MIN_REQUIRED` gates visible in the files surveyed; macfw relies on `IOFireWireLib` + `AudioServerPlugIn` which are both available on 10.13 but macfw's architecture (NuDCL + HAL plug-in) uses patterns the 2014 M-Audio stack deliberately avoided.

**`_extracted/` + A6 + CLAUDE.md:** we ship a signed `kext` (B3 §6: Developer ID + SHA-1 CMS was fine in 2014, but 10.13.4 needs SHA-256 CMS + UAKEL approval + Team ID), a Carbon-replaced AppKit panel, and the firmware helper LaunchDaemon. Bundle identifier is `com.openaudio.driver.fw1814` (CLAUDE.md "build PC state 2026-10-05"). No shared SHM name space with macfw.

**Agree / disagree:** **nothing to agree on at this layer** — different reverse-flow (userland HAL vs kext) and different macOS target. macfw does not inform our 10.13.6 kextutil path. **`_extracted/` wins.**

**Useful crumb for Part C/D.** macfw proves the modern path (user-space HAL plug-in via `IOFireWireLib` NuDCL) is feature-complete on this hardware class — if we ever port to 10.14+ (after this 10.13.6 release), macfw is the reference architecture, not the 2014 kext. Record for post-D roadmap.

---

## 10. Oddities worth citing

1. **macfw's device profile uses `unitSpecifierId = 0x0000a02d`, `unitSwVersion = 0x00014001`** (`_refs/macfw/devices/fw1814/profile.h:22-23,30-31`) — matched by product-name string. This is a **different matching key** than our kext (Vendor_ID `0x000D6C` + Model_ID `0x10071` runtime / `0x10070` bootloader — auto-memory `project_fw1814_ids.md`). Both match the same device because the Config ROM carries all four fields. macfw's choice avoids depending on the vendor-specific Model_ID (which can drift across firmware revisions); the 2014 kext uses Model_ID because its personality catalog is indexed by it. **No disagreement**, but note the dual key space when writing B5 ioreg truth.
2. **macfw hardware fingerprint claim.** `fwboot1814.cpp:216` asserts hardware model `0x00000083`. **Unverified** against anything in our A5/A6 decode. **Verification path:** check whether `_extracted/firmware/fw1814.bcd` header or the operational `0x10071` runtime Config ROM carries this value.
3. **CIP_BLOCKING with 8-event packets at 48 kHz** — macfw's packet sizing is 8 events per data-bearing packet, NODATA every 4th cycle (`special-firmware-notes.md:110-125`). This is a protocol-level fact our A1-Q17 `TPacketPool::Initialize` sizing must already match. **Verification path:** cross-check A1-Q17 computed buffer size against macfw's `232 bytes playback / 360 bytes capture` at 48 kHz S/PDIF.
4. **No ADAT formation implemented or verified by macfw.** Its channel map (`channel_map.h:1-62`) is 48-kHz / S/PDIF-mode only (10 PCM + 1 MIDI capture, 6 PCM + 1 MIDI playback). ADAT mode's 16 PCM + 1 MIDI capture + 12 PCM + 1 MIDI playback (our A1-Q3 § `fMode & 0x10`) is in Linux `snd-bebob` tables but not exercised by macfw. **No cross-check available** for the ADAT path; we must rely on A1-Q3 + FFADO/snd-bebob for Part C.

---

## Candidate A1 gaps (things macfw has that A1 doesn't currently cover)

1. **Full FFADO register map at `0xffc7:0x00700000+`** — we have A1-Q15 class graph but no "which register the AV/C vendor-dependent writer ultimately targets" decompile. Open as **A1-Q15e** (or extend Q15b): decompile `FW1814Device.vtable[0xba8]` (the AV/C vendor-dependent primitive) and the opcode `0xBA9` TFunctor applier to resolve the final FireWire write address, then cross-check against macfw's `0x00700000..0x0070009c` map.
2. **Hardware model + bootloader info block** — the 0x68-byte block at `0xffff:0xc8020000` is read by both the 2014 helper (A4-H2) and by macfw (`fwboot1814.cpp:217`), but our A5 doc covers the `.bcd` file layout, not this device-side info block. Open as **A5-follow-up** or **A4-H2a**: capture + decode the 0x68 bytes from a live FW1814 and cross-check against macfw's 10 preflight assertions.
3. **84-byte meter frame layout** — macfw records bytes 1, 2 as the two headphone-encoder transitions (`headphone_rotaries.h:7-9`) but doesn't document the other 82 bytes. B5 §10 locked meter-path = vendor UC `clientMemoryForType(1)` cap 70 shared ring; add macfw's frame-byte semantics to the B5 meter-frame layout queue.
4. **CIP_BLOCKING packet geometry per rate** — macfw has validated 44.1/48/88.2/96/176.4/192 packet-size cells across its six analog_main.cpp variants. Our A1-Q17 covers `TPacketPool::Initialize` at a low level but we don't yet have a per-rate `(DBS, max_packet_bytes, nodata_cadence)` cross-table. Low priority — can be synthesized at Part C synthesis time if needed.

## Implications for Part C / D

1. **The clone's control plane goes through MA* + `FWPropertyDict`, not raw FFADO registers.** This is already the CLAUDE.md baseline, but §5 above confirms it unambiguously — macfw's register-write shortcut skips the kext entirely and is only possible because macfw ships no kext. Our clone's `com.openaudio.driver.fw1814` kext must mirror the 2014 kext's `FWPropertyDict` dispatcher + opcode `0xBA9` delta apply (A1-Q15 Correction + Q15b).
2. **macfw's wire-protocol facts cross-validate FFADO + snd-bebob.** The stream-kick memory, the 44.1-vs-48 CIP_BLOCKING cadence, the plug map, the clock-source wire byte for Internal, and the boot-from-flash cue address are all independently confirmed by macfw. This hardens Part C synthesis: anything with 2+ sources (`_extracted/` + FFADO/snd-bebob/macfw) is treated as locked.
3. **Part D "modified reference vs. clone" trade-off is unchanged.** macfw's existence doesn't bias toward "modified reference" or "clone from scratch" — it's a third architecture (userland HAL) that we are not pursuing on 10.13.6. The B3 §6 breakers (UAKEL / SHA-256 CMS / IOAudio+IOFireWire reserved-slot drift) stand; macfw neither helps nor hurts.
4. **Post-D roadmap gift.** When we eventually port to 10.14+ (Catalina AudioServerPlugIn era onwards), macfw is the reference architecture. For now, treat it as a parallel project with different platform constraints and a different control-plane choice, not as a shipping target.
5. **The 46 MA* keys are the panel surface we must ship.** macfw's 44-verb CLI is a close-but-not-identical subset. Where they overlap, macfw's semantics (what each verb writes to which register) can inform our panel-side setProperties encoding. Where macfw omits (clock-source dropdown, digital-mode selection, SMUX, direct-monitor, Word Clock), we fall back to A1-Q5 + A2-P3 + B5 §10 for the authoritative definition.
