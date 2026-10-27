#!/usr/sbin/dtrace -qs
/* slwakewatch.d -- sleep/wake + interface power-loss capture on the SL 10.6 rig (official v1.9 driver).
   Same probes as capture 73 (IOLog format strings, IOSleep) plus IOService start/free of the driver classes is NOT probed
   (no symbols); the object lifecycle is read from ioreg snapshots taken before/after by the driving script.
   usage: sudo dtrace -qs slwakewatch.d -o out.txt */
fbt:mach_kernel:IOLog:entry
{
    printf("%d IOLOG %s\n", walltimestamp / 1000000, stringof(arg0));
}
fbt:mach_kernel:IOSleep:entry
/arg0 != 14/
{
    printf("%d SLEEP ms=%d\n", walltimestamp / 1000000, arg0);
}
