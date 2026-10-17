// m_audio_b_FWBaseEngine::performAudioEngineStop
// Entry: 0000543c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::performAudioEngineStop() */

undefined8 __thiscall m_audio_b_FWBaseEngine::performAudioEngineStop(m_audio_b_FWBaseEngine *this)

{
  long *plVar1;
  uint uVar2;
  
  m_audio_b_FWDiscreteEngine::DeactivateSubEngine((m_audio_b_FWDiscreteEngine *)this,1);
  plVar1 = *(long **)(this + 0x138);
  if ((plVar1 != (long *)0x0) && (plVar1[0x21] != 0)) {
    uVar2 = (**(code **)(*plVar1 + 0xaf0))(plVar1);
    _bzero((void *)plVar1[0x21],(ulong)uVar2 << 2);
    uVar2 = (**(code **)(*plVar1 + 0xaf0))(plVar1);
    _bzero((void *)plVar1[0x22],(ulong)uVar2 << 2);
  }
  return 0;
}
