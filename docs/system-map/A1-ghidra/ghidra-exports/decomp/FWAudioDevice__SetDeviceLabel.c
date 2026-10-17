// m_audio_b_FWAudioDevice::SetDeviceLabel
// Entry: 000028aa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e6)

/* m_audio_b_FWAudioDevice::SetDeviceLabel(char const*) */

undefined8 __thiscall
m_audio_b_FWAudioDevice::SetDeviceLabel(m_audio_b_FWAudioDevice *this,char *param_1)

{
  char *pcVar1;
  undefined8 uVar2;
  
  pcVar1 = operator_new__(0x100);
  if (pcVar1 == (char *)0x0) {
    uVar2 = 0xe00002bd;
  }
  else {
    uVar2 = (**(code **)(*(long *)this + 0xa50))(this);
    _snprintf(pcVar1,0x100,"%s (%s)",uVar2,param_1);
    if (*(char **)(this + 0xe8) != (char *)0x0) {
      m_audio_b_FWBaseEngine::SetLabel(*(char **)(this + 0xe8));
    }
    operator_delete(pcVar1);
    uVar2 = 0;
  }
  return uVar2;
}
