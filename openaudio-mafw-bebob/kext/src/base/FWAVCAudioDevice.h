// FWAVCAudioDevice module (base) -- reference m_audio_b_FWAVCAudioDevice (A1 Q-MC, Q-ME §7b, Q-AP0 §5, Q-AP0c).
//
// What the reference does in this layer (every BeBoB model talks AV/C):
//  * probe @0x22f78: the only probe in the chain; rejects until the nub's firmware is running (two-round attach, Q-SDK5).
//  * initHardware: parent initHardware -> SetBlankSlateClockSource (vt 0xb58) -> ResendCurrentMixerSettings (vt 0xb00) ->
//    StartReceptionOfControlPackets (vt 0xb78).
//  * AV/C unit + connection manager (AllocateConnectionManager @0x23c72), the four function-block builders (selector / feature / processing
//    mixer pair, async queue), CustomMixerWriteQuads (vendor window 0xFFC7:0x00700000).
//  * MAClockSource (SetClockSource @0x23858: pause engine, legacy-value conversion, SetClockSourceInternal = MODEL/revision, external-sync
//    flag, resume); base SetClockSourceInternal maps kind 4 through the model's MapChannelAndFunctionBlock.
//  * Mixer addressing is per model: MapChannelAndFunctionBlock (vt 0xb48) and MapRoutingParameters (vt 0xb40) turn a generic {kind, channel}
//    or {bus, pair, mixer} into the model's function block / plug; SetHeadphoneSelector and AVCControlSetLEDStatus are model bodies.
//  * Control-packet poll (FWHSCI, 50 ms, GetControlPacketSize bytes from 0xFFC7:0x00600000 -> the model's ReadHSCIData); sleep/wake.
#ifndef OA_FWAVCAUDIODEVICE_H
#define OA_FWAVCAUDIODEVICE_H

#include "FWAudioDevice.h"
#include "FWMixerMap.h"

class com_openaudio_FWAVCAudioDevice : public com_openaudio_FWAudioDevice
{
    OSDeclareAbstractStructors(com_openaudio_FWAVCAudioDevice)
public:
    // The only probe override in the device chain (Q-SDK5 obs. 5, ref @0x22f78).
    virtual IOService *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
    virtual bool initHardware(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService *provider) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    com_openaudio_FWMetaNub *metaNub() const { return fMetaNub; }
    com_openaudio_FWAvcUnit *avcUnit() const { return fAvcUnit; }
    com_openaudio_FWAvcConnectionManager *connectionManager() const { return fConnMgr; }
    // ref FWAVCAudioDevice::SetBlankSlateClockSource @0x22de8: with an engine, SetClockSourceInternal(0x40000000, true) (internal clock); then
    // IOSleep(2500) always. The FW1814 overrides it (@0xe4f0); the Rev2 models use this one (Q-AP0 §5).
    virtual IOReturn SetBlankSlateClockSource() APPLE_KEXT_OVERRIDE;
    // ref vt[0xba0] GetControlPacketSize / ReadHSCIData: per model (FW1814 @0xc962 / @0xd952, Audiophile @0xaf7e / @0xba92); Rev2 ReadHSCIData is
    // empty. The clone's base size 0 is a default (no base body in the exports); every model that polls overrides it.
    virtual UInt32 GetControlPacketSize() const { return 0; }
    virtual void ReadHSCIData(const UInt32 *packet) { (void)packet; }
    // Control-packet poll, ONE implementation for every model (generic-first, plan v4 AP-2.5 (d)). The reference repeats the same code per
    // class: FW1814 Start @0xe064 / Stop @0xda7e / Pause @0xdbbe / Resume @0xdba6 / stop @0xd9ce (HSCI at [+0x1270]) and Rev2 @0x13138 / Stop /
    // Pause / Resume / stop (HSCI at [+0x2c8]); all build an FWHSCI of GetControlPacketSize() bytes with ReadHSCIData as callback on the work loop,
    // Stop and Pause also ClearPeakMeters, stop() releases the HSCI before the parent. A model with no packet size (0) starts nothing.
    virtual bool StartReceptionOfControlPackets() APPLE_KEXT_OVERRIDE;
    void StopReceptionOfControlPackets();
    virtual void PauseReceptionOfControlPackets() APPLE_KEXT_OVERRIDE;
    virtual void ResumeReceptionOfControlPackets() APPLE_KEXT_OVERRIDE;
    bool IsReceptionOfControlPacketsEnabled() const { return fHsci != 0; }
    // ref CustomMixerWriteQuads @0x22c7a (vt[0xba8]): async block write of `quads` quadlets to 0xFFC7:0x00700000 + off (no-op without a nub)
    virtual IOReturn CustomMixerWriteQuads(UInt32 off, UInt32 quads, const void *data);
    virtual IOReturn InitializeUserSettableProperties() APPLE_KEXT_OVERRIDE;   // ref @0x...: base + MAFirmwareInfo + MAClockSource
    virtual void BusReset() APPLE_KEXT_OVERRIDE;                               // ref FWAudioDevice::BusReset @0x1d3c: connection manager replay
    UInt32 fModeBeforeSleep;                                                  // clone (R134): 0 = none
    bool     fMixerHold;                                                      // clone (R134): mixer writes held from sleep to the end of SystemWokeUp
    UInt64   fMixerHoldDeadline;                                              // clone (R134): uptime (abs) after which a forgotten hold lapses; 0 = none
    UInt32   fMixerHeldWrites;                                                // clone (R134): writes skipped while held (log only)
    UInt32   fHeldRegs[64];                                                   // clone (R134): last value per mixer quadlet (offset / 4) written while held
    UInt64   fHeldMask;                                                       // clone (R134): which of fHeldRegs are pending
    void     FlushHeldMixerWrites();
    void     ArmMixerHoldDeadline();
    virtual void WokeFromSleepArmed() APPLE_KEXT_OVERRIDE { ArmMixerHoldDeadline(); }
    virtual void SystemWillSleep() APPLE_KEXT_OVERRIDE;                        // ref @0x22b16
    virtual void SystemWokeUp() APPLE_KEXT_OVERRIDE;                           // ref @0x22d76
    virtual bool HasSoftwareControls() const { return true; }                  // vt[0xac8] (ref FW1814 @0xc32c)
    virtual void ResendCurrentMixerSettings() APPLE_KEXT_OVERRIDE;             // ref @0x23af8: throttle + clock source re-assert
    IOReturn SetClockSource(UInt32 clk);                                       // ref @0x23858 (MAClockSource setter)
    virtual IOReturn SetClockSourceInternal(UInt32 clk, bool blank) APPLE_KEXT_OVERRIDE;   // ref @0x23148: map kind 4 -> AV/C selector command

    // ---- per-model mixer mapping (AP-2.2; A1 Q-ME §7b, Q-AP0c §2-3). The generic FWSettings* setters reach the hardware only through these.
    // ref m_audio_b_FWAVCAudioDevice::FunctionBlockMapping / ::Routing (layouts in base/FWMixerMap.h, kernel-free)
    typedef FWFunctionBlockMapping FunctionBlockMapping;
    typedef FWRouting Routing;
    // vt[0xb48]: in {kind, ch}, out {fb, ch}. No base body in the reference exports (each model overrides it); the clone's base reports unsupported.
    virtual IOReturn MapChannelAndFunctionBlock(FunctionBlockMapping &m) { (void)m; return (IOReturn)0xE00002C7; }
    // vt[0xb40] ref @0x22a80: function block 1, plug 0, pairs unchanged
    virtual IOReturn MapRoutingParameters(Routing &r) { r.fb = 1; r.plug = 0; return kIOReturnSuccess; }
    // ABSwitch functors (ref [+0x28] / [+0x18]); per-model bodies (FW1814 @0xd1f8 / @0xdaaa)
    virtual IOReturn SetHeadphoneSelector(UInt32 pair, UInt32 which) { (void)pair; (void)which; return (IOReturn)0xE00002C7; }
    virtual IOReturn AVCControlSetLEDStatus(UInt32 state) { (void)state; return (IOReturn)0xE00002C7; }
    // the reference's AV/C function-block builders (@0x23074 / @0x231a8 / @0x2367c), frames in FWAvc.cpp, sent through the async queue
    IOReturn AVCSelectorCommand(UInt8 fb, UInt8 value);
    IOReturn AVCControlFeatureFunctionBlock(UInt8 fb, UInt8 controlSelector, UInt8 channel, UInt16 value);
    IOReturn AVCMixerConnectPair(bool connect, UInt8 fb, UInt8 plug, UInt8 inPair, UInt8 outPair);
    // ref AVCControlFunctionBlock: fills bytes 0-5 of `frame` (00 08 B8 <type> <fb> 10) and queues it (async, vt[0x8b0])
    IOReturn AVCControlFunctionBlock(UInt8 type, UInt8 fb, UInt8 *frame, UInt8 len);
    // ref AVCVendorCommand @0x23970 / @0x23ad8: 16-byte M-Audio vendor-dependent CONTROL (FWAvcFrames.h), synchronous (vt[0x8b8])
    IOReturn AVCVendorCommand(UInt8 a, UInt8 b, UInt32 n, const UInt8 *data);
    IOReturn AVCVendorCommand(UInt8 a, UInt8 b, UInt8 value) { return AVCVendorCommand(a, b, 1, &value); }
protected:
    UInt64 fLastResend;                                     // ref [+0x270]
    com_openaudio_FWHsci *fHsci;                            // ref [+0x2c8] (Rev2) / [+0x1270] (FW1814)
    com_openaudio_FWAvcUnit *fAvcUnit;                      // ref [+0x268]
    com_openaudio_FWAvcConnectionManager *fConnMgr;         // ref [+0xf0]
    virtual bool AllocateConnectionManager() APPLE_KEXT_OVERRIDE;     // ref @0x23c72
    void ReleaseConnectionManager();
};

#endif
