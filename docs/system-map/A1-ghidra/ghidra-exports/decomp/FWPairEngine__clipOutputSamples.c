// m_audio_b_FWPairEngine::clipOutputSamples
// Entry: 0001af04
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWPairEngine::clipOutputSamples(void const*, void*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

void __thiscall
m_audio_b_FWPairEngine::clipOutputSamples
          (m_audio_b_FWPairEngine *this,void *param_1,void *param_2,uint param_3,uint param_4,
          _IOAudioStreamFormat *param_5,IOAudioStream *param_6)

{
  m_audio_b_FWDiscreteEngine::ClipSubEngineOutput
            (*(m_audio_b_FWDiscreteEngine **)(this + 0x118),(uint)(1L << ((byte)this[300] & 0x3f)),
             param_3,param_4,param_5,param_6);
  return;
}
