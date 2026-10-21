// FWRev1AudioDevice::stop
// Entry: 000136c8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* WARNING: Switch with 1 destination removed at 0x000136d4 */
/* FWRev1AudioDevice::stop(IOService*) */

void FWRev1AudioDevice::stop(IOService *param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(param_1 + 0x268);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x600))(plVar1,1);
    *(undefined8 *)(param_1 + 0x268) = 0;
  }
  (*(code *)PTR_stop_00031738)(param_1);
  return;
}
