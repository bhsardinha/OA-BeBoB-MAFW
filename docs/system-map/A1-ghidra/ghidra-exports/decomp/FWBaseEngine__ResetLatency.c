// m_audio_b_FWBaseEngine::ResetLatency
// Entry: 00003f7a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWBaseEngine::ResetLatency() */

void __thiscall m_audio_b_FWBaseEngine::ResetLatency(m_audio_b_FWBaseEngine *this)

{
  code *UNRECOVERED_JUMPTABLE;
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  
  UNRECOVERED_JUMPTABLE = *(code **)(*(long *)this + 0xb70);
  iVar1 = (**(code **)(**(long **)(this + 0x138) + 0xab8))
                    (*(long **)(this + 0x138),*(undefined4 *)(this + 0x228));
  (*UNRECOVERED_JUMPTABLE)(this,iVar1 + 1U >> 1);
  UNRECOVERED_JUMPTABLE = *(code **)(*(long *)this + 0xb68);
  uVar2 = (**(code **)(**(long **)(this + 0x138) + 0xab8))
                    (*(long **)(this + 0x138),*(undefined4 *)(this + 0x228));
  (*UNRECOVERED_JUMPTABLE)(this,uVar2 >> 1);
  UNRECOVERED_JUMPTABLE = *(code **)(*(long *)this + 0xb78);
  uVar3 = (**(code **)(**(long **)(this + 0x138) + 0xac0))
                    (*(long **)(this + 0x138),*(undefined4 *)(this + 0x228));
                    /* WARNING: Could not recover jumptable at 0x00004012. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(this,uVar3);
  return;
}
