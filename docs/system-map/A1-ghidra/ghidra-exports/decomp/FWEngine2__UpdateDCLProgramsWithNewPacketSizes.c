// com_m_audio_FWEngine2::UpdateDCLProgramsWithNewPacketSizes
// Entry: 000122d6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* com_m_audio_FWEngine2::UpdateDCLProgramsWithNewPacketSizes() */

bool __thiscall
com_m_audio_FWEngine2::UpdateDCLProgramsWithNewPacketSizes(com_m_audio_FWEngine2 *this)

{
  int iVar1;
  
  this[0x347] = (com_m_audio_FWEngine2)((byte)this[0x347] | 1);
  iVar1 = m_audio_b_FWDCLProgram::SetPacketParameters
                    (*(m_audio_b_FWDCLProgram **)(this + 0x350),*(uint *)(this + 0x220),
                     *(uint *)(this + 0x2e0),*(uint *)(this + 0x2e8),(SYTInterval *)0x0,
                     (FDFValue *)0x0);
  if ((iVar1 == 0) &&
     ((*(uint *)(this + 0x8c) < 0x17701 || (this[0x2d6] != (com_m_audio_FWEngine2)0x0)))) {
    iVar1 = m_audio_b_FWDCLProgram::SetPacketParameters
                      (*(m_audio_b_FWDCLProgram **)(this + 0x358),*(uint *)(this + 0x220),
                       *(uint *)(this + 0x2e4),*(uint *)(this + 0x2e8),(SYTInterval *)0x0,
                       (FDFValue *)0x0);
  }
  return iVar1 == 0;
}
