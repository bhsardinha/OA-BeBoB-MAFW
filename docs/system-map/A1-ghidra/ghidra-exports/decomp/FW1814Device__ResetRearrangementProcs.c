// com_m_audio_FW1814Device::ResetRearrangementProcs
// Entry: 0000cad4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ResetRearrangementProcs(m_audio_b_FWBaseEngine*) */

void __thiscall
com_m_audio_FW1814Device::ResetRearrangementProcs
          (com_m_audio_FW1814Device *this,m_audio_b_FWBaseEngine *param_1)

{
  code *pcVar1;
  
  switch(*(undefined4 *)(param_1 + 0x2e0)) {
  case 4:
    pcVar1 = _RearrangeSampleFrameLRLR4;
    break;
  default:
    *(undefined8 *)(param_1 + 0x300) = 0;
    goto LAB_0000cb3e;
  case 6:
    pcVar1 = _RearrangeSampleFrameLRLR6SkipMIDI;
    break;
  case 8:
    pcVar1 = _RearrangeSampleFrameLRLR8SkipMIDI;
    break;
  case 10:
    pcVar1 = _RearrangeSampleFrameLRLR10SkipMIDI;
    break;
  case 0xc:
    pcVar1 = _RearrangeSampleFrameLRLR12SkipMIDI;
    break;
  case 0xe:
    pcVar1 = _RearrangeSampleFrameLRLR14SkipMIDI;
  }
  *(code **)(param_1 + 0x300) = pcVar1;
LAB_0000cb3e:
  if (*(int *)(param_1 + 0x2e4) == 2) {
    *(code **)(param_1 + 0x310) = _ConvertInputFramesOneStereoPairSkipMIDI;
    *(undefined8 *)(param_1 + 0x318) = 2;
    return;
  }
  *(code **)(param_1 + 0x310) = WorkaroundFor1814InputDelay;
  *(com_m_audio_FW1814Device **)(param_1 + 0x318) = this;
  return;
}
