// com_m_audio_FW1814Device::SetClockSourceInternal
// Entry: 0000e25c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SetClockSourceInternal(unsigned int, bool) */

int com_m_audio_FW1814Device::SetClockSourceInternal(uint param_1,bool param_2)

{
  long *plVar1;
  int iVar2;
  undefined1 *puVar3;
  char in_DL;
  undefined7 in_register_00000031;
  ulong uVar4;
  undefined4 in_register_0000003c;
  long *plVar5;
  undefined8 in_R9;
  uint uVar6;
  uint uVar7;
  undefined1 *puVar8;
  undefined4 uVar9;
  undefined1 local_6c [20];
  undefined1 *local_58;
  uint local_50;
  undefined1 local_4c [24];
  undefined4 local_34;
  
  plVar5 = (long *)CONCAT44(in_register_0000003c,param_1);
  uVar4 = CONCAT71(in_register_00000031,param_2);
  local_50 = 0x10;
  local_58 = local_4c;
  _bzero(local_58,0x10);
  uVar9 = 0x10;
  puVar8 = local_6c;
  _bzero(puVar8,0x10);
  plVar1 = (long *)plVar5[0x1d];
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0xa50))(plVar1);
  }
  if ((char)plVar5[0x371] == '\0') {
    (**(code **)(*(long *)plVar5[0x1d] + 0xb00))();
    *(undefined1 *)(plVar5 + 0x371) = 1;
    uVar7 = (uint)uVar4;
    if (uVar7 < 0x40000000) {
      iVar2 = -0x1ffffd3e;
    }
    else {
      *local_58 = 0;
      puVar3 = local_58;
      if (1 < local_50) {
        puVar3 = local_58 + 1;
      }
      *puVar3 = 0xff;
      puVar3 = local_58;
      if (2 < local_50) {
        puVar3 = local_58 + 2;
      }
      *puVar3 = 0;
      puVar3 = local_58;
      if (3 < local_50) {
        puVar3 = local_58 + 3;
      }
      *puVar3 = 4;
      puVar3 = local_58;
      if (4 < local_50) {
        puVar3 = local_58 + 4;
      }
      *puVar3 = 0;
      puVar3 = local_58;
      if (5 < local_50) {
        puVar3 = local_58 + 5;
      }
      *puVar3 = 4;
      uVar6 = uVar7 & 3;
      if (uVar6 == 1) {
        puVar3 = local_58;
        if (6 < local_50) {
          puVar3 = local_58 + 6;
        }
        *puVar3 = 1;
      }
      else if (uVar6 == 2) {
        puVar3 = local_58;
        if (6 < local_50) {
          puVar3 = local_58 + 6;
        }
        *puVar3 = 2;
      }
      else if (uVar6 == 3) {
        puVar3 = local_58;
        if (6 < local_50) {
          puVar3 = local_58 + 6;
        }
        *puVar3 = 0;
      }
      else {
        puVar3 = local_58;
        if (6 < local_50) {
          puVar3 = local_58 + 6;
        }
        *puVar3 = 3;
      }
      puVar3 = local_58;
      if (7 < local_50) {
        puVar3 = local_58 + 7;
      }
      *puVar3 = 0;
      puVar3 = local_58;
      if (8 < local_50) {
        puVar3 = local_58 + 8;
      }
      *puVar3 = 0;
      puVar3 = local_58;
      if (9 < local_50) {
        puVar3 = local_58 + 9;
      }
      *puVar3 = 0;
      local_34 = 0x10;
      (**(code **)(*(long *)plVar5[0x4d] + 0x8b8))
                ((long *)plVar5[0x4d],local_58,0x10,puVar8,&local_34,in_R9,puVar8,uVar9);
      _IOSleep(300);
      *(uint *)((long)plVar5 + 0x144) = uVar7;
      iVar2 = (*(code *)PTR_SetClockSourceInternal_0003c200)(plVar5,uVar4 & 0xffffffff,in_DL);
      if (iVar2 == 0) {
        _IOSleep(300);
        if (in_DL == '\0') {
          *(uint *)((long)plVar5 + 0x144) = uVar7;
          (**(code **)(*plVar5 + 0xbe0))(plVar5,uVar4 & 0xffffffff);
          FWSettingsLevels::SendToDevice((FWSettingsLevels *)(plVar5 + 0xa7));
        }
        *(undefined1 *)(plVar5 + 0x371) = 0;
        (**(code **)(*(long *)plVar5[0x1d] + 0xb08))();
        goto LAB_0000e4cb;
      }
    }
    *(undefined1 *)(plVar5 + 0x371) = 0;
    (**(code **)(*(long *)plVar5[0x1d] + 0xb10))();
  }
  else {
    iVar2 = 0;
  }
LAB_0000e4cb:
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0xa58))(plVar1);
  }
  return iVar2;
}
