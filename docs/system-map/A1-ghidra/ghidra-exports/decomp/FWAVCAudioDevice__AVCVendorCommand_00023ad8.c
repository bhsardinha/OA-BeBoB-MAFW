// m_audio_b_FWAVCAudioDevice::AVCVendorCommand
// Entry: 00023ad8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i2)

/* m_audio_b_FWAVCAudioDevice::AVCVendorCommand(unsigned char, unsigned char, unsigned char) */

void m_audio_b_FWAVCAudioDevice::AVCVendorCommand(uchar param_1,uchar param_2,uchar param_3)

{
  undefined3 in_register_00000011;
  
  AVCVendorCommand(param_1,param_2,CONCAT31(in_register_00000011,param_3),
                   (uchar *)((long)&MACH_HEADER.magic + 1));
  return;
}
