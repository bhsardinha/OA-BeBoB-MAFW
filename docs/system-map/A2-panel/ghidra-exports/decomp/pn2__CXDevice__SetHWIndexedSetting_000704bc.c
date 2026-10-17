// CXDevice::SetHWIndexedSetting
// Entry: 000704bc
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::SetHWIndexedSetting(unsigned long, unsigned short, unsigned long, void*) */

undefined2 __thiscall
CXDevice::SetHWIndexedSetting
          (CXDevice *this,ulong param_1,ushort param_2,ulong param_3,void *param_4)

{
  ushort uVar1;
  ushort uVar2;
  short sVar3;
  short sVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  OpaqueEventLoopRef *pOVar9;
  uint uVar10;
  undefined2 local_2c;
  uint local_24;
  uint local_20;
  ulong local_1c;
  NSString *local_18;
  ushort local_12;
  
  local_12 = param_2;
  uVar5 = _objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4,PTR_s_alloc_001d401c);
  uVar5 = _objc_msgSend(uVar5,PTR_s_init_001d4000);
  if (param_4 == (void *)0x0) {
    UDebugging::DebugThrow
              (0x6e696c50,"SetHWIndexedSetting",
               (uchar *)
               "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x6c1);
  }
  uVar1 = local_12;
  local_2c = 1;
  switch(param_1) {
  case 0x5ea:
    if (param_3 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,4,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x74f,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x74f);
    }
    local_2c = 0;
    SetChannelLevelProperty(this,(NSString *)&cf_MAAuxSendLevels,param_4,1,(uint)local_12);
    break;
  case 0x5eb:
    if ((local_12 != 0) && (uVar2 = (**(code **)(*(int *)this + 0x5c))(this), uVar1 <= uVar2)) {
      local_2c = 0;
      SetChannelLevelProperty(this,(NSString *)&cf_MAInputPan2,param_4,1,(uint)local_12);
    }
    break;
  case 0x5ec:
    if (param_3 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,4,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x747,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x747);
    }
    ChooseMixerBusAndChannelIndex(this,(NSString *)&cf_Levels,&local_18,&local_12);
    local_2c = 0;
    SetChannelLevelProperty(this,local_18,param_4,1,(uint)local_12);
    break;
  case 0x5ed:
  case 0x5f7:
    if (param_3 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x6c9,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x6c9);
    }
    uVar7 = Get64BitNumberProperty(this,(NSString *)&cf_MARotaryControlV2);
    ChooseRotaryPairPhaseAndBitIndex(this,param_1,&local_12,&local_1c);
    uVar6 = 1 << ((byte)(local_12 + local_1c) & 0x1f);
    if ((local_12 + local_1c & 0x20) != 0) {
      uVar6 = 0;
    }
    if (*(char *)param_4 == '\0') {
      uVar6 = ~uVar6 & uVar7;
    }
    else {
      uVar6 = uVar6 | uVar7;
    }
    local_2c = 0;
    Set64BitNumberProperty
              ((NSString *)this,
               CONCAT26((short)(uVar6 >> 0x10),CONCAT24((short)uVar6,&cf_MARotaryControlV2)));
    break;
  case 0x5ee:
    local_2c = 0;
    SetMixerPairBooleanProperty(this,(NSString *)&cf_Link,local_12,param_3,param_4);
    break;
  case 0x5ef:
    local_2c = 0;
    SetMixerPairBooleanProperty(this,(NSString *)&cf_Solo,local_12,param_3,param_4);
    break;
  case 0x5f0:
    local_2c = 0;
    SetMixerPairBooleanProperty(this,(NSString *)&cf_Mute,local_12,param_3,param_4);
    break;
  case 0x5f1:
    local_24 = 0;
    if (param_3 != 0) {
      uVar7 = 0;
      uVar6 = 0;
      do {
        if (*(char *)((int)param_4 + uVar6) != '\0') {
          uVar10 = 1 << ((byte)uVar6 & 0x1f);
          if ((uVar6 & 0x20) != 0) {
            uVar10 = 0;
          }
          uVar7 = uVar7 | uVar10;
          local_24 = uVar7;
        }
        uVar6 = uVar6 + 1;
      } while (param_3 != uVar6);
    }
    ChooseMixerBusAndChannelPair(this,(NSString *)&cf_RoutingV2,&local_18,&local_12);
    local_2c = 0;
    SetChannelLevelProperty(this,local_18,&local_24,1,(uint)local_12);
    break;
  case 0x5f4:
    if (param_3 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,4,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x757,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x757);
    }
    ChooseOutputBusAndChannelIndex(this,(NSString *)&cf_Levels,&local_18,&local_12);
    local_2c = 0;
    SetChannelLevelProperty(this,local_18,param_4,1,(uint)local_12);
    break;
  case 0x5f5:
    uVar6 = (uint)local_12;
    uVar7 = (**(code **)(*(int *)this + 0x18))(this);
    uVar1 = local_12;
    if (((int)((uVar7 & 0xffff) - 2) <= (int)uVar6) &&
       (uVar2 = (**(code **)(*(int *)this + 0x18))(this), uVar1 <= uVar2)) {
      uVar7 = (uint)local_12;
      iVar8 = (**(code **)(*(int *)this + 0x18))(this);
      local_2c = 0;
      SetChannelLevelProperty
                (this,(NSString *)&cf_MAAuxOutputPan2,param_4,1,uVar7 - (iVar8 - 2U & 0xffff));
    }
    break;
  case 0x5f8:
    if (param_3 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x719,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x719);
    }
    ChooseOutputBusAndChannelPair(this,(NSString *)&cf_Link,&local_18,&local_12);
    local_2c = 0;
    SetPairBooleanProperty(this,local_18,local_12,param_3,param_4);
    break;
  case 0x5f9:
    if (param_3 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x703,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x703);
    }
    uVar1 = local_12;
    if (local_12 != 0) {
      sVar3 = (**(code **)(*(int *)this + 0x18))(this);
      sVar4 = (**(code **)(*(int *)this + 0x60))(this);
      if (uVar1 <= (ushort)((sVar3 + -2) - sVar4)) {
        local_2c = 0;
        SetPairBooleanProperty(this,(NSString *)&cf_MAOutputSolo,local_12,param_3,param_4);
      }
    }
    break;
  case 0x5fa:
    if (param_3 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x710,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x710);
    }
    ChooseOutputBusAndChannelPair(this,(NSString *)&cf_Mute,&local_18,&local_12);
    local_2c = 0;
    SetPairBooleanProperty(this,local_18,local_12,param_3,param_4);
    break;
  case 0x5fb:
    if (param_3 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,2,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x6ed,"SetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"SetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x6ed);
    }
    uVar1 = local_12;
    if ((local_12 != 0) &&
       (uVar7 = (**(code **)(*(int *)this + 0x68))(this), uVar1 <= (ushort)((uVar7 & 0xfffe) >> 1)))
    {
      local_20 = (uint)(*(short *)param_4 != 1);
      local_2c = 0;
      SetChannelLevelProperty(this,(NSString *)&cf_MAMonitorAuxOnOutput,&local_20,1,(uint)local_12);
    }
  }
  if ((this[0x7c] == (CXDevice)0x0) &&
     (iVar8 = _objc_msgSend(*(undefined4 *)(this + 0x70),PTR_s_count_001d4080), iVar8 != 0)) {
    SendQueuedProperties(this);
  }
  pOVar9 = (OpaqueEventLoopRef *)_GetMainEventLoop();
  LTimerTask::Install((LTimerTask *)(this + 0xe0),pOVar9,1.0,1.0);
  _objc_msgSend(uVar5,PTR_s_release_001d4048);
  return local_2c;
}
