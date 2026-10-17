// FWSettingsLevels::SetAVCRouting
// Entry: 000225b4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsLevels::SetAVCRouting(m_audio_b_FWAVCAudioDevice::GenericBusType, unsigned int,
   unsigned int, unsigned int) */

ulong __thiscall
FWSettingsLevels::SetAVCRouting
          (FWSettingsLevels *this,undefined4 param_2,uint param_3,undefined4 param_4,uint param_5)

{
  uint uVar1;
  ulong uVar2;
  uint uVar3;
  undefined4 local_40;
  undefined4 local_3c;
  uint local_38;
  uchar local_34;
  uchar local_33;
  
  uVar2 = 0xe00002bc;
  uVar3 = 0;
  local_40 = param_2;
  while( true ) {
    if (param_3 <= uVar3) {
      return uVar2;
    }
    local_3c = param_4;
    local_38 = uVar3 + 1;
    uVar1 = (**(code **)(**(long **)(this + 0xa58) + 0xb40))(*(long **)(this + 0xa58),&local_40);
    uVar2 = (ulong)uVar1;
    if (uVar1 == 0xe00002d7) {
      uVar2 = 0;
    }
    if ((int)uVar2 != 0) break;
    uVar2 = m_audio_b_FWAVCAudioDevice::AVCMixerConnectPair
                      (*(m_audio_b_FWAVCAudioDevice **)(this + 0xa58),
                       (bool)((byte)(param_5 >> ((byte)uVar3 & 0x1f)) & 1),local_34,local_33,
                       (uchar)local_3c,(uchar)local_38);
    uVar3 = uVar3 + 1;
    if ((int)uVar2 != 0) {
      return uVar2;
    }
  }
  return uVar2;
}
