// m_audio_b_FWBaseEngine::SetIOFormat
// Entry: 000042aa
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::SetIOFormat(m_audio_b_FWAudioDevice::DeviceChannels&, unsigned int) */

int __thiscall
m_audio_b_FWBaseEngine::SetIOFormat
          (m_audio_b_FWBaseEngine *this,DeviceChannels *param_1,uint param_2)

{
  long lVar1;
  m_audio_b_FWBaseEngine mVar2;
  long lVar3;
  long *plVar4;
  int iVar5;
  ulong uVar6;
  
  lVar3 = *(long *)(param_1 + 0x38);
  mVar2 = this[0xa5];
  if (mVar2 == (m_audio_b_FWBaseEngine)0x0) {
    (**(code **)(*(long *)this + 0xb00))(this);
  }
  uVar6 = (ulong)param_2;
  lVar1 = lVar3 + uVar6 * 8;
  iVar5 = *(int *)(param_1 + 8);
  if (((iVar5 != *(int *)(this + 0x2e0)) || (*(int *)param_1 != *(int *)(this + 0x2e4))) ||
     (*(int *)(lVar3 + uVar6 * 8) != *(int *)(this + 0x8c))) {
    this[0x346] = (m_audio_b_FWBaseEngine)((byte)this[0x346] | 0x6f);
    iVar5 = *(int *)(param_1 + 8);
  }
  *(int *)(this + 0x2e0) = iVar5;
  *(undefined4 *)(this + 0x2ec) = *(undefined4 *)(param_1 + 0xc);
  *(undefined4 *)(this + 0x2e4) = *(undefined4 *)param_1;
  *(undefined4 *)(this + 0x2f0) = *(undefined4 *)(param_1 + 4);
  plVar4 = *(long **)(this + 0x138);
  if (plVar4 != (long *)0x0) {
    (**(code **)(*plVar4 + 0xaf8))(plVar4,this);
  }
  if (*(int *)(lVar3 + uVar6 * 8) != *(int *)(this + 0x8c)) {
    (**(code **)(*(long *)this + 0xb58))(this,lVar1);
    (**(code **)(*(long *)this + 0xce0))(this,lVar1);
  }
  iVar5 = SetStreamFormat((IOAudioStream *)this,*(DeviceChannels **)(this + 0x210),(uint)param_1);
  if ((iVar5 == 0) &&
     (iVar5 = SetStreamFormat((IOAudioStream *)this,*(DeviceChannels **)(this + 0x218),(uint)param_1
                             ), iVar5 == 0)) {
    if (*(long *)(param_1 + 0x18) == 0) {
      *(undefined8 *)(this + 800) = 0;
    }
    else {
      *(long *)(this + 800) = *(long *)(param_1 + 0x18);
    }
    if (*(long *)(param_1 + 0x20) == 0) {
      *(undefined8 *)(this + 0x328) = 0;
    }
    else {
      *(long *)(this + 0x328) = *(long *)(param_1 + 0x20);
    }
    if (*(long *)(param_1 + 0x28) == 0) {
      *(undefined8 *)(this + 0x330) = 0;
    }
    else {
      *(long *)(this + 0x330) = *(long *)(param_1 + 0x28);
    }
    if (mVar2 == (m_audio_b_FWBaseEngine)0x0) {
      (**(code **)(*(long *)this + 0xb08))(this);
    }
  }
  else if (mVar2 == (m_audio_b_FWBaseEngine)0x0) {
    (**(code **)(*(long *)this + 0xb10))(this);
  }
  return iVar5;
}
