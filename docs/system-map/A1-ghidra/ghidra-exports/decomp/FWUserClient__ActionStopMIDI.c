// com_m_audio_FWUserClient::ActionStopMIDI
// Entry: 0000749c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::ActionStopMIDI(m_audio_b_FWBaseEngine*) */

undefined8 com_m_audio_FWUserClient::ActionStopMIDI(m_audio_b_FWBaseEngine *param_1)

{
  CoreStopMIDI(param_1);
  return 0;
}
