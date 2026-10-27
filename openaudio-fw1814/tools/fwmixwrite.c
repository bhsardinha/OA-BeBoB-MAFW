// Bench tool (R134): write 2 quadlets of 0 to the FW1814 mixer window (node address 0xFFC7:0x00700000 + off, default 0x08) from user space with IOFireWireLib,
// to tell "the device refuses mixer writes" from "the kext's nub object fails". usage: fwmixwrite [off_hex] [GUID_hex]
#include <IOKit/IOKitLib.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char **argv) {
    UInt32 off = argc > 1 ? (UInt32)strtoul(argv[1], 0, 16) : 0x08;
    io_iterator_t it;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOFireWireDevice"), &it)) return 1;
    io_service_t s;
    while ((s = IOIteratorNext(it))) {
        CFNumberRef g = IORegistryEntryCreateCFProperty(s, CFSTR("GUID"), 0, 0);
        UInt64 guid = 0; if (g) CFNumberGetValue(g, kCFNumberSInt64Type, &guid);
        CFNumberRef m = IORegistryEntryCreateCFProperty(s, CFSTR("Model_ID"), 0, 0);
        int model = 0; if (m) CFNumberGetValue(m, kCFNumberSInt32Type, &model);
        if (model != 0x10071) continue;
        printf("device GUID %llx model %x\n", guid, model);
        IOCFPlugInInterface **plug; SInt32 score;
        if (IOCreatePlugInInterfaceForService(s, kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, &plug, &score)) { puts("plugin failed"); return 1; }
        IOFireWireLibDeviceRef dev;
        (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID_v9), (void **)&dev);
        IOReturn r = (*dev)->Open(dev); printf("open 0x%x\n", r);
        if (argc > 2 && argv[2][0] == 'h') { // read-only: dump the HSCI status window 0xFFC7:0x00600000 (96 bytes), twice, 1 s apart
            for (int n = 0; n < 2; n++) { FWAddress h = { 0, 0xFFC7, 0x00600000 }; UInt32 hq[24] = {0}; UInt32 hs = sizeof hq; UInt32 g; (*dev)->GetBusGeneration(dev, &g);
                r = (*dev)->Read(dev, (*dev)->GetDevice(dev), &h, hq, &hs, kFWDontFailOnReset, g); printf("HSCI read -> 0x%x gen %u\n", r, g);
                for (int i = 0; i < 24; i++) printf("%08x%s", hq[i], (i % 6 == 5) ? "\n" : " "); sleep(1); }
            (*dev)->Close(dev); return 0; }
        if (argc > 2) { UInt32 g0, g1; (*dev)->GetBusGeneration(dev, &g0); FWAddress rom = { 0, 0xFFFF, 0xF0000400 }; UInt32 rq[2]; UInt32 rs = 8; r = (*dev)->Read(dev, (*dev)->GetDevice(dev), &rom, rq, &rs, kFWDontFailOnReset, g0); usleep(500000); (*dev)->GetBusGeneration(dev, &g1); printf("rom-only: read 0x%x gen %u -> %u\n", r, g0, g1); (*dev)->Close(dev); return 0; }
        FWAddress a = { 0, 0xFFC7, 0x00700000 + off };
        UInt32 q[2] = { 0, 0 }; UInt32 sz = 8;
        UInt32 gen; (*dev)->GetBusGeneration(dev, &gen);
        r = (*dev)->Write(dev, (*dev)->GetDevice(dev), &a, q, &sz, kFWDontFailOnReset, gen);
        printf("write off 0x%x -> 0x%x (gen %u)\n", off, r, gen);
        r = (*dev)->Write(dev, (*dev)->GetDevice(dev), &a, q, &sz, kFWDontFailOnReset, 0);
        printf("write again (gen 0) -> 0x%x\n", r);
        { FWAddress rom = { 0, 0xFFFF, 0xF0000400 }; UInt32 rq[2] = {0,0}; UInt32 rs = 8;
          r = (*dev)->Read(dev, (*dev)->GetDevice(dev), &rom, rq, &rs, kFWDontFailOnReset, gen); printf("read config ROM -> 0x%x (%08x %08x)\n", r, rq[0], rq[1]); }
        { FWAddress mw = { 0, 0xFFC7, 0x00700000 + off }; UInt32 rq[2] = {0,0}; UInt32 rs = 8;
          r = (*dev)->Read(dev, (*dev)->GetDevice(dev), &mw, rq, &rs, kFWDontFailOnReset, gen); printf("read mixer window -> 0x%x (%08x %08x)\n", r, rq[0], rq[1]); }
        { FWAddress bl = { 0, 0xFFFF, 0xF0000B00 }; UInt32 rq[1] = {0}; UInt32 rs = 4;
          r = (*dev)->Read(dev, (*dev)->GetDevice(dev), &bl, rq, &rs, kFWDontFailOnReset, gen); printf("read FCP response reg -> 0x%x\n", r); }
        (*dev)->Close(dev);
        return 0;
    }
    puts("no FW1814 runtime device"); return 1;
}
