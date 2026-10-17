// FWSettingsABSwitch::SendToDevice
// Entry: 0001df6c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsABSwitch::SendToDevice() */

undefined8 __thiscall FWSettingsABSwitch::SendToDevice(FWSettingsABSwitch *this)

{
  *(uint *)(this + 0x104) = (uint)(*(int *)(this + 0x104) != 0);
  return 0;
}
