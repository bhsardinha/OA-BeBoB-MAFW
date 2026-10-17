// m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage
// Entry: 0001602a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage() */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::SendChangedPropertiesMessage(m_audio_b_FWKIPCUserClient *this)

{
  undefined8 uVar1;
  undefined4 uVar2;
  int iVar3;
  long *plVar4;
  undefined8 *puVar5;
  
  plVar4 = (long *)SerializeModifiedProperties(this);
  if (plVar4 != (long *)0x0) {
    puVar5 = (undefined8 *)(**(code **)(*plVar4 + 0x2e0))(plVar4);
    uVar2 = (**(code **)(*plVar4 + 0x1c8))(plVar4);
    if (*(long *)(this + 0xe8) != 0) {
      *(undefined4 *)(puVar5 + 5) = *(undefined4 *)(this + 0x108);
      puVar5[4] = *(undefined8 *)(this + 0x100);
      puVar5[3] = *(undefined8 *)(this + 0xf8);
      puVar5[2] = *(undefined8 *)(this + 0xf0);
      uVar1 = *(undefined8 *)(this + 0xe0);
      puVar5[1] = *(undefined8 *)(this + 0xe8);
      *puVar5 = uVar1;
      *(undefined4 *)((long)puVar5 + 4) = uVar2;
      *(undefined4 *)(puVar5 + 4) = 0;
      iVar3 = _mach_msg_send_from_kernel_proper();
      if (iVar3 != 0) {
        return 0xe00002bc;
      }
      return 0;
    }
  }
  return 0xe00002bc;
}
