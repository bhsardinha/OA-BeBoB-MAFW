// FWRev2AudioDevice::stop
// Entry: 00013078
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* WARNING: Switch with 1 destination removed at 0x000130c2 */
/* FWRev2AudioDevice::stop(IOService*) */

void FWRev2AudioDevice::stop(IOService *param_1)

{
  long *plVar1;
  
  if (*(m_audio_b_FWHSCI **)(param_1 + 0x2c8) != (m_audio_b_FWHSCI *)0x0) {
    m_audio_b_FWHSCI::Release(*(m_audio_b_FWHSCI **)(param_1 + 0x2c8));
    (**(code **)(**(long **)(param_1 + 0x2c8) + 0x28))();
    *(undefined8 *)(param_1 + 0x2c8) = 0;
  }
  plVar1 = *(long **)(param_1 + 0x268);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x600))(plVar1,1);
    *(undefined8 *)(param_1 + 0x268) = 0;
  }
  (*(code *)PTR_stop_00031738)(param_1);
  return;
}
