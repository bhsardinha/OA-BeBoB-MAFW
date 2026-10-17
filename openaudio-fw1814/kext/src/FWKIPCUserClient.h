// FWKIPCUserClient -- base of the vendor user client: Mach notification port of type 0 and the cache of property changes
// (A1 Q14 + Q-UC-surface §4; decomp FWKIPCUserClient__*.c). Mirrors m_audio_b_FWKIPCUserClient.
//
// Notification message (type 0 registration): 44-byte template { bits 0x13, size 0x2c, remote port, local 0, ..., +0x20 type,
// +0x24 refCon, +0x28 generation } sent with mach_msg_send_from_kernel_proper:
//   * property change: the modified-properties dictionary serialised as XML after the template, type field 0
//   * IOService message (message()): the template alone with the IOKit message code in the type field
// Every user client of a device is registered in a static set so the property dictionary can reach it.
// Hardening vs the reference: the registry set is lock protected; the "unknown notification type" path returns without touching state.
#ifndef OA_FWKIPCUSERCLIENT_H
#define OA_FWKIPCUSERCLIENT_H

#include <IOKit/IOUserClient.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOLocks.h>
#include <libkern/c++/OSDictionary.h>
#include <libkern/c++/OSSerialize.h>
#include <libkern/c++/OSSet.h>

class com_openaudio_FWKIPCUserClient : public IOUserClient
{
    OSDeclareAbstractStructors(com_openaudio_FWKIPCUserClient)
public:
    virtual bool initWithTask(task_t owningTask, void *securityToken, UInt32 type) APPLE_KEXT_OVERRIDE;   // ref @0x1618e
    virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;                                          // ref @0x1625a
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;                                           // ref @0x160e4
    virtual IOReturn clientClose() APPLE_KEXT_OVERRIDE;                                                   // ref @0x15c38
    virtual IOReturn registerNotificationPort(mach_port_t port, UInt32 type, UInt32 refCon) APPLE_KEXT_OVERRIDE;   // ref @0x15f3a
    virtual IOReturn message(UInt32 type, IOService *provider, void *argument = 0) APPLE_KEXT_OVERRIDE;  // ref @0x15fca

    // ref AddModifiedProperties @0x15e04 (vt[0x960]): merge a changed-properties dictionary into this client's pending set
    virtual void AddModifiedProperties(OSDictionary *d);
    // ref SendChangedPropertiesMessage @0x1602a: serialise + send the pending set; 0xE00002BC when nothing can be sent
    IOReturn SendChangedPropertiesMessage();
    // ref SendMachMessage @0x15faa: patch the type field and send `size` bytes from the kernel
    IOReturn SendMachMessage(UInt32 type, void *message, UInt32 size);

    IOService *provider() const { return fProvider; }                           // ref [+0xd8]: the device this client talks to

    // ref UserClients() / UserClientAdded / UserClientRemoved: the static registry
    static void ForEachClient(IOService *device, void (*fn)(com_openaudio_FWKIPCUserClient *, void *), void *arg);

protected:
    virtual void free() APPLE_KEXT_OVERRIDE;
#pragma pack(push, 4)
    struct Notification {                   // the 44-byte kernel message template
        UInt32 bits;                        // +0x00 0x13
        UInt32 size;                        // +0x04 0x2c
        void  *remote;                      // +0x08 the registered port
        void  *local;                       // +0x10 0
        UInt32 voucher;                     // +0x18
        UInt32 id;                          // +0x1c
        UInt32 type;                        // +0x20
        UInt32 refCon;                      // +0x24
        UInt32 generation;                  // +0x28
    };
#pragma pack(pop)
    IOService *fProvider;                   // [+0xd8]
    Notification fNote;                     // [+0xe0 .. +0x10b]
    OSDictionary *fPending;                 // [+0x110]
    volatile SInt32 fGeneration;            // [+0x118]
    IORecursiveLock *fLock;                 // [+0x120]
    IOBufferMemoryDescriptor *fSendBuf;     // [+0x128]
    OSSerialize *fSerializer;               // [+0x130]
    task_t fTask;                           // [+0x138]
    static void Register(com_openaudio_FWKIPCUserClient *c);
    static void Unregister(com_openaudio_FWKIPCUserClient *c);
    IOBufferMemoryDescriptor *SerializeModifiedProperties();                    // ref @0x15cba
};

#endif
