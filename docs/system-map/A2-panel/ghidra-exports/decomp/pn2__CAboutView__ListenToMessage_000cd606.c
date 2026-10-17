// CAboutView::ListenToMessage
// Entry: 000cd606
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (panel build, About tab)

/* CAboutView::ListenToMessage(long, void*) */

void CAboutView::ListenToMessage(long param_1,void *param_2)

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
  if ((void *)((int)&dylib_command_00001b84.cmdsize + 2) < param_2) {
    if (param_2 == (void *)((int)&dylib_command_00001b84.cmdsize + 3)) {
      (**(code **)(*(int *)(param_1 + 0xe8) + 0x24))(param_1 + 0xe8,0x450,0);
    }
    else if (param_2 == &dylib_command_00001b84.dylib) {
      (**(code **)(*(int *)(param_1 + 0xe8) + 0x24))(param_1 + 0xe8,0x452,0);
    }
    else if (param_2 == (void *)((int)&dylib_command_00001b84.dylib.name.offset + 1)) {
      (**(code **)(*(int *)(param_1 + 0xe8) + 0x24))(param_1 + 0xe8,0x451,0);
    }
    goto switchD_000cd67b_default;
  }
  switch(param_2) {
  case (void *)0x5de:
    break;
  case (void *)0x5df:
    piVar1 = *(int **)(param_1 + 0x11c);
    if (piVar1 == (int *)0x0) {
LAB_000cd7d4:
      local_13c = local_24c;
      LString::Assign((LString *)local_144,local_250 + 1,*local_250);
      _memcpy(local_136,local_246,0x100);
    }
    else {
      sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5df,0,0x20,local_34);
      if (sVar3 != 0) goto LAB_000cd7d4;
      LString::Assign((LString *)local_144,local_34);
    }
    (**(code **)(**(int **)(param_1 + 0x138) + 0x2c))(*(int **)(param_1 + 0x138),local_140);
    goto switchD_000cd67b_default;
  case (void *)0x5e0:
    piVar1 = *(int **)(param_1 + 0x11c);
    if (piVar1 == (int *)0x0) {
LAB_000cd85b:
      local_13c = local_24c;
      LString::Assign((LString *)local_144,local_250 + 1,*local_250);
      _memcpy(local_136,local_246,0x100);
    }
    else {
      sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5e0,0,0x20,local_34);
      if (sVar3 != 0) goto LAB_000cd85b;
      LString::Assign((LString *)local_144,local_34);
    }
    (**(code **)(**(int **)(param_1 + 0x134) + 0x2c))(*(int **)(param_1 + 0x134),local_140);
    goto switchD_000cd67b_default;
  case (void *)0x5e1:
    piVar1 = *(int **)(param_1 + 0x11c);
    if (piVar1 == (int *)0x0) {
LAB_000cd8e2:
      local_13c = local_24c;
      LString::Assign((LString *)local_144,local_250 + 1,*local_250);
      _memcpy(local_136,local_246,0x100);
    }
    else {
      sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5e1,0,0x20,local_34);
      if (sVar3 != 0) goto LAB_000cd8e2;
      LString::Assign((LString *)local_144,local_34);
    }
    (**(code **)(**(int **)(param_1 + 0x13c) + 0x2c))(*(int **)(param_1 + 0x13c),local_140);
  default:
    goto switchD_000cd67b_default;
  }
  piVar1 = *(int **)(param_1 + 0x11c);
  if (piVar1 == (int *)0x0) {
LAB_000cd6b9:
    local_13c = local_24c;
    LString::Assign((LString *)local_144,local_250 + 1,*local_250);
    _memcpy(local_136,local_246,0x100);
  }
  else {
    sVar3 = (**(code **)(*piVar1 + 0xc))(piVar1,0x5de,0,0x20,local_34);
    if (sVar3 != 0) goto LAB_000cd6b9;
    LString::Assign((LString *)local_144,local_34);
  }
  (**(code **)(**(int **)(param_1 + 0x130) + 0x2c))(*(int **)(param_1 + 0x130),local_140);
switchD_000cd67b_default:
  LString::~LString((LString *)local_254);
  LString::~LString((LString *)local_144);
  if (*(int *)puVar2 != local_14) {
                    /* WARNING: Subroutine does not return */
    ___stack_chk_fail();
  }
  return;
}
