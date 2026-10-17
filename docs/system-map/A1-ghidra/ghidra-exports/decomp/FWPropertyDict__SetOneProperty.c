// m_audio_b_FWPropertyDict::SetOneProperty
// Entry: 00009f74
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::SetOneProperty(OSSymbol const*, OSObject*) */

undefined8 __thiscall
m_audio_b_FWPropertyDict::SetOneProperty
          (m_audio_b_FWPropertyDict *this,OSSymbol *param_1,OSObject *param_2)

{
  char cVar1;
  long *plVar2;
  
  cVar1 = (**(code **)(**(long **)(this + 0x50) + 0x1e0))(*(long **)(this + 0x50),param_1);
  if (cVar1 == '\0') {
    ModifyProperty(this,param_1,param_2);
  }
  else {
    plVar2 = (long *)(**(code **)(**(long **)(this + 0x60) + 0x1f8))
                               (*(long **)(this + 0x60),param_1);
    (**(code **)(*plVar2 + 0x128))(plVar2,param_1,param_2);
  }
  return 0;
}
