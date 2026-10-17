// m_audio_b_FWBaseEngine::StartAudio
// Entry: 00004bca
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::StartAudio() */

int __thiscall m_audio_b_FWBaseEngine::StartAudio(m_audio_b_FWBaseEngine *this)

{
  undefined *puVar1;
  undefined4 uVar2;
  int iVar3;
  long *plVar4;
  OSMetaClassBase *pOVar5;
  long lVar6;
  undefined4 uVar7;
  bool bVar8;
  long *local_50;
  long *local_48;
  undefined8 local_40;
  undefined4 local_34;
  
  plVar4 = (long *)IOFireWireNub::getBus();
  (**(code **)(*plVar4 + 0x860))(plVar4,&local_34);
  _clock_interval_to_deadline(0x14,1000000,&local_40);
  _absolutetime_to_nanoseconds(local_40,this + 0x2c0);
  uVar2 = _AddFWCycleTimeToFWCycleTime(local_34,0xa0000);
  local_48 = (long *)0x0;
  pOVar5 = (OSMetaClassBase *)
           TCollectionIterator<m_audio_b_FWDCLProgram*>::start
                     ((TCollectionIterator<m_audio_b_FWDCLProgram*> *)&local_48,
                      *(OSCollection **)(this + 0x178));
  puVar1 = m_audio_b_FWDCLOutputProgram::metaClass;
  iVar3 = 0;
  while (pOVar5 != (OSMetaClassBase *)0x0) {
    lVar6 = OSMetaClassBase::safeMetaCast(pOVar5,(OSMetaClass *)puVar1);
    uVar7 = 0;
    if (lVar6 == 0) {
      uVar7 = uVar2;
    }
    iVar3 = (**(code **)(*(long *)pOVar5 + 0x130))(pOVar5,uVar7);
    if (iVar3 != 0) {
      bVar8 = false;
      goto LAB_00004ca1;
    }
    pOVar5 = (OSMetaClassBase *)(**(code **)(*local_48 + 0x128))();
  }
  bVar8 = true;
LAB_00004ca1:
  if (local_48 != (long *)0x0) {
    (**(code **)(*local_48 + 0x28))();
  }
  if (bVar8) {
    local_50 = (long *)0x0;
    plVar4 = (long *)TCollectionIterator<m_audio_b_FWDCLProgram*>::start
                               ((TCollectionIterator<m_audio_b_FWDCLProgram*> *)&local_50,
                                *(OSCollection **)(this + 0x178));
    while (plVar4 != (long *)0x0) {
      iVar3 = (**(code **)(*plVar4 + 0x138))();
      if (iVar3 != 0) {
        bVar8 = false;
        goto LAB_00004cfd;
      }
      plVar4 = (long *)(**(code **)(*local_50 + 0x128))();
    }
    bVar8 = true;
LAB_00004cfd:
    if (local_50 != (long *)0x0) {
      (**(code **)(*local_50 + 0x28))();
    }
    if (bVar8) {
      (**(code **)(*(long *)this + 0xae0))(this,0,&local_40);
    }
  }
  return iVar3;
}
