// CXSelectorHPDevice::GetHWIndexedSetting
// Entry: 00073df4
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXSelectorHPDevice::GetHWIndexedSetting(unsigned long, unsigned short, unsigned long, void*) */

undefined2 __thiscall
CXSelectorHPDevice::GetHWIndexedSetting
          (CXSelectorHPDevice *this,ulong param_1,ushort param_2,ulong param_3,void *param_4)

{
  undefined2 uVar1;
  undefined4 uVar2;
  cfstringStruct *pcVar3;
  undefined4 uVar4;
  
  uVar2 = _objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4,PTR_s_alloc_001d401c);
  uVar2 = _objc_msgSend(uVar2,PTR_s_init_001d4000);
  if (param_4 == (void *)0x0) {
    UDebugging::DebugThrow
              (0x6e696c50,"GetHWIndexedSetting",
               (uchar *)
               "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
               ,0x101);
  }
  if (param_1 == 0x5fd) {
    if (param_3 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,2,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
             ,0x108,"GetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWIndexedSetting",
                 (uchar *)
                 "s/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXSelectorHPDevice.mm"
                 ,0x108);
    }
    pcVar3 = &cf_MAHeadphoneSecondSource;
    if (param_2 == 8) {
      pcVar3 = &cf_MAHeadphoneSourceV2;
    }
    uVar4 = CXDevice::Get32BitNumberProperty((CXDevice *)this,(NSString *)pcVar3);
    uVar1 = (**(code **)(*(int *)this + 0x7c))(this,uVar4);
    *(undefined2 *)param_4 = uVar1;
    uVar1 = 0;
  }
  else {
    uVar1 = CXDevice::GetHWIndexedSetting((CXDevice *)this,param_1,param_2,param_3,param_4);
  }
  _objc_msgSend(uVar2,PTR_s_release_001d4048);
  return uVar1;
}
