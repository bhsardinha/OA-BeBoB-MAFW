// m_audio_b_FWPairStream::mixOutputSamples
// Entry: 0001b59e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWPairStream::mixOutputSamples(void const*, void*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

void m_audio_b_FWPairStream::mixOutputSamples
               (void *param_1,void *param_2,uint param_3,uint param_4,_IOAudioStreamFormat *param_5,
               IOAudioStream *param_6)

{
                    /* WARNING: Could not recover jumptable at 0x0001b5b4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)((long)param_1 + 0x88) + 0xac0))();
  return;
}
