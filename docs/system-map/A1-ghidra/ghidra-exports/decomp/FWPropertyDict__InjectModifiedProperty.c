// m_audio_b_FWPropertyDict::InjectModifiedProperty
// Entry: 00009eb4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::InjectModifiedProperty(OSSymbol const*, OSObject*, bool) */

void __thiscall
m_audio_b_FWPropertyDict::InjectModifiedProperty
          (m_audio_b_FWPropertyDict *this,OSSymbol *param_1,OSObject *param_2,bool param_3)

{
  char cVar1;
  
  cVar1 = (**(code **)(**(long **)(this + 0x68) + 0x138))();
  if (cVar1 == '\0') {
    return;
  }
  ModifyProperty(this,param_1,param_2);
  CheckModifiedProperties(this,param_3);
  return;
}
