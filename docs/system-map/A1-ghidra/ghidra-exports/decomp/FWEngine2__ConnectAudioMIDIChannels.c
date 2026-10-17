// com_m_audio_FWEngine2::ConnectAudioMIDIChannels
// Entry: 00012156
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* com_m_audio_FWEngine2::ConnectAudioMIDIChannels() */

ulong __thiscall com_m_audio_FWEngine2::ConnectAudioMIDIChannels(com_m_audio_FWEngine2 *this)

{
  com_m_audio_FWEngine2 cVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  uint uVar4;
  long *plVar5;
  ulong uVar6;
  undefined8 uVar7;
  long lVar8;
  
  cVar1 = this[0x348];
  if (cVar1 == (com_m_audio_FWEngine2)0x0) {
    plVar5 = (long *)OSMetaClassBase::safeMetaCast
                               (*(OSMetaClassBase **)(this + 0x138),
                                (OSMetaClass *)m_audio_b_FWAVCAudioDevice::metaClass);
    if (plVar5 != (long *)0x0) {
      (**(code **)(*plVar5 + 0xb50))
                (plVar5,1,0,*(undefined4 *)(this + 0x228),*(undefined4 *)(this + 0x2e4),100);
    }
    _IOSleep(500);
  }
  uVar6 = (**(code **)(**(long **)(this + 0x350) + 0x120))(*(long **)(this + 0x350),0);
  if ((int)uVar6 != 0) {
    return uVar6;
  }
  _IOSleep(500);
  plVar5 = (long *)OSMetaClassBase::safeMetaCast
                             (*(OSMetaClassBase **)(this + 0x138),
                              (OSMetaClass *)m_audio_b_FWAVCAudioDevice::metaClass);
  if (cVar1 == (com_m_audio_FWEngine2)0x0) {
    if (plVar5 == (long *)0x0) goto LAB_0001224d;
    uVar2 = *(undefined4 *)(this + 0x228);
    uVar3 = *(undefined4 *)(this + 0x2e0);
    lVar8 = *plVar5;
    uVar7 = 0;
  }
  else {
    if (plVar5 == (long *)0x0) goto LAB_0001224d;
    uVar2 = *(undefined4 *)(this + 0x228);
    uVar3 = *(undefined4 *)(this + 0x2e4);
    lVar8 = *plVar5;
    uVar7 = 1;
  }
  (**(code **)(lVar8 + 0xb50))(plVar5,uVar7,0,uVar2,uVar3,100);
LAB_0001224d:
  _IOSleep(500);
  uVar4 = (**(code **)(**(long **)(this + 0x358) + 0x120))(*(long **)(this + 0x358),0);
  if (uVar4 == 0) {
    if ((cVar1 != (com_m_audio_FWEngine2)0x0) &&
       (plVar5 = (long *)OSMetaClassBase::safeMetaCast
                                   (*(OSMetaClassBase **)(this + 0x138),
                                    (OSMetaClass *)m_audio_b_FWAVCAudioDevice::metaClass),
       plVar5 != (long *)0x0)) {
      (**(code **)(*plVar5 + 0xb50))
                (plVar5,0,0,*(undefined4 *)(this + 0x228),*(undefined4 *)(this + 0x2e0),100);
    }
  }
  else {
    (**(code **)(**(long **)(this + 0x350) + 0x128))();
  }
  return (ulong)uVar4;
}
