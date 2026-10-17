// m_audio_b_FWDiscreteEngine::clipOutputSamples
// Entry: 000188c6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::clipOutputSamples(void const*, void*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

void __thiscall
m_audio_b_FWDiscreteEngine::clipOutputSamples
          (m_audio_b_FWDiscreteEngine *this,void *param_1,void *param_2,uint param_3,uint param_4,
          _IOAudioStreamFormat *param_5,IOAudioStream *param_6)

{
  ClipSubEngineOutput(this,1,param_3,param_4,param_5,param_6);
  return;
}
