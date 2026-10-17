// FWSettings::Init
// Entry: 0001dd2e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings::Init(m_audio_b_FWAudioDevice*) */

undefined8 __thiscall FWSettings::Init(FWSettings *this,m_audio_b_FWAudioDevice *param_1)

{
  *(m_audio_b_FWAudioDevice **)(this + 8) = param_1;
  return 0;
}
