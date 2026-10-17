// m_audio_b_FWPropertyDict::ModifyProperty
// Entry: 00009f06
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::ModifyProperty(char const*, OSObject*) */

void __thiscall
m_audio_b_FWPropertyDict::ModifyProperty
          (m_audio_b_FWPropertyDict *this,char *param_1,OSObject *param_2)

{
  OSSymbol *pOVar1;
  
  pOVar1 = (OSSymbol *)OSSymbol::withCString(param_1);
  if (pOVar1 == (OSSymbol *)0x0) {
    return;
  }
  ModifyProperty(this,pOVar1,param_2);
                    /* WARNING: Could not recover jumptable at 0x00009f40. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)pOVar1 + 0x28))(pOVar1);
  return;
}
