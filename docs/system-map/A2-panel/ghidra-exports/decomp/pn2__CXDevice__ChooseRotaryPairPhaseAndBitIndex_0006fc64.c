// CXDevice::ChooseRotaryPairPhaseAndBitIndex
// Entry: 0006fc64
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXDevice::ChooseRotaryPairPhaseAndBitIndex(unsigned long, unsigned short&, unsigned long&) */

void __thiscall
CXDevice::ChooseRotaryPairPhaseAndBitIndex
          (CXDevice *this,ulong param_1,ushort *param_2,ulong *param_3)

{
  ushort uVar1;
  ulong uVar2;
  uint uVar3;
  
  if (param_1 == 0x5f7) {
    *param_3 = 0x20;
    uVar2 = 0x1f;
  }
  else {
    uVar1 = *param_2;
    if (uVar1 != 0) {
      uVar3 = (**(code **)(*(int *)this + 0x5c))(this);
      if (uVar1 <= (ushort)((uVar3 & 0xfffe) >> 1)) {
        *param_3 = 0x10;
        uVar2 = 0xf;
        goto LAB_0006fcd8;
      }
    }
    uVar3 = (**(code **)(*(int *)this + 0x5c))(this);
    *param_2 = *param_2 - (short)((uVar3 & 0xfffe) >> 1);
    *param_3 = 0;
    uVar2 = 0xffffffff;
  }
LAB_0006fcd8:
  *param_3 = uVar2;
  return;
}
