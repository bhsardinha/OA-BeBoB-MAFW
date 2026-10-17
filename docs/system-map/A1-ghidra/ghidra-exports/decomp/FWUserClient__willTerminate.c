// com_m_audio_FWUserClient::willTerminate
// Entry: 000076c2
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::willTerminate(IOService*, unsigned int) */

undefined8 com_m_audio_FWUserClient::willTerminate(IOService *param_1,uint param_2)

{
  if (*(long *)(param_1 + 0x288) != 0) {
    _semaphore_signal((semaphore_t)*(long *)(param_1 + 0x288));
  }
  return 1;
}
