// m_audio_b_FWAVCConnectionManager::CoreMakeP2POutputConnection
// Entry: 00023f86
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::CoreMakeP2POutputConnection(m_audio_b_FWAVCConnectionManager*,
   unsigned int, unsigned int) */

void m_audio_b_FWAVCConnectionManager::CoreMakeP2POutputConnection
               (m_audio_b_FWAVCConnectionManager *param_1,uint param_2,uint param_3)

{
  code *UNRECOVERED_JUMPTABLE;
  undefined4 uVar1;
  
  UNRECOVERED_JUMPTABLE = *(code **)(*(long *)param_1 + 0x178);
  uVar1 = (**(code **)(**(long **)(*(long *)(param_1 + 0x10) + 0xe0) + 0x850))();
                    /* WARNING: Could not recover jumptable at 0x00023fda. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(param_1,param_2 * 4 + -0xffff6fc,param_3,uVar1);
  return;
}
