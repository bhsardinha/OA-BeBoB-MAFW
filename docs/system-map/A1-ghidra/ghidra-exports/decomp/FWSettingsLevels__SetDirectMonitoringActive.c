// FWSettingsLevels::SetDirectMonitoringActive
// Entry: 000217ec
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SetDirectMonitoringActive(unsigned int) */

undefined8 FWSettingsLevels::SetDirectMonitoringActive(uint param_1)

{
  undefined4 in_register_0000003c;
  long lVar1;
  
  lVar1 = CONCAT44(in_register_0000003c,param_1);
  *(undefined4 *)(lVar1 + 0x11c) = 1;
  *(undefined1 *)(*(long *)(lVar1 + 0x8c0) + 0x11) = 0;
  (**(code **)(**(long **)(lVar1 + 0x8c0) + 0x28))(*(long **)(lVar1 + 0x8c0),0);
  return 0;
}
