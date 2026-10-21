// m_audio_FWb::TPTMFFunctor3<com_m_audio_FW410Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*),int,unsigned_int,unsigned_int,void*>::Call
// Entry: 00014b5a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_FWb::TPTMFFunctor3<com_m_audio_FW410Device*, int (m_audio_b_FWAVCAudioDevice::*)(unsigned
   int, unsigned int, void*), int, unsigned int, unsigned int, void*>::Call(unsigned int, unsigned
   int, void*) */

void m_audio_FWb::
     TPTMFFunctor3<com_m_audio_FW410Device*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int,unsigned_int,void*),int,unsigned_int,unsigned_int,void*>
     ::Call(uint param_1,uint param_2,void *param_3)

{
  undefined4 in_register_0000003c;
  long lVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  lVar1 = CONCAT44(in_register_0000003c,param_1);
  UNRECOVERED_JUMPTABLE = *(code **)(lVar1 + 0x10);
  if (((ulong)UNRECOVERED_JUMPTABLE & 1) != 0) {
    UNRECOVERED_JUMPTABLE =
         *(code **)(UNRECOVERED_JUMPTABLE +
                   *(long *)(*(long *)(lVar1 + 8) + *(long *)(lVar1 + 0x18)) + -1);
  }
                    /* WARNING: Could not recover jumptable at 0x00014b80. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(*(long *)(lVar1 + 8) + *(long *)(lVar1 + 0x18));
  return;
}
