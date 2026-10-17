// com_m_audio_FWEngine2::SetExternalSyncEnabled
// Entry: 00011bc0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::SetExternalSyncEnabled(bool) */

void __thiscall
com_m_audio_FWEngine2::SetExternalSyncEnabled(com_m_audio_FWEngine2 *this,bool param_1)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_SetExternalSyncEnabled_000336f8)(this);
  if (iVar1 == 0) {
    *(com_m_audio_FWEngine2 *)(*(long *)(this + 0x350) + 0x618) = this[0x2d0];
  }
  return;
}
