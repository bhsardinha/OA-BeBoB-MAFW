// _CELogChar
// Entry: 000112d0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2)

void _CELogChar(char param_1)

{
  long *plVar1;
  undefined8 extraout_RDX;
  
  plVar1 = (long *)CEDynamicLookup();
  if (plVar1 == (long *)0x0) {
    _IOLog("%c",(int)param_1);
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00011313. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0x850))(plVar1,(int)param_1,extraout_RDX,*(code **)(*plVar1 + 0x850));
  return;
}
