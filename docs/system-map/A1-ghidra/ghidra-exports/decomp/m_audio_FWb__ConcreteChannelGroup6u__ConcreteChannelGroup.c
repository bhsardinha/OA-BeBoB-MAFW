// m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup
// Entry: 0000b3b4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup() */

void __thiscall
m_audio_FWb::ConcreteChannelGroup<6u>::ConcreteChannelGroup(ConcreteChannelGroup<6u> *this)

{
  long lVar1;
  
  *(undefined ***)(this + 0x20) = &PTR__TFunctor_000354d0;
  *(undefined8 *)(this + 0x28) = 0;
  *(undefined ***)(this + 0x30) = &PTR__TFunctor_00035500;
  *(undefined8 *)(this + 0x38) = 0;
  this[0x12] = (ConcreteChannelGroup<6u>)0x0;
  *(undefined ***)this = &PTR__ConcreteChannelGroup_00035430;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x48) = 0;
    *(undefined8 *)(this + lVar1 + 0x50) = 0;
    *(undefined8 *)(this + lVar1 + 0x58) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0xc0);
  this[0x12] = (ConcreteChannelGroup<6u>)0x1;
  *(undefined8 *)(this + 8) = 0;
  this[0x10] = (ConcreteChannelGroup<6u>)0x0;
  this[0x11] = (ConcreteChannelGroup<6u>)0x0;
  return;
}
