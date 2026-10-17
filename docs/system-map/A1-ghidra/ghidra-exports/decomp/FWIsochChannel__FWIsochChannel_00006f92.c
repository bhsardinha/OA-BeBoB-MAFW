// m_audio_b_FWIsochChannel::m_audio_b_FWIsochChannel
// Entry: 00006f92
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWIsochChannel::m_audio_b_FWIsochChannel(OSMetaClass const*) */

void __thiscall
m_audio_b_FWIsochChannel::m_audio_b_FWIsochChannel
          (m_audio_b_FWIsochChannel *this,OSMetaClass *param_1)

{
  IOFWIsochChannel::IOFWIsochChannel((IOFWIsochChannel *)this,param_1);
  *(undefined ***)this = &PTR__m_audio_b_FWIsochChannel_00033840;
  return;
}
