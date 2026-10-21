# ARCHITECTURE_MAFW v1: OpenAudio BeBoB MAFW driver (modular, all models)

Status: v1, 2026-10-20. Describes `openaudio-mafw-bebob/kext` after plan v4 task AP-2.5 (R98). It follows the official M-Audio driver v1.11.2 (`_extracted/kext`), whose class structure is the spec. Every statement cites the reference address or the system-map note it comes from. Anything not yet verified on hardware is marked **unverified**.

## 1. Principle

* **Common on a base, the model on top** (user, 2026-10-20). `kext/src/base/` holds everything shared by the M-Audio BeBoB models. `kext/src/models/<Model>/` holds only what that hardware makes different. The Makefile builds `base/` plus every `models/*/` folder.
* **Generic first.** `base/` contains no model name, table, class or include in code. `scripts/conformance_check.py` enforces this with the check "base/ is model-free".
  * The one exception is the reference's own device table `kDeviceSpecs`. It lives in the base nub (`FWMetaNub`, `@0x46450`) and lists all 8 models by name.
* **Same shape as the reference.** One module per reference class, with the same virtual methods (vtable slots cited).
  * Adding a model means adding its folder and its plist personality (its `kDeviceSpecs` row already exists).
  * It never means editing the base.

## 2. Class layers

```
IOService
└─ FWMetaNub                      base/FWMetaNub            nub per FireWire device, firmware state machine, kDeviceSpecs (Q7, Q-AP0 §1)
IOAudioDevice
└─ FWDiscreteDevice               base/FWAudioDevice        engine start/stop forwards
   └─ FWAudioDevice               base/FWAudioDevice(+Props) names, engine creation, property dict (MA* keys), meter page, sync detection, power
      └─ FWAVCAudioDevice         base/FWAVCAudioDevice     probe, AV/C unit + CMP, function-block builders, clock, mixer mapping virtuals,
         │                                                  control-packet poll (FWHSCI), sleep/wake, blank-slate clock
         ├─ FWRev1AudioDevice     base/FWRev1AudioDevice    plain 0x18/0x19 stream format, Rev1 latency       → FW1814, ProjectMix
         └─ FWRev2AudioDevice     base/FWRev2AudioDevice    0x2F stream format, SIGNAL SOURCE sync, Rev2 latency, hardware peaks
                                                                                                         → Audiophile, 410, Solo, Ozonic, NRV10, Lightbridge
            (model)               models/<Model>/<Model>Device    + alias class com_m_audio_<Model>Device (reference MIDI plug-in / panel matching)
Settings (OSObject):  FWSettings → FWSettingsLevels, FWSettingsAuxSend, FWSettingsRotaryControl, FWSettingsABSwitch   base/FWSettings
                      model subclasses where the mixer is not AV/C: FWSettings1814Levels / FWSettings1814AuxSend   models/FW1814/FW1814Settings
Engines:              FWBaseEngine → FWDiscreteEngine → FWEngine2 (multichannel), FWPairEngine, FWTimeSlaveEngine           base/ (Q13)
User clients:         FWKIPCUserClient → FWUserClient (notifications, MIDI RPC, meter page = memory type 1)            base/ (Q-UC, Q14)
```

The superclasses come from the metaclass constructors (Q16 Correction 2026-10-14). The Rev1/Rev2 split is in Q-AP0 §2.

## 3. What a model supplies (virtuals it overrides)

| Area | Virtual (slot) | FW1814 (`models/FW1814`) | Audiophile (`models/FWAudiophile`, AP-3) |
|---|---|---|---|
| Names | `GetDescriptionString` 0xa50, `GetShortDescriptionString` 0xa58, `GetIconName` 0xa48 | "FireWire 1814", "FW 1814", "FW 1814 box.png" | strings read in AP-3.1 |
| Channels and rates | `GetDeviceChannelsForSampleRate` 0xa68 | S/PDIF / ADAT / 176-192 rows by mode (`FW1814Tables.h`) | one table, `@0x4aaa0` |
| Latency | `GetRoundTripLatencyForFDF` 0xab8 | `@0xcf30` | `@0xafde` |
| Engine | `AllocateAudioEngine` 0xa20, `SetupAudioEngine` 0xa38, `ResetRearrangementProcs` 0xaf8 | FWEngine2, `@0xed34`, input-delay workaround | AP-3.1/3.2 |
| Meter page | `GetFirstSWReturnPeakMeterIndex` 0xa80 (+ count 0xa88 if not = out channels) | 0x26 / 0xe | 0xe (count = 6 out channels) |
| Mixer addressing | `MapChannelAndFunctionBlock` 0xb48, `MapRoutingParameters` 0xb40 | register block (kinds 3/4/5/7/8), base routing | AV/C tables, Q-AP0c §2-3 |
| Mixer objects | `InitializeUserSettableProperties` 0xb28, `ResendCurrentMixerSettings` 0xb00, `ResetToFactorySettings` 0xb08 | groups + batch writers + 1814 settings subclasses | six groups, generic settings, Q-AP0c §4 |
| Front panel | `SetHeadphoneSelector`, `AVCControlSetLEDStatus` | register 0x98 / vendor `00 FF 00 03…` | selector FB 4 / vendor `00 FF 00 00 0D 6C…` |
| Clock | `SetClockSourceInternal` 0xb60, `SetBlankSlateClockSource` 0xb58 (base body is fine for Rev2), `SwitchDigitalSignals` 0xbe0 | HSCI vendor command + selector, optical modes | SIGNAL SOURCE internal / S/PDIF (`@0xb9d4`) |
| Status packet | `GetControlPacketSize` 0xba0, `ReadHSCIData` | 0x58 bytes, `@0xd952` | 0x3C bytes, `@0xba92` |
| Bus reset | `BusReset` 0xb20 | ReconnectP2P on a thread | base |

Everything not in this table is base code and is shared.

## 4. Call order (reference; the clone follows it)

1. **Attach, two rounds** (Q-SDK5, Q7).
   * `FWMetaNub::probe` / `start` pick the `kDeviceSpecs` row by "FireWire Product Name".
   * The device personality's `FWAVCAudioDevice::probe` (`@0x22f78`) rejects until the nub reports running firmware. The helper sends `MAFirmwareStart`, and round 2 attaches.
2. **`FWAVCAudioDevice::initHardware`.**
   * First the parent `FWAudioDevice::initHardware` (`@0x342e`) runs: names, `'1394'` transport, connection manager, `createAudioEngine`, `RequestFormatUpdateForSampleRate`, `InitializeUserSettableProperties` (base keys + `MAClockSource`/`MAFirmwareInfo`, then the model's mixer), meter page, `MARateLocked`, `ClearPeakMeters`, `ResetToFactorySettings`, and engine registration (Q-ME §7b "Init order").
   * Then **`SetBlankSlateClockSource`**: base = internal clock + `IOSleep(2500)`; the FW1814 has its own (`@0xe4f0`).
   * Then **`ResendCurrentMixerSettings`**: base throttle + `MAClockSource` re-apply, then the model's settings objects resend.
   * Then **`StartReceptionOfControlPackets`**: the 50 ms FWHSCI poll.
3. **Stream start** (`FWEngine2::ConnectAudioMIDIChannels`, Q-START): CMP connect on plug 0 each way, then the stream-format "kick" through the revision's `AVCControlPlugSignalFormat` (Rev1 8-byte / Rev2 `0x2F`) with the reference's 500 ms gaps.
4. **Rate or format change** (`FWBaseEngine::SetIOFormat`): it calls `SetBlankSlateClockSource` (vt 0xb58). On Rev2 this forces the internal clock and waits 2.5 s (Q-AP0 §5; the exact re-apply point of the user's clock is **unverified**).
5. **Clock change** (`MAClockSource` → `SetClockSource @0x23858`):
   * pause the engine;
   * convert legacy values;
   * run the model's `SetClockSourceInternal`;
   * set the external-sync flag;
   * `CompletePendingStateChanges`;
   * resume.
6. **Status packet** every 50 ms. `ReadHSCIData` (per model) fills the hardware meter slots and turns lock/FDF into `MASampleRate` / `MARateLocked`. Encoders call `FWSettingsRotaryControl`, and the A/B button calls `FWSettingsABSwitch`.
7. **Sleep / wake** (`SystemWillSleep @0x22b16`, `SystemWokeUp @0x22d76`).
   * Sleep: blank slate, pause packets and engine, `DisconnectAllPlugs`.
   * Wake: wait 500 ms, resend the mixer, blank slate, `ConnectAllPlugs`, resume.

## 5. Mixer path (generic → model)

* The panel writes `MA*` keys with `IORegistryEntrySetCFProperties` on the device. `FWPropertyDict::SetProperties` dispatches the key to the owning settings object.
* The generic setters:
  * call `MapChannelAndFunctionBlock({kind, ch})` or `MapRoutingParameters({bus, pair, mixer})` on the device;
  * then send the AV/C feature, selector or mixer frame (`FWAvc.cpp`).
* Kinds (Q-AP0c §2):

  | Kind | Control |
  |---|---|
  | 0 | aux send |
  | 1 | headphone |
  | 2 | output |
  | 3 | SW return |
  | 4 | clock |
  | 5 | aux monitor |
  | 6 | input |
  | 7 | aux out |
  | 8 | headphone selector |

* A register-mixer model (FW1814) works differently:
  * it installs **batch writers** on its channel groups, and these take precedence over the per-channel AV/C setters;
  * it overrides the three setters that address registers (`FWSettings1814*`).
* Mute, solo, link, dim and global mute are computed by the shared channel-group engine (`FWChannelGroup`, Q-ME §2) before anything is sent.

## 6. Adding a model (checklist)

1. Create `kext/src/models/<Model>/`, deriving from `FWRev1AudioDevice` or `FWRev2AudioDevice` as the reference does (`classes.md`).
2. Add the alias class `com_m_audio_<Model>Device` (abstract, no behaviour) and the concrete `com_openaudio_<Model>Device`.
3. Implement the §3 virtuals from the reference decompiles of that class. Put tables in `<Model>Tables.h` with their binary addresses.
4. Add an `Info.plist` personality: `IOClass`, `IOProviderClass FWMetaNub`, the product name, `IOUserClientClass com_openaudio_FWUserClient`. Add a `MakeAppleFWAudioGoAway` entry if the model needs one.
5. Add a conformance row for the model (meter slots, channel counts, kDeviceSpecs row), plus host tests for the frames and tables.
6. Panel: the model's channel layout and controls (see the panel manual). Helper: nothing, because it matches the generic base class.

## 7. Where the evidence is

Class list and vtables: `docs/system-map/A1-ghidra/ghidra-exports/{classes.md,vtables-*.txt,functions.tsv}`. Decompiles: `…/decomp/`. Disassembly: `…/otool-exports/mafw.asm`. Per-topic notes:

| Note | Topic |
|---|---|
| Q-ME | mixer framework |
| Q-MC / Q-START | AV/C, CMP, start |
| Q-AP0 / Q-AP0b / Q-AP0c / Q-AP0d | Rev2, status packet, AV/C mixer, converters |
| Q13 | engines |
| Q-UC | user client |

Resolutions: R93-R98 in `PROVENANCE-RESOLUTIONS.md`.
