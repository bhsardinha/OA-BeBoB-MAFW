// m_audio_b_FWHSCI::m_audio_b_FWHSCI
// Entry: 0001dc02
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::m_audio_b_FWHSCI() */

void __thiscall m_audio_b_FWHSCI::m_audio_b_FWHSCI(m_audio_b_FWHSCI *this)

{
  OSObject::OSObject((OSObject *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__m_audio_b_FWHSCI_00046f10;
  m_audio_b_FWKernelTimer::m_audio_b_FWKernelTimer((m_audio_b_FWKernelTimer *)(this + 0x20));
  m_audio_b_FWKernelTimer::m_audio_b_FWKernelTimer((m_audio_b_FWKernelTimer *)(this + 0x60));
  *(undefined ***)(this + 0xb0) = &PTR__TFunctor_00046da0;
  *(undefined8 *)(this + 0xb8) = 0;
  OSMetaClass::instanceConstructed();
  return;
}
