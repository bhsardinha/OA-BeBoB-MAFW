// FW1814Settings (module models/FW1814): the FW1814's settings subclasses. The generic settings (base/FWSettings.h) send through AV/C
// function blocks; the FW1814's mixer is a register block, so the reference subclasses override the three setters that address it and call the
// FW1814Device register writers (A1 Q15 "The hardware-apply path"; decomp FWSettings1814*__*.c).
#ifndef OA_FW1814SETTINGS_H
#define OA_FW1814SETTINGS_H

#include "FWSettings.h"

// ---- FW1814 subclasses (ref FWSettings1814Levels ctor @0xeec0, FWSettings1814AuxSend ctor @0xefda: same members, three overrides) -------
class com_openaudio_FWSettings1814Levels : public com_openaudio_FWSettingsLevels
{
    OSDeclareDefaultStructors(com_openaudio_FWSettings1814Levels)
public:
    virtual IOReturn SetInputPan(UInt32 idx, UInt32 v) APPLE_KEXT_OVERRIDE;     // ref @0xd128 -> FW1814Device vt[0xbc0]
    virtual IOReturn SetInputRouting(UInt32 idx, UInt32 v) APPLE_KEXT_OVERRIDE; // ref @0xd15c -> FW1814Device vt[0xbb8]
};

class com_openaudio_FWSettings1814AuxSend : public com_openaudio_FWSettingsAuxSend
{
    OSDeclareDefaultStructors(com_openaudio_FWSettings1814AuxSend)
public:
    virtual IOReturn SetAuxMonitor(UInt32 idx, UInt32 v) APPLE_KEXT_OVERRIDE;   // ref @0xd0b4 -> FW1814Device vt[0xbc8]
};

#endif
