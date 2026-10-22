# Capture 98 — FW1814 S/PDIF lock and rate field vs. the source's rate (SL rig, 2026-10-22)

Rig `fw1814-sl` (10.6.8, official driver 1.9 + panel 1.4), FW1814, engine idle (no audio client). Clock source set by the user in the official panel: **External – digital, S/PDIF coaxial**. Source: Line 6 POD X3 S/PDIF out (always on, master only; rate changed by the user on the POD, which steps 96 → 88.2 → 48 → 44.1 and back — user, 2026-10-22). 1814 rate changed by the user in the official panel.

Tool `openaudio-mafw-bebob/tools/slfwread.c`, new mode `block1814` (read-only block read of the 88-byte status packet at `0xFFC7:0x00600000`; word 20 = bytes `0x50..0x53`, decoded as the kext does: little-endian load of the wire bytes, lock = bit 24 = byte `0x53`, FDF = bits 16..18 = byte `0x52`). Built on the dev Mac (15.2 SDK) with `-arch x86_64 -mmacosx-version-min=10.6 -fno-stack-protector -D_FORTIFY_SOURCE=0 -Wl,-no_pie` (without these flags the binary segfaults on 10.6.8); on the rig as `~/fw1814-captures/tools/slfwread-b1814`. Nothing is written to the device.

Bytes below are `byte 0x52 (rate field) / byte 0x53 (lock)`; rate field: 1 = 44.1, 2 = 48, 3 = 88.2, 4 = 96 kHz, `ff` = none (same encoding as the Audiophile, capture 93).

## Results

| 1814 rate | POD rate | bytes | `MASampleRate` / `MARateLocked` (official kext) | file |
|---|---|---|---|---|
| 48 | 96 | `ff 00` | 48000 / 0 | first read of the session (not saved; reported in the conversation) |
| 96 | 96 | `04 01` | 96000 / 1 | `status-follow-…`, reads 0-260 |
| 96 | 88.2 (passing) | `03 01` | — | `status-follow-…`, reads 263-265, 333-334 |
| 96 | 48, 44.1 | `ff 00` (after `03 00`) | — | `status-follow-…`, reads 266-331 |
| 48 | 44.1 | `01 01` | **44100** / 0 | `step-reads.txt` step 1 |
| 48 | 48 | `02 01` | 48000 / 1 | step 2 |
| 48 | 88.2 | `02 00` | 48000 / 0 | step 3 |
| 48 | 96 | `02 00` | 48000 / 0 | step 4 |

`status-follow-20261022-192349.txt`: 480 reads, 250 ms sleep (the per-line timestamps are flush times from a buffered pipe, not read times; use the read index). `range-switch-20261022-193128.txt`: 400 reads at 103.5 ms each (measured: 50 reads in 5.18 s), POD at 96, the user switches the 1814 48 → 96 kHz in the official panel: `02 00` (0-281) → transitional `ff 00` / `ff 01` / `04 00` (282-340) → `04 01` from read 341. Change start to stable lock: 59 reads ≈ **6.1 s** with the official driver's rate-change sequence.

## Reading (synthesis from these reads; one source device, one interface)

* Within the 1814's current speed range (44.1/48 = single, 88.2/96 = double) the lock byte is 1 and the rate field is the **source's exact rate**, also when it differs from the 1814's rate (48 with a 44.1 source → `01 01`; 96 with an 88.2 source → `03 01`).
* Outside the range the lock byte is 0 and the rate field is stale (`02 00` with the 1814 at 48) or `ff`. It carries no information about the source.
* During a rate change the device shows `ff 01` (lock set, no rate) and `04 00`; a follower must only act on lock = 1 **and** a valid rate field, stable over several polls.
* The official 1.9 driver publishes the detected rate as `MASampleRate` (44100 in step 1) but does **not** change the engine rate; `MARateLocked` = lock && rate field == engine rate (as decoded in `A1-ghidra/Q15d-marateLocked-xref.md`).
* **Unverified:** 32 kHz and 176.4/192 kHz sources (the POD does not offer them); optical S/PDIF and ADAT sync; word clock; the Audiophile (same encoding at fixed rates in capture 93, range behaviour not tried); behaviour while streaming; our 0.2.x/0.3.0 driver (it decodes the same word the same way: `FW1814Device.cpp:371`, `FWAudioDeviceProps.cpp:207`).

Rig left with the 1814 at **96 kHz, external S/PDIF coaxial, locked** (the user's settings for this test; it was 48 kHz external unlocked before).
