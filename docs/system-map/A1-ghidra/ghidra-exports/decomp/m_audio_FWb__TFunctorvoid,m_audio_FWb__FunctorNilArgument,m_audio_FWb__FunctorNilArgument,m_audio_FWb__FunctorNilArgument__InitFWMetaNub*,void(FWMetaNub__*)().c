// m_audio_FWb::TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>::Init<com_m_audio_FWMetaNub*,void(com_m_audio_FWMetaNub::*)()>
// Entry: 0001bb2e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-19 (ap0)

/* bool m_audio_FWb::TFunctor<void, m_audio_FWb::FunctorNilArgument,
   m_audio_FWb::FunctorNilArgument, m_audio_FWb::FunctorNilArgument>::Init<com_m_audio_FWMetaNub*,
   void (com_m_audio_FWMetaNub::*)()>(com_m_audio_FWMetaNub*, void (com_m_audio_FWMetaNub::*)()) */

bool __thiscall
m_audio_FWb::
TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
::Init<com_m_audio_FWMetaNub*,void(com_m_audio_FWMetaNub::*)()>
          (TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
           *this,com_m_audio_FWMetaNub *param_1,_func_void *param_2)

{
  undefined8 *puVar1;
  bool bVar2;
  
  if (*(void **)(this + 8) != (void *)0x0) {
    operator_delete(*(void **)(this + 8));
  }
  if ((param_1 == (com_m_audio_FWMetaNub *)0x0) || (param_2 == (_func_void *)0x0)) {
    bVar2 = false;
    puVar1 = (undefined8 *)0x0;
  }
  else {
    puVar1 = operator_new(0x20);
    if (puVar1 != (undefined8 *)0x0) {
      *puVar1 = &PTR_Call_00046350;
      puVar1[1] = param_1;
      puVar1[2] = param_2;
      puVar1[3] = 0;
    }
    bVar2 = puVar1 != (undefined8 *)0x0;
  }
  *(undefined8 **)(this + 8) = puVar1;
  return bVar2;
}
