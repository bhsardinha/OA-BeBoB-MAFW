// com_m_audio_FWUserClient::WaitForTermination
// Entry: 0000761e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::WaitForTermination() */

undefined8 __thiscall com_m_audio_FWUserClient::WaitForTermination(com_m_audio_FWUserClient *this)

{
  long lVar1;
  
  lVar1 = *(long *)(this + 0x288);
  if (lVar1 == 0) {
    _semaphore_create((task_t)*(undefined8 *)PTR__kernel_task_000300e8,(semaphore_t *)(this + 0x288)
                      ,0,0);
    lVar1 = *(long *)(this + 0x288);
  }
  if (lVar1 != 0) {
    _semaphore_wait((semaphore_t)lVar1);
  }
  return 0;
}
