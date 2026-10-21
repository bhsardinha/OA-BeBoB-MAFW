// Bench tool: latency of the kext's change notification, the path the panel uses (vendor user client type 0, notification port type 0).
// Writes MAMetaVirtualRotateDataKnob (+1, then -1: the same code path as a front-panel click) N times and prints, for each, the time until
// the type-0 "properties changed" message arrives. MA_CLASS picks the unit (default: the first com_openaudio_FWAVCAudioDevice).
//   notelat [N]        default 10
// Build: cc -o notelat notelat.c -framework IOKit -framework CoreFoundation
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <stdio.h>
#include <stdlib.h>

static double ms(uint64_t t) { static mach_timebase_info_data_t tb; if (!tb.denom) mach_timebase_info(&tb); return (double)t * tb.numer / tb.denom / 1e6; }

int main(int argc, char **argv)
{
    int n = argc > 1 ? atoi(argv[1]) : 10;
    const char *cls = getenv("MA_CLASS") ? getenv("MA_CLASS") : "com_openaudio_FWAVCAudioDevice";
    io_service_t s = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(cls));
    if (!s) { printf("no %s\n", cls); return 1; }
    io_connect_t c;
    if (IOServiceOpen(s, mach_task_self(), 0, &c)) { puts("IOServiceOpen failed"); return 1; }
    mach_port_t port;
    mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);
    IOConnectSetNotificationPort(c, 0, port, 0);
    struct { mach_msg_header_t h; char body[4096]; } msg;
    // drain anything already queued
    while (mach_msg(&msg.h, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof msg, port, 50, MACH_PORT_NULL) == MACH_MSG_SUCCESS) {}
    double sum = 0, max = 0;
    for (int i = 0; i < n; i++) {
        int delta = (i & 1) ? -1 : 1;
        CFNumberRef v = CFNumberCreate(NULL, kCFNumberIntType, &delta);
        CFStringRef k = CFSTR("MAMetaVirtualRotateDataKnob");
        CFDictionaryRef d = CFDictionaryCreate(NULL, (const void **)&k, (const void **)&v, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        uint64_t t0 = mach_absolute_time();
        IORegistryEntrySetCFProperties(s, d);
        uint64_t t1 = mach_absolute_time();
        kern_return_t r = mach_msg(&msg.h, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof msg, port, 3000, MACH_PORT_NULL);
        uint64_t t2 = mach_absolute_time();
        CFRelease(d); CFRelease(v);
        if (r != MACH_MSG_SUCCESS) { printf("#%d: no notification within 3 s (0x%x)\n", i + 1, r); continue; }
        double lat = ms(t2 - t0);
        sum += lat; if (lat > max) max = lat;
        printf("#%d %+d: setProperties %.2f ms, notification after %.2f ms\n", i + 1, delta, ms(t1 - t0), lat);
        while (mach_msg(&msg.h, MACH_RCV_MSG | MACH_RCV_TIMEOUT, 0, sizeof msg, port, 20, MACH_PORT_NULL) == MACH_MSG_SUCCESS) {}   // extra messages
        usleep(200000);
    }
    printf("mean %.2f ms, max %.2f ms\n", sum / n, max);
    IOServiceClose(c);
    return 0;
}
