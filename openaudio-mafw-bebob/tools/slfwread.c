// slfwread -- read-only FireWire reads of an M-Audio BeBoB device through IOFireWireLib (bench tool for the SL 10.6 rig).
// Built on the build PC with the 10.13 SDK for 10.6: clang -arch x86_64 -mmacosx-version-min=10.6 slfwread.c
//   -framework IOKit -framework CoreFoundation -o slfwread
// usage:
//   slfwread status [N [ms]]   read the 60-byte status packet at 0xFFC7:0x00600000 N times (default 1, 50 ms apart) and
//                              print the 14 peaks (upper 16 bits of each big-endian quadlet) and the status word bytes
//   slfwread quad HI LO        read one quadlet at HI:LO (hex)
// It tries the IOFireWireUnit node first, then the IOFireWireDevice node, and reports every open/read status.
// Nothing is written to the device.
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static IOFireWireLibDeviceRef openNode(const char *cls)
{
    CFMutableDictionaryRef m = IOServiceMatching(cls);
    CFNumberRef v; int vid = 0x000D6C;
    v = CFNumberCreate(NULL, kCFNumberIntType, &vid);
    CFDictionarySetValue(m, CFSTR("Vendor_ID"), v);
    CFRelease(v);
    io_service_t s = IOServiceGetMatchingService(kIOMasterPortDefault, m);
    if (!s) { printf("%s: not found\n", cls); return NULL; }
    IOCFPlugInInterface **plug = NULL; SInt32 score;
    kern_return_t kr = IOCreatePlugInInterfaceForService(s, kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, &plug, &score);
    IOObjectRelease(s);
    if (kr || !plug) { printf("%s: plug-in 0x%x\n", cls, kr); return NULL; }
    IOFireWireLibDeviceRef dev = NULL;
    (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID), (LPVOID *)&dev);
    IODestroyPlugInInterface(plug);
    if (!dev) { printf("%s: no device interface\n", cls); return NULL; }
    IOReturn r = (*dev)->Open(dev);
    printf("%s: Open -> 0x%x\n", cls, r);
    if (r) { (*dev)->Release(dev); return NULL; }
    return dev;
}

static IOReturn readBlock(IOFireWireLibDeviceRef dev, UInt16 hi, UInt32 lo, void *buf, UInt32 *size)
{
    FWAddress a; a.nodeID = 0; a.addressHi = hi; a.addressLo = lo;
    return (*dev)->Read(dev, (*dev)->GetDevice(dev), &a, buf, size, false, 0);
}

int main(int argc, char **argv)
{
    if (argc < 2) { fprintf(stderr, "usage: slfwread status [N [ms]] | quad HI LO\n"); return 2; }
    IOFireWireLibDeviceRef dev = openNode("IOFireWireUnit");
    if (!dev) dev = openNode("IOFireWireDevice");
    if (!dev) return 1;
    if (!strcmp(argv[1], "quad") && argc == 4) {
        UInt32 q = 0, size = 4;
        IOReturn r = readBlock(dev, (UInt16)strtoul(argv[2], 0, 16), (UInt32)strtoul(argv[3], 0, 16), &q, &size);
        unsigned char *b = (unsigned char *)&q;
        printf("read -> 0x%x  bytes %02x %02x %02x %02x\n", r, b[0], b[1], b[2], b[3]);
    } else {
        int n = argc > 2 ? atoi(argv[2]) : 1, ms = argc > 3 ? atoi(argv[3]) : 50;
        for (int i = 0; i < n; i++) {
            unsigned char b[60]; UInt32 size = sizeof(b);
            IOReturn r = readBlock(dev, 0xFFC7, 0x00600000, b, &size);
            if (r) { printf("read %d -> 0x%x\n", i, r); break; }
            printf("%3d peaks", i);
            for (int k = 0; k < 14; k++) printf(" %5u", (unsigned)(b[4 * k] << 8 | b[4 * k + 1]));
            printf("  status w0..w3 = %02x %02x %02x %02x\n", b[56], b[57], b[58], b[59]);
            usleep(ms * 1000);
        }
    }
    (*dev)->Close(dev);
    (*dev)->Release(dev);
    return 0;
}
