// m_audio_FWb::TPTMFFunctor1<m_audio_b_FWAVCAudioDevice*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int),int,unsigned_int>::Call
// Entry: 00022bc2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* m_audio_FWb::TPTMFFunctor1<m_audio_b_FWAVCAudioDevice*, int
   (m_audio_b_FWAVCAudioDevice::*)(unsigned int), int, unsigned int>::Call(unsigned int) */

void __thiscall
m_audio_FWb::
TPTMFFunctor1<m_audio_b_FWAVCAudioDevice*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int),int,unsigned_int>
::Call(TPTMFFunctor1<m_audio_b_FWAVCAudioDevice*,int(m_audio_b_FWAVCAudioDevice::*)(unsigned_int),int,unsigned_int>
       *this,uint param_1)

{
  long lVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  UNRECOVERED_JUMPTABLE = *(code **)(this + 0x10);
  lVar1 = *(long *)(this + 0x18);
  if (((ulong)UNRECOVERED_JUMPTABLE & 1) != 0) {
    UNRECOVERED_JUMPTABLE =
         *(code **)(UNRECOVERED_JUMPTABLE + *(long *)(*(long *)(this + 8) + lVar1) + -1);
  }
                    /* WARNING: Could not recover jumptable at 0x00022be7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(*(long *)(this + 8) + lVar1,param_1,lVar1,UNRECOVERED_JUMPTABLE);
  return;
}
