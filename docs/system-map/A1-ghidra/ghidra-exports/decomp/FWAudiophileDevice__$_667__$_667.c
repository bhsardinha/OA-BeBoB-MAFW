// com_m_audio_FWAudiophileDevice::$_667::$_667
// Entry: 0000b4ae
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e2)

/* com_m_audio_FWAudiophileDevice::$_667::$_667() */

void __thiscall com_m_audio_FWAudiophileDevice::$_667::__667(__667 *this)

{
  long lVar1;
  
  m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup((ConcreteChannelGroup<6u> *)this);
  *(undefined ***)(this + 0x128) = &PTR__TFunctor_000354d0;
  *(undefined8 *)(this + 0x130) = 0;
  *(undefined ***)(this + 0x138) = &PTR__TFunctor_00035500;
  *(undefined8 *)(this + 0x140) = 0;
  this[0x11a] = (__667)0x0;
  *(undefined ***)(this + 0x108) = &PTR__ConcreteChannelGroup_00035480;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x150) = 0;
    *(undefined8 *)(this + lVar1 + 0x158) = 0;
    *(undefined8 *)(this + lVar1 + 0x160) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x80);
  this[0x11a] = (__667)0x1;
  *(undefined8 *)(this + 0x110) = 0;
  this[0x118] = (__667)0x0;
  this[0x119] = (__667)0x0;
  m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<6u> *)(this + 0x1d0));
  m_audio_FWb::ConcreteChannelGroup<2u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<2u> *)(this + 0x2d8));
  *(undefined ***)(this + 0x380) = &PTR__TFunctor_000354d0;
  *(undefined8 *)(this + 0x388) = 0;
  *(undefined ***)(this + 0x390) = &PTR__TFunctor_00035500;
  *(undefined8 *)(this + 0x398) = 0;
  this[0x372] = (__667)0x0;
  *(undefined ***)(this + 0x360) = &PTR__ConcreteChannelGroup_000353e0;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x3a8) = 0;
    *(undefined8 *)(this + lVar1 + 0x3b0) = 0;
    *(undefined8 *)(this + lVar1 + 0x3b8) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x140);
  this[0x372] = (__667)0x1;
  *(undefined8 *)(this + 0x368) = 0;
  this[0x370] = (__667)0x0;
  this[0x371] = (__667)0x0;
  m_audio_FWb::ConcreteChannelGroup<2u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<2u> *)(this + 0x4e8));
  return;
}
