#include "FW1814UserClient.h"
#include "FWBaseEngine.h"
#include "Trace.h"
#include <IOKit/IOWorkLoop.h>
#include <kern/ipc_mig.h>

static const IOReturn kErrNotReady = (IOReturn)0xE00002BC;
static const IOReturn kErrUnsupported = (IOReturn)0xE00002C7;
static const IOReturn kErrNoDevice = (IOReturn)0xE00002D8;

#define super com_openaudio_FWKIPCUserClient
OSDefineMetaClassAndStructors(com_openaudio_FW1814UserClient, com_openaudio_FWKIPCUserClient)

typedef com_openaudio_FW1814UserClient UC;

// ref sMethods @0x339e0 (legacy IOExternalMethod, stride 48): flags/counts read from the table bytes (Q-UC-surface §2)
IOExternalMethod com_openaudio_FW1814UserClient::sMethods[8] = {
    { 0, (IOMethod)&UC::EntryStartMIDI,         kIOUCScalarIScalarO, 0, 0 },                           // 500
    { 0, (IOMethod)&UC::EntryStopMIDI,          kIOUCScalarIScalarO, 0, 0 },                           // 501
    { 0, (IOMethod)&UC::EntryStopMIDI,          kIOUCScalarIScalarO, 0, 0 },                           // 502 (alias)
    { 0, (IOMethod)&UC::EntrySendMIDI,          kIOUCStructIStructO, 0x118, 4 },                       // 503
    { 0, (IOMethod)&UC::EntryDiceSetRequest,    kIOUCScalarIStructI, 0, kIOUCVariableStructureSize },  // 504
    { 0, (IOMethod)&UC::EntryDiceGetRequest,    kIOUCStructIStructO, kIOUCVariableStructureSize, kIOUCVariableStructureSize },   // 505
    { 0, (IOMethod)&UC::WaitForTermination,     kIOUCScalarIScalarO, 0, 0 },                           // 506
    { 0, (IOMethod)&UC::SetInitializedProperty, kIOUCScalarIScalarO, 0, 0 },                           // 507
};

// ref initWithTask @0x773e
bool com_openaudio_FW1814UserClient::initWithTask(task_t owningTask, void *securityToken, UInt32 type)
{
    bzero(&fMidiNote, sizeof(fMidiNote));
    fDevice = 0; fMidiStarted = false; fTermLock = 0; fTerminating = false;
    return super::initWithTask(owningTask, securityToken, type);
}

// ref start @0x76de: the provider must be a device of ours
bool com_openaudio_FW1814UserClient::start(IOService *provider)
{
    if (!super::start(provider)) return false;
    fDevice = OSDynamicCast(com_openaudio_FWAudioDevice, provider);
    if (!fDevice) return false;
    fTermLock = IOLockAlloc();
    return fTermLock != 0;
}

void com_openaudio_FW1814UserClient::stop(IOService *provider)
{
    fMidiNote.remote = 0;
    if (fTermLock) { IOLockLock(fTermLock); fTerminating = true; IOLockWakeup(fTermLock, &fTerminating, true); IOLockUnlock(fTermLock); }
    super::stop(provider);
    if (fTermLock) { IOLockFree(fTermLock); fTermLock = 0; }
    fDevice = 0;
}

// ref willTerminate @0x76c2: wake a client blocked in WaitForTermination
bool com_openaudio_FW1814UserClient::willTerminate(IOService *, IOOptionBits)
{
    if (fTermLock) { IOLockLock(fTermLock); fTerminating = true; IOLockWakeup(fTermLock, &fTerminating, true); IOLockUnlock(fTermLock); }
    return true;
}

// ref clientClose @0x71a8: drop the MIDI notification port and stop MIDI if this client started it. Added: terminate() (the reference
// leaks the client object, Q-UC Correction R16).
IOReturn com_openaudio_FW1814UserClient::clientClose()
{
    fMidiNote.remote = 0;
    if (fMidiStarted) {
        if (fDevice && fDevice->mainEngine()) fDevice->mainEngine()->UserStopMIDI();
        fMidiStarted = false;
    }
    super::clientClose();
    terminate();
    return kIOReturnSuccess;
}

// ref clientMemoryForType @0x7220
IOReturn com_openaudio_FW1814UserClient::clientMemoryForType(UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory)
{
    *memory = 0;
    if (!fDevice) return kErrNotReady;
    if (type == 1) {
        IOMemoryDescriptor *m = fDevice->meterMemory();
        if (!m) return kErrNotReady;
        m->retain();
        *memory = m;
        *options = kIOMapReadOnly;                                          // ref 0x1000
        return kIOReturnSuccess;
    }
    com_openaudio_FWBaseEngine *e = fDevice->mainEngine();
    if (e) {
        if (type >= 0x400 && type < 0x800) {                                // MIDI source port n: RX ring, read-only
            if (IOMemoryDescriptor *m = e->MidiPortMemory(false, type - 0x400)) { m->retain(); *memory = m; *options = kIOMapReadOnly; return kIOReturnSuccess; }
        } else if (type >= 0x800) {                                         // MIDI destination port n: TX ring, writable
            if (IOMemoryDescriptor *m = e->MidiPortMemory(true, type - 0x800)) { m->retain(); *memory = m; *options = 0; return kIOReturnSuccess; }
        }
    }
    return kErrUnsupported;
}

// ref getTargetAndMethodForIndex @0x72f6
IOExternalMethod *com_openaudio_FW1814UserClient::getTargetAndMethodForIndex(IOService **target, UInt32 index)
{
    if (fDevice && index - 500 < 8) { *target = this; return &sMethods[index - 500]; }
    return 0;
}

// ref registerNotificationPort @0x74aa: type 1 = MIDI-in push channel, anything else goes to the base class
IOReturn com_openaudio_FW1814UserClient::registerNotificationPort(mach_port_t port, UInt32 type, UInt32 refCon)
{
    if (type == 1) {
        fMidiNote.bits = 0x13;
        fMidiNote.size = 0x140;
        fMidiNote.local = 0;
        fMidiNote.voucher = 0; fMidiNote.id = 0;
        fMidiNote.remote = port;
        fMidiNote.refCon = refCon;
        return kIOReturnSuccess;
    }
    return super::registerNotificationPort(port, type, refCon);
}

// ---- selectors: MIDI actions run on the device work loop (ref runAction(ActionStartMIDI / ActionStopMIDI / ActionSendMIDI, engine, ...)) ----

static IOReturn ActionStartMIDI(OSObject *owner, void *, void *, void *, void *)
{
    com_openaudio_FWBaseEngine *e = OSDynamicCast(com_openaudio_FWBaseEngine, owner);
    return e ? e->UserStartMIDI() : kErrNoDevice;
}
static IOReturn ActionStopMIDI(OSObject *owner, void *, void *, void *, void *)
{
    com_openaudio_FWBaseEngine *e = OSDynamicCast(com_openaudio_FWBaseEngine, owner);
    if (e) e->UserStopMIDI();
    return kIOReturnSuccess;
}
static IOReturn ActionSendMIDI(OSObject *owner, void *packet, void *result, void *, void *)
{
    com_openaudio_FWBaseEngine *e = OSDynamicCast(com_openaudio_FWBaseEngine, owner);
    return e ? e->SendMIDI(packet, static_cast<UInt32 *>(result)) : kErrNoDevice;
}

IOReturn com_openaudio_FW1814UserClient::EntryStartMIDI()
{
    IOWorkLoop *wl = fDevice ? fDevice->getWorkLoop() : 0;
    if (!wl || !fDevice->mainEngine()) return kErrNoDevice;
    IOReturn r = wl->runAction(&ActionStartMIDI, fDevice->mainEngine());
    if (r == kIOReturnSuccess) fMidiStarted = true;
    return r;
}

IOReturn com_openaudio_FW1814UserClient::EntryStopMIDI()
{
    IOWorkLoop *wl = fDevice ? fDevice->getWorkLoop() : 0;
    if (!wl || !fDevice->mainEngine()) return kErrNoDevice;
    return wl->runAction(&ActionStopMIDI, fDevice->mainEngine());
}

IOReturn com_openaudio_FW1814UserClient::EntrySendMIDI(void *packet, UInt32 *result)
{
    IOWorkLoop *wl = fDevice ? fDevice->getWorkLoop() : 0;
    if (!wl || !fDevice->mainEngine()) return kErrNoDevice;
    return wl->runAction(&ActionSendMIDI, fDevice->mainEngine(), packet, result);
}

// ref WaitForTermination @0x761e (selector 506): block until the client is terminating
IOReturn com_openaudio_FW1814UserClient::WaitForTermination()
{
    if (!fTermLock) return kErrNotReady;
    IOLockLock(fTermLock);
    while (!fTerminating) IOLockSleep(fTermLock, &fTerminating, THREAD_ABORTSAFE);
    IOLockUnlock(fTermLock);
    return kIOReturnSuccess;
}

// ref SetInitializedProperty @0x71f6 (selector 507): the panel announces itself; published on the device
IOReturn com_openaudio_FW1814UserClient::SetInitializedProperty()
{
    if (fDevice) fDevice->setProperty("MADeviceInitialized", true);
    return kIOReturnSuccess;
}

// ref ReceivedMIDIPacket @0x757e: min(len + 0x40, 0x140) bytes; body: u32 at +0x28 = packet[0], 0x110 bytes from packet+8
IOReturn com_openaudio_FW1814UserClient::ReceivedMIDIPacket(const void *packetV)
{
    if (!fMidiNote.remote) return kErrNotReady;
    const UInt8 *pkt = static_cast<const UInt8 *>(packetV);
    UInt8 msg[0x140];
    bcopy(&fMidiNote, msg, sizeof(fMidiNote));
    bzero(msg + sizeof(fMidiNote), sizeof(msg) - sizeof(fMidiNote));
    UInt32 size = *reinterpret_cast<const UInt16 *>(pkt + 0x10) + 0x40;
    if (size > 0x140) size = 0x140;
    *reinterpret_cast<UInt32 *>(msg + 0x28) = *reinterpret_cast<const UInt32 *>(pkt);
    bcopy(pkt + 8, msg + 0x30, 0x110);
    reinterpret_cast<Notification *>(msg)->size = size;
    return SendMachMessage(1, msg, size);
}
