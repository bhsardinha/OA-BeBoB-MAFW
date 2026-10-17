// m_audio_b_FWPropertyDict::ModifyProperty
// Entry: 00009e28
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::ModifyProperty(OSSymbol const*, OSObject*) */

void __thiscall
m_audio_b_FWPropertyDict::ModifyProperty
          (m_audio_b_FWPropertyDict *this,OSSymbol *param_1,OSObject *param_2)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x58);
  if (plVar1 == (long *)0x0) {
    plVar1 = (long *)OSDictionary::withCapacity(10);
    *(long **)(this + 0x58) = plVar1;
  }
  if (plVar1 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00009e82. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0x1c0))(plVar1,param_1,param_2,*(code **)(*plVar1 + 0x1c0));
  return;
}
