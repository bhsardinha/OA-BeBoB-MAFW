// m_audio_b_FWAudioDevice::stop
// Entry: 00002094
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e5)

/* m_audio_b_FWAudioDevice::stop(IOService*) */

void m_audio_b_FWAudioDevice::stop(IOService *param_1)

{
  RemoveFromDeviceList((m_audio_b_FWAudioDevice *)param_1);
  if (*(long **)(param_1 + 0xf8) != (long *)0x0) {
    (**(code **)(**(long **)(param_1 + 0xf8) + 0x1c0))();
    *(undefined8 *)(param_1 + 0xf8) = 0;
  }
  if (*(long **)(param_1 + 0xf0) != (long *)0x0) {
    (**(code **)(**(long **)(param_1 + 0xf0) + 0x28))();
    *(undefined8 *)(param_1 + 0xf0) = 0;
  }
  if ((param_1[600] != (IOService)0x0) && (*(long *)(param_1 + 0x70) != 0)) {
    (*(code *)PTR_stop_00047858)(param_1);
  }
  param_1[600] = (IOService)0x0;
  return;
}
