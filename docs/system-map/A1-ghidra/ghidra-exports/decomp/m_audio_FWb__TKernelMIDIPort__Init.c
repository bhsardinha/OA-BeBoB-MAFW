// m_audio_FWb::TKernelMIDIPort::Init
// Entry: 0001e64a
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-h)

/* m_audio_FWb::TKernelMIDIPort::Init() */

undefined8 __thiscall m_audio_FWb::TKernelMIDIPort::Init(TKernelMIDIPort *this)

{
  long *plVar1;
  undefined4 *puVar2;
  long lVar3;
  
  plVar1 = (long *)IOBufferMemoryDescriptor::withOptions
                             (0x10003,0x2000,*(ulong *)PTR__page_size_000300f0);
  *(long **)(this + 0x20) = plVar1;
  if (plVar1 != (long *)0x0) {
    puVar2 = (undefined4 *)(**(code **)(*plVar1 + 0x2e0))(plVar1);
    *(undefined4 **)this = puVar2;
    *puVar2 = 0;
    puVar2 = (undefined4 *)(**(code **)(**(long **)(this + 0x20) + 0x2e0))();
    *(undefined4 **)(this + 0x10) = puVar2;
    *puVar2 = 0;
    *(undefined4 *)(*(long *)(this + 0x10) + 4) = 0;
    lVar3 = (**(code **)(**(long **)(this + 0x20) + 0x2e0))();
    *(long *)(this + 0x18) = lVar3 + 0x808;
    *(undefined4 *)(lVar3 + 0x808) = 0;
    *(undefined4 *)(*(long *)(this + 0x18) + 4) = 0;
  }
  return 1;
}
