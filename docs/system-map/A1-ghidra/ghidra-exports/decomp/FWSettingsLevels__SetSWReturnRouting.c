// FWSettingsLevels::SetSWReturnRouting
// Entry: 000218a6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SetSWReturnRouting(unsigned int, unsigned int) */

void __thiscall
FWSettingsLevels::SetSWReturnRouting(FWSettingsLevels *this,uint param_1,uint param_2)

{
  code *UNRECOVERED_JUMPTABLE;
  uint uVar1;
  
  UNRECOVERED_JUMPTABLE = *(code **)(*(long *)this + 0x60);
  uVar1 = (**(code **)(**(long **)(this + 0x8b0) + 0x10))();
                    /* WARNING: Could not recover jumptable at 0x000218f3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(this,1,uVar1 >> 1,param_1,param_2);
  return;
}
