// com_m_audio_FWAudiophileDevice::SetHeadphoneSelector
// Entry: 0000b6ee
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWAudiophileDevice::SetHeadphoneSelector(unsigned int, unsigned int) */

void __thiscall
com_m_audio_FWAudiophileDevice::SetHeadphoneSelector
          (com_m_audio_FWAudiophileDevice *this,uint param_1,uint param_2)

{
  int iVar1;
  uchar *puVar2;
  undefined4 local_48;
  uchar local_44;
  undefined1 local_43;
  undefined4 local_40;
  uchar *local_38;
  uint local_30;
  uchar local_2c [12];
  
  local_30 = 0xc;
  local_38 = local_2c;
  _bzero(local_38,0xc);
  local_48 = 8;
  local_44 = '\0';
  local_43 = (undefined1)param_1;
  local_40 = 0;
  iVar1 = (**(code **)(*(long *)this + 0xb48))(this,&local_48);
  if (iVar1 == 0) {
    puVar2 = local_38;
    if (6 < local_30) {
      puVar2 = local_38 + 6;
    }
    *puVar2 = '\x02';
    puVar2 = local_38;
    if (7 < local_30) {
      puVar2 = local_38 + 7;
    }
    *puVar2 = (uchar)param_2;
    puVar2 = local_38;
    if (8 < local_30) {
      puVar2 = local_38 + 8;
    }
    *puVar2 = '\x01';
    puVar2 = local_38;
    if (9 < local_30) {
      puVar2 = local_38 + 9;
    }
    *puVar2 = '\0';
    puVar2 = local_38;
    if (10 < local_30) {
      puVar2 = local_38 + 10;
    }
    *puVar2 = '\0';
    puVar2 = local_38;
    if (0xb < local_30) {
      puVar2 = local_38 + 0xb;
    }
    *puVar2 = '\0';
    m_audio_b_FWAVCAudioDevice::AVCControlFunctionBlock
              ((m_audio_b_FWAVCAudioDevice *)this,0x80,local_44,local_38,(uchar)local_30);
  }
  return;
}
