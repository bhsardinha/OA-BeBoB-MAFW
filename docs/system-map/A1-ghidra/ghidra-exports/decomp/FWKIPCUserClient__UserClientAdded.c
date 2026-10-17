// m_audio_b_FWKIPCUserClient::UserClientAdded
// Entry: 0001620c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::UserClientAdded(m_audio_b_FWKIPCUserClient*) */

void m_audio_b_FWKIPCUserClient::UserClientAdded(m_audio_b_FWKIPCUserClient *param_1)

{
  undefined8 in_RDX;
  undefined1 auVar1 [16];
  
  auVar1._8_8_ = in_RDX;
  auVar1._0_8_ = sUserClients;
  if (sUserClients == (long *)0x0) {
    auVar1 = OSSet::withCapacity(4);
  }
  sUserClients = auVar1._0_8_;
  if (sUserClients == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00016257. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*sUserClients + 0x1c0))
            (sUserClients,param_1,auVar1._8_8_,*(code **)(*sUserClients + 0x1c0));
  return;
}
