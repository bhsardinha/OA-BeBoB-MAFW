// m_audio_b_FWAVCConnectionManager::Disconnect
// Entry: 0002479a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::Disconnect(bool, unsigned int, unsigned int) */

void __thiscall
m_audio_b_FWAVCConnectionManager::Disconnect
          (m_audio_b_FWAVCConnectionManager *this,bool param_1,uint param_2,uint param_3)

{
  if (param_1) {
    Disconnect_iPCR(this,param_3);
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x000247b8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0x160))(this,param_3);
  return;
}
