// m_audio_b_FWPropertyDict::CopyAllPropertiesToDictionary
// Entry: 000098e4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::CopyAllPropertiesToDictionary(OSDictionary*) const */

void __thiscall
m_audio_b_FWPropertyDict::CopyAllPropertiesToDictionary
          (m_audio_b_FWPropertyDict *this,OSDictionary *param_1)

{
  code *pcVar1;
  int iVar2;
  long *plVar3;
  long *plVar4;
  undefined8 uVar5;
  undefined8 local_40;
  long *local_38;
  
  local_38 = (long *)0x0;
  plVar3 = (long *)TCollectionIterator<OSSymbol*>::start
                             ((TCollectionIterator<OSSymbol*> *)&local_38,
                              *(OSCollection **)(this + 0x60));
  plVar4 = local_38;
  while (plVar3 != (long *)0x0) {
    plVar4 = (long *)(**(code **)(**(long **)(this + 0x60) + 0x1f8))(*(long **)(this + 0x60),plVar3)
    ;
    if (plVar4 != (long *)0x0) {
      pcVar1 = *(code **)(*plVar4 + 0x120);
      uVar5 = (**(code **)(*plVar3 + 0x148))(plVar3);
      iVar2 = (*pcVar1)(plVar4,uVar5,&local_40);
      if (iVar2 == 0) {
        (**(code **)(*(long *)param_1 + 0x1c0))(param_1,plVar3,local_40);
      }
    }
    plVar4 = local_38;
    plVar3 = (long *)(**(code **)(*local_38 + 0x128))(local_38);
  }
  if (plVar4 != (long *)0x0) {
    (**(code **)(*plVar4 + 0x28))(plVar4);
  }
  return;
}
