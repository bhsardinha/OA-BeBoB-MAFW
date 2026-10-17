// com_m_audio_FWMetaNub::FirmwareUpdateBeBoB32
// Entry: 0001c5f2  Size: 359 bytes
// Prototype: undefined FirmwareUpdateBeBoB32(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::FirmwareUpdateBeBoB32() */

ulong __thiscall com_m_audio_FWMetaNub::FirmwareUpdateBeBoB32(com_m_audio_FWMetaNub *this)

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
    lVar1 = *(long *)(this + 0x140);
    uVar2 = DownloadSectionStart
                      (this,1,0,*(uint *)(lVar1 + 0x58),*(uint *)(lVar1 + 0x5c),
                       *(uint *)(lVar1 + 0x50));
    uVar3 = (ulong)uVar2;
    if (uVar2 == 0) {
      *(undefined8 *)(this + 0x148) = 0;
      *(undefined8 *)(this + 0x178) = 0;
      *(ulong *)(this + 0x160) = (ulong)*(uint *)(this + 0x138);
      *(undefined8 *)(this + 0x150) = 0;
      *(undefined8 *)(this + 0x168) = 0;
      *(undefined8 *)(this + 0x170) = 0;
      *(ulong *)(this + 0x180) = (ulong)*(uint *)(*(long *)(this + 0x140) + 0x3c);
      _IOLog("M-FW: FirmUp- Starting 3.2 or later upload (%lu bytes)\n");
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


