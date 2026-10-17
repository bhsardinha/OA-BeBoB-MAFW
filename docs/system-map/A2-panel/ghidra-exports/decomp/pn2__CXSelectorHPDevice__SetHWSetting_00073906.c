// CXSelectorHPDevice::SetHWSetting
// Entry: 00073906
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXSelectorHPDevice::SetHWSetting(unsigned long, unsigned long, void*) */

undefined2 __thiscall
CXSelectorHPDevice::SetHWSetting(CXSelectorHPDevice *this,ulong param_1,ulong param_2,void *param_3)

{
  undefined *puVar1;
  char cVar2;
  undefined2 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  
  uVar4 = _objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4,PTR_s_alloc_001d401c);
  uVar4 = _objc_msgSend(uVar4,PTR_s_init_001d4000);
  if (param_1 == 0x5fd) {
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,2,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0xb9,"SetHWSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0xb9);
    }
    cVar2 = (**(code **)(*(int *)this + 0x78))(this,*(undefined2 *)param_3);
    param_2 = 1 << (cVar2 - 1U & 0x1f);
    uVar3 = 0;
    CXDevice::Set32BitNumberProperty((CXDevice *)this,(NSString *)&cf_MAHeadphoneSourceV2,param_2);
  }
  else if (param_1 == 0x609) {
    if (param_2 != 6) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,6,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0xa9,"SetHWSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0xa9);
    }
    puVar1 = PTR_s_NSNumber_001d478c;
    cVar2 = (**(code **)(*(int *)this + 0x78))(this,*(undefined2 *)param_3);
    uVar5 = _objc_msgSend(puVar1,PTR_s_numberWithUnsignedLong__001d4068,1 << (cVar2 - 1U & 0x1f));
    puVar1 = PTR_s_NSNumber_001d478c;
    cVar2 = (**(code **)(*(int *)this + 0x78))(this,*(undefined2 *)((int)param_3 + 2));
    uVar6 = _objc_msgSend(puVar1,PTR_s_numberWithUnsignedLong__001d4068,1 << (cVar2 - 1U & 0x1f));
    puVar1 = PTR_s_NSNumber_001d478c;
    cVar2 = (**(code **)(*(int *)this + 0x78))(this,*(undefined2 *)((int)param_3 + 4));
    uVar7 = _objc_msgSend(puVar1,PTR_s_numberWithUnsignedLong__001d4068,1 << (cVar2 - 1U & 0x1f));
    param_2 = _objc_msgSend(PTR_s_NSArray_001d4788,PTR_s_arrayWithObjects__001d4120,uVar5,uVar6,
                            uVar7,0);
    uVar3 = 0;
    _objc_msgSend(*(undefined4 *)(this + 0x5c),PTR_s_setProperty_forKey__001d4064,param_2,
                  &cf_MAMomentarySwitchRoutings);
  }
  else if (param_1 == 0x608) {
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,2,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0x91,"SetHWSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0x91);
    }
    uVar3 = 0;
    param_2 = (ulong)(*(short *)param_3 == 3);
    CXDevice::Set32BitNumberProperty
              ((CXDevice *)this,(NSString *)&cf_MAMomentarySwitchSource,param_2);
  }
  else {
    uVar3 = CXDevice::SetHWSetting((CXDevice *)this,param_1,param_2,param_3);
  }
  _objc_msgSend(uVar4,PTR_s_release_001d4048,param_2);
  return uVar3;
}
