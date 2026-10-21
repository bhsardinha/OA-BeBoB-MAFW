# Q-AP0 — Audiophile phase 0: device table, nub selection and the Rev2 base (AP-0.1, AP-0.2, part of AP-0.4)

Status: **resolved 2026-10-19** except where a line says **unverified**. Bundle: v1.11.2 `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice` (file offset = VM address in this slice). Tracker rows: `PROVENANCE-MATRIX-AUDIOPHILE.xlsx` Itens `AP-0.1.*`, `AP-0.2.*`, `AP-0.4.1-0.4.3`; flags `AUD-AP-01..06, 08`. Resolution entry: `PROVENANCE-RESOLUTIONS.md` R77.

Evidence: Ghidra 12.1.4 headless decompiles filed 2026-10-19 under `ghidra-exports/decomp/` (tag `ap0`, 141 new files, produced by `scripts/decomp_by_name.sh` with `DECOMP_DATE=2026-10-19`); raw assembly `otool-exports/mafw.asm`; bytes of the slice read with Python; vtables read from `__ZTV30com_m_audio_FWAudiophileDevice @0x35680` and `__ZTV17FWRev2AudioDevice @0x3a9a0` (slot = VM of the vtable + 0x10 + slot offset; local pointers resolved with `nm -n`).

## 1. Device table and nub selection (AP-0.1)

### 1.1 `kDeviceSpecs @0x46450` — read from the bytes

8 entries × 3 pointers (stride `0x18`): `{runtime product name, bootloader product name, device class name}`.

| # | Runtime name | Bootloader name | Class |
|---|---|---|---|
| 0 | `FW 410` | `FW Bootloader` | `com_m_audio_FW410Device` |
| 1 | `FW Audiophile` | `FW Audiophile Bootloader` | `com_m_audio_FWAudiophileDevice` |
| 2 | `FW 1814` | `FW 1814 Bootloader` | `com_m_audio_FW1814Device` |
| 3 | `ProjectMix` | `ProjectMix` | `com_m_audio_FWProjectMixDevice` |
| 4 | `FW Solo` | `FWSolo Bootloader` | `com_m_audio_FWSoloDevice` |
| 5 | `Ozonic` | `Ozonic Bootloader` | `com_m_audio_FWOzonicDevice` |
| 6 | `NRV-10` | `NRV-10 Bootloader` | `com_m_audio_FWNRV10Device` |
| 7 | `ProFire Lightbridge` | `ProFire Lightbridge Bootloader` | `com_m_audio_FWLightbridgeDevice` |

The only code reference is `GetActualProductName` (`mafw.asm` `0x1ce1c`: `leaq __ZL12kDeviceSpecs(%rip)`). The third field (class name) has no code reader in the kext (`grep kDeviceSpecs` in `mafw.asm` = 1 hit); class selection is done by the plist personalities (§1.3).

### 1.2 `FWMetaNub::probe @0x1ce62`, `GetActualProductName @0x1ce06`, `start @0x1cf68`

* `probe` accepts the `IOFireWireDevice` when its `FireWire Vendor Name` equals `"M-Audio"` or `"M-AUDIO"` **and** `GetActualProductName(FireWire Product Name)` is non-null; score `0x7FFFFF`. **No Model_ID test** (decompile `FWMetaNub__probe.c`).
* `GetActualProductName` loops over all 8 entries, compares the name with field 0 and with field 1, and returns field 0 (the runtime name) on a match. The loop does not stop at the first match (the last match wins; no two entries share a name, so this is moot).
* `start` copies `FireWire Product Name`, `FireWire Vendor Name`, `Vendor_ID`, `Model_ID`, `GUID` from the provider onto the nub, publishes `MAActualProductName` = runtime name (so a bootloader nub `FW Audiophile Bootloader` publishes `FW Audiophile`), caches the bootloader info registers, publishes `GUIDString`, and registers the nub.

### 1.3 Device-class matching

`_extracted/kext/…/Info.plist` (read with `plutil`, 2026-10-19): every device personality has `IOProviderClass = com_m_audio_FWMetaNub` and **`FireWire Product Name`** (`FW 410`, `FW Audiophile`, `FW 1814`, `ProjectMix`, `FW Solo`, `Ozonic`, `NRV-10`, `ProFire Lightbridge`); only `com_m_audio_FWNRV10Device` adds `Model_ID = 65665`. The two `FWSoloAppleGoAway{1,12}` personalities (class `MakeAppleFWAudioGoAway`, provider `IOFireWireAVCSubUnit`) exist for the Solo only. Since the nub copies the product name from the FireWire device, a bootloader nub (`FW Audiophile Bootloader`) matches no device personality and stays a firmware-only nub.

`m_audio_b_FWAVCAudioDevice::probe` (decompile `FWAVCAudioDevice__probe.c`) then requires: provider is a `FWMetaNub`; same `FireWire Product Name` as the personality; equal `Model_ID` when both sides have one; nub `[+0xf0] == 0` (not in bootloader) and `[+0xf3] != 0` (firmware match started, Q7). Score `0x7FFFFF`.

Live (SL v1.9, capture 89): `com_m_audio_FWAudiophileDevice` attached to `com_m_audio_FWMetaNub`; FireWire node `FW Audiophile`, `Model_ID` 65632 (`0x10060`, capture 89b).

**Clone impact.** `openaudio-fw1814/kext/src/FWMetaNub.cpp:39-51` already mirrors the table with one entry and the same name rule; `FWAudioDeviceChain.cpp:38` mirrors the device probe. The multi-model tree only extends the table to 8 rows and adds one personality per implemented model (AP-2.1).

**Unverified:** that the Audiophile's bootloader enumerates with the same Model_ID `0x10060` (third-party: `snd-bebob` `bebob.c:150-157`). Check: AP-7.1 power-cycle with the interface on our test Mac (the 1814's bootloader uses a different Model_ID, `0x10070`).

## 2. `FWRev2AudioDevice` vs `FWRev1AudioDevice` (AP-0.2)

Class chain (metaclass constructors, Q16 Correction 2026-10-14): Audiophile, FW410, Ozonic, Solo, NRV10, Lightbridge → `FWRev2AudioDevice`; FW1814 (and ProjectMix through it) → `FWRev1AudioDevice`; both → `m_audio_b_FWAVCAudioDevice`. `FWRev2AudioDevice` ctor `0x13574` only calls the `FWAVCAudioDevice` ctor and sets the vtable (no fields of its own).

Audiophile vtable slots that matter (read from the bytes):

| Slot | Audiophile resolves to |
|---|---|
| `0xb50` | `FWRev2AudioDevice::AVCControlPlugSignalFormat @0x12e7e` |
| `0xba0` | `com_m_audio_FWAudiophileDevice::GetControlPacketSize @0xaf7e` (returns `0x3c`) |
| `0xbc0` | `FWRev2AudioDevice::GetAddressAndPlugForSyncType @0x12ae8` |
| `0xa60` | `m_audio_b_FWAudioDevice::GetDeviceChannels @0x1e52` |

### 2.1 Stream format — `AVCControlPlugSignalFormat @0x12e7e` (asm `0x12e7e-0x1305a`)

Rev1 (FW1814, `@0x13734`): one 8-byte CONTROL `00 FF (0x18|dir) <fdf> 90 <fmt> FF FF` (plain INPUT/OUTPUT PLUG SIGNAL FORMAT).

Rev2: **AV/C EXTENDED STREAM FORMAT INFORMATION (opcode `0x2F`)**, two steps.

Rate code (`r14b`, jump table at `0x1305c`, values from the decompile `switch`): FDF0 → `0x02`, FDF1 and default → `0x03`, FDF2 → `0x04`, FDF3 → `0x0A`, FDF4 → `0x05`, FDF5 → `0x06`, FDF6 → `0x07` (32 / 44.1 / 48 / 88.2 / 96 / 176.4 / 192 kHz; FDF table from Q10).

1. **List walk (STATUS, subfunction `C1`)**, `idx` = 0, 1, 2, … (asm `0x12f0b-0x12f7c`):
   `01 FF 2F C1 <dir'> 00 00 <plug> FF FF <idx>` — 11 bytes written, **sent with length 14** (`movl $0xe,%edx` at `0x12f51`); bytes 11-13 are uninitialised stack. `dir'` = 1 when the kext's direction argument is 0, else 0 (`sete %bl` at `0x12f01`). Response buffer: 512 bytes (`operator new[](0x200)`).
   The loop stops when `reply[13]` (the entry's sampling-frequency code) equals the wanted rate code; `reply[13] == 0` → return `0xE00002BC`; a command error → return it.
2. **Set (CONTROL, subfunction `C0`)** (asm `0x12f7e-0x1302b`): length `= 2·reply[15] + 0x12`, zero-filled, then
   `00 FF 2F C0 <dir'> 00 00 <plug> FF FF 90 40 <rate> 01` followed by `reply[15]` and its `2·reply[15]` bytes (the list entry's `{channels, format}` pairs), copied with `memmove(cmd+14, reply+15, 2·reply[15]+1)`; the last 3 bytes stay zero.

Cross-check (FFADO, read 2026-10-19), field by field:
* opcode `0x2F` = `AVC1394_STREAM_FORMAT_SUPPORT`, `C0`/`C1` = extended stream format / list (`_refs/ffado/src/libavc/streamformat/avc_extended_stream_format.h:35-41`);
* operand order subfunction, plug address, status, list index (C1 only), format information (`avc_extended_stream_format.cpp:340-351`); plug address = direction (`0x00` input / `0x01` output, `.h:36-37`), address mode `00` = unit, unit plug = type `00` (isochronous), id, reserved `FF` (`general/avc_extended_cmd_generic.cpp:42-48`);
* format information = root `0x90` (audio/music), level 1 `0x40` (AM824 compound) (`.h:45,57`), then sampling frequency, rate control, number of fields, `{channels, format}` pairs (`avc_extended_stream_format.cpp:159-177`);
* rate codes `02/03/04/0A/05/06/07` = FFADO `ESamplingFrequency` 32 k … 192 k (`libavc/avc_definitions.h:115-125`); rate control `01` = `eRC_DontCare` (`avc_extended_stream_format.h:108-109`).

So the kext's direction argument 0 is the AV/C **output** plug (`dir' = 1`) and vice versa. The FW1814's Rev1 path never sends this command (the 1814 firmware freezes on commands it does not understand, `snd-bebob` `bebob_maudio.c:30-33`), so the Rev2 base must never be used for the 1814.

**Clone note (AP-2.3):** the 14-byte STATUS with 3 garbage bytes is mirrored as 14 bytes with zeros (deviation: we do not reproduce uninitialised memory; the device reads only the defined operands in AV/C — **unverified**, settle with the first hardware run).

### 2.2 Sync source — `AVCSetupSyncStream(type, plug) @0x12c60` (asm `0x12c60-0x12e1d`)

AV/C **SIGNAL SOURCE** (opcode `0x1A`, FFADO `avc_signal_source.cpp:33`):

1. STATUS `01 FF 1A 0F FF FE 60 <plug>` (destination = music subunit `0x60`, plug `<plug>`). Only the return code is checked; the reply is not parsed.
2. `vt[0xbc0](this, type, plug, &cmd[4], &cmd[5])` = `GetAddressAndPlugForSyncType` writes the **source** address/plug into bytes 4/5 (asm `0x12d59-0x12d8a`).
3. CONTROL `00 FF 1A 0F <addr> <srcplug> 60 <plug>`; its status is the return value.

Reference quirk: if step 2 fails the function returns `r12d` = 0 (success) without sending step 3 (asm `0x12d8f` → `0x12e0e`).

`GetAddressAndPlugForSyncType @0x12ae8` (decompile; source for step 3):

| type | addr | src plug | Meaning (from the AV/C address, not from a name in the binary) |
|---|---|---|---|
| 0 | `0x60` | `1` if plug == 1, else plug + 1 | music subunit (internal clock) |
| 1 | `0xFF` | `0x81` | unit external plug 1 |
| 2 | `0xFF` | `0x00` | unit isochronous plug 0 (sync to the incoming stream) |
| 3 | `0xFF` | `0x83` | unit external plug 3 |
| 4 | `0xFF` | `0x82` | unit external plug 2 |
| other | — | — | `0xE00002C7` |

### 2.3 Audiophile clock — `SetClockSourceInternal(value, bool) @0xb9d4` (asm `0xb9d4-0xba4e`)

With the main engine `[+0xe8]` paused (`vt[0xa50]` = `FWBaseEngine::pauseAudioEngine` before, `vt[0xa58]` = `resumeAudioEngine` after — slot names from `vtables-engine-device-stream.txt`; corrected 2026-10-19, R85, earlier text said "lock"):
* `value <= 0x3FFFFFFF` (gate bit 30 clear) → `0xE00002C2` (`kIOReturnBadArgument`);
* `value & 3 == 1` → `AVCSetupSyncStream(4, 1)` → source `FF 82` = **S/PDIF in**;
* `value & 3 == 0` → `AVCSetupSyncStream(0, 1)` → source `60 01` = **internal**;
* `value & 3` = 2 or 3 → `0xE00002C2`. **The Audiophile has no word clock and no "internal-muted" mode** (unlike the 1814's 4-radio `MAClockSource`).
* The `bool` argument is not read.

Cross-check: `snd-firewire-ctl-services` `protocols/bebob/src/maudio/normal.rs:483-495` (`AudiophileClkProtocol`): destination music subunit plug 1, sources Internal = music subunit plug 1, S/PDIF = `Unit Ext(0x02)` (= `0x82`). Byte-for-byte agreement. Live: the SL v1.9 panel wrote `MAClockSource = 0x40000001` (S/PDIF) in capture 90.

### 2.4 Latency (`GetRoundTripLatencyForFDF`)

| Class | FDF0 | FDF1 44.1 | FDF2 48 | FDF3 88.2 | FDF4 96 | FDF5/6 | asm |
|---|---|---|---|---|---|---|---|
| Rev1 | 0 | 186 | 186 | 262 | 292 | 292 | `0x136da-0x1370a` |
| Rev2 base | 226 | 226 | 210 | 457 | 385 | 397 | decompile `@0x12a54` |
| Audiophile | 182 | 182 | 196 | 330 | 354 | 182 | `0xafde-0xb00c` |

**Open (AP-0.9.2, hypothesis):** SL v1.9 at 48 kHz shows `IOAudioEngineOutputSampleLatency` 210, `InputSampleLatency` 32, `SampleOffset` 48 (capture 89). 210 is the Rev2 base value for FDF2, not the Audiophile's 196, and the split differs from the v1.11.2 rule `out = rt>>1` (Q-MD). Either v1.9 differs (version skew) or the split is not what Q-MD says for Rev2 engines. Check: read the v1.11.2 caller chain for a Rev2 engine, and `ioreg` at all four rates on the rig (AP-0.9 b).

### 2.5 Control packets (Rev2) and the Audiophile's 60-byte status packet (AP-0.4.1-0.4.3)

* `StartReceptionOfControlPackets @0x13138`: `new FWHSCI`, store the device's workloop owner, packet size = `vt[0xba0]()` (Audiophile `0x3C`), install a functor (tag `0xBB9`) as the read callback, `FWHSCI::Start(workloop)`; object in `[+0x2c8]`. `Stop`/`Pause` call the HSCI and `ClearPeakMeters`; `Resume` only the HSCI; `stop()` releases HSCI then the AV/C unit `[+0x268]`.
* `FWRev2AudioDevice::ReadHSCIData`, `StartExternalSyncUnmuteTimer` are empty; `initHardware` only forwards to the parent.
* `FWAudiophileDevice::NeedsControlPacketsForReadingHardwareControls` = 1.
* `FWAudiophileDevice::ReadHSCIData @0xba92` (decompile; **asm not yet read**):
  * quadlets 0-13 → `UpdatePeakMeters(buf, 14)`: `peak[i] = ((q & 0xff00) << 8 | q << 24) >> 16` of the raw quadlet — i.e. the byte-swapped low 16 bits of the host-order word = the **upper 16 bits of the big-endian wire quadlet**, **if** the buffer holds raw wire bytes (unverified);
  * slots 14-19 ← `[+0x110][14..19]` (software-return peaks computed on the host; `GetFirstSWReturnPeakMeterIndex` = 14);
  * quadlet 14 (offset `0x38`) is the status word `s` (host order): `s > 0xFFFFFF` → external sync locked; `(s >> 16) & 0xFF` → rate code; both to `DetectExternalSyncChanges`; `(s >> 8) & 0xF` → `TRotaryControl [+0x1268]` → `FWSettingsRotaryControl::UserRotatedHardwareKnob` (`[+0x468]`); `(s >> 12) & 0xF` → `TRotaryControl [+0x126c]` → `SetLevelsFromKnob(…, "MAHeadphoneLevels", 0x38, group [+0x1548])`; `(s << 24) > 0x0FFFFFFF` (bits 4-7 of the low byte) → `FWSettingsABSwitch::MomentarySwitchPressed` (`[+0x2d0]`).
  * Which physical encoder is which (Level Controller vs headphone) and the byte order on the wire are **unverified**: cross-check `ctl-services` `normal.rs` meter decode (~791-940) and `snd-bebob` `audiophile_meter_labels` before AP-3.4.

## 3. Corrections triggered

* `Q16-rev1-rev2-deltas.md`: SignalSource phase labels inverted; sync-type names unsourced; Rev1 latency mapping; "Solo = Rev1" (flags `AUD-AP-01..04`).
* `Q10-roundtrip-latency.md:11`: "only truly supported at 48k/88.2k/96k" is an inference (`AUD-AP-08`).
* `docs/audiophile-manual-reference.md` §1 and §8 item 2: matching is by product name, and the probe table is now read (`AUD-AP-05`, `AUD-AP-06`).

## 4. `com_m_audio_FWAudiophileDevice` object layout (AP-0.5, added 2026-10-19, R81)

Instance size **`0x17E0`** (`MetaClass::alloc`, asm `0xb6c9` `movl $0x17e0,%edi`; metaclass ctor `0xc1f0`). Constructor `@0xc140` (decompile) + uses read in Q-AP0b/Q-AP0c:

| Offset | Field | Evidence |
|---|---|---|
| `0x000-0x2cf` | `FWRev2AudioDevice` / `FWAVCAudioDevice` / `FWAudioDevice` base: `+0xe8` **main engine** (`pauseAudioEngine`/`resumeAudioEngine` around clock changes — corrected R85), `+0xf8` property dict, `+0x108` shared peak page (52 slots), `+0x110` host peaks, `+0x118` `MAClockSource` 32-bit prop (value `+0x144`, setter functor `+0x150`), `+0x268` AV/C unit, `+0x270` last-resend time, `+0x2c8` HSCI | Q-AP0 §2, Q-AP0b, `FW32BitNumberProp::CallSetter` (value `+0x2c`, functor `+0x38`), `FWAVCAudioDevice::InitializeUserSettableProperties` (`this+0x118`, "MAClockSource") |
| `0x2d0-0x467` | `FWSettingsABSwitch`; its `+0x18` LED functor = `[+0x2e8]` tag `0xBC9` → **vt `0xbc8` = `AVCControlSetLEDStatus @0xb7d0`**; `[+0x2f8]` = `SetHeadphoneSelector` functor | ctor; `InitializeUserSettableProperties @0xb7e6`; vtable bytes `0x35680+0x10+0xbc8` |
| `0x468-0x52f` | `FWSettingsRotaryControl` (`+0x4a0` = `MARotaryControlV2` 64-bit prop, `+0x4e8` = `MAMetaVirtualRotateDataKnob` function prop) | ctor (`0x468+0x38`, `+0x80`) |
| `0x530-0x807` | `FWSettingsAuxSend` | ctor |
| `0x808-0x1267` | `FWSettingsLevels` | ctor |
| `0x1268`, `0x126c` | `TRotaryControl` steps (= 1): Level Controller, headphone | ctor asm `0xbee0/0xbeea` |
| `0x1270-0x17df` | `$_667` channel groups: outputs `<6>` `0x1270`, inputs `<4>` `0x1378`, SW returns `<6>` `0x1440`, headphone `<2>` `0x1548`, aux sends `<10>` `0x15d0`, aux out `<2>` `0x1758` | `$_667` ctor; `InitializeUserSettableProperties` call arguments |

Other vtable slots read: `0xb48` = `MapChannelAndFunctionBlock @0xb0e2`, `0xb40` = `MapRoutingParameters @0xba50`, `0xac8` = `m_audio_b_FWAudioDevice::HasSoftwareControls @0xa992` (returns 1).

**Consequence for resend (Q-AP0c §5):** `FWAVCAudioDevice::ResendCurrentMixerSettings` runs because `HasSoftwareControls()` = 1, and its `[+0x150]([+0x144])` call is the `MAClockSource` setter with the current value — the clock source is re-applied first, then the settings objects resend. The headphone selector is still not resent by any `SendToDevice` (**open**: check whether `SubmitHeadphoneSourceChange` runs on start).

## 5. Start-up, format change, clock and sleep choreography (generic `FWAVCAudioDevice`, used by the Audiophile; added 2026-10-19, R85)

Decompiles `FWAVCAudioDevice__{initHardware,SetBlankSlateClockSource,SetClockSource,SystemWillSleep,SystemWokeUp}.c`; callers found by `callq *0xNNN(%r..)` in `mafw.asm`; Audiophile vtable slots read from the bytes.

| Step | What runs (Audiophile resolution) |
|---|---|
| `initHardware @0x22f1x` | parent `initHardware` → `vt[0xb58]` **`SetBlankSlateClockSource`** → `vt[0xb00]` `ResendCurrentMixerSettings` (Audiophile `@0xb00e`) → `vt[0xb78]` `StartReceptionOfControlPackets` (Rev2 `@0x13138`); fails if the last one fails |
| `SetBlankSlateClockSource @0x22de8` (base; **the 1814 overrides it, the Audiophile does not**) | if an engine exists: `vt[0xb60]` = `SetClockSourceInternal(0x40000000, true)` → **internal clock** (AVCSetupSyncStream(0, 1)); then **`IOSleep(2500)`** always |
| callers of `vt[0xb58]` | `FWBaseEngine::SetIOFormat @~0x4379` (every rate/format change), `FWBaseEngine::initHardware`, `FWStream::setFormat`, `FWAVCAudioDevice::initHardware/SystemWillSleep/SystemWokeUp`, pair/aggregate engines |
| `SetClockSource(v) @…` (setter of `MAClockSource`) | pause engine; legacy values `< 0x40000000` converted (0→`0x40000000`, 1→`0x40000004`, 2→`0x40000001`, 3→`0x40000005`, with an IOLog); `SetClockSourceInternal(v, false)`; on success: external = (`v` bit 31 set ? `v & 0xF ≠ 0` : `(v & 3) ∈ {1, 2}`) and, if it changed, engine `vt[0xce8]` = `FWEngine2::SetExternalSyncEnabled(external)`; `CompletePendingStateChanges`; resume engine |
| `SystemWillSleep @0x22b16` | blank-slate clock; pause control packets (`vt[0xb88]`); pause engine; if engine `[+0x2d0]` (external sync) → `SetExternalSyncEnabled(false)`; engine `vt[0xcc0]` = `DisconnectAllPlugs` |
| `SystemWokeUp @0x22d76` | `IOSleep(500)`; resend mixer; blank-slate clock; engine `vt[0xcb8]` = `ConnectAllPlugs`; resume control packets (`vt[0xb90]`); resume engine |
| stream kick in `FWEngine2::ConnectAudioMIDIChannels @0x12156` | `vt[0xb50]` = Rev2 `0x2F` (§2.1) on plug 0 for each direction, with the 500 ms gaps of R1 (Q-START) |

Consequences: every rate change on the Audiophile forces the internal clock and waits 2.5 s (then the user's clock is re-applied through the resend/`MAClockSource` path — **unverified** exact re-apply point: check `CompletePendingStateChanges`/`SetIOFormat`). The clone's base class has `SetBlankSlateClockSource` and `StartReceptionOfControlPackets` as no-ops (`openaudio-fw1814/kext/src/FWAudioDeviceChain.h:86-87`) and implements them only in the 1814 class, so both base versions are **new work** for the Audiophile (Rev2 + base).

## 6. Channel table and rates (added 2026-10-19, R85)

`GetDeviceChannelsForSampleRate @0xafb4` returns one table for every rate: `sDeviceChannels @0x4aaa0` (address from the `lea` at `0xafb8`, bytes read): `in 4, inStream 4, out 6, outStream 6, midi 1, clipOffset {0,0,0}, inRates = outRates = 0x2f720`, groups `{1,1,1,0}` (input pair 1 analog), `{1,2,2,1}` (input pair 2 S/PDIF), `{0,1,2,0}` (output pairs 1-2 analog), `{0,3,3,1}` (output pair 3 S/PDIF), `{0,0,0,3}` (end) — layout as `FWDeviceChannels` (`openaudio-fw1814/kext/src/FWRates.h:57`). `kSupportedSampleRatesArray @0x2f720` = `{44100,0} {48000,0} {88200,0} {96000,0} {0}` (input and output). No mode bits, no ADAT tables.

## 7. Generic engine paths the Audiophile inherits (added 2026-10-19, R88)

Audiophile vtable (bytes): `0xaa8` `FWAudioDevice::SupportsAC3` (asm `0xa982`: returns 1), `0xab0` `WantsAC3MixerBypass` (`0xa98e`: 0), `0xb38` `AC3Activated` (`0xa9e1`: empty), `0xb30` `GetMaxBandwidthBytesPerCycle` (`0xa9d4`: `0x928` = 2344) — all base implementations; the 1814 clone's base class already returns the same values (`openaudio-fw1814/kext/src/FWAudioDeviceChain.h:47-48,76-78`). With `IndexForAC3OutputChannels = (2, 5)` the AC-3 pair is the S/PDIF output (Q-AP0d).

CMP: the clone's `fHostPlug[3]` (`FWCMP.h:69`) is the host's own PCR slots (reference `[+0x28]` = 3 entries), not the device's plug count; the engine connects device plug 0 in each direction (`ConnectAudioMIDIChannels`, Q-START), so the Audiophile's 2 oPCR / 2 iPCR need no change.

External sync: `ReadHSCIData` passes `(locked = w3 ≠ 0, rate = w2)` to `FWAudioDevice::DetectExternalSyncChanges(bool, FDFValue)` (clone `FWAudioDeviceChain.h:112-113`). **Unverified:** that the Audiophile encodes `w2` as an FDF index (0..6) rather than an AV/C rate code. Check: read `0xFFC7:0x00600000` on the SL rig while the clock is S/PDIF at 44.1 and 48 kHz (read-only quadlet read; needs the user's OK), or compare with the 1814's HSCI byte (Q6).
