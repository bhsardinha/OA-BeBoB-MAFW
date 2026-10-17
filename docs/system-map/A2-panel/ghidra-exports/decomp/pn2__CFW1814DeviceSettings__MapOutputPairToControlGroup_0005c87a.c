// CFW1814DeviceSettings::MapOutputPairToControlGroup
// Entry: 0005c87a
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::MapOutputPairToControlGroup(unsigned short) */

undefined2 __thiscall
CFW1814DeviceSettings::MapOutputPairToControlGroup(CFW1814DeviceSettings *this,ushort param_1)

{
  if (param_1 - 1 < 10) {
    return *(undefined2 *)(&DAT_000f877e + (uint)param_1 * 2);
  }
  return 2;
}
