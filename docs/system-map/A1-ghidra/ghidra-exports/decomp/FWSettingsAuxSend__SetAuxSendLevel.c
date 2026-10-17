// FWSettingsAuxSend::SetAuxSendLevel
// Entry: 00020d8c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* FWSettingsAuxSend::SetAuxSendLevel(unsigned int, unsigned int) */

void __thiscall
FWSettingsAuxSend::SetAuxSendLevel(FWSettingsAuxSend *this,uint param_1,uint param_2)

{
  int iVar1;
  undefined4 local_28;
  uchar local_24;
  uchar local_23;
  undefined4 local_20;
  
  local_28 = 0;
  local_24 = '\0';
  local_23 = (uchar)param_1;
  local_20 = 0;
  iVar1 = (**(code **)(**(long **)(this + 0x2d0) + 0xb48))(*(long **)(this + 0x2d0),&local_28);
  if (iVar1 == -0x1ffffd29) {
    iVar1 = 0;
  }
  if (iVar1 == 0) {
    m_audio_b_FWAVCAudioDevice::AVCControlFeatureFunctionBlock
              (*(m_audio_b_FWAVCAudioDevice **)(this + 0x2d0),local_24,'\x02',local_23,
               (ushort)(param_2 << 7));
  }
  return;
}
