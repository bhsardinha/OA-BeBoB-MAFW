// CXDevice::ReceiveBoolArrayChange
// Entry: 000710a0
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ReceiveBoolArrayChange(unsigned long, NSString*, objc_object*, unsigned long) */

void __thiscall
CXDevice::ReceiveBoolArrayChange
          (CXDevice *this,ulong param_1,NSString *param_2,objc_object *param_3,ulong param_4)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined1 local_11;
  
  iVar2 = _objc_msgSend(param_3,PTR_s_count_001d4080);
  if (iVar2 != 0) {
    iVar4 = 0;
    do {
      uVar3 = _objc_msgSend(param_3,PTR_s_objectAtIndex__001d406c,iVar4);
      cVar1 = _objc_msgSend(uVar3,PTR_s_boolValue_001d40ac);
      local_11 = cVar1 != '\0';
      (**(code **)(*(int *)this + 0x50))(this,param_1,param_4 + iVar4 & 0xffff,1,&local_11);
      iVar4 = iVar4 + 1;
    } while (iVar2 != iVar4);
  }
  return;
}
