// m_audio_b_FWAudioDevice::CreateStereoPairSubEngines
// Entry: 000030f2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j)

/* m_audio_b_FWAudioDevice::CreateStereoPairSubEngines() */

void __thiscall m_audio_b_FWAudioDevice::CreateStereoPairSubEngines(m_audio_b_FWAudioDevice *this)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  undefined *puVar4;
  char cVar5;
  long lVar6;
  OSCollection *pOVar7;
  ulong uVar8;
  long lVar9;
  m_audio_b_FWPairEngine *this_00;
  long *plVar10;
  char *pcVar11;
  OSCollection *pOVar12;
  uint uVar13;
  int local_54;
  uint local_40;
  int local_3c;
  long *local_38;
  
  lVar6 = (**(code **)(*(long *)this + 0xa60))();
  pOVar7 = (OSCollection *)OSSet::withCapacity(4);
  puVar4 = m_audio_b_FWPairEngine::metaClass;
  if (pOVar7 != (OSCollection *)0x0) {
    local_54 = 1;
    for (uVar8 = 0; uVar3 = *(uint *)(lVar6 + 0x4c + uVar8 * 0x10), uVar3 != 3;
        uVar8 = (ulong)((int)uVar8 + 1)) {
      lVar9 = uVar8 * 0x10;
      uVar2 = *(uint *)(lVar6 + 0x48 + lVar9);
      cVar1 = *(char *)(lVar6 + 0x40 + lVar9);
      local_40 = *(uint *)(lVar6 + 0x44 + lVar9);
      local_3c = local_40 * 2 + -2;
      uVar13 = 1;
      for (; local_40 <= uVar2; local_40 = local_40 + 1) {
        local_38 = (long *)0x0;
        pOVar12 = pOVar7;
        lVar9 = TCollectionIterator<m_audio_b_FWTTrio*>::start
                          ((TCollectionIterator<m_audio_b_FWTTrio*> *)&local_38,pOVar7);
        this_00 = (m_audio_b_FWPairEngine *)0x0;
        while ((lVar9 != 0 &&
               (((*(uint *)(lVar9 + 0xc) != uVar3 || (uVar13 != *(uint *)(lVar9 + 0x10))) ||
                (pOVar12 = (OSCollection *)puVar4,
                this_00 = (m_audio_b_FWPairEngine *)
                          OSMetaClassBase::safeMetaCast
                                    (*(OSMetaClassBase **)(lVar9 + 0x18),(OSMetaClass *)puVar4),
                this_00 == (m_audio_b_FWPairEngine *)0x0))))) {
          lVar9 = (**(code **)(*local_38 + 0x128))();
        }
        if (local_38 != (long *)0x0) {
          (**(code **)(*local_38 + 0x28))();
        }
        if (this_00 == (m_audio_b_FWPairEngine *)0x0) {
          this_00 = OSObject::operator_new
                              ((OSObject *)&segment_command_00000158.fileoff,(ulong)pOVar12);
          if (this_00 == (m_audio_b_FWPairEngine *)0x0) {
            return;
          }
          m_audio_b_FWPairEngine::m_audio_b_FWPairEngine(this_00);
          cVar5 = (**(code **)(*(long *)this_00 + 0x88))(this_00);
          if (cVar5 == '\0') {
            (**(code **)(*(long *)this_00 + 0x28))(this_00);
            return;
          }
          m_audio_b_FWTimeSlaveEngine::SetMasterEngine
                    ((m_audio_b_FWTimeSlaveEngine *)this_00,
                     *(m_audio_b_FWDiscreteEngine **)(this + 0xe8));
          *(int *)(this_00 + 300) = local_54;
          (**(code **)(**(long **)(this + 0xe8) + 0xc28))(*(long **)(this + 0xe8),this_00);
          plVar10 = (long *)m_audio_b_FWTTrio::trio(uVar3,uVar13,(OSObject *)this_00);
          if (plVar10 != (long *)0x0) {
            (**(code **)(*(long *)pOVar7 + 0x1c0))(pOVar7,plVar10);
            (**(code **)(*plVar10 + 0x28))(plVar10);
          }
          (**(code **)(*(long *)this_00 + 0x28))(this_00);
        }
        if (cVar1 == '\0') {
          this_00[0x17d] = (m_audio_b_FWPairEngine)0x1;
          *(int *)(this_00 + 0x130) = local_3c;
        }
        else {
          this_00[0x17c] = (m_audio_b_FWPairEngine)0x1;
          *(int *)(this_00 + 0x134) = local_3c;
        }
        uVar13 = uVar13 + 1;
        local_54 = local_54 + 1;
        local_3c = local_3c + 2;
      }
    }
    pcVar11 = (char *)(**(code **)(*(long *)this + 0xa50))(this);
    NameAndActivatePairEngines(this,(OSSet *)pOVar7,pcVar11);
    (**(code **)(*(long *)pOVar7 + 0x28))(pOVar7);
  }
  return;
}
