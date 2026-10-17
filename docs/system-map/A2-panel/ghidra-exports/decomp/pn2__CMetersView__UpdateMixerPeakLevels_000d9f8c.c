// CMetersView::UpdateMixerPeakLevels
// Entry: 000d9f8c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CMetersView::UpdateMixerPeakLevels(middleWareMessage*) */

void __thiscall CMetersView::UpdateMixerPeakLevels(CMetersView *this,middleWareMessage *param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  uint uVar4;
  
  if ((this[300] != (CMetersView)0x0) && (iVar3 = *(int *)(this + 0x124), *(int *)(iVar3 + 8) != 0))
  {
    iVar1 = *(int *)(param_1 + 0xc);
    uVar4 = 1;
    do {
      piVar2 = *(int **)(**(int **)(iVar3 + 0x14) + -4 + uVar4 * 4);
      if ((piVar2 != (int *)0x0) && (piVar2[10] != 0)) {
        (**(code **)(*piVar2 + 0x24))(piVar2,*(undefined4 *)(iVar1 + -4 + uVar4 * 4));
        iVar3 = *(int *)(this + 0x124);
      }
      uVar4 = uVar4 + 1;
    } while (uVar4 <= *(uint *)(iVar3 + 8));
  }
  return;
}
