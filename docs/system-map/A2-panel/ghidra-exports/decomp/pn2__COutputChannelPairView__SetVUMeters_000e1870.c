// COutputChannelPairView::SetVUMeters
// Entry: 000e1870
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* COutputChannelPairView::SetVUMeters(long, long, unsigned char) */

void __thiscall
COutputChannelPairView::SetVUMeters
          (COutputChannelPairView *this,long param_1,long param_2,uchar param_3)

{
  int *piVar1;
  char cVar2;
  
  cVar2 = (**(code **)(*(int *)this + 0x1fc))(this);
  if (cVar2 != '\0') {
    piVar1 = *(int **)(this + 200);
    if (param_3 == '\0') {
      (**(code **)(*piVar1 + 0x24))(piVar1,param_1);
      (**(code **)(**(int **)(this + 0xd4) + 0x24))(*(int **)(this + 0xd4),param_2);
    }
    else {
      (**(code **)(*piVar1 + 0x1e4))(piVar1,param_1);
      (**(code **)(**(int **)(this + 0xd4) + 0x1e4))(*(int **)(this + 0xd4),param_2);
    }
  }
  return;
}
