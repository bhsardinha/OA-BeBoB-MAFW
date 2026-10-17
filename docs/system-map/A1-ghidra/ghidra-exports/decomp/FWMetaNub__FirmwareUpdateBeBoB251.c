// com_m_audio_FWMetaNub::FirmwareUpdateBeBoB251
// Entry: 0001c75a  Size: 429 bytes
// Prototype: undefined FirmwareUpdateBeBoB251(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::FirmwareUpdateBeBoB251() */

ulong __thiscall com_m_audio_FWMetaNub::FirmwareUpdateBeBoB251(com_m_audio_FWMetaNub *this)

{
  long lVar1;
  uint uVar2;
  ulong uVar3;
  bool bVar4;
  bool local_31;
  
  local_31 = true;
  lVar1 = *(long *)(this + 0x140);
  uVar3 = DownloadSectionStart
                    (this,0,*(uint *)(lVar1 + 0x34),*(uint *)(lVar1 + 0x38),*(uint *)(lVar1 + 0x3c),
                     *(uint *)(lVar1 + 0x30));
  if ((int)uVar3 == 0) {
    _IOLog("M-FW: FirmUp- Starting code upload (%lu bytes)\n",*(undefined8 *)(this + 0x160));
    do {
      if (*(ulong *)(this + 0x150) % 0x19000 == 0) {
        _IOLog("M-FW: FirmUp-  %lu KB of %lu KB\n",*(ulong *)(this + 0x150) >> 10,
               *(ulong *)(this + 0x160) >> 10);
      }
      uVar3 = DownloadSendNextBlock(this,&local_31);
      if ((int)uVar3 != 0) {
        return uVar3;
      }
    } while (local_31 != false);
    DownloadStop(this);
    lVar1 = *(long *)(this + 0x140);
    uVar2 = DownloadSectionStart
                      (this,1,0,*(uint *)(lVar1 + 0x58),*(uint *)(lVar1 + 0x5c),
                       *(uint *)(lVar1 + 0x50));
    uVar3 = (ulong)uVar2;
    if (uVar2 == 0) {
      _IOLog("M-FW: FirmUp- Starting config upload (%lu bytes)\n",*(undefined8 *)(this + 0x160));
      local_31 = true;
      bVar4 = false;
      while (!bVar4) {
        if (*(ulong *)(this + 0x150) % 0x19000 == 0) {
          _IOLog("M-FW: FirmUp-  %lu KB of %lu KB\n",*(ulong *)(this + 0x150) >> 10,
                 *(ulong *)(this + 0x160) >> 10);
        }
        uVar3 = DownloadSendNextBlock(this,&local_31);
        if ((int)uVar3 != 0) {
          return uVar3;
        }
        uVar3 = uVar3 & 0xffffffff;
        bVar4 = local_31 == false;
      }
      DownloadStop(this);
    }
  }
  return uVar3;
}


