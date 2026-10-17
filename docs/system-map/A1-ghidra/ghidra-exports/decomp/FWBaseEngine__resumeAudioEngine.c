// m_audio_b_FWBaseEngine::resumeAudioEngine
// Entry: 00003dae
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::resumeAudioEngine() */

undefined8 __thiscall m_audio_b_FWBaseEngine::resumeAudioEngine(m_audio_b_FWBaseEngine *this)

{
  int iVar1;
  undefined8 uVar2;
  
  iVar1 = *(int *)(this + 0x234);
  *(int *)(this + 0x234) = iVar1 + -1;
  if (iVar1 + -1 == 0) {
    if (this[0x230] != (m_audio_b_FWBaseEngine)0x0) {
      iVar1 = (**(code **)(*(long *)this + 0xcf0))(this);
      if (iVar1 == 0) {
        this[0x230] = (m_audio_b_FWBaseEngine)0x0;
      }
    }
    uVar2 = (*(code *)PTR_resumeAudioEngine_00040808)(this);
    return uVar2;
  }
  return 0;
}
