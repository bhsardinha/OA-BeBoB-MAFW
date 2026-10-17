// m_audio_b_FWAVCConnectionManager::Disconnect_oPCR
// Entry: 00023f3e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::Disconnect_oPCR(unsigned int) */

void m_audio_b_FWAVCConnectionManager::Disconnect_oPCR(uint param_1)

{
  undefined4 in_register_0000003c;
  
                    /* WARNING: Could not recover jumptable at 0x00023f4d. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)CONCAT44(in_register_0000003c,param_1) + 0x160))();
  return;
}
