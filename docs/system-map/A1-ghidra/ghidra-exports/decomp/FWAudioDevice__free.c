// m_audio_b_FWAudioDevice::free
// Entry: 00002372
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e5)

/* m_audio_b_FWAudioDevice::free() */

void __thiscall m_audio_b_FWAudioDevice::free(m_audio_b_FWAudioDevice *this)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0x100);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x1f8))(plVar1,0);
    (**(code **)(**(long **)(this + 0x100) + 0x28))();
    *(undefined8 *)(this + 0x100) = 0;
  }
  operator_delete(*(void **)(this + 0x110));
  (*(code *)PTR_free_00047320)(this);
  return;
}
