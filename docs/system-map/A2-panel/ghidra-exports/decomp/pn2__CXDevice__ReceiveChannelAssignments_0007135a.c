// CXDevice::ReceiveChannelAssignments
// Entry: 0007135a
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ReceiveChannelAssignments(unsigned long, unsigned long, unsigned long, unsigned long,
   unsigned long long) */

void CXDevice::ReceiveChannelAssignments
               (ulong param_1,ulong param_2,ulong param_3,ulong param_4,ulonglong param_5)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint in_stack_0000001c;
  undefined1 local_11;
  
  uVar5 = (uint)param_5;
  for (; param_3 <= param_4; param_3 = param_3 + 1) {
    bVar1 = (byte)uVar5 & 0x1f;
    uVar3 = 1 << ((byte)uVar5 & 0x1f);
    uVar4 = uVar3;
    uVar2 = 0 << bVar1 | 1U >> 0x20 - bVar1;
    if ((uVar5 & 0x20) != 0) {
      uVar4 = 0;
      uVar2 = uVar3;
    }
    local_11 = (uVar4 & param_5._4_4_) != 0 || (uVar2 & in_stack_0000001c) != 0;
    (**(code **)(*(int *)param_1 + 0x50))(param_1,param_2,param_3 & 0xffff,1,&local_11);
    uVar5 = uVar5 + 1;
  }
  return;
}
