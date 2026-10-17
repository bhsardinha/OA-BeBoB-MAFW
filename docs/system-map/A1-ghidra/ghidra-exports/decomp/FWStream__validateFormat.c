// m_audio_b_FWStream::validateFormat
// Entry: 00011090
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWStream::validateFormat(_IOAudioStreamFormat*, _IOAudioStreamFormatExtension*,
   IOAudioStreamFormatDesc*) */

undefined1 __thiscall
m_audio_b_FWStream::validateFormat
          (m_audio_b_FWStream *this,_IOAudioStreamFormat *param_1,
          _IOAudioStreamFormatExtension *param_2,IOAudioStreamFormatDesc *param_3)

{
  long lVar1;
  long lVar2;
  long lVar3;
  uint uVar4;
  
  if (((param_1 != (_IOAudioStreamFormat *)0x0) && (lVar2 = *(long *)(this + 200), lVar2 != 0)) &&
     (*(uint *)(this + 0xd8) != 0)) {
    uVar4 = 0xffffffff;
    lVar1 = 0;
    while (lVar3 = lVar1, uVar4 = uVar4 + 1, uVar4 < *(uint *)(this + 0xd8)) {
      lVar1 = lVar3 + 0x48;
      if (((((*(int *)(lVar2 + lVar3) == *(int *)param_1) &&
            (*(int *)(lVar2 + 4 + lVar3) == *(int *)(param_1 + 4))) &&
           ((*(int *)(lVar2 + 8 + lVar3) == *(int *)(param_1 + 8) &&
            ((*(_IOAudioStreamFormat *)(lVar2 + 0xc + lVar3) == param_1[0xc] &&
             (*(_IOAudioStreamFormat *)(lVar2 + 0xd + lVar3) == param_1[0xd])))))) &&
          (*(_IOAudioStreamFormat *)(lVar2 + 0xe + lVar3) == param_1[0xe])) &&
         ((*(_IOAudioStreamFormat *)(lVar2 + 0xf + lVar3) == param_1[0xf] &&
          (*(_IOAudioStreamFormat *)(lVar2 + 0x10 + lVar3) == param_1[0x10])))) {
        *(undefined4 *)(param_1 + 0x14) = *(undefined4 *)(lVar2 + 0x14 + lVar3);
        if (param_3 == (IOAudioStreamFormatDesc *)0x0) {
          return 1;
        }
        _memcpy(param_3,(void *)(*(long *)(this + 200) + lVar3),0x48);
        return 1;
      }
    }
  }
  return 0;
}
