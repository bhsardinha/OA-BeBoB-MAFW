// com_m_audio_FWUserClient::CoreStopMIDI
// Entry: 0000714a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::CoreStopMIDI(m_audio_b_FWBaseEngine*) */

void com_m_audio_FWUserClient::CoreStopMIDI(m_audio_b_FWBaseEngine *param_1)

{
  if (param_1[0x298] == (m_audio_b_FWBaseEngine)0x0) {
    if ((0 < *(int *)(param_1 + 0x234)) && (param_1[0x230] != (m_audio_b_FWBaseEngine)0x0)) {
      param_1[0x230] = (m_audio_b_FWBaseEngine)0x0;
    }
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00007176. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)param_1 + 0xd00))();
  return;
}
