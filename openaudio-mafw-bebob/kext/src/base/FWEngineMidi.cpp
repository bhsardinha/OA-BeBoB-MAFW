// Engine-side MIDI (A1 Q-MH): port arrays, port-name properties, start / stop with the isoch reference counting.
#include "FWBaseEngine.h"
#include "FWEngine2.h"
#include "Trace.h"
#include <libkern/c++/OSArray.h>
#include <libkern/c++/OSString.h>

static const IOReturn kErrUnsupported = (IOReturn)0xE00002C7;
static const IOReturn kErrBusy2 = (IOReturn)0xE00002D2;

// ref SetMIDIPortProperties @0x5028: `MAMIDIInputPorts` / `MAMIDIOutputPorts` = array of port names ("1", "2", ... when none are given)
static void SetMIDIPortProperties(com_openaudio_FWAudioDevice *dev, UInt32 count, bool input, const char **names)
{
    if (!dev) return;
    const char *key = input ? "MAMIDIInputPorts" : "MAMIDIOutputPorts";
    if (count == 0) { dev->removeProperty(key); return; }
    OSArray *a = OSArray::withCapacity(4);
    if (!a) return;
    for (UInt32 i = 0; i < count; i++) {
        char def[2] = { (char)('1' + i), 0 };
        OSString *s = OSString::withCString(names ? names[i] : def);
        if (s) { a->setObject(s); s->release(); }
    }
    dev->setProperty(key, a);
    a->release();
}

static FWMIDIPort *MakePorts(UInt32 count, UInt32 &made)
{
    made = 0;
    if (count == 0) return 0;
    FWMIDIPort *p = static_cast<FWMIDIPort *>(IOMalloc((size_t)count * sizeof(FWMIDIPort)));
    if (!p) return 0;
    bzero(p, (size_t)count * sizeof(FWMIDIPort));
    for (UInt32 i = 0; i < count; i++) { if (!p[i].Init()) break; made++; }
    return p;
}

static void FreePorts(FWMIDIPort *p, UInt32 count)
{
    if (!p) return;
    for (UInt32 i = 0; i < count; i++) p[i].Release();
    IOFree(p, (size_t)count * sizeof(FWMIDIPort));
}

void com_openaudio_FWBaseEngine::SetMIDIInputPorts(com_openaudio_FWAudioDevice *dev, UInt32 count, const char **names)
{
    if (midiRunning()) return;
    FreePorts(fMidiIn, fShared.midiInPorts);
    fMidiIn = 0; fShared.midiInPortArray = 0; fShared.midiInPorts = 0;
    UInt32 made;
    fMidiIn = MakePorts(count, made);
    if (count != 0 && !fMidiIn) return;
    fShared.midiInPortArray = fMidiIn;
    fShared.midiInPorts = count;
    SetMIDIPortProperties(dev, count, true, names);
}

void com_openaudio_FWBaseEngine::SetMIDIOutputPorts(com_openaudio_FWAudioDevice *dev, UInt32 count, const char **names)
{
    if (midiRunning()) return;
    if (count > 8) count = 8;
    FreePorts(fMidiOut, fShared.midiOutPorts);
    fMidiOut = 0; fShared.midiOutPortArray = 0; fShared.midiOutPorts = 0;
    UInt32 made;
    fMidiOut = MakePorts(count, made);
    if (count != 0 && !fMidiOut) return;
    fShared.midiOutPortArray = fMidiOut;
    fShared.midiOutPorts = count;
    SetMIDIPortProperties(dev, count, false, names);
}

// M-S deviation: take over the donor's port arrays (the rings are not cleared: the plug-in keeps its own read positions)
void com_openaudio_FWBaseEngine::AdoptMIDIPortsFrom(com_openaudio_FWBaseEngine *d)
{
    if (!d || d == this || midiRunning() || d->midiRunning()) return;
    FreePorts(fMidiIn, fShared.midiInPorts);
    FreePorts(fMidiOut, fShared.midiOutPorts);
    fMidiIn = d->fMidiIn;   fShared.midiInPortArray = fMidiIn;   fShared.midiInPorts = d->fShared.midiInPorts;
    fMidiOut = d->fMidiOut; fShared.midiOutPortArray = fMidiOut; fShared.midiOutPorts = d->fShared.midiOutPorts;
    d->fMidiIn = 0;  d->fShared.midiInPortArray = 0;  d->fShared.midiInPorts = 0;
    d->fMidiOut = 0; d->fShared.midiOutPortArray = 0; d->fShared.midiOutPorts = 0;
}

IOMemoryDescriptor *com_openaudio_FWBaseEngine::MidiPortMemory(bool output, UInt32 port)
{
    if (output) return (port < fShared.midiOutPorts && fMidiOut) ? fMidiOut[port].mem : 0;
    return (port < fShared.midiInPorts && fMidiIn) ? fMidiIn[port].mem : 0;
}

// ref FWBaseEngine::StartMIDI @0x4118: refuses a second start (0xE00002D2) and an engine without ports (0xE00002C7)
IOReturn com_openaudio_FWBaseEngine::StartMIDI()
{
    if (midiRunning()) return kErrBusy2;
    if (fShared.midiInPorts == 0 && fShared.midiOutPorts == 0) return kErrUnsupported;
    IOReturn r = StartIsochStreams();                                        // vt[0xc18]
    if (r == kIOReturnSuccess) fShared.midiInEnabled = true;
    return r;
}

// ref FWBaseEngine::StopMIDI @0x40f6
IOReturn com_openaudio_FWBaseEngine::StopMIDI()
{
    if (midiRunning()) {
        fShared.midiInEnabled = false;
        StopIsochStreams();                                                  // vt[0xc20]
    }
    return kIOReturnSuccess;
}

// ref FWBaseEngine::StopMIDIAndClearBuffers @0x4d78
IOReturn com_openaudio_FWBaseEngine::StopMIDIAndClearBuffers()
{
    IOReturn r = StopMIDI();                                                 // vt[0xcf8]
    if (r == kIOReturnSuccess) {
        for (UInt32 i = 0; i < fShared.midiInPorts; i++) fMidiIn[i].Clear();
        for (UInt32 i = 0; i < fShared.midiOutPorts; i++) fMidiOut[i].Clear();
    }
    return r;
}

// ref FWEngine2::StartMIDI @0x120d4: the first isoch reference gets a 600 ms settle, then the base start takes a second reference
IOReturn com_openaudio_FWEngine2::StartMIDI()
{
    if (midiRunning()) return kErrBusy2;
    if (fShared.midiInPorts == 0 && fShared.midiOutPorts == 0) return kErrUnsupported;
    IOReturn r = StartIsochStreams();
    if (r != kIOReturnSuccess) return r;
    if (fIsochRefs == 1) IOSleep(600);
    r = com_openaudio_FWBaseEngine::StartMIDI();
    if (r != kIOReturnSuccess) StopIsochStreams();
    return r;
}

// ref FWEngine2::StopMIDI @0x11b86: base stop (one reference) and the extra reference taken by StartMIDI
IOReturn com_openaudio_FWEngine2::StopMIDI()
{
    if (midiRunning()) {
        com_openaudio_FWBaseEngine::StopMIDI();
        StopIsochStreams();
    }
    return kIOReturnSuccess;
}
