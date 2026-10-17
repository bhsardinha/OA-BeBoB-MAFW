// m_audio_b_FWBaseEngine::CompletePendingStateChangesAction
// Entry: 00003b2c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::CompletePendingStateChangesAction() */

undefined8 __thiscall
m_audio_b_FWBaseEngine::CompletePendingStateChangesAction(m_audio_b_FWBaseEngine *this)

{
  bool bVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  undefined8 uVar5;
  
  uVar3 = *(uint *)(this + 0x344);
  if ((this[0x2d3] == (m_audio_b_FWBaseEngine)0x0) || ((uVar3 & 0x48000) == 0)) {
    bVar1 = false;
  }
  else {
    bVar1 = true;
  }
  if ((uVar3 & 1) != 0) {
    return 0;
  }
  if (bVar1) {
    (**(code **)(*(long *)this + 0xcc0))(this);
    uVar3 = *(uint *)(this + 0x344) | 0x8000;
    *(uint *)(this + 0x344) = uVar3;
  }
  if ((uVar3 & 0x800000) != 0) {
    cVar2 = RecreateAudioStreams(this,true,true);
    if (cVar2 != '\0') {
      uVar3 = *(uint *)(this + 0x344);
      goto LAB_00003baa;
    }
LAB_00003b9a:
    uVar5 = 0xe00002bd;
    goto LAB_00003c9e;
  }
LAB_00003baa:
  if ((uVar3 & 0x200000) != 0) {
    cVar2 = ResetSampleBuffers(this);
    if (cVar2 == '\0') goto LAB_00003b9a;
    uVar3 = *(uint *)(this + 0x344);
  }
  if ((uVar3 & 0x400000) != 0) {
    m_audio_b_FWDiscreteEngine::ResetStereoPairIOFormats((m_audio_b_FWDiscreteEngine *)this);
    uVar3 = *(uint *)(this + 0x344);
  }
  if ((uVar3 & 0x40000) != 0) {
    (**(code **)(*(long *)this + 0xc78))(this);
    uVar3 = *(uint *)(this + 0x344);
  }
  if (((uVar3 & 0x8000) == 0) ||
     ((((uVar3 & 0x80000) != 0 && (this[0x2d0] == (m_audio_b_FWBaseEngine)0x0)) &&
      (this[0x348] == (m_audio_b_FWBaseEngine)0x0)))) {
LAB_00003c25:
    if ((uVar3 & 0x80000) != 0) {
      (**(code **)(*(long *)this + 0xcc8))(this);
      uVar3 = *(uint *)(this + 0x344);
    }
    if ((uVar3 & 0x1000000) != 0) {
      _IOSleep(1000);
      uVar3 = *(uint *)(this + 0x344);
    }
    if ((uVar3 & 0x100000) != 0) {
      m_audio_b_FWAudioDevice::SetStaticPropertyAndNotifyUserClients
                (*(m_audio_b_FWAudioDevice **)(this + 0x138),"MASampleRate",*(uint *)(this + 0x8c));
    }
    if ((this[0x2d3] != (m_audio_b_FWBaseEngine)0x0) && (this[0x230] != (m_audio_b_FWBaseEngine)0x0)
       ) {
      iVar4 = (**(code **)(*(long *)this + 0xcf0))(this);
      this[0x230] = (m_audio_b_FWBaseEngine)(iVar4 != 0);
    }
  }
  else {
    iVar4 = (**(code **)(*(long *)this + 0xcb8))(this);
    if (iVar4 == 0) {
      uVar3 = *(uint *)(this + 0x344);
      goto LAB_00003c25;
    }
  }
  uVar5 = 0;
LAB_00003c9e:
  *(undefined4 *)(this + 0x344) = 0;
  return uVar5;
}
