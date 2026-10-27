// FWAudioDevice module (base) -- see FWAudioDevice.h for what the reference does in this layer.
#include "FWAudioDevice.h"
#include "FWAVCAudioDevice.h"
#include "FWBaseEngine.h"
#include "FWMetaNub.h"
#include "FWPropertyDict.h"
#include "Trace.h"
#include <libkern/c++/OSString.h>

#define super IOAudioDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWDiscreteDevice, IOAudioDevice)
#undef super
#define super com_openaudio_FWDiscreteDevice
OSDefineMetaClassAndAbstractStructors(com_openaudio_FWAudioDevice, com_openaudio_FWDiscreteDevice)
#undef super

void com_openaudio_FWDiscreteDevice::free() { IOAudioDevice::free(); }

#include "FWBaseEngine.h"
#include "FWEngine2.h"

// ref GetBandwidthBytesAtSampleRate @0x1ee2
UInt32 com_openaudio_FWAudioDevice::GetBandwidthBytesAtSampleRate(UInt32 rateHz) const
{
    FWDeviceChannels ch;
    GetDeviceChannelsForSampleRate(rateHz, ch);
    return FWBandwidthBytesPerCycle(ch, fMainEngine ? fMainEngine->sytInterval() : 0);
}

// ref RequestFormatUpdateForSampleRate @0x2a84: the rate's index in the table's rate list (0 when absent), then SetIOFormat.
IOReturn com_openaudio_FWAudioDevice::RequestFormatUpdateForSampleRate(UInt32 rateHz)
{
    if (!fMainEngine) return kIOReturnNotReady;
    FWDeviceChannels ch;
    GetDeviceChannelsForSampleRate(rateHz, ch);
    UInt32 idx = 0;
    for (UInt32 i = 0; ch.outRates[i].whole != 0; i++) if (ch.outRates[i].whole == rateHz) { idx = i; break; }
    return fMainEngine->SetIOFormat(ch, idx);
}

// ref createAudioEngine @0x2fd8
bool com_openaudio_FWAudioDevice::createAudioEngine()
{
    deactivateAllAudioEngines();                                 // ref vt[0x988]
    fMainEngine = 0;
    com_openaudio_FWBaseEngine *eng = AllocateAudioEngine();
    if (!eng) return false;
    if (!eng->init(0)) { eng->release(); return false; }
    SetupAudioEngine(eng);
    SetChannelNamesForMainEngine(eng);
    if (activateAudioEngine(eng) != kIOReturnSuccess) { eng->release(); return false; }
    if (OSString *uid = eng->getGlobalUniqueID()) { setProperty("MAMainEngineUniqueID", uid); uid->release(); }   // ref vt[0xb30]
    fMainEngine = eng;                                           // ref [+0xe8]
    eng->release();                                              // the device keeps its own reference (IOAudioDevice::audioEngines)
    return true;
}

#include <IOKit/firewire/IOFireWireNub.h>

// ref GetDeviceChannels @0x1e52: the row for the engine's current rate
void com_openaudio_FWAudioDevice::GetDeviceChannels(FWDeviceChannels &out) const
{
    GetDeviceChannelsForSampleRate(fMainEngine ? fMainEngine->rateHz() : 44100, out);
}

UInt32 com_openaudio_FWAudioDevice::FetchIndexIntoSampleRateArray(const FWSampleRateEntry *rates, UInt32 rate)
{
    for (UInt32 i = 0; rates[i].whole != 0; i++) if (rates[i].whole == rate) return i;
    return 0;
}

#include "FWBusReset.h"

static void WakeAfterSleepThread(void *arg)
{
    com_openaudio_FWAudioDevice *d = static_cast<com_openaudio_FWAudioDevice *>(arg);
    OA_LOG("WAKE-DIAG: SystemWokeUp begin");
    d->SystemWokeUp();
    OA_LOG("WAKE-DIAG: SystemWokeUp end");
    d->release();
}

// ref FWAudioDevice::performPowerStateChange @0x1e96 (decomp FWAudioDevice__performPowerStateChange.c)
IOReturn com_openaudio_FWAudioDevice::performPowerStateChange(IOAudioDevicePowerState oldState, IOAudioDevicePowerState newState, UInt32 *us)
{
    *us = 0;
    OA_LOG("WAKE-DIAG: performPowerStateChange %d -> %d (fWokeFromSleep %d)", (int)oldState, (int)newState, (int)fWokeFromSleep);
    if (oldState == kIOAudioDeviceSleep && newState > kIOAudioDeviceSleep) { fWokeFromSleep = true; WokeFromSleepArmed(); }
    else if (oldState > kIOAudioDeviceSleep && newState == kIOAudioDeviceSleep) SystemWillSleep();
    return kIOReturnSuccess;
}

// ref FWAudioDevice::message @0x3344: after the device is ready, "service resumed" (the FireWire bus reset is over) triggers BusReset();
// when it follows a system sleep the reference instead runs WakeAfterSleep on a new thread (power management, a later item).
// Every other message is swallowed (the reference returns 0 without calling the superclass).
IOReturn com_openaudio_FWAudioDevice::message(UInt32 type, IOService *, void *)
{
    OA_LOG("WAKE-DIAG: message 0x%x (fReady %d, fWokeFromSleep %d)", (unsigned)type, (int)fReady, (int)fWokeFromSleep);
    if (fReady && type == kIOMessageServiceIsResumed) {
        if (!fWokeFromSleep) BusReset();                                      // ref vt[0xb20]
        else {                                                                // ref: WakeAfterSleep (= SystemWokeUp) on a new thread
            retain();
            if (!FWRunOnNewThread(&WakeAfterSleepThread, this)) release();
            fWokeFromSleep = false;
        }
    }
    return kIOReturnSuccess;
}

#include "FWPairEngine.h"

// ref SetExportStereoPairEngines @0x1dd6: with an engine, 0 removes every pair engine and anything else creates them
IOReturn com_openaudio_FWAudioDevice::SetExportStereoPairEngines(UInt32 on)
{
    if (!fMainEngine) return kIOReturnSuccess;
    if (on == 0) fMainEngine->RemoveAllSubEngines();
    else CreateStereoPairSubEngines();
    return kIOReturnSuccess;
}

// ref CreateStereoPairSubEngines @0x30f2 (decomp FWAudioDevice__CreateStereoPairSubEngines.c). One pair engine per (signal type, pair index
// inside its group); an input and an output group of the same (type, index) share the engine. Engine bit numbers count up for EVERY pair
// visited (also when the engine already exists), starting at 1 -- bit 0 is the main engine. The channel offset of a pair is (first pair * 2 - 2)
// plus 2 per further pair of its group. Names (ref NameAndActivatePairEngines): "<device> <Analog|S/PDIF|ADAT> <2n-1>/<2n>[ In| Out]", where
// the suffix is present only for an engine with a single direction.
void com_openaudio_FWAudioDevice::CreateStereoPairSubEngines()
{
    if (!fMainEngine) return;
    FWDeviceChannels row;
    GetDeviceChannels(row);
    struct Trio { UInt32 type, idx; com_openaudio_FWPairEngine *engine; };
    Trio trios[16]; unsigned nTrios = 0;
    UInt32 bitIndex = 1;
    for (UInt32 g = 0; g < 5 && row.group[g][3] != 3; g++) {
        const bool input = row.group[g][0] != 0;
        const UInt32 type = row.group[g][3];
        UInt32 chan = row.group[g][1] * 2 - 2;
        UInt32 idx = 1;
        for (UInt32 pair = row.group[g][1]; pair <= row.group[g][2]; pair++, idx++, bitIndex++, chan += 2) {
            com_openaudio_FWPairEngine *pe = 0;
            for (unsigned k = 0; k < nTrios; k++) if (trios[k].type == type && trios[k].idx == idx) { pe = trios[k].engine; break; }
            if (!pe) {
                if (nTrios >= 16) return;
                pe = OSTypeAlloc(com_openaudio_FWPairEngine);
                if (!pe) return;
                if (!pe->init(0)) { pe->release(); return; }
                pe->SetMasterEngine(fMainEngine);
                pe->SetIndex(bitIndex);
                fMainEngine->AddSubEngine(pe);                                  // the master's set keeps it alive
                pe->release();
                trios[nTrios].type = type; trios[nTrios].idx = idx; trios[nTrios].engine = pe; nTrios++;
            }
            pe->AddDirection(!input, chan);
        }
    }
    for (unsigned k = 0; k < nTrios; k++) {
        com_openaudio_FWPairEngine *pe = trios[k].engine;
        const char *tname = (trios[k].type == 1) ? "S/PDIF" : (trios[k].type == 2) ? "ADAT" : "Analog";   // ref kSPDIFString / kADATString / kAnalogString @0x2da8f..
        char name[256];
        snprintf(name, sizeof(name), "%s %s %d/%d%s", GetDescriptionString(), tname, (int)(trios[k].idx * 2 - 1), (int)(trios[k].idx * 2),
                 !pe->hasInput() ? " Out" : !pe->hasOutput() ? " In" : "");
        pe->setDescription(name);
        activateAudioEngine(pe);
    }
}
