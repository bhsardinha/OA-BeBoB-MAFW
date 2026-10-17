// com_m_audio_FWUserClient::ReceivedMIDIPacket
// Entry: 0000757e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWUserClient::ReceivedMIDIPacket(MAMIDIPacket*) */

undefined8 __thiscall
com_m_audio_FWUserClient::ReceivedMIDIPacket(com_m_audio_FWUserClient *this,MAMIDIPacket *param_1)

{
  undefined8 uVar1;
  MAudioMessage local_160 [4];
  uint local_15c;
  undefined4 local_138;
  undefined1 local_130 [272];
  
  if (*(long *)(this + 0x150) == 0) {
    uVar1 = 0xe00002bc;
  }
  else {
    _memcpy(local_160,this + 0x148,0x140);
    local_15c = 0x140;
    if (*(ushort *)(param_1 + 0x10) + 0x40 < 0x140) {
      local_15c = *(ushort *)(param_1 + 0x10) + 0x40;
    }
    local_138 = *(undefined4 *)param_1;
    _memcpy(local_130,param_1 + 8,0x110);
    uVar1 = m_audio_b_FWKIPCUserClient::SendMachMessage
                      ((m_audio_b_FWKIPCUserClient *)this,1,local_160,(ulong)local_15c);
  }
  return uVar1;
}
