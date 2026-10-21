#include "FWKIPCUserClient.h"
#include "Trace.h"
#include <libkern/c++/OSCollectionIterator.h>
#include <libkern/OSAtomic.h>
#include <kern/ipc_mig.h>

static const IOReturn kErrBadArg  = (IOReturn)0xE00002C2;
static const IOReturn kErrNotReady = (IOReturn)0xE00002BC;

#define super IOUserClient
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWKIPCUserClient, IOUserClient)

static OSSet *sUserClients;                 // ref sUserClients
static IOLock *sLock;

static IOLock *RegistryLock()
{
    if (!sLock) sLock = IOLockAlloc();      // first use is from start() on the work loop; one lock for the life of the kext
    return sLock;
}

void com_openaudio_FWKIPCUserClient::Register(com_openaudio_FWKIPCUserClient *c)
{
    IOLock *l = RegistryLock();
    if (!l) return;
    IOLockLock(l);
    if (!sUserClients) sUserClients = OSSet::withCapacity(4);
    if (sUserClients) sUserClients->setObject(c);
    IOLockUnlock(l);
}

void com_openaudio_FWKIPCUserClient::Unregister(com_openaudio_FWKIPCUserClient *c)
{
    IOLock *l = RegistryLock();
    if (!l) return;
    IOLockLock(l);
    if (sUserClients) {
        sUserClients->removeObject(c);
        if (sUserClients->getCount() == 0) { sUserClients->release(); sUserClients = 0; }
    }
    IOLockUnlock(l);
}

// Runs fn for every registered client of `device`. The registry lock is held while the clients are collected, not while fn runs.
void com_openaudio_FWKIPCUserClient::ForEachClient(IOService *device, void (*fn)(com_openaudio_FWKIPCUserClient *, void *), void *arg)
{
    IOLock *l = RegistryLock();
    if (!l) return;
    OSArray *copy = 0;
    IOLockLock(l);
    if (sUserClients && sUserClients->getCount()) {
        copy = OSArray::withCapacity(sUserClients->getCount());
        OSCollectionIterator *it = OSCollectionIterator::withCollection(sUserClients);
        if (it && copy) {
            while (com_openaudio_FWKIPCUserClient *c = OSDynamicCast(com_openaudio_FWKIPCUserClient, it->getNextObject()))
                if (c->fProvider == device) copy->setObject(c);                  // retains
        }
        if (it) it->release();
    }
    IOLockUnlock(l);
    if (!copy) return;
    for (unsigned i = 0; i < copy->getCount(); i++)
        if (com_openaudio_FWKIPCUserClient *c = OSDynamicCast(com_openaudio_FWKIPCUserClient, copy->getObject(i))) fn(c, arg);
    copy->release();
}

// ref initWithTask @0x1618e
bool com_openaudio_FWKIPCUserClient::initWithTask(task_t owningTask, void *securityToken, UInt32 type)
{
    bzero(&fNote, sizeof(fNote));
    fGeneration = 0; fPending = 0; fLock = 0; fSendBuf = 0; fSerializer = 0; fProvider = 0;
    fTask = owningTask;
    return super::initWithTask(owningTask, securityToken, type);
}

// ref start @0x1625a: after the superclass start the client joins the registry
bool com_openaudio_FWKIPCUserClient::start(IOService *provider)
{
    if (!super::start(provider)) return false;
    fProvider = provider;
    Register(this);
    return true;
}

// ref stop @0x160e4
void com_openaudio_FWKIPCUserClient::stop(IOService *provider)
{
    fNote.remote = 0;
    Unregister(this);
    if (fPending)    { fPending->release();    fPending = 0; }
    if (fLock)       { IORecursiveLockFree(fLock); fLock = 0; }
    if (fSendBuf)    { fSendBuf->release();    fSendBuf = 0; }
    if (fSerializer) { fSerializer->release(); fSerializer = 0; }
    super::stop(provider);
}

void com_openaudio_FWKIPCUserClient::free()
{
    if (fPending)    { fPending->release();    fPending = 0; }
    if (fLock)       { IORecursiveLockFree(fLock); fLock = 0; }
    if (fSendBuf)    { fSendBuf->release();    fSendBuf = 0; }
    if (fSerializer) { fSerializer->release(); fSerializer = 0; }
    super::free();
}

IOReturn com_openaudio_FWKIPCUserClient::clientClose() { fNote.remote = 0; return kIOReturnSuccess; }          // ref @0x15c38

// ref registerNotificationPort @0x15f3a: only type 0 (property changes / IOService messages)
IOReturn com_openaudio_FWKIPCUserClient::registerNotificationPort(mach_port_t port, UInt32 type, UInt32 refCon)
{
    if (type != 0) {
        IOLog("M-FW: Someone registered for an unknown message type:%X\n", (unsigned)type);
        return kErrBadArg;
    }
    fNote.bits = 0x13;
    fNote.size = 0x2c;
    fNote.local = 0;
    fNote.voucher = 0; fNote.id = 0;
    fNote.remote = port;
    fNote.generation = (UInt32)fGeneration;
    fNote.refCon = refCon;
    return kIOReturnSuccess;
}

IOReturn com_openaudio_FWKIPCUserClient::SendMachMessage(UInt32 type, void *message, UInt32 size)
{
    reinterpret_cast<Notification *>(message)->type = type;
    return mach_msg_send_from_kernel_proper(reinterpret_cast<mach_msg_header_t *>(message), size) == 0 ? kIOReturnSuccess : kErrNotReady;
}

// ref message @0x15fca: forward the IOKit message code to the client (type field = the code), then the superclass
IOReturn com_openaudio_FWKIPCUserClient::message(UInt32 type, IOService *provider, void *argument)
{
    if (fNote.remote) {
        fNote.type = type;
        mach_msg_send_from_kernel_proper(reinterpret_cast<mach_msg_header_t *>(&fNote), fNote.size);
    }
    return super::message(type, provider, argument);
}

// ref AddModifiedProperties @0x15e04: the pending dictionary (and its lock) is created on first use; entries replace older ones
void com_openaudio_FWKIPCUserClient::AddModifiedProperties(OSDictionary *d)
{
    if (!d) return;
    if (!fPending) {
        fPending = OSDictionary::withCapacity(d->getCount());
        if (!fPending) return;
        fLock = IORecursiveLockAlloc();
        if (!fLock) return;
    }
    OSIncrementAtomic(&fGeneration);
    IORecursiveLockLock(fLock);
    OSCollectionIterator *it = OSCollectionIterator::withCollection(d);
    if (it) {
        while (OSSymbol *k = OSDynamicCast(OSSymbol, it->getNextObject())) fPending->setObject(k, d->getObject(k));
        it->release();
    }
    IORecursiveLockUnlock(fLock);
}

// ref SerializeModifiedProperties @0x15cba: [44-byte template room][XML of the pending dictionary]; the pending set is emptied.
// Returns NULL when there is nothing to send (the reference would return the previous buffer).
IOBufferMemoryDescriptor *com_openaudio_FWKIPCUserClient::SerializeModifiedProperties()
{
    if (!fNote.remote || !fPending || !fLock) return 0;
    if (!fSerializer) { fSerializer = OSSerialize::withCapacity(0x200); if (!fSerializer) return 0; }
    else fSerializer->clearText();
    IORecursiveLockLock(fLock);
    const bool ok = fPending->serialize(fSerializer);
    fPending->flushCollection();
    IORecursiveLockUnlock(fLock);
    if (!ok) return 0;
    const size_t len = fSerializer->getLength();
    if (!fSendBuf || fSendBuf->getLength() != len + 0x2c) {
        if (fSendBuf) fSendBuf->release();
        fSendBuf = IOBufferMemoryDescriptor::withCapacity(len + 0x2c, kIODirectionOut, false);
    }
    if (!fSendBuf) return 0;
    bcopy(fSerializer->text(), static_cast<UInt8 *>(fSendBuf->getBytesNoCopy()) + 0x2c, len);
    return fSendBuf;
}

// ref SendChangedPropertiesMessage @0x1602a
IOReturn com_openaudio_FWKIPCUserClient::SendChangedPropertiesMessage()
{
    IOBufferMemoryDescriptor *buf = SerializeModifiedProperties();
    if (!buf || !fNote.remote) return kErrNotReady;
    Notification *m = static_cast<Notification *>(buf->getBytesNoCopy());
    *m = fNote;                                                            // template over the first 44 bytes
    m->size = (UInt32)buf->getLength();
    m->type = 0;
    return mach_msg_send_from_kernel_proper(reinterpret_cast<mach_msg_header_t *>(m), m->size) == 0 ? kIOReturnSuccess : kErrNotReady;
}
