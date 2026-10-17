// m_audio_b_FWDCLProgram::CalcIsochPacketHeaders
// Entry: 0002519c  Size: 125 bytes
// Prototype: undefined CalcIsochPacketHeaders(m_audio_b_FWDCLProgram * this, uint * param_1, uint param_2, uint param_3, bool param_4)


/* m_audio_b_FWDCLProgram::CalcIsochPacketHeaders(unsigned int*, unsigned int, unsigned int, bool)
    */

void __thiscall
m_audio_b_FWDCLProgram::CalcIsochPacketHeaders
          (m_audio_b_FWDCLProgram *this,uint *param_1,uint param_2,uint param_3,bool param_4)

{
  uint uVar1;
  
  uVar1 = param_3 | *(uint *)(this + 0x498);
  *param_1 = (uVar1 & 0xff00) * 0x100 + (uVar1 >> 8 & 0xff00) + uVar1 * 0x1000000 + (uVar1 >> 0x18);
  uVar1 = *(uint *)(this + 0x49c);
  if (!param_4) {
    param_1[1] = (uVar1 >> 0x18) - 0x100;
    return;
  }
  param_1[1] = ((*(int *)(this + 0x4a4) << 0x10 | uVar1) >> 8 & 0xff00) +
               ((param_2 | uVar1) & 0xff00) * 0x100 +
               (param_2 | uVar1) * 0x1000000 + (uVar1 >> 0x18);
  return;
}


