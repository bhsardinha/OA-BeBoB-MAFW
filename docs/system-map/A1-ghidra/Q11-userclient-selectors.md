# Q11 — `FWUserClient` external-method dispatch table

> **Correction (2026-10-14, P5/R35):** selectors 506/507 are **not** ProTools no-ops: 506 = `FWUserClient::WaitForTermination @0x761e` (blocking Mach-semaphore wait released by `stop`), 507 = `SetInitializedProperty @0x71f6` (`MADeviceInitialized = 1`). Use `Q-UC-surface.md` §2 and C5 §2; the rows and 'ProTools' sections below are superseded.
> **Correction (2026-10-14):** see `Q-UC-surface.md` §2 — the table is legacy `IOExternalMethod` (`{object, PMF, flags, count0, count1}`), so the ScIn/StIn columns below are mis-assigned; 503 = `kIOUCStructIStructO` (in 0x118, out 4), 504 = `kIOUCScalarIStructI` (variable), 505 = StructIStructO (variable/variable). Selectors 506/507 have no relocation: `0x969`/`0x971` are virtual PMFs → `vt[0x968]` `WaitForTermination()` / `vt[0x970]` `SetInitializedProperty()`, not the ProTools stubs.

**Status:** done.

**Scope:** `FWUserClient::getTargetAndMethodForIndex` @ 0x72f6 (docs/system-map/A1-ghidra/otool-exports/mafw.asm:7295); `sMethods` table in `__DATA.__const` @ 0x339e0 (8 × 48-byte `IOExternalMethodDispatch`-shaped entries). `Entry*` method bodies @ 0x732c..0x7491.

## Answer in one line

**User-space clients call selectors `500..507` (`0x1F4..0x1FB`); the dispatch table has 8 entries routing them to MIDI start/stop/stop-alias/send, Dice set/get, and ProTools open/error-report. ProTools entries are no-op stubs that always return 0 — vestigial from a 2008-era ProTools LE integration.**

## `getTargetAndMethodForIndex(IOService **target, uint32_t selector)` @ 0x72f6

```c
IOExternalMethodDispatch *FWUserClient::getTargetAndMethodForIndex(
    IOService **target, uint32_t sel)
{
    if (!this->fAttached /* [+0x290] */) return NULL;   // no device attached → reject
    uint32_t idx = sel + 0xFFFFFE0C;                    // idx = sel - 500
    if (idx > 7) return NULL;                           // only selectors 500..507 valid
    *target = this;                                     // target = user client itself
    return &sMethods[idx];                              // 48-byte stride (3*16 shl 4)
}
```

Three gates: (1) a userclient-attached-to-device check at `[+0x290]`, (2) bias subtract to make selectors zero-based, (3) upper bound of 8. No in-method dispatch; IOKit's `externalMethod` wrapper does the actual call.

## `sMethods[]` table — raw bytes decoded

Each entry is 48 bytes (`0x30`), laid out as:
- bytes 0..7:  reserved / zero
- bytes 8..15: function pointer
- bytes 16..19: `scalarInputCount`
- bytes 24..27: `structureInputSize`  (`0xFFFFFFFF` = variable)
- bytes 32..35: `scalarOutputCount`
- bytes 40..43: `structureOutputSize` (`0xFFFFFFFF` = variable)

| Sel | Idx | File offset | Function (va) | Symbol | ScIn | StIn | ScOut | StOut |
|---|---|---|---|---|---|---|---|---|
| 500 (0x1F4) | 0 | 0x339e0 | `0x732c` | `EntryStartMIDI()` | 0 | 0 | 0 | 0 |
| 501 (0x1F5) | 1 | 0x33a10 | `0x7398` | `EntryStopMIDI()` | 0 | 0 | 0 | 0 |
| 502 (0x1F6) | 2 | 0x33a40 | `0x7398` | `EntryStopMIDI()` **(alias)** | 0 | 0 | 0 | 0 |
| 503 (0x1F7) | 3 | 0x33a70 | `0x73fa` | `EntrySendMIDI(MAMIDIPacket*, uint32_t*)` | 3 | 0x118 (280) | 4 | 0 |
| 504 (0x1F8) | 4 | 0x33aa0 | `0x746a` | `EntryDiceSetRequest(MAFireWireDiceRequest*)` | 4 | var | 0 | 0 |
| 505 (0x1F9) | 5 | 0x33ad0 | `0x7476` | `EntryDiceGetRequest(MAFireWireDiceRequest*, void*)` | 3 | var | 0 | var |
| 506 (0x1FA) | 6 | 0x33b00 | stub @ `0x969` | `EntryProToolsOpen()` **(no-op)** | 0 | 0 | 0 | 0 |
| 507 (0x1FB) | 7 | 0x33b30 | stub @ `0x971` | `EntryProToolsErrorReport(int*)` **(no-op)** | 0 | 0 | 0 | 0 |

Selector 502 duplicating 501 looks like a userland-API versioning accident — probably "legacy StopMIDI" that mapped to the same handler to maintain binary compat for a 2009-era client. The addresses `0x969` and `0x971` are in a code stub section before `__text` proper and are likely branch thunks into `EntryProToolsOpen` @ 0x7482 and `EntryProToolsErrorReport` @ 0x748a — both of which are literal `xor %eax, %eax; ret` no-ops.

## Entry function stubs — bodies decoded

### `EntryStartMIDI()` @ 0x732c
Fetches `[this+0x290]` (the device nub pointer), builds a bound `TPTMFFunctor1<FWBaseEngine*>` pointing at `ActionStartMIDI`, submits it to the device's workloop via a command gate, returns the gate's status. Classic IOKit "do work on the workloop" pattern.

### `EntryStopMIDI()` @ 0x7398
Mirror of StartMIDI with `ActionStopMIDI` as the gate target. Both `ActionStart/StopMIDI` tail-call to `CoreStart/StopMIDI` @ 0x7185 / 0x7170.

### `EntrySendMIDI(MAMIDIPacket *pkt, uint32_t *bytesWritten)` @ 0x73fa
Takes a 280-byte `MAMIDIPacket` struct (byte count hint in `scalarInputCount=3`) plus a pointer to receive the "bytes queued" count. Gates `ActionSendMIDI` onto the workloop. The 280-byte structure is enough for ~140 MIDI bytes with headers — matches a reasonable per-call MIDI batch.

### `EntryDiceSetRequest(MAFireWireDiceRequest *req)` @ 0x746a
"Dice" here is the branding for the custom HSCI / mixer-register plumbing — misnomer, since M-Audio didn't actually use TC DICE chips, but the API was likely written to be transport-agnostic. 4 scalar args + variable struct → covers all the mixer / levels / routing commands the control panel issues.

### `EntryDiceGetRequest(MAFireWireDiceRequest *req, void *outBuf)` @ 0x7476
Same as DiceSet but with a response buffer. Variable in and out sizes.

### `EntryProToolsOpen()` / `EntryProToolsErrorReport(int*)` @ 0x7482 / 0x748a
Both bodies are literally:
```asm
pushq %rbp; movq %rsp, %rbp; xorl %eax, %eax; popq %rbp; ret
```
No-op stubs that always return 0. These were presumably used by a 2008-era ProTools LE build that integrated at the Mbox-era FireWire layer; by the 2014 panel build they'd been reduced to compile-stable stubs so a legacy ProTools plug-in wouldn't fail to open the userclient.

## Clone implications

- Selector numbers are hardware-independent but client-visible — the control panel app (`_extracted/panel/M-Audio FireWire.app`) and the prefpane will call selector numbers 500..505 at minimum. Clone's userclient **must preserve these selectors** to keep the reference panel usable.
- `MAMIDIPacket` and `MAFireWireDiceRequest` struct layouts are userland ABI. Needs separate decode (Q11a).
- Pro Tools stubs can be omitted or kept as inert for parity.

## Delta summary

| Aspect | Reference | Clone (expected) | Impact |
|---|---|---|---|
| Selector base = 500 | yes | **must mirror** | Client ABI |
| 8 selector slots with 502 == 501 alias | yes | required for panel parity | Simple to mirror |
| 48-byte `IOExternalMethodDispatch` entry stride | yes | matches IOKit header for 10.5+ | Standard |
| MIDI gated via workloop command gate | yes | required | Thread-safety primitive |
| Dice* request struct ABI | opaque, 1-arg + 3-arg forms | **must preserve exactly** | Panel sends these |
| ProTools stubs | no-op | optional | Can be omitted on clone |

## Follow-up queue impact

- **Q11a (new):** decode `MAMIDIPacket` (280-byte) and `MAFireWireDiceRequest` struct layouts — the userland ABI the panel + midi plugin send. Needs cross-check against the panel's `M-Audio FireWire.app` binary and `MAudioFireWireMIDI.plugin`.
- **Q11b (new):** decode `ActionStartMIDI` / `CoreStartMIDI` / `ActionSendMIDI` — the workloop-side work that turns the MIDI selector into isoch sideband bytes. Likely queues bytes into the TX DCL's MIDI quadlet.
- Follow-up: identify what `[+0x290]` is set from (expect `start()` or `attach()` plumbing). Needed to understand userclient lifecycle on 10.13 if the clone implements this.

## Provenance

- Offsets: `docs/system-map/A1-ghidra/otool-exports/mafw.asm` + `otool -v -s __DATA __const` on `_extracted/kext/M-AudioFireWireBeBoB.x86_64-slice`, 2026-10-06.
- Selector bias `+0xFFFFFE0C = -500`: immediate at 0x7304.
- Upper bound `cmpl $0x7, %edx`: at 0x730a.
- Entry stride `rax*3 << 4 = rax*48`: `leaq (%rax,%rax,2)` + `shlq $0x4` at 0x7314 / 0x7318.
- sMethods table at virtual 0x339e0 (symbol `__ZN24com_m_audio_FWUserClient8sMethodsE`, nm-confirmed).
- EntrySendMIDI argument sizes `0x118 = 280`: byte at 0x33a90 offset.
- ProTools no-op body: `xor %eax, %eax; ret` at 0x7486 and 0x748e.
