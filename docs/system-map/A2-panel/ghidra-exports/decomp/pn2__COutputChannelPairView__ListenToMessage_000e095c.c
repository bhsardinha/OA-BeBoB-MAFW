// COutputChannelPairView::ListenToMessage
// Entry: 000e095c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* COutputChannelPairView::ListenToMessage(long, void*) */

void __thiscall
COutputChannelPairView::ListenToMessage(COutputChannelPairView *this,long param_1,void *param_2)

{
  COutputChannelPairView CVar1;
  char cVar2;
  short sVar3;
  short sVar4;
  int iVar5;
  int *piVar6;
  short local_1c;
  short local_1a;
  undefined1 local_18;
  char local_17;
  undefined1 local_16;
  char local_15;
  undefined1 local_14;
  char local_13;
  undefined1 local_12;
  char local_11;
  
  if ((uint)param_1 < 0x5e5) {
    switch(param_1) {
    case 0x7a:
      goto switchD_000e0990_caseD_7a;
    case 0x7b:
      cVar2 = (**(code **)(*(int *)this + 0x20c))(this);
      if (cVar2 == '\0') {
        return;
      }
      local_13 = '\0';
      iVar5 = (**(code **)(**(int **)(this + 0xe4) + 0x20))(*(int **)(this + 0xe4));
      local_14 = iVar5 == 1;
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5f8,*(undefined2 *)(this + 0xb6),1,&local_13);
      if (sVar4 != 0) {
        return;
      }
      if ((bool)local_13 == (iVar5 == 1)) {
        return;
      }
      (**(code **)(**(int **)(this + 0xc0) + 0x18))
                (*(int **)(this + 0xc0),0x5f8,*(undefined2 *)(this + 0xb6),1,&local_14,0);
      *(undefined1 *)(*(int *)(this + 0xc4) + 0x90) = local_14;
      *(undefined1 *)(*(int *)(this + 0xd0) + 0x90) = local_14;
      return;
    case 0x7c:
      cVar2 = (**(code **)(*(int *)this + 0x210))(this);
      if (cVar2 == '\0') {
        return;
      }
      local_15 = '\0';
      iVar5 = (**(code **)(**(int **)(this + 0xe8) + 0x20))(*(int **)(this + 0xe8));
      local_16 = iVar5 == 1;
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5f9,*(undefined2 *)(this + 0xb6),1,&local_15);
      if (sVar4 != 0) {
        return;
      }
      if ((bool)local_15 == (iVar5 == 1)) {
        return;
      }
      (**(code **)(**(int **)(this + 0xc0) + 0x18))
                (*(int **)(this + 0xc0),0x5f9,*(undefined2 *)(this + 0xb6),1,&local_16,0);
      return;
    case 0x7d:
      cVar2 = (**(code **)(*(int *)this + 0x214))(this);
      if (cVar2 == '\0') {
        return;
      }
      local_17 = '\0';
      iVar5 = (**(code **)(**(int **)(this + 0xec) + 0x20))(*(int **)(this + 0xec));
      local_18 = iVar5 == 1;
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5fa,*(undefined2 *)(this + 0xb6),1,&local_17);
      if (sVar4 != 0) {
        return;
      }
      if ((bool)local_17 == (iVar5 == 1)) {
        return;
      }
      (**(code **)(**(int **)(this + 0xc0) + 0x18))
                (*(int **)(this + 0xc0),0x5fa,*(undefined2 *)(this + 0xb6),1,&local_18,0);
      return;
    default:
      return;
    case 0x83:
      cVar2 = (**(code **)(*(int *)this + 0x218))(this);
      if (cVar2 == '\0') {
        return;
      }
      local_1a = 0;
      iVar5 = (**(code **)(**(int **)(this + 0xf0) + 0x20))(*(int **)(this + 0xf0));
      sVar3 = (iVar5 != 1) + 1;
      local_1c = sVar3;
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5fb,*(undefined2 *)(this + 0xb6),2,&local_1a);
      if (sVar4 != 0) {
        return;
      }
      if (local_1a == sVar3) {
        return;
      }
      (**(code **)(**(int **)(this + 0xc0) + 0x18))
                (*(int **)(this + 0xc0),0x5fb,*(undefined2 *)(this + 0xb6),2,&local_1c,0);
      return;
    }
  }
  CVar1 = this[0xb4];
  if ((uint)param_1 < 0x5f4) {
    if (param_1 != 0x5e5) {
      return;
    }
    cVar2 = (**(code **)(*(int *)this + 0x208))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    sVar4 = **(short **)((int)param_2 + 0xc);
    sVar3 = (**(code **)(**(int **)(this + 0xc0) + 0x2c))
                      (*(int **)(this + 0xc0),*(undefined2 *)(this + 0xb6));
    piVar6 = *(int **)(this + 0xe0);
    if (sVar3 == sVar4) {
      (**(code **)(*piVar6 + 200))(piVar6);
    }
    else {
      (**(code **)(*piVar6 + 0xcc))(piVar6);
    }
    goto LAB_000e0fb2;
  }
  switch(param_1) {
  case 0x5f4:
    cVar2 = (**(code **)(*(int *)this + 0x1f8))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    if (*(short *)((int)param_2 + 4) == *(short *)(this + 0xb8)) {
      piVar6 = *(int **)(this + 0xc4);
    }
    else {
      if (*(short *)((int)param_2 + 4) != *(short *)(this + 0xba)) goto LAB_000e0fb2;
      piVar6 = *(int **)(this + 0xd0);
    }
    break;
  case 0x5f5:
    cVar2 = (**(code **)(*(int *)this + 0x200))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    if (*(short *)((int)param_2 + 4) == *(short *)(this + 0xb8)) {
      piVar6 = *(int **)(this + 0xcc);
    }
    else {
      if (*(short *)((int)param_2 + 4) != *(short *)(this + 0xba)) goto LAB_000e0fb2;
      piVar6 = *(int **)(this + 0xd8);
    }
    break;
  case 0x5f6:
    cVar2 = (**(code **)(*(int *)this + 0x204))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    piVar6 = *(int **)(this + 0xdc);
    break;
  case 0x5f7:
    cVar2 = (**(code **)(*(int *)this + 0x208))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    piVar6 = *(int **)(this + 0xe0);
    goto LAB_000e0f2c;
  case 0x5f8:
    cVar2 = (**(code **)(*(int *)this + 0x20c))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    (**(code **)(**(int **)(this + 0xe4) + 0x24))
              (*(int **)(this + 0xe4),**(undefined1 **)((int)param_2 + 0xc));
    *(undefined1 *)(*(int *)(this + 0xc4) + 0x90) = **(undefined1 **)((int)param_2 + 0xc);
    *(undefined1 *)(*(int *)(this + 0xd0) + 0x90) = **(undefined1 **)((int)param_2 + 0xc);
    goto LAB_000e0fb2;
  case 0x5f9:
    cVar2 = (**(code **)(*(int *)this + 0x210))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    piVar6 = *(int **)(this + 0xe8);
    goto LAB_000e0f2c;
  case 0x5fa:
    cVar2 = (**(code **)(*(int *)this + 0x214))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    piVar6 = *(int **)(this + 0xec);
LAB_000e0f2c:
    (**(code **)(*piVar6 + 0x24))(piVar6,**(undefined1 **)((int)param_2 + 0xc));
    goto LAB_000e0fb2;
  case 0x5fb:
    cVar2 = (**(code **)(*(int *)this + 0x218))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (COutputChannelPairView)0x0) {
      this[0xb4] = (COutputChannelPairView)0x0;
    }
    (**(code **)(**(int **)(this + 0xf0) + 0x24))
              (*(int **)(this + 0xf0),**(short **)((int)param_2 + 0xc) == 1);
    goto LAB_000e0fb2;
  default:
    goto switchD_000e0990_caseD_7e;
  }
  (**(code **)(*piVar6 + 0x1d8))(piVar6,**(undefined4 **)((int)param_2 + 0xc));
LAB_000e0fb2:
  if (CVar1 != (COutputChannelPairView)0x0) {
    this[0xb4] = (COutputChannelPairView)0x1;
  }
switchD_000e0990_caseD_7e:
  return;
switchD_000e0990_caseD_7a:
  cVar2 = (**(code **)(*(int *)this + 0x208))(this);
  if (cVar2 == '\0') {
    return;
  }
  local_11 = '\0';
  iVar5 = (**(code **)(**(int **)(this + 0xe0) + 0x20))(*(int **)(this + 0xe0));
  local_12 = iVar5 == 1;
  sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                    (*(int **)(this + 0xc0),0x5f7,*(undefined2 *)(this + 0xb6),1,&local_11);
  if (sVar4 != 0) {
    return;
  }
  if ((bool)local_11 == (iVar5 == 1)) {
    return;
  }
  (**(code **)(**(int **)(this + 0xc0) + 0x18))
            (*(int **)(this + 0xc0),0x5f7,*(undefined2 *)(this + 0xb6),1,&local_12,0);
  return;
}
