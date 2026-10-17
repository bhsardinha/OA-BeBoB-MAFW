# P-ClockUI — Hardware-tab widgets → `MAClockSource` bits (Q-CLOCK-UI)

> **Correction (2026-10-14) — provenance audit resolutions** (evidence: [PROVENANCE-RESOLUTIONS.md](../PROVENANCE-RESOLUTIONS.md)).
> - 'Value 3 also calls SetDigitalInputActive(false)': no such symbol; the engine slot `vt[0xce8]` is `SetExternalSyncEnabled(bool) @0x11bc0`.


**Status:** done 2026-10-14. Sync-radio bit values for the v1.5.2 panel are **unverified** (see §2). Answers BUILD-MANUAL §2.G Q-CLOCK-UI.

**Sources:**
- v1.5.2 panel (`_extracted/panel/`): Ghidra read-only decompile of `CXDevice::ChangeDigitalSourceAttribute @ 0x71fea` (scratchpad run 2026-10-14); `P7-fup-translator.md` §3.1 cookie table; `P5-hardware-view.md` (widget IDs, `AdaptPortsToSettings`).
- v1.9 panel live captures: B5 §10 (caps 66/68/69).
- Kext v1.11.x: `SetClockSourceInternal @ 0xe25c` raw `0xe371..0xe425` (A1-Q5 Correction 2026-10-14).

## 1. The write primitive (v1.5.2 panel)

```c
// CXDevice::ChangeDigitalSourceAttribute(mask, bits, set, gate) @ 0x71fea
old = Get32BitNumberProperty("MAClockSource");
new = ((old & 0x3fffffff) | gate) & ~mask | (set ? bits : 0);   // gate = 0x40000000 at every call site (P7-fup §3.1)
NSLog("setclocksource %lX", new);
Set32BitNumberProperty("MAClockSource", new);                    // → IORegistryEntrySetCFProperties (B5 §10)
```

Every digital-I/O widget is a **read-modify-write of one bit field** of `MAClockSource`. Bits 30–31 are cleared and the gate is re-applied on every write.

## 2. Widget → bit table

| Widget (P5 IDs) | Panel cookie | Call | Bit(s) | Value |
|---|---|---|---|---|
| Sync Source radio `0x1452..0x1455` | via base `CHardwareView` (P5) | — | **[1:0]** | B5 §10 (v1.9 capture): `0x1452` internal–muted = `11`, `0x1453` internal–unmuted = `00`, `0x1454` external–digital = `01`, `0x1455` word clock = `10`. **Unverified for v1.5.2.** Check: the `CHardwareView` sync-button handler in the panel binary. |
| Active input (S/PDIF port) radio | `0x60f` | `CDSA(4, 4, in == 1)` | **2** | set when setting == 1. B5 §10: 1 = coaxial, 0 = optical |
| (also) | `0x600` | `CDSA(4, 4, bool)` | 2 | same bit, bool form |
| Optical-IN radio `0x157d` (`0x157e` S/PDIF, `0x157f` ADAT) | `0x60d` | `CDSA(8, 8, in == 2)` | **3** | 1 = ADAT |
| Optical-OUT radio `0x1587` (`0x1588` S/PDIF, `0x1589` ADAT) | `0x60e` | `CDSA(0x10, 0x10, out == 2)` | **4** | 1 = ADAT |
| ADAT 4-ch (S/MUX) checkbox `0x159b` | `0x611` | `CDSA(0x20, 0x20, bool)` | **5** | 1 = on |
| `0x5ff` | `0x5ff` | `CDSA(?, ?, bool)` | **unverified** | P7-fup lists the mask as "—". Check `SetHWSetting` case `0x5ff`. |

**Correction to BUILD-MANUAL §2.G (2026-10-14):** its bit list said "bit 2 = S/PDIF port (coax=0 / optical=1)". The capture-backed B5 §10 says 0 = optical, 1 = coaxial. Use B5.

## 3. UI gating (pure UI, no RPC — P5 `AdaptPortsToSettings`, corrected 2026-10-09)

- When `bothPairActive` (cookie `0x5fe`) is false, all four port groups are disabled.
- Otherwise, the optical-in and optical-out groups are always enabled.
- The active-input group is disabled when optical-in = ADAT.
- The ADAT-4ch checkbox is enabled when optical-in = ADAT **or** optical-out = ADAT.

## 4. What the kext does with each bit (v1.11.x)

| Bit | Kext consumer | Wire effect |
|---|---|---|
| 30 | gate in `SetClockSourceInternal` (`cmp 0x3fffffff`, `0xe2ea`) | below it, `0xE00002C2` and nothing is sent |
| [1:0] | `SetClockSourceInternal` | HSCI byte 6 = `{0→3, 1→1, 2→2, 3→0}` (`0xe371..0xe3d1`). Value 3 also calls `SetDigitalInputActive(false)` (B5 §10) |
| 3 | `SetClockSourceInternal` | HSCI byte 7 = bit 3 (`0xe3e2`) |
| 4 | `SetClockSourceInternal` | HSCI byte 8 = bit 4 (`0xe3fe`); also `SwitchDigitalSignals @ 0xe5dc` at the next format resolve (B5 §10) |
| 5 | `SetClockSourceInternal` | HSCI byte 9 = bit 5 (`0xe41a`); also `SwitchDigitalSignals` |
| 2 | not in the HSCI payload | **unverified consumer.** Check: xrefs to `[+0x144]` with `test $0x4` / `and $0x4` in `mafw.asm` |

> **Correction (2026-10-14) to B5 §10's kext-side notes:** "HSCI payload bytes 7, 8, 9 are hardcoded zero" and "bits 4, 5 … applied lazily" are superseded. Bits 3/4/5 go on the wire **immediately** in the same HSCI write as the sync source (A1-Q5 Correction 2026-10-14). The capture-derived bit meanings in B5 §10 stand.

## 5. Clone implications (reasoning)

The panel must do the same read-modify-write on one 32-bit word, always OR in the gate, and push the word via `IORegistryEntrySetCFProperties`. Because the kext re-sends the whole HSCI envelope on every write, each control change re-asserts the sync source, which mirrors the reference. The factory default is `0x40000003` (Q3b).
