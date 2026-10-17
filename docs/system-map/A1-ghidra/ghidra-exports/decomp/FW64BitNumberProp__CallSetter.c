// m_audio_b_FW64BitNumberProp::CallSetter
// Entry: 0000f7ac
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FW64BitNumberProp::CallSetter(OSSymbol const*, OSObject*) */

undefined8 __thiscall
m_audio_b_FW64BitNumberProp::CallSetter
          (m_audio_b_FW64BitNumberProp *this,OSSymbol *param_1,OSObject *param_2)

{
  undefined8 *puVar1;
  long *plVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  
  plVar2 = (long *)OSMetaClassBase::safeMetaCast
                             ((OSMetaClassBase *)param_2,*(OSMetaClass **)PTR_metaClass_00030080);
  if (plVar2 == (long *)0x0) {
    uVar3 = 0xe00002c2;
  }
  else {
    uVar4 = (**(code **)(*plVar2 + 0x150))(plVar2);
    puVar1 = *(undefined8 **)(this + 0x40);
    if (puVar1 == (undefined8 *)0x0) {
      uVar3 = 0;
    }
    else {
      uVar3 = (**(code **)*puVar1)(puVar1,uVar4);
      if ((int)uVar3 != 0) {
        return uVar3;
      }
    }
    *(undefined8 *)(this + 0x30) = uVar4;
  }
  return uVar3;
}
