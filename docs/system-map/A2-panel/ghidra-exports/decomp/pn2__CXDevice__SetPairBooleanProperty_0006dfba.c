// CXDevice::SetPairBooleanProperty
// Entry: 0006dfba
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::SetPairBooleanProperty(NSString*, unsigned short, unsigned long, void*) */

void __thiscall
CXDevice::SetPairBooleanProperty
          (CXDevice *this,NSString *param_1,ushort param_2,ulong param_3,void *param_4)

{
  uint local_14;
  
  if (param_3 != 1) {
    OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
           "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
           ,0x316,"SetPairBooleanProperty");
    UDebugging::DebugThrow
              (0x61737274,"SetPairBooleanProperty",
               (uchar *)
               "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x316);
  }
  local_14 = (uint)*(byte *)param_4;
  SetChannelLevelProperty(this,param_1,&local_14,2,(uint)param_2 * 2 + -1,(uint)param_2 * 2);
  return;
}
