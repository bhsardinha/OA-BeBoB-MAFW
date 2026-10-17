# Q-CMP-CONNECT — isoch channel, PCR connect, bandwidth and speed

> **Correction (2026-10-15, M-C / R44):** §1 lists `chan->addListener(local)` / `chan->setTalker(remote)` — reversed. Slots read from the vtable: the LOCAL DCL port is the channel's talker and the REMOTE port its listener, for TX and RX; `AdjustOutputDCL`'s `*(channel+0x28)` is `fTalker` = the local port. Also: `HandlePCRLock` is an empty callback; the host-PCR plug family (output vs input) is inferred from the value layout. See [`Q-MC-cmp-isoch-avc-start.md`](Q-MC-cmp-isoch-avc-start.md).

> **Update (2026-10-14, P4/R38):** the per-rate parameters this note left open are now tabulated in `Q-P4-cmp-per-rate-and-mixer-power-on.md` Part A — channel counts per mode and rate (four `DeviceChannels` tables, input chosen by `MAClockSource` bit 3, output by bit 4), `m = 1`, packet bytes `4·(a+m)·SYT + 8/12`, pool pages, bandwidth bytes per cycle `(in+out+2m)·SYT·4 + 16` (≤ 1424, below the 2344 device maximum and the 6250 check), speed S400 (SL ioreg), channel from the IRM. The statement in §4 that the "100 ms" of the old stream-kick memory is the CMP retry back-off stands.
> **Correction (2026-10-14) — Part A completion batch 1** (evidence: [PROVENANCE-RESOLUTIONS.md §R8](../PROVENANCE-RESOLUTIONS.md)). `nub->vt[0x850]()` is `IOFireWireNub::FWSpeed() const @0xef38` (IOFireWireFamily 4.7.2, 10.13.6). The 122-quadlet payload's meaning is still **unverified**.


**Status:** done 2026-10-14. Answers BUILD-MANUAL §2.C Q-CMP-CONNECT. Corrects Q3a §3 ("no CMP byte construction in M-Audio code") and Q1b (`[+0x60]` is not `fSpeed`).

**Sources:** raw `otool-exports/mafw.asm`, plus Ghidra 12.1.4 read-only decompiles of the A1 project (scratchpad run 2026-10-14; functions and addresses cited inline). Bundle: `_extracted/` v1.11.x x86_64 slice.

## 1. Call chain (FW1814)

```
FWEngine2::ConnectAudioMIDIChannels @ 0x12156        (sequence: Q3a §3 Correction)
 └─ [+0x350] FWDCLOutputProgram::Connect(ch) @ 0x25790   → CreateIsochChannelAndPorts(talking=true,  ch, [prog+0x60])
 └─ [+0x358] FWDCLInputProgram::Connect(ch)  @ 0x257a4   → CreateIsochChannelAndPorts(talking=false, ch, [prog+0x60])

FWDCLProgram::CreateIsochChannelAndPorts(bool talking, uint ch, uint packetSize) @ 0x255c2
  [prog+0x88] = ch
  local  = bus->vt[0x858](talking, dclProgram, …)        createLocalIsochPort (Q1b)
  remote = new FWRemoteIsochPort; remote->init(connMgr = [[prog+0x70]+0xf0], !talking, ch)   (@ 0x183c6)
  chan   = new FWIsochChannel;    chan->init(controller, doIRM = true, packetSize, speed = nub->vt[0x850](), ForceStopNotificationProc, prog)
  chan->addListener(local)  (vt 0x138) ; chan->setTalker(remote) (vt 0x140) ; chan->allocateChannel() (vt 0x148)
  [prog+0x58] = chan ; [prog+0x8c] = 1
```

- **Channel number:** chosen by the IRM. `IOFWIsochChannel::init(..., doIRM = true, ...)` is followed by `allocateChannel`. `FWRemoteIsochPort::getSupported` (`0x183d8`) returns speed `0x7fffffff` and channel mask `0xffffffffffffffff` (any channel). There is no fixed channel number.
- **Bandwidth:** computed by IOFireWireFamily from `packetSize` and speed. The kext passes no bandwidth units.
- **Speed:** `nub->vt[0x850]()` on the device nub. **Unverified which IOFireWireNub method that slot is** (likely `FWSpeed()`). Check: resolve `IOFireWireNub` vtable slot `0x850` in the 10.13 SDK header order.
- **PCR connect:** when IOFireWireFamily allocates the remote port, `FWRemoteIsochPort::allocatePort` (`0x183ee`) calls `connMgr->vt[0x120](!isTalker, speed, ch-plug)`, which leads to the connection manager below.

## 2. Packet size per direction — `[prog+0x60]`

`FWDCLProgram::SetPacketSize(packetSize, count, audioQ, midiQ)` @ `0x25e56` stores `+0x60 = packetSize`, `+0x64`, `+0x68`. It panics `"IsochStreamRunning during SetPacketSize"` if `[+0x8d]` is set (`0x25e83..0x25e95`).

| Direction | Caller | `packetSize` (bytes) | `count` |
|---|---|---|---|
| RX | `FWDCLInputProgram::SetPacketSize(a, m)` (~`0x260ac`) | `4 · (a + m) · SYTInterval + 12` (`lea 0xc(,%rax,4)`) | `0x100` = 256 = RX ring 64 × 4 (Q-DCL) |
| TX | `FWDCLOutputProgram::SetPacketSize(a, m)` (~`0x260d6`) | `4 · (a + m) · SYTInterval + 8` (`lea 0x8(,%rax,4)`) | `[+0x4e0] << 2` = 4 · P (Q-DCL TX total) |

`SYTInterval` is `[prog+0x4ac]` (Q15e §3 Correction). `a + m` is the AM824 data-block size, the DBS that SetupCIP writes into CIP header 1 from `[+0x64] + [+0x68]` (Q2). The 8 bytes are the CIP header; the extra 4 on RX is the isoch packet header delivered to the receive buffer (**reasoning**, consistent with Q-DCL's 1304 RX / 1568 TX buffer sizes). Per-rate DBS values: see Q-DCL §cipDataQ.

## 3. CMP — PCR addresses and the lock sequence

**Device PCR addresses** (standard IEC 61883-1 register space at `0xFFFF:…`):
- `CoreMakeP2POutputConnection` (`0x23f86`): device **oPCR[n] = `0xF0000904 + 4n`** (`param_2*4 − 0xffff6fc`). It writes speed = `[device+0xe0]->vt[0x850]()` via `vt[0x178]`.
- `CoreMakeP2PInputConnection` (`0x23f50`): device **iPCR[n] = `0xF0000984 + 4n`** (`param_2*4 − 0xffff67c`). Speed argument `0x80000000` = "don't change".
- Both are scheduled on a thread via `[this+0x10]->vt[0x688]()->vt[0x1a0]` (`makeP2P{Input,Output}Connection`, `0x243f2`, `0x24336`).

**`updateP2PCount(pcrAddr, delta, …, channel, speed)`** @ `0x244f0` — read, modify, lock:
1. Read the quadlet at `0xFFFF:pcrAddr` (`[dev+0xe0]->vt[0x888]`, submit `vt[0x140]`) and byte-swap it to host order.
2. New value = old | `0x80000000` (online).
3. If `channel != 0xffffffff`: bits 21:16 = `channel & 0x3f`.
4. If `speed != 0x80000000`: bits 15:14 = `speed & 3`.
5. Point-to-point count bits 29:24: `+1` if `delta ≥ 1` (`(v & 0xc0ffffff) + 0x1000000`), otherwise cleared (`v & 0xc0ffffff`).
6. Compare-and-swap at `0xFFFF:pcrAddr` (`vt[0x8a0]`, submit, `vt[0x1f0]` = "lock succeeded?"). Up to **4 attempts**, `IOSleep(100)` between them. A result of `0xE0008014` counts as retryable.

Overhead-ID and payload fields of the device oPCR are **not** modified.

**Host-local PCR** — `Connect_iPCR(plug, ch)` @ `0x247ba`:
- Allocates a plug in `IOFireWirePCRSpace::getPCRAddressSpace(bus)` with handler `HandlePCRLock` (`vt[0x1b8]`).
- Sets its value to **`ch << 16 | 0xC1003C7A`** (`vt[0x1d0]`):
  - online = 1
  - broadcast = 1
  - p2p = 1
  - channel = `ch`
  - data-rate bits = 0
  - overhead-ID = `0xF`
  - payload = `0x7A` (122 quadlets)
- Then calls `vt[0x148](plug, ch)`.
- **Unverified:** the meaning of a fixed 122-quadlet payload for every rate. Check the host-side PCR consumer.

`Connect(bool, uint, uint)` @ `0x248ce` dispatches: `true` → `Connect_iPCR`, `false` → `vt[0x158]` (the oPCR path).

## 4. Implications for the clone (reasoning)

- Mirror the kext: let IOFireWireFamily's IRM allocate the channel. Do the CMP read/modify/lock on device oPCR/iPCR with ≤4 retries and 100 ms back-off. Don't hardcode a channel or bandwidth.
- `packetSize` must follow §2 exactly, because IOFireWireFamily derives bandwidth from it.
- The "100 ms" in memory `project_fw1814_stream_kick.md` corresponds to the **CMP lock retry back-off**, not a gap between FCP commands (Q3a §3 Correction).

## Provenance

`mafw.asm`: `0x255c2` CreateIsochChannelAndPorts; `0x25790`/`0x257a4` Connect; `0x25e56..0x25e7f` SetPacketSize; `0x260ac..0x260f3` per-direction callers; `0x183c6..0x18416` FWRemoteIsochPort; `0x23f50..0x23ff2` CoreMake/Break; `0x244f0` updateP2PCount; `0x247ba` Connect_iPCR; `0x248ce` Connect. Vtable slot resolution from the slice's `__DATA,__const` vtables (`FWDCLOutputProgram`/`FWDCLInputProgram` slot `0x120` = `Connect`, `0x128` = `Disconnect`).
