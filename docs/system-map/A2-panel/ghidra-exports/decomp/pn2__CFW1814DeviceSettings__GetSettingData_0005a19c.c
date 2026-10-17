// CFW1814DeviceSettings::GetSettingData
// Entry: 0005a19c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (panel build)

/* CFW1814DeviceSettings::GetSettingData(unsigned long, unsigned short, unsigned long, void*) */

undefined2 __thiscall
CFW1814DeviceSettings::GetSettingData
          (CFW1814DeviceSettings *this,ulong param_1,ushort param_2,ulong param_3,void *param_4)

{
  CFW1814DeviceSettings CVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  
  uVar2 = 1;
  if (0x37 < param_1 - 0x5de) {
    return 1;
  }
  uVar4 = (uint)param_2;
  switch(param_1) {
  case 0x5de:
    uVar2 = (**(code **)(**(int **)(this + 0x68) + 0x20))
                      (*(int **)(this + 0x68),0x5de,param_3,param_4);
    return uVar2;
  case 0x5df:
    uVar2 = (**(code **)(**(int **)(this + 0x68) + 0x20))
                      (*(int **)(this + 0x68),0x5df,param_3,param_4);
    return uVar2;
  case 0x5e0:
    uVar2 = (**(code **)(**(int **)(this + 0x68) + 0x20))
                      (*(int **)(this + 0x68),0x5e0,param_3,param_4);
    return uVar2;
  case 0x5e1:
    uVar2 = (**(code **)(**(int **)(this + 0x68) + 0x20))
                      (*(int **)(this + 0x68),0x5e1,param_3,param_4);
    return uVar2;
  default:
    goto switchD_0005a1d5_caseD_5e2;
  case 0x5e4:
    if (param_3 != 8) {
      return 3;
    }
    *(undefined4 *)param_4 = *(undefined4 *)(this + 0x84);
    *(undefined4 *)((int)param_4 + 4) = 0;
    goto LAB_0005a755;
  case 0x5e5:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 0x88);
    goto LAB_0005a752;
  case 0x5e6:
    if (param_3 != 1) {
      return 3;
    }
    CVar1 = this[0x8a];
    break;
  case 0x5e7:
    if (param_3 != 1) {
      return 3;
    }
    CVar1 = this[0x8b];
    break;
  case 0x5ea:
    if (param_3 != 4) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0x16 < param_2) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + uVar4 * 4 + 0x88);
    goto LAB_0005a741;
  case 0x5eb:
    if (param_3 != 4) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0x12 < param_2) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + uVar4 * 4 + 0x108);
    goto LAB_0005a741;
  case 0x5ec:
    if (param_3 != 4) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0x16 < param_2) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + uVar4 * 4 + 0x188);
    goto LAB_0005a741;
  case 0x5ed:
    if (param_3 != 1) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0xb < param_2) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x20b];
    break;
  case 0x5ee:
    if (param_3 != 1) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0xb < param_2) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x21b];
    break;
  case 0x5ef:
    if (param_3 != 1) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0xb < param_2) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x22b];
    break;
  case 0x5f0:
    if (param_3 != 1) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0xb < param_2) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x23b];
    break;
  case 0x5f1:
    if (param_3 != 2) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (0xb < param_2) {
      return 3;
    }
    iVar5 = 0;
    do {
      *(CFW1814DeviceSettings *)((int)param_4 + iVar5) = this[iVar5 + uVar4 * 2 + 0x24a];
      iVar5 = iVar5 + 1;
    } while ((short)iVar5 != 2);
    return 0;
  case 0x5f4:
    if (param_3 != 4) {
      return 3;
    }
    if ((param_2 == 0 || 4 < param_2) && (5 < (ushort)(param_2 - 0xf))) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + uVar4 * 4 + 0x268);
    goto LAB_0005a741;
  case 0x5f6:
    if (param_3 != 4) {
      return 3;
    }
    if ((param_2 == 0 || 2 < param_2) && (2 < (ushort)(param_2 - 8))) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + uVar4 * 4 + 0x308);
    goto LAB_0005a741;
  case 0x5f7:
    if (param_3 != 1) {
      return 3;
    }
    if ((param_2 == 0 || 2 < param_2) && (param_2 != 10)) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x333];
    break;
  case 0x5f8:
    if (param_3 != 1) {
      return 3;
    }
    if ((param_2 == 0 || 2 < param_2) && (2 < (ushort)(param_2 - 8))) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x33d];
    break;
  case 0x5f9:
    if (param_3 != 1) {
      return 3;
    }
    if (1 < (ushort)(param_2 - 1)) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x347];
    break;
  case 0x5fa:
    if (param_3 != 1) {
      return 3;
    }
    if ((param_2 == 0 || 2 < param_2) && (2 < (ushort)(param_2 - 8))) {
      return 3;
    }
    CVar1 = this[uVar4 + 0x351];
    break;
  case 0x5fb:
    if (param_3 != 2) {
      return 3;
    }
    if (param_2 == 0) {
      return 3;
    }
    if (3 < param_2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + uVar4 * 2 + 0x35a);
    goto LAB_0005a752;
  case 0x5fd:
    if (param_3 != 2) {
      return 3;
    }
    if (param_2 == 9) {
      uVar2 = *(undefined2 *)(this + 0x372);
    }
    else {
      if (param_2 != 8) {
        return 3;
      }
      uVar2 = *(undefined2 *)(this + 0x370);
    }
    goto LAB_0005a752;
  case 0x5fe:
    if (param_3 != 1) {
      return 3;
    }
    CVar1 = this[0x374];
    break;
  case 0x5ff:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 900);
    goto LAB_0005a752;
  case 0x601:
    if (param_3 != 4) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + 0x378);
    goto LAB_0005a741;
  case 0x602:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 0x37c);
    goto LAB_0005a752;
  case 0x605:
    if (param_3 != 4) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + 0x380);
    goto LAB_0005a741;
  case 0x608:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 0x392);
    goto LAB_0005a752;
  case 0x609:
    if (param_3 != 6) {
      return 3;
    }
    iVar5 = 0;
    do {
      *(undefined2 *)((int)param_4 + iVar5 * 2) = *(undefined2 *)(this + iVar5 * 2 + 0x394);
      iVar5 = iVar5 + 1;
    } while ((short)iVar5 != 3);
    return 0;
  case 0x60a:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 0x37e);
    goto LAB_0005a752;
  case 0x60d:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 0x38c);
    goto LAB_0005a752;
  case 0x60e:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 0x38e);
    goto LAB_0005a752;
  case 0x60f:
    if (param_3 != 2) {
      return 3;
    }
    uVar2 = *(undefined2 *)(this + 0x390);
LAB_0005a752:
    *(undefined2 *)param_4 = uVar2;
    goto LAB_0005a755;
  case 0x610:
    if (param_3 != 1) {
      return 3;
    }
    CVar1 = this[0x39a];
    break;
  case 0x611:
    if (param_3 != 1) {
      return 3;
    }
    CVar1 = this[0x39b];
    break;
  case 0x615:
    if (param_3 != 4) {
      return 3;
    }
    uVar3 = *(undefined4 *)(this + 0x388);
LAB_0005a741:
    *(undefined4 *)param_4 = uVar3;
    goto LAB_0005a755;
  }
  *(CFW1814DeviceSettings *)param_4 = CVar1;
LAB_0005a755:
  uVar2 = 0;
switchD_0005a1d5_caseD_5e2:
  return uVar2;
}
