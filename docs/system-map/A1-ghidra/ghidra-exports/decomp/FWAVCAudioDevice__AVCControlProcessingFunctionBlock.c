// m_audio_b_FWAVCAudioDevice::AVCControlProcessingFunctionBlock
// Entry: 00022ac6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_b_FWAVCAudioDevice::AVCControlProcessingFunctionBlock(unsigned char, unsigned char,
   unsigned char, unsigned char, unsigned char, unsigned char*, unsigned char) */

void __thiscall
m_audio_b_FWAVCAudioDevice::AVCControlProcessingFunctionBlock
          (m_audio_b_FWAVCAudioDevice *this,uchar param_1,uchar param_2,uchar param_3,uchar param_4,
          uchar param_5,uchar *param_6,uchar param_7)

{
  code *UNRECOVERED_JUMPTABLE;
  
  param_6[6] = '\x04';
  param_6[7] = param_3;
  param_6[8] = param_4;
  param_6[9] = param_5;
  param_6[10] = param_2;
  *param_6 = '\0';
  param_6[1] = '\b';
  param_6[2] = 0xb8;
  param_6[3] = 0x82;
  param_6[4] = param_1;
  param_6[5] = '\x10';
  UNRECOVERED_JUMPTABLE = *(code **)(**(long **)(this + 0x268) + 0x8b0);
                    /* WARNING: Could not recover jumptable at 0x00022b12. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(*(long **)(this + 0x268),param_6,param_7,0,UNRECOVERED_JUMPTABLE);
  return;
}
