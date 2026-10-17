// CX1814Device::OutputChannelPairMaskToHPRoutingPairs
// Entry: 00072ea6
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CX1814Device::OutputChannelPairMaskToHPRoutingPairs(unsigned long) */

short __thiscall
CX1814Device::OutputChannelPairMaskToHPRoutingPairs(CX1814Device *this,ulong param_1)

{
  short sVar1;
  short sVar2;
  
  sVar1 = CXSelectorHPDevice::OutputChannelPairMaskToHPRoutingPairs
                    ((CXSelectorHPDevice *)this,param_1);
  sVar2 = 4;
  if (sVar1 != 3) {
    sVar2 = sVar1;
  }
  return sVar2;
}
