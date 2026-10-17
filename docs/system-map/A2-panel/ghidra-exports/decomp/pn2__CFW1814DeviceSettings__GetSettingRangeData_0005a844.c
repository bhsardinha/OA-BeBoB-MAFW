// CFW1814DeviceSettings::GetSettingRangeData
// Entry: 0005a844
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::GetSettingRangeData(unsigned long, ValueRangeType*) */

undefined4 __thiscall
CFW1814DeviceSettings::GetSettingRangeData
          (CFW1814DeviceSettings *this,ulong param_1,ValueRangeType *param_2)

{
  undefined4 uVar1;
  
  uVar1 = 1;
  if (param_1 < 0x5e9) {
    if (param_1 != 0x5e4) {
      return 1;
    }
    *(undefined4 *)param_2 = *(undefined4 *)(this + 0x39c);
    *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x3a0);
    uVar1 = *(undefined4 *)(this + 0x3a4);
  }
  else if (param_1 < 0x5f3) {
    switch(param_1) {
    case 0x5e9:
      *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3a8);
      *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x3ac);
      uVar1 = *(undefined4 *)(this + 0x3b0);
      break;
    case 0x5ea:
      *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3b4);
      *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x3b8);
      uVar1 = *(undefined4 *)(this + 0x3bc);
      break;
    case 0x5eb:
      *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3c0);
      *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x3c4);
      uVar1 = *(undefined4 *)(this + 0x3c8);
      break;
    case 0x5ec:
      *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3cc);
      *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x3d0);
      uVar1 = *(undefined4 *)(this + 0x3d4);
      break;
    default:
      goto switchD_0005a8b2_default;
    }
  }
  else if (param_1 < 0x601) {
    if (param_1 == 0x5f3) {
      *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3d8);
      *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x3dc);
      uVar1 = *(undefined4 *)(this + 0x3e0);
    }
    else if (param_1 == 0x5f4) {
      *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3e4);
      *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 1000);
      uVar1 = *(undefined4 *)(this + 0x3ec);
    }
    else {
      if (param_1 != 0x5f6) {
        return 1;
      }
      *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3f0);
      *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x3f4);
      uVar1 = *(undefined4 *)(this + 0x3f8);
    }
  }
  else if (param_1 == 0x601) {
    *(undefined4 *)param_2 = *(undefined4 *)(this + 0x414);
    *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x418);
    uVar1 = *(undefined4 *)(this + 0x41c);
  }
  else if (param_1 == 0x605) {
    *(undefined4 *)param_2 = *(undefined4 *)(this + 0x3fc);
    *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x400);
    uVar1 = *(undefined4 *)(this + 0x404);
  }
  else {
    if (param_1 != 0x60a) {
      return 1;
    }
    *(undefined4 *)param_2 = *(undefined4 *)(this + 0x408);
    *(undefined4 *)(param_2 + 4) = *(undefined4 *)(this + 0x40c);
    uVar1 = *(undefined4 *)(this + 0x410);
  }
  *(undefined4 *)(param_2 + 8) = uVar1;
  uVar1 = 0;
switchD_0005a8b2_default:
  return uVar1;
}
