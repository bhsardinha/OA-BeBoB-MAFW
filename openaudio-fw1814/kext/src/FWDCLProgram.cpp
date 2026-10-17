// FWDCLProgram -- M-B part 2. Port of m_audio_b_FWDCLProgram / FWDCLOutputProgram / FWDCLInputProgram.
// Every function cites its reference address; the decompiles are in docs/system-map/A1-ghidra/ghidra-exports/decomp/
// (q-mb-dcl-callbacks.c, FWDCLProgram__*.c, FWDCLOutputProgram__*.c, FWDCLInputProgram__*.c) and are explained in
// docs/system-map/A1-ghidra/Q-MB-dcl-callbacks.md and Q-START-channel-start-order.md.
// Connect/ResetPort (channel + IRM + CMP) and the engine's RX-then-TX Start order are M-C (FWIsochChannel/FWStreamControl).
#include "FWDCLProgram.h"
#include "Trace.h"
#include "FWMidi.h"
#include <IOKit/firewire/IOFWUtils.h>
#include <kern/clock.h>

#define super OSObject
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWDCLProgram, OSObject)
OSDefineMetaClassAndStructors(com_openaudio_FWDCLOutputProgram, com_openaudio_FWDCLProgram)
OSDefineMetaClassAndStructors(com_openaudio_FWDCLInputProgram, com_openaudio_FWDCLProgram)

static const UInt32 kTxBanks       = 4;      // ref BuildTransmittingDCLProgram: four 4-iteration loops
static const UInt32 kTxPrimers     = 4;      // 4 one-packet segments per bank
static const UInt32 kRxSegments    = 64;     // ref cmpl $0x40 in BuildReceivingDCLProgram
static const UInt32 kRxPktsPerSeg  = 4;      // ref movl $0x4,%edx @0x25a99, fixed at every rate
static const UInt32 kRxTotalPkts   = kRxSegments * kRxPktsPerSeg;   // 256 (ref Input::SetPacketSize count 0x100)
static const IOReturn kErrBadParam = (IOReturn)0xE00002BC;           // ref SetPacketParameters failure
static const IOReturn kErrNoMemory = (IOReturn)0xE00002BD;           // ref RebuildDCLProgramCore NULL

// ---------------------------------------------------------------------------------------------------------------------
// FWDCLProgram (base)
// ---------------------------------------------------------------------------------------------------------------------

// ref Initialize @0x27458: stores nub + shared sync, audioQ = 2, SetupCIP(44100, 8, 0x116A, 1), RebuildDCLProgram.
bool com_openaudio_FWDCLProgram::Initialize(IOFireWireNub *nub, FWDCLSharedSyncData *shared)
{
    if (!nub || !shared || !init()) return false;
    fNub = nub;
    fShared = shared;
    fChannel = 0;
    fPort = 0;
    fPortOwned = false;
    fConnMgr = 0;
    fPlug = 0;
    fSecondary = 0;
    fConnected = fRunning = fPrimary = false;
    fTx = isTransmit();
    fPacketBytes = 0;
    fAudioQ = 2;
    fMidiQ = 0;
    fPacketCount = 0;
    fCallbackCount = 0;
    bzero(fCallbackData, sizeof(fCallbackData));
    UInt32 node;
    LocalNodeID(node);
    fCip.Setup(44100, 8, 0x116A, 1, node, fAudioQ, fMidiQ);
    return RebuildDCLProgram() == kIOReturnSuccess;
}

void com_openaudio_FWDCLProgram::free()
{
    Disconnect();
    ReleasePort();
    fBuilder.Free();
    fPool.Release();
    super::free();
}

void com_openaudio_FWDCLProgram::ReleasePort()
{
    if (fPort && fPortOwned) {
        fPort->release();       // frees the AppleFWOHCI DCL program, which unmaps the pool pages
    }
    fPort = 0;                  // when a channel owns the port, the channel's free() releases it
    fPortOwned = false;
}

// ref SetupCIP @0x27102: header 1 uses the LOCAL node id (this host is the CIP source), not the device's.
void com_openaudio_FWDCLProgram::LocalNodeID(UInt32 &nodeId) const
{
    UInt32 gen = 0;
    UInt16 node = 0, local = 0;
    nodeId = 0;
    if (fNub && fNub->getNodeIDGeneration(gen, node, local) == kIOReturnSuccess) nodeId = local & 0x3f;
}

// ref SetPacketParameters @0x272f4: SetupCIP for the rate (unknown rate -> 44.1 kHz row), then vt[0x118] SetPacketSize.
IOReturn com_openaudio_FWDCLProgram::SetPacketParameters(UInt32 rateHz, UInt32 audioQ, UInt32 midiQ,
                                                         UInt32 *outSytInterval, UInt32 *outFdf)
{
    fAudioQ = audioQ;
    fMidiQ = midiQ;
    const FWRateInfo &ri = FWRateLookup(rateHz);
    UInt32 node;
    LocalNodeID(node);
    fCip.Setup(ri.rateHz, ri.sytInterval, ri.sytIncrement, ri.fdf, node, audioQ, midiQ);
    if (!SetPacketSize(audioQ, midiQ)) return kErrBadParam;
    if (outSytInterval) *outSytInterval = fCip.sytInterval;
    if (outFdf) *outFdf = fCip.fdf;
    return kIOReturnSuccess;
}

// ref RebuildDCLProgram @0x25586: free the builder (NULL-tested), then tail-call vt[0x150].
IOReturn com_openaudio_FWDCLProgram::RebuildDCLProgram()
{
    fBuilder.Free();
    return RebuildDCLProgramCore();
}

// ref BuildPacketDCLSegment @0x2587a:
//   AddLabel; [tagSync: AddSetTagSyncBits(1,0)];
//   callback ? AddTimestampedTransferPackets(op,n) + AddCallProc : AddTransferPackets(op,n)
bool com_openaudio_FWDCLProgram::BuildPacketDCLSegment(UInt32 packets, DCLLabel **outLabel, UInt32 refcon,
                                                       DCLCallCommandProc *cb, bool tagSync)
{
    DCLLabel *label = fBuilder.AddLabel();
    if (!label) return false;
    if (outLabel) *outLabel = label;
    if (tagSync && !fBuilder.AddSetTagSyncBits(1, 0)) return false;
    const UInt32 op = fTx ? kDCLSendPacketStartOp : kDCLReceivePacketStartOp;
    if (cb) {
        if (!fBuilder.AddTimestampedTransferPackets(op, packets)) return false;
        void **slot = &fCallbackData[refcon * 2];
        slot[0] = this;
        slot[1] = reinterpret_cast<void *>((uintptr_t)refcon);
        if (!fBuilder.AddCallProc(cb, slot)) return false;
    } else {
        if (!fBuilder.AddTransferPackets(op, packets)) return false;
    }
    return true;
}

// ref base SetPacketSize @0x25e56 (Ghidra q-mb-dcl-callbacks.c:258). Order as in the reference: refuse while running
// (the reference panics), release the talker (M-C: channel owns it), rebuild the chain, re-init the pool, then walk the chain:
//  * every transfer DCL gets the next pool packet from a forward iterator that STARTS AT PACKET 1 (CommonInit(1,-1),
//    wrapping to 0), i.e. DCL k <- pool packet (k+1) mod N (Q-MB §4), size = packetBytes, header seeded NODATA
//    (CalcIsochPacketHeaders(buf, syt 0, dbc 0, sync false));
//  * every PtrTimeStamp DCL gets slot k in the LAST page of the pool allocation (P1 constraints 1-3).
bool com_openaudio_FWDCLProgram::SetPacketSizeBase(UInt32 packetBytes, UInt32 count, UInt32 audioQ, UInt32 midiQ)
{
    fPacketBytes = packetBytes;
    fAudioQ = audioQ;
    fMidiQ = midiQ;
    if (fRunning) {
        OA_LOG("IsochStreamRunning during SetPacketSize");   // ref: panic FWDCLProgram.cpp:1727; the clone refuses
        return false;
    }
    if (!fNub || !fNub->getBus()) return false;
    if (fChannel) { fChannel->ReleaseTalker(); fPort = 0; }       // ref @0x25e9f: the old DCL port dies with the old chain
    if (RebuildDCLProgram() != kIOReturnSuccess) return false;
    fPool.Release();
    if (!fPool.Initialize(fNub->getBus(), count, packetBytes, audioQ, midiQ)) return false;
    fPool.SetAudioPool(true);                                  // ref [+0x51] = 1
    fPacketCount = count;

    UInt32 pkt = 0, ts = 0, nTransfer = 0, nStamp = 0;
    for (DCLCommand *c = fBuilder.Head(); c; c = c->pNextDCLCommand) {
        if (c->opcode == kDCLSendPacketStartOp || c->opcode == kDCLReceivePacketStartOp) {
            DCLTransferPacket *t = reinterpret_cast<DCLTransferPacket *>(c);
            UInt8 *buf = fPool.GetIndexedPacket((pkt + 1) % count);
            if (!buf) { OA_LOG("(dclPacket->buffer == NULL) at index %u", pkt); return false; }   // ref panic text
            t->buffer = buf;
            t->size = packetBytes;
            fCip.CalcIsochPacketHeaders(reinterpret_cast<UInt32 *>(buf), 0, 0, false);
            pkt++;
            nTransfer++;
        } else if (c->opcode == kDCLPtrTimeStampOp) {
            DCLPtrTimeStamp *p = reinterpret_cast<DCLPtrTimeStamp *>(c);
            p->timeStampPtr = fPool.TimestampSlot(ts++);
            if (!p->timeStampPtr) return false;
            nStamp++;
        }
    }
    OA_LOG("%s program built: %u transfer DCLs, %u timestamp DCLs, %u B x %u packets", fTx ? "TX" : "RX", nTransfer, nStamp,
           packetBytes, count);
    return pkt == count;
}

// ref CreateIsochChannelAndPorts @0x255c2 (port half only): classic 7-argument call, info = NULL (Q1b). M-C moves this into
// the channel setup (Connect); kept here so M-C can call it once the chain is built.
IOReturn com_openaudio_FWDCLProgram::CreateLocalPort()
{
    if (!fBuilder.Head()) return kIOReturnNotReady;
    fPort = fNub->getBus()->createLocalIsochPort(fTx, fBuilder.Head(), 0, 0, 0, 0);
    fPortOwned = (fPort != 0);
    if (!fPort) {
        OA_LOG("createLocalIsochPort(%s) returned NULL (kIOReturnNoResources path, ref 0xE00002BE)", fTx ? "TX" : "RX");
        return (IOReturn)0xE00002BE;
    }
    OA_LOG("createLocalIsochPort(%s) OK: %p", fTx ? "TX" : "RX", fPort);
    return kIOReturnSuccess;
}

// ref ForceStopNotificationProc @0x25766: log and mark the program stopped ([+0x8d] = 0); the channel was force-stopped.
static IOReturn ForceStopNotificationProc(void *refcon, IOFWIsochChannel *channel, UInt32 reason)
{
    OA_LOG("M-FW: channel %p force stop proc (reason:%X)", channel, (unsigned)reason);
    static_cast<com_openaudio_FWDCLProgram *>(refcon)->ForceStop();
    return kIOReturnSuccess;
}

// ref Connect @0x25790 (TX) / @0x257a4 (RX) -> CreateIsochChannelAndPorts @0x255c2 (decomp FWDCLProgram__CreateIsochChannelAndPorts.c).
// Slot-verified wiring (Q-MC): chan->setTalker(LOCAL DCL port), chan->addListener(REMOTE port), then allocateChannel.
IOReturn com_openaudio_FWDCLProgram::Connect(UInt32 plug)
{
    if (!fNub || !fConnMgr || !fBuilder.Head()) return kIOReturnNotReady;
    fPlug = plug;
    IOFireWireBus *bus = fNub->getBus();
    IOFWLocalIsochPort *local = bus ? bus->createLocalIsochPort(fTx, fBuilder.Head(), 0, 0, 0, 0) : 0;
    if (!local) return (IOReturn)0xE00002BE;
    com_openaudio_FWRemoteIsochPort *remote = OSTypeAlloc(com_openaudio_FWRemoteIsochPort);
    if (!remote || !remote->init(fConnMgr, fTx /* remote end is an input plug when we talk */, plug)) {
        if (remote) remote->release();
        local->release();
        return (IOReturn)0xE00002BD;
    }
    com_openaudio_FWIsochChannel *chan = OSTypeAlloc(com_openaudio_FWIsochChannel);
    if (!chan || !chan->init(fNub->getController(), true, fPacketBytes, fNub->FWSpeed(), &ForceStopNotificationProc, this)) {
        if (chan) chan->release();
        remote->release();
        local->release();
        return (IOReturn)0xE00002BD;
    }
    IOReturn r = chan->setTalker(local);
    if (r == kIOReturnSuccess) r = chan->addListener(remote);
    local->release();                                       // the channel retained them
    remote->release();
    if (r == kIOReturnSuccess) r = chan->allocateChannel(); // IRM channel + bandwidth, then both ports' allocatePort (CMP)
    if (r != kIOReturnSuccess) { chan->release(); return r; }
    fChannel = chan;
    fPort = local;
    fPortOwned = false;
    fConnected = true;
    return kIOReturnSuccess;
}

// ref Disconnect @0x250a2
void com_openaudio_FWDCLProgram::Disconnect()
{
    if (!fChannel) return;
    fChannel->releaseChannel();
    fChannel->release();
    fChannel = 0;
    fPort = 0;
    fConnected = false;
}

// ref ResetPort @0x2541a: needs a connected program with a built chain and a channel. Re-creates the local port with a
// cycle-match start (TX: state = cycleTime, mask 0x7fff000; RX: state = cycleTime >> 12, mask 0x7fff), installs it as the
// channel's talker and allocates it on the channel already in use.
IOReturn com_openaudio_FWDCLProgram::ResetPort(UInt32 cycleTime)
{
    IOFireWireBus *bus = fNub ? fNub->getBus() : 0;
    if (!fConnected || !bus || !fBuilder.Head() || !fChannel) return kErrBadParam;
    fCip.sytBase = cycleTime;
    if (fChannel->talker()) static_cast<IOFWIsochPort *>(fChannel->talker())->releasePort();
    const UInt32 state = fTx ? cycleTime : (cycleTime >> 12);
    const UInt32 mask = fTx ? 0x7fff000 : 0x7fff;
    IOFWLocalIsochPort *np = bus->createLocalIsochPort(fTx, fBuilder.Head(), 0, 1, state, mask);
    if (!np) return (IOReturn)0xE00002BE;
    IOReturn r = fChannel->setTalker(np);
    if (r == kIOReturnSuccess) {
        r = np->allocatePort(fNub->FWSpeed(), fChannel->channelNumber());
        if (r == kIOReturnSuccess) fPort = np;
    }
    np->release();
    return r;
}

// ref Start @0x250e4 (-> FWIsochChannel::start @0x6eb0 -> IOFWIsochChannel::start). Q-START.
IOReturn com_openaudio_FWDCLProgram::Start()
{
    if (fRunning) return (IOReturn)0xE00002C2;
    if (!fChannel) return kIOReturnNotReady;
    IOReturn r = fChannel->start();
    if (r == kIOReturnSuccess) fRunning = true;
    return r;
}

// ref Stop @0x25120
void com_openaudio_FWDCLProgram::Stop()
{
    if (fChannel && fRunning) {
        fChannel->stop();
        fRunning = false;
    }
}

// ref CycleTimeToHostNanos @0x26284: bracket getCycleTime between two clock_get_uptime until the bracket is < 150 us, then
// host_ns = t0 + (t1 - t0) / 2 + 125,000 * dCycles, dCycles = nearest value modulo 8000 (direct if |d| < 0x7d1).
// Clone hardening (not in the reference): the retry loop is capped so a stalled bus cannot hang the interrupt path.
UInt64 com_openaudio_FWDCLProgram::CycleTimeToHostNanos(UInt32 cycleTime)
{
    IOFireWireBus *bus = fNub ? fNub->getBus() : 0;
    if (!bus) return 0;
    UInt32 now = 0;
    UInt64 ns0 = 0, ns1 = 0;
    for (int tries = 0; tries < 32; tries++) {
        AbsoluteTime a0, a1;
        clock_get_uptime(&a0);
        bus->getCycleTime(now);
        clock_get_uptime(&a1);
        absolutetime_to_nanoseconds(a0, &ns0);
        absolutetime_to_nanoseconds(a1, &ns1);
        if (ns1 - ns0 < 150000) break;
    }
    const SInt32 cyc = (cycleTime >> 12) & 0x1fff;
    const SInt32 cur = (now >> 12) & 0x1fff;
    SInt32 d = cyc - cur;
    SInt32 alt = (cur < cyc) ? d - 8000 : d + 8000;
    SInt32 mag = (cur < cyc) ? d : -d;
    if (mag < 0x7d1) alt = d;
    return (UInt64)((SInt64)alt * 125000) + ns0 + ((ns1 - ns0) >> 1);
}

void com_openaudio_FWDCLProgram::Dump() const
{
    OA_LOG("%s: rate %u syt %u packet %u B x %u, pool %u pages", fTx ? "TX" : "RX", fCip.rate, fCip.sytInterval, fPacketBytes,
           fPacketCount, fPool.pages());
}

// ---------------------------------------------------------------------------------------------------------------------
// FWDCLOutputProgram
// ---------------------------------------------------------------------------------------------------------------------

bool com_openaudio_FWDCLOutputProgram::Initialize(IOFireWireNub *nub, FWDCLSharedSyncData *shared)
{
    fLap = 0;
    bzero(fHist, sizeof(fHist));
    fHistIdx = 0;
    fExternalSync = false;
    fHdr = fHdr2 = 0;
    fValid = 0;
    fScratchEntries = 0;
    return com_openaudio_FWDCLProgram::Initialize(nub, shared);
}

void com_openaudio_FWDCLOutputProgram::free()
{
    if (fHdr)   IOFree(fHdr,   (size_t)fScratchEntries * sizeof(UInt64));
    if (fHdr2)  IOFree(fHdr2,  (size_t)fScratchEntries * sizeof(UInt64));
    if (fValid) IOFree(fValid, (size_t)fScratchEntries * sizeof(bool));
    fHdr = fHdr2 = 0;
    fValid = 0;
    fScratchEntries = 0;
    com_openaudio_FWDCLProgram::free();
}

// ref Output::SetPacketSize @0x260d2: base((a+m) * SYT * 4 + 8, P << 2, a, m). The scratch arrays replace the reference's
// alloca in OutputDCLCallback (interrupt stack): P x {hdr, hdr2, valid}.
bool com_openaudio_FWDCLOutputProgram::SetPacketSize(UInt32 audioQ, UInt32 midiQ)
{
    const UInt32 P = fCip.perBank;
    if (!SetPacketSizeBase((midiQ + audioQ) * fCip.sytInterval * 4 + 8, P << 2, audioQ, midiQ)) return false;
    if (fScratchEntries != P) {
        if (fHdr)   IOFree(fHdr,   (size_t)fScratchEntries * sizeof(UInt64));
        if (fHdr2)  IOFree(fHdr2,  (size_t)fScratchEntries * sizeof(UInt64));
        if (fValid) IOFree(fValid, (size_t)fScratchEntries * sizeof(bool));
        fHdr = static_cast<UInt64 *>(IOMalloc((size_t)P * sizeof(UInt64)));
        fHdr2 = static_cast<UInt64 *>(IOMalloc((size_t)P * sizeof(UInt64)));
        fValid = static_cast<bool *>(IOMalloc((size_t)P * sizeof(bool)));
        fScratchEntries = P;
        if (!fHdr || !fHdr2 || !fValid) return false;          // free() releases what was allocated (P recorded)
    }
    return true;
}

// ref RebuildDCLProgramCore @0x25df4: builds with OutputDCLStaticCallback + P; NULL -> 0xE00002BD.
IOReturn com_openaudio_FWDCLOutputProgram::RebuildDCLProgramCore()
{
    return BuildTransmittingDCLProgram() == kIOReturnSuccess ? kIOReturnSuccess : kErrNoMemory;
}

// ref BuildTransmittingDCLProgram @0x25ae4: 4 banks x (4 one-packet "primer" segments, no callback, the first of each bank
// carries SetTagSyncBits(1,0) [asm 0x25b50: isLast = (primer == 0)] + 1 big segment of P-4 packets with the callback) + a
// jump per bank; jump of bank b targets the first label of bank b+1 (wrapping), linker @0x25d9d..0x25dde.
IOReturn com_openaudio_FWDCLOutputProgram::BuildTransmittingDCLProgram()
{
    const UInt32 P = fCip.perBank;
    if (P <= kTxPrimers) return kIOReturnBadArgument;
    DCLLabel *labels[kTxBanks * (kTxPrimers + 1)];
    DCLJump  *jumps[kTxBanks];

    size_t bytes = 0;
    for (UInt32 b = 0; b < kTxBanks; b++) {
        for (UInt32 p = 0; p < kTxPrimers; p++) bytes += FWDCLBuilder::BytesForSegment(1, false, false, p == 0);
        bytes += FWDCLBuilder::BytesForSegment(P - kTxPrimers, true, true, false);
        bytes += FWDCLBuilder::BytesForJump();
    }
    if (!fBuilder.Init(bytes)) return kIOReturnNoMemory;

    for (UInt32 b = 0; b < kTxBanks; b++) {
        for (UInt32 p = 0; p < kTxPrimers; p++)
            if (!BuildPacketDCLSegment(1, &labels[b * (kTxPrimers + 1) + p], b, 0, p == 0)) return kIOReturnNoMemory;
        if (!BuildPacketDCLSegment(P - kTxPrimers, &labels[b * (kTxPrimers + 1) + kTxPrimers], b,
                                   &OutputDCLStaticCallback, false))
            return kIOReturnNoMemory;
        jumps[b] = fBuilder.AddJump();
        if (!jumps[b]) return kIOReturnNoMemory;
    }
    for (UInt32 b = 0; b < kTxBanks; b++)
        jumps[b]->pJumpDCLLabel = labels[((b + 1) % kTxBanks) * (kTxPrimers + 1)];
    OA_LOG("TX chain: %u banks x (%u primers + 1 x %u pkts), arena %lu of %lu B", kTxBanks, kTxPrimers, P - kTxPrimers,
           (unsigned long)fBuilder.Used(), (unsigned long)fBuilder.Capacity());
    return kIOReturnSuccess;
}

// ref OutputDCLStaticCallback @0x25e2e: procData = {program, bank}.
void com_openaudio_FWDCLOutputProgram::OutputDCLStaticCallback(DCLCommand *cmd)
{
    if (!cmd) return;
    void **d = static_cast<void **>(reinterpret_cast<DCLCallProc *>(cmd)->procData);
    if (!d || !d[0]) return;
    static_cast<com_openaudio_FWDCLOutputProgram *>(static_cast<com_openaudio_FWDCLProgram *>(d[0]))
        ->OutputDCLCallback((UInt32)(uintptr_t)d[1], false);
}

// ref AdjustOutputDCL @0x2636e: unused < 5 -> re-point the jump that PRECEDES bank b at label 5*b + unused. Bank 0's
// predecessor is jump 3. unused >= 5 changes nothing (a jump left from an earlier lap stays; reference behaviour).
void com_openaudio_FWDCLOutputProgram::AdjustOutputDCL(UInt32 bank, UInt32 unused)
{
    if (unused >= 5) return;
    UInt32 jump, label = unused;
    switch (bank) {
        case 1:  jump = 0; label = unused + 5;  break;
        case 2:  jump = 1; label = unused + 10; break;
        case 3:  jump = 2; label = unused + 15; break;
        default: jump = 3; break;
    }
    fBuilder.ModifyJumpLabel(fPort, jump, label);
}

// ref OutputDCLCallback @0x263c2 (Q-MB §3). prime = true when run from Start() to fill the four banks before the channel
// starts. Argument order of Add/SubtractFWCycleTime verified in asm 0x66c1, 0x6703, 0x6779: Subtract(a, b) = a - b.
void com_openaudio_FWDCLOutputProgram::OutputDCLCallback(UInt32 bank, bool prime)
{
    if (!fPrimary || bank > 3 || !fShared) return;
    FWDCLSharedSyncData *sh = fShared;
    const UInt32 P = fCip.perBank;
    const UInt32 sytInt = fCip.sytInterval;
    UInt32 *tsSlot = fPool.TimestampSlot(bank);
    if (!tsSlot || !fHdr || !fHdr2 || !fValid || fScratchEntries < P) return;
    UInt32 ts = *tsSlot;                              // end time of the bank, written by the OHCI PtrTimeStamp DCL

    if (!prime) {
        if (bank == 3) fLap++;
        UInt64 ns = CycleTimeToHostNanos(ts);
        UInt32 idx = (fHistIdx & 1) ^ 1;
        fHist[idx].bank = bank;
        fHist[idx].nanos = ns;
        fHist[idx].lap = fLap;
        fHistIdx = idx;
        sh->lastBank = bank;
        if (bank == 3) {
            if (fLap >= 3) {
                // ref vt[0xae0] on shared+0x78 = IOAudioEngine::takeTimeStamp(true, &abstime)  (C2 §6.1)
                AbsoluteTime abs;
                nanoseconds_to_absolutetime(ns, &abs);
                if (sh->engine) sh->engine->takeTimeStamp(true, &abs);
            } else if (fLap == 2) {
                sh->resyncCycleTime = ts;
                sh->resyncPending = true;
            }
        }
    }
    if (!fRunning && !prime) return;

    UInt16 *table = sh->sytTable;
    UInt32 wr = sh->sytWrite;
    UInt32 rd = sh->sytRead;

    if (!prime) {                                      // keep the presentation clock ahead of the transmit clock
        SInt32 c = (SInt32)((ts >> 12) & 0x1fff) + (SInt32)fCip.q12;
        SInt32 d = (SInt32)((fCip.sytBase >> 12) & 0x1fff) - c;
        if (d <= -0xfa1) d += 8000;
        if ((UInt32)(d - 3) > 2)
            fCip.sytBase = (fCip.sytBase & 0xfff) + (((UInt32)c * 0x1000u + 0x4000u) & 0x1fff000u) + (ts & 0xfe000000u);
    }

    bool useQueue = false;
    if (fExternalSync && table) {
        UInt32 avail = wr - rd;
        if (avail >= P + 4) {
            if (avail > P * 3) rd = wr - 4 - P;
            UInt32 base = fCip.q12 + (ts >> 12);
            UInt32 e = table[rd & 0x7ff];
            if (e != 0xffff && ((e + base * (UInt32)-0x1000) & 0xffff) - 0x1000 > 0x2000) {
                do {
                    do { rd++; e = table[rd & 0x7ff]; } while (e == 0xffff);
                } while (((e + base * (UInt32)-0x1000) & 0xf000) - 0x1000 > 0x1000);
            }
            useQueue = true;
        }
    }

    UInt32 n = 0, samples = 0;
    if (P != 0) {
        do {
            const UInt32 i = n++;
            bool sync;
            UInt16 syt = 0;
            if (useQueue) {
                UInt32 e = table[rd & 0x7ff];
                if (e == 0xffff) {
                    rd++;
                    sync = false;
                } else {
                    UInt32 delta;
                    if (e < fCip.sytLast) delta = AddFWCycleTimeToFWCycleTime(SubtractFWCycleTimeFromFWCycleTime(0xfc00, fCip.sytLast), e);
                    else                  delta = SubtractFWCycleTimeFromFWCycleTime(e, fCip.sytLast);
                    fCip.sytBase = AddFWCycleTimeToFWCycleTime(fCip.sytBase, delta);
                    fCip.sytLast = e;
                    rd++;
                    sync = true;
                }
            } else {
                fCip.cadAcc += fCip.cadA;
                if (fCip.cadAcc > fCip.cadB) { fCip.cadAcc -= fCip.cadB; sync = false; }
                else {
                    fCip.fracAcc += fCip.fracC;
                    UInt32 inc;
                    if (fCip.fracAcc > fCip.fracD) { fCip.fracAcc -= fCip.fracD; inc = fCip.sytIncrement; }
                    else                           { inc = fCip.sytIncrement + 1; }
                    fCip.sytBase = AddFWCycleTimeToFWCycleTime(fCip.sytBase, inc >> 1);
                    fCip.sytBase = AddFWCycleTimeToFWCycleTime(fCip.sytBase, inc >> 1);
                    fCip.sytBase = AddFWCycleTimeToFWCycleTime(fCip.sytBase, inc & 1);
                    sync = true;
                }
            }
            if (sync) { syt = (UInt16)fCip.sytBase; samples += sytInt; }
            fValid[i] = sync;
            fCip.CalcIsochPacketHeaders(reinterpret_cast<UInt32 *>(&fHdr[i]), syt, fCip.dbc, sync);
            com_openaudio_FWDCLOutputProgram *sec = static_cast<com_openaudio_FWDCLOutputProgram *>(fSecondary);
            if (sec && sec->fHdr2)
                sec->fCip.CalcIsochPacketHeaders(reinterpret_cast<UInt32 *>(&fHdr2[i]), syt, fCip.dbc, sync);
            fCip.dbc = (UInt8)(fCip.dbc + sytInt);       // advances after EVERY packet, data or NODATA (Q-MB note 4)
        } while (n < P && samples < 0x800);
    }
    sh->sytRead = rd;

    const UInt32 unused = P - n;
    AdjustOutputDCL(bank, unused);
    com_openaudio_FWDCLOutputProgram *sec = static_cast<com_openaudio_FWDCLOutputProgram *>(fSecondary);
    if (sec) sec->AdjustOutputDCL(bank, unused);

    fPool.ZeroElements(bank);
    fPool.SetValidPackets(bank, unused, n, fValid);
    fPool.InitGroupCIPsForOutput(bank, n, fHdr);
    if (sec) {
        sec->fPool.ZeroElements(bank);
        sec->fPool.SetValidPackets(bank, unused, n, fValid);
        sec->fPool.InitGroupCIPsForOutput(bank, n, fHdr2);
    }
}

// ref Output::Start @0x26900: reset history/lap, prime banks 0..3, then channel start; outputRunning++ on success.
// (MIDI schedule words +0x558.. / +0x5d8.. are cleared by M-H, which owns them.)
IOReturn com_openaudio_FWDCLOutputProgram::Start()
{
    fHist[0].bank = 3;
    fHist[0].nanos = fShared ? fShared->startNanos : 0;
    fHist[0].lap = 0;
    fHist[1].bank = 0;
    fHist[1].nanos = 0;
    fHist[1].lap = 0;
    fHistIdx = 0;
    if (fShared) fShared->lastBank = 0;
    fLap = 0;
    if (fShared) for (UInt32 p = 0; p < fShared->midiOutPorts && p < 8; p++) { fMidiSched[p][0] = fMidiSched[p][1] = 0; fMidiLastTs[p] = 0; }   // ref Start @0x26900
    for (UInt32 b = 0; b < kTxBanks; b++) OutputDCLCallback(b, true);
    IOReturn r = com_openaudio_FWDCLProgram::Start();
    if (r == kIOReturnSuccess && fShared) fShared->outputRunning++;
    return r;
}

// ref Output::Stop @0x25160
void com_openaudio_FWDCLOutputProgram::Stop()
{
    if (fRunning) {
        com_openaudio_FWDCLProgram::Stop();
        if (fShared) fShared->outputRunning--;
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// FWDCLInputProgram
// ---------------------------------------------------------------------------------------------------------------------

// ref Input::SetPacketSize @0x260a8: base((a+m) * SYT * 4 + 12, 0x100, a, m).
bool com_openaudio_FWDCLInputProgram::SetPacketSize(UInt32 audioQ, UInt32 midiQ)
{
    return SetPacketSizeBase((midiQ + audioQ) * fCip.sytInterval * 4 + 12, kRxTotalPkts, audioQ, midiQ);
}

IOReturn com_openaudio_FWDCLInputProgram::RebuildDCLProgramCore()
{
    return BuildReceivingDCLProgram() == kIOReturnSuccess ? kIOReturnSuccess : kErrNoMemory;
}

// ref BuildReceivingDCLProgram @0x25962: 64 flat segments x 4 packets (callback on each) + a jump per segment, ring-linked.
IOReturn com_openaudio_FWDCLInputProgram::BuildReceivingDCLProgram()
{
    DCLLabel *labels[kRxSegments];
    DCLJump  *jumps[kRxSegments];
    size_t bytes = kRxSegments * (FWDCLBuilder::BytesForSegment(kRxPktsPerSeg, true, true, false) + FWDCLBuilder::BytesForJump());
    if (!fBuilder.Init(bytes)) return kIOReturnNoMemory;
    for (UInt32 i = 0; i < kRxSegments; i++) {
        if (!BuildPacketDCLSegment(kRxPktsPerSeg, &labels[i], i, &InputDCLStaticCallback, false)) return kIOReturnNoMemory;
        jumps[i] = fBuilder.AddJump();
        if (!jumps[i]) return kIOReturnNoMemory;
    }
    for (UInt32 i = 0; i < kRxSegments; i++) jumps[i]->pJumpDCLLabel = labels[(i + 1) & (kRxSegments - 1)];
    OA_LOG("RX chain: %u segs x %u pkts, arena %lu of %lu B", kRxSegments, kRxPktsPerSeg,
           (unsigned long)fBuilder.Used(), (unsigned long)fBuilder.Capacity());
    return kIOReturnSuccess;
}

// ref InputDCLStaticCallback @0x25abe
void com_openaudio_FWDCLInputProgram::InputDCLStaticCallback(DCLCommand *cmd)
{
    if (!cmd) return;
    void **d = static_cast<void **>(reinterpret_cast<DCLCallProc *>(cmd)->procData);
    if (!d || !d[0]) return;
    static_cast<com_openaudio_FWDCLInputProgram *>(static_cast<com_openaudio_FWDCLProgram *>(d[0]))
        ->InputDCLCallbackCopyPacketGroupToSamples((UInt32)(uintptr_t)d[1]);
}

// ref Input::Start @0x2500e
IOReturn com_openaudio_FWDCLInputProgram::Start()
{
    fNoDataCount = 0;
    fDbcValid = false;
    if (fShared) fShared->lastInputSegment = 0x3f;
    return com_openaudio_FWDCLProgram::Start();
}

// ref IOSynchronize @0x257b6
IOReturn com_openaudio_FWDCLInputProgram::IOSynchronize()
{
    if (!fChannel || !fPort) return (IOReturn)0xE00002BC;
    return fPort->synchronizeWithIO();
}

// ref ResyncInputBuffer @0x2610e (asm argument order checked): primary only; delta = nearest cycle difference to the
// TX resync cycle (mod 8000); if > 0, position the input read pointer at the sample the SYT points at.
bool com_openaudio_FWDCLInputProgram::ResyncInputBuffer(UInt32 syt)
{
    if (!fPrimary || !fShared) return false;
    const SInt32 a = (syt >> 12) & 0x1fff;
    const SInt32 b = (fShared->resyncCycleTime >> 12) & 0x1fff;
    SInt32 d = a - b;
    SInt32 alt = (b < a) ? d - 8000 : d + 8000;
    SInt32 mag = (b < a) ? d : -d;
    if (mag < 0x7d1) alt = d;
    if (alt <= 0) return false;
    UInt32 v = SubtractFWCycleTimeFromFWCycleTime(syt & 0x1ffffff, fShared->resyncCycleTime & 0x1ffffff);
    SInt32 num = (SInt32)(fCip.sytIncrement / 2) +
                 (SInt32)(((v >> 12) & 0x1fff) * 0xc00u + (v >> 25) * 0x1770000u + (v & 0xfff)) * (SInt32)fCip.sytInterval;
    UInt32 samples = (UInt32)(num / (SInt32)fCip.sytIncrement) & 0x1fff;
    const UInt32 bytesPerFrame = (fMidiQ + fAudioQ) * 4;
    if (fShared->inputReadByte / bytesPerFrame != samples) fShared->inputReadByte = bytesPerFrame * samples;
    return true;
}

// ref CopySamplesFromPacket @0x26208: payload starts at packet + 3 quadlets (4-byte isoch header + 8-byte CIP).
void com_openaudio_FWDCLInputProgram::CopySamplesFromPacket(UInt32 quadlets, const UInt32 *packet, TRingBuffer &ring)
{
    if (!ring.data) return;
    const UInt32 oldW = ring.writeOff;
    ring.writeWrapped = ring.AdvanceIndex(quadlets * 4, ring.writeOff, ring.writeWraps);
    if (ring.writeOff != 0 && ring.writeWrapped) {
        const UInt32 spill = ring.writeOff >> 2;
        quadlets -= spill;
        bcopy(packet + (quadlets + 3), ring.data, (size_t)spill << 2);
    }
    bcopy(packet + 3, ring.data + oldW, (size_t)quadlets << 2);
}

// ref ProcessCompoundMIDIInputForPacket @0x2525c (M-H owns the port objects; the layout below is what the decompile reads:
// port entry [0x28 stride] -> {UInt32 writeCount; UInt8 ring[0x200]}). `payload` = packet + 0xc bytes.
void com_openaudio_FWDCLInputProgram::ProcessCompoundMIDIInputForPacket(const UInt32 *payload)
{
    if (!fShared || !fShared->midiInPortArray) return;
    const UInt32 blockQ = fAudioQ + fMidiQ;
    if (fCip.sytInterval * blockQ == 0) return;
    UInt32 port = 0;
    for (UInt32 b = 0; b < fCip.sytInterval; b++) {
        UInt32 w = payload[fAudioQ + b * blockQ];
        if ((UInt8)w == 0x81 && port < fShared->midiInPorts) {
            UInt32 *rec = static_cast<FWMIDIPort *>(fShared->midiInPortArray)[port].rx;
            if (rec) {
                reinterpret_cast<UInt8 *>(rec + 1)[rec[0] % 0x200] = (UInt8)(w >> 8);
                rec[0]++;
            }
        }
        port = (port + 1 < 8) ? port + 1 : 0;
    }
}

// ---- MIDI output (M-H) -------------------------------------------------------------------------------------------------------------

#include "FWMidi.h"

// ref ConvertTimestampNanosToAbsPacketIndex @0x26a06: a stamp that is 0 or less than 4 ms ahead becomes now + 4 ms; the index is counted from
// the latest history entry: laps * packets-per-lap + first active packet of the following bank + elapsed ns * 8000 packets/s (+12.8 offset)
SInt64 com_openaudio_FWDCLOutputProgram::ConvertTimestampNanosToAbsPacketIndex(UInt64 stamp, UInt64 now)
{
    if (stamp == 0 || (SInt64)(stamp - now) < 4000000) stamp = now + 4000000;
    const History &h = fHist[fHistIdx & 1];
    const UInt32 first = fPool.GetIndexOfActivePacket((h.bank + 1) & 3, 1);
    return (SInt64)fPool.totalPackets() * (SInt64)h.lap + first + (SInt64)((stamp - h.nanos) * 8000 + 12800000000ULL) / 1000000000;
}

// ref ScheduleMidiPacket @0x26ab2: place up to `length` bytes of the packet into free MIDI slots (0x80 = empty) of the port's data block in
// consecutive-eligible TX packets, as 0x81 | byte << 8. Returns the number of bytes placed.
UInt32 com_openaudio_FWDCLOutputProgram::ScheduleMidiPacket(void *slotV, UInt32 port, UInt32 mode, UInt64 now)
{
    FWMIDISlot *slot = static_cast<FWMIDISlot *>(slotV);
    UInt64 pos = (UInt64)ConvertTimestampNanosToAbsPacketIndex(slot->time, now);
    UInt64 &state = fMidiSched[port][mode];
    if (state > pos) pos = state;
    const UInt32 wordIdx = fAudioQ + 2 + port * (fAudioQ + 1);
    const UInt32 perGroup = fPool.packetsPerGroup(), count = fPool.totalPackets();
    if (perGroup == 0 || count == 0) return 0;
    UInt32 sent = 0, tries = 0;
    do {
        tries++;
        UInt64 cand = pos;
        if (cand % perGroup < 4) cand = cand + 4 - cand % perGroup;                // skip the primer packets of each group
        UInt8 *pk = fPool.GetIndexedPacket((UInt32)(cand % count));
        if (!pk || *reinterpret_cast<SInt16 *>(pk + 6) == -1 || *reinterpret_cast<UInt32 *>(pk + (size_t)wordIdx * 4) != 0x80) {
            pos = cand + 1;                                                          // NODATA packet or slot taken
        } else {
            pos = cand + 8;
            if (state <= cand) {
                state = cand;
                if (slot->length > 3) return sent;
                *reinterpret_cast<UInt32 *>(pk + (size_t)wordIdx * 4) = (UInt32)slot->data[sent] * 0x100 + 0x81;
                sent++;
                if (sent == slot->length) return sent;
                pos = cand + 1;
            }
        }
    } while ((SInt32)tries <= 200);
    return sent;
}

// ref ProcessCompoundMIDIOutput @0x26be6 (decomp FWDCLOutputProgram__ProcessCompoundMIDIOutput.c). For every destination port: realtime
// packets first (one packet per scheduled call), then timestamped packets that are due within 20 ms (consecutive stamps are spread >= 1 ms);
// at most `budget` bytes per port per call. A partly scheduled packet keeps its unsent tail and stays at the head of its ring.
int com_openaudio_FWDCLOutputProgram::ProcessCompoundMIDIOutput(UInt32 budgetPerPort)
{
    if (!fShared || fShared->midiOutPorts == 0 || !fShared->midiOutPortArray) return 0;
    AbsoluteTime nowT; clock_get_uptime(&nowT);
    UInt64 now; absolutetime_to_nanoseconds(nowT, &now);
    FWMIDIPort *ports = static_cast<FWMIDIPort *>(fShared->midiOutPortArray);
    int total = 0;
    for (UInt32 port = 0; port < fShared->midiOutPorts; port++) {
        FWMIDIPort &p = ports[port];
        int budget = (int)budgetPerPort;
        for (;;) {
            if (budget < 1) break;
            FWMIDISlot *slot = 0;
            if (p.NextRealtime(slot)) {
                if (ScheduleMidiPacket(slot, port, 0, now) == 0) { p.UnconsumeRealtime(); break; }
                total++; budget--;
                continue;
            }
            if (!p.NextTimestamped(slot, now, 20000000)) break;
            UInt64 t = slot->time;
            if ((SInt64)(t - fMidiLastTs[port]) < 1000000) { slot->time = fMidiLastTs[port] + 1000000; t = slot->time; }
            fMidiLastTs[port] = t;
            const UInt32 len = slot->length;
            if ((int)len <= budget) {
                const UInt32 sent = ScheduleMidiPacket(slot, port, 1, now);
                total += (int)len;
                if (sent == len) { budget -= (int)len; continue; }
                if (sent != 0) {
                    const UInt32 rest = len - sent;
                    bcopy(slot->data + sent, slot->data, rest);
                    slot->length = rest;
                }
                p.UnconsumeTimestamped();
                break;
            }
            slot->length = (UInt32)budget;
            const UInt32 sent = ScheduleMidiPacket(slot, port, 1, now);
            if (sent == 0) { slot->length = len; p.UnconsumeTimestamped(); break; }
            total += (int)sent;
            const UInt32 rest = len - sent;
            if (len != sent) for (UInt32 i = 0; i < rest; i++) slot->data[i] = slot->data[sent + i];
            slot->length = rest;
            p.UnconsumeTimestamped();
            break;
        }
    }
    return total;
}

// ref InputDCLCallbackCopyPacketGroupToSamples @0x26e3e (Q-MB §5): drain the 4 packets of `segment`, catching up over any
// segments the interrupt path missed (modulo 64).
void com_openaudio_FWDCLInputProgram::InputDCLCallbackCopyPacketGroupToSamples(UInt32 segment)
{
    FWDCLSharedSyncData *sh = fShared;
    if (!sh || !sh->sytTable) return;
    UInt32 seg = segment, last = segment;
    const UInt32 next = (sh->lastInputSegment + 1) & 0x3f;
    if (next != segment) {
        UInt32 t = (segment - sh->lastInputSegment) + 0x3f;
        UInt32 d = t & 0x3f;
        if (d > 0x20) d = t | 0xffffffc0u;
        if ((SInt32)d < 0) return;
        seg = next;
        last = d + next;
        if (last < seg) return;
    }
    const UInt32 blockQ = fAudioQ + fMidiQ;
    for (;; seg++) {
        UInt32 wr = sh->sytWrite;
        const bool midiOn = sh->midiInEnabled;
        UInt32 *slot = fPool.TimestampSlot(seg & 0x3f);
        UInt32 t = slot ? SubtractFWCycleTimeFromFWCycleTime(*slot, 0x3000) : 0;
        if (fPrimary) sh->lastInputSegment = seg & 0x3f;
        for (UInt32 j = 0; j < kRxPktsPerSeg; j++) {
            UInt32 *pkt = reinterpret_cast<UInt32 *>(fPool.GetIndexedPacket(((seg * 4) & 0xff) + j));
            if (!pkt) continue;
            const UInt32 q1 = pkt[1];
            if (q1 & 0x80) continue;                        // already consumed
            const UInt16 raw = *reinterpret_cast<UInt16 *>(reinterpret_cast<UInt8 *>(pkt) + 10);
            if (fPrimary) sh->sytTable[wr++ & 0x7ff] = __builtin_bswap16(raw);
            if (raw == 0xffff) {
                fNoDataCount++;
            } else {
                fNoDataCount = 0;
                if (fMidiQ != 0 && midiOn && sh->midiInEnabled) ProcessCompoundMIDIInputForPacket(pkt + 3);
                const UInt8 dbc = (UInt8)(q1 >> 24);
                if (!fDbcValid) { fDbcValid = true; fExpectedDbc = dbc; }
                if (fExpectedDbc != dbc) fExpectedDbc = dbc;            // re-seed on mismatch (reference tracks, never drops)
                CopySamplesFromPacket(blockQ * fCip.sytInterval, pkt, sh->inputRing);
                fExpectedDbc = (UInt8)(fExpectedDbc + (UInt8)fCip.sytInterval);
                pkt[1] = 0x80;                                           // consumed marker
                if (fPrimary && sh->resyncPending)
                    sh->resyncPending = !ResyncInputBuffer((UInt32)__builtin_bswap16(raw) + (t & 0xffff0000u));
            }
            t = AddFWCycleTimeToFWCycleTime(t, 0x1000);
        }
        if (fPrimary) sh->sytWrite = wr;
        if (fMidiQ != 0 && midiOn && fTxProgram && (seg & 1) == 0) fTxProgram->ProcessCompoundMIDIOutput(3);
        if (last < seg + 1) break;
    }
}
