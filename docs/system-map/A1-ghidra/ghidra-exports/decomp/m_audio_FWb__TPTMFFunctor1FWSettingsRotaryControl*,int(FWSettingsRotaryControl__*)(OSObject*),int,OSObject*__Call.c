// m_audio_FWb::TPTMFFunctor1<FWSettingsRotaryControl*,int(FWSettingsRotaryControl::*)(OSObject*),int,OSObject*>::Call
// Entry: 0002109e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::TPTMFFunctor1<FWSettingsRotaryControl*, int (FWSettingsRotaryControl::*)(OSObject*),
   int, OSObject*>::Call(OSObject*) */

void __thiscall
m_audio_FWb::
TPTMFFunctor1<FWSettingsRotaryControl*,int(FWSettingsRotaryControl::*)(OSObject*),int,OSObject*>::
Call(TPTMFFunctor1<FWSettingsRotaryControl*,int(FWSettingsRotaryControl::*)(OSObject*),int,OSObject*>
     *this,OSObject *param_1)

{
  long lVar1;
  code *UNRECOVERED_JUMPTABLE;
  
  UNRECOVERED_JUMPTABLE = *(code **)(this + 0x10);
  lVar1 = *(long *)(this + 0x18);
  if (((ulong)UNRECOVERED_JUMPTABLE & 1) != 0) {
    UNRECOVERED_JUMPTABLE =
         *(code **)(UNRECOVERED_JUMPTABLE + *(long *)(*(long *)(this + 8) + lVar1) + -1);
  }
                    /* WARNING: Could not recover jumptable at 0x000210c3. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(*(long *)(this + 8) + lVar1,param_1,lVar1,UNRECOVERED_JUMPTABLE);
  return;
}
