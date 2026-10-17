// com_m_audio_FWAudiophileDevice::SetupAudioEngine
// Entry: 0000bb9a  Size: 153 bytes
// Prototype: undefined SetupAudioEngine(com_m_audio_FWAudiophileDevice * this, m_audio_b_FWBaseEngine * param_1)


/* com_m_audio_FWAudiophileDevice::SetupAudioEngine(m_audio_b_FWBaseEngine*) */

void __thiscall
com_m_audio_FWAudiophileDevice::SetupAudioEngine
          (com_m_audio_FWAudiophileDevice *this,m_audio_b_FWBaseEngine *param_1)

{
  *(undefined4 *)(param_1 + 0x2e8) = 1;
  m_audio_b_FWBaseEngine::SetMIDIInputPorts(param_1,(m_audio_b_FWAudioDevice *)this,1,(char **)0x0);
  m_audio_b_FWBaseEngine::SetMIDIOutputPorts(param_1,(m_audio_b_FWAudioDevice *)this,1,(char **)0x0)
  ;
  *(undefined4 *)(param_1 + 0x2e4) = 4;
  *(undefined4 *)(param_1 + 0x2e0) = 6;
  *(undefined4 *)(param_1 + 0x2f0) = 4;
  *(undefined4 *)(param_1 + 0x2ec) = 6;
  *(code **)(param_1 + 0x310) = _ConvertLLRR4SkipMIDI1;
  *(com_m_audio_FWAudiophileDevice **)(param_1 + 0x318) = this;
  *(code **)(param_1 + 0x300) = _RearrangeSampleFrameLLRR6;
  *(code **)(param_1 + 0x308) = _SamplePeaks6;
  return;
}


