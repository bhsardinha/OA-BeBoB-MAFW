// m_audio_b_FWBaseEngine::DisableAC3Format
// Entry: 0000596a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* m_audio_b_FWBaseEngine::DisableAC3Format() */

void __thiscall m_audio_b_FWBaseEngine::DisableAC3Format(m_audio_b_FWBaseEngine *this)

{
  m_audio_b_FWBaseEngine mVar1;
  long lVar2;
  
  mVar1 = this[0x2d3];
  lVar2 = (**(code **)(**(long **)(this + 0x138) + 0xa60))();
  if ((this[0x2d4] != (m_audio_b_FWBaseEngine)0x0) && (this[0x238] == (m_audio_b_FWBaseEngine)0x0))
  {
    if (mVar1 != (m_audio_b_FWBaseEngine)0x0) {
      (**(code **)(*(long *)this + 0xcc0))(this);
    }
    this[0x2d4] = (m_audio_b_FWBaseEngine)0x0;
    *(undefined4 *)(this + 0x2ec) = *(undefined4 *)(lVar2 + 0xc);
    ResetSampleBuffers(this);
    (**(code **)(*(long *)this + 0xc78))(this);
    if (mVar1 != (m_audio_b_FWBaseEngine)0x0) {
      (**(code **)(*(long *)this + 0xcb8))(this);
    }
                    /* WARNING: Could not recover jumptable at 0x00005a0a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(long **)(this + 0x138) + 0xb38))(*(long **)(this + 0x138),0);
    return;
  }
  return;
}
