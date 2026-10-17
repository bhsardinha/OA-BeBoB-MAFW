// com_m_audio_FWUserClient::registerNotificationPort
// Entry: 000074aa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* com_m_audio_FWUserClient::registerNotificationPort(ipc_port*, unsigned int, unsigned int) */

undefined8 __thiscall
com_m_audio_FWUserClient::registerNotificationPort
          (com_m_audio_FWUserClient *this,ipc_port *param_1,uint param_2,uint param_3)

{
  if (param_2 == 1) {
    *(undefined4 *)(this + 0x148) = 0x13;
    *(undefined4 *)(this + 0x14c) = 0x140;
    *(undefined8 *)(this + 0x158) = 0;
    *(undefined4 *)(this + 0x160) = 0;
    *(undefined4 *)(this + 0x164) = 0;
    *(ipc_port **)(this + 0x150) = param_1;
    *(uint *)(this + 0x16c) = param_3;
  }
  else {
    (*(code *)PTR_registerNotificationPort_0003dc28)();
  }
  return 0;
}
