// m_audio_b_FWBaseEngine::TellClientsSampleRateChanged
// Entry: 00005422
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::TellClientsSampleRateChanged(unsigned int) */

void __thiscall
m_audio_b_FWBaseEngine::TellClientsSampleRateChanged(m_audio_b_FWBaseEngine *this,uint param_1)

{
  m_audio_b_FWAudioDevice::SetStaticPropertyAndNotifyUserClients
            (*(m_audio_b_FWAudioDevice **)(this + 0x138),"MASampleRate",param_1);
  return;
}
