// m_audio_b_FWKIPCUserClient::AddModifiedProperties
// Entry: 00015e04
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::AddModifiedProperties(OSDictionary*) */

void __thiscall
m_audio_b_FWKIPCUserClient::AddModifiedProperties
          (m_audio_b_FWKIPCUserClient *this,OSDictionary *param_1)

{
  code *pcVar1;
  uint uVar2;
  long lVar3;
  long *plVar4;
  undefined8 uVar5;
  
  if (*(long *)(this + 0x110) == 0) {
    uVar2 = (**(code **)(*(long *)param_1 + 0x130))(param_1);
    lVar3 = OSDictionary::withCapacity(uVar2);
    *(long *)(this + 0x110) = lVar3;
    if (lVar3 == 0) {
      return;
    }
    lVar3 = _IORecursiveLockAlloc();
    *(long *)(this + 0x120) = lVar3;
    if (lVar3 == 0) {
      return;
    }
  }
  _OSIncrementAtomic(this + 0x118);
  _IORecursiveLockLock(*(undefined8 *)(this + 0x120));
  if ((param_1 == (OSDictionary *)0x0) ||
     (plVar4 = (long *)OSCollectionIterator::withCollection((OSCollection *)param_1),
     plVar4 == (long *)0x0)) {
    _IORecursiveLockUnlock(*(undefined8 *)(this + 0x120));
    return;
  }
  lVar3 = (**(code **)(*plVar4 + 0x128))(plVar4);
  while (lVar3 != 0) {
    pcVar1 = *(code **)(**(long **)(this + 0x110) + 0x1c0);
    uVar5 = (**(code **)(*(long *)param_1 + 0x1f8))(param_1,lVar3);
    (*pcVar1)(*(undefined8 *)(this + 0x110),lVar3,uVar5);
    lVar3 = (**(code **)(*plVar4 + 0x128))(plVar4);
  }
  _IORecursiveLockUnlock(*(undefined8 *)(this + 0x120));
  if (plVar4 == (long *)0x0) {
    return;
  }
                    /* WARNING: Could not recover jumptable at 0x00015f36. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*plVar4 + 0x28))(plVar4);
  return;
}
