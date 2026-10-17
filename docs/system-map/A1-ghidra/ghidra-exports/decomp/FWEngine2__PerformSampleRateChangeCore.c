// com_m_audio_FWEngine2::PerformSampleRateChangeCore
// Entry: 000117e6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::PerformSampleRateChangeCore(_IOAudioSampleRate const*) */

undefined1 __thiscall
com_m_audio_FWEngine2::PerformSampleRateChangeCore
          (com_m_audio_FWEngine2 *this,_IOAudioSampleRate *param_1)

{
  undefined1 uVar1;
  
  if (((this[0x2d3] == (com_m_audio_FWEngine2)0x0) || (this[0x348] != (com_m_audio_FWEngine2)0x0))
     || ((this[0x2d0] != (com_m_audio_FWEngine2)0x0 && (this[0x2d1] == (com_m_audio_FWEngine2)0x0)))
     ) {
    uVar1 = (*(code *)PTR_PerformSampleRateChangeCore_000336f0)(this,param_1);
  }
  else {
    (**(code **)(*(long *)this + 0xcc0))(this);
    uVar1 = (*(code *)PTR_PerformSampleRateChangeCore_000336f0)(this,param_1);
    this[0x345] = (com_m_audio_FWEngine2)((byte)this[0x345] | 0x80);
  }
  return uVar1;
}
