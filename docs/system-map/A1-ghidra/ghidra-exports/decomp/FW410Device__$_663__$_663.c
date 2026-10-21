// com_m_audio_FW410Device::$_663::$_663
// Entry: 000149c6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* com_m_audio_FW410Device::$_663::$_663() */

void __thiscall com_m_audio_FW410Device::$_663::__663(__663 *this)

{
  long lVar1;
  
  m_audio_FWb::ConcreteChannelGroup<10u>::ConcreteChannelGroup((ConcreteChannelGroup<10u> *)this);
  *(undefined ***)(this + 0x1a8) = &PTR__TFunctor_0003c460;
  *(undefined8 *)(this + 0x1b0) = 0;
  *(undefined ***)(this + 0x1b8) = &PTR__TFunctor_0003c490;
  *(undefined8 *)(this + 0x1c0) = 0;
  this[0x19a] = (__663)0x0;
  *(undefined ***)(this + 0x188) = &PTR__ConcreteChannelGroup_0003c370;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x1d0) = 0;
    *(undefined8 *)(this + lVar1 + 0x1d8) = 0;
    *(undefined8 *)(this + lVar1 + 0x1e0) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x80);
  this[0x19a] = (__663)0x1;
  *(undefined8 *)(this + 400) = 0;
  this[0x198] = (__663)0x0;
  this[0x199] = (__663)0x0;
  m_audio_FWb::ConcreteChannelGroup<10u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<10u> *)(this + 0x250));
  m_audio_FWb::ConcreteChannelGroup<2u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<2u> *)(this + 0x3d8));
  *(undefined ***)(this + 0x480) = &PTR__TFunctor_0003c460;
  *(undefined8 *)(this + 0x488) = 0;
  *(undefined ***)(this + 0x490) = &PTR__TFunctor_0003c490;
  *(undefined8 *)(this + 0x498) = 0;
  this[0x472] = (__663)0x0;
  *(undefined ***)(this + 0x460) = &PTR__ConcreteChannelGroup_0003c2d0;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x4a8) = 0;
    *(undefined8 *)(this + lVar1 + 0x4b0) = 0;
    *(undefined8 *)(this + lVar1 + 0x4b8) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x1c0);
  this[0x472] = (__663)0x1;
  *(undefined8 *)(this + 0x468) = 0;
  this[0x470] = (__663)0x0;
  this[0x471] = (__663)0x0;
  m_audio_FWb::ConcreteChannelGroup<2u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<2u> *)(this + 0x668));
  return;
}
