// CX1814Device::TranslateHPRoutingPairsForBitConversion
// Entry: 00072e8e
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CX1814Device::TranslateHPRoutingPairsForBitConversion(unsigned short) */

short __thiscall
CX1814Device::TranslateHPRoutingPairsForBitConversion(CX1814Device *this,ushort param_1)

{
  return param_1 - (param_1 == 4);
}
