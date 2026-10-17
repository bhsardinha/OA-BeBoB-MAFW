// FWRev2AudioDevice::performPowerStateChange
// Entry: 00012b5c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e2b)

/* FWRev2AudioDevice::performPowerStateChange(_IOAudioDevicePowerState, _IOAudioDevicePowerState,
   unsigned int*) */

void FWRev2AudioDevice::performPowerStateChange(void)

{
  (*(code *)PTR_performPowerStateChange_000499a0)();
  return;
}
