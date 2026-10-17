// CFW1814DeviceSettings::CalculateOutputPairBalanceLevel
// Entry: 0005a134
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::CalculateOutputPairBalanceLevel(unsigned short) */

int __thiscall
CFW1814DeviceSettings::CalculateOutputPairBalanceLevel(CFW1814DeviceSettings *this,ushort param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  ushort local_c;
  ushort local_a;
  
  (**(code **)(*(int *)this + 0x24))(this,param_1,&local_a,&local_c);
  iVar3 = (*(int *)(this + 0x3f4) - *(int *)(this + 0x3f0)) / 2 + *(int *)(this + 0x3f0);
  iVar1 = *(int *)(this + (uint)local_c * 4 + 0x268);
  iVar2 = *(int *)(this + (uint)local_a * 4 + 0x268);
  if ((iVar1 < iVar2) || (iVar2 < iVar1)) {
    iVar3 = (iVar3 - iVar2) + iVar1;
  }
  return iVar3;
}
