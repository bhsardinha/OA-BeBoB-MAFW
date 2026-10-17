// CXDevice::SetChannelLevelProperty
// Entry: 0006d9c2
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::SetChannelLevelProperty(NSString*, void*, unsigned long, ...) */

void __thiscall
CXDevice::SetChannelLevelProperty(CXDevice *this,NSString *param_1,void *param_2,ulong param_3,...)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  int *local_88;
  int local_84 [23];
  int local_28;
  undefined4 local_24;
  ulong local_20;
  void *local_1c;
  NSString *local_18;
  CXDevice *local_14;
  
  local_14 = this;
  local_18 = param_1;
  local_1c = param_2;
  local_20 = param_3;
  uVar2 = *(undefined4 *)param_2;
  local_24 = uVar2;
  local_28 = FetchArrayProperty(this,param_1);
  if (local_28 == 0) {
    _NSLog(&cf_Nocachedlevels);
  }
  else {
    _objc_exception_try_enter(local_84);
    iVar1 = __setjmp(local_84);
    if (iVar1 == 0) {
      uVar2 = _objc_msgSend(PTR_s_NSNumber_001d478c,PTR_s_numberWithLong__001d4074,uVar2);
      if (local_20 != 0) {
        uVar3 = 0;
        local_88 = (int *)&stack0x00000014;
        do {
          _objc_msgSend(local_28,PTR_s_replaceObjectAtIndex_withObject__001d4078,*local_88 + -1,
                        uVar2);
          uVar3 = uVar3 + 1;
          local_88 = local_88 + 1;
        } while (uVar3 < local_20);
      }
      _objc_msgSend(*(undefined4 *)(this + 0x70),PTR_s_addObject__001d407c,local_18);
      _objc_exception_try_exit(local_84);
    }
    else {
      uVar2 = _objc_exception_extract(local_84);
      iVar1 = _objc_exception_match(PTR_s_NSException_001d4790,uVar2);
      if (iVar1 == 0) {
        uVar2 = _objc_exception_extract(local_84);
                    /* WARNING: Subroutine does not return */
        _objc_exception_throw(uVar2);
      }
    }
  }
  return;
}
