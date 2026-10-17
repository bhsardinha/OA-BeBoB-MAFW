// FWSettingsABSwitch::SetMomentarySwitchSource
// Entry: 0001e53e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SetMomentarySwitchSource(unsigned int) */

undefined8 FWSettingsABSwitch::SetMomentarySwitchSource(uint param_1)

{
  undefined4 in_register_0000003c;
  
  SendMixerSettingsForCurrentMomentaryMode
            ((FWSettingsABSwitch *)CONCAT44(in_register_0000003c,param_1));
  return 0;
}
