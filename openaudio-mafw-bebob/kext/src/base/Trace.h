// Tracing for the OpenAudio BeBoB MAFW kext.
//
// The reference logs with IOLog ("M-FW: ..." strings, mafw.asm 0x1d159 etc.). 10.13 has os_log in the
// kernel (<os/log.h>, KPI 'com.apple.kpi.dsep'), but its use from a kext is not part of the
// libraries declared in Q-INFO (iokit/libkern/mach). M-A therefore traces with IOLog only; os_log /
// os_signpost move in with M-L (latency tracing) if kpi.dsep is added to OSBundleLibraries.
#ifndef OA_TRACE_H
#define OA_TRACE_H

#include <IOKit/IOLib.h>

#define OA_TAG "OpenAudioBeBoBMAFW: "
#define OA_LOG(fmt, ...)  IOLog(OA_TAG fmt "\n", ##__VA_ARGS__)

#endif
