# Capture 95 — AP-1.4: renamed driver (OpenAudio BeBoB MAFW 0.2.0) with the FW1814 on the iMac (2026-10-19)

iMac build PC (10.13.6), FW1814 GUID `D6C040012337F`, analog loopback cable Out 3 → In 1. Test kext `com.openaudio.OpenAudioBeBoBMAFW` 0.2.0 (UUID `46A0EED6-9604-3E28-ABAE-B2E881F4416B`, built from `openaudio-mafw-bebob/` at AP-1.1, = the 0.1.0 code renamed) loaded from `/tmp` with `openaudio-mafw-bebob/scripts/load-test-kext.sh`; restored afterwards to the installed 0.1.0 (UUID `C703D75F…`).

Logs:
* `ap14-201351.log` — first attempt: load OK, `oa-fwhelper --once` → `MAFirmwareStart` 0x0, device "FireWire 1814" 48 kHz locked; loopcheck **no signal**. Diagnosis (same session): also no signal with 0.1.0 restored; the driver's meter page showed Output 3 at 16384 and Input 1 at 0 → the input was not set to line (user, 2026-10-19). Not a driver result.
* `ap14-run2-202135.log` — second run after the user switched the input to line: load OK; new helper in normal mode replayed the saved settings (`MAOutputLevels` −80 ×4 = the saved mix, factory would be 0); outputs 3/4 raised to 0 dB for the test only; `loopcheck 34` with afplay Funk, Glass + Funk, empty-trash, Ping as a second client: **events 0, loop level 0.495** (capture 88 reference: events 0, level 0.498); outputs 3/4 back to −80; 0.1.0 kext + helper restored; saved levels −80 ×4.

Notes: (1) The first run of `ap14-run2` was interrupted by the user after the load; the remaining steps were run separately (same log). (2) The installed 0.1.0 kext on the iMac (`C703D75F…`, 2026-10-17) is older than the current 0.1.0 source (fresh build UUID `FC8AD9DA…`, includes R75/R76); the installed panel equals the current source (MD5 `efcc28b6…`).
