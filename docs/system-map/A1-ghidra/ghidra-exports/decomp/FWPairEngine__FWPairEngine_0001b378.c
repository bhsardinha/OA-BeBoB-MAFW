// m_audio_b_FWPairEngine::m_audio_b_FWPairEngine
// Entry: 0001b378
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::m_audio_b_FWPairEngine(OSMetaClass const*) */

void __thiscall
m_audio_b_FWPairEngine::m_audio_b_FWPairEngine(m_audio_b_FWPairEngine *this,OSMetaClass *param_1)

{
  m_audio_b_FWTimeSlaveEngine::m_audio_b_FWTimeSlaveEngine
            ((m_audio_b_FWTimeSlaveEngine *)this,param_1);
  *(undefined ***)this = &PTR__m_audio_b_FWPairEngine_00044ac0;
  *(undefined8 *)(this + 0x148) = 0;
  *(undefined8 *)(this + 0x150) = 0;
  *(undefined4 *)(this + 0x158) = 0;
  *(undefined2 *)(this + 0x16c) = 0;
  *(undefined8 *)(this + 0x164) = 0;
  *(undefined8 *)(this + 0x15c) = 0;
  return;
}
