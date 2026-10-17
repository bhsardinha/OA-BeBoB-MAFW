// m_audio_b_FWAudioDevice::NameAndActivatePairEngines
// Entry: 00002750
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWAudioDevice::NameAndActivatePairEngines(OSSet*, char const*) */

void __thiscall
m_audio_b_FWAudioDevice::NameAndActivatePairEngines
          (m_audio_b_FWAudioDevice *this,OSSet *param_1,char *param_2)

{
  undefined *puVar1;
  long lVar2;
  long *plVar3;
  undefined **ppuVar4;
  char *pcVar5;
  long *local_138;
  char local_130 [256];
  
  local_138 = (long *)0x0;
  lVar2 = TCollectionIterator<m_audio_b_FWTTrio*>::start
                    ((TCollectionIterator<m_audio_b_FWTTrio*> *)&local_138,(OSCollection *)param_1);
  puVar1 = m_audio_b_FWPairEngine::metaClass;
  plVar3 = local_138;
  if (lVar2 != 0) {
    do {
      plVar3 = (long *)OSMetaClassBase::safeMetaCast
                                 (*(OSMetaClassBase **)(lVar2 + 0x18),(OSMetaClass *)puVar1);
      if (plVar3 != (long *)0x0) {
        if (*(int *)(lVar2 + 0xc) == 1) {
          ppuVar4 = &kSPDIFString;
        }
        else if (*(int *)(lVar2 + 0xc) == 2) {
          ppuVar4 = &kADATString;
        }
        else {
          ppuVar4 = &kAnalogString;
        }
        if (*(char *)((long)plVar3 + 0x17c) == '\0') {
          pcVar5 = " Out";
        }
        else if (*(char *)((long)plVar3 + 0x17d) == '\0') {
          pcVar5 = " In";
        }
        else {
          pcVar5 = "";
        }
        _snprintf(local_130,0x100,"%s %s %d/%d%s",param_2,*ppuVar4,
                  (ulong)(*(int *)(lVar2 + 0x10) * 2 - 1),*(int *)(lVar2 + 0x10) * 2,pcVar5);
        (**(code **)(*plVar3 + 0x9f8))(plVar3,local_130);
        (**(code **)(*(long *)this + 0x978))(this,plVar3);
      }
      plVar3 = local_138;
      lVar2 = (**(code **)(*local_138 + 0x128))(local_138);
    } while (lVar2 != 0);
  }
  if (plVar3 != (long *)0x0) {
    (**(code **)(*plVar3 + 0x28))(plVar3);
  }
  return;
}
