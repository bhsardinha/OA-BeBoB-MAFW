// CAboutView::AdaptUIElementsToSettings
// Entry: 000cda5a
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (panel build, About tab)

/* CAboutView::AdaptUIElementsToSettings() */

void __thiscall CAboutView::AdaptUIElementsToSettings(CAboutView *this)

{
  int *piVar1;
  undefined *puVar2;
  short sVar3;
  LStr255 local_254 [4];
  uchar *local_250;
  undefined4 local_24c;
  undefined1 local_246 [258];
  LStr255 local_144 [4];
  undefined4 local_140;
  undefined4 local_13c;
  undefined1 local_136 [258];
  uchar local_34 [32];
  int local_14;
  
  puVar2 = PTR____stack_chk_guard_00163028;
  local_14 = *(int *)PTR____stack_chk_guard_00163028;
  ::LStr255::LStr255(local_144);
  ::LStr255::LStr255(local_254,0x3e9,1);
  piVar1 = *(int **)(this + 0x11c);
  if (piVar1 == (int *)0x0) {
LAB_000cdae4:
    local_13c = local_24c;
    LString::Assign((LString *)local_144,local_250 + 1,*local_250);
    _memcpy(local_136,local_246,0x100);
  }
  else {
    sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5de,0,0x20,local_34);
    if (sVar3 != 0) goto LAB_000cdae4;
    LString::Assign((LString *)local_144,local_34);
  }
  (**(code **)(**(int **)(this + 0x130) + 0x2c))(*(int **)(this + 0x130),local_140);
  piVar1 = *(int **)(this + 0x11c);
  if (piVar1 == (int *)0x0) {
LAB_000cdb8a:
    local_13c = local_24c;
    LString::Assign((LString *)local_144,local_250 + 1,*local_250);
    _memcpy(local_136,local_246,0x100);
  }
  else {
    sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5e0,0,0x20,local_34);
    if (sVar3 != 0) goto LAB_000cdb8a;
    LString::Assign((LString *)local_144,local_34);
  }
  (**(code **)(**(int **)(this + 0x134) + 0x2c))(*(int **)(this + 0x134),local_140);
  piVar1 = *(int **)(this + 0x11c);
  if (piVar1 == (int *)0x0) {
LAB_000cdc30:
    local_13c = local_24c;
    LString::Assign((LString *)local_144,local_250 + 1,*local_250);
    _memcpy(local_136,local_246,0x100);
  }
  else {
    sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5df,0,0x20,local_34);
    if (sVar3 != 0) goto LAB_000cdc30;
    LString::Assign((LString *)local_144,local_34);
  }
  (**(code **)(**(int **)(this + 0x138) + 0x2c))(*(int **)(this + 0x138),local_140);
  piVar1 = *(int **)(this + 0x11c);
  if (piVar1 != (int *)0x0) {
    sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5e1,0,0x20,local_34);
    if (sVar3 == 0) {
      LString::Assign((LString *)local_144,local_34);
      goto LAB_000cdd2a;
    }
  }
  local_13c = local_24c;
  LString::Assign((LString *)local_144,local_250 + 1,*local_250);
  _memcpy(local_136,local_246,0x100);
LAB_000cdd2a:
  (**(code **)(**(int **)(this + 0x13c) + 0x2c))(*(int **)(this + 0x13c),local_140);
  LString::~LString((LString *)local_254);
  LString::~LString((LString *)local_144);
  if (*(int *)puVar2 == local_14) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  ___stack_chk_fail();
}
