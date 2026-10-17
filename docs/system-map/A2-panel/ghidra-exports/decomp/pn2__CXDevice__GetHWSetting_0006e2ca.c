// CXDevice::GetHWSetting
// Entry: 0006e2ca
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::GetHWSetting(unsigned long, unsigned long, void*) */

undefined4 __thiscall
CXDevice::GetHWSetting(CXDevice *this,ulong param_1,ulong param_2,void *param_3)

{
  double dVar1;
  double dVar2;
  double dVar3;
  undefined *puVar4;
  char cVar5;
  ushort uVar6;
  ushort uVar7;
  ushort uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  uint uVar11;
  int iVar12;
  undefined4 uVar13;
  uint uVar14;
  uint uVar15;
  bool bVar16;
  longdouble lVar17;
  ulonglong uVar18;
  undefined4 local_6c;
  
  uVar9 = _objc_msgSend(PTR_s_NSAutoreleasePool_001d47a4);
  uVar9 = _objc_msgSend(uVar9);
  if (param_3 == (void *)0x0) {
    UDebugging::DebugThrow
              (0x6e696c50,"GetHWSetting",
               (uchar *)
               "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x3a0);
  }
  uVar13 = 1;
  switch(param_1) {
  case 0x5de:
    if (param_2 != 0x20) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x452);
    }
    puVar4 = PTR_s_NSString_001d4794;
    uVar13 = CalendarDateFromFirmwareDate
                       (*(ulong *)(this + 0xd0),*(ulong *)(this + 0xcc),*(ulong *)(this + 200),
                        *(ulong *)(this + 0xd0));
    uVar10 = _objc_msgSend(puVar4,PTR_s_stringWithFormat__001d40a4,&cf___,uVar13);
    uVar13 = 0;
    _CFStringGetPascalString(uVar10,param_3,0x1f,0);
    break;
  case 0x5df:
    if (param_2 != 0x20) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x463);
    }
    uVar10 = _objc_msgSend(PTR_s_NSString_001d4794,PTR_s_stringWithFormat__001d40a4,
                           &cf_model_dversion_d,*(undefined4 *)(this + 0xa0),
                           *(undefined4 *)(this + 0xa4));
    uVar13 = 0;
    _CFStringGetPascalString(uVar10,param_3,0x1f,0);
    break;
  case 0x5e0:
    if (param_2 != 0x20) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x442);
    }
    puVar4 = PTR_s_NSString_001d4794;
    uVar13 = CalendarDateFromFirmwareDate
                       (*(ulong *)(this + 0xb0),*(ulong *)(this + 0xac),*(ulong *)(this + 0xa8),
                        *(ulong *)(this + 0xb0));
    uVar10 = _objc_msgSend(puVar4,PTR_s_stringWithFormat__001d40a4,&cf___,uVar13);
    uVar13 = 0;
    _CFStringGetPascalString(uVar10,param_3,0x1f,0);
    break;
  case 0x5e1:
    uVar13 = 0;
    GetVersionProperty(this,param_2,param_3,(NSString *)&cf_GUID);
    break;
  case 0x5e2:
    if (param_2 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3b9);
    }
    *(CXDevice *)param_3 = this[0x7c];
    goto LAB_0006ef3f;
  case 0x5e5:
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3ce);
    }
    uVar18 = Get64BitNumberProperty(this,(NSString *)&cf_MARotaryControlV2);
    if ((short)uVar18 != 0) {
      *(undefined2 *)param_3 = 1;
    }
    if ((short)(uVar18 >> 0x20) != 0) {
      *(undefined2 *)param_3 = 2;
    }
    if ((uVar18 & 0xffff0000) != 0) {
      *(undefined2 *)param_3 = 3;
    }
    if ((uVar18 & 0x300000000000000) != 0) {
      *(undefined2 *)param_3 = 4;
    }
    uVar13 = 0;
    if ((uVar18 & 0x4000000000000) != 0) {
      *(undefined2 *)param_3 = 5;
    }
    break;
  case 0x5e6:
    if (param_2 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3c0);
    }
    iVar12 = Get32BitNumberProperty(this,(NSString *)&cf_MAGlobalMute);
    goto LAB_0006eec3;
  case 0x5e7:
    if (param_2 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3c7);
    }
    iVar12 = Get32BitNumberProperty(this,(NSString *)&cf_MAGlobalDim);
    goto LAB_0006eec3;
  case 0x5e9:
    uVar11 = (**(code **)(*(int *)this + 0x1c))(this);
    if ((uVar11 & 0xffff) * 4 != param_2) {
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x491);
    }
    uVar13 = 0;
    if (*(int *)(this + 0x68) == 0) break;
    cVar5 = (**(code **)(*(int *)this + 0x6c))(this);
    dVar3 = DAT_000f8b48;
    dVar2 = DAT_000f8b40;
    uVar10 = DAT_000f7d38._4_4_;
    uVar13 = (undefined4)DAT_000f7d38;
    if (cVar5 == '\x01') {
      for (uVar11 = 0; uVar15 = (**(code **)(*(int *)this + 0x1c))(this), uVar11 < (uVar15 & 0xffff)
          ; uVar11 = uVar11 + 1) {
        uVar15 = *(uint *)(*(int *)(this + 0x68) + uVar11 * 4);
        iVar12 = -0x40;
        if (uVar15 != 0) {
          lVar17 = (longdouble)
                   _log10(((double)((ulonglong)uVar15 | CONCAT44(uVar10,uVar13)) -
                          (double)CONCAT44(uVar10,uVar13)) * dVar2);
          lVar17 = (longdouble)_ceil((double)lVar17 * dVar3);
          iVar12 = (int)lVar17;
        }
        *(int *)((int)param_3 + uVar11 * 4) = iVar12;
      }
    }
    else {
      uVar6 = (**(code **)(*(int *)this + 0x5c))(this);
      uVar7 = (**(code **)(*(int *)this + 0x18))(this);
      dVar3 = DAT_000f8b48;
      dVar2 = DAT_000f8b40;
      uVar10 = DAT_000f7d38._4_4_;
      uVar13 = (undefined4)DAT_000f7d38;
      for (uVar11 = 0; uVar15 = (**(code **)(*(int *)this + 0x5c))(this), uVar11 < (uVar15 & 0xffff)
          ; uVar11 = uVar11 + 1) {
        uVar15 = *(uint *)(*(int *)(this + 0x68) + uVar11 * 4);
        iVar12 = -0x40;
        if (uVar15 != 0) {
          dVar1 = (double)CONCAT44(uVar10,uVar13);
          lVar17 = (longdouble)
                   _log10(((double)((ulonglong)uVar15 | (ulonglong)dVar1) - dVar1) * dVar2);
          lVar17 = (longdouble)_ceil((double)lVar17 * dVar3);
          iVar12 = (int)lVar17;
        }
        *(int *)((int)param_3 + uVar11 * 4) = iVar12;
      }
      uVar11 = 0;
      while( true ) {
        uVar8 = (**(code **)(*(int *)this + 0x1c))(this);
        iVar12 = (**(code **)(*(int *)this + 0x5c))(this);
        if (((uint)uVar8 - iVar12 & 0xffff) <= uVar11) break;
        uVar15 = *(uint *)(*(int *)(this + 0x68) + (uint)uVar6 * 4 + (uint)uVar7 * 4 + uVar11 * 4);
        iVar12 = -0x40;
        if (uVar15 != 0) {
          dVar1 = (double)CONCAT44(uVar10,uVar13);
          lVar17 = (longdouble)
                   _log10(((double)((ulonglong)uVar15 | (ulonglong)dVar1) - dVar1) * dVar2);
          lVar17 = (longdouble)_ceil((double)lVar17 * dVar3);
          iVar12 = (int)lVar17;
        }
        uVar15 = (**(code **)(*(int *)this + 0x5c))(this);
        *(int *)((int)param_3 + ((uVar15 & 0xffff) + uVar11) * 4) = iVar12;
        uVar11 = uVar11 + 1;
      }
    }
    goto LAB_0006ef3f;
  case 0x5f3:
    uVar11 = (**(code **)(*(int *)this + 0x18))(this);
    if ((uVar11 & 0xffff) * 4 != param_2) {
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x474);
    }
    uVar13 = 0;
    if (*(int *)(this + 0x68) == 0) break;
    cVar5 = (**(code **)(*(int *)this + 0x6c))(this);
    if (cVar5 == '\x01') {
      uVar11 = (**(code **)(*(int *)this + 0x1c))(this);
    }
    else {
      uVar11 = (**(code **)(*(int *)this + 0x5c))(this);
    }
    dVar3 = DAT_000f8b48;
    dVar2 = DAT_000f8b40;
    uVar10 = DAT_000f7d38._4_4_;
    uVar13 = (undefined4)DAT_000f7d38;
    for (uVar15 = 0; uVar14 = (**(code **)(*(int *)this + 0x18))(this), uVar15 < (uVar14 & 0xffff);
        uVar15 = uVar15 + 1) {
      uVar14 = *(uint *)(*(int *)(this + 0x68) + (uVar11 & 0xffff) * 4 + uVar15 * 4);
      iVar12 = -0x40;
      if (uVar14 != 0) {
        lVar17 = (longdouble)
                 _log10(((double)((ulonglong)uVar14 | CONCAT44(uVar10,uVar13)) -
                        (double)CONCAT44(uVar10,uVar13)) * dVar2);
        lVar17 = (longdouble)_ceil((double)lVar17 * dVar3);
        iVar12 = (int)lVar17;
      }
      *(int *)((int)param_3 + uVar15 * 4) = iVar12;
    }
    goto LAB_0006ef3f;
  case 0x5ff:
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x417);
    }
    uVar11 = Get32BitNumberProperty(this,(NSString *)&cf_MAClockSource);
    *(undefined2 *)param_3 = *(undefined2 *)(&DAT_000f8638 + (uVar11 & 3) * 2);
    goto LAB_0006ef3f;
  case 0x600:
    uVar13 = 0;
    GetDigitalSourceAttribute<unsigned_short>(this,4,1,0,param_2,param_3);
    break;
  case 0x601:
    if (param_2 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x40e);
    }
    uVar13 = *(undefined4 *)(this + 0x84);
    goto LAB_0006ef3a;
  case 0x602:
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3ef);
    }
    iVar12 = Get32BitNumberProperty(this,(NSString *)&cf_MARateLocked);
    bVar16 = iVar12 != 1;
    goto LAB_0006ee3d;
  case 0x603:
    if (param_2 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3f8);
    }
    uVar13 = _objc_msgSend(*(undefined4 *)(this + 0x60),PTR_s_supportedSampleRates_001d409c);
    uVar13 = _objc_msgSend(uVar13,PTR_s_count_001d4080);
    goto LAB_0006ef3a;
  case 0x604:
    uVar10 = _objc_msgSend(*(undefined4 *)(this + 0x60));
    uVar11 = _objc_msgSend(uVar10);
    iVar12 = _objc_msgSend(uVar10);
    if (iVar12 * 4 != param_2) {
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x404);
    }
    uVar13 = 0;
    if (uVar11 == 0) break;
    uVar15 = 0;
    do {
      _objc_msgSend(uVar10,PTR_s_objectAtIndex__001d406c,uVar15);
      lVar17 = (longdouble)_objc_msgSend_fpret();
      local_6c = (undefined4)(longlong)lVar17;
      *(undefined4 *)((int)param_3 + uVar15 * 4) = local_6c;
      uVar15 = uVar15 + 1;
    } while (uVar15 < uVar11);
    goto LAB_0006ef3f;
  case 0x60d:
    uVar13 = 0;
    GetDigitalSourceAttribute<unsigned_short>(this,8,2,1,param_2,param_3);
    break;
  case 0x60e:
    uVar13 = 0;
    GetDigitalSourceAttribute<unsigned_short>(this,0x10,2,1,param_2,param_3);
    break;
  case 0x60f:
    uVar13 = 0;
    GetDigitalSourceAttribute<unsigned_short>(this,4,1,2,param_2,param_3);
    break;
  case 0x611:
    uVar13 = 0;
    GetDigitalSourceAttribute<bool>(this,0x20,true,false,param_2,param_3);
    break;
  case 0x612:
    if (param_2 != 2) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x4b4);
    }
    iVar12 = Get32BitNumberProperty(this,(NSString *)&cf_MASPDIFMixerBypass);
    bVar16 = iVar12 == 0;
LAB_0006ee3d:
    *(ushort *)param_3 = (ushort)bVar16;
    goto LAB_0006ef3f;
  case 0x614:
    if (param_2 != 1) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3ac);
    }
    iVar12 = Get32BitNumberProperty(this,(NSString *)&cf_MAEnableStereoPairEngines);
LAB_0006eec3:
    *(bool *)param_3 = iVar12 != 0;
    goto LAB_0006ef3f;
  case 0x615:
    if (param_2 != 4) {
      OSXLog("Incorrect size:%d expected:%d File:%s Line:%d Func:%s");
      UDebugging::DebugThrow
                (0x61737274,"GetHWSetting",
                 (uchar *)
                 "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
                 ,0x3b2);
    }
    uVar13 = (**(code **)(*(int *)this + 0x58))(this);
LAB_0006ef3a:
    *(undefined4 *)param_3 = uVar13;
LAB_0006ef3f:
    uVar13 = 0;
  }
  _objc_msgSend(uVar9,PTR_s_release_001d4048);
  return uVar13;
}
