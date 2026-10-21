# HAP — Helper, firmware and MIDI plug-in for the FireWire Audiophile (phase 0, helper/MIDI step)

Status: **resolved 2026-10-19** for the mapping; hardware behaviour of the Audiophile's bootloader **unverified**. Tracker Itens `AP-0.8.*` (helper/MIDI rows), Build `AP-4.*`, `AP-6.1`; resolution R83.

> **Update (2026-10-20, R109):** §2's "one device class `com_openaudio_FW1814Device`" is superseded in `openaudio-mafw-bebob`: the helper matches the generic base `com_openaudio_FWAVCAudioDevice` (`firmware-helper/main.c:25`), which IOKit matching resolves to every model's device by inheritance — the same coverage as the reference's 8-entry class table (H10) without listing models (plan v4 "generic first"). The nub watch (`com_openaudio_FWMetaNub`, any model) and `MAFirmwareStart` are unchanged; persistence is per GUID and key-agnostic (`Settings.c`), so a second model needs no helper change.

## 1. Reference helper is product-agnostic

`A4-helper/H10-multi-product-dispatch.md` (2026-10-09): the reference daemon watches `com_m_audio_FWMetaNub` plus an 8-entry class table (`__DATA.__const 0x97f4..0x9814`, includes `com_m_audio_FWAudiophileDevice`); no per-product code path. Firmware-file selection (H4): strip `" Bootloader"`, aliases `"FW"`→`FW 410` and `"FWSolo"`→`FW Solo`, then every `*.bcd` under `Firmware/<name>/` — for the Audiophile `Firmware/FW Audiophile/FWAP.bcd` (`_extracted/helper/Firmware/FW Audiophile/`).

## 2. What the clone helper does today (`openaudio-fw1814/firmware-helper/`)

* `main.c:24-25`: watches `com_openaudio_FWMetaNub` (any model) and **one** device class `com_openaudio_FW1814Device`; on every nub match writes `MAFirmwareStart = "YES"` (`main.c:52`), which makes the kext send *Go* to a bootloader and re-register the nub (Q-SDK5, Q-P3 §4).
* The reference's firmware-**update** branch (bundled `.bcd`, date comparison, `MAFirmwareHalt`/`MAFirmwareUpgradeImage`) is **not implemented** (user scope cut 2026-10-15, no flash writes; `main.c:8-10`). The clone never reads a `.bcd`.
* Settings persistence (`Settings.c`): per-GUID (`%llX`) in `com.m-audio.device-settings.firewire`, payload = the kext's `MAMetaAllProperties` (`Settings.c:94`) — model-agnostic.

**Consequence for the Audiophile:** the helper needs only a device-class **table** (the reference's 8-entry loop, H10) instead of the single `kDeviceClass`. The bootloader start path is the kext's generic BeBoB bootloader protocol (`FWMetaNub`, Q-P3), shared by all models.

**Plan impact (needs the user's decision):** plan tasks AP-4.1 (`.bcd` reader honouring `FWAP.bcd`'s header) and AP-4.4 (ship firmware files) exist only for the firmware-update branch the project does not implement. Proposal: cancel both (scope cut), keep AP-4.2/4.3. `audiophile-manual-reference.md` §6 (FWAP.bcd layout) stays as documentation.

**Unverified:** that the Audiophile's flash holds a runnable image and auto-boots like the 1814 (~3 s, Q7 Correction 2026-10-15). Evidence so far: the SL rig runs it (capture 89 `MAFirmwareInfo` dates `2007-05-04 10:22:12` / `2003-10-06 10:00:41`); `snd-bebob` `bebob_maudio.c:10-24` (firmware ≥ 5058 boots from flash after a cue). Check: AP-7.1.

## 3. MIDI plug-in (M-Audio's, reused unmodified)

* `A3-midi/M1-factory-and-driver-interface.md:174-186`: the plug-in calls `IOServiceMatching()` on a class list that includes **`com_m_audio_FWAudiophileDevice`**; `M2-device-discovery.md`: one entity per device, sources/destinations from `MAMIDIInputPorts`/`MAMIDIOutputPorts` (arrays of names), name from `IOAudioDeviceName`.
* The 1814 clone makes the unmodified plug-in find it with an alias class `com_m_audio_FW1814Device` between `com_openaudio_FWRev1AudioDevice` and `com_openaudio_FW1814Device` (`openaudio-fw1814/kext/src/FWAudioDeviceChain.h:183-191`; verified loading on 10.13.6, R58).
* Audiophile needs: alias class **`com_m_audio_FWAudiophileDevice`** in the same position (over the Rev2 base); `MAMIDIInputPorts = ("1")`, `MAMIDIOutputPorts = ("1")`; `IOAudioDeviceName = "FireWire Audiophile"` — all three values as on the SL v1.9 registry (capture 89). Engine side: `SetupAudioEngine` sets 1 MIDI in / 1 MIDI out (Q-AP0 §2.5 / AP-0.7.1).
* Constraint (already true for the 1814): the official M-Audio kext must never be installed together with ours (same class names).
