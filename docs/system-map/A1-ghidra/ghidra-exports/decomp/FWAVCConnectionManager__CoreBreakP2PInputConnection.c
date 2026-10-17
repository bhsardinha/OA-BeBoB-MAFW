// m_audio_b_FWAVCConnectionManager::CoreBreakP2PInputConnection
// Entry: 00023f6e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::CoreBreakP2PInputConnection(m_audio_b_FWAVCConnectionManager*,
   unsigned int) */

void m_audio_b_FWAVCConnectionManager::CoreBreakP2PInputConnection
               (m_audio_b_FWAVCConnectionManager *param_1,uint param_2)

{
                    /* WARNING: Could not recover jumptable at 0x00023f84. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)param_1 + 0x180))(param_1,param_2 * 4 + -0xffff67c);
  return;
}
