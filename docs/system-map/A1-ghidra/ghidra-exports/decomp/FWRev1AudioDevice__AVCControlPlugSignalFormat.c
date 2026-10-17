// FWRev1AudioDevice::AVCControlPlugSignalFormat
// Entry: 00013734
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* FWRev1AudioDevice::AVCControlPlugSignalFormat(m_audio_b_FWAVCAudioDevice::PlugDirection, unsigned
   char, FDFValue, unsigned int, m_audio_b_FWAVCAudioDevice::PlugFormat) */

void FWRev1AudioDevice::AVCControlPlugSignalFormat
               (long param_1,int param_2,byte param_3,byte param_4)

{
  byte *pbVar1;
  undefined1 local_54 [12];
  byte *local_48;
  uint local_40;
  byte local_3c [16];
  undefined4 local_2c;
  
  local_40 = 8;
  local_48 = local_3c;
  _bzero(local_48,8);
  _bzero(local_54,8);
  *local_48 = 0;
  pbVar1 = local_48;
  if (1 < local_40) {
    pbVar1 = local_48 + 1;
  }
  *pbVar1 = 0xff;
  pbVar1 = local_48;
  if (2 < local_40) {
    pbVar1 = local_48 + 2;
  }
  *pbVar1 = param_2 != 0 | 0x18;
  pbVar1 = local_48;
  if (3 < local_40) {
    pbVar1 = local_48 + 3;
  }
  *pbVar1 = param_3;
  pbVar1 = local_48;
  if (4 < local_40) {
    pbVar1 = local_48 + 4;
  }
  *pbVar1 = 0x90;
  pbVar1 = local_48;
  if (5 < local_40) {
    pbVar1 = local_48 + 5;
  }
  *pbVar1 = param_4;
  pbVar1 = local_48;
  if (6 < local_40) {
    pbVar1 = local_48 + 6;
  }
  *pbVar1 = 0xff;
  pbVar1 = local_48;
  if (7 < local_40) {
    pbVar1 = local_48 + 7;
  }
  *pbVar1 = 0xff;
  local_2c = 8;
  (**(code **)(**(long **)(param_1 + 0x268) + 0x8b8))
            (*(long **)(param_1 + 0x268),local_48,8,local_54,&local_2c);
  return;
}
