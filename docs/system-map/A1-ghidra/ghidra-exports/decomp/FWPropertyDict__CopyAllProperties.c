// m_audio_b_FWPropertyDict::CopyAllProperties
// Entry: 00009fe0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::CopyAllProperties() const */

OSDictionary * __thiscall
m_audio_b_FWPropertyDict::CopyAllProperties(m_audio_b_FWPropertyDict *this)

{
  uint uVar1;
  OSDictionary *pOVar2;
  
  uVar1 = (**(code **)(**(long **)(this + 0x60) + 0x130))();
  pOVar2 = (OSDictionary *)OSDictionary::withCapacity(uVar1);
  if (pOVar2 != (OSDictionary *)0x0) {
    CopyAllPropertiesToDictionary(this,pOVar2);
  }
  return pOVar2;
}
