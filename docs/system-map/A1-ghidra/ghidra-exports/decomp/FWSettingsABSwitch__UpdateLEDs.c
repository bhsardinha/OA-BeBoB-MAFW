// FWSettingsABSwitch::UpdateLEDs
// Entry: 0001ddbe
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::UpdateLEDs() */

void __thiscall FWSettingsABSwitch::UpdateLEDs(FWSettingsABSwitch *this)

{
  undefined8 uVar1;
  
  if (*(int *)(this + 0x104) == 0) {
    if (*(int *)(this + 0xd0) == 1) {
      uVar1 = 2;
    }
    else if (*(int *)(this + 0xd0) == 0) {
      uVar1 = 1;
    }
    else {
      uVar1 = 3;
    }
  }
  else {
    uVar1 = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x0001ddfc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)**(undefined8 **)(this + 0x18))(*(undefined8 **)(this + 0x18),uVar1);
  return;
}
