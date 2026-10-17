// CXSelectorHPDevice::HPRoutingPairsToOutputChannelPairMask
// Entry: 0007355c
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CXSelectorHPDevice::HPRoutingPairsToOutputChannelPairMask(unsigned short) */

int __thiscall
CXSelectorHPDevice::HPRoutingPairsToOutputChannelPairMask(CXSelectorHPDevice *this,ushort param_1)

{
  char cVar1;
  
  cVar1 = (**(code **)(*(int *)this + 0x78))(this,param_1);
  return 1 << (cVar1 - 1U & 0x1f);
}
