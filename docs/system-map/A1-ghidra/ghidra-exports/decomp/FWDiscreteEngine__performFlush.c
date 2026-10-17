// m_audio_b_FWDiscreteEngine::performFlush
// Entry: 00018bbc
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* m_audio_b_FWDiscreteEngine::performFlush() */

void __thiscall m_audio_b_FWDiscreteEngine::performFlush(m_audio_b_FWDiscreteEngine *this)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  
  if (*(int *)(this + 0x100) == 0) {
    iVar2 = (**(code **)(*(long *)this + 0xa70))(this);
    if (iVar2 == 1) {
      if (*(uint *)(this + 0x128) < 2) {
        uVar1 = *(uint *)(*(long *)(this + 0xb0) + 4);
        uVar3 = (**(code **)(*(long *)this + 0xa38))(this);
        if ((*(uint *)(this + 0xa0) < uVar1) ||
           ((uVar1 == *(uint *)(this + 0xa0) && (*(uint *)(this + 0x9c) < uVar3)))) {
                    /* WARNING: Could not recover jumptable at 0x00018c24. Too many branches */
                    /* WARNING: Treating indirect jump as call */
          (**(code **)(*(long *)this + 0xa48))(this);
          return;
        }
      }
      else if ((*(uint *)(this + 0x128) & 1) != 0) {
        DeactivateSubEngine(this,1);
        return;
      }
    }
  }
  return;
}
