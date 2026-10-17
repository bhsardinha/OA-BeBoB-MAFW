// CXDevice::GetHWIndexedSetting
// Entry: 0006fce2
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::GetHWIndexedSetting(unsigned long, unsigned short, unsigned long, void*) */

undefined4 __thiscall
CXDevice::GetHWIndexedSetting
          (CXDevice *this,ulong param_1,ushort param_2,ulong param_3,void *param_4)

{
  byte bVar1;
  ushort uVar2;
  ushort uVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  byte bVar9;
  undefined4 uVar10;
  undefined8 uVar11;
  uint local_3c;
  int local_38;
  ulong local_34;
  NSString *local_30;
  ushort local_2a;
  NSString *local_28;
  ushort local_22;
  NSString *local_20;
  ushort local_1a;
  NSString *local_18;
  ushort local_12;
  
  local_2a = param_2;
  uVar4 = _objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4,PTR_s_alloc_001d401c);
  uVar4 = _objc_msgSend(uVar4,PTR_s_init_001d4000);
  if (param_4 == (void *)0x0) {
    UDebugging::DebugThrow
              (0x6e696c50,"GetHWIndexedSetting",
               (uchar *)
               "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x609);
  }
  uVar2 = local_2a;
  uVar10 = 0;
  switch(param_1) {
  case 0x5ea:
    if (param_3 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,4,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x690,"GetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x690);
    }
    GetChannelLevelProperty(this,(NSString *)&cf_MAAuxSendLevels,(uint)local_2a,param_4);
    uVar10 = 0;
    break;
  case 0x5eb:
    uVar10 = 1;
    if ((local_2a != 0) && (uVar3 = (**(code **)(*(int *)this + 0x5c))(this), uVar2 <= uVar3)) {
      GetChannelLevelProperty(this,(NSString *)&cf_MAInputPan2,(uint)local_2a,param_4);
      uVar10 = 0;
    }
    break;
  case 0x5ec:
    if (param_3 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,4,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x684,"GetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x684);
    }
    ChooseMixerBusAndChannelIndex(this,(NSString *)&cf_Levels,&local_30,&local_2a);
    GetChannelLevelProperty(this,local_30,(uint)local_2a,param_4);
    uVar10 = 0;
    break;
  case 0x5ed:
  case 0x5f7:
    if (param_3 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,1,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x612,"GetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x612);
    }
    uVar11 = Get64BitNumberProperty(this,(NSString *)&cf_MARotaryControlV2);
    ChooseRotaryPairPhaseAndBitIndex(this,param_1,&local_2a,&local_34);
    bVar9 = (byte)(local_2a + local_34);
    bVar1 = bVar9 & 0x1f;
    uVar5 = 1 << (bVar9 & 0x1f);
    uVar6 = uVar5;
    uVar8 = 0 << bVar1 | 1U >> 0x20 - bVar1;
    if ((local_2a + local_34 & 0x20) != 0) {
      uVar6 = 0;
      uVar8 = uVar5;
    }
    *(bool *)param_4 =
         (uVar6 & (uint)uVar11) != 0 || (uVar8 & (uint)((ulonglong)uVar11 >> 0x20)) != 0;
  default:
switchD_0006fd7b_default:
    uVar10 = 0;
    break;
  case 0x5ee:
    local_12 = local_2a;
    ChooseMixerBusAndChannelPair(this,(NSString *)&cf_Link,&local_18,&local_12);
    GetPairBooleanProperty(this,local_18,local_12,param_3,param_4);
    uVar10 = 0;
    break;
  case 0x5ef:
    local_1a = local_2a;
    ChooseMixerBusAndChannelPair(this,(NSString *)&cf_Solo,&local_20,&local_1a);
    GetPairBooleanProperty(this,local_20,local_1a,param_3,param_4);
    uVar10 = 0;
    break;
  case 0x5f0:
    local_22 = local_2a;
    ChooseMixerBusAndChannelPair(this,(NSString *)&cf_Mute,&local_28,&local_22);
    GetPairBooleanProperty(this,local_28,local_22,param_3,param_4);
    uVar10 = 0;
    break;
  case 0x5f1:
    ChooseMixerBusAndChannelPair(this,(NSString *)&cf_RoutingV2,&local_30,&local_2a);
    GetChannelLevelProperty(this,local_30,(uint)local_2a,&local_3c);
    if (param_3 != 0) {
      uVar8 = 0;
      do {
        *(bool *)((int)param_4 + uVar8) = (local_3c >> (uVar8 & 0x1f) & 1) != 0;
        uVar8 = uVar8 + 1;
      } while (param_3 != uVar8);
    }
    goto switchD_0006fd7b_default;
  case 0x5f2:
  case 0x5f3:
  case 0x5f6:
    break;
  case 0x5f4:
    if (param_3 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_3,4,
             "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
             ,0x698,"GetHWIndexedSetting");
      UDebugging::DebugThrow
                (0x61737274,"GetHWIndexedSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x698);
    }
    ChooseOutputBusAndChannelIndex(this,(NSString *)&cf_Levels,&local_30,&local_2a);
    GetChannelLevelProperty(this,local_30,(uint)local_2a,param_4);
    uVar10 = 0;
    break;
  case 0x5f5:
    uVar6 = (uint)local_2a;
    uVar8 = (**(code **)(*(int *)this + 0x18))(this);
    uVar2 = local_2a;
    uVar10 = 1;
    if (((int)((uVar8 & 0xffff) - 2) <= (int)uVar6) &&
       (uVar3 = (**(code **)(*(int *)this + 0x18))(this), uVar2 <= uVar3)) {
      uVar6 = (uint)local_2a;
      uVar8 = (**(code **)(*(int *)this + 0x18))(this);
      GetChannelLevelProperty
                (this,(NSString *)&cf_MAAuxOutputPan2,(uVar6 + 2) - (uVar8 & 0xffff),param_4);
      uVar10 = 0;
    }
    break;
  case 0x5f8:
    ChooseOutputBusAndChannelPair(this,(NSString *)&cf_Link,&local_30,&local_2a);
    GetPairBooleanProperty(this,local_30,local_2a,param_3,param_4);
    uVar10 = 0;
    break;
  case 0x5f9:
    uVar10 = 1;
    if (local_2a != 0) {
      uVar3 = (**(code **)(*(int *)this + 0x18))(this);
      iVar7 = (**(code **)(*(int *)this + 0x60))(this);
      if (uVar2 <= (ushort)(((uVar3 - 2) - iVar7 & 0xfffe) >> 1)) {
        GetPairBooleanProperty(this,(NSString *)&cf_MAOutputSolo,local_2a,param_3,param_4);
        uVar10 = 0;
      }
    }
    break;
  case 0x5fa:
    ChooseOutputBusAndChannelPair(this,(NSString *)&cf_Mute,&local_30,&local_2a);
    GetPairBooleanProperty(this,local_30,local_2a,param_3,param_4);
    uVar10 = 0;
    break;
  case 0x5fb:
    uVar10 = 1;
    if ((local_2a == 0) ||
       (uVar8 = (**(code **)(*(int *)this + 0x68))(this), (ushort)((uVar8 & 0xfffe) >> 1) < uVar2))
    break;
    GetChannelLevelProperty(this,(NSString *)&cf_MAMonitorAuxOnOutput,(uint)local_2a,&local_38);
    *(ushort *)param_4 = 2 - (ushort)(local_38 == 0);
    goto switchD_0006fd7b_default;
  }
  _objc_msgSend(uVar4,PTR_s_release_001d4048);
  return uVar10;
}
