// m_audio_b_FWAVCConnectionManager::Connect_oPCR
// Entry: 00023f2c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::Connect_oPCR(unsigned int, unsigned int) */

void m_audio_b_FWAVCConnectionManager::Connect_oPCR(uint param_1,uint param_2)

{
  undefined4 in_register_0000003c;
  
                    /* WARNING: Could not recover jumptable at 0x00023f3b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)CONCAT44(in_register_0000003c,param_1) + 0x158))();
  return;
}
