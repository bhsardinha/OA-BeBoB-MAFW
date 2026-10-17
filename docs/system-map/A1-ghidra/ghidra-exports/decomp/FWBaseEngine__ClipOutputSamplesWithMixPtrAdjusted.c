// m_audio_b_FWBaseEngine::ClipOutputSamplesWithMixPtrAdjusted
// Entry: 000040aa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-d2b)

/* m_audio_b_FWBaseEngine::ClipOutputSamplesWithMixPtrAdjusted(void const*, void*, unsigned int,
   unsigned int, _IOAudioStreamFormat const*, IOAudioStream*) */

undefined8
m_audio_b_FWBaseEngine::ClipOutputSamplesWithMixPtrAdjusted
          (void *param_1,void *param_2,uint param_3,uint param_4,_IOAudioStreamFormat *param_5,
          IOAudioStream *param_6)

{
  if (*(int *)(*(long *)((long)param_1 + 0x218) + 0xb4) == 0x63616333) {
    if ((*(char *)((long)param_1 + 0x2d4) != '\0') && ((uint)param_5 < 0x601)) {
      (**(code **)(*(long *)param_1 + 0xca8))();
    }
  }
  else {
    (**(code **)(*(long *)param_1 + 0xca0))();
  }
  return 0;
}
