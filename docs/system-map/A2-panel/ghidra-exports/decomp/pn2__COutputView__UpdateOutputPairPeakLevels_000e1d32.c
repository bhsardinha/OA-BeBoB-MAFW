// COutputView::UpdateOutputPairPeakLevels
// Entry: 000e1d32
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* COutputView::UpdateOutputPairPeakLevels(middleWareMessage*) */

void __thiscall
COutputView::UpdateOutputPairPeakLevels(COutputView *this,middleWareMessage *param_1)

{
  int iVar1;
  COutputChannelPairView *this_00;
  int iVar2;
  uint uVar3;
  ushort local_14;
  ushort local_12;
  
  if (*(int *)(*(int *)(this + 0x124) + 8) != 0) {
    iVar1 = *(int *)(param_1 + 0xc);
    uVar3 = 1;
    do {
      (**(code **)(**(int **)(this + 0x11c) + 0x24))
                (*(int **)(this + 0x11c),uVar3 & 0xffff,&local_12,&local_14);
      iVar2 = *(int *)(this + 0x124);
      this_00 = *(COutputChannelPairView **)(**(int **)(iVar2 + 0x14) + -4 + uVar3 * 4);
      if ((this_00 != (COutputChannelPairView *)0x0) && (*(int *)(this_00 + 0x28) != 0)) {
        COutputChannelPairView::SetVUMeters
                  (this_00,*(long *)(iVar1 + -4 + (uint)local_12 * 4),
                   *(long *)(iVar1 + -4 + (uint)local_14 * 4),'\0');
        iVar2 = *(int *)(this + 0x124);
      }
      uVar3 = uVar3 + 1;
    } while (uVar3 <= *(uint *)(iVar2 + 8));
  }
  return;
}
