// m_audio_b_FWKIPCUserClient::stop
// Entry: 000160e4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::stop(IOService*) */

void __thiscall
m_audio_b_FWKIPCUserClient::stop(m_audio_b_FWKIPCUserClient *this,IOService *param_1)

{
  *(undefined8 *)(this + 0xe8) = 0;
  UserClientRemoved(this);
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
