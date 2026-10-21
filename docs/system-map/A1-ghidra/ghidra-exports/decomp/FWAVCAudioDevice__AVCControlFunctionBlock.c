// m_audio_b_FWAVCAudioDevice::AVCControlFunctionBlock
// Entry: 00022a90
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_b_FWAVCAudioDevice::AVCControlFunctionBlock(unsigned char, unsigned char, unsigned char*,
   unsigned char) */

void __thiscall
m_audio_b_FWAVCAudioDevice::AVCControlFunctionBlock
          (m_audio_b_FWAVCAudioDevice *this,uchar param_1,uchar param_2,uchar *param_3,uchar param_4
          )

{
  undefined3 in_register_00000081;
  
  *param_3 = '\0';
  param_3[1] = '\b';
  param_3[2] = 0xb8;
  param_3[3] = param_1;
  param_3[4] = param_2;
  param_3[5] = '\x10';
                    /* WARNING: Could not recover jumptable at 0x00022ac4. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)(this + 0x268) + 0x8b0))
            (*(long **)(this + 0x268),param_3,CONCAT31(in_register_00000081,param_4),0);
  return;
}
