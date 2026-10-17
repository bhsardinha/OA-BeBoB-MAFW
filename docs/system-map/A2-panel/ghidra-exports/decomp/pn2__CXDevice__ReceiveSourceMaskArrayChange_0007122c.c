// CXDevice::ReceiveSourceMaskArrayChange
// Entry: 0007122c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ReceiveSourceMaskArrayChange(unsigned long, NSString*, objc_object*, unsigned long) */

void __thiscall
CXDevice::ReceiveSourceMaskArrayChange
          (CXDevice *this,ulong param_1,NSString *param_2,objc_object *param_3,ulong param_4)

{
  int iVar1;
  code *pcVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  uint uVar6;
  undefined1 *puVar7;
  uint uVar8;
  int iVar9;
  undefined1 auStack_3c [16];
  int *local_2c;
  uint local_28;
  int local_24;
  int local_20;
  undefined1 *local_1c;
  uint local_18;
  int local_14;
  
  local_20 = 0x7123a;
  local_2c = (int *)PTR____stack_chk_guard_00163028;
  local_14 = *(int *)PTR____stack_chk_guard_00163028;
  iVar3 = _objc_msgSend(param_3,PTR_s_count_001d4080);
  local_24 = iVar3;
  uVar4 = (**(code **)(*(int *)this + 100))(this);
  puVar7 = auStack_3c;
  if (iVar3 != 0) {
    uVar8 = (uVar4 & 0xfffe) >> 1;
    local_28 = (uVar4 & 0xfffe) >> 1;
    iVar3 = 0;
    puVar7 = auStack_3c;
    local_18 = uVar8;
    do {
      iVar1 = local_20;
      uVar5 = *(undefined4 *)(&UNK_00162e32 + local_20);
      *(int *)(puVar7 + -8) = iVar3;
      *(undefined4 *)(puVar7 + -0xc) = uVar5;
      *(objc_object **)(puVar7 + -0x10) = param_3;
      *(undefined4 *)(puVar7 + -0x14) = 0x712bd;
      uVar5 = _objc_msgSend();
      uVar4 = local_28;
      iVar9 = (int)puVar7 - (uVar8 + 0xf & 0xfffffff0);
      local_1c = puVar7;
      *(undefined4 *)(iVar9 + -0xc) = *(undefined4 *)(&UNK_00162e1e + iVar1);
      *(undefined4 *)(iVar9 + -0x10) = uVar5;
      *(undefined4 *)(iVar9 + -0x14) = 0x712e7;
      uVar8 = _objc_msgSend();
      uVar6 = 0;
      if ((short)local_18 != 0) {
        do {
          *(bool *)(iVar9 + uVar6) = (uVar8 >> (uVar6 & 0x1f) & 1) != 0;
          uVar6 = uVar6 + 1;
        } while (uVar4 != uVar6);
      }
      iVar1 = *(int *)this;
      *(int *)(iVar9 + -0x10) = iVar9;
      uVar8 = local_18;
      *(uint *)(iVar9 + -0x14) = local_18;
      *(ulong *)(iVar9 + -0x18) = iVar3 + param_4 & 0xffff;
      *(ulong *)(iVar9 + -0x1c) = param_1;
      *(CXDevice **)(iVar9 + -0x20) = this;
      pcVar2 = *(code **)(iVar1 + 0x50);
      *(undefined4 *)(iVar9 + -0x24) = 0x71332;
      (*pcVar2)();
      iVar3 = iVar3 + 1;
      puVar7 = local_1c;
    } while (iVar3 != local_24);
  }
  if (*local_2c == local_14) {
    return;
  }
                    /* WARNING: Subroutine does not return */
  *(undefined **)(puVar7 + -4) = &UNK_00071359;
  ___stack_chk_fail();
}
