// m_audio_b_FWBaseEngine::ReplicateFormatForEachSampleRate
// Entry: 00003a9a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::ReplicateFormatForEachSampleRate(IOAudioStream*, _IOAudioStreamFormat*,
   _IOAudioSampleRate const*) */

void __thiscall
m_audio_b_FWBaseEngine::ReplicateFormatForEachSampleRate
          (m_audio_b_FWBaseEngine *this,IOAudioStream *param_1,_IOAudioStreamFormat *param_2,
          _IOAudioSampleRate *param_3)

{
  _IOAudioSampleRate *p_Var1;
  int iVar2;
  
  iVar2 = *(int *)param_3;
  while (iVar2 != 0) {
    (**(code **)(*(long *)param_1 + 0xa58))(param_1,param_2,param_3,param_3,0,0);
    p_Var1 = param_3 + 8;
    param_3 = param_3 + 8;
    iVar2 = *(int *)p_Var1;
  }
  return;
}
