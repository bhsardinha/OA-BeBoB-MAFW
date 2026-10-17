// m_audio_FWb::ConcreteChannelGroup<10u>::ConcreteChannelGroup
// Entry: 000148d0
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<10u>::ConcreteChannelGroup() */

void __thiscall
m_audio_FWb::ConcreteChannelGroup<10u>::ConcreteChannelGroup(ConcreteChannelGroup<10u> *this)

{
  long lVar1;
  
  *(undefined ***)(this + 0x20) = &PTR__TFunctor_0003c460;
  *(undefined8 *)(this + 0x28) = 0;
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_0003c490;
  *(undefined8 *)(this + 0x38) = 0;
  this[0x12] = (ConcreteChannelGroup<10u>)0x0;
  *(undefined ***)this = &PTR__ConcreteChannelGroup_0003c320;
  lVar1 = 0x48;
  do {
    *(undefined8 *)(this + lVar1) = 0;
    *(undefined8 *)(this + lVar1 + 8) = 0;
    *(undefined8 *)(this + lVar1 + 0x10) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x188);
  this[0x12] = (ConcreteChannelGroup<10u>)0x1;
  *(undefined8 *)(this + 8) = 0;
  this[0x10] = (ConcreteChannelGroup<10u>)0x0;
  this[0x11] = (ConcreteChannelGroup<10u>)0x0;
  return;
}
