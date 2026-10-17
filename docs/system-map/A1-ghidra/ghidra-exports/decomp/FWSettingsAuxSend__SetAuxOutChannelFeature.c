// FWSettingsAuxSend::SetAuxOutChannelFeature
// Entry: 00020cd4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::SetAuxOutChannelFeature(unsigned int, unsigned char, unsigned int) */

undefined8 __thiscall
FWSettingsAuxSend::SetAuxOutChannelFeature
          (FWSettingsAuxSend *this,uint param_1,uchar param_2,uint param_3)

{
  uint uVar1;
  undefined8 uVar2;
  undefined4 local_38;
  uchar local_34;
  uchar local_33;
  undefined4 local_30;
  
  local_38 = 7;
  local_34 = '\0';
  local_33 = (uchar)param_1;
  local_30 = 0;
  if ((param_1 != 0) && (uVar1 = (**(code **)(**(long **)(this + 0x18) + 0x10))(), param_1 <= uVar1)
     ) {
    uVar2 = (**(code **)(**(long **)(this + 0x2d0) + 0xb48))(*(long **)(this + 0x2d0),&local_38);
    if ((int)uVar2 != 0) {
      return uVar2;
    }
    uVar2 = m_audio_b_FWAVCAudioDevice::AVCControlFeatureFunctionBlock
                      (*(m_audio_b_FWAVCAudioDevice **)(this + 0x2d0),local_34,param_2,local_33,
                       (ushort)(param_3 << 7));
    return uVar2;
  }
  return 0xe00002c2;
}
