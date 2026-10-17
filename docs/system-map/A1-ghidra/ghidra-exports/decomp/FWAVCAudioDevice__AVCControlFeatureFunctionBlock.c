// m_audio_b_FWAVCAudioDevice::AVCControlFeatureFunctionBlock
// Entry: 000231a8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAVCAudioDevice::AVCControlFeatureFunctionBlock(unsigned char, unsigned char, unsigned
   char, unsigned short) */

void __thiscall
m_audio_b_FWAVCAudioDevice::AVCControlFeatureFunctionBlock
          (m_audio_b_FWAVCAudioDevice *this,uchar param_1,uchar param_2,uchar param_3,ushort param_4
          )

{
  undefined1 local_3c;
  undefined1 local_3b;
  undefined1 local_3a;
  undefined1 local_39;
  uchar local_38;
  undefined1 local_37;
  undefined1 local_36;
  uchar local_35;
  uchar local_34;
  undefined1 local_33;
  undefined1 local_32;
  undefined1 local_31;
  
  _bzero(&local_3c,0xc);
  local_36 = 2;
  local_33 = 2;
  local_32 = (undefined1)(param_4 >> 8);
  local_31 = (undefined1)param_4;
  local_3c = 0;
  local_3b = 8;
  local_3a = 0xb8;
  local_39 = 0x81;
  local_37 = 0x10;
  local_38 = param_1;
  local_35 = param_3;
  local_34 = param_2;
  (**(code **)(**(long **)(this + 0x268) + 0x8b0))(*(long **)(this + 0x268),&local_3c,0xc,0);
  return;
}
