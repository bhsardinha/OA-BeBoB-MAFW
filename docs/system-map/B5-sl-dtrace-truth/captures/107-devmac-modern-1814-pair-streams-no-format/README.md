# Capture 107 — stereo-pair devices without channels: pair streams have no current format (dev Mac, Modern 0.4.0, FW1814, 2026-10-27)

Machine: dev Mac (MacBookAir7,2, macOS 14.8.9, `system.txt`), IOAudioFamily **540.3**, driver **0.3.0** (package 0.4.0 Modern, installed from the `.pkg`; `kextstat.txt`). FW1814 GUID `0xD6C040074B9FB` on the Thunderbolt-to-FireWire adapter, ADAT mode (16 in / 12 out), **48 kHz**, the multichannel engine running with one client (`main-engine.txt`). The user had switched on "Show stereo pairs as separate devices" and reported: the devices appear, but no input or output channel is attached to them. Read-only: nothing loaded, unloaded or switched.

| File | Content |
|---|---|
| `ioreg-pair-engines.txt` | the 8 `com_openaudio_FWPairEngine` (Analog 1/2, 3/4, 5/6 In, 7/8 In, ADAT 1/2 … 7/8) and their 14 `com_openaudio_FWPairStream` |
| `ioreg-main-streams.txt` | the two `com_openaudio_FWStream` of the multichannel engine (available-format arrays dropped) |
| `hal-system-profiler.txt` | `system_profiler SPAudioDataType` |

## Observations
* Every pair engine has `IOAudioSampleRate` = 48000 and every pair stream lists one available format with minimum = maximum = 48000. So `ResetIOFormat(48000)` ran: it is the only code that clears and re-adds those formats.
* **None of the 14 pair streams has `IOAudioStreamFormat`** (`grep -c '"IOAudioStreamFormat"' ioreg-pair-engines.txt` = 0). Both main streams have it (12 out, 16 in).
* CoreAudio lists the eight pair devices with **no input or output channels and "Current SampleRate: 0"**. The multichannel device shows 16 / 12 at 48000 (`hal-system-profiler.txt`).

## Cause (source reading, Apple IOAudioFamily 540.3 = the loaded version; `_refs/IOAudioFamily-src/`)
* `IOAudioStream::setFormat(fmt, callDriver)` → `setFormat(fmt, NULL, callDriver)` → `validateFormat(fmt, NULL, &desc)` = `validateFormat(…, audioEngine->getSampleRate())` (IOAudioStream.cpp:624-627). Then the 4-argument `setFormat` validates again against `audioEngine->getSampleRate()` when the format dictionary has no rate (:362-368). `validateFormat` refuses a format whose min/max rate range does not contain that rate (:262-268). On refusal it logs "IOAudioStream::setFormat - invalid format." and returns `kIOReturnBadArgument` (:526-530), and `IOAudioStreamFormat` is never set.
* The clone's `ResetIOFormat` mirrored the reference order (ref @0x1ab42: vt[0xa68]/[0xa58]/[0xa38] = clear/add/setFormat on each stream, **then** vt[0xb58] `setSampleRate`). The pair engine is still at the 44.1 kHz that `initHardware` set (ref @0x1aff8: `0xac44` → vt[0xb58]). So `setFormat` is refused whenever the master rate is not 44.1 kHz, and at every later rate change.
* The same rate check is in IOAudioFamily 206.5 (10.13, IOAudioStream.cpp:261), 183.4.11 (:149) and 170.4.16 (1.7.0, :149). The reference had the same refusal. On the SL rig the 10.6 HAL still gave the pair devices 2 in / 2 out at 96 kHz (capture 103). The macOS 14 HAL gives 0 channels.
* Capture 106 (iMac, Classic, FW410) tried the pairs at **44.1 kHz**, the one rate where the order does not matter. That is why they worked there.

## Fix (R141, driver 0.3.1)
`FWPairEngine::ResetIOFormat` now calls `setSampleRate(rate)` before the stream loop. The rest stays in the reference order. The built Modern kext calls vt[0xb00], vt[0xb58], then clear/add/setFormat, then vt[0xa80] (`otool -tV`, 2026-10-27).

## After the fix (package 0.4.1 Modern installed, 2026-10-27)
`after-0.4.1-kextstat.txt` (driver 0.3.1, UUID `93720671-5952-3724-BBFF-515F7B3D43CA`), `after-0.4.1-ioreg-pair-streams.txt`: **14 / 14 pair streams have `IOAudioStreamFormat`**; `after-0.4.1-hal-system-profiler.txt`: every pair device has its channels (Analog 1/2 and 3/4 2/2, Analog 5/6 and 7/8 In 2/0, ADAT 1/2 … 7/8 2/2) at 48000. User: "worked as intended". Not checked: other rates, a rate change with pairs on, Classic.
