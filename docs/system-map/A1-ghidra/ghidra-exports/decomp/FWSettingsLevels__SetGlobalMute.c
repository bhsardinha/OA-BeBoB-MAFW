// FWSettingsLevels::SetGlobalMute
// Entry: 00021818
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SetGlobalMute(unsigned int) */

undefined8 __thiscall FWSettingsLevels::SetGlobalMute(FWSettingsLevels *this,uint param_1)

{
  *(uint *)(this + 0x9c) = (uint)(param_1 != 0);
  *(bool *)(*(long *)(this + 0x8b8) + 0x11) = param_1 != 0;
  (**(code **)(**(long **)(this + 0x8b8) + 0x28))(*(long **)(this + 0x8b8),0);
  return 0;
}
