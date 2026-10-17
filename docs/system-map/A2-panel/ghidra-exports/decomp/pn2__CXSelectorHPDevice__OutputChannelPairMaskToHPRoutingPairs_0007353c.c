// CXSelectorHPDevice::OutputChannelPairMaskToHPRoutingPairs
// Entry: 0007353c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXSelectorHPDevice::OutputChannelPairMaskToHPRoutingPairs(unsigned long) */

uint __thiscall
CXSelectorHPDevice::OutputChannelPairMaskToHPRoutingPairs(CXSelectorHPDevice *this,ulong param_1)

{
  uint uVar1;
  uint uVar2;
  
  uVar2 = 0;
  do {
    if (3 < uVar2) {
      return 0;
    }
    uVar1 = uVar2 & 0x1f;
    uVar2 = uVar2 + 1;
  } while ((param_1 >> uVar1 & 1) == 0);
  return uVar2 & 0xffff;
}
