// com_m_audio_FWEngine2::StopIsochStreams
// Entry: 00011ed2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::StopIsochStreams() */

void __thiscall com_m_audio_FWEngine2::StopIsochStreams(com_m_audio_FWEngine2 *this)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 0x138),
                              (OSMetaClass *)m_audio_b_FWAVCAudioDevice::metaClass);
  if (plVar1 != (long *)0x0) {
    (*(code *)PTR_StopIsochStreams_00033630)(this);
    if (*(int *)(this + 0x22c) == 0) {
                    /* WARNING: Could not recover jumptable at 0x00011f22. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*plVar1 + 0xb80))();
      return;
    }
  }
  return;
}
