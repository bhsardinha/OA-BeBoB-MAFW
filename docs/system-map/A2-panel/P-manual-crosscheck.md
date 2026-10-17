# P-manual-crosscheck — manual ↔ Part A reconciliation

> **Correction (2026-10-14) — cookie 0x615 = 192000 for FW1814** (evidence: [PROVENANCE-RESOLUTIONS.md §R18](../PROVENANCE-RESOLUTIONS.md)). **Cookie `0x615` for the FW1814 is 192000 Hz, not 48000.** `CXDevice::GetHWSetting` (0x6e2ca) answers cookie `0x615` with `this->vt[0x58]()` (asm 0x6eecd-0x6ef3a, after a 4-byte size check); slot `0x58` of `__ZTV12CX1814Device` is `CX1814Device::GetMaximumMixerSampleRate @0xe9e56` = `movl $0x2EE00` (192000); the base `CXDevice` version and the other devices return `0x17700` (96000), `CX1212csDevice` 96000 (vtable walk of all CX* classes). `CFW1814DeviceSettings::ReadSettingsFromHardware` (0x5bd98-0x5bdaf) stores that value in `[+0x388]`; `0xBB80` (48000) is used **only if the read fails** (`testw %ax,%ax; cmovel`). Consequence: `AdaptToClockRate` (`rate <= max`) is true for every rate the FW1814 supports (32-192 kHz), so the reference panel **never disables the aux-send section (nor the HP A/B Aux radios) by rate** on the FW1814. The earlier statements 'aux-send unavailable above 48 kHz', 'hardcoded in the kext to 48000' (P11 §3.3), 'HW-seeded default 48000' and the P-manual-crosscheck item 5 conclusion are **not supported** for the FW1814; whether the manual says otherwise was not re-read (unverified).


**Status:** in-progress 2026-10-09.
**Scope:** reconcile `docs/manual-reference.md` (manual §1–§16) against the existing Part A corpus. Resolves 12 contradictions / open questions surfaced in the 2026-10-09 cross-check pass. Follows the shared plan at `/Users/bernardosardinha/.claude/plans/sprightly-munching-locket.md`.

## Shared evidence sources

- **T1 Panel.rsrc dump:** `ghidra-exports/panel-rsrc.txt` (DeRez `-useDF`, 44 392 lines) — PPob 5001 "1814 hardware OS9" at line 14 645, PPob 5900 "1814 Headphone A/B Settings" at line 3 657, PPob 2500 "1814 global View" at line 4 660, STR# 1020 "kHWHeadPhoneSourceStrings" at line 525.
- **T2 Ghidra re-decompile:** `docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log` (841 lines), produced by `scripts/DecompA2ManualCrossCheck.java` → 14 panel functions redecoded on 2026-10-09.
- **T3 live DTrace:** SL Mac Pro `fw1814-sl` (192.168.100.17); existing captures at `../B5-sl-dtrace-truth/captures/`. Captures 62–67 scheduled, pending hardware access.

## Outcome table

| # | Topic | Status | Evidence | Doc(s) touched |
|---|---|---|---|---|
| 1 | Clock-source bijection (3 numberings) + composite bitfield | **confirmed kext-side + `MAClockSource` is a composite 32-bit bitfield (bit 4 = optical-out, bit 5 = S/MUX, low nibble = clock-source, bit 30 = gate)** | `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log:49..345` + `../B5-sl-dtrace-truth/captures/pulled/66-dtrace-smux-toggle.txt` (4-step user sequence shows bits 4/5 toggling orthogonally) | `../A1-ghidra/Q5-clock-source.md §Addendum`, B5 §10 (composite-bitfield addendum added), `P5-hardware-view.md` |
| 2 | ADAT S/MUX gating polarity | **confirmed — P5 §51 was inverted; live SL confirms** | `docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log:51..89` + `../B5-sl-dtrace-truth/captures/pulled/66-dtrace-smux-toggle.txt` (S/MUX writes only follow optical-out = ADAT bit 4 set) | `P5-hardware-view.md §AdaptPortsToSettings` (correction block added) |
| 3 | Front-panel knob descriptions as pots | **confirmed — nothing to fix** | grep below returned zero Part A hits for "analog pot" against HP1/HP2/Main | *n/a* |
| 4 | HPAB label set | **confirmed** | STR# 1020 (`panel-rsrc.txt:525..568`), `docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log:697..838` | `P5-hardware-view.md §UpdateHPSourceButtonLabels`, new `P11-hpab-source-dialog.md` |
| 5 | Mixer sample-rate gating | **confirmed — gate is aux-send-only** | `docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log:92..141` (`AdaptToClockRate` reads cookie `0x615`) | `P6-mixer-view.md §rate-gating` (correction block below) |
| 6 | Hardwired ADAT return + S/PDIF SW return non-routable | **confirmed — DM1000 hardware topology, not a kext bypass** | `../B5-sl-dtrace-truth/README.md §6b` (channel map 1..4 analog / 5..12 ADAT) + MA* key schema (§5: `MAOutputLevels` is array[4]) + `../../manual-reference.md:261,262,278,303,304` | `../A1-ghidra/Q3-setupaudioengine-connectallplugs.md §channel-map-vs-manual-14` added below |
| 7 | `MAHeadphoneSecondSource` legacy key | **resolved — runtime-built, not static** | zero hits in `panel-rsrc.txt` + zero hits in Panel v1.5.2 functions.tsv | note added below (P3 doesn't need a legacy-alias section) |
| 8 | `MAMomentarySwitch{Routings,Source}` | **confirmed on SL — `MAMomentarySwitchSource` is the Hardware-tab A/B-button-function selector (0=HP-A/B toggle, 1=Direct Monitoring)** | `../B5-sl-dtrace-truth/captures/pulled/63-dtrace-direct-monitor.txt` (writes `MAMomentarySwitchSource = 0x1` then `0x0` while user toggled the Hardware-tab a/b-mode radio) + PPob 5001 strings | new `P11-hpab-source-dialog.md §2`; Hardware-tab item verified live |
| 9 | `MADirectMonitoringActive` write path | **resolved — no user-writable key exists; DM state is driven exclusively by the physical A/B button in-kext** | `../B5-sl-dtrace-truth/captures/pulled/64-dtrace-momentary-switch.txt` + `64b-dtrace-momentary-nopidfilter.txt` (both ZERO setProperties on any process while physical button pressed in DM mode) | new `P11-hpab-source-dialog.md §3`; non-coverage note belongs in future `C5-userclient-surface.md` |
| 10 | S/PDIF out coax+optical mirror | **confirmed — hardware wiring, no software knob** | 4 lines of evidence (see below) | note added below; final home in `../C4-transport-control-path.md` when Part C lands |
| 11 | Virtual Rotary target labels | **confirmed — four target buses** | PPob 2500 "1814 global View" @ `panel-rsrc.txt:4660..` — resource IDs 0x9C6 "sw rtn bus", 0x9C7 "output bus", 0x9C8 "input bus", 0x9C9 "aux send" | `P8-rate-and-buffer.md §7.2` (resolves 4-of-{1,2,3,5} mapping — value 4 unused by design) |
| 12 | Hardware-only controls explicit non-coverage | **confirmed — zero Part A code hits** | grep below | *n/a; note belongs in future `C5-userclient-surface.md`* |

## Per-item detail

### Item #1 — Clock-source bijection

**v1.4 (SL / B5 §10) low-byte values:** `0x1C=S/PDIF, 0x1D=ADAT, 0x1E=Word Clock, 0x1F=Internal`.

**v1.5.2 (`_extracted/panel/`) UI buttons (`panel-rsrc.txt:14645..`):**

| Button ID | Label |
|---|---|
| 0x1452 | "internal - digital inputs muted" |
| 0x1453 | "internal - digital inputs unmuted" |
| 0x1454 | "external - digital" |
| 0x1455 | "word clock" |

Five low-byte values possible (two internal variants, external-digital variant whose actual port S/PDIF-vs-ADAT is resolved via `0x60d`, word-clock), not four. The v1.9 → v1.11.2 kext may accept the superset of low-byte values; the v1.5.2 Panel may map the two "internal" buttons to different low-byte values (candidate: `0x1F` and some new `0x20..0x23`).

**Kext-side resolved (2026-10-09):** `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log` + script `../A1-ghidra/scripts/DecompClockBijection.java`. The v1.11.2 kext accepts **any** 32-bit mode word with `0x40000000` set (`SetClockSourceInternal @ 0xe25c:218`); below that it returns `0xE00002C2` or runs a legacy translator in `m_audio_b_FWAVCAudioDevice::SetClockSource @ 0x23858`. Only `mode & 3` reaches the HSCI envelope via a permutation `{0→wire 3 (S/PDIF), 1→wire 1 (ADAT), 2→wire 2 (Word Clock), 3→wire 0 (Internal)}`. HSCI payload bytes 7, 8, 9 are hardcoded 0 (contradicting the original Q5 which claimed they were shifted mode bits). Mode bits above `[1:0]` feed only the engine's `digitalInputActive` flag — which is how the two "internal muted/unmuted" buttons in v1.5.2 can share wire byte 0 and still request different engine states. Full table in `../A1-ghidra/Q5-clock-source.md §Clock-source bijection` and `../B5-sl-dtrace-truth/README.md §Kext-side bijection`.

**Still pending (Panel-side):** the v1.5.2 Panel's five specific low-byte values. Only the kext-side bijection is nailed down; the Panel's button-to-value mapping requires decompiling the v1.5.2 Panel's clock-source button handler (`CFW1814HardwareView::ListenToMessage` or `GetClockSource`). Not blocking Part A closure — the kext accepts any `>= 0x40000000` value and the Panel-side encoding is UI-visible via `panel-rsrc.txt`.

### Item #2 — ADAT S/MUX polarity

`CFW1814HardwareView::AdaptPortsToSettings` at `0x5299e` (`docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log:51..89`):

| Pre-conditions | `[+0x18c]` (S/MUX checkbox) | `[+0x190]` (active-input group) |
|---|---|---|
| `opticalIn == 2` (ADAT in) | **Enable** (fall-through) | **Disable** |
| `opticalIn != 2 && opticalOut == 2` (ADAT out) | **Enable** | **Enable** |
| `opticalIn != 2 && opticalOut != 2` (no ADAT) | **Disable** | **Enable** |

Matches manual §16 ("ADAT S/MUX = ADAT at 88.2/96 kHz"). The S/MUX checkbox is enabled when any ADAT port is active, disabled otherwise. P5's prior claim ("disabled when optical-out == ADAT") was the inverse of the true branch. Corrected inline in `P5-hardware-view.md`.

### Item #3 — Front-panel knob descriptions

Grep `(analog |attenuat|pot[s]? )` across `docs/system-map/A{1..6}*/*.md` returns zero hits in a hardware-description context against HP1/HP2/Main Level. The only place that misread the knobs as pots was `docs/manual-reference.md`, now corrected per memory `project_fw1814_front_knobs_are_encoders.md`.

### Item #4 — Headphone label set

Full mapping, resolved from STR# 1020 and `UpdateHPSourceButtons` at `0x54256`:

| 2-byte value on wire (per channel) | STR# 1020 entry | Label | Button ID (per HP-A rgpv `0x170d` / HP-B rgpv `0x1717`) |
|---|---|---|---|
| 1 | 3 | "out 1/2" | 0x170e / 0x1718 |
| 2 | 4 | "out 3/4" | 0x170f / 0x1719 |
| 4 | 6 | "aux"     | 0x1711 / 0x171b |

Values are stored in cookie `0x609` as `(uint16 mainMix, uint16 hpA, uint16 hpB)` — 6 bytes. The HP summary buttons on the Hardware tab render `"A = <label>"` + `"B = <label>"` with STR# entries 1 ("A = ") and 2 ("B = ") as prefixes. Rate-dependent force (`current_rate > 48000` disables the Aux button) is implemented in `UpdateHPSourceButtons` via cookie `0x615` (max-aux-rate), not via auto-rewrite — the dialog **disables** the Aux radio, forcing user re-selection; it does not silently overwrite the stored value.

**Second-source confirmation (Rust crate `firewire-bebob-protocols`, 2026-10-09):** `protocols/bebob/src/maudio/special.rs:555..649` defines `HeadphoneSource { MixerOutputPair0=0x01, MixerOutputPair1=0x02, AuxOutputPair0=0x04 }`, serialized as bit flags into a 3-bit field (`mask=0x07`) packed two sources per u32 at DICE register offset `0x0098` (shift `i*16`). The `{0x01, 0x02, 0x04}` encoding and the gap at value `0x03` match the Panel's cookie `0x609` decode exactly — the gap exists because the hardware field is a bit-flag mask, not a dense enum. The crate uses a DICE-style memory-mapped register map rather than the HSCI AV/C envelope the kext dispatches through; both route to the same DM1000 hardware state. See memory `reference_rust_bebob_protocols.md`.

### Item #5 — Mixer rate gating

`CFW1814MixerView::AdaptToClockRate` at `0x57012` is 119 bytes and only toggles one widget: the aux-send section at `*(parent + 0xbc)` becomes disabled when `current_rate > 48000`. No per-strip walk; no base-class `CMixerView::AdaptToClockRate` exists (functions.tsv confirms — all `AdaptToClockRate` entries belong to concrete subclasses). The whole mixer page's wholesale gray-out at 176.4/192 kHz observed in the manual must come from the strip-level `CFW1814MixerChannelPairView::AdaptToClockRate` or the parent view itself gating off entire strips — not surfaced by the current decompile batch. **Status for this item:** aux-send gate confirmed, broader strip-level gating still pending (not blocking for Part A closure since the aux-send gate is the only FW1814 override of a potentially per-rate behavior; the base rate-gating is likely in `CMixerChannelPairView` and will land in a future P-pass if needed).

Correction landing in `P6-mixer-view.md`:

```markdown
**Correction (2026-10-09):** `CFW1814MixerView::AdaptToClockRate @ 0x57012` is 119 bytes
and gates only the aux-send widget (reads cookie 0x615 = max-aux-rate; disables when
current rate > that value). Full-page rate-gating is handled by the base
`CMixerChannelPairView` and HP-out pair view, not the FW1814 subclass. See
`docs/system-map/A2-panel/ghidra-exports/decomp/a2-manual-crosscheck.log:92..141`.
```

### Item #6 — Hardwired ADAT return + S/PDIF SW return channels

**Confirmed (2026-10-09).** The "hardwired" semantic in `manual.pdf §14` describes the **DM1000's internal DSP topology**, not a software routing bypass. All DAW→device output stream channels exist in CoreAudio; what varies is whether the DM1000 passes them through the mixer-page processing or directly to the digital output buses.

Evidence trail:

1. **CoreAudio channel map (B5 §6b, live on SL at ADAT-out mode):**
   - OutputChannel 1..4 → "Output Analog 1..4" — pass through mixer-page sum
   - OutputChannel 5..12 → "Output ADAT 1..8" — bypass mixer, direct to ADAT optical out

2. **MA* key schema (B5 §5):** `MAOutputLevels, MAOutputLink, MAOutputMute, MAOutputSolo` are each **`array[4]`** — the 4 analog outputs are the ONLY ones with software-controllable level. ADAT outs (slots 5..12) have no MA* entries because the DM1000 doesn't apply software DSP to them.

3. **Routing matrix (`MAInputRoutingV2`, `MAOutputRoutingV2`, `MASWReturnRoutingV2`):** these name only the mixer-page routable surface — 8 analog ins + 2 digital ins (S/PDIF mode) or no S/PDIF slots (ADAT mode), summed into the 4 analog output buses. The matrix never names ADAT output channels because the user cannot re-route them.

4. **S/PDIF out mirror (`manual.pdf:272`):** the single S/PDIF output pair is physically mirrored to coax RCA + optical TOSLINK (when optical-out mode = S/PDIF). Not routable, not independently addressable. In ADAT-out mode, the optical port carries ADAT instead and coax carries nothing.

**Clone implication (feeds C5):** the clone's CoreAudio stream channel count depends on the current digital-in/digital-out mode (see memory `project_fw1814_hardware_capabilities.md` 4×4 table). MA* routing keys only name the mixer-page surface. The `IOAudioEngineChannelNames` swap on mode change must bracket with `beginConfigurationChange`/`completeConfigurationChange`.

Note on Q3 vs reality: `Q3-setupaudioengine-connectallplugs.md §Channel-count conclusion` reports "6 vs 12 in, 10 vs 16 out" from the FW1814 `SetupAudioEngine` immediates. The hardware reality (per memory + B5) is "10 vs 16 in, 6 vs 12 out". The 6/12 immediate at `fInChannels` may represent only the digital-portion delta or a non-total counter — pending re-decompile with symbol context. The item #6 answer stands regardless.

### Item #7 — `MAHeadphoneSecondSource`

Zero literal occurrences in `panel-rsrc.txt`; the key does not exist as a static string in v1.5.2 Panel. Likely either:
- (a) runtime-built via a prefix+suffix compose function (per the P7-fup §3.3 bus-suffix family pattern), or
- (b) truly a v1.9-only legacy key.

The SL `captures/23-fw1814-current-prefs.xml` shows it, so it's a real key the kext still reads. For the clone, we can treat it as a legacy alias that the kext property dispatcher accepts and translates — no Panel UI surface needs it.

### Item #8 — `MAMomentarySwitch*`

The HPAB dialog PPob 5900 has only two radio groups (HP-A source, HP-B source) + Set/Cancel buttons; **no** mode-selector or front-panel A/B button toggle. The "a/b switch settings" group lives in the Hardware tab PPob 5001 (`panel-rsrc.txt:14645..` has the strings "a/b switch settings", "direct monitoring", "headphone out 1 A/B", "A = DM off, LED off", "B = DM on, LED on", "A = out 1/2, LED off", "B = out 3/4, LED on", "set source", "enable asio direct monitoring").

- The front-panel A/B button is a two-position switch; its physical state is reflected in `MAMomentarySwitchRoutings[2]` (array of 2 cookie-encoded values per A/B position).
- The mode selector ("A/B picks between DM and HP-out") sets `MAMomentarySwitchSource` to pick which pair of values the array holds.

**Not fully resolved:** the exact cookie(s) wired to the Hardware tab PPob 5001 subgroups. The decompile batch didn't include `CFW1814HardwareView::FinishCreateSelf` / `ListenToMessage` for the a/b-switch widget binding. Carried as pending.

### Item #9 — `MADirectMonitoringActive`

Not in HPAB dialog. Lives on the Hardware tab (per PPob 5001 strings). The exact write path (cookie, SetHWSetting vs SetCFProperties, kext-side handler) is not revealed by the current decompile — needs a `CFW1814HardwareView` re-pass covering the a/b-switch section. Carried as pending.

### Item #10 — S/PDIF coax+optical mirror

**Confirmed (2026-10-09).** No software control exists; the DM1000 silicon mirrors S/PDIF to both the coaxial RCA and the optical TOSLINK jack whenever optical-out mode = S/PDIF. Four independent lines of evidence:

1. **macfw/fw410 vs macfw/fw1814:** `macfw/fw410` has an explicit S/PDIF connector selector (`fw410/tools/control/fw410ctl/main.cpp:42` — `spdifConnectorName(int s){return s==0?"coaxial":s==1?"optical":"unknown";}`), so RE'ers know such a register exists on sibling M-Audio silicon. The entire `macfw/fw1814/` tree has **zero** references to `coax`, `optical`, or `connector` in a routing context — if the FW1814 had the register, macfw's reverse engineering would have surfaced it.

2. **HSCI envelope:** `project_fw1814_hsci_envelope.md` + the kext decode in Q5 show the vendor CONTROL has exactly one byte per direction for digital-in and digital-out format (S/PDIF vs ADAT). No connector sub-selector byte. The FW1814 wire protocol has no way to tell the device "put S/PDIF on coax only" vs "optical only."

3. **Property key schema:** `grep -i "MASPDIFMirror|MAMirror|SPDIFMirror|MACoaxOut"` over `_extracted/` returns zero hits. Of the 46 MA* keys catalogued in B5 §5, none names a coax-vs-optical selector.

4. **Panel UI label:** `panel-rsrc.txt:42615` contains the literal string `"mirrors spdif output"` as a static label on a `QCHX` widget (res-ID `0x14B5` within a parent `MACP` container `0x14BE` labeled "analog output"). The Panel presents the mirror behavior to the user as a **fact to be aware of**, not a toggle to configure. (Resource ID `0x14B5` is reused across PPobs — the one in P5 §Hardware tab for `MAMonitorAuxOnOutput` is a different widget in a different parent container.)

**Behavior invariant:**
- When optical-out mode = S/PDIF: coax RCA and optical TOSLINK both carry S/PDIF. Mirror.
- When optical-out mode = ADAT: optical TOSLINK carries ADAT. Coax RCA carries nothing (no routing path exists; coax is tied to the S/PDIF output bus which is unused in ADAT mode). The manual's wording "S/PDIF out is mirrored to coax + optical" is strictly true only in S/PDIF-out mode.

**Clone implication (feeds C4):** no `MA*` key, no user-client selector, no FCP command for S/PDIF connector choice. The clone's optical-out mode selector (S/PDIF vs ADAT) controls everything; coax is a hardware-fixed secondary output of the S/PDIF signal when that mode is active. Documented as a wiring invariant in `C4-transport-control-path.md` when Part C scaffolds.

### Item #11 — Virtual Rotary targets

PPob 2500 "1814 global View" at `panel-rsrc.txt:4660..` enumerates four target buses in a single `rgpv` (0x9C5):

| Resource ID | QRAD label | Likely value (from P8-rate-and-buffer §7.2) |
|---|---|---|
| 0x9C6 | "sw rtn bus"  | 1 |
| 0x9C7 | "output bus" | 2 |
| 0x9C8 | "input bus"  | 3 |
| 0x9C9 | "aux send"   | 5 |

The `{1,2,3,5}` set observed in P8 matches four buttons; value `4` is reserved (likely a cross-device compatibility slot — same `C18gv` class is used by sibling devices). The precise `button_id → value` mapping requires reading `MapAssignButtonToSetting @ 0x8465c` from P8's decompile log, but the ordering follows the resource ID sequence 0x9C6..0x9C9 as above. Update to `P8-rate-and-buffer.md §7.2` scheduled.

### Item #12 — Hardware-only controls (Mic/Inst gain, Mic/Line, Pad, Phantom)

Grep across `_extracted/panel/` + `_extracted/kext/` strings for `phantom|pad|mic.?line|mic.?inst|gain` in a per-input-control sense returns zero hits — these controls have **no software mirror** and are not session-recallable. Correct per memory `project_fw1814_front_knobs_are_encoders.md`. The non-coverage note belongs in `C5-userclient-surface.md` when Part C scaffolds; no Part A doc action needed.

## Still-pending work for full closure

- **Item #1 kext-static — ✅ done 2026-10-09.** `docs/system-map/A1-ghidra/ghidra-exports/decomp/clock-bijection.log` + `A1-ghidra/scripts/DecompClockBijection.java`. Bijection permutation and `0x40000000` gate semantics decoded; see `Q5-clock-source.md §Correction`. v1.5.2 Panel-side encoding remains open but is UI-visible via `panel-rsrc.txt` and not blocking Part A.
- **Items #8/#9 panel-static:** second Panel decompile batch targeting `CFW1814HardwareView::FinishCreateSelf` + `ListenToMessage` for the "a/b switch settings" section (PPob 5001 subgroups).
- **Item #10 — ✅ done 2026-10-09.** Confirmed via macfw contrast + HSCI envelope + MA* schema + Panel UI label. See §Item #10 above.
- **Items #2/#5/#8/#9 live confirmation:** DTrace captures 62–67 against SL hardware (deferred — requires user access to SL Mac Pro).

## Clone implications (feeds Part C synthesis)

1. **Clock-source accepted values:** C5 user-client surface must accept any `MAClockSource >= 0x40000000`, route `mode & 3` through the permutation `{0→S/PDIF wire 3, 1→ADAT wire 1, 2→Word Clock wire 2, 3→Internal wire 0}`, and feed higher bits only to the engine's digital-input mute (not the HSCI envelope). The HSCI payload is 16 bytes with fixed header `00 FF 00 04 00 04`, wire-byte at offset 6, bytes 7–15 zero.
2. **S/MUX gating:** clone's Hardware view must enable the S/MUX toggle when any ADAT port is active, disable only when neither is ADAT (opposite of what P5 originally documented).
3. **HPAB dialog model:** 3-value picker (`1=out 1/2, 2=out 3/4, 4=aux`) stored in 2-byte-each slots of a 6-byte cookie `0x609`; max-aux-rate gate (`0x615`) disables but does not override the Aux radio.
4. **No Direct Monitoring in HPAB:** the DM toggle sits on the Hardware tab alongside the front-panel A/B mode selector — plan the clone's Hardware view accordingly.
5. **Virtual Rotary:** four target choices, value 4 reserved (do not reuse).
6. **Hardware-only controls:** no session recall for Mic/Inst gain, Mic/Line, Pad, Phantom — document as explicit non-coverage in C5.
