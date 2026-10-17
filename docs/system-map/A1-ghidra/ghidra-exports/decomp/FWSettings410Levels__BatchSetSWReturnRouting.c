// FWSettings410Levels::BatchSetSWReturnRouting
// Entry: 0001449a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettings410Levels::BatchSetSWReturnRouting() */

void __thiscall FWSettings410Levels::BatchSetSWReturnRouting(FWSettings410Levels *this)

{
  long *plVar1;
  
  plVar1 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 8),
                              (OSMetaClass *)&com_m_audio_FW410Device::gMetaClass);
                    /* WARNING: Could not recover jumptable at 0x000144b5. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar1 + 0xbe0))(plVar1);
  return;
}
