# A2 Panel queue — resume index

Working directory for Part A2 — the Carbon `M-Audio FireWire.app` control-panel binary (i386, 2.55 MB, Metrowerks PowerPlant framework).

**Target binary:** `_extracted/panel/M-Audio FireWire.app/Contents/MacOS/Panel`.

**Toolchain:** Ghidra 12.1.4 headless, project under `ghidra-project/panel-app`, exports under `ghidra-exports/`. Scripts in `scripts/` are reused from A1 but write to the project-local `ghidra-exports/` (not A1's).

**Initial numbers:** 8619 functions, 629 classes (vs A1 kext's 2862 / 222).

## Class landscape (relevant subset)

**Core app framework:**
- `CControlPanelApp` (45) — main app, event loop, device-list owner
- `CDeviceList` (20) — IOKit device enumeration + hotplug
- `CDeviceSettings` (47) — base for per-device settings state
- `CDeviceSettingsWindow` (61) — base for per-device settings window
- `CDeviceWindowManager` (19) — window lifecycle

**Per-device layer (one set per supported M-Audio device):**
For each of FW1814 / FW1212 / FW410 / FWAdat / FWDJ / FWM4 / FWMini / FWNebula / FWOz / Solo:
- `CFW<N>DeviceSettings` (~35) — kext interface + state
- `CFW<N>HardwareView` (~44) — hardware tab (clock source, sync source, phantom power, nickname)
- `CFW<N>MixerView` (~26) — mixer tab
- `CFW<N>OutputView` (~25) — output routing tab
- `CFW<N>GlobalSettingsView` (~17) — sample rate, buffer size
- `CFW<N>MiniMetersView` (~17) — level meters
- `CFW<N>MixerChannelPairView` (~15) — single channel-pair strip
- `CFW<N>HPOutChannelPairView` (~16) — headphone-out strip
- `CFW<N>HPABSourceDialog` (~32) — headphone A/B source picker

**Device-agnostic widgets:** `CClockControlView`, `CAboutView`, `CAboutWindow`.

**PowerPlant base classes:** `LAction`, `LWindow`, `LView`, `LControl`, `LCommander`, `LListener`, etc.

## Q queue (seed)

| # | Question | File | Status |
|---|---|---|---|
| P1 | Device discovery + user-client open path (`CControlPanelApp` + `CDeviceList` → `IOServiceOpen`) | `P1-device-discovery.md` | done |
| P2 | Notification receive path — Mach port on `FWKIPCUserClient` (closes A1-Q14a) | `P2-notification-receive.md` | done |
| P3 | Key-string table used for setProperties / mixer payloads (closes A1-Q15a) | `P3-property-key-strings.md` | done |
| P4 | `MAMIDIPacket` struct layout + Send/Recv path (closes A1-Q11a) | *n/a — reclassified to A3 MIDI plugin pass* | deferred |
| Q15b | Which IOService in the kext dispatches non-`MASampleRate` property writes? (opened by P3) | `../A1-ghidra/Q15b-setproperties-dispatch.md` | done — `FWPropertyDict::SetProperties`; no separate `FWMixer::setProperties` |
| P5 | `CFW1814HardwareView` — clock source + sync source + optical type + MOM + ADAT-4ch UI → kext RPC mapping | `P5-hardware-view.md` | done |
| P6 | `CFW1814MixerView` — topology rebuild + per-rate gating + Solo (level/pan/mute live in base `CMixerView`) | `P6-mixer-view.md` | done |
| P6-fup1 | Decode `CMixerView::ListenToMessage` / `CMixerChannelPairView::ListenToMessage` for level/pan/mute/aux-send cookies | *closed by P7* — band `0x5ea..0x5ff` fully mapped | — |
| P6-fup2 | Confirm `0x601` + `0x615` are kext-published-only (no SetCPSetting anywhere) | *closed by P7* — `0x615` read-only; `0x601` is bidirectional (P8 writer) | — |
| P7 | `CFW1814DeviceSettings` state model — full cookie↔field table + dispatcher chain | `P7-device-settings.md` | done |
| P7-fup | Find the cookie→NSString translator | `P7-fup-translator.md` — `CXDevice::{Get,Set}HW[Indexed]Setting` is the seam (CMiddleWareDevice vtable override, not an LBroadcaster listener). 8 new scalar NSString keys + bus-suffix family surfaced. | done |
| P8 | Sample-rate / buffer-size change path (lives in `CHardwareView`, not Global Settings — plus the actual FW1814 "Global Settings" = Virtual Rotary Assignment) | `P8-rate-and-buffer.md` — rate uses CoreAudio `setNominalSampleRate:` (not property); buffer-size is UI-local ladder keyed by cookie `0x5ff`; 2 new RO cookies `0x603`/`0x604` for rate list | done |
| P8-fup | Find NSString pair for the rate-list cookies `0x603` (count) / `0x604` (array) — needed for kext property schema | *closed by `P8-rate-and-buffer.md` §10* — no NSStrings; cookies are CoreAudio-synthesized from `[audioObject supportedSampleRates]`; no kext property needed | done |
| P9 | Peak-meter dispatch — polling model (not Mach push), FW1814 SW-return-not-embedded override, cookies 0x60d/0x60e | `P9-peak-meters.md` — pump at 30 Hz; cookies 0x5e9/0x5f3/0x601; corrects P2 bodyType 1/2 claim | done |
| P9-fup1 | Confirm fetch path — IOConnectCallScalarMethod vs clientMemoryForType(1) via xrefs on `_IOConnectMapMemory` | *closed by `P9-peak-meters.md` §7 (revised by fup3)* — shm IS the peak ring, read by GetHWSetting(0x5e9/0x5f3) with cookie-indexed slicing + log10 dB conversion | done |
| P9-fup2 | Decode `CX1814Device::DoTask` to find SW-return peak cookie (FW1814 override) | *closed by `P9-peak-meters.md` §7.4* — no override; SW-return peaks live at a FURTHER OFFSET in the same shm, past the output region | done |
| P9-fup3 | Decode `CXDevice::GetHWSetting` @ 0x6e2ca to see what the type-1 shm snapshot at `this[0x68]` is actually used for | *closed by `P9-peak-meters.md` §7.5* — shm is the live peak ring; layout `[inCh | outCh | swRetCh]` u32 slots; also corrects fup1 and reclassifies A1-selector-0x1fa | done |
| P2-correction | Downgrade P2 bodyType 1/2 wording — only bodyType 0 is handled; peaks/rate-lock are pulled | *applied inline in `P2-notification-receive.md`* | done |
| A1-selector-0x1fa | Kext userclient — selector `0x1fa` semantics | *closed by `P9-peak-meters.md` §7.3 (revised)* — vestigial ProTools no-op (A1-Q11 `EntryProToolsOpen`); mirror the no-op stub; MUST NOT be blocking | done |
| P10 | AC3 passthrough UI toggle (A1-Q12 said FW1814 inherits SupportsAC3=0 — expect no-toggle confirmation) | `P10-ac3-passthrough.md` — no panel UI exists; kext-only 1-line override `SupportsAC3=1` on FW1814 enables it; deferred to Part D | done |
| P-ClockUI | Hardware-tab widgets → `MAClockSource` bits (Q-CLOCK-UI) | `P-ClockUI-encoding.md` | **done 2026-10-14** — `ChangeDigitalSourceAttribute @ 0x71fea` RMW of MAClockSource; bit 2 port, 3 opt-in ADAT, 4 opt-out ADAT, 5 S/MUX, [1:0] sync, 30 gate |

## How to resume

1. Pick the next unblocked `P*` from the table.
2. Grep `ghidra-exports/functions.tsv` for the method, note the address.
3. Add the address to a scoped Ghidra Java script under `scripts/` (follow `A1-ghidra/scripts/DecompQ15.java` as template).
4. Run `/opt/local/share/java/ghidra-12.1.4/support/analyzeHeadless ghidra-project panel-app -process -noanalysis -scriptPath scripts -postScript <Script>.java > /tmp/<name>.log 2>&1`.
5. Decode against System V IA-32 ABI (stack-based args, caller cleans up; `this` passed on stack for `__thiscall`-equivalent — PowerPlant uses stdcall-ish variant).
6. Write `P<n>-<topic>.md` with pseudocode, kext cross-reference, clone implication.

## Known binary quirks

- **i386 Mach-O** — all pointers are 4 bytes, not 8. Watch for size differences vs the A1 kext decompile output.
- **PowerPlant classes** (`LAction`, `LWindow`, etc.) are Metrowerks CodeWarrior C++ runtime. Virtual tables use the classic MWCPP layout; `vtable[0]` is a RTTI pointer, methods start at `vtable[1]` or `vtable[2]`.
- **Carbon event handlers** are often registered via `InstallEventHandler` with a trampoline — real logic is in a static C function that unpacks the event, then calls the C++ instance method.
- **IORegistryEntryCreateCFProperties / IORegistryEntrySetCFProperties** are the primary kext-interface channel — selectors from A1-Q11 may not be used at all if the panel uses property-based control exclusively.


## Panel build and screen findings (2026-10-16)

| ID | Question | Document | State |
|---|---|---|---|
| P12 | v1.9 panel screens, dependencies, rate behaviour, user statements | `P12-panel-screens-v19.md`, `screenshots/` | done |
| PN1 | Approved panel wireframe | `PN1-wireframe.html` | approved 2026-10-16 |
| PN2 | Control → MA* key map, 54-key catalog | `PN2-control-key-map.md`, `MaKeys.swift` | done (5 smaller open items in P13 §7) |
| P13 | Panel translator decompile: numbering, rotary mask, ranges, headphone cookies, peak map/dB, balance rule, About tab | `P13-translator-index-map.md`, `ghidra-exports/decomp/pn2__*.c` | done |
| P14 | Clone panel architecture, rules, open items | `P14-clone-panel-design.md` | in progress |
