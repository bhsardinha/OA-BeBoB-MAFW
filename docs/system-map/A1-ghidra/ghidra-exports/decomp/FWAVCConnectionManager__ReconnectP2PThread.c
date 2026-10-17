// m_audio_b_FWAVCConnectionManager::ReconnectP2PThread
// Entry: 00023ff4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::ReconnectP2PThread() */

void __thiscall
m_audio_b_FWAVCConnectionManager::ReconnectP2PThread(m_audio_b_FWAVCConnectionManager *this)

{
  (**(code **)(*(long *)this + 0x168))(this);
  (**(code **)(**(long **)(this + 0x18) + 0x28))();
                    /* WARNING: Could not recover jumptable at 0x00024023. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)(this + 0x10) + 0x28))();
  return;
}
