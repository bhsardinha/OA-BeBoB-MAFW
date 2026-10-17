// CFW1814DeviceSettings::CFW1814DeviceSettings
// Entry: 0005944e
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::CFW1814DeviceSettings(unsigned long) */

void __thiscall
CFW1814DeviceSettings::CFW1814DeviceSettings(CFW1814DeviceSettings *this,ulong param_1)

{
  code *pcVar1;
  int iVar2;
  CMiddleWareManager *this_00;
  undefined4 uVar3;
  
  CDeviceSettings::CDeviceSettings((CDeviceSettings *)this,param_1);
  *(undefined ***)this = &PTR__CFW1814DeviceSettings_001809a8;
  *(undefined **)(this + 0x30) = &DAT_00180a68;
  *(undefined4 *)(this + 0x84) = 0;
  *(undefined4 *)(this + 0x424) = 0;
  *(undefined4 *)(this + 0x420) = 0;
  *(undefined4 *)(this + 0x42c) = 0;
  *(undefined4 *)(this + 0x428) = 0;
  *(undefined4 *)(this + 0x434) = 0;
  *(undefined4 *)(this + 0x430) = 0;
  *(undefined4 *)(this + 0x438) = 0;
  *(undefined2 *)(this + 0x88) = 1;
  *(undefined2 *)(this + 0x8a) = 0;
  iVar2 = 0;
  do {
    *(undefined4 *)(this + iVar2 * 4 + 0x8c) = 0xffffffc0;
    *(undefined4 *)(this + iVar2 * 4 + 0x10c) = 0;
    *(undefined4 *)(this + iVar2 * 4 + 0x18c) = 0xffffffc0;
    iVar2 = iVar2 + 1;
  } while ((short)iVar2 != 0x20);
  iVar2 = 0;
  do {
    this[iVar2 + 0x20c] = (CFW1814DeviceSettings)0x0;
    this[iVar2 + 0x21c] = (CFW1814DeviceSettings)0x0;
    this[iVar2 + 0x22c] = (CFW1814DeviceSettings)0x0;
    this[iVar2 + 0x23c] = (CFW1814DeviceSettings)0x0;
    *(undefined2 *)(this + iVar2 * 2 + 0x24c) = 0;
    iVar2 = iVar2 + 1;
  } while ((short)iVar2 != 0x10);
  iVar2 = 0;
  do {
    *(undefined4 *)(this + iVar2 * 4 + 0x26c) = 0xffffffc0;
    *(undefined4 *)(this + iVar2 * 4 + 700) = 0xffffffc0;
    iVar2 = iVar2 + 1;
  } while ((short)iVar2 != 0x14);
  iVar2 = 0;
  do {
    *(undefined4 *)(this + iVar2 * 4 + 0x30c) = 0;
    this[iVar2 + 0x334] = (CFW1814DeviceSettings)0x0;
    this[iVar2 + 0x33e] = (CFW1814DeviceSettings)0x0;
    this[iVar2 + 0x348] = (CFW1814DeviceSettings)0x0;
    this[iVar2 + 0x352] = (CFW1814DeviceSettings)0x0;
    *(undefined2 *)(this + iVar2 * 2 + 0x35c) = 1;
    iVar2 = iVar2 + 1;
  } while ((short)iVar2 != 10);
  *(undefined2 *)(this + 0x370) = 1;
  *(undefined2 *)(this + 0x372) = 2;
  this[0x374] = (CFW1814DeviceSettings)0x1;
  *(undefined4 *)(this + 0x378) = 0;
  *(undefined2 *)(this + 0x37c) = 1;
  *(undefined4 *)(this + 0x388) = 48000;
  *(undefined4 *)(this + 0x382) = 0;
  *(undefined4 *)(this + 0x37e) = 0;
  *(undefined2 *)(this + 0x38c) = 1;
  *(undefined2 *)(this + 0x38e) = 1;
  *(undefined2 *)(this + 0x390) = 1;
  *(undefined2 *)(this + 0x392) = 3;
  *(undefined2 *)(this + 0x394) = 1;
  *(undefined2 *)(this + 0x396) = 2;
  *(undefined2 *)(this + 0x398) = 3;
  this[0x39a] = (CFW1814DeviceSettings)0x1;
  this[0x39b] = (CFW1814DeviceSettings)0x0;
  *(undefined4 *)(this + 0x39c) = 0xffff0001;
  *(undefined4 *)(this + 0x3a0) = 0xffff;
  *(undefined4 *)(this + 0x3a4) = 0x3f800000;
  *(undefined4 *)(this + 0x3a8) = 0xffffffc0;
  *(undefined4 *)(this + 0x3ac) = 0;
  *(undefined4 *)(this + 0x3b0) = 0x3f800000;
  *(undefined4 *)(this + 0x3b4) = 0xffffffc0;
  *(undefined4 *)(this + 0x3b8) = 0;
  *(undefined4 *)(this + 0x3bc) = 0x3f800000;
  *(undefined4 *)(this + 0x3c0) = 0xffffffc0;
  *(undefined4 *)(this + 0x3c4) = 0;
  *(undefined4 *)(this + 0x3c8) = 0x3f800000;
  *(undefined4 *)(this + 0x3cc) = 0xffffffc0;
  *(undefined4 *)(this + 0x3d0) = 0;
  *(undefined4 *)(this + 0x3d4) = 0x3f800000;
  *(undefined4 *)(this + 0x3d8) = 0xffffffc0;
  *(undefined4 *)(this + 0x3dc) = 0;
  *(undefined4 *)(this + 0x3e0) = 0x3f800000;
  *(undefined4 *)(this + 0x3e4) = 0xffffffc0;
  *(undefined4 *)(this + 1000) = 0;
  *(undefined4 *)(this + 0x3ec) = 0x3f800000;
  *(undefined4 *)(this + 0x3f0) = 0xffffffc0;
  *(undefined4 *)(this + 0x3f4) = 0x40;
  *(undefined4 *)(this + 0x3f8) = 0x3f800000;
  *(undefined4 *)(this + 0x3fc) = 0;
  *(undefined4 *)(this + 0x400) = 0;
  *(undefined4 *)(this + 0x404) = 0x3f800000;
  *(undefined4 *)(this + 0x408) = 0;
  *(undefined4 *)(this + 0x40c) = 0;
  *(undefined4 *)(this + 0x410) = 0x3f800000;
  *(undefined4 *)(this + 0x414) = 0;
  *(undefined4 *)(this + 0x418) = 0;
  *(undefined4 *)(this + 0x41c) = 0x3f800000;
  pcVar1 = *(code **)(*(int *)this + 0x94);
  this_00 = (CMiddleWareManager *)CMiddleWareManager::GetMiddleWareMgr();
  uVar3 = CMiddleWareManager::GetDevice(this_00,param_1);
  (*pcVar1)(this,uVar3);
  return;
}
