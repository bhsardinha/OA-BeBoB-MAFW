// m_audio_b_FWDiscreteEngine::timerCallback
// Entry: 00018e2e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::timerCallback(OSObject*, IOAudioDevice*) */

void m_audio_b_FWDiscreteEngine::timerCallback(OSObject *param_1,IOAudioDevice *param_2)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             ((OSMetaClassBase *)param_2,*(OSMetaClass **)PTR_metaClass_00030028);
  if (plVar1 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00018e55. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0xa98))(plVar1);
  return;
}
