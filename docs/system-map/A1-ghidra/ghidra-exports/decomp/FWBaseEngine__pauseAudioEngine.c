// m_audio_b_FWBaseEngine::pauseAudioEngine
// Entry: 00003d54
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::pauseAudioEngine() */

undefined8 __thiscall m_audio_b_FWBaseEngine::pauseAudioEngine(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  undefined8 uVar2;
  
  iVar1 = *(int *)(this + 0x234);
  *(int *)(this + 0x234) = iVar1 + 1;
  if (iVar1 + 1 < 2) {
    if (this[0x298] != (m_audio_b_FWBaseEngine)0x0) {
      this[0x230] = this[0x298];
      (**(code **)(*(long *)this + 0xcf8))(this);
    }
    uVar2 = (*(code *)PTR_pauseAudioEngine_00040800)(this);
    return uVar2;
  }
  return 0;
}
