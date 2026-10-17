// FWDCLSharedSyncData + TRingBuffer: state shared between the TX program, the RX program and the engine.
// Field names are ours; each is tied to the reference offset it was read at (Ghidra decomp q-mb-dcl-callbacks.c,
// docs/system-map/A1-ghidra/Q-MB-dcl-callbacks.md). Only fields the DCL programs touch are declared; the engine
// milestones (M-D/M-J) add their own members after these.
#ifndef OA_FWSHAREDSYNC_H
#define OA_FWSHAREDSYNC_H

#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/audio/IOAudioEngine.h>

// m_audio_FWb::TRingBuffer (A1 decomp 0x7b6a..0x7cc4). Byte ring with wrap counters, filled by the RX program.
class TRingBuffer
{
public:
    TRingBuffer() : mem(0), data(0), size(0), readOff(0), writeOff(0), readWraps(0), writeWraps(0),
                    readWrapped(false), writeWrapped(false) {}
    ~TRingBuffer() { Invalidate(); }

    bool InitializeWithSizeInBytes(UInt32 bytes);   // ref @0x7cc4: IOBMD::withOptions(0, bytes, 0x1000), prepare, bzero
    void Invalidate();                               // ref @0x7c6a: NULL-safe
    // ref AdvanceIndex @0x7b6a: idx += n; if idx >= size { idx -= size; wraps++; return true }
    bool AdvanceIndex(UInt32 n, UInt32 &idx, UInt32 &wraps) const;
    void WriteNextData(UInt32 n, const void *src);   // ref @0x7b8a
    void ReadNextData(UInt32 n, void *dst);          // ref @0x7bf8

    IOBufferMemoryDescriptor *mem;   // [+0x00]
    UInt8  *data;                    // [+0x08]
    UInt32  size;                    // [+0x10]
    UInt32  readOff;                 // [+0x14]
    UInt32  writeOff;                // [+0x18]
    UInt32  readWraps;               // [+0x1c]
    UInt32  writeWraps;              // [+0x20]
    bool    readWrapped;             // [+0x24]
    bool    writeWrapped;            // [+0x25]
};

#define kFWSytTableEntries 2048      // table index is masked with 0x7ff (OutputDCLCallback, InputDCLCallbackCopy...)
#define kFWSytEmpty        0xFFFF

struct FWDCLSharedSyncData
{
    SInt32  outputRunning;           // [+0x00] ++ in OutputProgram::Start, -- in Stop
    UInt32  resyncCycleTime;         // [+0x0c] set by the TX callback of bank 3 on lap 2
    bool    resyncRequested;         // [+0x10] engine [+0x250]: set by ConvertInputSamplesCore on an input underrun while no resync is
                                     //         pending; cleared by StartIsochStreams. No consumer found in the decompiles (unverified).
    bool    resyncPending;           // [+0x11] input buffer must be re-aligned (cleared by ResyncInputBuffer)
    UInt32  lastBank;                // [+0x14] last TX bank whose callback ran
    UInt16 *sytTable;                // [+0x18] SYT of every received packet, host order, 0xFFFF = none (2048 entries)
    UInt32  sytWrite;                // [+0x20] RX writer index into sytTable
    UInt32  sytRead;                 // [+0x24] TX reader index into sytTable
    TRingBuffer inputRing;           // [+0x28] RX payload quadlets, consumed by the engine's input converters
    UInt32  inputReadByte;           // [+0x40] byte position set by ResyncInputBuffer
    bool    midiInEnabled;           // [+0x58] RX MIDI parsing gate (M-H)
    UInt32  midiInPorts;             // [+0x5c]
    UInt32  midiOutPorts;            // [+0x60]
    void   *midiOutPortArray;        // [+0x68] TMIDIPort[], 0x28 stride (M-H)
    void   *midiInPortArray;         // [+0x70] (M-H)
    IOAudioEngine *engine;           // [+0x78] receives takeTimeStamp from the TX callback of bank 3
    UInt64  startNanos;              // [+0x80]
    UInt32  lastInputSegment;        // [+0x88] RX segment index of the last processed segment (0x3f at Start)
};

#endif
