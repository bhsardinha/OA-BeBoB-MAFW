# RATE_FOLLOW_PLAN v1: follow the rate of an external clock

**Status:** v1, 2026-10-22. **Implemented and working** (package 0.3.1, driver 0.2.2, panel 1.5; Modern edition on 14.8.9 with a FW1814 + POD X3, user report, capture 99, R131). Decisions by the user (2026-10-22): follow only while no client runs the device's IO; a "Follow external clock rate" setting, on by default; one automatic try of the other speed range per change. A clone feature: the official driver detects the source's rate but never follows it (capture 98). Evidence: `docs/system-map/B5-sl-dtrace-truth/captures/98-sl-1814-spdif-follow/README.md`.

## 1. What the hardware reports (capture 98, FW1814 + official driver 1.9, S/PDIF coaxial, POD X3 as master)

* The status packet's lock byte and rate field (FW1814 word 20; Audiophile quadlet 14) give the **source's exact rate** while the source is in the interface's current speed range (44.1/48 or 88.2/96), even when it differs from the interface's own rate (48 + 44.1 source -> `01 01`; 96 + 88.2 source -> `03 01`).
* Outside the range: lock 0, the rate field stale or `ff`.
* While the interface changes rate the status passes through `ff 01`, `04 00`, `ff 00` for up to ~6 s (official driver).
* **Unverified:** the Audiophile's range behaviour, optical S/PDIF, ADAT, word clock, 32 / 176.4 / 192 kHz sources, behaviour while streaming. (Our driver on the 1814: verified 2026-10-22, capture 99.)

## 2. Driver (kext)

* `DetectExternalSyncChanges(locked, fdf)` (reference behaviour unchanged) also publishes **`MAExternalSyncRate`**: the rate in Hz when the lock bit is set and the rate field is a valid FDF index (0..6), else 0. Only on change.
* Registry table only (`setProperty`), **not** the property dictionary: no user-client notification, so the helper does not persist or replay it (it persists every notified key, Settings.c `received`), and a replay can never leave a stale value.
* Generic (base class): both models feed `DetectExternalSyncChanges`.

## 3. Panel

* **`RateFollower`** (pure logic, unit-tested): per interface, ticked every 0.25 s by the panel controller (runs also when the panel is hidden / launched at login). Inputs: setting on, sync source = **External – digital** (the measured case; word clock is not followed: unverified), `MAExternalSyncRate`, CoreAudio nominal rate, available rates, `DeviceIsRunningSomewhere`, time.
  1. **Source seen:** the detected rate is stable for 1.5 s, differs from the current rate, is offered by CoreAudio and is at most 96 kHz -> set it through CoreAudio (the panel's rate path, PN.8), if no client runs IO; otherwise show "source at X kHz, follows when playback stops" and follow as soon as the device is idle.
  2. **Nothing seen:** no valid detection for 8 s (a lock takes up to ~6 s after a rate change, capture 98) and idle -> one try of the other speed range (48 <-> 96 kHz; 88.2 if 96 is not offered). If a source shows up there, rule 1 picks the exact rate. If nothing shows within 8 s, back to the starting rate; no further try until a source is seen again, the sync source is re-selected or the setting is toggled.
  3. After any change the follower waits 2 s before reading again. Inactive above 96 kHz (digital I/O is off there) and when the driver does not publish the key (older driver: the setting is greyed).
* **UI:** Hardware tab, Sample Rate card: checkbox "Follow external clock rate"; the status line shows the source's rate when it differs ("Source at 44.1 kHz - follows when playback stops"). Status menu: "Follow External Rate" toggle next to Sync Source; the rate / lock line names the source's rate.
* Setting: per Mac (`PanelPrefs`, default on).

## 4. Deviation note

The reference panel greys the rate while externally synced and leaves the engine at the old rate (B5 capture 80, C2 §3). Following is new behaviour, decided by the user (2026-10-22); the kext's reference behaviour (`MASampleRate`, `MARateLocked`) is unchanged.

## 5. Checks (on request, R123)

Host test of `RateFollower`. On the hardware (when the user asks): the 1814 with our driver, POD X3 at 44.1 / 48 / 88.2 / 96, the interface starting at 48 and at 96, idle and with a client playing.
