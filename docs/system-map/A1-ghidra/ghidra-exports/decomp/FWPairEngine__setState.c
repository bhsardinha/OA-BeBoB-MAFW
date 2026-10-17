// m_audio_b_FWPairEngine::setState
// Entry: 0001ad40
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::setState(_IOAudioEngineState) */

undefined4 __thiscall
m_audio_b_FWPairEngine::setState(m_audio_b_FWPairEngine *this,undefined4 param_2)

{
  undefined4 uVar1;
  
  uVar1 = *(undefined4 *)(this + 0xa8);
  *(undefined4 *)(this + 0xa8) = param_2;
  (**(code **)(*(long *)this + 0x280))(this,"IOAudioEngineState",param_2,0x20);
  return uVar1;
}
