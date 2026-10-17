// m_audio_FWb::BaseChannelGroup::SendLevelsToDevice
// Entry: 0001ef64
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::BaseChannelGroup::SendLevelsToDevice(int*) */

void m_audio_FWb::BaseChannelGroup::SendLevelsToDevice(int *param_1)

{
  undefined8 *puVar1;
  uint uVar2;
  ushort *puVar3;
  ulong uVar4;
  ushort *puVar5;
  int iVar6;
  
  uVar2 = (**(code **)(*(long *)param_1 + 0x10))(param_1);
  uVar4 = (ulong)uVar2;
  if (*(long *)(param_1 + 0xe) == 0) {
    puVar3 = (ushort *)AdjustedLevels((BaseChannelGroup *)param_1);
    if (puVar3 == (ushort *)0x0) {
      return;
    }
    if (uVar2 != 0) {
      iVar6 = 1;
      puVar5 = puVar3;
      do {
        puVar1 = *(undefined8 **)(param_1 + 10);
        if (puVar1 != (undefined8 *)0x0) {
          (**(code **)*puVar1)(puVar1,iVar6,*(undefined4 *)puVar5);
        }
        puVar5 = puVar5 + 2;
        iVar6 = iVar6 + 1;
        uVar4 = uVar4 - 1;
      } while (uVar4 != 0);
    }
  }
  else {
    puVar3 = (ushort *)AdjustedAndMultipliedLevels((BaseChannelGroup *)param_1);
    if (puVar3 == (ushort *)0x0) {
      return;
    }
    puVar5 = puVar3;
    if (uVar2 != 0) {
      do {
        *puVar5 = *puVar5 << 8 | *puVar5 >> 8;
        uVar4 = uVar4 - 1;
        puVar5 = puVar5 + 1;
      } while (uVar4 != 0);
    }
    uVar2 = (**(code **)(*(long *)param_1 + 0x10))(param_1);
    (**(code **)**(undefined8 **)(param_1 + 0xe))
              (*(undefined8 **)(param_1 + 0xe),param_1[0x10],uVar2 >> 1,puVar3);
  }
  operator_delete__(puVar3);
  return;
}
