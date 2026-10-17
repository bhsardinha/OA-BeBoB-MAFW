// m_audio_b_FWDiscreteEngine::StaticCreateAggregateAudioStreams
// Entry: 00018d36
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::StaticCreateAggregateAudioStreams(OSObject*, void*, void*, void*,
   void*) */

undefined8
m_audio_b_FWDiscreteEngine::StaticCreateAggregateAudioStreams
          (OSObject *param_1,void *param_2,void *param_3,void *param_4,void *param_5)

{
  long *plVar1;
  undefined8 uVar2;
  undefined1 auVar3 [16];
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             ((OSMetaClassBase *)param_1,(OSMetaClass *)&gMetaClass);
  auVar3 = OSMetaClassBase::safeMetaCast
                     (param_2,(OSMetaClass *)m_audio_b_FWAggregateEngine::metaClass);
  if ((plVar1 != (long *)0x0) && (auVar3._0_8_ != 0)) {
                    /* WARNING: Could not recover jumptable at 0x00018d89. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    uVar2 = (**(code **)(*plVar1 + 0xc50))
                      (plVar1,auVar3._0_8_,auVar3._8_8_,*(code **)(*plVar1 + 0xc50));
    return uVar2;
  }
  return 0xe00002c2;
}
