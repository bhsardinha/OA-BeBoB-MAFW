// com_m_audio_FW1814Device::SetupAudioEngine
// Entry: 0000ed34
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SetupAudioEngine(m_audio_b_FWBaseEngine*) */

void __thiscall
com_m_audio_FW1814Device::SetupAudioEngine
          (com_m_audio_FW1814Device *this,m_audio_b_FWBaseEngine *param_1)

{
  m_audio_b_FWBaseEngine::SetMIDIInputPorts(param_1,(m_audio_b_FWAudioDevice *)this,1,(char **)0x0);
  m_audio_b_FWBaseEngine::SetMIDIOutputPorts(param_1,(m_audio_b_FWAudioDevice *)this,1,(char **)0x0)
  ;
  param_1[0x2d5] = (m_audio_b_FWBaseEngine)0x1;
  param_1[0x2d6] = (m_audio_b_FWBaseEngine)0x1;
  param_1[0x348] = (m_audio_b_FWBaseEngine)0x1;
  *(undefined4 *)(param_1 + 0x2e8) = 1;
  if (((byte)this[0x144] & 0x10) == 0) {
    *(undefined4 *)(param_1 + 0x2ec) = 6;
    *(undefined4 *)(param_1 + 0x2e0) = 6;
  }
  else {
    *(undefined4 *)(param_1 + 0x2ec) = 0xc;
    *(undefined4 *)(param_1 + 0x2e0) = 0xc;
  }
  if (((byte)this[0x144] & 8) == 0) {
    *(undefined4 *)(param_1 + 0x2f0) = 10;
    *(undefined4 *)(param_1 + 0x2e4) = 10;
  }
  else {
    *(undefined4 *)(param_1 + 0x2f0) = 0x10;
    *(undefined4 *)(param_1 + 0x2e4) = 0x10;
  }
                    /* WARNING: Could not recover jumptable at 0x0000ee00. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xaf8))(this,param_1);
  return;
}
