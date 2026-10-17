// m_audio_b_FWKIPCUserClient::registerNotificationPort
// Entry: 00015f3a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::registerNotificationPort(ipc_port*, unsigned int, unsigned int) */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::registerNotificationPort
          (m_audio_b_FWKIPCUserClient *this,ipc_port *param_1,uint param_2,uint param_3)

{
  if (param_2 == 0) {
    *(undefined4 *)(this + 0xe0) = 0x13;
    *(undefined4 *)(this + 0xe4) = 0x2c;
    *(undefined8 *)(this + 0xf0) = 0;
    *(undefined4 *)(this + 0xf8) = 0;
    *(undefined4 *)(this + 0xfc) = 0;
    *(ipc_port **)(this + 0xe8) = param_1;
    *(undefined4 *)(this + 0x108) = *(undefined4 *)(this + 0x118);
    *(uint *)(this + 0x104) = param_3;
    return 0;
  }
  _IOLog("M-FW: Someone registered for an unknown message type:%X\n",param_2);
  return 0xe00002c2;
}
