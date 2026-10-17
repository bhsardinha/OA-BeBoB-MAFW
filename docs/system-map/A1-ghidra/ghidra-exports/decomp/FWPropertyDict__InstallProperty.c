// m_audio_b_FWPropertyDict::InstallProperty
// Entry: 0000a060
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::InstallProperty(m_audio_b_FWBaseProp*) */

undefined8 __thiscall
m_audio_b_FWPropertyDict::InstallProperty
          (m_audio_b_FWPropertyDict *this,m_audio_b_FWBaseProp *param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x60);
  if (plVar1 == (long *)0x0) {
    plVar1 = (long *)OSDictionary::withCapacity(0x20);
    *(long **)(this + 0x60) = plVar1;
    if (plVar1 == (long *)0x0) {
      return 0xe00002bd;
    }
  }
  (**(code **)(*plVar1 + 0x1c0))(plVar1,*(undefined8 *)(param_1 + 0x18),param_1);
  if (param_1[0x28] != (m_audio_b_FWBaseProp)0x0) {
    plVar1 = *(long **)(this + 0x50);
    if (plVar1 == (long *)0x0) {
      plVar1 = (long *)OSSet::withCapacity(4);
      *(long **)(this + 0x50) = plVar1;
      if (plVar1 == (long *)0x0) {
        return 0;
      }
    }
    (**(code **)(*plVar1 + 0x1c0))(plVar1,*(undefined8 *)(param_1 + 0x18));
  }
  *(m_audio_b_FWPropertyDict **)(param_1 + 0x20) = this;
  return 0;
}
