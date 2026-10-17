// FWSettingsRotaryControl::VirtualRotateKnob
// Entry: 000213f8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsRotaryControl::VirtualRotateKnob(OSObject*) */

undefined8 __thiscall
FWSettingsRotaryControl::VirtualRotateKnob(FWSettingsRotaryControl *this,OSObject *param_1)

{
  int iVar1;
  long *plVar2;
  undefined8 uVar3;
  
  plVar2 = (long *)OSMetaClassBase::safeMetaCast
                             ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030080);
  if (plVar2 == (long *)0x0) {
    uVar3 = 0xe00002bc;
  }
  else {
    iVar1 = (**(code **)(*plVar2 + 0x148))(plVar2);
    UserRotatedHardwareKnob(this,iVar1);
    uVar3 = 0;
  }
  return uVar3;
}
