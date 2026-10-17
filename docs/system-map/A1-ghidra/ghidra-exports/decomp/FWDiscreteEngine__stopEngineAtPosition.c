// m_audio_b_FWDiscreteEngine::stopEngineAtPosition
// Entry: 00018786
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::stopEngineAtPosition(IOAudioEnginePosition*) */

void __thiscall
m_audio_b_FWDiscreteEngine::stopEngineAtPosition
          (m_audio_b_FWDiscreteEngine *this,IOAudioEnginePosition *param_1)

{
  undefined4 uVar1;
  
  if (param_1 != (IOAudioEnginePosition *)0x0) {
    uVar1 = *(undefined4 *)(param_1 + 4);
    *(undefined4 *)(this + 0x9c) = *(undefined4 *)param_1;
    *(undefined4 *)(this + 0xa0) = uVar1;
    return;
  }
  *(undefined4 *)(this + 0xa0) = 0;
  *(undefined4 *)(this + 0x9c) = 0;
  return;
}
