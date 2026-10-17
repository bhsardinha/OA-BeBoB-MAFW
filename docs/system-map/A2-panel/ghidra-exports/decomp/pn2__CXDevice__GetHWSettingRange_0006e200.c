// CXDevice::GetHWSettingRange
// Entry: 0006e200
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::GetHWSettingRange(unsigned long, ValueRangeType*) */

undefined4 __thiscall
CXDevice::GetHWSettingRange(CXDevice *this,ulong param_1,ValueRangeType *param_2)

{
  undefined4 uVar1;
  uint uVar2;
  
  if (param_2 == (ValueRangeType *)0x0) {
    UDebugging::DebugThrow
              (0x6e696c50,"GetHWSettingRange",
               (uchar *)
               "i/Volumes/MacBuild_Source/SVN_Source/FireWireBeBoB_01.11.002/Source/ControlPanel/../Middleware/CXDevice.mm"
               ,0x355);
  }
  uVar2 = param_1 - 0x5e8;
  if (uVar2 < 0xe) {
    if ((0xc03U >> (uVar2 & 0x1f) & 1) != 0) {
      *(undefined4 *)param_2 = 0xffffffc0;
      *(undefined4 *)(param_2 + 4) = 0;
      *(undefined4 *)(param_2 + 8) = 0x3f800000;
      return 0;
    }
    if ((0x1014U >> (uVar2 & 0x1f) & 1) == 0) {
      if ((0x2008U >> (uVar2 & 0x1f) & 1) == 0) goto LAB_0006e2a8;
      *(undefined4 *)param_2 = 0xffffff01;
      *(undefined4 *)(param_2 + 4) = 0xff;
    }
    else {
      *(undefined4 *)param_2 = 0xffffff7f;
      *(undefined4 *)(param_2 + 4) = 0;
    }
    *(undefined4 *)(param_2 + 8) = 0x3f000000;
    uVar1 = 0;
  }
  else {
LAB_0006e2a8:
    *(undefined4 *)param_2 = 0;
    *(undefined4 *)(param_2 + 4) = 100;
    *(undefined4 *)(param_2 + 8) = 0x3f800000;
    uVar1 = 1;
  }
  return uVar1;
}
