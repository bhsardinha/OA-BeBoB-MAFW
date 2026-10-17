// m_audio_FWb::TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>::Init<m_audio_b_FWHSCI*,void(m_audio_b_FWHSCI::*)()>
// Entry: 0001d796
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* bool m_audio_FWb::TFunctor<void, m_audio_FWb::FunctorNilArgument,
   m_audio_FWb::FunctorNilArgument, m_audio_FWb::FunctorNilArgument>::Init<m_audio_b_FWHSCI*, void
   (m_audio_b_FWHSCI::*)()>(m_audio_b_FWHSCI*, void (m_audio_b_FWHSCI::*)()) */

bool __thiscall
m_audio_FWb::
TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
::Init<m_audio_b_FWHSCI*,void(m_audio_b_FWHSCI::*)()>
          (TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
           *this,m_audio_b_FWHSCI *param_1,_func_void *param_2)

{
  undefined8 *puVar1;
  bool bVar2;
  
  if (*(void **)(this + 8) != (void *)0x0) {
    operator_delete(*(void **)(this + 8));
  }
  if ((param_1 == (m_audio_b_FWHSCI *)0x0) || (param_2 == (_func_void *)0x0)) {
    bVar2 = false;
    puVar1 = (undefined8 *)0x0;
  }
  else {
    puVar1 = operator_new(0x20);
    if (puVar1 != (undefined8 *)0x0) {
      *puVar1 = &PTR_Call_00046e00;
      puVar1[1] = param_1;
      puVar1[2] = param_2;
      puVar1[3] = 0;
    }
    bVar2 = puVar1 != (undefined8 *)0x0;
  }
  *(undefined8 **)(this + 8) = puVar1;
  return bVar2;
}
