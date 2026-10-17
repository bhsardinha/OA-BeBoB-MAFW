// m_audio_b_FWDCLOutputProgram::Start
// Entry: 00026900
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (Q-START)

/* m_audio_b_FWDCLOutputProgram::Start() */

void __thiscall m_audio_b_FWDCLOutputProgram::Start(m_audio_b_FWDCLOutputProgram *this)

{
  uint uVar1;
  int iVar2;
  long lVar3;
  
  *(undefined4 *)(this + 0x520) = 3;
  *(undefined8 *)(this + 0x530) = 0;
  *(undefined8 *)(this + 0x528) = *(undefined8 *)(*(long *)(this + 0x80) + 0x80);
  *(undefined4 *)(this + 0x538) = 0;
  *(undefined8 *)(this + 0x548) = 0;
  *(undefined8 *)(this + 0x540) = 0;
  *(undefined4 *)(this + 0x550) = 0;
  *(undefined4 *)(*(long *)(this + 0x80) + 0x14) = 0;
  *(undefined8 *)(this + 0x518) = 0;
  *(undefined8 *)(this + 0x508) = 0;
  *(undefined4 *)(this + 0x510) = 0;
  if (*(int *)(*(long *)(this + 0x80) + 0x60) != 0) {
    uVar1 = 0;
    do {
      lVar3 = (ulong)uVar1 * 0x10;
      *(undefined8 *)(this + lVar3 + 0x558) = 0;
      *(undefined8 *)(this + lVar3 + 0x560) = 0;
      *(undefined8 *)(this + (ulong)uVar1 * 8 + 0x5d8) = 0;
      uVar1 = uVar1 + 1;
    } while (uVar1 < *(uint *)(*(long *)(this + 0x80) + 0x60));
  }
  lVar3 = 0;
  do {
    OutputDCLCallback(this,lVar3,true);
    lVar3 = lVar3 + 1;
  } while (lVar3 != 4);
  iVar2 = m_audio_b_FWDCLProgram::Start((m_audio_b_FWDCLProgram *)this);
  if (iVar2 == 0) {
    **(int **)(this + 0x80) = **(int **)(this + 0x80) + 1;
  }
  return;
}
