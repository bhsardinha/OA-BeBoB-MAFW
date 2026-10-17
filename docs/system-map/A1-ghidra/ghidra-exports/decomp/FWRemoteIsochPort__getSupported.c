// m_audio_b_FWRemoteIsochPort::getSupported
// Entry: 000183d8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWRemoteIsochPort::getSupported(IOFWSpeed&, unsigned long long&) */

undefined8 __thiscall
m_audio_b_FWRemoteIsochPort::getSupported
          (m_audio_b_FWRemoteIsochPort *this,IOFWSpeed *param_1,ulonglong *param_2)

{
  *(undefined4 *)param_1 = 0x7fffffff;
  *param_2 = 0xffffffffffffffff;
  return 0;
}
