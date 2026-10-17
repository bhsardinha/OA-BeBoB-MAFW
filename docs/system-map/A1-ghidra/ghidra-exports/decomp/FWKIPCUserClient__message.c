// m_audio_b_FWKIPCUserClient::message
// Entry: 00015fca
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::message(unsigned int, IOService*, void*) */

void __thiscall
m_audio_b_FWKIPCUserClient::message
          (m_audio_b_FWKIPCUserClient *this,uint param_1,IOService *param_2,void *param_3)

{
  if (*(long *)(this + 0xe8) != 0) {
    *(uint *)(this + 0x100) = param_1;
    _mach_msg_send_from_kernel_proper(this + 0xe0,*(undefined4 *)(this + 0xe4));
  }
                    /* WARNING: Could not recover jumptable at 0x00016023. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300a0 + 0x740))(this,param_1,param_2,param_3);
  return;
}
