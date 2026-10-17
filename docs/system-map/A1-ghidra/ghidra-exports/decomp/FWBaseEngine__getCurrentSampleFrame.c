// m_audio_b_FWBaseEngine::getCurrentSampleFrame
// Entry: 00003e0c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::getCurrentSampleFrame() */

int __thiscall m_audio_b_FWBaseEngine::getCurrentSampleFrame(m_audio_b_FWBaseEngine *this)

{
  int local_18 [4];
  
  local_18[0] = 1;
  local_18[1] = 2;
  local_18[2] = 3;
  local_18[3] = 0;
  return local_18[*(uint *)(this + 0x254)] << 0xb;
}
