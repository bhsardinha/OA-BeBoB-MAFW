// m_audio_b_FWDiscreteEngine::GetStatusDescriptor
// Entry: 00018e5c
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* m_audio_b_FWDiscreteEngine::GetStatusDescriptor() */

long __thiscall m_audio_b_FWDiscreteEngine::GetStatusDescriptor(m_audio_b_FWDiscreteEngine *this)

{
  void *pvVar1;
  long *plVar2;
  void *pvVar3;
  undefined8 uVar4;
  long lVar5;
  
  lVar5 = *(long *)(this + 0x120);
  if (lVar5 == 0) {
    if (*(long *)(this + 0x110) == 0) {
      plVar2 = (long *)IOBufferMemoryDescriptor::withOptions
                                 (0x10003,0x1000,*(ulong *)PTR__page_size_000300f0);
      *(long **)(this + 0x120) = plVar2;
      lVar5 = 0;
      if (plVar2 != (long *)0x0) {
        pvVar1 = *(void **)(this + 0xb0);
        pvVar3 = (void *)(**(code **)(*plVar2 + 0x2e0))(plVar2);
        _memcpy(pvVar3,pvVar1,0x18);
        _IOFreeAligned(*(undefined8 *)(this + 0xb0),0x1000);
        uVar4 = (**(code **)(**(long **)(this + 0x120) + 0x2e0))();
        *(undefined8 *)(this + 0xb0) = uVar4;
        lVar5 = *(long *)(this + 0x120);
      }
    }
    else {
      lVar5 = *(long *)(*(long *)(this + 0x110) + 8);
      *(long *)(this + 0x120) = lVar5;
    }
  }
  return lVar5;
}
