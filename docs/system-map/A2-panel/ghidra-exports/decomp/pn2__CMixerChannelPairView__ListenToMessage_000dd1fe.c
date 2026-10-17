// CMixerChannelPairView::ListenToMessage
// Entry: 000dd1fe
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CMixerChannelPairView::ListenToMessage(long, void*) */

void __thiscall
CMixerChannelPairView::ListenToMessage(CMixerChannelPairView *this,long param_1,void *param_2)

{
  CMixerChannelPairView CVar1;
  char cVar2;
  short sVar3;
  short sVar4;
  int iVar5;
  int *piVar6;
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
    case 0x79:
      goto switchD_000dd232_caseD_79;
    case 0x7a:
      cVar2 = (**(code **)(*(int *)this + 0x20c))(this);
      if (cVar2 == '\0') {
        return;
      }
      local_13 = '\0';
      iVar5 = (**(code **)(**(int **)(this + 0xe8) + 0x20))(*(int **)(this + 0xe8));
      local_14 = iVar5 == 1;
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5ee,*(undefined2 *)(this + 0xb6),1,&local_13);
      if (sVar4 != 0) {
        return;
      }
      if ((bool)local_13 == (iVar5 == 1)) {
        return;
      }
      (**(code **)(**(int **)(this + 0xc0) + 0x18))
                (*(int **)(this + 0xc0),0x5ee,*(undefined2 *)(this + 0xb6),1,&local_14,0);
      *(undefined1 *)(*(int *)(this + 200) + 0x90) = local_14;
      *(undefined1 *)(*(int *)(this + 0xd8) + 0x90) = local_14;
      return;
    case 0x7b:
      cVar2 = (**(code **)(*(int *)this + 0x210))(this);
      if (cVar2 == '\0') {
        return;
      }
      local_15 = '\0';
      iVar5 = (**(code **)(**(int **)(this + 0xec) + 0x20))(*(int **)(this + 0xec));
      local_16 = iVar5 == 1;
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5ef,*(undefined2 *)(this + 0xb6),1,&local_15);
      if (sVar4 != 0) {
        return;
      }
      if ((bool)local_15 == (iVar5 == 1)) {
        return;
      }
      (**(code **)(**(int **)(this + 0xc0) + 0x18))
                (*(int **)(this + 0xc0),0x5ef,*(undefined2 *)(this + 0xb6),1,&local_16,0);
      return;
    case 0x7c:
      cVar2 = (**(code **)(*(int *)this + 0x214))(this);
      if (cVar2 == '\0') {
        return;
      }
      local_17 = '\0';
      iVar5 = (**(code **)(**(int **)(this + 0xf0) + 0x20))(*(int **)(this + 0xf0));
      local_18 = iVar5 == 1;
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5f0,*(undefined2 *)(this + 0xb6),1,&local_17);
      if (sVar4 != 0) {
        return;
      }
      if ((bool)local_17 == (iVar5 == 1)) {
        return;
      }
      (**(code **)(**(int **)(this + 0xc0) + 0x18))
                (*(int **)(this + 0xc0),0x5f0,*(undefined2 *)(this + 0xb6),1,&local_18,0);
      return;
    default:
      return;
    }
  }
  if (0xb < param_1 - 0x5e5U) {
    return;
  }
  CVar1 = this[0xb4];
  switch(param_1) {
  case 0x5e5:
    cVar2 = (**(code **)(*(int *)this + 0x208))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    sVar4 = **(short **)((int)param_2 + 0xc);
    sVar3 = (**(code **)(**(int **)(this + 0xc0) + 0x30))
                      (*(int **)(this + 0xc0),*(undefined2 *)(this + 0xb6));
    piVar6 = *(int **)(this + 0xe4);
    if (sVar3 == sVar4) {
      (**(code **)(*piVar6 + 200))(piVar6);
    }
    else {
      (**(code **)(*piVar6 + 0xcc))(piVar6);
    }
    goto LAB_000dd769;
  default:
    goto switchD_000dd232_default;
  case 0x5ea:
    cVar2 = (**(code **)(*(int *)this + 0x1f8))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    if (*(short *)((int)param_2 + 4) == *(short *)(this + 0xb8)) {
      piVar6 = *(int **)(this + 0xc4);
    }
    else {
      if (*(short *)((int)param_2 + 4) != *(short *)(this + 0xba)) goto LAB_000dd769;
      piVar6 = *(int **)(this + 0xd4);
    }
    break;
  case 0x5eb:
    cVar2 = (**(code **)(*(int *)this + 0x204))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    if (*(short *)((int)param_2 + 4) == *(short *)(this + 0xb8)) {
      piVar6 = *(int **)(this + 0xd0);
    }
    else {
      if (*(short *)((int)param_2 + 4) != *(short *)(this + 0xba)) goto LAB_000dd769;
      piVar6 = *(int **)(this + 0xe0);
    }
    break;
  case 0x5ec:
    cVar2 = (**(code **)(*(int *)this + 0x1fc))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    if (*(short *)((int)param_2 + 4) == *(short *)(this + 0xb8)) {
      piVar6 = *(int **)(this + 200);
    }
    else {
      if (*(short *)((int)param_2 + 4) != *(short *)(this + 0xba)) goto LAB_000dd769;
      piVar6 = *(int **)(this + 0xd8);
    }
    break;
  case 0x5ed:
    cVar2 = (**(code **)(*(int *)this + 0x208))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    piVar6 = *(int **)(this + 0xe4);
    goto LAB_000dd705;
  case 0x5ee:
    cVar2 = (**(code **)(*(int *)this + 0x20c))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    (**(code **)(**(int **)(this + 0xe8) + 0x24))
              (*(int **)(this + 0xe8),**(undefined1 **)((int)param_2 + 0xc));
    *(undefined1 *)(*(int *)(this + 200) + 0x90) = **(undefined1 **)((int)param_2 + 0xc);
    *(undefined1 *)(*(int *)(this + 0xd8) + 0x90) = **(undefined1 **)((int)param_2 + 0xc);
    goto LAB_000dd769;
  case 0x5ef:
    cVar2 = (**(code **)(*(int *)this + 0x210))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    piVar6 = *(int **)(this + 0xec);
    goto LAB_000dd705;
  case 0x5f0:
    cVar2 = (**(code **)(*(int *)this + 0x214))(this);
    if (cVar2 == '\0') {
      return;
    }
    if (CVar1 != (CMixerChannelPairView)0x0) {
      this[0xb4] = (CMixerChannelPairView)0x0;
    }
    piVar6 = *(int **)(this + 0xf0);
LAB_000dd705:
    (**(code **)(*piVar6 + 0x24))(piVar6,**(undefined1 **)((int)param_2 + 0xc));
    goto LAB_000dd769;
  }
  (**(code **)(*piVar6 + 0x1d8))(piVar6,**(undefined4 **)((int)param_2 + 0xc));
LAB_000dd769:
  if (CVar1 != (CMixerChannelPairView)0x0) {
    this[0xb4] = (CMixerChannelPairView)0x1;
  }
switchD_000dd232_default:
  return;
switchD_000dd232_caseD_79:
  cVar2 = (**(code **)(*(int *)this + 0x208))(this);
  if (cVar2 == '\0') {
    return;
  }
  local_11 = '\0';
  iVar5 = (**(code **)(**(int **)(this + 0xe4) + 0x20))(*(int **)(this + 0xe4));
  local_12 = iVar5 == 1;
  sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                    (*(int **)(this + 0xc0),0x5ed,*(undefined2 *)(this + 0xb6),1,&local_11);
  if (sVar4 != 0) {
    return;
  }
  if ((bool)local_11 == (iVar5 == 1)) {
    return;
  }
  (**(code **)(**(int **)(this + 0xc0) + 0x18))
            (*(int **)(this + 0xc0),0x5ed,*(undefined2 *)(this + 0xb6),1,&local_12,0);
  return;
}
