// com_m_audio_FWEngine2::RecalculateBandwidth
// Entry: 0001235a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::RecalculateBandwidth() */

undefined4 __thiscall com_m_audio_FWEngine2::RecalculateBandwidth(com_m_audio_FWEngine2 *this)

{
  undefined4 uVar1;
  uint uVar2;
  long *plVar3;
  
  if (this[0x348] == (com_m_audio_FWEngine2)0x0) {
    if ((this[0x2d0] == (com_m_audio_FWEngine2)0x0) || (this[0x2d1] != (com_m_audio_FWEngine2)0x0))
    {
      if (this[0x2d3] == (com_m_audio_FWEngine2)0x0) {
        if (((byte)this[0x345] & 0x80) == 0) goto LAB_0001243c;
      }
      else {
        (**(code **)(*(long *)this + 0xcc0))(this);
      }
      uVar1 = (**(code **)(*(long *)this + 0xcb8))(this);
      goto LAB_0001243e;
    }
  }
  else {
    plVar3 = (long *)OSMetaClassBase::safeMetaCast
                               (*(OSMetaClassBase **)(this + 0x138),
                                (OSMetaClass *)m_audio_b_FWAVCAudioDevice::metaClass);
    if (plVar3 != (long *)0x0) {
      (**(code **)(*plVar3 + 0xb50))
                (plVar3,1,0,*(undefined4 *)(this + 0x228),*(undefined4 *)(this + 0x2e4),100);
    }
    plVar3 = (long *)OSMetaClassBase::safeMetaCast
                               (*(OSMetaClassBase **)(this + 0x138),
                                (OSMetaClass *)m_audio_b_FWAVCAudioDevice::metaClass);
    if (plVar3 != (long *)0x0) {
      (**(code **)(*plVar3 + 0xb50))
                (plVar3,0,0,*(undefined4 *)(this + 0x228),*(undefined4 *)(this + 0x2e0),100);
    }
  }
LAB_0001243c:
  uVar1 = 0;
LAB_0001243e:
  if (*(long *)(this + 0x138) != 0) {
    uVar2 = m_audio_b_FWAudioDevice::GetBandwidthBytesAtSampleRate((uint)*(long *)(this + 0x138));
    m_audio_b_FWAudioDevice::SetStaticPropertyAndNotifyUserClients
              (*(m_audio_b_FWAudioDevice **)(this + 0x138),"MABytesPerCycle",uVar2);
  }
  return uVar1;
}
