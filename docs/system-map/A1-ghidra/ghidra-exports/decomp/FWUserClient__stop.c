// com_m_audio_FWUserClient::stop
// Entry: 0000766c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* WARNING: Switch with 1 destination removed at 0x000076bb */
/* com_m_audio_FWUserClient::stop(IOService*) */

void __thiscall com_m_audio_FWUserClient::stop(com_m_audio_FWUserClient *this,IOService *param_1)

{
  *(undefined8 *)(this + 0x150) = 0;
  if (*(long *)(this + 0x288) != 0) {
    _semaphore_destroy((task_t)*(undefined8 *)PTR__kernel_task_000300e8,
                       (semaphore_t)*(long *)(this + 0x288));
    *(undefined8 *)(this + 0x288) = 0;
  }
  *(undefined8 *)(this + 0xe8) = 0;
  m_audio_b_FWKIPCUserClient::UserClientRemoved((m_audio_b_FWKIPCUserClient *)this);
  if (*(long **)(this + 0x110) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x110) + 0x28))();
    *(undefined8 *)(this + 0x110) = 0;
  }
  if (*(long *)(this + 0x120) != 0) {
    _IORecursiveLockFree();
    *(undefined8 *)(this + 0x120) = 0;
  }
  if (*(long **)(this + 0x128) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x128) + 0x28))();
    *(undefined8 *)(this + 0x128) = 0;
  }
  if (*(long **)(this + 0x130) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x130) + 0x28))();
    *(undefined8 *)(this + 0x130) = 0;
  }
                    /* WARNING: Could not recover jumptable at 0x00016188. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300a0 + 0x5d8))(this,param_1);
  return;
}
