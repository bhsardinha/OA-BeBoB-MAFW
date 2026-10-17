// FWSettings1814Levels::SetInputRouting
// Entry: 0000d15c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings1814Levels::SetInputRouting(unsigned int, unsigned int) */

void __thiscall
FWSettings1814Levels::SetInputRouting(FWSettings1814Levels *this,uint param_1,uint param_2)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 8),
                              (OSMetaClass *)&com_m_audio_FW1814Device::gMetaClass);
                    /* WARNING: Could not recover jumptable at 0x0000d18e. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 3000))(plVar1,param_1,param_2,*(code **)(*plVar1 + 3000));
  return;
}
