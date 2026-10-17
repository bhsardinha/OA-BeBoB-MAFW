// com_openaudio_MakeAppleFWAudioGoAway -- probe-steal so AppleFWAudio stays off the FW1814's AV/C sub-units.
// Mirrors ref probe @0x129cc (Q8). The reference plist only scopes it to "FW Solo"; our plist extends it to
// "FW 1814" (Q-INFO personality 3, deliberate deviation for 10.13 matching).
#ifndef OA_MAKEAPPLEFWAUDIOGOAWAY_H
#define OA_MAKEAPPLEFWAUDIOGOAWAY_H
#include <IOKit/IOService.h>

class com_openaudio_MakeAppleFWAudioGoAway : public IOService
{
    OSDeclareDefaultStructors(com_openaudio_MakeAppleFWAudioGoAway)
public:
    virtual IOService *probe(IOService *provider, SInt32 *score) APPLE_KEXT_OVERRIDE;
};
#endif
