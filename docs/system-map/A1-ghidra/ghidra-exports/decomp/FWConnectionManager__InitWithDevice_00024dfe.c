// m_audio_b_FWConnectionManager::InitWithDevice
// Entry: 00024dfe
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWConnectionManager::InitWithDevice(m_audio_b_FWAudioDevice*) */

undefined8 __thiscall
m_audio_b_FWConnectionManager::InitWithDevice
          (m_audio_b_FWConnectionManager *this,m_audio_b_FWAudioDevice *param_1)

{
  *(m_audio_b_FWAudioDevice **)(this + 0x10) = param_1;
  return 0;
}
