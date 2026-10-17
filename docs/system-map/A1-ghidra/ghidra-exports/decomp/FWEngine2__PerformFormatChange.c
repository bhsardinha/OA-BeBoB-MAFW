// com_m_audio_FWEngine2::PerformFormatChange
// Entry: 000124cc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::PerformFormatChange(IOAudioStream*, _IOAudioStreamFormat const*,
   _IOAudioSampleRate const*) */

undefined4 __thiscall
com_m_audio_FWEngine2::PerformFormatChange
          (com_m_audio_FWEngine2 *this,IOAudioStream *param_1,_IOAudioStreamFormat *param_2,
          _IOAudioSampleRate *param_3)

{
  int iVar1;
  bool bVar2;
  bool bVar3;
  undefined4 uVar4;
  _IOAudioSampleRate *p_Var5;
  _IOAudioSampleRate *p_Var6;
  int local_38 [2];
  undefined4 local_30;
  undefined4 local_2c;
  
  if (this != (com_m_audio_FWEngine2 *)0x0) {
    (**(code **)(*(long *)this + 0xa50))(this);
  }
  if ((param_2 == (_IOAudioStreamFormat *)0x0) && (param_3 == (_IOAudioSampleRate *)0x0)) {
    uVar4 = 0;
    goto LAB_000125ff;
  }
  p_Var6 = param_3;
  if (param_2 == (_IOAudioStreamFormat *)0x0) {
    bVar2 = false;
    bVar3 = false;
LAB_0001254e:
    if (this[0x2d4] == (com_m_audio_FWEngine2)0x0) {
      p_Var5 = param_3;
      if (param_3 == (_IOAudioSampleRate *)0x0) {
        p_Var5 = (_IOAudioSampleRate *)(this + 0x220);
      }
      (**(code **)(**(long **)(this + 0x138) + 0xa78))
                (*(long **)(this + 0x138),*(undefined4 *)p_Var5);
    }
    if (bVar2) goto LAB_00012580;
    if (bVar3) {
      m_audio_b_FWBaseEngine::DisableAC3Format((m_audio_b_FWBaseEngine *)this);
    }
  }
  else {
    if ((*(int *)(param_2 + 4) == 0x63616333) && (this[0x2d4] == (com_m_audio_FWEngine2)0x0)) {
      bVar2 = true;
    }
    else {
      bVar2 = false;
    }
    if ((*(int *)(param_2 + 4) == 0x63616333) || (this[0x2d4] == (com_m_audio_FWEngine2)0x0)) {
      bVar3 = false;
    }
    else {
      bVar3 = true;
    }
    if (!bVar2) goto LAB_0001254e;
LAB_00012580:
    local_30 = 48000;
    local_2c = 0;
    m_audio_b_FWBaseEngine::EnableAC3Format((m_audio_b_FWBaseEngine *)this);
    p_Var6 = (_IOAudioSampleRate *)&local_30;
    if (param_3 != (_IOAudioSampleRate *)0x0) {
      p_Var6 = param_3;
    }
  }
  if (p_Var6 == (_IOAudioSampleRate *)0x0) {
    if (bVar3) {
      iVar1 = *(int *)(this + 0x220);
      goto LAB_000125d6;
    }
  }
  else {
    iVar1 = *(int *)(this + 0x220);
    if ((*(int *)p_Var6 != iVar1) || (bVar3)) {
LAB_000125d6:
      local_38[0] = iVar1;
      local_38[1] = 0;
      p_Var5 = (_IOAudioSampleRate *)local_38;
      if (p_Var6 != (_IOAudioSampleRate *)0x0) {
        p_Var5 = p_Var6;
      }
      (**(code **)(*(long *)this + 0xce0))(this,p_Var5);
    }
  }
  uVar4 = m_audio_b_FWBaseEngine::CompletePendingStateChanges((m_audio_b_FWBaseEngine *)this);
LAB_000125ff:
  if (this != (com_m_audio_FWEngine2 *)0x0) {
    (**(code **)(*(long *)this + 0xa58))(this);
  }
  return uVar4;
}
