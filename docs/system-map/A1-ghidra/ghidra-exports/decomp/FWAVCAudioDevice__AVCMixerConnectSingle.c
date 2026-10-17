// m_audio_b_FWAVCAudioDevice::AVCMixerConnectSingle
// Entry: 00023586
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e9)

/* m_audio_b_FWAVCAudioDevice::AVCMixerConnectSingle(bool, unsigned char, unsigned char, unsigned
   char, unsigned char) */

void __thiscall
m_audio_b_FWAVCAudioDevice::AVCMixerConnectSingle
          (m_audio_b_FWAVCAudioDevice *this,bool param_1,uchar param_2,uchar param_3,uchar param_4,
          uchar param_5)

{
  undefined1 local_44;
  undefined1 local_43;
  undefined1 local_42;
  undefined1 local_41;
  uchar local_40;
  undefined1 local_3f;
  undefined1 local_3e;
  uchar local_3d;
  uchar local_3c;
  uchar local_3b;
  undefined1 local_3a;
  undefined1 local_39;
  char local_38;
  undefined1 local_37;
  undefined1 local_36;
  undefined1 local_35;
  
  _bzero(&local_44,0x10);
  local_39 = 2;
  local_38 = !param_1 << 7;
  local_37 = 0;
  local_36 = 0;
  local_35 = 0;
  local_3e = 4;
  local_3a = 3;
  local_44 = 0;
  local_43 = 8;
  local_42 = 0xb8;
  local_41 = 0x82;
  local_3f = 0x10;
  local_40 = param_2;
  local_3d = param_3;
  local_3c = param_4;
  local_3b = param_5;
  (**(code **)(**(long **)(this + 0x268) + 0x8b0))(*(long **)(this + 0x268),&local_44,0x10,0);
  return;
}
