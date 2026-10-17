// m_audio_FWb::TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>::Init<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int)>
// Entry: 00020e00
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* bool m_audio_FWb::TFunctor<int (unsigned int, unsigned int), m_audio_FWb::FunctorNilArgument,
   m_audio_FWb::FunctorNilArgument, m_audio_FWb::FunctorNilArgument>::Init<FWSettingsAuxSend*, int
   (FWSettingsAuxSend::*)(unsigned int, unsigned int)>(FWSettingsAuxSend*, int
   (FWSettingsAuxSend::*)(unsigned int, unsigned int)) */

bool m_audio_FWb::
     TFunctor<int(unsigned_int,unsigned_int),m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
     ::Init<FWSettingsAuxSend*,int(FWSettingsAuxSend::*)(unsigned_int,unsigned_int)>
               (FWSettingsAuxSend *param_1,_func_int_uint_uint *param_2)

{
  undefined8 *puVar1;
  undefined8 in_RCX;
  long in_RDX;
  bool bVar2;
  
  if (*(void **)(param_1 + 8) != (void *)0x0) {
    operator_delete(*(void **)(param_1 + 8));
  }
  if ((param_2 == (_func_int_uint_uint *)0x0) || (in_RDX == 0)) {
    bVar2 = false;
    puVar1 = (undefined8 *)0x0;
  }
  else {
    puVar1 = operator_new(0x20);
    if (puVar1 != (undefined8 *)0x0) {
      *puVar1 = &PTR_Call_00048d20;
      puVar1[1] = param_2;
      puVar1[2] = in_RDX;
      puVar1[3] = in_RCX;
    }
    bVar2 = puVar1 != (undefined8 *)0x0;
  }
  *(undefined8 **)(param_1 + 8) = puVar1;
  return bVar2;
}
