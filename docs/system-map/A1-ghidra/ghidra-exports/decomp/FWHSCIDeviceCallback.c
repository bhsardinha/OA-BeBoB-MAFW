// FWHSCIDeviceCallback
// Entry: 0001d78c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* FWHSCIDeviceCallback(void*, int, IOFireWireNub*, IOFWCommand*) */

void FWHSCIDeviceCallback(void *param_1,int param_2,IOFireWireNub *param_3,IOFWCommand *param_4)

{
  m_audio_b_FWHSCI::HSCIReadComplete(param_1,param_2);
  return;
}
