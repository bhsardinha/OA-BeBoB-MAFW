# docs — how the FireWire 1814 driver was reverse engineered and built

Everything here supports the code in [`../openaudio-fw1814/`](../openaudio-fw1814/). The notes were written while the driver was being built, so they are dated and keep their corrections; this page tells you where to start and how to read them.

## Start here

| If you want to… | Read |
|---|---|
| use or build the driver | the project [`README`](../README.md) |
| know what the **Classic** (macOS 10.13) and **Modern** (macOS 11 to 15) editions do differently | [`EDITIONS.md`](EDITIONS.md) |
| see the plan for other macOS versions (10.6.8 to 15, Apple Silicon) and the evidence behind it | [`compatibility plan v3`](compatibility/BROAD_COMPATIBILITY_PLAN_v3.md), its workbook and `evidence/` |
| understand how the interface and the original driver work | [`system-map/README.md`](system-map/README.md) — the index of the reverse-engineering notes, and the **C1–C6** summaries (lifecycle, clocking, communication, transport/control, user client, latency) |
| see why things are the way they are | [`system-map/PROVENANCE-RESOLUTIONS.md`](system-map/PROVENANCE-RESOLUTIONS.md) — dated log of every finding, correction and test result (R1 … R129) |
| see how the multi-model driver is layered, and how to add a model | [`ARCHITECTURE_MAFW_v1.md`](ARCHITECTURE_MAFW_v1.md) |
| see what was decided and what is next | [`STATE.md`](STATE.md), [`NEXT_STEPS.md`](project/NEXT_STEPS.md), the current build plan [`BUILD_AUDIOPHILE_PLAN_v9.md`](BUILD_AUDIOPHILE_PLAN_v9.md) |
| look at hardware captures | [`system-map/B5-sl-dtrace-truth/`](system-map/B5-sl-dtrace-truth/) — DTrace, `ioreg` and screenshots from a Snow Leopard Mac running the official driver; and [`system-map/build-pc-runs/`](system-map/build-pc-runs/), [`system-map/build-pc-panics/`](system-map/build-pc-panics/) — what this driver did on 10.13.6, including kernel panic reports |

## How this folder is organised

| Where | What |
|---|---|
| `docs/` (top level) | Current documents: this index, `EDITIONS.md`, `STATE.md`, `ARCHITECTURE_MAFW_v1.md`, `BUILD_AUDIOPHILE_PLAN_v9.md` (current plan), `BUILD-MANUAL.md`, the notes distilled from the user manuals, `MACOS12_PORT_PERSPECTIVE.md` (the 2026-10-19 assessment of a macOS 12+ port, with its status note), `images/` |
| [`system-map/`](system-map/README.md) | The reverse-engineering notes and the resolutions log (below) |
| [`compatibility/`](compatibility/) | The plan for the whole macOS range: `BROAD_COMPATIBILITY_PLAN_v1` to `v3` (v3 is current; each version is kept), the matching workbooks, and `evidence/` (raw outputs of the static reads and the first run on macOS 14) |
| [`project/`](project/NEXT_STEPS.md) | The working files: `NEXT_STEPS.md`, `NEW_PLAN.md`, `PROVENANCE-AUDIT.md` and the two trackers (`PROVENANCE-MATRIX.xlsx` for 0.1.0, `PROVENANCE-MATRIX-AUDIOPHILE.xlsx` for 0.2.x; in Portuguese) |
| [`archive/`](archive/) | Superseded or historical documents, kept as written: the earlier Audiophile plans `v2` to `v8`, the 0.1.0 build plan, the early proposal, the conformance and readiness notes, the plan flowchart |

In the working repository the `compatibility/`, `project/` and `archive/` contents live at the repository root and in `docs/` under the names the dated notes use (`BROAD_COMPATIBILITY/`, `NEXT_STEPS.md`, `docs/BUILD-PLAN.md` …); the published repository gathers them as above. Paths written as text inside dated notes keep the working names.

## What is in `system-map/`

| Folder / file | Subject |
|---|---|
| `A1-ghidra/` | The official kext: class chain, probe/start, DCL rings, AV/C and HSCI commands, clocking, mixer property model, user client. `ghidra-exports/` holds decompiled text, `otool-exports/` disassembly listings. |
| `A2-panel/` | The official Carbon panel: every control and the registry key it writes, the meter path, screens. |
| `A3-midi/` | The CoreMIDI plug-in: entry points, wire format, endpoint model. |
| `A4-helper/` | The firmware loader / device helper daemon and the firmware upload protocol. |
| `A5-firmware.md`, `A6-prefpane.md` | Firmware container format; the preference pane (a launcher only). |
| `B1`–`B4` | Cross-checks against FFADO, Linux snd-bebob, the Rust snd-firewire-ctl-services and macfw. |
| `B5-sl-dtrace-truth/` | Live captures from the real interface with the official driver. |
| `C1`–`C6`, `D-decision.md` | The synthesis: how the whole system works and why this design was chosen. |

## How to read the notes

- **Provenance.** A claim is supposed to carry its source: a binary offset, an FFADO or snd-bebob `file:line`, or a dated capture. Anything not verified is labelled `guess`, `hypothesis` or `unverified` together with the check that would settle it.
- **Corrections.** When a later finding contradicted an earlier note, the earlier file got a dated `Correction` block instead of silent edits. Read the top of a file first.
- **`_extracted/` references.** The analysis cites addresses in the official M-Audio FireWire 1.11.2 binaries (kext, panel, plug-in, helper) as `_extracted/…`. Those files are not in this repository. To reproduce a reading, unpack the official 1.11.2 installer, load the binary in Ghidra 12.1.4 (headless recipe below) and go to the address.
- **Versions.** Findings name their bundle: the 1.11.2 installer (kext bundle 1.11.3, panel 1.5.2) is the spec; live captures come from driver 1.9 / panel 1.4 on Snow Leopard.
- **Private paths.** Notes mention the author's test machines (a 10.13.6 "build PC" and a Snow Leopard "SL rig") and their addresses; they are not needed to understand the results.

## Other files

[`manual-reference.md`](manual-reference.md) and [`audiophile-manual-reference.md`](audiophile-manual-reference.md) — notes distilled from the FW1814 and FireWire Audiophile user manuals (the manuals themselves are M-Audio's: <https://www.m-audio.com>). The trackers in `project/` are in Portuguese.

## Reproducing a reading

The Ghidra project databases are not included (they embed the original binaries). Re-create one from your own copy of the official 1.11.2 installer and use the scripts in `system-map/A1-ghidra/scripts/`:

```sh
# JDK 21 + Ghidra 12.1.4; <project> is a Ghidra project you created from the unpacked kext binary
JAVA_HOME=<jdk>/Contents/Home <ghidra>/support/analyzeHeadless <project dir> <project name> -process -noanalysis -readOnly \
  -scriptPath system-map/A1-ghidra/scripts -postScript DecompList.java <hex addresses...>
```

`DecompList.java` decompiles the function that contains each address; addresses in the notes are virtual addresses of the 1.11.2 x86_64 kext slice (`otool -tV` listings in `system-map/A1-ghidra/otool-exports/` show the same code).
