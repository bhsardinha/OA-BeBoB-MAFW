// com_m_audio_FWMetaNub::RegregisterDeviceThread
// Entry: 0001bcc2  Size: 123 bytes
// Prototype: undefined RegregisterDeviceThread(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::RegregisterDeviceThread() */

void __thiscall com_m_audio_FWMetaNub::RegregisterDeviceThread(com_m_audio_FWMetaNub *this)

{
  char cVar1;
  int iVar2;
  
  if (this[0xf4] == (com_m_audio_FWMetaNub)0x0) {
    cVar1 = IOService::isInactive();
    if (cVar1 == '\0') {
      iVar2 = FirmwareCacheInfoRegisters((IOService *)this);
      if (iVar2 == 0) {
        _IOLog("M-FW: Reregister: device still here, re-registering%c\n",0x2e);
        (**(code **)(*(long *)this + 0x5b0))(this,0);
        goto LAB_0001bd2b;
      }
    }
  }
  _IOLog("M-FW: Reregister: device node vanished%c\n",0x2e);
LAB_0001bd2b:
                    /* WARNING: Could not recover jumptable at 0x0001bd3a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0x28))(this);
  return;
}


