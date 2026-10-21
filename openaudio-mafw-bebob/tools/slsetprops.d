#!/usr/sbin/dtrace -qs
/* slsetprops.d -- record every IORegistryEntrySetCFProperties call made by the official panel (SL 10.6 rig, fbt on mach_kernel).
   Same probe as captures 68-70/90; prints wall-clock time so each record can be matched to the action log.
   usage: sudo dtrace -qs slsetprops.d -o out.txt */
fbt:mach_kernel:is_io_registry_entry_set_properties:entry
/execname == "Panel"/
{
    printf("[wall=%d pid=%d exec=%s size=%d]\n", walltimestamp / 1000000, pid, execname, arg2);
    tracemem(arg1, 768);
    printf("\n");
}
