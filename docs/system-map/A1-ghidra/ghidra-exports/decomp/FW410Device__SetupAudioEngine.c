// com_m_audio_FW410Device::SetupAudioEngine
// Entry: 000154ee
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::SetupAudioEngine(m_audio_b_FWBaseEngine*) */

void __thiscall
com_m_audio_FW410Device::SetupAudioEngine
          (com_m_audio_FW410Device *this,m_audio_b_FWBaseEngine *param_1)

{
  m_audio_b_FWBaseEngine::SetMIDIInputPorts(param_1,(m_audio_b_FWAudioDevice *)this,1,(char **)0x0);
  m_audio_b_FWBaseEngine::SetMIDIOutputPorts(param_1,(m_audio_b_FWAudioDevice *)this,1,(char **)0x0)
  ;
  param_1[0x2d6] = (m_audio_b_FWBaseEngine)0x1;
  param_1[0x34b] = (m_audio_b_FWBaseEngine)0x1;
  param_1[0x34a] = (m_audio_b_FWBaseEngine)0x1;
  *(undefined4 *)(param_1 + 0x2e4) = 4;
  *(undefined4 *)(param_1 + 0x2e0) = 10;
  *(undefined4 *)(param_1 + 0x2e8) = 1;
  *(undefined4 *)(param_1 + 0x2f0) = 4;
  *(undefined4 *)(param_1 + 0x2ec) = 10;
  *(code **)(param_1 + 0x300) = _RearrangeFrameLLRR10WithMIDI;
  *(code **)(param_1 + 0x308) = _SamplePeaks10;
  *(code **)(param_1 + 0x310) = _ConvertLLRR4SkipMIDI1;
  *(com_m_audio_FW410Device **)(param_1 + 0x318) = this;
  return;
}
