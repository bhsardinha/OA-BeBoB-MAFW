// m_audio_b_FWHSCI::HSCIReadComplete
// Entry: 0001d80e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::HSCIReadComplete(int) */

void __thiscall m_audio_b_FWHSCI::HSCIReadComplete(m_audio_b_FWHSCI *this,int param_1)

{
  undefined8 uVar1;
  TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
  *this_00;
  
  if (param_1 != 0) {
                    /* WARNING: Could not recover jumptable at 0x0001d860. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(**(long **)(this + 0x30) + 0x1d0))(*(long **)(this + 0x30),1000);
    return;
  }
  uVar1 = *(undefined8 *)(this + 0xa8);
  this_00 = operator_new(0x20);
  if (this_00 ==
      (TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
       *)0x0) {
    return;
  }
  *(undefined ***)this_00 = &PTR__TFunctor_00046dd0;
  *(undefined8 *)(this_00 + 8) = 0;
  *(m_audio_b_FWHSCI **)(this_00 + 0x18) = this;
  *(undefined8 *)(this_00 + 0x10) = uVar1;
  m_audio_FWb::
  TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
  ::Init<m_audio_b_FWHSCI*,void(m_audio_b_FWHSCI::*)()>(this_00,this,HSCISafelyComplete);
  m_audio_FWb::UKernelCallHelpers::InvokeOnNewThread((CallCenter *)this_00);
  return;
}
