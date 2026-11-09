// OpenAudio BeBoB MAFW - fwbusreset: issues FireWire bus resets from software through the local node (IOFireWireLib BusReset), to drive the
// driver's bus-reset path (FW1814Device::BusReset -> ReconnectP2P, R144) without touching a cable or a power switch.
//   cc -mmacosx-version-min=10.13 fwbusreset.c -framework IOKit -framework CoreFoundation -o fwbusreset
//   ./fwbusreset [count=1] [gap_ms=0]      e.g. ./fwbusreset 3 400  = three resets 400 ms apart
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int count = argc > 1 ? atoi(argv[1]) : 1, gap = argc > 2 ? atoi(argv[2]) : 0;
    io_service_t node = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOFireWireLocalNode"));
    if (!node) { fprintf(stderr, "no IOFireWireLocalNode\n"); return 1; }
    IOCFPlugInInterface **plug = NULL; SInt32 score = 0;
    kern_return_t kr = IOCreatePlugInInterfaceForService(node, kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, &plug, &score);
    if (kr || !plug) { fprintf(stderr, "IOCreatePlugInInterfaceForService 0x%x\n", kr); return 1; }
    IOFireWireLibDeviceRef dev = NULL;
    (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID), (void **)&dev);
    if (!dev) { fprintf(stderr, "no device interface\n"); return 1; }
    IOReturn r = (*dev)->Open(dev);
    if (r) { fprintf(stderr, "Open 0x%x\n", r); return 1; }
    for (int i = 0; i < count; i++) {
        r = (*dev)->BusReset(dev);
        printf("bus reset %d -> 0x%x\n", i + 1, r);
        if (gap && i + 1 < count) usleep((useconds_t)gap * 1000);
    }
    (*dev)->Close(dev);
    (*dev)->Release(dev);
    IODestroyPlugInInterface(plug);
    return 0;
}
