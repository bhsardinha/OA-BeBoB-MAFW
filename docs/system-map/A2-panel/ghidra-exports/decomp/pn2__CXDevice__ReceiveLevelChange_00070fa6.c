// CXDevice::ReceiveLevelChange
// Entry: 00070fa6
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ReceiveLevelChange(unsigned long, NSString*, objc_object*, unsigned long, bool) */

void __thiscall
CXDevice::ReceiveLevelChange
          (CXDevice *this,ulong param_1,NSString *param_2,objc_object *param_3,ulong param_4,
          bool param_5)

{
  short sVar1;
  ushort uVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  int local_14;
  
  iVar3 = _objc_msgSend(param_3,PTR_s_count_001d4080);
  if (iVar3 != 0) {
    iVar7 = 0;
    do {
      uVar4 = _objc_msgSend(param_3,PTR_s_objectAtIndex__001d406c,iVar7);
      sVar1 = _objc_msgSend(uVar4,PTR_s_longValue_001d4070);
      local_14 = (int)sVar1;
      uVar8 = iVar7 + param_4;
      if (param_5) {
        uVar2 = (**(code **)(*(int *)this + 0x1c))(this);
        iVar5 = (**(code **)(*(int *)this + 0x5c))(this);
        if (((uint)uVar2 - iVar5 & 0xffff) < uVar8) {
          uVar2 = (**(code **)(*(int *)this + 0x1c))(this);
          iVar5 = (**(code **)(*(int *)this + 0x5c))(this);
          uVar6 = -((uint)uVar2 - iVar5 & 0xffff);
        }
        else {
          uVar6 = (**(code **)(*(int *)this + 0x5c))(this);
          uVar6 = uVar6 & 0xffff;
        }
        uVar8 = uVar8 + uVar6;
      }
      (**(code **)(*(int *)this + 0x50))(this,param_1,uVar8 & 0xffff,4,&local_14);
      iVar7 = iVar7 + 1;
    } while (iVar3 != iVar7);
  }
  return;
}
