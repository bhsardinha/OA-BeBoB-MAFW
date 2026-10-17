// com_m_audio_FWUserClient::SetInitializedProperty
// Entry: 000071f6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::SetInitializedProperty() */

undefined8 __thiscall
com_m_audio_FWUserClient::SetInitializedProperty(com_m_audio_FWUserClient *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x290);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x278))(plVar1,"MADeviceInitialized",1);
  }
  return 0;
}
