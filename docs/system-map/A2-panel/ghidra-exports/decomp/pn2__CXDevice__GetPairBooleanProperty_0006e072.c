// CXDevice::GetPairBooleanProperty
// Entry: 0006e072
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::GetPairBooleanProperty(NSString*, unsigned short, unsigned long, void*) */

void __thiscall
CXDevice::GetPairBooleanProperty
          (CXDevice *this,NSString *param_1,ushort param_2,ulong param_3,void *param_4)

{
  int local_18;
  int local_14;
  
  if (param_3 != 1) {
    OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
           "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
           ,0x329,"GetPairBooleanProperty");
    UDebugging::DebugThrow
              (0x61737274,"GetPairBooleanProperty",
               (uchar *)
               "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x329);
  }
  local_14 = 0;
  local_18 = 0;
  GetChannelLevelProperty(this,param_1,(uint)param_2 * 2 - 1,&local_14);
  GetChannelLevelProperty(this,param_1,(uint)param_2 * 2,&local_18);
  *(bool *)param_4 = local_14 != 0 || local_18 != 0;
  return;
}
