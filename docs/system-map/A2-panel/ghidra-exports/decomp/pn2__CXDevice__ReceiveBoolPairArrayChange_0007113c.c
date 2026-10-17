// CXDevice::ReceiveBoolPairArrayChange
// Entry: 0007113c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ReceiveBoolPairArrayChange(unsigned long, NSString*, objc_object*, unsigned long) */

void __thiscall
CXDevice::ReceiveBoolPairArrayChange
          (CXDevice *this,ulong param_1,NSString *param_2,objc_object *param_3,ulong param_4)

{
  char cVar1;
  char cVar2;
  uint uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int iVar6;
  uint uVar7;
  undefined1 local_11;
  
  uVar3 = _objc_msgSend(param_3,PTR_s_count_001d4080);
  if (uVar3 >> 1 != 0) {
    uVar7 = 0;
    iVar6 = 1;
    do {
      uVar4 = _objc_msgSend(param_3,PTR_s_objectAtIndex__001d406c,iVar6 + -1);
      uVar5 = _objc_msgSend(param_3,PTR_s_objectAtIndex__001d406c,iVar6);
      cVar1 = _objc_msgSend(uVar4,PTR_s_boolValue_001d40ac);
      cVar2 = _objc_msgSend(uVar5,PTR_s_boolValue_001d40ac);
      local_11 = cVar2 != '\0' || cVar1 != '\0';
      (**(code **)(*(int *)this + 0x50))(this,param_1,param_4 + uVar7 & 0xffff,1,&local_11);
      iVar6 = iVar6 + 2;
      uVar7 = uVar7 + 1;
    } while (uVar7 < uVar3 >> 1);
  }
  return;
}
