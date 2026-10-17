// CXSelectorHPDevice::GetHWSetting
// Entry: 00073584
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXSelectorHPDevice::GetHWSetting(unsigned long, unsigned long, void*) */

undefined2 __thiscall
CXSelectorHPDevice::GetHWSetting(CXSelectorHPDevice *this,ulong param_1,ulong param_2,void *param_3)

{
  code *pcVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined2 local_12;
  
  uVar3 = _objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4,PTR_s_alloc_001d401c);
  uVar3 = _objc_msgSend(uVar3,PTR_s_init_001d4000);
  if (param_3 == (void *)0x0) {
    UDebugging::DebugThrow
              (0x6e696c50,"GetHWSetting",
               (uchar *)
               "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
               ,0x38);
  }
  if (param_1 == 0x5fd) {
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,2,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0x6b,"GetHWSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0x6b);
    }
    uVar5 = CXDevice::Get32BitNumberProperty((CXDevice *)this,(NSString *)&cf_MAHeadphoneSourceV2);
    uVar2 = (**(code **)(*(int *)this + 0x7c))(this,uVar5);
    *(undefined2 *)param_3 = uVar2;
    local_12 = 0;
  }
  else if (param_1 == 0x609) {
    if (param_2 != 6) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,6,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0x59,"GetHWSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0x59);
    }
    iVar4 = _objc_msgSend(*(undefined4 *)(this + 0x5c),PTR_s_propertyForKey__001d4018,
                          &cf_MAMomentarySwitchRoutings);
    local_12 = 0;
    if (iVar4 != 0) {
      pcVar1 = *(code **)(*(int *)this + 0x7c);
      uVar5 = _objc_msgSend(iVar4,PTR_s_objectAtIndex__001d406c,0);
      uVar5 = _objc_msgSend(uVar5,PTR_s_unsignedLongValue_001d4058);
      uVar2 = (*pcVar1)(this,uVar5);
      *(undefined2 *)param_3 = uVar2;
      pcVar1 = *(code **)(*(int *)this + 0x7c);
      uVar5 = _objc_msgSend(iVar4,PTR_s_objectAtIndex__001d406c,1);
      uVar5 = _objc_msgSend(uVar5,PTR_s_unsignedLongValue_001d4058);
      uVar2 = (*pcVar1)(this,uVar5);
      *(undefined2 *)((int)param_3 + 2) = uVar2;
      pcVar1 = *(code **)(*(int *)this + 0x7c);
      uVar5 = _objc_msgSend(iVar4,PTR_s_objectAtIndex__001d406c,2);
      uVar5 = _objc_msgSend(uVar5,PTR_s_unsignedLongValue_001d4058);
      uVar2 = (*pcVar1)(this,uVar5);
      *(undefined2 *)((int)param_3 + 4) = uVar2;
    }
  }
  else if (param_1 == 0x608) {
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,2,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0x3f,"GetHWSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0x3f);
    }
    iVar4 = CXDevice::Get32BitNumberProperty
                      ((CXDevice *)this,(NSString *)&cf_MAMomentarySwitchSource);
    uVar2 = 1;
    if (iVar4 == 1) {
      uVar2 = 3;
    }
    *(undefined2 *)param_3 = uVar2;
    local_12 = 0;
  }
  else {
    local_12 = CXDevice::GetHWSetting((CXDevice *)this,param_1,param_2,param_3);
  }
  _objc_msgSend(uVar3,PTR_s_release_001d4048);
  return local_12;
}
