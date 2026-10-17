// com_m_audio_FWMetaNub::ReregisterDeviceTimerFired
// Entry: 0001bc5e  Size: 99 bytes
// Prototype: undefined ReregisterDeviceTimerFired(com_m_audio_FWMetaNub * this)


/* com_m_audio_FWMetaNub::ReregisterDeviceTimerFired() */

void __thiscall com_m_audio_FWMetaNub::ReregisterDeviceTimerFired(com_m_audio_FWMetaNub *this)

{
  TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
  *this_00;
  
  this_00 = operator_new(0x20);
  if (this_00 ==
      (TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
       *)0x0) {
    return;
  }
  *(undefined ***)this_00 = &PTR__TFunctor_00046320;
  *(undefined8 *)(this_00 + 8) = 0;
  *(com_m_audio_FWMetaNub **)(this_00 + 0x18) = this;
  *(undefined8 *)(this_00 + 0x10) = 0;
  m_audio_FWb::
  TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
  ::Init<com_m_audio_FWMetaNub*,void(com_m_audio_FWMetaNub::*)()>
            (this_00,this,RegregisterDeviceThread);
  m_audio_FWb::UKernelCallHelpers::InvokeOnNewThread((CallCenter *)this_00);
  return;
}


