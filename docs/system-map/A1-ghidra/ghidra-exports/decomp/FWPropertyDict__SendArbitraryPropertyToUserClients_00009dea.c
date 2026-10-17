// m_audio_b_FWPropertyDict::SendArbitraryPropertyToUserClients
// Entry: 00009dea
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWPropertyDict::SendArbitraryPropertyToUserClients(char const*, OSObject*) */

void __thiscall
m_audio_b_FWPropertyDict::SendArbitraryPropertyToUserClients
          (m_audio_b_FWPropertyDict *this,char *param_1,OSObject *param_2)

{
  OSSymbol *pOVar1;
  
  pOVar1 = (OSSymbol *)OSSymbol::withCString(param_1);
  if (pOVar1 == (OSSymbol *)0x0) {
    return;
  }
  SendArbitraryPropertyToUserClients(this,pOVar1,param_2);
                    /* WARNING: Could not recover jumptable at 0x00009e24. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)pOVar1 + 0x28))(pOVar1);
  return;
}
