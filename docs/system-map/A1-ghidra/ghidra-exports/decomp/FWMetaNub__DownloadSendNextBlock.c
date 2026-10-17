// com_m_audio_FWMetaNub::DownloadSendNextBlock
// Entry: 0001c332  Size: 309 bytes
// Prototype: undefined DownloadSendNextBlock(com_m_audio_FWMetaNub * this, bool * param_1)


/* com_m_audio_FWMetaNub::DownloadSendNextBlock(bool&) */

void __thiscall
com_m_audio_FWMetaNub::DownloadSendNextBlock(com_m_audio_FWMetaNub *this,bool *param_1)

{
  long lVar1;
  long lVar2;
  int iVar3;
  long lVar4;
  MAFireWireResponseDevice local_88 [72];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  int local_34;
  undefined4 local_30;
  undefined2 local_28;
  undefined2 local_26;
  undefined4 local_24;
  undefined2 local_20;
  undefined2 local_1e;
  undefined4 local_1c;
  
  lVar1 = *(long *)(this + 0x140);
  *param_1 = true;
  lVar2 = *(long *)(this + 0x150);
  lVar4 = *(long *)(this + 0x158);
  if (*(ulong *)(this + 0x160) < (ulong)(lVar4 + lVar2)) {
    lVar4 = *(ulong *)(this + 0x160) - lVar2;
    *(long *)(this + 0x158) = lVar4;
  }
  local_28 = 0;
  local_26 = 0xffff;
  local_24 = 0xc8021040;
  iVar3 = FireWireWriteCommand
                    (this,(FWAddressStruct *)&local_28,(uint)lVar4,
                     (void *)(lVar1 + lVar2 + *(long *)(this + 0x148)));
  if (iVar3 == 0) {
    local_3c = 0x3050000;
    local_38 = *(undefined4 *)(this + 0x168);
    local_34 = *(int *)(this + 0x150) + *(int *)(this + 0x178);
    local_30 = *(undefined4 *)(this + 0x158);
    local_20 = 0;
    local_1e = 0xffff;
    local_1c = 0xc8021000;
    local_40 = *(undefined4 *)(this + 0x98);
    iVar3 = FireWireWriteCommand(this,(FWAddressStruct *)&local_20,0x14,&local_40);
    if (iVar3 == 0) {
      iVar3 = FirmwareReadResponse(this,local_88,0x10);
      if (iVar3 == 0) {
        lVar1 = *(long *)(this + 0x150);
        *(long *)(this + 0x150) = *(long *)(this + 0x158) + lVar1;
        *(long *)(this + 0x168) = *(long *)(this + 0x168) + 1;
        if (*(ulong *)(this + 0x160) <= (ulong)(*(long *)(this + 0x158) + lVar1)) {
          *param_1 = false;
        }
      }
    }
  }
  return;
}


