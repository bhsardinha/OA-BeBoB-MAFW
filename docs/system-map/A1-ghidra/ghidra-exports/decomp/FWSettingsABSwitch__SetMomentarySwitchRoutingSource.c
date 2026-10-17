// FWSettingsABSwitch::SetMomentarySwitchRoutingSource
// Entry: 0001e34c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SetMomentarySwitchRoutingSource(unsigned int, unsigned int) */

undefined8 __thiscall
FWSettingsABSwitch::SetMomentarySwitchRoutingSource
          (FWSettingsABSwitch *this,uint param_1,uint param_2)

{
  if (param_1 - 1 == *(int *)(this + 0xd0)) {
    *(uint *)(this + 0x144) = param_2;
    SubmitHeadphoneSourceChange(this);
  }
  return 0;
}
