// com_m_audio_FWEngine2::SetAC3OutputState
// Entry: 00011f28
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-i)

/* com_m_audio_FWEngine2::SetAC3OutputState(bool) */

undefined8 __thiscall
com_m_audio_FWEngine2::SetAC3OutputState(com_m_audio_FWEngine2 *this,bool param_1)

{
  char cVar1;
  m_audio_b_FWAVCAudioDevice *this_00;
  undefined8 uVar2;
  undefined4 local_28;
  uchar local_24;
  undefined1 local_23;
  undefined4 local_20;
  
  this_00 = (m_audio_b_FWAVCAudioDevice *)
            OSMetaClassBase::safeMetaCast
                      (*(OSMetaClassBase **)(this + 0x138),
                       (OSMetaClass *)m_audio_b_FWAVCAudioDevice::metaClass);
  if ((this_00 != (m_audio_b_FWAVCAudioDevice *)0x0) &&
     (cVar1 = (**(code **)(*(long *)this_00 + 0xab0))(this_00), cVar1 != '\0')) {
    local_28 = 9;
    local_24 = '\0';
    local_23 = 0;
    local_20 = 0;
    m_audio_b_FWAVCAudioDevice::AVCVendorCommand((uchar)this_00,'\0','\x01');
    uVar2 = (**(code **)(*(long *)this_00 + 0xb48))(this_00,&local_28);
    if ((int)uVar2 != 0) {
      return uVar2;
    }
    uVar2 = m_audio_b_FWAVCAudioDevice::AVCSelectorCommand(this_00,local_24,!param_1);
    return uVar2;
  }
  return 0;
}
