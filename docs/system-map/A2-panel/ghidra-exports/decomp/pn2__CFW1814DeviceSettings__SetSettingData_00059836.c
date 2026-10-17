// CFW1814DeviceSettings::SetSettingData
// Entry: 00059836
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (panel build)

/* CFW1814DeviceSettings::SetSettingData(unsigned long, unsigned short, unsigned long, void*, bool&)
    */

undefined4 __thiscall
CFW1814DeviceSettings::SetSettingData
          (CFW1814DeviceSettings *this,ulong param_1,ushort param_2,ulong param_3,void *param_4,
          bool *param_5)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  undefined4 uVar6;
  bool local_4d;
  undefined8 local_4c;
  undefined8 local_44;
  uint local_3c;
  int local_34;
  int local_30;
  ushort local_2a;
  ushort local_28;
  ushort local_26;
  undefined8 local_24;
  undefined8 local_1c;
  uint local_14;
  
  if (0x33 < param_1 - 0x5e2) {
switchD_0005986b_caseD_5e8:
    local_4d = true;
    uVar6 = 1;
    goto switchD_0005986b_caseD_5e3;
  }
  uVar4 = (uint)param_2;
  local_4d = false;
  uVar6 = 0;
  switch(param_1) {
  case 0x5e2:
    local_4d = param_3 == 1;
    goto LAB_00059ab8;
  case 0x5e3:
    goto switchD_0005986b_caseD_5e3;
  case 0x5e4:
    if (param_3 == 8) {
      *(undefined4 *)(this + 0x84) = 0;
      goto switchD_0005986b_caseD_5e3;
    }
    goto LAB_00059f52;
  case 0x5e5:
    local_4d = true;
    if (param_3 == 2) {
      *(undefined2 *)(this + 0x88) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5e6:
    if (param_3 == 1) {
      this[0x8a] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    goto LAB_00059f52;
  case 0x5e7:
    if (param_3 == 1) {
      this[0x8b] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    goto LAB_00059f52;
  default:
    goto switchD_0005986b_caseD_5e8;
  case 0x5e9:
    local_4d = param_3 == 0x80;
    goto LAB_00059ab8;
  case 0x5ea:
    local_4d = true;
    if (((param_3 == 4) && (param_2 != 0)) && (param_2 < 0x17)) {
      *(undefined4 *)(this + uVar4 * 4 + 0x88) = *(undefined4 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5eb:
    local_4d = true;
    if (((param_3 == 4) && (param_2 != 0)) && (param_2 < 0x13)) {
      *(undefined4 *)(this + uVar4 * 4 + 0x108) = *(undefined4 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5ec:
    local_4d = true;
    if (((param_3 == 4) && (param_2 != 0)) && (param_2 < 0x17)) {
      *(undefined4 *)(this + uVar4 * 4 + 0x188) = *(undefined4 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5ed:
    local_4d = true;
    if (((param_3 == 1) && (param_2 != 0)) && (param_2 < 0xc)) {
      this[uVar4 + 0x20b] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5ee:
    local_4d = true;
    if (((param_3 == 1) && (param_2 != 0)) && (param_2 < 0xc)) {
      this[uVar4 + 0x21b] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5ef:
    local_4d = true;
    if (((param_3 == 1) && (param_2 != 0)) && (param_2 < 0xc)) {
      this[uVar4 + 0x22b] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5f0:
    local_4d = true;
    if (((param_3 == 1) && (param_2 != 0)) && (param_2 < 0xc)) {
      this[uVar4 + 0x23b] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5f1:
    local_4d = true;
    if (((param_3 == 2) && (param_2 != 0)) && (param_2 < 0xc)) {
      iVar3 = 0;
      do {
        this[iVar3 + uVar4 * 2 + 0x24a] = *(CFW1814DeviceSettings *)((int)param_4 + iVar3);
        iVar3 = iVar3 + 1;
      } while ((short)iVar3 != 2);
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5f3:
    local_4d = param_3 == 0x50;
LAB_00059ab8:
    uVar6 = 3;
    if (local_4d) {
      uVar6 = 0;
    }
    local_4d = !local_4d;
    goto switchD_0005986b_caseD_5e3;
  case 0x5f4:
    local_4d = true;
    if ((param_3 == 4) && ((param_2 != 0 && param_2 < 5 || ((ushort)(param_2 - 0xf) < 6)))) {
      local_14 = DAT_000f8720;
      local_1c = DAT_000f8718;
      local_24 = DAT_000f8710;
      uVar1 = *(undefined4 *)param_4;
      *(undefined4 *)(this + uVar4 * 4 + 0x268) = uVar1;
      *(undefined4 *)(this + uVar4 * 4 + 0x2b8) = uVar1;
      (**(code **)(*(int *)this + 0x28))(this,uVar4,&local_26);
      uVar1 = CalculateOutputPairBalanceLevel(this,local_26);
      *(undefined4 *)(this + (uint)local_26 * 4 + 0x308) = uVar1;
      local_1c = CONCAT44(this + (uint)local_26 * 4 + 0x308,(undefined4)local_1c);
      local_14 = local_14 & 0xffff0000;
      local_24._0_6_ = CONCAT24(local_26,(undefined4)local_24);
      LBroadcaster::BroadcastMessage((LBroadcaster *)(this + 0x30),0x5f6,&local_24);
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5f6:
    local_4d = true;
    if ((param_3 == 4) && ((param_2 != 0 && param_2 < 3 || ((ushort)(param_2 - 8) < 3)))) {
      local_3c = DAT_000f8734;
      local_44 = DAT_000f872c;
      local_4c = DAT_000f8724;
      *(undefined4 *)(this + uVar4 * 4 + 0x308) = *(undefined4 *)param_4;
      (**(code **)(*(int *)this + 0x24))(this,uVar4,&local_28,&local_2a);
      uVar2 = (uint)local_28;
      local_34 = *(int *)(this + (uint)local_2a * 4 + 0x2b8);
      if (*(int *)(this + (uint)local_2a * 4 + 0x2b8) <= *(int *)(this + uVar2 * 4 + 0x2b8)) {
        local_34 = *(int *)(this + uVar2 * 4 + 0x2b8);
      }
      iVar3 = *(int *)(this + uVar4 * 4 + 0x308);
      iVar5 = (*(int *)(this + 0x3f4) - *(int *)(this + 0x3f0)) / 2 + *(int *)(this + 0x3f0);
      local_30 = local_34;
      if (iVar3 < iVar5) {
        local_34 = (iVar3 - iVar5) + local_34;
        if (local_34 < *(int *)(this + 0x3e4)) {
          local_34 = *(int *)(this + 0x3e4);
        }
      }
      else if (iVar3 - iVar5 != 0 && iVar5 <= iVar3) {
        local_30 = (iVar5 - iVar3) + local_34;
        if (local_30 < *(int *)(this + 0x3e4)) {
          local_30 = *(int *)(this + 0x3e4);
        }
      }
      *(int *)(this + uVar2 * 4 + 0x268) = local_30;
      *(int *)(this + (uint)local_2a * 4 + 0x268) = local_34;
      (**(code **)(**(int **)(this + 0x68) + 0x2c))(*(int **)(this + 0x68),0x5f4,uVar2,4,&local_30);
      (**(code **)(**(int **)(this + 0x68) + 0x2c))
                (*(int **)(this + 0x68),0x5f4,local_2a,4,&local_34);
      local_44 = CONCAT44(&local_30,4);
      local_3c = local_3c & 0xffff0000;
      local_4c._0_6_ = CONCAT24(local_28,(undefined4)local_4c);
      LBroadcaster::BroadcastMessage((LBroadcaster *)(this + 0x30),0x5f4,&local_4c);
      local_44 = CONCAT44(&local_34,4);
      local_3c = local_3c & 0xffff0000;
      local_4c._0_6_ = CONCAT24(local_2a,(undefined4)local_4c);
      LBroadcaster::BroadcastMessage((LBroadcaster *)(this + 0x30),0x5f4,&local_4c);
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5f7:
    local_4d = true;
    if ((param_3 == 1) && ((param_2 != 0 && param_2 < 3 || (param_2 == 10)))) {
      this[uVar4 + 0x333] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5f8:
    local_4d = true;
    if ((param_3 == 1) && ((param_2 != 0 && param_2 < 3 || ((ushort)(param_2 - 8) < 3)))) {
      this[uVar4 + 0x33d] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5f9:
    local_4d = true;
    if ((param_3 == 1) && ((ushort)(param_2 - 1) < 2)) {
      this[uVar4 + 0x347] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5fa:
    local_4d = true;
    if ((param_3 == 1) && ((param_2 != 0 && param_2 < 3 || ((ushort)(param_2 - 8) < 3)))) {
      this[uVar4 + 0x351] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5fb:
    local_4d = true;
    if (((param_3 == 2) && (param_2 != 0)) && (param_2 < 4)) {
      *(undefined2 *)(this + uVar4 * 2 + 0x35a) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5fd:
    local_4d = true;
    if ((param_3 == 2) && ((ushort)(param_2 - 8) < 2)) {
      if (param_2 == 8) {
        *(undefined2 *)(this + 0x370) = *(undefined2 *)param_4;
      }
      else {
        *(undefined2 *)(this + 0x372) = *(undefined2 *)param_4;
      }
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5fe:
    local_4d = true;
    if (param_3 == 1) {
      this[0x374] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x5ff:
    local_4d = true;
    if (param_3 == 2) {
      *(undefined2 *)(this + 900) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x601:
    local_4d = true;
    if (param_3 == 4) {
      *(undefined4 *)(this + 0x378) = *(undefined4 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x602:
    if (param_3 == 2) {
      *(undefined2 *)(this + 0x37c) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    goto LAB_00059f52;
  case 0x605:
    local_4d = true;
    if (param_3 == 4) {
      *(undefined4 *)(this + 0x380) = *(undefined4 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x608:
    local_4d = true;
    if (param_3 == 2) {
      *(undefined2 *)(this + 0x392) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x609:
    local_4d = true;
    if (param_3 == 6) {
      iVar3 = 0;
      do {
        *(undefined2 *)(this + iVar3 * 2 + 0x394) = *(undefined2 *)((int)param_4 + iVar3 * 2);
        iVar3 = iVar3 + 1;
      } while ((short)iVar3 != 3);
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x60a:
    local_4d = true;
    if (param_3 == 2) {
      *(undefined2 *)(this + 0x37e) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x60d:
    local_4d = true;
    if (param_3 == 2) {
      *(undefined2 *)(this + 0x38c) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x60e:
    local_4d = true;
    if (param_3 == 2) {
      *(undefined2 *)(this + 0x38e) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x60f:
    local_4d = true;
    if (param_3 == 2) {
      *(undefined2 *)(this + 0x390) = *(undefined2 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x610:
    local_4d = true;
    if (param_3 == 1) {
      this[0x39a] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x611:
    local_4d = true;
    if (param_3 == 1) {
      this[0x39b] = *(CFW1814DeviceSettings *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
    break;
  case 0x615:
    if (param_3 == 4) {
      *(undefined4 *)(this + 0x388) = *(undefined4 *)param_4;
      goto switchD_0005986b_caseD_5e3;
    }
LAB_00059f52:
    uVar6 = 3;
    local_4d = true;
    goto switchD_0005986b_caseD_5e3;
  }
  local_4d = true;
  uVar6 = 3;
switchD_0005986b_caseD_5e3:
  *param_5 = local_4d;
  return uVar6;
}
