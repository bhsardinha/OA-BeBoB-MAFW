// com_openaudio_FWMetaNub -- see FWMetaNub.h. Every behaviour below cites the reference disassembly
// (docs/system-map/A1-ghidra/otool-exports/mafw.asm and ghidra-exports/decomp/FWMetaNub__*.c, "ref @addr")
// or the P3 write-up (A1-ghidra/Q-P3-kext-firmware-path.md), or is labelled as clone hardening / deviation.
#include "FWMetaNub.h"
#include "Trace.h"

#include <IOKit/IOLib.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOMessage.h>
#include <IOKit/firewire/IOFWCommand.h>
#include <IOKit/firewire/IOFWRegs.h>
#include <IOKit/firewire/IOFireWireController.h>
#include <kern/clock.h>
#include <libkern/c++/OSCollectionIterator.h>
#include <libkern/c++/OSDictionary.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/c++/OSString.h>
#include <libkern/c++/OSSymbol.h>
#include <libkern/c++/OSData.h>
#include <libkern/c++/OSBoolean.h>

#define super IOService
OSDefineMetaClassAndStructors(com_openaudio_FWMetaNub, IOService)

// Bootloader registers (Q-P3 §2; FFADO bebob_dl_mgr.cpp:45-49).
static const UInt32 kBootloaderAddrHi     = 0xFFFF;
static const UInt32 kBootloaderInfoAddrLo = 0xC8020000;   // ref FirmwareCacheInfoRegisters: read 0x50 bytes
static const UInt32 kBootloaderReqAddrLo  = 0xC8021000;   // request register
static const UInt32 kBootloaderInfoLen    = 0x50;
static const UInt32 kInfoBootloaderVerOff = 0x0C;         // ref [+0x9c] = fInfoBlock[0x0c] (Q7 Correction, R37)
static const UInt32 kFWCommandTimeoutUS   = 10000000;     // ref @0x1bdea / FireWireReadCommand.c: 10000000 (unit assumed us, Q-P3 §8)
static const UInt32 kFWStatusRetryA       = 0xE00002D7;   // FireWireReadCommand: IOSleep(2500) then retry, max 10 tries
static const UInt32 kReregisterDelayMS    = 2500;         // ref @0x1bc45: 0x9c4

// Bootloader command word (q1 = operandCount<<24 | code<<16 | commandId(0)), Q-P3 §4; FFADO bebob_dl_codes.cpp.
static const UInt32 kCmdGo                = 0x01110000;   // code 0x11, 1 operand: start-mode (0 = application)


// ref kDeviceSpecs @0x46450: 8 entries x {runtime product name, bootloader product name, device class}, read from the slice
// bytes (Q-AP0 §1.1, R77). The nub accepts all eight, as the reference does; a model only gets a device object when its class
// exists and an Info.plist personality matches its "FireWire Product Name" (FW 1814 now, FW Audiophile in AP-3.1). The class
// column has no code reader in the reference either (one xref to the table, from GetActualProductName); it is kept for parity.
struct DeviceSpec { const char *primary; const char *alias; const char *deviceClass; };
static const DeviceSpec kDeviceSpecs[] = {
    { "FW 410",              "FW Bootloader",                  "com_m_audio_FW410Device" },
    { "FW Audiophile",       "FW Audiophile Bootloader",       "com_m_audio_FWAudiophileDevice" },
    { "FW 1814",             "FW 1814 Bootloader",             "com_m_audio_FW1814Device" },
    { "ProjectMix",          "ProjectMix",                     "com_m_audio_FWProjectMixDevice" },
    { "FW Solo",             "FWSolo Bootloader",              "com_m_audio_FWSoloDevice" },
    { "Ozonic",              "Ozonic Bootloader",              "com_m_audio_FWOzonicDevice" },
    { "NRV-10",              "NRV-10 Bootloader",              "com_m_audio_FWNRV10Device" },
    { "ProFire Lightbridge", "ProFire Lightbridge Bootloader", "com_m_audio_FWLightbridgeDevice" },
};

// ref GetActualProductName @0x1ce06: compares with both names of every row and returns the runtime name; the reference loop
// does not stop at the first match (the last match wins — moot, no name repeats).
const char *com_openaudio_FWMetaNub::GetActualProductName(const char *name)
{
    if (!name) return 0;
    const char *found = 0;
    for (unsigned i = 0; i < sizeof(kDeviceSpecs) / sizeof(kDeviceSpecs[0]); i++) {
        if (strcmp(name, kDeviceSpecs[i].primary) == 0 || strcmp(name, kDeviceSpecs[i].alias) == 0)
            found = kDeviceSpecs[i].primary;
    }
    return found;
}

const char *com_openaudio_FWMetaNub::StateString() const   // ref @0x1b81e
{
    switch (fState) {
        case kStateNubStart:        return "nub start";
        case kStateInitialRegister: return "initial-register";
        case kStateReregister:      return "reregister";
        case kStateFirmwareStart:   return "firmware-start";
        case kStateFirmwareHalt:    return "firmware-halt";
        case kStateFirmwareUpdate:  return "firmware-update";
        default:                    return "<unknown>";
    }
}

bool com_openaudio_FWMetaNub::init(OSDictionary *dict)
{
    if (!super::init(dict)) return false;
    fState = kStateNubStart;
    fNub = 0;
    fBootloaderActive = false;
    fUpdateInProgress = false;
    fServiceClosing = false;
    fFirmwareMatchStarted = false;
    fStopped = false;
    bzero(fInfoBlock, sizeof(fInfoBlock));
    fReregisterCall = 0;
    fReregisterPending = false;
    return true;
}

// 10.13.6 operator delete(NULL) panics (Q-SDK5 obs. 10): every release below is explicitly NULL-tested.
void com_openaudio_FWMetaNub::free()
{
    if (fReregisterCall) {
        thread_call_free(fReregisterCall);   // a pending callout holds a retain on us, so free() cannot run while one is pending
        fReregisterCall = 0;
    }
    super::free();
}

UInt32 com_openaudio_FWMetaNub::ProtocolVersion() const
{
    UInt32 v;
    bcopy(&fInfoBlock[0x08], &v, sizeof(v));   // ref [this+0x98] (Q-P3 §3): raw quadlet, used verbatim as q0 of every command
    return v;
}

// ref probe @0x1ce62 (Q-INFO): vendor NAME in {"M-Audio","M-AUDIO"}, product name must resolve in the device table;
// NOT a Model_ID test. Score 0x7FFFFF on match.
IOService *com_openaudio_FWMetaNub::probe(IOService *provider, SInt32 *score)
{
    OSString *vendor = OSDynamicCast(OSString, provider ? provider->getProperty("FireWire Vendor Name") : 0);
    if (!vendor || !(vendor->isEqualTo("M-Audio") || vendor->isEqualTo("M-AUDIO"))) {
        *score = 0;
        return 0;
    }
    OSString *product = OSDynamicCast(OSString, provider->getProperty("FireWire Product Name"));
    if (!product || !GetActualProductName(product->getCStringNoCopy())) {
        *score = 0;
        return 0;
    }
    OA_LOG("M-Audio FireWire device found: %s", product->getCStringNoCopy());
    *score = 0x7FFFFF;
    return this;
}

bool com_openaudio_FWMetaNub::CopyPropertyFrom(IOService *provider, const char *key)
{
    OSObject *obj = provider->getProperty(key);
    if (!obj) return false;
    return setProperty(key, obj);
}

// ref FireWireWriteCommand @0x1bd3e: one block/quad write to the bootloader, 10 s timeout, status returned.
// Blocking submit: callers are setProperties/start/callout threads, never the FireWire workloop.
IOReturn com_openaudio_FWMetaNub::FireWireWriteCommand(UInt32 addrHi, UInt32 addrLo, UInt32 len, const void *data)
{
    if (!fNub) return kIOReturnBusy;
    IOBufferMemoryDescriptor *buf = IOBufferMemoryDescriptor::withBytes(data, len, kIODirectionOut, false);
    if (!buf) return kIOReturnBusy;                                    // ref returns 0xE00002BC
    IOReturn ret = buf->prepare();
    if (ret == kIOReturnSuccess) {
        FWAddress addr(addrHi, addrLo);
        IOFWWriteCommand *cmd = fNub->createWriteCommand(addr, buf, 0, 0, false);
        if (!cmd) {
            ret = kIOReturnBusy;
        } else {
            cmd->setTimeout(kFWCommandTimeoutUS);
            ret = cmd->submit();
            if (ret == kIOReturnSuccess) ret = cmd->getStatus();
            cmd->release();
        }
        buf->complete();
    }
    buf->release();
    return ret;
}

// ref FireWireReadCommand @0x1c06e: up to 10 attempts; status 0xE00002D7 -> IOSleep(2500) and retry unless
// stopped/inactive; any other status ends the loop (FireWireReadCommand.c).
IOReturn com_openaudio_FWMetaNub::FireWireReadCommand(UInt32 addrHi, UInt32 addrLo, UInt32 len, void *dst)
{
    if (!fNub) return kIOReturnBusy;
    IOBufferMemoryDescriptor *buf = IOBufferMemoryDescriptor::withCapacity(len, kIODirectionInOut);
    if (!buf) return (IOReturn)0xE00002BC;
    IOReturn ret = buf->prepare();
    if (ret == kIOReturnSuccess) {
        ret = (IOReturn)0xE00002BC;
        for (unsigned attempt = 0; attempt < 10; attempt++) {
            FWAddress addr(addrHi, addrLo);
            IOFWReadCommand *cmd = fNub->createReadCommand(addr, buf, 0, 0, false);
            if (!cmd) break;
            cmd->setTimeout(kFWCommandTimeoutUS);
            ret = cmd->submit();
            if (ret == kIOReturnSuccess) ret = cmd->getStatus();
            cmd->release();
            if ((UInt32)ret != kFWStatusRetryA) {
                if (ret == kIOReturnSuccess) buf->readBytes(0, dst, len);
                break;
            }
            IOSleep(kReregisterDelayMS);
            if (fStopped || isInactive()) break;
        }
        buf->complete();
    }
    buf->release();
    return ret;
}

// ref @0x1cc9e: bzero 0x50, read, [+0xf0] = ([+0x9c] != 0), publish raw block as OSData "MAFirmwareInfo".
IOReturn com_openaudio_FWMetaNub::FirmwareCacheInfoRegisters()
{
    if (!fNub) return kIOReturnBusy;
    bzero(fInfoBlock, sizeof(fInfoBlock));
    IOReturn ret = FireWireReadCommand(kBootloaderAddrHi, kBootloaderInfoAddrLo, kBootloaderInfoLen, fInfoBlock);
    UInt32 bootloaderVersion;
    bcopy(&fInfoBlock[kInfoBootloaderVerOff], &bootloaderVersion, sizeof(bootloaderVersion));
    fBootloaderActive = (bootloaderVersion != 0);
    OSData *info = OSData::withBytes(fInfoBlock, sizeof(fInfoBlock));
    if (info) {
        setProperty("MAFirmwareInfo", info);
        info->release();
    }
    return ret;
}

// ref start @0x1cf68.
bool com_openaudio_FWMetaNub::start(IOService *provider)
{
    if (!super::start(provider)) return false;
    if (!provider) return false;
    fNub = OSDynamicCast(IOFireWireNub, provider);
    if (!fNub || !fNub->getController()) return false;

    CopyPropertyFrom(provider, "FireWire Product Name");
    CopyPropertyFrom(provider, "FireWire Vendor Name");
    CopyPropertyFrom(provider, "Vendor_ID");
    CopyPropertyFrom(provider, "Model_ID");
    CopyPropertyFrom(provider, "GUID");

    OSString *product = OSDynamicCast(OSString, getProperty("FireWire Product Name"));
    const char *actual = product ? GetActualProductName(product->getCStringNoCopy()) : 0;
    if (!actual) return false;
    setProperty("MAActualProductName", actual);

    if (FirmwareCacheInfoRegisters() != kIOReturnSuccess) return false;

    char guidStr[0x40] = "<unknown>";
    OSNumber *guid = OSDynamicCast(OSNumber, getProperty("GUID"));
    if (guid) snprintf(guidStr, sizeof(guidStr), "%llx", guid->unsigned64BitValue());
    setProperty("GUIDString", guidStr);
    OA_LOG("metanub %p GUID:%s product:%s bootloader:%d", this, guidStr, actual, fBootloaderActive);

    fState = kStateInitialRegister;
    registerService();      // round 1: every FW1814Device candidate is rejected by FWAVCAudioDevice::probe (Q-SDK5)
    return true;
}

// ref stop @0x1ba84. The reference also drops this nub from the static sNubDictionary keyed by GUIDString;
// that registry is only consulted for multi-device replacement (M-K) and is not built yet.
void com_openaudio_FWMetaNub::stop(IOService *provider)
{
    fStopped = true;
    if (fReregisterCall && fReregisterPending) {
        if (thread_call_cancel(fReregisterCall)) {   // cancelled before it ran: drop the retain taken when arming
            fReregisterPending = false;
            release();
        }
    }
    super::stop(provider);
}

// ref message @0x1b9d6 (FWMetaNub__message.c): on kIOFWMessageServiceIsRequestingClose copy [+0xf1] -> [+0xf2];
// then vt[0x740](this,type,0,0) = IOService::messageClients(type) and the base IOService::message(type,provider,arg).
// (This corrects Q7 line 65: the "global log hook" is the superclass vtable, i.e. IOService::message, not a logger.)
IOReturn com_openaudio_FWMetaNub::message(UInt32 type, IOService *provider, void *argument)
{
    if (type == 0xE0000020)
        fServiceClosing = fUpdateInProgress;
    messageClients(type, 0, 0);
    return super::message(type, provider, argument);
}

// ---------------------------------------------------------------------------------------------------
// Re-register path (ref @0x1bba6 / 0x1bc5e / 0x1bcc2).

void com_openaudio_FWMetaNub::ReregisterCallout(thread_call_param_t p0, thread_call_param_t)
{
    static_cast<com_openaudio_FWMetaNub *>(p0)->RegregisterDeviceThread();
}

IOReturn com_openaudio_FWMetaNub::ReregisterDeviceAfterFirmwareReset()
{
    fState = kStateReregister;
    if (fStopped) return kIOReturnSuccess;
    if (!fReregisterCall) {
        fReregisterCall = thread_call_allocate(&com_openaudio_FWMetaNub::ReregisterCallout, this);
        if (!fReregisterCall) return kIOReturnNoMemory;
    }
    retain();                                   // ref vt[0x20]; balanced at the end of RegregisterDeviceThread
    uint64_t deadline;
    clock_interval_to_deadline(kReregisterDelayMS, kMillisecondScale, &deadline);
    if (thread_call_enter_delayed(fReregisterCall, deadline)) release();   // was already pending: keep one retain
    fReregisterPending = true;
    return kIOReturnSuccess;
}

void com_openaudio_FWMetaNub::RegregisterDeviceThread()
{
    fReregisterPending = false;
    if (fStopped || (fNub && fNub->isInactive()) || FirmwareCacheInfoRegisters() != kIOReturnSuccess) {
        OA_LOG("M-FW: Reregister: device node vanished.");
    } else {
        OA_LOG("M-FW: Reregister: device still here, re-registering.");
        registerService();
    }
    release();
}

// ---------------------------------------------------------------------------------------------------
// Start (ref @0x1beb4). Halt/Reset/Update are out of scope, see SetOneProperty.

IOReturn com_openaudio_FWMetaNub::FirmwareStart()
{
    fState = kStateFirmwareStart;
    if (!fBootloaderActive) {
        if (!fFirmwareMatchStarted) {
            OA_LOG("M-FW: Firmware active: registering metanub.");
            fFirmwareMatchStarted = true;
            registerService();      // round 2: FWAVCAudioDevice::probe now accepts (Q-SDK5 obs. 5)
        } else {
            OA_LOG("M-FW: Firnware active and we already started matching.");
        }
        return kIOReturnSuccess;
    }
    OA_LOG("M-FW: Bootloader active: starting firmware.");
    UInt32 q[3] = { ProtocolVersion(), kCmdGo, 0 /* start-mode: application */ };
    IOReturn ret = FireWireWriteCommand(kBootloaderAddrHi, kBootloaderReqAddrLo, sizeof(q), q);
    ReregisterDeviceAfterFirmwareReset();
    return ret;
}

// ---------------------------------------------------------------------------------------------------
// Property surface (ref SetOneProperty @0x1ca24, setProperties @0x1cb7c).

// Scope cut (user decision 2026-10-15, BUILD-PLAN M-FW): the clone never writes the device's flash. The reference
// also handles MAFirmwareUpgradeImage (BridgeCo DownloadStart/Block/End, Q-P3 §4-§5) and MAFirmwareHalt (Reset to
// bootloader, only a prelude to an update); neither is implemented. They are refused, loudly, so a stray writer
// is visible in the log. Recoverable from git history (tag before this commit) if ever wanted.
bool com_openaudio_FWMetaNub::SetOneProperty(const OSString *key, OSObject *value)
{
    if (key->isEqualTo("MAFirmwareStart"))                          // value not inspected (ref 0x1cae1..0x1caf8)
        return FirmwareStart() == kIOReturnSuccess;
    if (key->isEqualTo("MAFirmwareHalt") || key->isEqualTo("MAFirmwareUpgradeImage")) {
        OA_LOG("M-FW: %s refused: firmware updates are out of scope for this driver", key->getCStringNoCopy());
        return false;
    }
    return false;
}

// ref setProperties: a dictionary is iterated key by key; a lone OSString is treated as a key.
IOReturn com_openaudio_FWMetaNub::setProperties(OSObject *properties)
{
    OSDictionary *dict = OSDynamicCast(OSDictionary, properties);
    if (!dict) return super::setProperties(properties);
    bool allOk = true;
    OSCollectionIterator *it = OSCollectionIterator::withCollection(dict);
    if (!it) return kIOReturnNoMemory;
    while (const OSSymbol *key = OSDynamicCast(OSSymbol, it->getNextObject())) {
        if (!SetOneProperty(key, dict->getObject(key))) allOk = false;
    }
    it->release();
    return allOk ? kIOReturnSuccess : kIOReturnError;
}
