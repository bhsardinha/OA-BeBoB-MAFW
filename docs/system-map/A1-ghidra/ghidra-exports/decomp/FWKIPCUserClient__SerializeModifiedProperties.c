// m_audio_b_FWKIPCUserClient::SerializeModifiedProperties
// Entry: 00015cba
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-f)

/* m_audio_b_FWKIPCUserClient::SerializeModifiedProperties() */

undefined8 __thiscall
m_audio_b_FWKIPCUserClient::SerializeModifiedProperties(m_audio_b_FWKIPCUserClient *this)

{
  char cVar1;
  uint uVar2;
  long lVar3;
  long *plVar4;
  void *pvVar5;
  ulong uVar6;
  
  if ((*(long *)(this + 0xe8) == 0) || (*(long *)(this + 0x110) == 0)) goto LAB_00015df1;
  if (*(long **)(this + 0x130) == (long *)0x0) {
    lVar3 = OSSerialize::withCapacity(0x200);
    *(long *)(this + 0x130) = lVar3;
    if (lVar3 == 0) goto LAB_00015df1;
  }
  else {
    (**(code **)(**(long **)(this + 0x130) + 0x120))();
  }
  _IORecursiveLockLock(*(undefined8 *)(this + 0x120));
  cVar1 = (**(code **)(**(long **)(this + 0x110) + 0x30))
                    (*(long **)(this + 0x110),*(undefined8 *)(this + 0x130));
  (**(code **)(**(long **)(this + 0x110) + 0x158))();
  _IORecursiveLockUnlock(*(undefined8 *)(this + 0x120));
  if (cVar1 == '\0') goto LAB_00015df1;
  uVar2 = (**(code **)(**(long **)(this + 0x130) + 0x158))();
  uVar6 = (ulong)uVar2;
  plVar4 = (long *)0x0;
  if (*(long **)(this + 0x128) == (long *)0x0) {
LAB_00015d99:
    if (plVar4 != (long *)0x0) {
      (**(code **)(*plVar4 + 0x28))();
    }
    plVar4 = (long *)IOBufferMemoryDescriptor::withCapacity(uVar6 + 0x2c,1,false);
    *(long **)(this + 0x128) = plVar4;
  }
  else {
    lVar3 = (**(code **)(**(long **)(this + 0x128) + 0x2d8))();
    plVar4 = *(long **)(this + 0x128);
    if (lVar3 != uVar6 + 0x2c) goto LAB_00015d99;
  }
  if (plVar4 != (long *)0x0) {
    lVar3 = (**(code **)(*plVar4 + 0x2e0))();
    pvVar5 = (void *)(**(code **)(**(long **)(this + 0x130) + 0x118))();
    _memcpy((void *)(lVar3 + 0x2c),pvVar5,uVar6);
  }
LAB_00015df1:
  return *(undefined8 *)(this + 0x128);
}
