// CXDevice::SetMixerPairBooleanProperty
// Entry: 0006dee0
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::SetMixerPairBooleanProperty(NSString*, unsigned short, unsigned long, void*) */

void __thiscall
CXDevice::SetMixerPairBooleanProperty
          (CXDevice *this,NSString *param_1,ushort param_2,ulong param_3,void *param_4)

{
  uint local_1c;
  NSString *local_18;
  ushort local_12;
  
  local_12 = param_2;
  if (param_3 != 1) {
    OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
           "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
           ,0x2fd,"SetMixerPairBooleanProperty");
    UDebugging::DebugThrow
              (0x61737274,"SetMixerPairBooleanProperty",
               (uchar *)
               "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x2fd);
  }
  local_1c = (uint)*(byte *)param_4;
  ChooseMixerBusAndChannelPair(this,param_1,&local_18,&local_12);
  SetChannelLevelProperty(this,local_18,&local_1c,2,(uint)local_12 * 2 + -1,(uint)local_12 * 2);
  return;
}
