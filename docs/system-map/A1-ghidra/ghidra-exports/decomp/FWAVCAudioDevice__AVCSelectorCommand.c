// m_audio_b_FWAVCAudioDevice::AVCSelectorCommand
// Entry: 00023074
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAVCAudioDevice::AVCSelectorCommand(unsigned char, unsigned char) */

void __thiscall
m_audio_b_FWAVCAudioDevice::AVCSelectorCommand
          (m_audio_b_FWAVCAudioDevice *this,uchar param_1,uchar param_2)

{
  undefined1 local_2c;
  undefined1 local_2b;
  undefined1 local_2a;
  undefined1 local_29;
  uchar local_28;
  undefined1 local_27;
  undefined1 local_26;
  uchar local_25;
  undefined1 local_24;
  undefined1 local_23;
  undefined1 local_22;
  undefined1 local_21;
  
  _bzero(&local_2c,0xc);
  local_26 = 2;
  local_24 = 1;
  local_23 = 0;
  local_22 = 0;
  local_21 = 0;
  local_2c = 0;
  local_2b = 8;
  local_2a = 0xb8;
  local_29 = 0x80;
  local_27 = 0x10;
  local_28 = param_1;
  local_25 = param_2;
  (**(code **)(**(long **)(this + 0x268) + 0x8b0))(*(long **)(this + 0x268),&local_2c,0xc,0);
  return;
}
