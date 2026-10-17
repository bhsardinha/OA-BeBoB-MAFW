// m_audio_b_FWBaseEngine::performErase
// Entry: 00004e08
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::performErase() */

void __thiscall m_audio_b_FWBaseEngine::performErase(m_audio_b_FWBaseEngine *this)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  char cVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  long *plVar8;
  long *plVar9;
  void *pvVar10;
  uint uVar11;
  
  cVar4 = (**(code **)(*(long *)this + 0xa88))();
  if (((cVar4 == '\0') || (iVar5 = (**(code **)(*(long *)this + 0xa70))(), iVar5 != 1)) ||
     (plVar8 = (long *)OSCollectionIterator::withCollection(*(OSCollection **)(this + 0xe0)),
     plVar8 == (long *)0x0)) {
    return;
  }
  uVar6 = (**(code **)(*(long *)this + 0xa38))(this);
  uVar1 = *(uint *)(*(long *)(this + 0xb0) + 0x10);
  plVar9 = (long *)(**(code **)(*plVar8 + 0x128))(plVar8);
  if (plVar9 != (long *)0x0) {
    do {
      (**(code **)(*plVar9 + 0xaa8))(plVar9);
      pvVar10 = (void *)(**(code **)(*plVar9 + 0xa00))(plVar9);
      uVar2 = *(uint *)(plVar9 + 0x16);
      iVar5 = uVar2 * 4;
      uVar11 = iVar5 * uVar6;
      uVar7 = (**(code **)(*plVar9 + 0xa08))(plVar9);
      if (uVar6 < uVar1) {
        if (((uVar11 <= uVar7) &&
            (iVar3 = *(int *)(this + 0x88), uVar7 = (**(code **)(*plVar9 + 0xa08))(plVar9),
            (uint)(iVar3 * iVar5) <= uVar7)) &&
           ((pvVar10 != (void *)0x0 && ((uVar2 & 0x3fffffff) != 0)))) {
          _bzero(pvVar10,(ulong)uVar11);
          iVar3 = *(int *)(this + 0x88) - uVar1;
LAB_00004f66:
          _bzero((void *)((ulong)(iVar5 * uVar1) + (long)pvVar10),(ulong)(uint)(iVar3 * iVar5));
        }
      }
      else if (((uVar11 <= uVar7) && (pvVar10 != (void *)0x0)) &&
              (iVar3 = uVar6 - uVar1, (uVar2 & 0x3fffffff) != 0)) goto LAB_00004f66;
      (**(code **)(*plVar9 + 0xab0))(plVar9);
      plVar9 = (long *)(**(code **)(*plVar8 + 0x128))(plVar8);
    } while (plVar9 != (long *)0x0);
  }
  *(uint *)(*(long *)(this + 0xb0) + 0x10) = uVar6;
                    /* WARNING: Could not recover jumptable at 0x00004fb9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar8 + 0x28))(plVar8);
  return;
}
