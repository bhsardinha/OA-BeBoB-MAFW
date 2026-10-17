// _CELogString
// Entry: 000113f4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _CELogString(undefined8 param_1)

{
  long *plVar1;
  undefined8 extraout_RDX;
  
  plVar1 = (long *)CEDynamicLookup();
  if (plVar1 == (long *)0x0) {
    _IOLog("%s",param_1);
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00011438. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0x858))(plVar1,param_1,extraout_RDX,*(code **)(*plVar1 + 0x858));
  return;
}
