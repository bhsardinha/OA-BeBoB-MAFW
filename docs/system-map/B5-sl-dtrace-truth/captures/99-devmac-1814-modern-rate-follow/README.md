# Capture 99 — "Follow external clock rate" on the FW1814 with our driver, Modern edition (dev Mac, 2026-10-22)

Dev Mac (MacBookAir7,2, macOS 14.8.9, Thunderbolt-to-FireWire adapter), package **0.3.1 Modern** (driver 0.2.2, kext UUID `2A89CF82-C090-3A0C-ACD5-29973A8C4267`, panel 1.5) installed by the user with the `.pkg` (Allow + restart). Interface: the **second FW1814**, GUID `0xD6C040074B9FB` (first FW1814 on the Modern edition). Source: Line 6 POD X3, S/PDIF coaxial, master; sync source External – digital, coaxial; "Follow external clock rate" on.

**User report (2026-10-22, verbatim):** *"worked perfectly. it detects if unlocked and sees if is single speed or double speed. than, it change if not matching the speed range. in the range it can detect the correct rate and auto lock. worked PERFECT"*

So, as designed (RATE_FOLLOW_PLAN_v1): out of range -> one try of the other speed range; in range -> the exact rate, locked. The rates and starting points the user tried were not listed one by one (not logged here).

`state-after-test.txt`: registry state read by Claude after the test (read-only): loaded kext = the package's, `MASampleRate` 48000, `MARateLocked` 1, `MAExternalSyncRate` 48000.

**Not verified here:** follow while a client plays (the follower waits by design), recording / MIDI / other functions of the FW1814 on Modern, the Classic 0.3.1 package, the Audiophile with the follower, optical S/PDIF, ADAT, word clock.
