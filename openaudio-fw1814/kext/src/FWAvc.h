// FWAvc -- AV/C over FCP. Mirrors m_audio_b_FWAVCUnit (A1 Q-MC, decomp FWAVCUnit__*.c) and
// FWRev1AudioDevice::AVCControlPlugSignalFormat @0x13734 (decomp FWRev1AudioDevice__AVCControlPlugSignalFormat.c).
// The reference unit is a private IOFireWireAVCUnit SUBCLASS (see com_openaudio_FWAvcUnit below); the clone builds the same. Correction
// (2026-10-17): the earlier text said the clone could use IOFireWireAVCCommand::withNub directly on the nub -- that never received a response.
// Scope: the synchronous commands the stream start needs AND (since 2026-10-17) the async queue (AVCCommand @0x17fc / FinishAsyncCommand
// @0x16c8) and updateSubUnits are not used by the stream start and arrive with the milestone that needs them (M-E).
#ifndef OA_FWAVC_H
#define OA_FWAVC_H

#include <libkern/c++/OSObject.h>
#include <IOKit/IOLocks.h>
#include <IOKit/firewire/IOFireWireNub.h>
#include <IOKit/avc/IOFireWireAVCCommand.h>
#include <IOKit/avc/IOFireWireAVCUnit.h>

class com_openaudio_FWAvcUnit;

// ref m_audio_b_FWAsyncAVCCommand (A1 Q-MC, asm 0x13ac..): an IOFireWireAVCAsynchronousCommand with the queue link at [+0x98] and the owning
// unit at [+0xa0]. Only the head of the unit's list is submitted; the completion callback starts the next one.
class com_openaudio_FWAsyncAVCCommand : public IOFireWireAVCAsynchronousCommand
{
    OSDeclareDefaultStructors(com_openaudio_FWAsyncAVCCommand)
public:
    com_openaudio_FWAsyncAVCCommand *fNext;      // ref [+0x98]
    com_openaudio_FWAvcUnit         *fUnit;      // ref [+0xa0]
};

// ref m_audio_b_FWAVCUnit (A1 Q-MC; object 0xF8 B, created by AllocateConnectionManager @0x23c72): a private subclass of Apple's IOFireWireAVCUnit.
// Correction (2026-10-17, SW-return routing bug): the clone used a bare OSObject and Apple's IOFireWireAVCCommand::withNub on the nub with no
// IOFireWireAVCUnit anywhere, so no FCP response was ever delivered and every command timed out after ~1.27 s (the device still executed it).
// The reference builds a real IOFireWireAVCUnit: init(copy of the nub's properties), attach(the device), start(the FireWire nub). On the SL rig it
// is a registered child of FW1814Device (ioreg) and appears in an IOFireWireAVCUnit class match. start / stop are guarded calls into the base
// class (byte flag at [+0xe0], asm 0x1692 / 0x165e), updateSubUnits is empty (asm 0x13cc: no Apple sub-unit nubs), commands go through the
// reference's own entry points: AVCCommandExtendedTimeout @0x1594 (vt 0x8a8), AVCCommand @0x17fc = async queue (vt 0x8b0),
// AVCCommandWithErrorResponse @0x1508 (vt 0x8b8).
class com_openaudio_FWAvcUnit : public IOFireWireAVCUnit
{
    OSDeclareDefaultStructors(com_openaudio_FWAvcUnit)
public:
    // ref AllocateConnectionManager @0x23c72, steps 1-6: props = nub->dictionaryWithProperties; init(props); attach(device); start(nub).
    bool Setup(IOService *device, IOFireWireNub *nub);
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool start(IOService *provider) APPLE_KEXT_OVERRIDE;          // ref @0x1692
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;           // ref @0x165e
    virtual void updateSubUnits(bool firstTime) APPLE_KEXT_OVERRIDE;      // ref @0x13cc: empty

    // ref AVCCommandExtendedTimeout @0x1594 (vt 0x8a8): 1..0x200 byte command, 1 s timeout, one command at a time.
    IOReturn AVCCommandExtendedTimeout(const UInt8 *cmd, UInt32 len, UInt8 *resp, UInt32 *respLen);
    // ref AVCCommandWithErrorResponse @0x1508 (vt 0x8b8): up to 25 retries (IOSleep 1000) on bus reset, then CheckAVCError.
    IOReturn AVCCommandWithErrorResponse(const UInt8 *cmd, UInt32 len, UInt8 *resp, UInt32 *respLen);
    // ref AVCCommand @0x17fc (vt 0x8b0): queue an asynchronous command; the response is not looked at (the reference callers pass no functor).
    IOReturn AVCCommandAsync(const UInt8 *cmd, UInt32 len);
    // ref FinishAsyncCommand @0x16c8: pop the head, submit the next, release.
    void FinishAsyncCommand(com_openaudio_FWAsyncAVCCommand *c);
    // ref CheckAVCError @0x14b6: maps the response code and inserts the post-command pause the device needs.
    static IOReturn CheckAVCError(IOReturn r, const UInt8 *resp);

    IOFireWireNub *nub() const { return fDevice; }          // IOFireWireAVCNub::fDevice, set by the base start (ref [+0x88])
private:
    void StartQueueHead();                                  // submit the head until one is accepted (a refused command is dropped)
    // Inherited protected state used exactly as in the reference (asm 0x1594): avcLock = [+0x98] serialises commands, fCommand = [+0xa0] is the
    // command the base class's AVCResponse hands the FCP response to, cmdLock = [+0xd0] guards fCommand, fStarted = byte [+0xe0].
    IOLock        *fAsyncLock;    // the reference has no lock here; the clone's callers and the FireWire completion run on different threads
    com_openaudio_FWAsyncAVCCommand *fAsyncHead;   // ref [+0xe8]
    com_openaudio_FWAsyncAVCCommand *fAsyncTail;   // ref [+0xf0]
};

// ref FWRev1AudioDevice::AVCControlPlugSignalFormat @0x13734: 8-byte CONTROL
//   00 FF (0x18 | input) <plug> 90 <FDF/SFC index> FF FF   (Rev1 ignores the other three arguments; Q3a §3 Correction)
// via [+0x268]->vt[0x8b8] = AVCCommandWithErrorResponse. The result is not checked by the caller (ConnectAudioMIDIChannels).
IOReturn FWAvcControlPlugSignalFormat(com_openaudio_FWAvcUnit *unit, bool input, UInt8 plug, UInt8 fdf);

// ref FWAVCAudioDevice::AVCSelectorCommand @0x23074: 12-byte CONTROL to the audio subunit, SELECTOR (0xB8) of function block `selector`:
//   00 08 B8 80 <selector> 10 02 <value> 01 00 00 00
// The reference sends it asynchronously through [+0x268]->vt[0x8b0] (AVCCommand @0x17fc) and ignores the response; the clone sends it
// synchronously (deviation: the async command queue arrives with the AV/C slice, Q-ME §8), so the caller blocks for the device's answer.
IOReturn FWAvcSelectorCommand(com_openaudio_FWAvcUnit *unit, UInt8 selector, UInt8 value);

// ref FWAVCAudioDevice::AVCControlFeatureFunctionBlock @0x231a8: 12-byte CONTROL on the audio subunit, FUNCTION BLOCK (0xB8), feature block (0x81):
//   00 08 B8 81 <fb> 10 02 <channel> <control selector> 02 <value hi> <value lo>          (control selector 2 = volume, 3 = balance)
IOReturn FWAvcFeatureFunctionBlock(com_openaudio_FWAvcUnit *unit, UInt8 functionBlock, UInt8 controlSelector, UInt8 channel, UInt16 value);
// ref FWAVCAudioDevice::AVCMixerConnectSingle @0x23586 / AVCMixerConnectPair @0x2367c: 16-byte CONTROL, FUNCTION BLOCK, processing (0x82):
//   00 08 B8 82 <fb> 10 04 <b> <src> <dst> 03 02 <0x00 connect | 0x80 disconnect> 00 00 00
// The pair connects (2*src-1 -> 2*dst-1) and (2*src -> 2*dst) (left / right).
IOReturn FWAvcMixerConnectSingle(com_openaudio_FWAvcUnit *unit, bool connect, UInt8 fb, UInt8 b, UInt8 src, UInt8 dst);
IOReturn FWAvcMixerConnectPair(com_openaudio_FWAvcUnit *unit, bool connect, UInt8 fb, UInt8 b, UInt8 src, UInt8 dst);

#endif
