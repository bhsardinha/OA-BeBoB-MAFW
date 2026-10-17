// m_audio_b_FWAudioDevice::copyProperty
// Entry: 00002a34
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWAudioDevice::copyProperty(char const*) const */

void __thiscall m_audio_b_FWAudioDevice::copyProperty(m_audio_b_FWAudioDevice *this,char *param_1)

{
  long lVar1;
  
  lVar1 = HandleDigiHandshakeProperty(param_1);
  if ((lVar1 == 0) &&
     (*(m_audio_b_FWPropertyDict **)(this + 0xf8) != (m_audio_b_FWPropertyDict *)0x0)) {
    lVar1 = m_audio_b_FWPropertyDict::CopyProperty
                      (*(m_audio_b_FWPropertyDict **)(this + 0xf8),param_1);
  }
  if (lVar1 != 0) {
    return;
  }
  (*(code *)PTR_copyProperty_00047578)(this,param_1);
  return;
}
