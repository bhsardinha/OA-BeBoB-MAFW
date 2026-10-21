// com_m_audio_FWMetaNub::com_m_audio_FWMetaNub
// Entry: 0001d2d6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FWMetaNub::com_m_audio_FWMetaNub() */

void __thiscall com_m_audio_FWMetaNub::com_m_audio_FWMetaNub(com_m_audio_FWMetaNub *this)

{
  IOService::IOService((IOService *)this,(OSMetaClass *)&gMetaClass);
  *(undefined ***)this = &PTR__com_m_audio_FWMetaNub_00046520;
  m_audio_b_FWKernelTimer::m_audio_b_FWKernelTimer((m_audio_b_FWKernelTimer *)(this + 0xf8));
  OSMetaClass::instanceConstructed();
  return;
}
