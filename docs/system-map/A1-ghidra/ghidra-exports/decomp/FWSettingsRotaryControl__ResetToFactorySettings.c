// FWSettingsRotaryControl::ResetToFactorySettings
// Entry: 00021060
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsRotaryControl::ResetToFactorySettings() */

undefined8 __thiscall FWSettingsRotaryControl::ResetToFactorySettings(FWSettingsRotaryControl *this)

{
  *(undefined8 *)(this + 0x68) = 0xffff00000000;
  return 0;
}
