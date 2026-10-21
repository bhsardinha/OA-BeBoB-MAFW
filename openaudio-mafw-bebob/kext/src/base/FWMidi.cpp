#include "FWMidi.h"
#include <IOKit/IOLib.h>

// ref TKernelMIDIPort::Init @0x1e64a: one 8 KiB page-aligned shared descriptor; all pointers into it; counters zero.
bool FWMIDIPort::Init()
{
    mem = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared | kIODirectionInOut, 0x2000, PAGE_SIZE);
    if (!mem) return true;                                              // the reference returns 1 even when the allocation fails
    UInt8 *b = static_cast<UInt8 *>(mem->getBytesNoCopy());
    bzero(b, 0x2000);
    rx = reinterpret_cast<UInt32 *>(b);
    rxRead = 0;
    ts = b;
    rt = b + 0x808;
    return true;
}

void FWMIDIPort::Release()
{
    if (mem) { mem->release(); mem = 0; }
    rx = 0; ts = rt = 0;
}

// ref Clear @0x4d36: read position 0; the three regions zeroed (0x204 byte ring, two 0x808 packet rings)
void FWMIDIPort::Clear()
{
    rxRead = 0;
    if (rx) bzero(rx, 0x204);
    if (ts) bzero(ts, 0x808);
    if (rt) bzero(rt, 0x808);
}

static inline FWMIDISlot *SlotAt(UInt8 *ring, UInt32 i) { return reinterpret_cast<FWMIDISlot *>(ring + 8 + (size_t)i * 16); }

bool FWMIDIPort::NextRealtime(FWMIDISlot *&slot)
{
    if (!rt) return false;
    UInt32 *h = reinterpret_cast<UInt32 *>(rt);
    if (h[0] == h[1]) return false;
    slot = SlotAt(rt, h[1] % kFWMIDIRingSlots);
    h[1] = (h[1] + 1) % kFWMIDIRingSlots;
    return true;
}

bool FWMIDIPort::NextTimestamped(FWMIDISlot *&slot, UInt64 now, UInt64 window)
{
    if (!ts) return false;
    UInt32 *h = reinterpret_cast<UInt32 *>(ts);
    if (h[0] == h[1]) return false;
    FWMIDISlot *s = SlotAt(ts, h[1] % kFWMIDIRingSlots);
    slot = s;
    if ((SInt64)(s->time - now) >= (SInt64)window) return false;       // not due yet (the reference leaves it unconsumed)
    h[1] = (h[1] + 1) % kFWMIDIRingSlots;
    return true;
}

void FWMIDIPort::UnconsumeRealtime()     { if (rt) { UInt32 *h = reinterpret_cast<UInt32 *>(rt); h[1] = h[1] ? h[1] - 1 : kFWMIDIRingSlots - 1; } }
void FWMIDIPort::UnconsumeTimestamped()  { if (ts) { UInt32 *h = reinterpret_cast<UInt32 *>(ts); h[1] = h[1] ? h[1] - 1 : kFWMIDIRingSlots - 1; } }
