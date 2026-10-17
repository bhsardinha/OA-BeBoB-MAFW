// m_audio_b_FWBaseEngine::performFormatChange
// Entry: 00003e54
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::performFormatChange(IOAudioStream*, _IOAudioStreamFormat const*,
   _IOAudioSampleRate const*) */

void __thiscall
m_audio_b_FWBaseEngine::performFormatChange
          (m_audio_b_FWBaseEngine *this,IOAudioStream *param_1,_IOAudioStreamFormat *param_2,
          _IOAudioSampleRate *param_3)

{
  long *plVar1;
  
  plVar1 = (long *)(**(code **)(*(long *)this + 0x688))(this);
  (**(code **)(*plVar1 + 0x1a0))(plVar1,ActionPerformFormatChange,this,param_1,param_2,param_3,0);
  return;
}
