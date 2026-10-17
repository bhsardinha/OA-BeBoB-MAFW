// m_audio_b_FWAVCConnectionManager::Connect
// Entry: 000248ce
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::Connect(bool, unsigned int, unsigned int) */

void __thiscall
m_audio_b_FWAVCConnectionManager::Connect
          (m_audio_b_FWAVCConnectionManager *this,bool param_1,uint param_2,uint param_3)

{
  if (param_1) {
    Connect_iPCR(this,param_3,param_2);
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000248ec. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0x158))(this,param_3);
  return;
}
