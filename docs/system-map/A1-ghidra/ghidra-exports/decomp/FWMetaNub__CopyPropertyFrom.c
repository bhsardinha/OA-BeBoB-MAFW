// com_m_audio_FWMetaNub::CopyPropertyFrom
// Entry: 0001b7de
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::CopyPropertyFrom(IOService*, char const*) */

void __thiscall
com_m_audio_FWMetaNub::CopyPropertyFrom
          (com_m_audio_FWMetaNub *this,IOService *param_1,char *param_2)

{
  long lVar1;
  
  lVar1 = (**(code **)(*(long *)param_1 + 0x2b8))(param_1,param_2);
  if (lVar1 == 0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x0001b81b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0x268))(this,param_2,lVar1,*(code **)(*(long *)this + 0x268));
  return;
}
