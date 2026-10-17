// CXDevice::ReceiveRotaryControlAssignments
// Entry: 000713d8
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ReceiveRotaryControlAssignments(unsigned long, NSString*, objc_object*, unsigned long)
    */

void CXDevice::ReceiveRotaryControlAssignments
               (ulong param_1,NSString *param_2,objc_object *param_3,ulong param_4)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  
  uVar1 = (**(code **)(*(int *)param_1 + 0x18))(param_1);
  iVar4 = (**(code **)(*(int *)param_1 + 0x60))(param_1);
  uVar2 = (**(code **)(*(int *)param_1 + 0x5c))(param_1);
  uVar3 = (**(code **)(*(int *)param_1 + 0x1c))(param_1);
  iVar5 = (**(code **)(*(int *)param_1 + 0x5c))(param_1);
  uVar6 = _objc_msgSend(param_4,PTR_s_unsignedLongLongValue_001d405c);
  ReceiveChannelAssignments
            (param_1,0x5f7,1,((uVar1 - 2) - iVar4 & 0xfffe) >> 1,CONCAT44(uVar6,0x20));
  uVar7 = (uint)(uVar2 >> 1);
  ReceiveChannelAssignments(param_1,0x5ed,1,uVar7,CONCAT44(uVar6,0x10));
  ReceiveChannelAssignments
            (param_1,0x5ed,uVar7 + 1,(((uint)uVar3 - iVar5 & 0xfffe) >> 1) + uVar7,
             (ulonglong)uVar6 << 0x20);
  return;
}
