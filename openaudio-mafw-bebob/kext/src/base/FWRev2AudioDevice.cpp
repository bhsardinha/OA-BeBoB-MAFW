// FWRev2AudioDevice module (base) -- see FWRev2AudioDevice.h.
#include "FWRev2AudioDevice.h"
#include "FWAvc.h"
#include "FWAvcFrames.h"
#include "FWEngine2.h"

#define super com_openaudio_FWAVCAudioDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWRev2AudioDevice, com_openaudio_FWAVCAudioDevice)
#undef super

void com_openaudio_FWRev2AudioDevice::free() { com_openaudio_FWAVCAudioDevice::free(); }

// ref FWRev2AudioDevice::AVCControlPlugSignalFormat @0x12e7e (asm 0x12e7e-0x1305a, Q-AP0 §2.1). The reference direction argument is 1 for the
// input plug (as Rev1 0x18 | dir), so dir' = (direction == 0) = 1 for the output plug. The reference first calls vt[0xa60] GetDeviceChannels and
// discards the result (not mirrored: no effect). The reply buffer is zeroed (the reference's operator new[] is not), so a failed first STATUS
// returns 0xE00002BC as the reference does when byte 13 reads 0.
IOReturn com_openaudio_FWRev2AudioDevice::AVCControlPlugSignalFormat(bool input, UInt8 plug, UInt8 fdf)
{
    if (!fAvcUnit) return kIOReturnNotReady;
    const UInt32 dir = input ? 1 : 0;
    const UInt8 rate = FWAvcRev2RateCode(fdf);
    UInt8 *reply = static_cast<UInt8 *>(IOMalloc(kFWAvcRev2ReplyLen));
    if (!reply) return (IOReturn)0xE00002BD;
    bzero(reply, kFWAvcRev2ReplyLen);
    IOReturn r;
    UInt8 idx = 0;
    for (;;) {
        UInt8 list[kFWAvcRev2ListLen];
        FWAvcRev2FormatListFrame(list, dir, plug, idx);
        UInt32 rl = kFWAvcRev2ReplyLen;
        r = fAvcUnit->AVCCommandWithErrorResponse(list, kFWAvcRev2ListLen, reply, &rl);     // ref vt[0x8b8]
        if (reply[13] == 0) { r = (IOReturn)0xE00002BC; goto done; }
        if (r != kIOReturnSuccess) goto done;
        idx++;
        if (reply[13] == rate) break;
    }
    {
        UInt8 set[2 * 0xff + 0x12];
        const UInt32 len = FWAvcRev2FormatSetFrame(set, sizeof(set), dir, plug, rate, reply);
        UInt32 rl = len;
        r = fAvcUnit->AVCCommandWithErrorResponse(set, len, reply, &rl);
    }
done:
    IOFree(reply, kFWAvcRev2ReplyLen);
    {   // diagnostics ring (AP-3.9): which entry of the list matched and the C0 result
        char t[96]; snprintf(t, sizeof(t), "Rev2 0x2F %s plug %u rate %02x: %u list entries, r=0x%x", input ? "in" : "out", plug, rate, idx, (unsigned)r);
        DebugTrace(t);
    }
    return r;
}

// ref @0x12a54
UInt32 com_openaudio_FWRev2AudioDevice::GetRoundTripLatencyForFDF(UInt32 fdf) const
{
    switch (fdf) {
    case 2: return 0xd2;
    case 3: return 0x1c9;
    case 4: return 0x181;
    case 5: case 6: return 0x18d;
    default: return 0xe2;
    }
}

IOReturn com_openaudio_FWRev2AudioDevice::GetAddressAndPlugForSyncType(UInt32 type, UInt8 plug, UInt8 &addr, UInt8 &srcPlug)
{
    return (IOReturn)FWAvcRev2SyncSource(type, plug, &addr, &srcPlug);
}

// ref AVCSetupSyncStream @0x12c60 (asm 0x12c60-0x12e1d): SIGNAL SOURCE STATUS (only the return code is checked), source from vt[0xbc0], CONTROL.
// Reference quirk kept: if GetAddressAndPlugForSyncType fails, the function returns the STATUS result (success) without the CONTROL.
IOReturn com_openaudio_FWRev2AudioDevice::AVCSetupSyncStream(UInt32 type, UInt8 plug)
{
    if (!fAvcUnit) return kIOReturnNotReady;
    UInt8 cmd[8], resp[8];
    bzero(resp, sizeof(resp));
    FWAvcSignalSourceStatusFrame(cmd, plug);
    UInt32 rl = 8;
    IOReturn r = fAvcUnit->AVCCommandWithErrorResponse(cmd, 8, resp, &rl);
    if (r != kIOReturnSuccess) return r;
    UInt8 addr = 0, src = 0;
    if (GetAddressAndPlugForSyncType(type, plug, addr, src) != kIOReturnSuccess) return r;
    FWAvcSignalSourceControlFrame(cmd, addr, src, plug);
    rl = 8;
    r = fAvcUnit->AVCCommandWithErrorResponse(cmd, 8, resp, &rl);
    char t[80]; snprintf(t, sizeof(t), "Rev2 0x1A sync type %u: source %02x %02x -> plug %u, r=0x%x", (unsigned)type, addr, src, plug, (unsigned)r);
    DebugTrace(t);                                                            // diagnostics ring (AP-3.9)
    return r;
}

// ref UpdatePeakMeters(buf, n): hardware peak slots 0..n-1 of the shared page
void com_openaudio_FWRev2AudioDevice::UpdatePeakMeters(const UInt32 *buf, UInt32 n)
{
    if (!fMeters) return;
    for (UInt32 i = 0; i < n; i++) fMeters[i] = FWRev2PeakFromQuadlet(buf[i]);
}

// ref FWRev2AudioDevice::AllocateAudioEngine @0xb66a (decompile): every Rev2 model streams through the multichannel engine
com_openaudio_FWBaseEngine *com_openaudio_FWRev2AudioDevice::AllocateAudioEngine()
{
    return OSTypeAlloc(com_openaudio_FWEngine2);
}

bool com_openaudio_FWRev2AudioDevice::initHardware(IOService *provider)
{
    if (!com_openaudio_FWAVCAudioDevice::initHardware(provider)) return false;
    QueryPlugChannelInfo();
    return true;
}

// STATUS only; every result goes to the diagnostics ring, nothing is acted on. Directions: 1 = output plug (device -> host), 0 = input plug.
void com_openaudio_FWRev2AudioDevice::QueryPlugChannelInfo()
{
    if (!fAvcUnit) return;
    UInt8 *resp = static_cast<UInt8 *>(IOMalloc(kFWAvcRev2ReplyLen));
    if (!resp) return;
    for (UInt8 dir = 0; dir < 2; dir++) {
        UInt8 cmd[kFWAvcPlugInfoClusterLen];
        FWAvcExtPlugInfoChannelPositionFrame(cmd, dir, 0);
        bzero(resp, kFWAvcRev2ReplyLen);
        UInt32 rl = kFWAvcRev2ReplyLen;
        IOReturn r = fAvcUnit->AVCCommandWithErrorResponse(cmd, kFWAvcPlugInfoPosLen, resp, &rl);
        char sum[160]; UInt32 n = (r == kIOReturnSuccess) ? FWAvcFormatChannelPosition(resp, rl, sum, sizeof(sum)) : 0;
        char t[200]; snprintf(t, sizeof(t), "plug info %s 0 position r=0x%x resp0 %02x: %s", dir ? "out" : "in", (unsigned)r, resp[0], n ? sum : "-");
        DebugTrace(t);
        for (UInt32 c = 1; c <= n && c <= 8; c++) {
            FWAvcExtPlugInfoClusterFrame(cmd, dir, 0, (UInt8)c);
            bzero(resp, kFWAvcRev2ReplyLen);
            rl = kFWAvcRev2ReplyLen;
            r = fAvcUnit->AVCCommandWithErrorResponse(cmd, kFWAvcPlugInfoClusterLen, resp, &rl);
            // response: ... 07 <index> <port type> <name length> <name>  (port type 0x03 line, 0x04 S/PDIF, 0x08 analog, 0x09 digital; FFADO :204-213)
            char name[40] = "";
            const UInt32 nl = (rl > 12) ? resp[12] : 0;
            for (UInt32 k = 0; k < nl && k + 1 < sizeof(name) && 13 + k < rl; k++) name[k] = (char)resp[13 + k];
            snprintf(t, sizeof(t), "plug info %s 0 cluster %u r=0x%x type %02x name '%s'", dir ? "out" : "in", (unsigned)c, (unsigned)r,
                     rl > 11 ? resp[11] : 0xff, name);
            DebugTrace(t);
        }
    }
    IOFree(resp, kFWAvcRev2ReplyLen);
}

