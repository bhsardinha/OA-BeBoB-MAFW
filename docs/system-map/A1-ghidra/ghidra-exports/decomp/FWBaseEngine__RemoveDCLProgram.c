// m_audio_b_FWBaseEngine::RemoveDCLProgram
// Entry: 000044ec
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::RemoveDCLProgram(m_audio_b_FWDCLProgram*) */

undefined8 __thiscall
m_audio_b_FWBaseEngine::RemoveDCLProgram
          (m_audio_b_FWBaseEngine *this,m_audio_b_FWDCLProgram *param_1)

{
  long *plVar1;
  int iVar2;
  undefined8 uVar3;
  
  if (this[0x2d3] == (m_audio_b_FWBaseEngine)0x0) {
    plVar1 = *(long **)(this + 0x178);
    if ((plVar1 != (long *)0x0) &&
       (iVar2 = (**(code **)(*plVar1 + 0x1f8))(plVar1,param_1,0), iVar2 != -1)) {
      (**(code **)(**(long **)(this + 0x178) + 0x1d8))(*(long **)(this + 0x178),iVar2);
      return 0;
    }
    uVar3 = 0xe00002c2;
  }
  else {
    uVar3 = 0xe00002bc;
  }
  return uVar3;
}
