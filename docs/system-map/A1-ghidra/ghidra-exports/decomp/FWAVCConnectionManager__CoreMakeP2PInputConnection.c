// m_audio_b_FWAVCConnectionManager::CoreMakeP2PInputConnection
// Entry: 00023f50
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::CoreMakeP2PInputConnection(m_audio_b_FWAVCConnectionManager*,
   unsigned int, unsigned int) */

void m_audio_b_FWAVCConnectionManager::CoreMakeP2PInputConnection
               (m_audio_b_FWAVCConnectionManager *param_1,uint param_2,uint param_3)

{
                    /* WARNING: Could not recover jumptable at 0x00023f6b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)param_1 + 0x178))(param_1,param_2 * 4 + -0xffff67c,param_3,0x80000000);
  return;
}
