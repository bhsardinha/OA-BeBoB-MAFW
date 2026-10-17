// FWSettingsLevels::SetInputPan
// Entry: 00021878
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SetInputPan(unsigned int, unsigned int) */

void __thiscall FWSettingsLevels::SetInputPan(FWSettingsLevels *this,uint param_1,uint param_2)

{
                    /* WARNING: Could not recover jumptable at 0x0002188d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0x48))(this,param_1,3,-param_2);
  return;
}
