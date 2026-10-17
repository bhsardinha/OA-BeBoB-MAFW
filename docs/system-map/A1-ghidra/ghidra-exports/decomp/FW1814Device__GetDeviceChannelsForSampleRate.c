// com_m_audio_FW1814Device::GetDeviceChannelsForSampleRate
// Entry: 0000c998
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* com_m_audio_FW1814Device::GetDeviceChannelsForSampleRate(unsigned int) const */

undefined ** __thiscall
com_m_audio_FW1814Device::GetDeviceChannelsForSampleRate
          (com_m_audio_FW1814Device *this,uint param_1)

{
  uint uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  if ((int)param_1 < 0x2ee00) {
    if ((param_1 == 0x15888) || (param_1 == 96000)) {
      uVar1 = *(uint *)(this + 0x144);
      puVar2 = &sADAT88_96DeviceChannels;
      goto LAB_0000c9fd;
    }
    if (param_1 == 0x2b110) {
      return (undefined **)&s176_192DeviceChannels;
    }
  }
  else if (param_1 == 0x2ee00) {
    return (undefined **)&s176_192DeviceChannels;
  }
  uVar1 = *(uint *)(this + 0x144);
  puVar2 = &sADAT44_48DeviceChannels;
LAB_0000c9fd:
  puVar4 = &sSPDIF44_96DeviceChannels;
  if ((uVar1 & 0x10) != 0) {
    puVar4 = puVar2;
  }
  puVar3 = &sSPDIF44_96DeviceChannels;
  if ((uVar1 & 8) != 0) {
    puVar3 = puVar2;
  }
  sDeviceChannels._0_4_ = *puVar3;
  _DAT_0004ac68 = puVar4[2];
  sDeviceChannels._4_4_ = puVar3[1];
  _DAT_0004ac6c = puVar4[3];
  DAT_0004aca0 = *(undefined1 *)(puVar3 + 0x10);
  _DAT_0004aca4 = puVar3[0x11];
  _DAT_0004aca8 = puVar3[0x12];
  _DAT_0004acac = puVar3[0x13];
  DAT_0004acb0 = *(undefined1 *)(puVar3 + 0x14);
  _DAT_0004acb4 = puVar3[0x15];
  _DAT_0004acb8 = puVar3[0x16];
  _DAT_0004acbc = puVar3[0x17];
  DAT_0004acc0 = *(undefined1 *)(puVar4 + 0x18);
  _DAT_0004acc4 = puVar4[0x19];
  _DAT_0004acc8 = puVar4[0x1a];
  _DAT_0004accc = puVar4[0x1b];
  DAT_0004acd0 = *(undefined1 *)(puVar4 + 0x1c);
  _DAT_0004acd4 = puVar4[0x1d];
  _DAT_0004acd8 = puVar4[0x1e];
  _DAT_0004acdc = puVar4[0x1f];
  return &sDeviceChannels;
}
