# Conformance check (Build row IN.7, BUILD-PLAN §0)

`openaudio-fw1814/scripts/conformance-check.sh` (Python, static, no hardware or Swift/Xcode toolchain needed; run it on the dev Mac) compares the panel, the kext, the plist and the helper:

1. **Keys:** every `MA*` key of the panel catalog (`MaKeys.swift`) is registered by the kext (`AddArray`, `AddNumber32/64`, `AddFunction`, `AddChannelGroup`, `InitGroupProp` expansion) with a compatible kind; descriptor keys and the nub's firmware keys are checked as published literals; `MASampleRate` as the `setProperties` fast path; and the kext has no key the catalog does not know.
2. **Counts:** array element counts of the catalog equal the kext's (group templates in `FWMixer.h`, counts in the `AddArray` calls).
3. **Matching:** the panel's match class is a personality `IOClass`; `FW1814Device` carries `IOUserClientClass`; the helper's nub class is a personality `IOClass`; `MAFirmwareStart`/`MAFirmwareInfo` exist in the kext.
4. **Meter page:** 52 slots, SW-return slots at 38, output slots after the 18 input slots, `clientMemoryForType(1)`, panel maps type 1.
5. **Per-rate channel counts:** the panel layout (analog 8/4 + S/PDIF 2 or ADAT 8; ADAT halved at 88.2/96 kHz; 2 in / 4 out at 176.4/192 kHz) equals `FWRates.h`.
6. **CoreAudio controls** are created in `FWSettingsLevels::Init`.

Exit status 1 on any FAIL. Run it after every change to the catalog, the mixer groups or the plist.

**History:** the first run (2026-10-16) found one real mismatch, fixed the same day: `MAMonitorAuxOnOutput` has **3** elements in the kext (`(output.Count() >> 1) + 1`, as in the reference `FWSettingsAuxSend::Init` call) while the panel catalog said 10 (the reference translator's limit of 10 output pairs is a panel-side bound). It also found that `MAMetaVirtualRotateDataKnob` is a function property, not a number property, in the kext.

What it cannot prove (needs hardware): notification header layout on 10.13, HSCI byte order, registry `GUID`, real meter values; see `docs/INTEGRATION-READINESS.md` §4.
