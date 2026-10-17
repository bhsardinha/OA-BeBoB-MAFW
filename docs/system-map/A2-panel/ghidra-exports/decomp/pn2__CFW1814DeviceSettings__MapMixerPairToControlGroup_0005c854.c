// CFW1814DeviceSettings::MapMixerPairToControlGroup
// Entry: 0005c854
// Source: Ghidra 12.1.4 headless DecompList.java on panel-app, 2026-10-16 (PN.2 open items)

/* CFW1814DeviceSettings::MapMixerPairToControlGroup(unsigned short) */

undefined2 __thiscall
CFW1814DeviceSettings::MapMixerPairToControlGroup(CFW1814DeviceSettings *this,ushort param_1)

{
  if (param_1 - 1 < 0x10) {
    return *(undefined2 *)(&DAT_000f875e + (uint)param_1 * 2);
  }
  return 1;
}
