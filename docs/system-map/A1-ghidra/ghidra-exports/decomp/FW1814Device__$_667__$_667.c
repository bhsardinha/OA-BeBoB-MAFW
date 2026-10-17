// com_m_audio_FW1814Device::$_667::$_667
// Entry: 0000d442
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e2)

/* com_m_audio_FW1814Device::$_667::$_667() */

void __thiscall com_m_audio_FW1814Device::$_667::__667(__667 *this)

{
  long lVar1;
  
  m_audio_FWb::ConcreteChannelGroup<4u>::ConcreteChannelGroup((ConcreteChannelGroup<4u> *)this);
  *(undefined ***)(this + 0xe8) = &PTR__TFunctor_00036750;
  *(undefined8 *)(this + 0xf0) = 0;
  *(undefined ***)(this + 0xf8) = &PTR__TFunctor_00036780;
  *(undefined8 *)(this + 0x100) = 0;
  this[0xda] = (__667)0x0;
  *(undefined ***)(this + 200) = &PTR__ConcreteChannelGroup_00036380;
  lVar1 = 0x110;
  do {
    *(undefined8 *)(this + lVar1) = 0;
    *(undefined8 *)(this + lVar1 + 8) = 0;
    *(undefined8 *)(this + lVar1 + 0x10) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x350);
  this[0xda] = (__667)0x1;
  *(undefined8 *)(this + 0xd0) = 0;
  this[0xd8] = (__667)0x0;
  this[0xd9] = (__667)0x0;
  m_audio_FWb::ConcreteChannelGroup<4u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<4u> *)(this + 0x350));
  m_audio_FWb::ConcreteChannelGroup<4u>::ConcreteChannelGroup
            ((ConcreteChannelGroup<4u> *)(this + 0x418));
  *(undefined ***)(this + 0x500) = &PTR__TFunctor_00036750;
  *(undefined8 *)(this + 0x508) = 0;
  *(undefined ***)(this + 0x510) = &PTR__TFunctor_00036780;
  *(undefined8 *)(this + 0x518) = 0;
  this[0x4f2] = (__667)0x0;
  *(undefined ***)(this + 0x4e0) = &PTR__ConcreteChannelGroup_000362e0;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x528) = 0;
    *(undefined8 *)(this + lVar1 + 0x530) = 0;
    *(undefined8 *)(this + lVar1 + 0x538) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x2c0);
  this[0x4f2] = (__667)0x1;
  *(undefined8 *)(this + 0x4e8) = 0;
  this[0x4f0] = (__667)0x0;
  this[0x4f1] = (__667)0x0;
  *(undefined ***)(this + 0x808) = &PTR__TFunctor_00036750;
  *(undefined8 *)(this + 0x810) = 0;
  *(undefined ***)(this + 0x818) = &PTR__TFunctor_00036780;
  *(undefined8 *)(this + 0x820) = 0;
  this[0x7fa] = (__667)0x0;
  *(undefined ***)(this + 0x7e8) = &PTR__ConcreteChannelGroup_00036290;
  lVar1 = 0;
  do {
    *(undefined8 *)(this + lVar1 + 0x830) = 0;
    *(undefined8 *)(this + lVar1 + 0x838) = 0;
    *(undefined8 *)(this + lVar1 + 0x840) = 0;
    lVar1 = lVar1 + 0x20;
  } while (lVar1 != 0x40);
  this[0x7fa] = (__667)0x1;
  *(undefined8 *)(this + 0x7f0) = 0;
  this[0x7f8] = (__667)0x0;
  this[0x7f9] = (__667)0x0;
  return;
}
