// m_audio_b_FWAudioDevice::DetectExternalSyncChanges
// Entry: 00002aca
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWAudioDevice::DetectExternalSyncChanges(bool, FDFValue) */

void __thiscall
m_audio_b_FWAudioDevice::DetectExternalSyncChanges
          (m_audio_b_FWAudioDevice *this,m_audio_b_FWAudioDevice param_1,int param_3)

{
  char *pcVar1;
  uint uVar2;
  
  if (*(int *)(this + 0x25c) != param_3) {
    *(int *)(this + 0x25c) = param_3;
    switch(param_3) {
    case 0:
      uVar2 = 32000;
      break;
    case 1:
      uVar2 = 0xac44;
      break;
    case 2:
      uVar2 = 48000;
      break;
    case 3:
      uVar2 = 0x15888;
      break;
    case 4:
      uVar2 = 96000;
      break;
    case 5:
      uVar2 = 0x2b110;
      break;
    case 6:
      uVar2 = 0x2ee00;
      break;
    default:
      pcVar1 = " mismatch";
      if (param_1 == (m_audio_b_FWAudioDevice)0x0) {
        pcVar1 = "";
      }
      _IOLog("M-FW: invalid sample rate detected (FDF:%X)%s\n",param_3,pcVar1);
      if (*(long *)(this + 0xe8) == 0) {
        uVar2 = 0;
      }
      else {
        uVar2 = *(uint *)(*(long *)(this + 0xe8) + 0x220);
      }
      *(undefined4 *)(this + 0x25c) = 0xff;
    }
    SetStaticPropertyAndNotifyUserClients(this,"MASampleRate",uVar2);
  }
  if (((param_1 != (m_audio_b_FWAudioDevice)0x0) && (*(long *)(this + 0xe8) != 0)) &&
     (*(uint *)(this + 0x25c) != (uint)*(byte *)(*(long *)(this + 0xe8) + 0x228))) {
    param_1 = (m_audio_b_FWAudioDevice)0x0;
  }
  if (param_1 != this[0x25a]) {
    this[0x25a] = param_1;
    SetStaticPropertyAndNotifyUserClients(this,"MARateLocked",(uint)(byte)param_1);
    return;
  }
  return;
}
