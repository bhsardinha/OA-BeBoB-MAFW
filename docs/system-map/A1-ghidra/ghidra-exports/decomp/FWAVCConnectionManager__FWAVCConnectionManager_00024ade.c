// m_audio_b_FWAVCConnectionManager::m_audio_b_FWAVCConnectionManager
// Entry: 00024ade
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCConnectionManager::m_audio_b_FWAVCConnectionManager(OSMetaClass const*) */

void __thiscall
m_audio_b_FWAVCConnectionManager::m_audio_b_FWAVCConnectionManager
          (m_audio_b_FWAVCConnectionManager *this,OSMetaClass *param_1)

{
  m_audio_b_FWConnectionManager::m_audio_b_FWConnectionManager
            ((m_audio_b_FWConnectionManager *)this,param_1);
  *(undefined ***)this = &PTR__m_audio_b_FWAVCConnectionManager_00049e10;
  *(undefined4 *)(this + 0x28) = 3;
  *(m_audio_b_FWAVCConnectionManager **)(this + 0x20) = this + 0x2c;
  _bzero(this + 0x2c,0xc);
  return;
}
