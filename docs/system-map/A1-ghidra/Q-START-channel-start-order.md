# Q-START — who starts the isoch channels, and in what order (2026-10-15)

Bundle: `_extracted/` v1.11.2 kext, x86_64 slice, Ghidra 12.1.4 headless (`scripts/DecompList.java`, `scripts/VtDump2.java`).
Closes the hypothesis in `Q-MB-dcl-callbacks.md` §7 item 5 and `C2-sync-path.md` Correction (2026-10-15) item (3); workbook row `A1-Q3a.5`.

Raw decompiles (all under `ghidra-exports/decomp/`, header carries address + date):
`FWEngine2__StartAudio.c`, `FWEngine2__StartPCMAudio.c`, `FWEngine2__StartAC3Output.c`, `FWEngine2__ActivatePCMOutput.c`, `FWEngine2__ActivatePCMInput.c`, `FWEngine2__ConnectAudioMIDIChannels.c`, `FWBaseEngine__StartIsochStreams.c`, `FWBaseEngine__StartAudio.c`, `FWDCLProgram__Start.c`, `FWDCLOutputProgram__Start.c`, `FWDCLInputProgram__Start.c`, `FWDCLProgram__Stop.c`, `FWDCLOutputProgram__Stop.c`, `FWDCLProgram__Disconnect.c`, `FWDCLOutputProgram__ResetPort.c`, `FWDCLInputProgram__ResetPort.c`, `FWDCLProgram__CreateIsochChannelAndPorts.c`, `FWIsochChannel__start.c`, `FWIsochChannel__stop.c`, `FWRemoteIsochPort__start.c`.

## 1. Vtable slots (read from the vtable data, not guessed)

`VtDump2.java` reads `vtable_symbol + 0x10 + slot` (the object's vtable pointer skips two header words; the first dump `VtDump.java` printed raw offsets and is **+0x10 off**, ignore its labels).

| Class | Slot | Function |
|---|---|---|
| `FWDCLOutputProgram` / `FWDCLInputProgram` | `0x120` | `Connect` (`0x25790` / `0x257a4`) |
| | `0x128` | `Disconnect` (`0x250a2`) |
| | `0x130` | `ResetPort(uint)` (`0x25566` / `0x25578`) |
| | `0x138` | **`Start`** (`0x26900` Output / `0x2500e` Input) |
| | `0x140` | `Stop` (`0x25160` / `0x25120`) |
| | `0x148` | `IOSynchronize` (`0x25158` / `0x257b6`; Output inherits the stub returning 0) |
| `FWEngine2` | `0xc18` | `FWBaseEngine::StartIsochStreams` `0x54b0` |
| | `0xcd0` | `FWEngine2::StartAudio` `0x12024` |
| | `0xd08` | `StartPCMAudio` `0x11b20` |
| | `0xd10` | `StartAC3Output` `0x1185e` |
| | `0xd20` / `0xd28` | `ActivatePCMOutput` `0x11a8c` / `ActivatePCMInput` `0x11a5e` |
| | `0xd18` | `ActivateAC3Output` `0x11fee` |

(Dump 1 listed the program slots at `0x128..0x160`; subtracting the `0x10` header gives the real slots above: Connect `0x120`, Disconnect `0x128`, ResetPort `0x130`, Start `0x138`, Stop `0x140`, IOSynchronize `0x148`, RebuildDCLProgramCore `0x150`. `SetPacketSize` is real `0x118`.)

## 2. Observations (cited)

1. `ConnectAudioMIDIChannels @0x12156` calls `vt[0x120]` on `[+0x350]` (TX) and `[+0x358]` (RX) = **`Connect`** only. `Connect` → `CreateIsochChannelAndPorts @0x255c2` builds the local port, `FWRemoteIsochPort`, `FWIsochChannel`, then channel `vt[0x148]` (allocate channel/bandwidth — **unverified name**, IOFWIsochChannel external slot) and sets `[+0x8c]=1`. It never sets `[+0x8d]` (the "started" flag). **Connect ≠ Start — confirmed.**
2. `FWDCLProgram::Start @0x250e4`: if `[+0x8d]==0`, calls channel `vt[0x158]` = `m_audio_b_FWIsochChannel::start @0x6eb0`, which calls IOFWIsochChannel start (`vtable+0x168`, external) and sets channel `[+0x78]`; then `[+0x8d]=1`. `FWRemoteIsochPort::start @0x1843e` returns 0 (no-op).
3. `FWDCLOutputProgram::Start @0x26900`: zeroes the cadence/queue state (`+0x508..+0x5d8`), sets lap/bank state `+0x520=3`, **primes all four banks** by calling `OutputDCLCallback(this, bank, true)` for bank 0..3, then `FWDCLProgram::Start`; on success increments the shared counter `**(this+0x80)`.
4. `FWDCLInputProgram::Start @0x2500e`: clears `+0x510`, `+0x504`, sets shared `+0x88 = 0x3f`, then `FWDCLProgram::Start`.
5. `FWEngine2::StartAudio @0x12024` (vt `0xcd0`): when `[+0x238]==0`: if `[+0x2d4]==0` → `vt[0xd08]` = `StartPCMAudio`; else `[+0x338]=0` and `vt[0xd10]` = `StartAC3Output`; `[+0x238]=1` on success.
6. `StartPCMAudio @0x11b20` (needs `[+0x2d3]`, the "initialised" flag): unless (`rate [+0x8c] > 96000` and `[+0x2d6]==0`), calls `vt[0xd28]` = **`ActivatePCMInput`**; then `vt[0xd20]` = **`ActivatePCMOutput`**; on failure `vt[0xd40]` = `DeactivatePCMInput`.
7. `ActivatePCMOutput @0x11a8c` = tail-call `vt[0x138]` (= **Start**) on `[+0x350]` (TX). `ActivatePCMInput @0x11a5e` (requires `[+0x2d4]==0`) = tail-call `vt[0x138]` on `[+0x358]` (RX).
8. `FWBaseEngine::StartIsochStreams @0x54b0` (vt `0xc18`) reference-counts `[+0x22c]`; on the 0→1 transition it clears the ring/sample state (`bzero [+0x270], [+0x278]`) and calls `vt[0xcd0]` = `FWEngine2::StartAudio`.
9. `FWBaseEngine::StartAudio @0x4bca` is a **different** path (it is not `FWEngine2`'s `vt[0xcd0]`): bus `vt[0x860]` (get cycle time) → deadline → for each program in `[+0x178]`: `vt[0x130]` = `ResetPort(startCycle)` with `0` for `FWDCLOutputProgram` instances and `cycle + 0xA0000` (via `AddFWCycleTimeToFWCycleTime`) for the others; then each `vt[0x138]` = `Start`; then engine `vt[0xae0]` = `IOAudioEngine::takeTimeStamp(0, &deadline)`. **Which engine classes use this path is unverified** (`FWEngine2` overrides `vt[0xcd0]`, so it does not).

## 3. Implications for the clone (reasoning, not observation)

* The R1 sequence of C2 §7 mixes two phases: the 500 ms sleeps in `ConnectAudioMIDIChannels` sit between **Connect** calls and the AV/C `vt[0xb50]` stream commands (args `(1|0, 0, [+0x228], [+0x2e0|0x2e4], 100)`), not between DCL starts. Channel start happens later in `StartAudio` → `StartPCMAudio`: **RX program Start first, then TX program Start** (RX is skipped above 96 kHz unless `[+0x2d6]`). The decode of `vt[0xb50]` argument meaning is **not** done here (existing audit item on the stream-kick order stays open).
* Clone M-C: `Connect()` must not call channel `start`; add `Start()` per program, TX `Start` primes four banks via the callback with `first=true` before `channel->start`. M-B's `FWDCLProgram.cpp` port should keep `Connect`/`Start` split (offsets `+0x8c` connected / `+0x8d` started).
* **Still unverified:** the exact IOFWIsochChannel external vtable slots `0x148` / `0x168` (names inferred from position — check against the 10.13.6 `IOFireWireFamily` slot table); the order in which `vt[0xb50]` kicks fall relative to the two `Activate*` calls at run time (needs an SL capture, not static).
