// m_audio_b_FWPropertyDict::CopyProperty
// Entry: 0000a1e0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::CopyProperty(char const*) const */

long * __thiscall
m_audio_b_FWPropertyDict::CopyProperty(m_audio_b_FWPropertyDict *this,char *param_1)

{
  int iVar1;
  long *plVar2;
  long *local_20;
  
  local_20 = (long *)0x0;
  plVar2 = *(long **)(this + 0x60);
  if (plVar2 != (long *)0x0) {
    plVar2 = (long *)(**(code **)(*plVar2 + 0x208))(plVar2,param_1);
    if (plVar2 == (long *)0x0) {
      iVar1 = _strcmp(param_1,"MAMetaAllProperties");
      if (iVar1 == 0) {
        local_20 = (long *)CopyAllProperties(this);
        (**(code **)(*local_20 + 0x20))(local_20);
      }
    }
    else {
      (**(code **)(*plVar2 + 0x120))(plVar2,param_1,&local_20);
      if (local_20 != (long *)0x0) {
        (**(code **)(*local_20 + 0x20))();
      }
    }
  }
  return local_20;
}
