// m_audio_b_FWStream::mixOutputSamples
// Entry: 00010b38
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWStream::mixOutputSamples(void const*, void*, unsigned int, unsigned int,
   _IOAudioStreamFormat const*, IOAudioStream*) */

void m_audio_b_FWStream::mixOutputSamples
               (void *param_1,void *param_2,uint param_3,uint param_4,_IOAudioStreamFormat *param_5,
               IOAudioStream *param_6)

{
  long *plVar1;
  
  plVar1 = *(long **)((long)param_1 + 0x88);
  if (1 < *(uint *)(plVar1 + 0x25)) {
                    /* WARNING: Could not recover jumptable at 0x00010b5a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*plVar1 + 0xac0))(plVar1);
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00010b65. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300b8 + 0x898))();
  return;
}
