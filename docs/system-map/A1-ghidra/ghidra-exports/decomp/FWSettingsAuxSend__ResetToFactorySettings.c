// FWSettingsAuxSend::ResetToFactorySettings
// Entry: 00020b9c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::ResetToFactorySettings() */

undefined8 __thiscall FWSettingsAuxSend::ResetToFactorySettings(FWSettingsAuxSend *this)

{
  long *plVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  
  plVar1 = *(long **)(this + 0x10);
  if (plVar1 != (long *)0x0) {
    iVar2 = (**(code **)(*plVar1 + 0x10))(plVar1);
    if (iVar2 != 0) {
      iVar4 = 0;
      do {
        puVar3 = (undefined4 *)(**(code **)(*plVar1 + 0x18))(plVar1);
        *puVar3 = 0xffffff01;
        iVar4 = iVar4 + 1;
      } while (iVar2 != iVar4);
    }
  }
  m_audio_b_FWBaseArrayProp::Set((m_audio_b_FWBaseArrayProp *)(this + 0x260),0,0xffffff01);
  m_audio_b_FWBaseArrayProp::Set((m_audio_b_FWBaseArrayProp *)(this + 0x260),1,0xff);
  m_audio_b_FWBaseArrayProp::Fill((m_audio_b_FWBaseArrayProp *)(this + 0x200),0);
  return 0;
}
