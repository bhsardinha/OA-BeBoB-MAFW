// FWSettings1814Levels::SetInputPan
// Entry: 0000d128
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings1814Levels::SetInputPan(unsigned int, unsigned int) */

void __thiscall
FWSettings1814Levels::SetInputPan(FWSettings1814Levels *this,uint param_1,uint param_2)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 8),
                              (OSMetaClass *)&com_m_audio_FW1814Device::gMetaClass);
                    /* WARNING: Could not recover jumptable at 0x0000d15a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0xbc0))(plVar1,param_1,param_2,*(code **)(*plVar1 + 0xbc0));
  return;
}
