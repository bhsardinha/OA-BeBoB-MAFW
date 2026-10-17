// com_m_audio_FWMetaNub::DownloadStop
// Entry: 0001c28c  Size: 165 bytes
// Prototype: undefined DownloadStop(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::DownloadStop() */

void __thiscall com_m_audio_FWMetaNub::DownloadStop(com_m_audio_FWMetaNub *this)

{
  int iVar1;
  char *pcVar2;
  MAFireWireResponseDevice local_70 [12];
  int local_64;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined2 local_18;
  undefined2 local_16;
  undefined4 local_14;
  
  local_24 = 0x60000;
  local_20 = *(undefined4 *)(this + 0x180);
  local_18 = 0;
  local_16 = 0xffff;
  local_14 = 0xc8021000;
  local_28 = *(undefined4 *)(this + 0x98);
  iVar1 = FireWireWriteCommand(this,(FWAddressStruct *)&local_18,0xc,&local_28);
  if (iVar1 == 0) {
    iVar1 = FirmwareReadResponse(this,local_70,0x28);
    if (iVar1 == 0) {
      _IOLog("M-FW: FirmUp- Uploaded %lu bytes\n",*(undefined8 *)(this + 0x150));
      if (local_64 == 0) {
        pcVar2 = "M-FW: FirmUp- Successful upload%c\n";
        local_64 = 0x21;
      }
      else {
        pcVar2 = "M-FW: FirmUp- Upload failed with error $%X\n";
      }
      _IOLog(pcVar2,local_64);
    }
  }
  return;
}


