# FireWire Audiophile — Manual Reference (developer-oriented, Mac only)

> **Correction (2026-10-19, R79-R81):** §8 items 3 and 5 (mixer AV/C bytes, `MapChannelAndFunctionBlock` kinds, the aux row for stream 5/6 = function block `0x08`) are resolved in `docs/system-map/A1-ghidra/Q-AP0-avc-mixer.md`; item 10 (object layout) in `Q-AP0-nub-and-rev2.md` §4; the converters in `Q-AP0-converters.md`. The kext's factory reset sets output levels to 0 dB, not the manual's −6 dB (§5) — open.

> **Correction (2026-10-19, R78):** §8 items 6 (LED command) and 7 (polling) are resolved: LED = 16-byte vendor-dependent CONTROL `00 FF 00 00 0D 6C 02 00 01 <v>` (v 0 off / 1 A / 2 B), status packet read every 50 ms (1000 ms after an error). The §4.3 decode is confirmed in asm, with the encoder nibbles being detents (1 = up, 2 = down; low nibble = Level Controller, high = headphone). Details: `docs/system-map/A1-ghidra/Q-AP0-status-packet-front-panel.md`.

> **Correction (2026-10-19, R77):** §1 "Kext matching" is wrong where it says the device classes match the nub "by class alone": every device personality matches the nub's **`FireWire Product Name`** (`FW Audiophile`), and `FWMetaNub::probe` selects by vendor name + product name from `kDeviceSpecs` (runtime or bootloader name), **never by Model_ID**. §8 item 2 is resolved. §3.4 latencies confirmed in asm. The Audiophile's clock accepts only internal (`MAClockSource` bits[1:0] = 0) and S/PDIF (= 1); 2/3 are refused. Details: `docs/system-map/A1-ghidra/Q-AP0-nub-and-rev2.md`.


Companion to `docs/manual-reference.md` (FW1814). Source documents:

* **Manual**: `docs/audiophile manual.pdf` (57 pp., PDF title `MA_FWAP-EN-Mnl_Ver1-rev.doc`, 2004-06-16), transcribed verbatim in `docs/audiophile-manual.md`. Cited as `man p.N`.
* **Reference kext** `_extracted/kext/M-AudioFireWireBeBoB.kext` v1.11.2 (the spec for this project): `ghidra`/`otool` listings under `docs/system-map/A1-ghidra/`. Cited as `kext @0xADDR` (addresses of the x86_64 slice).
* **Firmware** `_extracted/helper/Firmware/FW Audiophile/FWAP.bcd`. Cited as `bcd`.
* **Parallel reverse engineering** (decodes opaque bytes only; the kext wins on conflict): FFADO `_refs/ffado/src/bebob/maudio/{fwap.xml,normal_avdevice.cpp}` (`ffado`), Linux `_refs/linux-snd-bebob/sound/firewire/bebob/{bebob.c,bebob_maudio.c}` (`snd-bebob`), Rust `_refs/snd-firewire-ctl-services/protocols/bebob/src/maudio/normal.rs` (`ctl-services`).

Rules as in the FW1814 reference: every fact cites its origin; anything not read in the binary or measured is **unverified** with the check that settles it. Nothing here was tested on an Audiophile (none is connected to this project's machines yet).

Scope: this is what a developer needs to *implement* the device, not how to use it. The usage narrative of the manual (DJ cueing, effects loops, OS 9/XP installers, OMS) is intentionally omitted; `docs/audiophile-manual.md` has it.

---

## 1. Identity and enumeration

| Item | Value | Source |
|---|---|---|
| Vendor_ID | `0x000D6C` (M-Audio / Midiman) | `bcd` config ROM `NodeVendorId`, `ModelVendorID` |
| **Model_ID (runtime)** | **`0x00010060`** (`ModelID` in root and unit directory) | `bcd` appendix `[ModelID … 0x00010060]`; `ffado` `bebob_avdevice.cpp:186`, `normal_avdevice.cpp:39`; `snd-bebob` `bebob.c:62 MODEL_MAUDIO_AUDIOPHILE_BOTH` |
| Model_ID (bootloader) | same `0x00010060` — the bootloader is told apart **by the unit text** `"FW Audiophile Bootloader"` (runtime text: `"FW Audiophile"`) | `snd-bebob` `bebob.c:150-157,196` |
| Hardware version / model | hardware version `0xD`; bootloader-info "model ID 0x00000d" | `bcd` `[HardwareVersion … 0xD]`; `ctl-services` `normal.rs:120-135` |
| GUID prefix | `0x000D6C03…` (the sample GUID in `ffado` `fwap.xml` is `0x000D6C031088FA68`) | `ffado` `fwap.xml` |
| Unit spec / SW version | AV/C unit: `UnitSpecID 0x0000A02D`, `UnitSWVersion 0x00014001`; BCO directory: `UnitSpecID 0x7f5`, `UnitSWVersion 1` | `bcd` appendix |
| Plugs | `NumberOfOPCR 2`, `NumberOfIPCR 2` (the FW1814 has 2 oPCR / 3 iPCR) | `bcd` appendix `[1394]`; 1814: memory `project_fw1814_plugs` |
| Silicon | BridgeCo **DM1000E** (the FW410 is DM1000) | `ctl-services` `normal.rs:9`; Rev2 class in the kext (see §7) |
| Power | 6-pin FireWire bus power or 12 V DC; the second 6-pin port passes bus power but sources none | `man p.10` (items 9, 10) |
| 1394 PHY/LLC | `LLCPhyMode 1`, `1394PowerClass 0x01`, `MaxRec 0x08`, IRM/CMC/ISC/BMC capable | `bcd` appendix |

**Kext matching**: the reference matches by class, not Model_ID: `FWMetaNub` matches every FireWire device, and its `probe` filters by config ROM; the per-model device classes (`com_m_audio_FWAudiophileDevice`) match the nub by class alone (`docs/system-map/A1-reference-kext.md` Finding 1; the plist personality for the Audiophile has no `Model_ID`, `_extracted/kext/…/Info.plist` lines 110-120, product name `"FW Audiophile"`). Which Model_IDs `FWMetaNub::probe` maps to which device class **was not read for the Audiophile** (**unverified**; check: decompile `FWMetaNub::probe` and its model table).

**Bootstrapping**: after power-on the unit sits in the bootloader (`snd-bebob` `bebob_maudio.c:10-24`: "Firewire 410/Audiophile/1814 and ProjectMix I/O wait to download firmware blob"; firmware ≥ build 5058 keeps the image in flash and only needs a *cue*). The kext path is the same as the FW1814's: the helper hands `FWAP.bcd` to the kext (`MAFirmwareUpgradeImage`, `docs/system-map/A5-firmware.md`) — **but the Audiophile `.bcd` header differs, see §6**. Hot-plug warning in the manual (`man p.5-6, 56-57`): same rule as the 1814 — power-cycle the interface instead of replugging.

---

## 2. I/O inventory

`man p.5-7, 10`; counts cross-checked against the kext (§3).

| Direction | Count | Detail |
|---|---|---|
| Analog in | 2 | unbalanced RCA, −10 dBV; fixed gain (no mic pre-amps, no pads, no phantom) — **no analog control exists to expose** |
| S/PDIF in | 2 | coaxial RCA only (no optical) |
| Analog out | 4 | unbalanced RCA, −10 dBV (outs 1-4) |
| S/PDIF out | 2 | coaxial RCA; AC-3/DTS pass-through "Windows and Mac OS X only" |
| Headphone out | 1 stereo | 1/4" TRS, dedicated level **encoder**, not a stream |
| MIDI | 1 in / 1 out | 5-pin DIN; "sixteen channels" = one port, 16 MIDI channels |
| FireWire | 2 × 6-pin | one to host, one daisy-chain |
| **Host streams** | **4 in / 6 out** | in = analog 1-2 + S/PDIF L/R; out = analog 1-4 + S/PDIF L/R (`man p.5-6`; kext §3) |
| Rates | 44.1 / 48 / 88.2 / 96 kHz | kext table §3; manual p.7 lists "32k to 96k for analog" and "44.1…96k for S/PDIF" — 32 kHz is **not** in the kext's list |
| Resolution | 24-bit | `man p.6` |

Hardware indicators (not driver state): S/PDIF in/out LEDs, power LED (blue; blinks on power, steady when the host connection is valid — `man p.9`), and the two **HP-source A/B LEDs**, which the *driver* sets (§4.4).

Front-panel items the driver *does* see (via the 60-byte status packet, §4.3): the **Level Controller** encoder, the **headphone level** encoder and the **momentary HP-source/Direct-Monitor switch**. Everything else on the panel is analog or an LED.

---

## 3. Stream layout and per-device constants (kext)

All values from the Audiophile overrides of the reference device class. Decompiles in `docs/system-map/A1-ghidra/ghidra-exports/decomp/FWAudiophileDevice__*.c`; the others were decompiled for this document (headless Ghidra 12.1.4, 2026-10-19) and are filed as `audiophile-overrides-2026-10-19.txt` in the same directory.

### 3.1 Channel counts and rates

* `GetDeviceChannelsForSampleRate @0xafb4` ignores the rate and returns one static table (`sDeviceChannels @0x4aaa0`, read from the binary): words `4, 4, 6, 6, 1, …` — **in 4/4, out 6/6, MIDI quadlets per block `+0x10 = 1`**, same structure as the 1814's `DeviceChannels` (Q-P4 §A.1). So the counts are **the same at 44.1, 48, 88.2 and 96 kHz** (no S/MUX-style reduction, no digital-mode dependence; contrast the FW1814's rate/mode-dependent tables).
* `GetSupportedInput/OutputSampleRates @0xafc2/@0xafd0` both return `kSupportedSampleRatesArray @0x2f720`: `44100, 48000, 88200, 96000` (zero-terminated; entries are 8 bytes).
* `SetupAudioEngine @0xbb9a`: `engine[+0x2e8]=1` (MIDI quadlets), `[+0x2e4]=[+0x2f0]=4` (in), `[+0x2e0]=[+0x2ec]=6` (out), one MIDI input and one MIDI output port (`SetMIDIInputPorts/SetMIDIOutputPorts(…,1,0)`), then installs `[+0x310]=_ConvertLLRR4SkipMIDI1`, `[+0x300]=_RearrangeSampleFrameLLRR6`, `[+0x308]=_SamplePeaks6`, `[+0x318]=device`. **Differences from `FW1814Device::SetupAudioEngine @0xed34`**: the 1814 also sets `[+0x2d5]=[+0x2d6]=[+0x348]=1` and calls `vt[0xaf8]` (`ResetRearrangementProcs`, because its procs depend on the digital mode); the Audiophile sets none of those and installs the procs directly — it has a single fixed layout. The meaning of `[+0x2d5]/[+0x348]` is not decoded (`[+0x2d6]` = refresh RX packet parameters at every rate, Q-P4 §A.1).

### 3.2 Wire frame

Per data block (one sample frame): **out = 6 audio quadlets + 1 MIDI quadlet; in = 4 audio + 1 MIDI** (`ffado` `fwap.xml` "Dimension 7 / 5", Format 6 = MBLA, Format 13 = MIDI conformant). Isoch packet sizes follow the project formulas (Q-P4 §A.1: TX `4·(a+m)·SYT+8`, RX `4·(a+m)·SYT+12`) — **computed, not measured**:

| Rate | SYT interval | TX bytes/pkt (a=6,m=1) | RX bytes/pkt (a=4,m=1) | bandwidth B/cycle `(in+out+2m)·SYT·4+16` |
|---|---|---|---|---|
| 44.1 / 48 kHz | 8 | 232 | 172 | 400 |
| 88.2 / 96 kHz | 16 | 456 | 332 | 784 |

(The 1814 at 44.1/48 S/PDIF mode is 232 TX / 364 RX / 592 bandwidth; the 1814 TX packet is identical to the Audiophile's.)

### 3.3 Channel order on the wire (derived from the kext's converters)

Read from the machine code of the converter routines (`otool -tV`, `mafw.asm`):

* **Output, `_RearrangeSampleFrameLLRR6 @0x2c0e5`**: per frame, clamps each sample to ±24 bit, byte-swaps, adds label `0x40` (MBLA), and writes `wire[i] = host[p[i]]` with **`p = 0, 2, 4, 1, 3, 5`**. With the host order `[1, 2, 3, 4, S/PDIF L, S/PDIF R]` (= `[A.L, A.R, B.L, B.R, C.L, C.R]`) the wire is **`[A.L, B.L, C.L, A.R, B.R, C.R]`**, i.e. *all lefts then all rights* ("LLRR").
* **Input, `_ConvertLLRR4SkipMIDI1 @0x28eeb`**: reads 5-quadlet frames (stride `0x14`, skips the MIDI quadlet at slot 4), sign-extends 24 bit, and writes `host = [wire1, wire3, wire0, wire2]`, i.e. wire `[X.L, Y.L, X.R, Y.R]` with **host 1/2 = pair Y, host 3/4 = pair X**. With the manual's input order (analog 1, 2, then S/PDIF L, R — `man p.49`), wire = `[S/PDIF L, Analog L, S/PDIF R, Analog R]`.
* **Corroboration**: `FWAudiophileDevice::IndexForAC3OutputChannels @0xaa0e` returns `(2, 5)`; wire slots 2 and 5 are exactly the S/PDIF pair under the "LLRR" derivation above (pair C = `C.L` at slot 2, `C.R` at slot 5).
* **Conflict**: `ffado` `fwap.xml` lists the stream positions differently (out: S/PDIF L/R at 0/1, "LineOut1" L/R at 2/4, "LineOut2" L/R at 3/5; in: S/PDIF 0/1, line 2/3). The kext is the authority (project rule), but the physical port ↔ wire-slot mapping above is **derived, not measured**. Settle with a loop test on hardware (play a tone on each host channel, read it at the RCA/S/PDIF output; the same with the inputs) or by reading the AV/C plug cluster info with `ExtendedPlugInfo`.
* The MIDI quadlet sits after the audio quadlets (out slot 6, in slot 4) — `ffado`, and the "SkipMIDI1" in the converter name.

### 3.4 Latency and sync constants

* `GetRoundTripLatencyForFDF @0xafde` (frames; FDF index 0=32k, 1=44.1k, 2=48k, 3=88.2k, 4=96k as in `kext/src/FWRates.h`): 32k/44.1k → `0xb6` (182), 48k → `0xc4` (196), 88.2k → `0x14a` (330), 96k → `0x162` (354). The 1814 values are 210/210/225/358/378 (`FWRoundTripLatencyForFDF`): the Audiophile is ≈ 28 frames lower at every rate.
* The shared `FWAudioDevice::GetSafetyOffsetForFDF @0x1d58` is not overridden by the Audiophile (no such symbol in its list); the same table applies (**inferred from the symbol list, not from a call trace**).
* Peak meter page: `FWAudioDevice::GetPeakLevelCount @0x1d30 = 0x34` (52 slots) is shared; the Audiophile uses 20: **14 hardware slots + 6 software-return slots** (`GetFirstSWReturnPeakMeterIndex @0xaa02 = 0xe`; the 1814 returns different values). `_SamplePeaks6` is the output peak proc.
* Engine/device naming: `GetDescriptionString` `"FireWire Audiophile"`, `GetShortDescriptionString` `"FWAudiophile"`, `GetIconName` `"FW AP box.png"`; the reference panel icon is `FAdS.icns`.

---

## 4. Control plane

**There is no HSCI, no `0xFFC7:0x00700000` mixer register window and no vendor-dependent clock command on this device.** `snd-bebob` `bebob_maudio.c:26-37` states the split: the 1814 and ProjectMix use the "special" firmware that freezes on unknown commands and is mixed by direct writes to `0xFFC700700000…9c`; **the Audiophile is a "normal" BeBoB device mixed through standard AV/C**. The kext confirms it: the Audiophile class derives from **`FWRev2AudioDevice`** (constructor @0xbe6c calls `FWRev2AudioDevice::FWRev2AudioDevice`), whereas the FW1814 derives from **`FWRev1AudioDevice`** (constructor `FW1814Device::C2` calls `FWRev1AudioDevice::C2`; CLAUDE.md, "Rev1 vs Rev2 differ in the AV/C stream-format command: plain 8-byte `0x18/0x19` vs extended `0x2F`"; `A1-ghidra-overview.md`'s "FWRev2 — the FW1814 generation" is superseded). The clone's `FWAvc`/stream-format code is therefore Rev1 and an Audiophile needs the Rev2 variants but implements **`MapChannelAndFunctionBlock`, `MapRoutingParameters`, `SetHeadphoneSelector`, `AVCControlSetLEDStatus`, `ResendCurrentMixerSettings`, `ResetToFactorySettings`, `SetClockSourceInternal`** (the AV/C paths), and does not override the 1814's `CustomMixerWriteQuads`/`FWSettings1814Levels` machinery.

### 4.1 Mixer topology (AV/C function blocks)

Signal flow (`ctl-services` `normal.rs:46-70`, the Audiophile diagram): inputs (analog in 1/2, S/PDIF in 1/2, three stream-in pairs) feed a **10×6 mixer** (outputs `mixer-1/2`, `3/4`, `5/6`) and a **10×2 aux mixer**; each of the three output pairs (`analog 1/2`, `3/4`, `digital 1/2`) has a 2-way selector (**mixer-output | aux-output**) = the panel's `main/aux` toggle; the headphone has a 4-way selector (`mixer 1/2`, `3/4`, `5/6`, `aux`) "one source only". Mapping to the manual: `man p.36-39` (mixer page, output page).

Function-block ids used by the reference implementation of the same device, from `ctl-services` (`normal.rs:512-640`) and cross-checked against the kext's mapping function (§4.2):

| FB id | Role | Channels |
|---|---|---|
| `0x00`,`0x01`,`0x02` | stream-in pairs 1/2, 3/4, 5/6 into the mixer | |
| `0x03` | analog in 1/2 into the mixer | |
| `0x04` | digital in 1/2 into the mixer | |
| `0x04` (selector) | headphone source selector (4 plugs) | |
| `0x01`,`0x02`,`0x03` (selectors) | output-pair selectors mixer/aux for analog 1/2, 3/4, S/PDIF | |
| `0x04`,`0x05` (feature) | physical inputs analog / digital level + L/R balance | 2 ch each |
| `0x06`,`0x07`,`0x08` | aux-source feature blocks for stream pairs 1/2, 3/4, 5/6 | 2 ch each |
| `0x09`,`0x0a` | aux-source feature blocks for analog in, digital in | 2 ch each |
| `0x0b` | aux output feature block | 2 ch |
| `0x0c`,`0x0d`,`0x0e` | physical output level: analog 1/2, analog 3/4, digital | 2 ch each |
| `0x0f` | headphone level | 2 ch |

(Ids are reused between feature blocks and selectors; AV/C addresses them by function-block *type* + id. The table above is `ctl-services`'s, not decoded from the kext; **the byte-level commands the kext sends were not read** — the kext's generic AV/C layer `m_audio_b_FWAVCAudioDevice::AVCControlFunctionBlock` is shared with the 1814's non-HSCI paths.)

Mixer cross-point values: `SRC_OFF = 0x8000`, `SRC_ON = 0` (`ctl-services` `normal.rs` mixer trait) — i.e. routing is a per-cross-point mute, not a gain; the per-strip level is a separate feature block. **The ASIC "gets heavy load from AV/C status requests for the mixer parameters, so reads often time out"** (`normal.rs`, `cache()` doc): read-back must be conservative. The reference never reads the mixer back; it keeps state in its property objects and re-sends it (`ResendCurrentMixerSettings @0xb00e`).

### 4.2 Kext mapping functions (the "driver-side" table)

* `MapChannelAndFunctionBlock @0xb0e2` — switch over a *kind* (enum `0…9`) turning (host channel, kind) into (function block id, channel in block). Decoded from the machine code (raw; the enum names were not recovered):
  * kind 0: 4-row table at `0x2f750`: host channels `1-2 → FB 9`, `3-4 → FB 10`, `5-6 → FB 6`, `7-8 → FB 7` (renumbered inside the block). Matches the aux-source blocks of §4.1 (analog in, digital in, stream 1/2, stream 3/4). **Stream pair 5/6 (FB 8) has no row** — **unverified** whether the S/PDIF software return has no aux send in the reference or whether a row is missing; check against the panel (the manual says "two software auxiliary sends are available per stereo pair", `man p.34`).
  * kind 1 → FB `0xf` (headphone level); kind 2 → FB `0xc`/`0xd`/`0xe` by channel pair 1-2/3-4/5-6 (physical outputs); kind 3 → FB `1`/`2`/`3` by pair (the output selectors); kind 5 → FB = channel; kind 6 → FB `4` for 1-2 and `5` for 3-4; kind 7 → FB `0xb` (aux output); kind 8 → FB `4`; kind 4 → `kIOReturnUnsupported` (`0xe00002c7`); kind 9 → `kIOReturnBadArgument`.
* `MapRoutingParameters @0xba50` — looks the (source, bus) up in `_kRouteTable @0x2f7a0` with 15 entries (`FetchRoutingEntry(…, 0xf, table)`), then sets two output fields to 1. Table (raw words `{kind, group, bus, code}`): `kind 1` (software returns) groups 1-3 × buses 1-3 → codes `0x001-0x003, 0x101-0x103, 0x201-0x203`; `kind 0` (hardware inputs) group 1 (analog) and 2 (S/PDIF) × buses 1-3 → `0x301-0x303, 0x401-0x403`. I.e. **a 5-source × 3-bus matrix**, the manual's "output routing" buttons `1/2`, `3/4`, `spd` per strip (`man p.37`), `code = (source index << 8) | bus`.
* `ResendCurrentMixerSettings @0xb00e` / `ResetToFactorySettings @0xb078` call the base `FWAudioDevice` implementation and then the same on four settings objects in order: `FWSettingsABSwitch` (+0x2d0), `FWSettingsRotaryControl` (+0x468), `FWSettingsAuxSend` (+0x530), `FWSettingsLevels` (+0x808). Compare the 1814's list in `FW1814Device__ResendCurrentMixerSettings.c`.

### 4.3 Hardware status packet (`ReadHSCIData`, 60 bytes)

* `GetControlPacketSize @0xaf7e = 0x3c` (60); the 1814's is `0x26`. `NeedsControlPacketsForReadingHardwareControls @0xaa20 = 1`.
* Same address and size as the Linux `METER_SIZE_AUDIOPHILE 60` ("with sync info", `snd-bebob` `bebob_maudio.c:58`): a block read of `0xFFC7:0x00600000`. The firmware advertises this window in its BCO directory: `DataAddress = 0xFFFFC7:0x600000`, `HeaderAddress = 0xFFFFC7:0x100000` (`bcd` appendix). (The 1814 reads the same address, and "no asynchronous reads outside the 50 ms window" is a hardware invariant documented for the 1814 in `B1-ffado-correlate.md` §8.8 — **unverified for the Audiophile**: it is a "normal" device; `snd-bebob` reads it freely. Check the reference's polling interval for the Audiophile.)
* Layout, 15 big-endian quadlets: **14 peak levels** (7 pairs: analog in, digital in, analog out 1/2, analog out 3/4, digital out, headphone, aux — `snd-bebob` `audiophile_meter_labels`; `ctl-services` `AudiophileMeterProtocol`: 4 physical in, 6 physical out, headphone ×2, aux ×2) then **one status quadlet** at byte offset `0x38`.
* `ReadHSCIData @0xba92` (decoded from the machine code) on the status quadlet `q` (host-order read of the wire bytes; the byte numbers below are the wire byte order):
  * `UpdatePeakMeters(packet, 14)`; copies 6 software-return peaks from the device's software-peak array (`+0x110`) into the shared page (`+0x108`) at index 14…19.
  * `q >> 16 & 0xff` = sample-rate code and `q > 0xffffff` = "synced" flag → `DetectExternalSyncChanges(synced, fdf)`. Matches `snd-bebob` (`check_clk_sync`: `buf[size-2] != 0xff`, "if synced, equals the SFC of FDF") and `ctl-services` (`frame[len-1] > 0`). **Which wire byte is which differs by endianness between the two descriptions**; the code above is the kext's, read as a little-endian dword.
  * `q >> 8 & 0xf` → `TRotaryControl[+0x1268]` → on change `FWSettingsRotaryControl::UserRotatedHardwareKnob` = the **Level Controller** encoder.
  * `q >> 12 & 0xf` → `TRotaryControl[+0x126c]` → on change `SetLevelsFromKnob(…, "MAHeadphoneLevels", 0x38, group +0x1548, delta)` = the **headphone level** encoder (writes the software headphone fader; there is **no analog headphone pot**, `man p.9` "dedicated level control" is a digital encoder like the 1814's).
  * `q & 0xff >= 0x10` (any of bits 4-7) → `FWSettingsABSwitch::MomentarySwitchPressed` = the **HP-source / Direct-Monitor** button. `ctl-services` decodes the same fields (`rotaries` from the 2nd byte nibbles, `switch` from `& 0xf0` of the first byte). Encoder codes: nibble 1 = clockwise step, 2 = counter-clockwise (`normal.rs`: `ROTARY_STEP 0x200`, range `0…0x7fff`) — **the kext's own step handling (`TRotaryControl::RotaryValueChanged`) was not decoded**.

### 4.4 Headphone selector, LEDs, clock

* **`SetHeadphoneSelector(fb, plug) @0xb6ee`**: builds an AV/C CONTROL to a function block and sends it through `AVCControlFunctionBlock(0x80, …)`; the 12-byte payload has `[6]=0x02`, `[7]=plug` (selector input 0…3), `[8]=0x01`, rest 0. `ctl-services` documents the headphone selector as FB `0x04` with input plugs `0=mixer 1/2, 1=3/4, 2=5/6, 3=aux`. Registered with the ABSwitch object in `InitializeUserSettableProperties` (@0xb7e6).
* **`AVCControlSetLEDStatus(value) @0xb7d0`** = `AVCVendorCommand(value, 2, 0)`: a vendor-dependent AV/C command (operand `2`) that lights the A/B LED (`man p.9` item 3: LEDs show A/B, or the Direct Monitoring state). Registered as a virtual (vt `0xbc8`) callback of the ABSwitch object. **The command's company id / opcode bytes were not read** (check `AVCVendorCommand` args, shared with the 1814's `AVCControlSetLEDStatus`).
* **Clock — `SetClockSourceInternal @0xb9d4`**: wraps `FWRev2AudioDevice::AVCSetupSyncStream(arg, 1)` in the engine's `vt[0xa50]/[0xa58]` pair (config-change lock/unlock). The `MAClockSource` value must be `≥ 0x40000000` (gate bit 30): **`(v & 3) == 1` → `AVCSetupSyncStream(4, 1)` (external S/PDIF), `v == 0x40000000` → `AVCSetupSyncStream(0, 1)` (internal), any other low bits → `kIOReturnBadArgument`**. So the Audiophile's clock selector has **two values only**; compare the 1814's composite bitfield (memory `project_maclocksource_is_composite_bitfield`). The `ctl-services` description of the same selector: sync destination = music subunit plug `0x01`; sources = *internal* (music subunit plug 1) and *S/PDIF* (unit external plug `0x02`). The meaning of the integer `4`/`0` passed to `AVCSetupSyncStream` is **unverified** (probably a plug/source index).
* The manual's sync choice is "Internal / External" (`man p.41`).

### 4.5 User-visible settings objects

`InitializeUserSettableProperties @0xb7e6` initialises, in order: the base class; the rotary-control settings (+0x468) bound to the channel groups (+0x1440, +0x1270, +0x1378, +0x1548, +0x1758); the aux-send settings (+0x530, `pairs = group.count/2`); the levels settings (+0x808); the A/B switch (+0x2d0); then attaches the two callbacks (LED status, `SetHeadphoneSelector`) and calls `FWSettingsABSwitch::UpdateLEDs`. The channel-group layout (`FWAudiophileDevice::$_667` @0xb4ae) is three `ConcreteChannelGroup<6>` (levels / mutes / solos…) and `<2>` groups. The names/cookies of the `MA*` keys that the Audiophile exposes to the panel were **not extracted**: the 1814's catalog is in `docs/system-map/A2-panel/PN2-control-key-map.md`; the Audiophile subset must be derived from `FWAudiophileDevice::InitializeUserSettableProperties` + `FW410Device::…` (similar, 78 lines) — **open item**.

---

## 5. Panel model (what the host UI must offer)

From the manual (`man p.34-42`) and the Audiophile's channel groups:

* **Mixer page**: strips `sw rtn 1/2`, `sw rtn 3/4`, `spdif sw rtn` (3 software-return pairs), `analog in`, `spdif in` (2 hardware pairs). Per strip: fader, pan (hardware strips only), 2 aux-send knobs (L/R), `link`, `mute`, `solo`, **output-routing buttons `1/2`, `3/4`, `spd`** (any combination; sums), `ctrl` (Level-Controller membership). Meters per output bus + aux.
* **Output page**: `1/2`, `3/4`, `spdif` output strips (fader pair, `bal`, `link`, `mute`, `solo`, **`main/aux` switch**), the aux master (fader pair, `bal`, pan, `link`, `mute`), the **phones** strip (fader pair, `bal`, `link`, `mute`, source-assign buttons for `1/2`, `3/4`, `spdif`, `aux`); no `ctrl` on phones (own encoder).
* **Hardware page**: sync source `internal | external`; headphone-source A and B radio sets (4 choices each); momentary-switch function `HP source | Direct Monitoring`; sample-rate-detected readout. The ASIO/WDM buffer-size field and the OS 9 bit-depth field do not apply on the Mac (`man p.40-41`); CoreAudio drives the buffer size.
* **Global**: reset/save/load/delete settings, Level-Controller assignment (SW return | Output | Input | Aux send, per-strip `ctrl` override), **main mute and dim (−20 dB)**, a "Unit 1" selector (the manual: "only a single unit is supported", `man p.35`).
* **About page**: firmware version, driver version, update link.

Defaults (`man p.55`): mixer faders 0 dBFS; **output faders −6 dB**; output strips `main`; headphone source `1/2`; `sw rtn 1/2 → out 1/2`, `sw rtn 3/4 → out 3/4`, `sw rtn spdif → spd`; aux knobs at minimum; analog/digital inputs **unrouted**; pans hard L/R; balances center; mute/solo/link off; Level Controller on `output` (all outputs). (Compare the 1814: output faders 0 dB.)

The reference panel is a Carbon app whose per-model UI classes are named `CX<Model>Device` (`CXAudiophileDevice` exists in the panel binary; strings: `com_m_audio_FWAudiophileDevice`, `FW Audiophile`). It was **not decompiled for the Audiophile** (the 1814's `CX1814Device` was, A2-panel). Its view layout, ranges (fader dB range, pan law) and key names must be read before building the Audiophile panel view. The panel ships `FW410*`, `FWDJ*`, etc. artwork for every model in the same bundle.

---

## 6. Firmware file (`FWAP.bcd`)

Facts checked here (Python, 2026-10-19):

| Field | `FWAP.bcd` (Audiophile) | `fw1814.bcd` (A5 doc) |
|---|---|---|
| Magic / version | `bCoD` / 1 | same |
| Build date | `20070504102212` (= the firmware version `snd-firewire-ctl-services` documents for the Audiophile, `normal.rs:120-135`, "ID 0x00010060") | `20070713080440` |
| `0x18` | **`0x00010060`** (software/model id) | zeros |
| `0x1c` / `0x20` / `0x24` | `0x00ffffff` / `0x0d` / `0x0d6c` (revision / hardware model / vendor) | zeros |
| `0x2c` | header CRC32 `0xe3c4bb1e` over `0x00..0x5f` with the field zeroed — **verified** (same rule as the 1814 file; the header proper is longer, the name field runs to `0x77`) | `0xb2c28acb` over `0x00..0x5f` — verified |
| `0x30` | payload start **`0x78`** | `0x70` |
| `0x34` | **load base `0x20080000`** (equals "image base address 0x20080000" in the `bco-bootloader-info` output of `ctl-services`) | 0 |
| `0x38` / `0x3c` | payload length `0x113e3c`, CRC32 `0x37b1cc3d` — **verified** | `0xe4a9c` / `0xb84c5237` |
| appendix | start `0x113eb4`, length `0xdac`, CRC32 `0x29382f2b` — **verified** | start `0xe4b0c`, length `0x146c` |
| product name (after the header, up to the payload) | `FW Audiophile# M-Audio#` | `Midiman FW 106` |
| file size | `0x115000` — **padded with `0xff` after the appendix** (appendix ends at `0x114c60`) | no trailing data |

**Consequences for the helper/kext hand-off**: the 1814 notes ("both regions are contiguous, appendix ends at EOF", header `0x60` bytes) **do not hold for this file**: a parser must use the offsets and lengths *in the file*, not "end of file", and not a fixed header size of `0x70`. The reference helper hands the image to the kext opaquely (A5 "clone implications" 1), so the same path works; the clone's CRC pre-check must be generalised. The image is larger than the 1814's (1.1 MB vs 0.94 MB).

Appendix (the firmware's own attribute namespace, read from the file) — facts a driver can rely on:

* `ClockDividers`: `dars`/`syt`/`csp` tables for 32 000, 44 100, 48 000, 88 200, 96 000 Hz (`syt`/`csp` also list 176.4/192 kHz, `dars` — the audio clock — does not): **the audio clock stops at 96 kHz**, consistent with the kext.
* `1394`: `NumberOfOPCR 2`, `NumberOfIPCR 2`; asynchronous queues (`AsyResponseTx 600`, `AsyRequestTx 600`, `AsyResponseRx 1500`, `AsyRequestRx 1500`, `IsoTx 1000`).
* `AppSettings`: `Recorder PacketDelay 1000, SytOffset 375`; `Playback PacketDelay 825`; `Route MaxNoOfLoopbackChannels 30`; `DefaultSampleRate 1` (index).
* Config ROM contents of §1; `sci` slave address `0x28` (the codec/LED controller bus, **unverified use**); `uart2` 31250 baud (MIDI).

---

## 7. Where the Audiophile differs from the FW1814 (implementation-driving table)

| Topic | FW1814 (clone today) | FW Audiophile (reference) |
|---|---|---|
| Device class chain | `FW1814Device : FWRev1AudioDevice` | `FWAudiophileDevice : FWRev2AudioDevice` (ctor @0xbe6c) — **different base**: Rev2 uses the extended AV/C stream-format command (`0x2F`), `AVCSetupSyncStream`, `UpdatePeakMeters` |
| Engine | `FWEngine2` | same family; `SetupAudioEngine` sets 4/6 channels, no rearrange reset |
| Streams | 10/6 (S/PDIF) · 16/12 (ADAT) · 12/8 · 2/4 by rate and mode | **4 in / 6 out at every rate**, no modes |
| Rates | 44.1-192 kHz | **44.1 / 48 / 88.2 / 96 kHz** |
| Plugs | 2 oPCR / 3 iPCR | 2 oPCR / 2 iPCR |
| Mixer path | HSCI + direct writes to `0xFFC7:0x00700000`, 160-byte batch, typed property dict with the DICE-like window | **standard AV/C function blocks**; `CustomMixerWriteQuads` not used |
| Status packet | HSCI 12-byte CONTROL envelope for clock/LED; heartbeat read of the control window; `GetControlPacketSize 0x26` | 60-byte block read at `0xFFC7:0x00600000`; clock via `AVCSetupSyncStream`; LED via vendor AV/C `(value, 2, 0)` |
| Clock selector | composite bitfield (sync bits, S/PDIF port, optical in/out, S/MUX, gate) | internal / S/PDIF external (+ gate bit 30) |
| Meters | 52-slot page: 18 in · 20 out · 14 SW | same 52-slot page, **14 hardware + 6 SW used** |
| Front panel | 2 headphone encoders + Level Controller + A/B | 1 headphone encoder + Level Controller + A/B (momentary) |
| Sample converters | `_ConvertLLRRInput10/16`, `_RearrangeSampleFrameLLRR…` by mode | `_ConvertLLRR4SkipMIDI1`, `_RearrangeSampleFrameLLRR6`, `_SamplePeaks6` |
| Round-trip latency (44.1/48/88.2/96) | 210 / 225 / 358 / 378 | 182 / 196 / 330 / 354 |
| Aux send | 2 L/R per strip; manual says 44.1/48 only | per strip pair; no rate restriction stated for the Audiophile (mixer is in the ASIC, **unverified at 88.2/96**) |
| MIDI | 1 in / 1 out | 1 in / 1 out (same MIDI quadlet position logic) |
| AC-3 | `IndexForAC3OutputChannels` | `(2, 5)` (S/PDIF pair under the LLRR order) |
| Firmware file | `fw1814.bcd`, header 0x70, no padding | `FWAP.bcd`, header 0x78 + load base, padded to 0x115000 |
| Control to match | Model_ID `0x10071` runtime | **`0x00010060` for both** bootloader and runtime (text differs) |

---

## 8. Unverified / open items (each with the check that settles it)

1. Port ↔ wire-slot mapping of §3.3 (conflicts with `fwap.xml`). *Check:* loopback tone per channel on hardware, or `ExtendedPlugInfo` cluster info.
2. `FWMetaNub::probe`'s model table (which Model_ID selects `FWAudiophileDevice`) and how the bootloader/runtime split by unit text works in the reference. *Check:* decompile `FWMetaNub::probe` and its callees.
3. The AV/C bytes the kext sends for level/pan/mute/solo/routing/selector/aux (FB types and control attributes), and in which order the initial state is applied. *Check:* decompile `m_audio_b_FWAVCAudioDevice::AVCControlFunctionBlock` callers and the settings classes' `SendToDevice`; capture with DTrace on an SL rig connected to an Audiophile (**no Audiophile on the SL rig at present**).
4. Names of the `MA*` property keys/cookies in the Audiophile's `InitializeUserSettableProperties` and the panel's `CXAudiophileDevice` view. *Check:* diff against `FW410Device::InitializeUserSettableProperties` and the 1814 catalog (`PN2-control-key-map.md`).
5. The kind enum of `MapChannelAndFunctionBlock` and the missing aux row for stream pair 5/6 (§4.2).
6. The meaning of `AVCSetupSyncStream`'s first argument (`4` vs `0`) and the exact vendor command for the LED (§4.4).
7. Whether the 50 ms polling rule of the 1814 status window applies (§4.3).
8. Whether aux sends/solo behave differently at 88.2/96 kHz (the 1814's restriction was refuted for the 1814 by the panel code, R18; nothing known for the Audiophile).
9. Behaviour on bus reset, sleep/wake, AC-3 pass-through and the second FireWire port as a daisy chain (not documented beyond the manual's bus-power note).
10. `FWAudiophileDevice` object layout (`+0x2d0 ABSwitch, +0x468 Rotary, +0x530 AuxSend, +0x808 Levels, +0x1268/+0x126c TRotaryControl, +0x1270 groups`) is from constructor/initializer reads only; the full field list was not decoded.

## 9. Source index

* Manual: `docs/audiophile manual.pdf` → `docs/audiophile-manual.md` (this session).
* Kext listings: `docs/system-map/A1-ghidra/ghidra-exports/{functions.tsv,decomp/FWAudiophileDevice__*.c,decomp/FW410Device__*.c}`, `otool-exports/mafw.asm` (symbols `__ZN30com_m_audio_FWAudiophileDevice…`), data tables read from `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` (`__const @0x2f700` file offset 194304, `__data @0x4aaa0` file offset 305824).
* Functions decompiled for this document, filed as `docs/system-map/A1-ghidra/ghidra-exports/decomp/audiophile-overrides-2026-10-19.txt`: `ResendCurrentMixerSettings @0xb00e`, `ResetToFactorySettings @0xb078`, `MapChannelAndFunctionBlock @0xb0e2`, `SetHeadphoneSelector @0xb6ee`, `AVCControlSetLEDStatus @0xb7d0`, `MapRoutingParameters @0xba50`, `FWAudiophileDevice` ctor @0xbe6c (headless Ghidra 12.1.4 `DecompList.java`, 2026-10-19).
* FFADO: `_refs/ffado/src/bebob/maudio/fwap.xml`; Linux: `_refs/linux-snd-bebob/sound/firewire/bebob/bebob_maudio.c`; Rust: `_refs/snd-firewire-ctl-services/protocols/bebob/src/maudio/normal.rs` (Audiophile classes, lines 474-570 and the meter trait around 791-940).
* Firmware: `_extracted/helper/Firmware/FW Audiophile/FWAP.bcd`; header parser used 2026-10-19 (Python, CRC32 via `zlib`).
* Project background: `docs/manual-reference.md` (FW1814 counterpart), `docs/system-map/A1-reference-kext.md`, `A1-ghidra/A1-ghidra-overview.md`, `A1-ghidra/Q-P4-cmp-per-rate-and-mixer-power-on.md`, `A5-firmware.md`, `B1-ffado-correlate.md`.
