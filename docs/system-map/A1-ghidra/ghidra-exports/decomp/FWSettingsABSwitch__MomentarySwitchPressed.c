// FWSettingsABSwitch::MomentarySwitchPressed
// Entry: 0001e54c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::MomentarySwitchPressed() */

void __thiscall FWSettingsABSwitch::MomentarySwitchPressed(FWSettingsABSwitch *this)

{
  *(undefined4 *)(this + 0x104) = 0;
  if (*(int *)(this + 0xd0) == 1) {
    *(undefined4 *)(this + 0xd0) = 0;
  }
  else if (*(int *)(this + 0xd0) == 0) {
    *(undefined4 *)(this + 0xd0) = 1;
  }
  SendMixerSettingsForCurrentMomentaryMode(this);
  UpdateLEDs(this);
  return;
}
