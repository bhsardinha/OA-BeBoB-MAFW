// m_audio_b_FWRemoteIsochPort::init
// Entry: 000183c6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWRemoteIsochPort::init(m_audio_b_FWConnectionManager*, bool, unsigned int) */

undefined8 __thiscall
m_audio_b_FWRemoteIsochPort::init
          (m_audio_b_FWRemoteIsochPort *this,m_audio_b_FWConnectionManager *param_1,bool param_2,
          uint param_3)

{
  this[0x18] = (m_audio_b_FWRemoteIsochPort)param_2;
  *(uint *)(this + 0x1c) = param_3;
  *(m_audio_b_FWConnectionManager **)(this + 0x10) = param_1;
  return 0;
}
