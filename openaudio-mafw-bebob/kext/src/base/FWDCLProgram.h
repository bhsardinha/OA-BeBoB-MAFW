// FWDCLProgram -- one classic-DCL isoch program (TX or RX): CIP state, packet pool, DCL chain, callbacks.
// Mirrors m_audio_b_FWDCLProgram / FWDCLOutputProgram / FWDCLInputProgram (A1 Q1, Q1a, Q1b, Q2, Q-DCL, Q-P1P2;
// C2 §5-§6, C4 §3; Ghidra decomp q-mb-dcl-callbacks.c; docs/system-map/A1-ghidra/Q-MB-dcl-callbacks.md).
// Channel/IRM (Connect, CreateIsochChannelAndPorts, ResetPort) and the AV/C start sequence are M-C
// (FWIsochChannel/FWCMP/FWStreamControl); this class holds the IOFWIsochChannel* they will install.
#ifndef OA_FWDCLPROGRAM_H
#define OA_FWDCLPROGRAM_H

#include <libkern/c++/OSObject.h>
#include <IOKit/firewire/IOFireWireNub.h>
#include <IOKit/firewire/IOFWLocalIsochPort.h>
#include <IOKit/firewire/IOFWIsochChannel.h>
#include "FWDCLBuilder.h"
#include "TPacketPool.h"
#include "FWRates.h"
#include "FWCip.h"
#include "FWSharedSync.h"
#include "FWIsochChannel.h"
#include "FWCMP.h"

class com_openaudio_FWDCLProgram : public OSObject
{
    OSDeclareAbstractStructors(com_openaudio_FWDCLProgram)
public:
    // ref Initialize @0x27458: stores nub + shared sync, audioQ = 2, SetupCIP(44100, 8, 0x116A, 1), RebuildDCLProgram.
    virtual bool Initialize(IOFireWireNub *nub, FWDCLSharedSyncData *shared);
    virtual void free() APPLE_KEXT_OVERRIDE;

    // ref SetPacketParameters @0x272f4: SetupCIP for the rate, then SetPacketSize(audioQ, midiQ). 0xE00002BC on failure.
    IOReturn SetPacketParameters(UInt32 rateHz, UInt32 audioQ, UInt32 midiQ, UInt32 *outSytInterval = 0, UInt32 *outFdf = 0);
    // ref vt[0x118]: Output/Input SetPacketSize(audioQ, midiQ) (packet bytes and count are per direction).
    virtual bool SetPacketSize(UInt32 audioQ, UInt32 midiQ) = 0;

    IOReturn RebuildDCLProgram();                    // ref @0x25586: free the builder, then RebuildDCLProgramCore
    IOReturn CreateLocalPort();                      // classic createLocalIsochPort(talking, head, NULL, 0, 0, 0) (Q1b)
    void     ReleasePort();                          // port first, then chain, then pool

    // M-C. ref Connect @0x25790 / @0x257a4 -> CreateIsochChannelAndPorts @0x255c2: local DCL port (talker) + FWRemoteIsochPort
    // (listener) + FWIsochChannel(doIRM = true, packetSize, speed), then allocateChannel (IRM channel + CMP via the manager).
    // `plug` is the literal 0 the reference passes (Q-CMP-connect). ref Disconnect @0x250a2: releaseChannel + drop the channel.
    void     SetConnectionManager(com_openaudio_FWAvcConnectionManager *m) { fConnMgr = m; }
    virtual IOReturn Connect(UInt32 plug);
    virtual void     Disconnect();
    // ref ResetPort @0x2541a (engine start path of FWBaseEngine::StartAudio, Q-START): re-create the local port with a
    // cycle-match start and re-install it as the channel's talker. `cycleTime` = start cycle (RX: >> 12 as start state).
    IOReturn ResetPort(UInt32 cycleTime);
    bool     isConnected() const { return fConnected; }

    // ref Start/Stop @0x250e4 / 0x25120: start/stop the isoch channel.
    virtual IOReturn Start();
    virtual void     Stop();
    // ref IOSynchronize vt[0x148]: base @0x25158 returns 0; the RX program (@0x257b6) calls IOFWLocalIsochPort::synchronizeWithIO()
    // on the channel's local port (0xE00002BC when there is none).
    virtual IOReturn IOSynchronize() { return kIOReturnSuccess; }
    bool     isRunning() const { return fRunning; }
    void     ForceStop()       { fRunning = false; }          // ref @0x2518e

    void     SetPrimary(bool p)                { fPrimary = p; }       // ref [+0x8e], set by FWEngine2::initHardware
    void     SetSecondary(com_openaudio_FWDCLProgram *s) { fSecondary = s; }   // ref [+0x90] (stereo-pair engines)
    bool     isTx() const                      { return fTx; }
    IOFWLocalIsochPort *port() const           { return fPort; }
    FWCipState &cip()                          { return fCip; }
    TPacketPool &pool()                        { return fPool; }
    UInt32   audioQuadlets() const             { return fAudioQ; }
    UInt32   midiQuadlets() const              { return fMidiQ; }
    UInt32   packetBytes() const               { return fPacketBytes; }
    UInt32   callbackCount() const             { return fCallbackCount; }

    // ref CycleTimeToHostNanos @0x26284: host time (ns) of a FireWire cycle-time value.
    UInt64   CycleTimeToHostNanos(UInt32 cycleTime);
    // Clone hardening: callbacks and Rebuild/SetPacketSize must not overlap (the reference relies on the engine gate).
    void     Dump() const;

protected:
    virtual bool     isTransmit() const = 0;
    virtual IOReturn RebuildDCLProgramCore() = 0;                    // ref vt[0x150]
    virtual bool     BuildChainReady() { return true; }
    // ref base SetPacketSize @0x25e56: rebuild chain, re-init pool, assign buffers + timestamp slots, seed packet headers.
    bool     SetPacketSizeBase(UInt32 packetBytes, UInt32 count, UInt32 audioQ, UInt32 midiQ);
    bool     BuildPacketDCLSegment(UInt32 packets, DCLLabel **outLabel, UInt32 refcon, DCLCallCommandProc *cb, bool tagSync);
    void     LocalNodeID(UInt32 &nodeId) const;

    IOFireWireNub       *fNub;            // [+0x78]
    FWDCLSharedSyncData *fShared;         // [+0x80]
    com_openaudio_FWIsochChannel *fChannel;   // [+0x58]
    IOFWLocalIsochPort  *fPort;           // channel's local port ([channel+0x28] = fTalker in the reference)
    bool         fPortOwned;              // true only for the standalone CreateLocalPort() path
    com_openaudio_FWAvcConnectionManager *fConnMgr;   // [[dev]+0xf0]
    UInt32       fPlug;                   // [+0x88]
    com_openaudio_FWDCLProgram *fSecondary; // [+0x90]
    bool         fConnected;              // [+0x8c]
    bool         fRunning;                // [+0x8d]
    bool         fPrimary;                // [+0x8e]
    bool         fTx;
    UInt32       fPacketBytes;            // [+0x60]
    UInt32       fAudioQ;                 // [+0x64]
    UInt32       fMidiQ;                  // [+0x68]
    FWCipState   fCip;                    // [+0x498..+0x4e0]
    TPacketPool  fPool;                   // [+0x18]
    FWDCLBuilder fBuilder;                // [+0x10]
    UInt32       fPacketCount;
    void        *fCallbackData[2 * 64];   // {program, refcon} pairs: ref this->callbacks[refcon*2]
    volatile UInt32 fCallbackCount;
};

class com_openaudio_FWDCLOutputProgram : public com_openaudio_FWDCLProgram
{
    OSDeclareDefaultStructors(com_openaudio_FWDCLOutputProgram)
public:
    virtual bool Initialize(IOFireWireNub *nub, FWDCLSharedSyncData *shared) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool SetPacketSize(UInt32 audioQ, UInt32 midiQ) APPLE_KEXT_OVERRIDE;      // ref @0x260d2
    virtual IOReturn Start() APPLE_KEXT_OVERRIDE;                                      // ref @0x26900
    virtual void Stop() APPLE_KEXT_OVERRIDE;                                           // ref @0x25160

    // ref [+0x618], FWEngine2::SetExternalSyncEnabled: when set and enough input SYTs are queued, TX presentation times
    // follow the received packets instead of the free-running accumulators.
    void SetExternalSync(bool on) { fExternalSync = on; }
    static void OutputDCLStaticCallback(DCLCommand *cmd);                              // ref @0x25e2e
    void OutputDCLCallback(UInt32 bank, bool prime);                                   // ref @0x263c2
    void AdjustOutputDCL(UInt32 bank, UInt32 unused);                                  // ref @0x2636e
    // M-H: MIDI scheduling into the TX packets (ScheduleMidiPacket @0x26ab2, ProcessCompoundMIDIOutput @0x26be6).
    virtual int ProcessCompoundMIDIOutput(UInt32 maxBytes);
    // Clone bench aid (2026-10-17, "audio loops ~200 ms when a second client starts"): timing of the TX bank callbacks, read on demand through the
    // device property MADebugTxStats (no registry mutation from the callback). Gap = time since the previous bank callback, dur = time spent inside it.
    void ResetTxStats();
    void TxStatsString(char *out, size_t n) const;
    UInt32 ScheduleMidiPacket(void *slot, UInt32 port, UInt32 mode, UInt64 nowNanos);           // ref @0x26ab2
    SInt64 ConvertTimestampNanosToAbsPacketIndex(UInt64 stampNanos, UInt64 nowNanos);          // ref @0x26a06
protected:
    virtual bool isTransmit() const APPLE_KEXT_OVERRIDE { return true; }
    virtual IOReturn RebuildDCLProgramCore() APPLE_KEXT_OVERRIDE;                      // ref @0x25df4
    IOReturn BuildTransmittingDCLProgram();                                            // ref @0x25ae4

    struct History { UInt32 bank; UInt64 nanos; UInt64 lap; };
    struct TxStats { UInt64 start, last, maxGap, minGap, maxDur; UInt32 calls, over100, over200, nev; struct { UInt32 gapMs, bank, atMs; } ev[8]; } fStats;
    UInt64   fLap;                         // [+0x518]
    History  fHist[2];                     // [+0x520], [+0x538]
    UInt32   fHistIdx;                     // [+0x550]
    UInt64   fMidiSched[8][2];             // [+0x558] next packet index per (port, mode 0 realtime / 1 timestamped)
    UInt64   fMidiLastTs[8];               // [+0x5d8] last timestamp per port (1 ms minimum spacing)
    bool     fExternalSync;                // [+0x618]
    UInt64  *fHdr;                         // per-callback scratch: P x 8-byte CIP headers (ref uses alloca)
    UInt64  *fHdr2;
    bool    *fValid;
    UInt32   fScratchEntries;
};

class com_openaudio_FWDCLInputProgram : public com_openaudio_FWDCLProgram
{
    OSDeclareDefaultStructors(com_openaudio_FWDCLInputProgram)
public:
    virtual bool SetPacketSize(UInt32 audioQ, UInt32 midiQ) APPLE_KEXT_OVERRIDE;      // ref @0x260a8
    virtual IOReturn Start() APPLE_KEXT_OVERRIDE;                                      // ref @0x2500e
    virtual IOReturn IOSynchronize() APPLE_KEXT_OVERRIDE;                              // ref @0x257b6
    void SetTxProgram(com_openaudio_FWDCLOutputProgram *tx) { fTxProgram = tx; }       // ref [+0x508]
    static void InputDCLStaticCallback(DCLCommand *cmd);                               // ref @0x25abe
    void InputDCLCallbackCopyPacketGroupToSamples(UInt32 segment);                     // ref @0x26e3e
    bool ResyncInputBuffer(UInt32 sytWithSeconds);                                     // ref @0x2610e
    void CopySamplesFromPacket(UInt32 quadlets, const UInt32 *packet, TRingBuffer &ring); // ref @0x26208
    void ProcessCompoundMIDIInputForPacket(const UInt32 *payload);                     // ref @0x2525c
protected:
    virtual bool isTransmit() const APPLE_KEXT_OVERRIDE { return false; }
    virtual IOReturn RebuildDCLProgramCore() APPLE_KEXT_OVERRIDE;                      // ref @0x25a86
    IOReturn BuildReceivingDCLProgram();                                               // ref @0x25962

    com_openaudio_FWDCLOutputProgram *fTxProgram;   // [+0x508]
    bool    fDbcValid;                     // [+0x504]
    UInt8   fExpectedDbc;                  // [+0x505]
    UInt32  fNoDataCount;                  // [+0x510]
};

#endif
