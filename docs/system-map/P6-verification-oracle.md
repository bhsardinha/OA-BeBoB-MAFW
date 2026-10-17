# P6 — Verification oracle (what "correct" means, per milestone)

Date: 2026-10-14. Status: spec only; no harness code exists (code frozen until G1). Parent: `docs/BUILD-PLAN.md` §2 P6.
Acceptance (plan): harness spec for `tests/` + list of captures per milestone.

Authority order for every oracle: `_extracted/` disassembly > SL DTrace/ioreg captures (B5) > macfw/FFADO/snd-bebob logs (decoders only, per `feedback_source_authority_hierarchy`).

## 1. What the SL rig can and cannot observe (B5 §12)

| Observable on SL (10.6.8, v1.9) | Not observable on SL |
|---|---|
| `ioreg` property snapshots (captures 12, 40/41, 50/51, 67a-c, 72) | FCP / HSCI frame bytes (no kext fbt probes) |
| MIG-level `is_io_registry_entry_set_properties` payloads (captures 60-69) | Async mixer register writes (`0xFFC7:0x00700000+`) |
| `pid$target::` userland probes, `syscall::`, `profile-N` | Isoch packet bytes, DCL callbacks |
| Panel meter page, via `pid$target` + `copyin` of the mapped page (**unverified**; not yet attempted) | Bus-level timing |

Consequence: wire bytes of async traffic and isoch payloads cannot be taken from the reference on hardware. They come from (a) static derivation out of the disassembly (O1) and (b) third-party hardware logs (O4). Anything that rests only on those is labelled as such in the test that uses it.

## 2. Oracle categories

| ID | Oracle | Source of truth | Strength |
|---|---|---|---|
| O1 | Static golden vectors | Disassembly (`A1-ghidra/*`, `otool-exports/mafw.asm`) | Exact for bytes the kext computes; says nothing about device acceptance |
| O2 | ioreg snapshot diffs | SL captures 12, 40/41, 50/51, 67a-c, 72 | Exact for property names/values/latencies/channel counts/MABytesPerCycle |
| O3 | `setProperties` replay | SL captures 60-69 (full XML payloads) | Exact for panel→kext control surface; clone must accept each payload and produce the same registry delta |
| O4 | Third-party hardware logs | `_refs/macfw/devices/fw1814/analysis/` (first-duplex-capture-success, native-44100-success, dynamic-rate-switching-success, special-firmware-notes, capture/playback-channel-map); FFADO/snd-bebob | Real device on the wire, but different host stack; decoder only |
| O5 | NEW captures (need user OK) | see §5 | Closes the gaps SL cannot |
| O6 | Hardware acceptance | Clone on build PC + FW1814 | Lock, audio, loopback latency vs C6 §7 bar |

### O1 golden vectors to extract (each becomes a file in `tests/golden/`, with its offset cite in a header comment)

1. HSCI 12-byte CONTROL envelope and 16-byte clock frame (C4 / `project_fw1814_hsci_envelope`; MAClockSource bit 3/4/5 bijection, B5 §10).
2. AV/C plug signal-format frames `00 FF 19|18 00 90 <SFC> FF FF` per rate (macfw `first-duplex-capture-success.md` shows `00 ff 19 00 90 02 ff ff` → response `0f ff 19 00 90 02 ff ff`).
3. Firmware command words (Q-P3, R37): Reset 0x02, DownloadStart 0x04, DownloadBlock 0x05, DownloadEnd 0x06, Go 0x11; register addresses; 512-byte blocking.
4. Mixer register words and level encoding (A1-Q15f, wire map).
5. CIP header formula and SYT/FDF/rate table (Q-P4): TX template `01 0d 00 00 | 90 ff ff ff` (capture 74), DBS/packet-bytes table, pool-page counts, bandwidth formula.
6. DeviceChannels tables (4 tables, Q-P4) and MA* property defaults (A2-P3).
7. Probe/lifecycle: two-round attach (R31) — expressed as a unit test of the state machine with a mock provider: round 1 `probe` must return NULL, no `delete(NULL)` in any `free()`.

### O4 specifics usable now
- Capture CIP: data packets 360 B, FMT 0x10, FDF 0x02, DBS 11, DBC +8 per data packet; NODATA = 8 B `01 02 00 <dbc> 90 02 ff ff`, DBC held across NODATA (macfw `first-duplex-capture-success.md`). Playback DBS 7 (macfw playback-channel-map).
- Caveat: positions 8/9 zero and position 10 MIDI no-data in that S/PDIF-baseline run; channel assignment is from macfw's own mapping, so channel-order tests are **unverified** against the reference until O5/O6.
- Compare against reference: Q-P4 gives 4·(a+m)·SYT+8 TX / +12 RX; the DBS-11 capture stream must be reproducible by that formula for the S/PDIF mode. Test T-CIP-1 checks it; mismatch is a finding, not a test to relax.

## 3. Harness spec (`tests/`, created only after G1)

```
tests/
  golden/          # O1 vectors, one file per category, offset cite in header
  capture/         # copies/links of SL captures used as fixtures (O2/O3), never edited
  unit/            # freestanding encoders (header-only, no IOKit) so they compile on the dev Mac
  ioreg_diff/      # compares clone `ioreg -l -w0 -r -c <class>` vs capture, ignoring declared-volatile keys
  setprops_replay/ # feeds each capture 60-69 payload through the property dispatcher, asserts registry delta
  packet_compare/  # CIP/AM824 packet builder vs O1+O4 vectors
  latency/         # loopback + IOAudioEngine latency properties vs C6 §7
  README.md        # how to run, what needs hardware
```

Rules:
- Encoders/decoders (HSCI, AV/C, CIP, mixer words, firmware words) live in `core/`-style freestanding headers inside the kext tree (no new shared library; the optional `core/` decision stays with the user) so unit tests build off-device.
- Each test names its oracle ID and cite; a test with no cite is rejected (provenance rule).
- Three tiers: **T0** runs on dev Mac (unit/golden/ioreg parser/replay against mock), **T1** needs build PC without device, **T2** needs device. Nothing in T1/T2 runs before G1 and the staged-kext rule (`kextutil -n` first).
- Volatile ioreg keys (timestamps, IDs, GUID-specific, MAFirmwareInfo live fields) are listed in one `volatile.txt`, each with a reason.

## 4. Milestone → oracle matrix

Milestone names are the `Build` tab rows (M-A…M-M); tracks H/PN/IN as in BUILD-PLAN.

| Milestone | Oracle(s) | Existing artefacts | Missing |
|---|---|---|---|
| M-A kext skeleton, nub, two-round probe | O1.7, O2 (class/personality in 12) | 12, 05, R31 doc | none |
| M-B pool + DCL accepted (`generateBufferMap`) | O1.5; run-time success only O6 | Q-P1P2, 74 | **run-time acceptance unverified** until build PC test (T1) |
| M-C firmware upload path | O1.3, O4 (macfw special-firmware-notes) | Q-P3, 12 line 3920 | CRC quadlet on DownloadEnd, FirmwareUpdateBeBoB32 (open in R37) → O5-d |
| M-D AV/C + HSCI clock/rate | O1.1-2, O3 (60-66, 72), O4 | 60-66, 72, macfw | wire bytes of HSCI not observable on SL → derived only |
| M-E CMP + stream start + isoch RX/TX | O1.5, O4, O6 | 74, macfw duplex | exact reference CIP/AM824 on wire → O5-a |
| M-F per-rate/mode channel tables | O1.6, O2 (67a-c, 72) | 67a-c, 72 | none for S/PDIF/ADAT modes seen; 176/192 kHz mode ioreg → O5-c |
| M-G IOAudioEngine family + latency props | O2 (72), O6 | 72 (32/32/64/64/128/128) | latency formula (4×SYT) still hypothesis → O6 loopback |
| M-H mixer init + resend | O1.4, O3 (61-63) | 61-63, Q-P4 | aux-pan, SW-return frames, MAInputPan2 half-word order, headphone-selector initial image → decode with FFADO/macfw, check by ear (O6) |
| M-I property surface (46 MA* keys) | O3 (60-69), O2 (30) | 30, 60-69 | none |
| M-J user client + notifications | O1 (selectors 500-507), O3 | A1-Q11/Q14 | none |
| M-K meter page | O5-b | 70 | pid$target copyin of `uint32 peak[52]` with test tone |
| M-L MIDI | O2 (ports), O4 | 12 | MIDI wire bytes not observed |
| M-M teardown / sleep / bus reset | O2, O6 | 72 sleeps, 73 power-cycle | clone behaviour on power-cycle → O6 |
| H.* helper | O1.3 | Q-P3 | — |
| PN.* panel | O3 payload set is the panel contract | 60-69 | wireframe sign-off (user) |
| IN.* installer | n/a (signing/SIP, see plan) | — | SIP-on check IN.1 |

## 5. Proposed new captures (none run; each needs explicit user OK)

- **O5-a** Passive isoch listener: a second FireWire node on the bus, joined to the allocated channel, recording CIP header + AM824 labels while the SL reference streams. Closes the reference-bytes gap. Hardware availability **unknown** — ask user.
- **O5-b** Meter page: `dtrace -p <Panel pid>` with `pid$target` + `copyin` of the mapped region while a known tone plays; confirms `peak[52]` layout/scale. Probe feasibility **unverified**.
- **O5-c** `ioreg` snapshot at 176.4/192 kHz mode (capture 72 swept rates for latency; per-mode channel counts exist for 44-96 only per 67a-c — verify before assuming). Beware: rate change while streaming panics v1.9 (65b); pause audio first.
- **O5-d** Firmware: no capture planned on the reference (never re-flash for testing). CRC/DownloadEnd questions resolved by disassembly re-read + macfw notes.
- Safety: no hot-plug (use interface power-cycle), no reference kext on 10.13.

## 6. Honest gaps

1. Async frame bytes (FCP, HSCI, mixer register writes) and isoch payloads on the reference are never observed; our oracle is derivation + third-party logs. Device acceptance is the only real proof (O6).
2. Channel-order to physical jack mapping rests on macfw/FFADO until O5-a/O6.
3. Latency model (4×SYT) is a hypothesis until loopback measurement.
4. M-B acceptance of the classic DCL program on 10.13.6 is run-time unverified.
