// CXDevice::SetHWSetting
// Entry: 0006f436
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::SetHWSetting(unsigned long, unsigned long, void*) */

undefined4 __thiscall
CXDevice::SetHWSetting(CXDevice *this,ulong param_1,ulong param_2,void *param_3)

{
  bool bVar1;
  ushort uVar2;
  CXDevice *this_00;
  undefined4 uVar3;
  undefined4 uVar4;
  OpaqueEventLoopRef *pOVar5;
  int iVar6;
  undefined4 uVar7;
  
  this_00 = (CXDevice *)_objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4,PTR_s_alloc_001d401c);
  uVar3 = _objc_msgSend(this_00,PTR_s_init_001d4000);
  uVar7 = 1;
  if (param_1 < 0x5ff) {
    switch(param_1) {
    case 0x5e2:
      if (param_2 != 1) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,1,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x527,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x527);
      }
      this[0x7c] = *(CXDevice *)param_3;
      uVar7 = 0;
      break;
    case 0x5e3:
      _objc_msgSend(*(undefined4 *)(this + 0x6c),PTR_s_removeAllObjects_001d4094);
      uVar7 = 0;
      Set32BitNumberProperty(this,(NSString *)&cf_MAMetaResetToFactorySettings,1);
      break;
    case 0x5e4:
      if (param_2 != 8) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,8,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x52d,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x52d);
      }
      iVar6 = 1;
      if (*(uint *)((int)param_3 + 4) < 0x4e) {
        iVar6 = 0xd - *(uint *)((int)param_3 + 4) / 6;
      }
      uVar7 = 0;
      if (iVar6 != 0) {
        Set32BitNumberProperty
                  (this,(NSString *)&cf_MAMetaVirtualRotateDataKnob,iVar6 * *(int *)param_3);
      }
      break;
    case 0x5e5:
      if (param_2 != 2) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,2,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x535,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x535);
      }
      uVar4 = 0;
      if (*(ushort *)param_3 - 1 < 5) {
        uVar4 = *(undefined4 *)("8CXDevice" + (uint)*(ushort *)param_3 * 8 + 8);
      }
      uVar7 = 0;
      Set64BitNumberProperty((NSString *)this,CONCAT44(uVar4,&cf_MARotaryControlV2));
      break;
    case 0x5e6:
      if (param_2 != 1) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,1,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x517,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x517);
      }
      uVar7 = 0;
      Set32BitNumberProperty(this,(NSString *)&cf_MAGlobalMute,(uint)*(byte *)param_3);
      break;
    case 0x5e7:
      if (param_2 != 1) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,1,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x51f,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x51f);
      }
      uVar7 = 0;
      Set32BitNumberProperty(this,(NSString *)&cf_MAGlobalDim,(uint)*(byte *)param_3);
    }
  }
  else {
    switch(param_1) {
    case 0x5ff:
      uVar2 = ExtractParamValue<unsigned_short>(this_00,param_2,param_3);
      if (3 < uVar2) {
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x566);
      }
      uVar7 = 0;
      ChangeDigitalSourceAttribute
                (this,3,*(ulong *)(&DAT_000f8b50 + (uint)uVar2 * 4),true,0x40000000);
      break;
    case 0x600:
      uVar2 = ExtractParamValue<unsigned_short>(this_00,param_2,param_3);
      uVar7 = 0;
      ChangeDigitalSourceAttribute(this,4,4,uVar2 == 1,0x40000000);
      break;
    case 0x601:
      if (param_2 != 4) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,4,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x59b,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x59b);
      }
      *(undefined4 *)(this + 0x84) = *(undefined4 *)param_3;
      pOVar5 = (OpaqueEventLoopRef *)_GetMainEventLoop();
      uVar7 = 0;
      LTimerTask::Install((LTimerTask *)(this + 0xf8),pOVar5,1.0,1.0);
      break;
    case 0x60d:
      uVar2 = ExtractParamValue<unsigned_short>(this_00,param_2,param_3);
      uVar7 = 0;
      ChangeDigitalSourceAttribute(this,8,8,uVar2 == 2,0x40000000);
      break;
    case 0x60e:
      uVar2 = ExtractParamValue<unsigned_short>(this_00,param_2,param_3);
      uVar7 = 0;
      ChangeDigitalSourceAttribute(this,0x10,0x10,uVar2 == 2,0x40000000);
      break;
    case 0x60f:
      uVar2 = ExtractParamValue<unsigned_short>(this_00,param_2,param_3);
      uVar7 = 0;
      ChangeDigitalSourceAttribute(this,4,4,uVar2 == 1,0x40000000);
      break;
    case 0x611:
      bVar1 = ExtractParamValue<bool>(this_00,param_2,param_3);
      uVar7 = 0;
      ChangeDigitalSourceAttribute(this,0x20,0x20,bVar1,0x40000000);
      break;
    case 0x612:
      if (param_2 != 2) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,2,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x592,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x592);
      }
      uVar7 = 0;
      Set32BitNumberProperty(this,(NSString *)&cf_MASPDIFMixerBypass,(uint)(*(short *)param_3 == 0))
      ;
      break;
    case 0x614:
      if (param_2 != 1) {
        OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s",param_2,1,
               "/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x504,"SetHWSetting");
        UDebugging::DebugThrow
                  (0x61737274,"SetHWSetting",
                   (uchar *)
                   "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                   ,0x504);
      }
      uVar7 = 0;
      Set32BitNumberProperty(this,(NSString *)&cf_MAEnableStereoPairEngines,(uint)*(byte *)param_3);
    }
  }
  if ((this[0x7c] == (CXDevice)0x0) &&
     (iVar6 = _objc_msgSend(*(undefined4 *)(this + 0x70),PTR_s_count_001d4080), iVar6 != 0)) {
    SendQueuedProperties(this);
  }
  _objc_msgSend(uVar3,PTR_s_release_001d4048);
  return uVar7;
}
