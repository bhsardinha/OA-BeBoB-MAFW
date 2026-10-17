// m_audio_b_FWBaseEngine::ChangeSampleRateToFDFValue
// Entry: 000052e8
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::ChangeSampleRateToFDFValue(FDFValue) */

void __thiscall
m_audio_b_FWBaseEngine::ChangeSampleRateToFDFValue(m_audio_b_FWBaseEngine *this,undefined4 param_2)

{
  int iVar1;
  
  switch(param_2) {
  case 0:
    iVar1 = 32000;
    break;
  case 1:
    iVar1 = 0xac44;
    break;
  case 2:
    iVar1 = 48000;
    break;
  case 3:
    iVar1 = 0x15888;
    break;
  case 4:
    iVar1 = 96000;
    break;
  case 5:
    iVar1 = 0x2b110;
    break;
  case 6:
    iVar1 = 0x2ee00;
    break;
  default:
    _IOLog("M-FW: invalid sample rate detected (FDF:%X)\n");
    iVar1 = *(int *)(this + 0x220);
  }
  if ((iVar1 != *(int *)(this + 0x220)) || (this[0x2d1] != (m_audio_b_FWBaseEngine)0x0)) {
    if (this != (m_audio_b_FWBaseEngine *)0x0) {
      (**(code **)(*(long *)this + 0xa50))(this);
    }
    (**(code **)(*(long *)this + 0xb00))(this);
    this[0x346] = (m_audio_b_FWBaseEngine)((byte)this[0x346] | 0x6f);
    (**(code **)(**(long **)(this + 0x138) + 0xa78))(*(long **)(this + 0x138),iVar1);
    CompletePendingStateChanges(this);
    (**(code **)(*(long *)this + 0xb08))(this);
    if (this != (m_audio_b_FWBaseEngine *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x000053c7. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*(long *)this + 0xa58))(this);
      return;
    }
  }
  return;
}
