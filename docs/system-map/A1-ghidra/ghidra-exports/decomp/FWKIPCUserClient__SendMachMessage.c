// m_audio_b_FWKIPCUserClient::SendMachMessage
// Entry: 00015faa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::SendMachMessage(unsigned int, MAudioMessage*, unsigned long) */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::SendMachMessage
          (m_audio_b_FWKIPCUserClient *this,uint param_1,MAudioMessage *param_2,ulong param_3)

{
  int iVar1;
  undefined8 uVar2;
  
  *(uint *)(param_2 + 0x20) = param_1;
  iVar1 = _mach_msg_send_from_kernel_proper(param_2,param_3 & 0xffffffff);
  uVar2 = 0xe00002bc;
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  return uVar2;
}
