// m_audio_b_FWAVCUnit::start
// Entry: 00001692
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCUnit::start(IOService*) */

IOService m_audio_b_FWAVCUnit::start(IOService *param_1)

{
  IOService IVar1;
  
  IVar1 = param_1[0xe0];
  if (IVar1 == (IOService)0x0) {
    IVar1 = (IOService)(**(code **)(PTR_vtable_000300d0 + 0x5d0))(param_1);
    param_1[0xe0] = IVar1;
  }
  return IVar1;
}
