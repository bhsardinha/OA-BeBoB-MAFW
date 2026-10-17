// FWSettingsABSwitch::SetHeadphoneSecondSource
// Entry: 0001ddfe
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SetHeadphoneSecondSource(unsigned int) */

void FWSettingsABSwitch::SetHeadphoneSecondSource(uint param_1)

{
  code *UNRECOVERED_JUMPTABLE;
  undefined4 uVar1;
  undefined4 in_register_0000003c;
  long *plVar2;
  
  plVar2 = (long *)CONCAT44(in_register_0000003c,param_1);
  uVar1 = (**(code **)(*plVar2 + 0x28))(plVar2);
  UNRECOVERED_JUMPTABLE = (code *)**(undefined8 **)plVar2[5];
                    /* WARNING: Could not recover jumptable at 0x0001de2a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)((undefined8 *)plVar2[5],1,uVar1,UNRECOVERED_JUMPTABLE);
  return;
}
