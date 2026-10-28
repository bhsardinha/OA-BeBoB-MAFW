# Capture 108 — two FW1814s on the iMac, Advanced window bench pass (2026-10-27)

Build PC (10.13.6, kext 0.3.0 installed), units `0xD6C040074B9FB` and `0xD6C040012337F` on the iMac's FW800 bus, both 48 kHz locked, no
audio client. Panel = Advanced window build (branch `panel-redesign`), run as `--snapshot DIR --advanced` (scratch store; every unit:
first opening -> DAW, then Console, Hybrid, Monitor = the unit's mixer as it was), mixer keys read back from the driver after each mode.

## First pass (root of this folder): 74B9FB slow
* 12337F: every scene landed (read back): DAW `MAHeadphoneSecondSource` 2, `MARotaryControlV2` 0x100000000 (Line 1/2); Console
  `MARotaryControlV2` 0x0 (**the empty knob mask is accepted by the kext** — closes the M3 "unverified"); Hybrid 0x100000000; Monitor back
  to the unit's values.
* 74B9FB: the writes returned 0x0 but the read-back 1.2 s later still showed the old values. `maprop` (now with `MA_GUID`) measured
  0.55 s for a 2-key batch on 74B9FB vs 0.02 s on 12337F, 5.8 s for a full dump write that included `MAClockSource` (its AV/C commands
  timed out, `r=0xe00002d6`, ~1 s each). The user also reported the A/B button not working on "the second unit".
* After a power cycle 74B9FB stayed on the bus as "Unknown Device" (PHY present, config ROM unreadable); six bus resets in 11 s preceded
  its disappearance. **The user found a bad contact in the unit's FireWire port** and reseated it with everything off.

## After the contact fix (`after-contact-fix/`)
* `maprop`: both units 0.02 s (2 keys) and 0.08 s (36-key mixer batch without the clock key).
* Advanced pass: both units identical and correct in every mode (log `adv-real4.log`, 8 writes all 0x0, read-back per mode).

**Correction (2026-10-27):** the first pass's hypothesis "the driver is not ready for two FW1814s" is withdrawn: the slowness, the AV/C
timeouts and (probably) the A/B button came from the bad contact. Two FW1814s on one bus now run the Advanced scenes correctly. Still
unverified: the A/B button on the second unit after the fix; streaming on both units at once; the Dual aggregate (M7).

## Later the same evening: 74B9FB would not lock (23:14-23:21), fixed by a power cycle
* Both units at 44.1 kHz (changed from 48 at ~23:11; 12 `SetClockSource` events 23:11-23:19 from the sync-source menu). 74B9FB:
  `MAClockSource` 0x40000018 (internal, digital inputs unmuted, ADAT in/out), `MARateLocked` 0; LOCK-DIAG alternating locked/unlocked
  ~600 times a minute; no bus reset in the period. 12337F: 0x4000001B, locked.
* The panel wrote no clock: only the Dual setup's "Set B to Word clock" button does (sync 2), and the scenes carry no clock key.
* The user power-cycled 74B9FB: it locked at 44.1 kHz with the same clock word, so the clock setting was not the cause. A slower toggle
  remained (~35 a minute, "locked 1 ext 44100" / "locked 0 ext 0"). **Correction (user, same evening):** the units are not cabled to
  each other by optical, and 74B9FB was on "internal - digital inputs unmuted": an unconnected optical input toggling its detection is the
  likely source (which unit logs it cannot be told: LOCK-DIAG lines carry no GUID). Driver to-do (diagnostics only, when the user asks):
  put the unit's GUID in LOCK-DIAG / CMP / HSCI log lines.
* Both units publish "host plug 0, slot 0" in their CMP traces (channels 0 and 1): **unverified** whether that is one shared host PCR;
  to check against the reference's multi-unit handling before any driver change (memory: check the reference for multi-device handling).
