// TCollectionIterator<m_audio_b_FWTTrio*>::start
// Entry: 0000232e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-j2)

/* TCollectionIterator<m_audio_b_FWTTrio*>::start(OSCollection const*) */

undefined8 __thiscall
TCollectionIterator<m_audio_b_FWTTrio*>::start
          (TCollectionIterator<m_audio_b_FWTTrio*> *this,OSCollection *param_1)

{
  long *plVar1;
  undefined8 uVar2;
  
  if (param_1 == (OSCollection *)0x0) {
    *(undefined8 *)this = 0;
  }
  else {
    plVar1 = (long *)OSCollectionIterator::withCollection(param_1);
    *(long **)this = plVar1;
    if (plVar1 != (long *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x0000235b. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      uVar2 = (**(code **)(*plVar1 + 0x128))(plVar1);
      return uVar2;
    }
  }
  return 0;
}
