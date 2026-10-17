// COutputChannelPairView::AdaptUIElementsToSettings
// Entry: 000e108c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* COutputChannelPairView::AdaptUIElementsToSettings() */

void __thiscall COutputChannelPairView::AdaptUIElementsToSettings(COutputChannelPairView *this)

{
  undefined2 uVar1;
  int iVar2;
  int *piVar3;
  char cVar4;
  short sVar5;
  undefined4 uVar6;
  uint uVar7;
  uint uVar8;
  byte bVar9;
  bool bVar10;
  byte local_25;
  undefined4 local_24;
  undefined4 local_20;
  float local_1c;
  short local_16;
  undefined4 local_14;
  
  local_14 = 0;
  local_16 = 0;
  local_25 = 0;
  cVar4 = (**(code **)(*(int *)this + 0x1f8))(this);
  sVar5 = 0;
  if (cVar4 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5f4,&local_24);
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f4,*(undefined2 *)(this + 0xb8),4,&local_14);
    (**(code **)(**(int **)(this + 0xc4) + 0x1dc))(*(int **)(this + 0xc4),local_24,local_20);
    uVar6 = local_14;
    if (sVar5 != 0) {
      uVar6 = 0;
    }
    (**(code **)(**(int **)(this + 0xc4) + 0x1d8))(*(int **)(this + 0xc4),uVar6);
    uVar1 = *(undefined2 *)(this + 0xb8);
    iVar2 = *(int *)(this + 0xc4);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5f4;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f4,*(undefined2 *)(this + 0xba),4,&local_14);
    (**(code **)(**(int **)(this + 0xd0) + 0x1dc))(*(int **)(this + 0xd0),local_24,local_20);
    uVar6 = 0;
    if (sVar5 == 0) {
      uVar6 = local_14;
    }
    (**(code **)(**(int **)(this + 0xd0) + 0x1d8))(*(int **)(this + 0xd0),uVar6);
    uVar1 = *(undefined2 *)(this + 0xba);
    iVar2 = *(int *)(this + 0xd0);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5f4;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
    cVar4 = (**(code **)(*(int *)this + 0x20c))(this);
    if (cVar4 == '\0') {
      *(undefined1 *)(*(int *)(this + 0xc4) + 0x90) = 0;
      *(undefined1 *)(*(int *)(this + 0xd0) + 0x90) = 0;
    }
    else {
      sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                        (*(int **)(this + 0xc0),0x5f8,*(undefined2 *)(this + 0xb6),1,&local_25);
      if (sVar5 != 0) {
        local_25 = 0;
      }
      (**(code **)(**(int **)(this + 0xe4) + 0x24))(*(int **)(this + 0xe4),local_25);
      *(byte *)(*(int *)(this + 0xc4) + 0x90) = local_25;
      *(byte *)(*(int *)(this + 0xd0) + 0x90) = local_25;
    }
  }
  cVar4 = (**(code **)(*(int *)this + 0x1fc))(this);
  if (cVar4 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5f3,&local_24);
    (**(code **)(**(int **)(this + 200) + 0x1e8))(*(int **)(this + 200),local_24,local_20);
    (**(code **)(**(int **)(this + 200) + 0x1e4))(*(int **)(this + 200),local_24);
    (**(code **)(**(int **)(this + 0xd4) + 0x1e8))(*(int **)(this + 0xd4),local_24,local_20);
    (**(code **)(**(int **)(this + 0xd4) + 0x1e4))(*(int **)(this + 0xd4),local_24);
  }
  cVar4 = (**(code **)(*(int *)this + 0x200))(this);
  if (cVar4 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5f5,&local_24);
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f5,*(undefined2 *)(this + 0xb8),4,&local_14);
    (**(code **)(**(int **)(this + 0xcc) + 0x1dc))(*(int **)(this + 0xcc),local_24,local_20);
    CMABalanceKnob::SetDisplayValueMultiplier(*(CMABalanceKnob **)(this + 0xcc),local_1c);
    uVar6 = local_14;
    if (sVar5 != 0) {
      uVar6 = 0;
    }
    (**(code **)(**(int **)(this + 0xcc) + 0x1d8))(*(int **)(this + 0xcc),uVar6);
    uVar1 = *(undefined2 *)(this + 0xb8);
    iVar2 = *(int *)(this + 0xcc);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5f5;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f5,*(undefined2 *)(this + 0xba),4,&local_14);
    (**(code **)(**(int **)(this + 0xd8) + 0x1dc))(*(int **)(this + 0xd8),local_24,local_20);
    CMABalanceKnob::SetDisplayValueMultiplier(*(CMABalanceKnob **)(this + 0xd8),local_1c);
    uVar6 = 0;
    if (sVar5 == 0) {
      uVar6 = local_14;
    }
    (**(code **)(**(int **)(this + 0xd8) + 0x1d8))(*(int **)(this + 0xd8),uVar6);
    uVar1 = *(undefined2 *)(this + 0xba);
    iVar2 = *(int *)(this + 0xd8);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5f5;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
  }
  cVar4 = (**(code **)(*(int *)this + 0x204))(this);
  if (cVar4 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0x10))(*(int **)(this + 0xc0),0x5f6,&local_24);
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f6,*(undefined2 *)(this + 0xb6),4,&local_14);
    (**(code **)(**(int **)(this + 0xdc) + 0x1dc))(*(int **)(this + 0xdc),local_24,local_20);
    CMABalanceKnob::SetDisplayValueMultiplier(*(CMABalanceKnob **)(this + 0xdc),local_1c);
    uVar6 = 0;
    if (sVar5 == 0) {
      uVar6 = local_14;
    }
    (**(code **)(**(int **)(this + 0xdc) + 0x1d8))(*(int **)(this + 0xdc),uVar6);
    uVar1 = *(undefined2 *)(this + 0xb6);
    iVar2 = *(int *)(this + 0xdc);
    *(undefined4 *)(iVar2 + 0x84) = *(undefined4 *)(this + 0xc0);
    *(undefined4 *)(iVar2 + 0x88) = 0x5f6;
    *(undefined1 *)(iVar2 + 0x8c) = 1;
    *(undefined2 *)(iVar2 + 0x8e) = uVar1;
  }
  cVar4 = (**(code **)(*(int *)this + 0x208))(this);
  if (cVar4 != '\0') {
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))(*(int **)(this + 0xc0),0x5e5,0,2,&local_16)
    ;
    if ((sVar5 == 0) &&
       (sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0x2c))
                          (*(int **)(this + 0xc0),*(undefined2 *)(this + 0xb6)), sVar5 == local_16))
    {
      cVar4 = LPane::IsEnabled(*(LPane **)(this + 0xe0));
      (**(code **)(**(int **)(this + 0xe0) + 200))(*(int **)(this + 0xe0));
      bVar10 = cVar4 == '\0';
    }
    else {
      cVar4 = LPane::IsEnabled(*(LPane **)(this + 0xe0));
      (**(code **)(**(int **)(this + 0xe0) + 0xcc))(*(int **)(this + 0xe0));
      bVar10 = cVar4 != '\0';
    }
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f7,*(undefined2 *)(this + 0xb6),1,&local_25);
    if (sVar5 != 0) {
      local_25 = 0;
    }
    if (bVar10) {
      uVar8 = (uint)local_25;
    }
    else {
      uVar7 = (**(code **)(**(int **)(this + 0xe0) + 0x20))(*(int **)(this + 0xe0));
      uVar8 = (uint)local_25;
      bVar10 = uVar7 != uVar8;
    }
    piVar3 = *(int **)(this + 0xe0);
    (**(code **)(*piVar3 + 0x24))(piVar3,uVar8);
    if (bVar10) {
      piVar3 = *(int **)(this + 0xe0);
      (**(code **)(*piVar3 + 0xd0))(piVar3);
    }
  }
  cVar4 = (**(code **)(*(int *)this + 0x210))(this);
  if (cVar4 != '\0') {
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5f9,*(undefined2 *)(this + 0xb6),1,&local_25);
    bVar9 = 0;
    if (sVar5 == 0) {
      bVar9 = local_25;
    }
    (**(code **)(**(int **)(this + 0xe8) + 0x24))(*(int **)(this + 0xe8),bVar9);
  }
  cVar4 = (**(code **)(*(int *)this + 0x214))(this);
  if (cVar4 != '\0') {
    sVar5 = (**(code **)(**(int **)(this + 0xc0) + 0xc))
                      (*(int **)(this + 0xc0),0x5fa,*(undefined2 *)(this + 0xb6),1,&local_25);
    bVar9 = 0;
    if (sVar5 == 0) {
      bVar9 = local_25;
    }
    (**(code **)(**(int **)(this + 0xec) + 0x24))(*(int **)(this + 0xec),bVar9);
  }
  cVar4 = (**(code **)(*(int *)this + 0x218))(this);
  if (cVar4 != '\0') {
    (**(code **)(**(int **)(this + 0xc0) + 0xc))
              (*(int **)(this + 0xc0),0x5fb,*(undefined2 *)(this + 0xb6),2,&local_16);
    (**(code **)(**(int **)(this + 0xf0) + 0x24))
              (*(int **)(this + 0xf0),sVar5 == 0 && local_16 == 1);
  }
  return;
}
