# Glue — state

**2026-11-09. Glue 1.0.0 is published** with OpenAudio BeBoB MAFW 0.6.0 (GitHub release v0.6.0): disk images
`OpenAudio-Glue-1.0.0-classic.dmg` (macOS 10.13 to 10.15) and `-modern.dmg` (macOS 11 to 15, Intel). FireWire 1814 only. Branch
`glue` is merged into `main`. Quality over CPU (user). GUI: OA MAFW panel style, plug-in-simple (user).

**User, 2026-11-02 (G12): "the tests in the real high sierra iMac ran fine. everything works as should."** This is the user's
report on the build PC (10.13.6, two FW1814s). No capture of it is recorded.

| Phase | Status |
|---|---|
| 0 — checks | **done** (G1–G6) |
| 1 — DSP library + tests + bench | **done** (G7, G8); Colour v2 and the global Drive |
| 2 — kext glue tap | **released** in driver 0.4.0 (R143), with `ringGeneration` (G11) |
| 3 — AudioServerPlugIn | **released**; runs on hardware (G11, G12); FireWire 1814 only |
| 4 — Glue app | **released**; the main panel 2.2 has a menu item that opens it |
| 5 — installer + docs | **released**: Classic and Modern packages, a disk image with an Uninstall app (G13) |

## Open
1. **Modern edition** (macOS 11–15 Intel): built from the same binaries, **never run**. On demand.
2. **Core Audio SIGILL at restarts** (G11): unverified whether Glue adds to it (Apple code, also seen without Glue). On demand.
3. **A unit switched on or off while Glue is in use:** the main driver's bus-reset fix (main R144) was tried by the user with the
   Glue setup on 2026-11-09 (audio pauses and comes back, no panic); no capture of the Glue device across it is recorded.

## Dual (G9, G10)
The target setup is two FW1814s in the panel's Dual mode. The "Dual FireWire 1814 Glue" aggregate (Glue A, Glue B, FW1814 A,
FW1814 B) gives processed inputs 1–32, raw inputs 33–64 and outputs 1–24. The app creates it. The two units can sync over word
clock or ADAT (G10).
