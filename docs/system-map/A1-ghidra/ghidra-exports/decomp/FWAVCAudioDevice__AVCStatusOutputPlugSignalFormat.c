// m_audio_b_FWAVCAudioDevice::AVCStatusOutputPlugSignalFormat
// Entry: 00023374
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* WARNING: Removing unreachable block (ram,0x00023470) */
/* m_audio_b_FWAVCAudioDevice::AVCStatusOutputPlugSignalFormat(unsigned char, FDFValue&) */

undefined4 __thiscall
m_audio_b_FWAVCAudioDevice::AVCStatusOutputPlugSignalFormat
          (m_audio_b_FWAVCAudioDevice *this,uchar param_1,FDFValue *param_2)

{
  long *plVar1;
  undefined4 uVar2;
  uchar *puVar3;
  char local_4c [5];
  byte local_47;
  uchar *local_40;
  uint local_38;
  uchar local_34 [16];
  undefined4 local_24;
  
  local_38 = 8;
  local_40 = local_34;
  _bzero(local_40,8);
  _bzero(local_4c,8);
  *local_40 = '\x01';
  puVar3 = local_40;
  if (1 < local_38) {
    puVar3 = local_40 + 1;
  }
  *puVar3 = 0xff;
  puVar3 = local_40;
  if (2 < local_38) {
    puVar3 = local_40 + 2;
  }
  *puVar3 = '\x18';
  puVar3 = local_40;
  if (3 < local_38) {
    puVar3 = local_40 + 3;
  }
  *puVar3 = param_1;
  puVar3 = local_40;
  if (4 < local_38) {
    puVar3 = local_40 + 4;
  }
  *puVar3 = 0xff;
  puVar3 = local_40;
  if (5 < local_38) {
    puVar3 = local_40 + 5;
  }
  *puVar3 = 0xff;
  puVar3 = local_40;
  if (6 < local_38) {
    puVar3 = local_40 + 6;
  }
  *puVar3 = 0xff;
  puVar3 = local_40;
  if (7 < local_38) {
    puVar3 = local_40 + 7;
  }
  *puVar3 = 0xff;
  local_24 = 8;
  plVar1 = *(long **)(this + 0x268);
  if (plVar1 == (long *)0x0) {
    uVar2 = 0xe00002c2;
  }
  else {
    uVar2 = (**(code **)(*plVar1 + 0x8b8))(plVar1,local_40,8,local_4c,&local_24);
    *(uint *)param_2 = (uint)local_47;
    if (local_4c[0] != '\f') {
      uVar2 = 0xe00002c7;
    }
  }
  return uVar2;
}
