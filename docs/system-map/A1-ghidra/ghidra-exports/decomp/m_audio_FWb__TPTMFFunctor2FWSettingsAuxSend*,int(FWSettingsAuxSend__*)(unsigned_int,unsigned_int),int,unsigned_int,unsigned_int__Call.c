// m_audio_FWb::TPTMFFunctor2<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int),int,unsigned_int,unsigned_int>::Call
// Entry: 00020a84
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::TPTMFFunctor2<FWSettingsAuxSend*, int (FWSettingsAuxSend::*)(unsigned int, unsigned
   int), int, unsigned int, unsigned int>::Call(unsigned int, unsigned int) */

void __thiscall
m_audio_FWb::
TPTMFFunctor2<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int),int,unsigned_int,unsigned_int>
::Call(TPTMFFunctor2<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int),int,unsigned_int,unsigned_int>
       *this,uint param_1,uint param_2)

{
  code *UNRECOVERED_JUMPTABLE;
  
  UNRECOVERED_JUMPTABLE = *(code **)(this + 0x10);
  if (((ulong)UNRECOVERED_JUMPTABLE & 1) != 0) {
    UNRECOVERED_JUMPTABLE =
         *(code **)(UNRECOVERED_JUMPTABLE +
                   *(long *)(*(long *)(this + 8) + *(long *)(this + 0x18)) + -1);
  }
                    /* WARNING: Could not recover jumptable at 0x00020aa9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)
            (*(long *)(this + 8) + *(long *)(this + 0x18),param_1,param_2,UNRECOVERED_JUMPTABLE);
  return;
}
