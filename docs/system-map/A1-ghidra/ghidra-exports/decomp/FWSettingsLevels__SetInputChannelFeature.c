// FWSettingsLevels::SetInputChannelFeature
// Entry: 0002254c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SetInputChannelFeature(unsigned int, unsigned char, unsigned int) */

void __thiscall
FWSettingsLevels::SetInputChannelFeature
          (FWSettingsLevels *this,uint param_1,uchar param_2,uint param_3)

{
  undefined4 local_30;
  uchar local_2c;
  uchar local_2b;
  undefined4 local_28;
  
  local_30 = 6;
  local_2c = '\0';
  local_2b = (uchar)param_1;
  local_28 = 0;
  (**(code **)(**(long **)(this + 0xa58) + 0xb48))(*(long **)(this + 0xa58),&local_30);
  m_audio_b_FWAVCAudioDevice::AVCControlFeatureFunctionBlock
            (*(m_audio_b_FWAVCAudioDevice **)(this + 0xa58),local_2c,param_2,local_2b,
             (ushort)(param_3 << 7));
  return;
}
