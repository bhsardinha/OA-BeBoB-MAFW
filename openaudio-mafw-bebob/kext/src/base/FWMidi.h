// FWMidi -- kernel MIDI ports and the TX/RX conformant-MIDI slot handling (A1 Q-MH; decomp m_audio_FWb__{TKernelMIDIPort,TMIDIPort}__*.c,
// FWDCLInputProgram__ProcessCompoundMIDIInputForPacket.c, FWDCLOutputProgram__{ProcessCompoundMIDIOutput,ScheduleMidiPacket}.c,
// FWBaseEngine__{Set,Start,Stop}MIDI*.c, FWEngine2__{Start,Stop}MIDI.c).
//
// A port owns one 8 KiB kernel/user shared page (IOBufferMemoryDescriptor options 0x10003). Roles overlay the same bytes:
//   SOURCE port (device -> host, user `0x400+n`, read-only):  u32 write counter at +0, 0x200-byte byte ring at +4.
//   DESTINATION port (host -> device, user `0x800+n`):  timestamped ring at +0x000 = {u32 head, u32 tail, 128 x 16-byte slots},
//                                                        realtime ring    at +0x808 = same layout.
// Slot (16 bytes) = DigiFWMIDIPacket { u64 timestamp (ns, host clock); u32 length (<= 4); u8 data[4] }.
// The writer (user space) advances `head`; the kernel consumes at `tail` and may "un-consume" a partly sent packet.
#ifndef OA_FWMIDI_H
#define OA_FWMIDI_H

#include <libkern/OSTypes.h>
#include <IOKit/IOBufferMemoryDescriptor.h>

#define kFWMIDIRingSlots  128
#define kFWMIDIByteRing   0x200

struct FWMIDISlot { UInt64 time; UInt32 length; UInt8 data[4]; };

struct FWMIDIPort                    // 0x28 bytes, the layout read by the DCL code (rx counter pointer at +0)
{
    UInt32 *rx;                      // [+0x00] source ring base (counter, then bytes)
    UInt32  rxRead;                  // [+0x08]
    UInt8  *ts;                      // [+0x10] timestamped ring (destination ports)
    UInt8  *rt;                      // [+0x18] realtime ring (destination ports)
    IOBufferMemoryDescriptor *mem;   // [+0x20]

    bool Init();                                                         // ref TKernelMIDIPort::Init @0x1e64a
    void Release();
    void Clear();                                                        // ref TMIDIPort::Clear @0x4d36
    bool NextRealtime(FWMIDISlot *&slot);                                // ref CheckForNewRealtimePacket @0x1e7b2 (consumes)
    bool NextTimestamped(FWMIDISlot *&slot, UInt64 now, UInt64 window);  // ref CheckForNewTimestampedPacket @0x1e7fa (consumes if due)
    void UnconsumeRealtime();
    void UnconsumeTimestamped();
};

#endif
