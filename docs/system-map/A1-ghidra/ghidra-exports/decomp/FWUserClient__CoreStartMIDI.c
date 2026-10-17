// com_m_audio_FWUserClient::CoreStartMIDI
// Entry: 0000717c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::CoreStartMIDI(m_audio_b_FWBaseEngine*) */

undefined8 com_m_audio_FWUserClient::CoreStartMIDI(m_audio_b_FWBaseEngine *param_1)

{
  undefined8 uVar1;
  
  if ((0 < *(int *)(param_1 + 0x234)) && (param_1[0x230] == (m_audio_b_FWBaseEngine)0x0)) {
    param_1[0x230] = (m_audio_b_FWBaseEngine)0x1;
    return 0;
  }
                    /* WARNING: Could not recover jumptable at 0x000071a1. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar1 = (**(code **)(*(long *)param_1 + 0xcf0))();
  return uVar1;
}
