// m_audio_b_FWAVCUnit::CheckAVCError
// Entry: 000014b6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWAVCUnit::CheckAVCError(int, unsigned char*) */

int __thiscall
m_audio_b_FWAVCUnit::CheckAVCError(m_audio_b_FWAVCUnit *this,int param_1,uchar *param_2)

{
  byte bVar1;
  undefined8 uVar2;
  
  if (param_1 == 0) {
    bVar1 = *param_2;
    if (bVar1 - 8 < 8) {
      if (bVar1 != 0xf) {
        if (bVar1 == 10) {
          return -0x1ffffd3e;
        }
        if (bVar1 != 8) {
          return 0;
        }
        return -0x1ffffd39;
      }
      uVar2 = 500;
    }
    else {
      uVar2 = 1000;
    }
    _IOSleep(uVar2);
  }
  return param_1;
}
