// FWSettings410Levels::SetInputRouting
// Entry: 00014432
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings410Levels::SetInputRouting(unsigned int, unsigned int) */

void __thiscall
FWSettings410Levels::SetInputRouting(FWSettings410Levels *this,uint param_1,uint param_2)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 8),
                              (OSMetaClass *)&com_m_audio_FW410Device::gMetaClass);
                    /* WARNING: Could not recover jumptable at 0x00014464. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0xbc8))(plVar1,param_1,param_2,*(code **)(*plVar1 + 0xbc8));
  return;
}
