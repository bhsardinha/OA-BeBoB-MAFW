// com_m_audio_FW1814Device::SelectFunctionBlock
// Entry: 0000dd9e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SelectFunctionBlock(unsigned char&,
   com_m_audio_FW1814Device::SelectMap1814*, unsigned char&) */

undefined8 __thiscall
com_m_audio_FW1814Device::SelectFunctionBlock
          (com_m_audio_FW1814Device *this,uchar *param_1,SelectMap1814 *param_2,uchar *param_3)

{
  SelectMap1814 SVar1;
  byte bVar2;
  uint uVar3;
  long lVar4;
  
  uVar3 = 0;
  while( true ) {
    lVar4 = (ulong)uVar3 * 0x10;
    if (*(uint *)(param_2 + lVar4 + 4) == 0) {
      return 0xe00002c2;
    }
    if ((*(uint *)(param_2 + lVar4) <= (uint)*param_1) &&
       ((uint)*param_1 <= *(uint *)(param_2 + lVar4 + 4))) break;
    uVar3 = uVar3 + 1;
  }
  SVar1 = param_2[lVar4 + 8];
  *param_3 = (uchar)SVar1;
  if ((uVar3 != 0) && (SVar1 != param_2[(ulong)(uVar3 - 1) * 0x10 + 8])) {
    *param_1 = (*param_1 + '\x01') - (char)param_2[lVar4];
  }
  if (*(int *)(param_2 + lVar4 + 0xc) < 1) {
LAB_0000de12:
    return 0;
  }
  switch(*(int *)(param_2 + lVar4 + 0xc)) {
  case 1:
    bVar2 = (byte)this[0x144] & 8;
    break;
  case 2:
    bVar2 = (byte)this[0x144] & 0x10;
    break;
  case 3:
    bVar2 = ((byte)this[0x144] & 8) >> 3;
    goto LAB_0000de3a;
  case 4:
    bVar2 = ((byte)this[0x144] & 0x10) >> 4;
LAB_0000de3a:
    if (bVar2 == 0) {
      return 0xe00002d7;
    }
    return 0;
  default:
    goto LAB_0000de12;
  }
  if (bVar2 != 0) {
    return 0xe00002d7;
  }
  return 0;
}
