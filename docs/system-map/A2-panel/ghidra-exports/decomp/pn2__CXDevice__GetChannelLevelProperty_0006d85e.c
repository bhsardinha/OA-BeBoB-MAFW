// CXDevice::GetChannelLevelProperty
// Entry: 0006d85e
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::GetChannelLevelProperty(NSString*, unsigned long, void*) */

void __thiscall
CXDevice::GetChannelLevelProperty(CXDevice *this,NSString *param_1,ulong param_2,void *param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int local_84 [23];
  undefined4 local_28;
  int local_24;
  undefined4 *local_20;
  ulong local_1c;
  NSString *local_18;
  CXDevice *local_14;
  
  local_14 = this;
  local_18 = param_1;
  local_1c = param_2;
  local_20 = param_3;
  iVar1 = FetchArrayProperty(this,param_1);
  local_24 = iVar1;
  if (iVar1 == 0) {
    *(undefined4 *)param_3 = 0;
    _NSLog(&cf___nopropertyfound__GetChannelLevelPropkey___channel__lu,param_1,param_2);
  }
  else {
    _objc_exception_try_enter(local_84);
    iVar2 = __setjmp(local_84);
    if (iVar2 == 0) {
      uVar3 = _objc_msgSend(iVar1,PTR_s_objectAtIndex__001d406c,param_2 - 1);
      local_28 = _objc_msgSend(uVar3,PTR_s_longValue_001d4070);
      *local_20 = local_28;
      _objc_exception_try_exit(local_84);
    }
    else {
      uVar3 = _objc_exception_extract(local_84);
      iVar1 = _objc_exception_match(PTR_s_NSException_001d4790,uVar3);
      if (iVar1 == 0) {
        uVar3 = _objc_exception_extract(local_84);
                    /* WARNING: Subroutine does not return */
        _objc_exception_throw(uVar3);
      }
      _NSLog(&cf___accessbeyondarray__GetChannelLevelPropkey___channel__lu,local_18,local_1c);
      *local_20 = 0;
    }
  }
  return;
}
