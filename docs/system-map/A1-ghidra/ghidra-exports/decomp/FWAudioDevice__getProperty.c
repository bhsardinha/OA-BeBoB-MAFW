// m_audio_b_FWAudioDevice::getProperty
// Entry: 000029c4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAudioDevice::getProperty(char const*) const */

long * __thiscall m_audio_b_FWAudioDevice::getProperty(m_audio_b_FWAudioDevice *this,char *param_1)

{
  long *plVar1;
  
  plVar1 = (long *)HandleDigiHandshakeProperty(param_1);
  if ((plVar1 == (long *)0x0) &&
     (*(m_audio_b_FWPropertyDict **)(this + 0xf8) != (m_audio_b_FWPropertyDict *)0x0)) {
    plVar1 = (long *)m_audio_b_FWPropertyDict::CopyProperty
                               (*(m_audio_b_FWPropertyDict **)(this + 0xf8),param_1);
  }
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x28))(plVar1);
    return plVar1;
  }
  plVar1 = (long *)(*(code *)PTR_getProperty_00047548)(this,param_1);
  return plVar1;
}
