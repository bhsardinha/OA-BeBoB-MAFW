// m_audio_b_FWPropertyDict::SetProperties
// Entry: 0000a0e0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::SetProperties(OSObject*) */

undefined8 __thiscall
m_audio_b_FWPropertyDict::SetProperties(m_audio_b_FWPropertyDict *this,OSObject *param_1)

{
  long lVar1;
  long *plVar2;
  OSSymbol *pOVar3;
  OSObject *pOVar4;
  undefined8 uVar5;
  
  lVar1 = OSMetaClassBase::safeMetaCast
                    ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030008);
  if (lVar1 == 0) {
    lVar1 = OSMetaClassBase::safeMetaCast
                      ((OSMetaClassBase *)param_1,*(OSMetaClass **)PTR_metaClass_00030090);
    if (lVar1 != 0) {
      pOVar3 = (OSSymbol *)OSSymbol::withString((OSString *)param_1);
      SetOneProperty(this,pOVar3,(OSObject *)0x0);
      (**(code **)(*(long *)pOVar3 + 0x28))(pOVar3);
      uVar5 = 0;
      goto LAB_0000a1c5;
    }
  }
  else if ((param_1 != (OSObject *)0x0) &&
          (plVar2 = (long *)OSCollectionIterator::withCollection((OSCollection *)param_1),
          plVar2 != (long *)0x0)) {
    pOVar3 = (OSSymbol *)(**(code **)(*plVar2 + 0x128))(plVar2);
    if (pOVar3 == (OSSymbol *)0x0) {
      uVar5 = 0xe00002c2;
    }
    else {
      do {
        pOVar4 = (OSObject *)(**(code **)(*(long *)param_1 + 0x1f8))(param_1,pOVar3);
        SetOneProperty(this,pOVar3,pOVar4);
        pOVar3 = (OSSymbol *)(**(code **)(*plVar2 + 0x128))(plVar2);
      } while (pOVar3 != (OSSymbol *)0x0);
      uVar5 = 0;
    }
    if (plVar2 != (long *)0x0) {
      (**(code **)(*plVar2 + 0x28))(plVar2);
    }
    goto LAB_0000a1c5;
  }
  uVar5 = 0xe00002c2;
LAB_0000a1c5:
  if (*(long *)(this + 0x30) != 0) {
    IOEventSource::signalWorkAvailable();
  }
  return uVar5;
}
