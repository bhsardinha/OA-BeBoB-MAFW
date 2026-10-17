// m_audio_b_FW32BitNumberProp::CallSetter
// Entry: 0000f806
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FW32BitNumberProp::CallSetter(OSSymbol const*, OSObject*) */

undefined8 __thiscall
m_audio_b_FW32BitNumberProp::CallSetter
          (m_audio_b_FW32BitNumberProp *this,OSSymbol *param_1,OSObject *param_2)

{
  undefined8 *puVar1;
  undefined4 uVar2;
  long *plVar3;
  undefined8 uVar4;
  
  plVar3 = (long *)OSMetaClassBase::safeMetaCast
                             ((OSMetaClassBase *)param_2,*(OSMetaClass **)PTR_metaClass_00030080);
  if (plVar3 == (long *)0x0) {
    uVar4 = 0xe00002c2;
  }
  else {
    uVar2 = (**(code **)(*plVar3 + 0x148))(plVar3);
    puVar1 = *(undefined8 **)(this + 0x38);
    if (puVar1 == (undefined8 *)0x0) {
      uVar4 = 0;
    }
    else {
      uVar4 = (**(code **)*puVar1)(puVar1,uVar2);
      if ((int)uVar4 != 0) {
        return uVar4;
      }
    }
    *(undefined4 *)(this + 0x2c) = uVar2;
  }
  return uVar4;
}
