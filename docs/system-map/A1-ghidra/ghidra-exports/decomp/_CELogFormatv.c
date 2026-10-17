// _CELogFormatv
// Entry: 000113bf
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _CELogFormatv(undefined8 param_1,undefined8 param_2)

{
  long *plVar1;
  
  plVar1 = (long *)CEDynamicLookup();
  if (plVar1 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000113f2. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0x868))(plVar1,param_1,param_2,*(code **)(*plVar1 + 0x868));
  return;
}
