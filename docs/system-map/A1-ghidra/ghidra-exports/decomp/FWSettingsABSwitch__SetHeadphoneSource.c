// FWSettingsABSwitch::SetHeadphoneSource
// Entry: 0001de2c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SetHeadphoneSource(unsigned int) */

void __thiscall FWSettingsABSwitch::SetHeadphoneSource(FWSettingsABSwitch *this,uint param_1)

{
  code *UNRECOVERED_JUMPTABLE;
  undefined4 uVar1;
  
  *(undefined4 *)(this + 0x104) = 1;
  UpdateLEDs(this);
  uVar1 = (**(code **)(*(long *)this + 0x28))(this,param_1);
  UNRECOVERED_JUMPTABLE = *(code **)**(undefined8 **)(this + 0x28);
                    /* WARNING: Could not recover jumptable at 0x0001de68. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(*(undefined8 **)(this + 0x28),0,uVar1,UNRECOVERED_JUMPTABLE);
  return;
}
