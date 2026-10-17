// com_m_audio_FW1814Device::IsReceptionOfControlPacketsEnabled
// Entry: 0000c3d2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::IsReceptionOfControlPacketsEnabled() */

bool __thiscall
com_m_audio_FW1814Device::IsReceptionOfControlPacketsEnabled(com_m_audio_FW1814Device *this)

{
  return *(long *)(this + 0x1270) != 0;
}
