// CXSelectorHPDevice::SetHWIndexedSetting
// Entry: 00073c64
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXSelectorHPDevice::SetHWIndexedSetting(unsigned long, unsigned short, unsigned long, void*) */

undefined2 __thiscall
CXSelectorHPDevice::SetHWIndexedSetting
          (CXSelectorHPDevice *this,ulong param_1,ushort param_2,ulong param_3,void *param_4)

{
  char cVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  cfstringStruct *pcVar4;
  
  uVar3 = _objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4,PTR_s_alloc_001d401c);
  uVar3 = _objc_msgSend(uVar3,PTR_s_init_001d4000);
  if (param_4 == (void *)0x0) {
    UDebugging::DebugThrow
              (0x6e696c50,"SetHWIndexedSetting",
               (uchar *)
               "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
               ,0xd1);
  }
  if (param_1 == 0x5fd) {
    if (param_3 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,2,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0xd9,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0xd9);
    }
    cVar1 = (**(code **)(*(int *)this + 0x78))(this,*(undefined2 *)param_4);
    uVar2 = 0;
    pcVar4 = &cf_MAHeadphoneSecondSource;
    if (param_2 == 8) {
      pcVar4 = &cf_MAHeadphoneSourceV2;
    }
    CXDevice::Set32BitNumberProperty((CXDevice *)this,(NSString *)pcVar4,1 << (cVar1 - 1U & 0x1f));
  }
  else {
    uVar2 = CXDevice::SetHWIndexedSetting((CXDevice *)this,param_1,param_2,param_3,param_4);
  }
  _objc_msgSend(uVar3,PTR_s_release_001d4048);
  return uVar2;
}
