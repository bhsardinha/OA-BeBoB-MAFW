// ForceStopNotificationProc
// Entry: 00025766
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* ForceStopNotificationProc(void*, IOFWIsochChannel*, unsigned int) */

undefined8 ForceStopNotificationProc(void *param_1,IOFWIsochChannel *param_2,uint param_3)

{
  _IOLog("M-FW: channel %p force stop proc (reason:%X)\n");
  *(undefined1 *)((long)param_1 + 0x8d) = 0;
  return 0;
}
