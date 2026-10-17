// CMixerChannelPairView::AdaptUIElementsToSettings
// Entry: 000dd838
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CMixerChannelPairView::AdaptUIElementsToSettings() */

void __thiscall CMixerChannelPairView::AdaptUIElementsToSettings(CMixerChannelPairView *this)

{
  undefined2 uVar1;
  int iVar2;
  char cVar3;
  short sVar4;
  undefined4 uVar5;
  undefined1 uVar6;
  undefined1 local_25;
  undefined4 local_24;
  undefined4 local_20;
  float local_1c;
  short local_16;
  undefined4 local_14;
  
  local_14 = 0;
  local_16 = 0;
  local_25 = 0;
  cVar3 = (**(code **)(*(int *)this + 0x1f8))(this);
  if (cVar3 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5ea,&local_24);
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5ea,*(undefined2 *)(this + 0xb8),4,&local_14);
    (**(code **)(**(int **)(this + 0xc4) + 0x1dc))(*(int **)(this + 0xc4),local_24,local_20);
    CMALevelKnob::SetDisplayValueMultiplier(*(CMALevelKnob **)(this + 0xc4),local_1c);
    uVar5 = local_14;
    if (sVar4 != 0) {
      uVar5 = 0;
    }
    (**(code **)(**(int **)(this + 0xc4) + 0x1d8))(*(int **)(this + 0xc4),uVar5);
    uVar1 = *(undefined2 *)(this + 0xb8);
    iVar2 = *(int *)(this + 0xc4);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5ea;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5ea,*(undefined2 *)(this + 0xba),4,&local_14);
    (**(code **)(**(int **)(this + 0xd4) + 0x1dc))(*(int **)(this + 0xd4),local_24,local_20);
    CMALevelKnob::SetDisplayValueMultiplier(*(CMALevelKnob **)(this + 0xd4),local_1c);
    uVar5 = 0;
    if (sVar4 == 0) {
      uVar5 = local_14;
    }
    (**(code **)(**(int **)(this + 0xd4) + 0x1d8))(*(int **)(this + 0xd4),uVar5);
    uVar1 = *(undefined2 *)(this + 0xba);
    iVar2 = *(int *)(this + 0xd4);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5ea;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
  }
  cVar3 = (**(code **)(*(int *)this + 0x1fc))(this);
  if (cVar3 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5ec,&local_24);
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5ec,*(undefined2 *)(this + 0xb8),4,&local_14);
    (**(code **)(**(int **)(this + 200) + 0x1dc))(*(int **)(this + 200),local_24,local_20);
    uVar5 = local_14;
    if (sVar4 != 0) {
      uVar5 = 0;
    }
    (**(code **)(**(int **)(this + 200) + 0x1d8))(*(int **)(this + 200),uVar5);
    uVar1 = *(undefined2 *)(this + 0xb8);
    iVar2 = *(int *)(this + 200);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5ec;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5ec,*(undefined2 *)(this + 0xba),4,&local_14);
    (**(code **)(**(int **)(this + 0xd8) + 0x1dc))(*(int **)(this + 0xd8),local_24,local_20);
    uVar5 = 0;
    if (sVar4 == 0) {
      uVar5 = local_14;
    }
    (**(code **)(**(int **)(this + 0xd8) + 0x1d8))(*(int **)(this + 0xd8),uVar5);
    uVar1 = *(undefined2 *)(this + 0xba);
    iVar2 = *(int *)(this + 0xd8);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5ec;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
    cVar3 = (**(code **)(*(int *)this + 0x20c))(this);
    if (cVar3 == '\0') {
      *(undefined1 *)(*(int *)(this + 200) + 0x90) = 0;
      *(undefined1 *)(*(int *)(this + 0xd8) + 0x90) = 0;
    }
    else {
      sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5ee,*(undefined2 *)(this + 0xb6),1,&local_25);
      if (sVar4 != 0) {
        local_25 = 0;
      }
      (**(code **)(**(int **)(this + 0xe8) + 0x24))(*(int **)(this + 0xe8),local_25);
      *(undefined1 *)(*(int *)(this + 200) + 0x90) = local_25;
      *(undefined1 *)(*(int *)(this + 0xd8) + 0x90) = local_25;
    }
  }
  cVar3 = (**(code **)(*(int *)this + 0x200))(this);
  if (cVar3 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5e9,&local_24);
    (**(code **)(**(int **)(this + 0xcc) + 0x1e8))(*(int **)(this + 0xcc),local_24,local_20);
    (**(code **)(**(int **)(this + 0xcc) + 0x1e4))(*(int **)(this + 0xcc),local_24);
    (**(code **)(**(int **)(this + 0xdc) + 0x1e8))(*(int **)(this + 0xdc),local_24,local_20);
    (**(code **)(**(int **)(this + 0xdc) + 0x1e4))(*(int **)(this + 0xdc),local_24);
  }
  cVar3 = (**(code **)(*(int *)this + 0x204))(this);
  if (cVar3 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5eb,&local_24);
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5eb,*(undefined2 *)(this + 0xb8),4,&local_14);
    (**(code **)(**(int **)(this + 0xd0) + 0x1dc))(*(int **)(this + 0xd0),local_24,local_20);
    CMABalanceKnob::SetDisplayValueMultiplier(*(CMABalanceKnob **)(this + 0xd0),local_1c);
    uVar5 = local_14;
    if (sVar4 != 0) {
      uVar5 = 0;
    }
    (**(code **)(**(int **)(this + 0xd0) + 0x1d8))(*(int **)(this + 0xd0),uVar5);
    uVar1 = *(undefined2 *)(this + 0xb8);
    iVar2 = *(int *)(this + 0xd0);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5eb;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5eb,*(undefined2 *)(this + 0xba),4,&local_14);
    (**(code **)(**(int **)(this + 0xe0) + 0x1dc))(*(int **)(this + 0xe0),local_24,local_20);
    CMABalanceKnob::SetDisplayValueMultiplier(*(CMABalanceKnob **)(this + 0xe0),local_1c);
    uVar5 = 0;
    if (sVar4 == 0) {
      uVar5 = local_14;
    }
    (**(code **)(**(int **)(this + 0xe0) + 0x1d8))(*(int **)(this + 0xe0),uVar5);
    uVar1 = *(undefined2 *)(this + 0xba);
    iVar2 = *(int *)(this + 0xe0);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5eb;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
  }
  cVar3 = (**(code **)(*(int *)this + 0x208))(this);
  if (cVar3 != '\0') {
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))(*(int **)(this + 0xc0),0x5e5,0,2,&local_16)
    ;
    if ((sVar4 == 0) &&
       (sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0x30))
                          (*(int **)(this + 0xc0),*(undefined2 *)(this + 0xb6)), sVar4 == local_16))
    {
      (**(code **)(**(int **)(this + 0xe4) + 200))(*(int **)(this + 0xe4));
    }
    else {
      (**(code **)(**(int **)(this + 0xe4) + 0xcc))(*(int **)(this + 0xe4));
    }
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5ed,*(undefined2 *)(this + 0xb6),1,&local_25);
    uVar6 = 0;
    if (sVar4 == 0) {
      uVar6 = local_25;
    }
    (**(code **)(**(int **)(this + 0xe4) + 0x24))(*(int **)(this + 0xe4),uVar6);
  }
  cVar3 = (**(code **)(*(int *)this + 0x210))(this);
  if (cVar3 != '\0') {
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5ef,*(undefined2 *)(this + 0xb6),1,&local_25);
    uVar6 = 0;
    if (sVar4 == 0) {
      uVar6 = local_25;
    }
    (**(code **)(**(int **)(this + 0xec) + 0x24))(*(int **)(this + 0xec),uVar6);
  }
  cVar3 = (**(code **)(*(int *)this + 0x214))(this);
  if (cVar3 != '\0') {
    sVar4 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f0,*(undefined2 *)(this + 0xb6),1,&local_25);
    uVar6 = 0;
    if (sVar4 == 0) {
      uVar6 = local_25;
    }
    (**(code **)(**(int **)(this + 0xf0) + 0x24))(*(int **)(this + 0xf0),uVar6);
  }
  return;
}
