// m_audio_b_FWAudioDevice::GetSignalTypeForChannelPair
// Entry: 00001e06
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWAudioDevice::GetSignalTypeForChannelPair(bool, unsigned int) */

int __thiscall
m_audio_b_FWAudioDevice::GetSignalTypeForChannelPair
          (m_audio_b_FWAudioDevice *this,bool param_1,uint param_2)

{
  int iVar1;
  long lVar2;
  uint uVar3;
  long lVar4;
  
  lVar2 = (**(code **)(*(long *)this + 0xa60))();
  uVar3 = 0;
  while( true ) {
    lVar4 = (ulong)uVar3 * 0x10;
    iVar1 = *(int *)(lVar2 + 0x4c + lVar4);
    if (iVar1 == 3) {
      return 0;
    }
    if ((((bool)*(char *)(lVar2 + 0x40 + lVar4) == param_1) &&
        (*(uint *)(lVar2 + 0x44 + lVar4) <= param_2)) &&
       (param_2 <= *(uint *)(lVar2 + 0x48 + lVar4))) break;
    uVar3 = uVar3 + 1;
  }
  return iVar1;
}
