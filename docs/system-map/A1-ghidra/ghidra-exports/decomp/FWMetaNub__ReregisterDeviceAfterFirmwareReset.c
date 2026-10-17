// com_m_audio_FWMetaNub::ReregisterDeviceAfterFirmwareReset
// Entry: 0001bba6  Size: 183 bytes
// Prototype: undefined ReregisterDeviceAfterFirmwareReset(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::ReregisterDeviceAfterFirmwareReset() */

undefined8 __thiscall
com_m_audio_FWMetaNub::ReregisterDeviceAfterFirmwareReset(com_m_audio_FWMetaNub *this)

{
  bool bVar1;
  char cVar2;
  IOWorkLoop *pIVar3;
  
  *(undefined4 *)(this + 0x88) = 2;
  (**(code **)(*(long *)this + 0x20))(this);
  pIVar3 = (IOWorkLoop *)(**(code **)(*(long *)this + 0x688))(this);
  if (this[0x120] == (com_m_audio_FWMetaNub)0x0) {
    bVar1 = m_audio_FWb::
            TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
            ::Init<com_m_audio_FWMetaNub*,void(com_m_audio_FWMetaNub::*)()>
                      ((TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
                        *)(this + 0x128),this,ReregisterDeviceTimerFired);
    if (!bVar1) {
      return 0;
    }
    cVar2 = m_audio_b_FWKernelTimer::Initialize
                      ((m_audio_b_FWKernelTimer *)(this + 0xf8),pIVar3,(TFunctor *)(this + 0x128));
    if (cVar2 == '\0') {
      return 0;
    }
  }
  cVar2 = (**(code **)(**(long **)(this + 0x108) + 0x160))();
  if (cVar2 == '\0') {
    (**(code **)(**(long **)(this + 0x108) + 0x150))();
  }
  (**(code **)(**(long **)(this + 0x108) + 0x1d0))(*(long **)(this + 0x108),0x9c4);
  return 0;
}


