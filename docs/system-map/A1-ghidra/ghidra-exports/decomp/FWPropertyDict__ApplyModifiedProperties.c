// m_audio_b_FWPropertyDict::ApplyModifiedProperties
// Entry: 00009b40
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_b_FWPropertyDict::ApplyModifiedProperties(bool) */

void __thiscall
m_audio_b_FWPropertyDict::ApplyModifiedProperties(m_audio_b_FWPropertyDict *this,bool param_1)

{
  long *plVar1;
  char cVar2;
  int iVar3;
  long lVar4;
  IOService *pIVar5;
  long *plVar6;
  long *local_38;
  
  cVar2 = _OSCompareAndSwap(0,1,this + 0x70);
  if ((cVar2 != '\0') && (*(OSDictionary **)(this + 0x58) != (OSDictionary *)0x0)) {
    CacheModifiedPropertyDictionaryInUserClients(this,*(OSDictionary **)(this + 0x58));
    local_38 = (long *)0x0;
    lVar4 = TCollectionIterator<OSSymbol*>::start
                      ((TCollectionIterator<OSSymbol*> *)&local_38,*(OSCollection **)(this + 0x58));
    plVar1 = local_38;
    while (lVar4 != 0) {
      pIVar5 = (IOService *)
               (**(code **)(**(long **)(this + 0x60) + 0x1f8))(*(long **)(this + 0x60),lVar4);
      plVar6 = *(long **)(this + 0x58);
      if (pIVar5 == (IOService *)0x0) {
        (**(code **)(*plVar6 + 0x1d8))(plVar6,lVar4);
        (**(code **)(*plVar1 + 0x118))(plVar1);
      }
      else {
        plVar6 = (long *)(**(code **)(*plVar6 + 0x1f8))(plVar6,lVar4);
        if (plVar6 != (long *)0x0) {
          (**(code **)(*plVar6 + 0x20))(plVar6);
          (**(code **)(**(long **)(this + 0x58) + 0x1d8))(*(long **)(this + 0x58),lVar4);
          (**(code **)(*plVar1 + 0x118))(plVar1);
          iVar3 = (**(code **)(*(long *)pIVar5 + 0x128))(pIVar5,lVar4,plVar6);
          (**(code **)(*plVar6 + 0x28))(plVar6);
          if ((iVar3 == 0) && (param_1)) {
            m_audio_b_FWBaseProp::NotifyAudioControlsOfChange(pIVar5);
          }
        }
      }
      lVar4 = (**(code **)(*plVar1 + 0x128))(plVar1);
    }
    if (plVar1 != (long *)0x0) {
      (**(code **)(*plVar1 + 0x28))(plVar1);
    }
    _OSCompareAndSwap(1,0,this + 0x70);
  }
  return;
}
