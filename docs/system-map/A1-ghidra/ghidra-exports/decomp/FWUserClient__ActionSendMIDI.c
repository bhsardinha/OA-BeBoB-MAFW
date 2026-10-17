// com_m_audio_FWUserClient::ActionSendMIDI
// Entry: 00007574
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::ActionSendMIDI(m_audio_b_FWBaseEngine*, MAMIDIPacket*, unsigned int*)
    */

void com_m_audio_FWUserClient::ActionSendMIDI
               (m_audio_b_FWBaseEngine *param_1,MAMIDIPacket *param_2,uint *param_3)

{
  m_audio_b_FWBaseEngine::SendMIDI((MAMIDIPacket *)param_1,(uint *)param_2);
  return;
}
