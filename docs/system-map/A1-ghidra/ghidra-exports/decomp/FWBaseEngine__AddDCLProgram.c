// m_audio_b_FWBaseEngine::AddDCLProgram
// Entry: 00004fcc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::AddDCLProgram(m_audio_b_FWDCLProgram*) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::AddDCLProgram(m_audio_b_FWBaseEngine *this,m_audio_b_FWDCLProgram *param_1)

{
  undefined8 uVar1;
  long *plVar2;
  
  if (this[0x2d3] == (m_audio_b_FWBaseEngine)0x0) {
    plVar2 = *(long **)(this + 0x178);
    if (plVar2 == (long *)0x0) {
      plVar2 = (long *)OSArray::withCapacity(8);
      *(long **)(this + 0x178) = plVar2;
      if (plVar2 == (long *)0x0) {
        return 0xe00002bd;
      }
    }
    (**(code **)(*plVar2 + 0x1b8))(plVar2,param_1);
    uVar1 = 0;
  }
  else {
    uVar1 = 0xe00002bc;
  }
  return uVar1;
}
