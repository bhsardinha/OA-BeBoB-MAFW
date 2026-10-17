// m_audio_b_FWHSCI::Start
// Entry: 0001d94e
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::Start(IOWorkLoop*) */

undefined1 __thiscall m_audio_b_FWHSCI::Start(m_audio_b_FWHSCI *this,IOWorkLoop *param_1)

{
  int iVar1;
  bool bVar2;
  char cVar3;
  void *pvVar4;
  void *pvVar5;
  
  iVar1 = *(int *)(this + 0x18);
  *(int *)(this + 0x18) = iVar1 + 1;
  if (iVar1 != 0) {
    return 1;
  }
  if (this[0x48] == (m_audio_b_FWHSCI)0x0) {
    pvVar4 = operator_new__((ulong)(*(uint *)(this + 0xc0) >> 2) << 2);
    *(void **)(this + 200) = pvVar4;
    pvVar5 = (void *)0x0;
    if (pvVar4 == (void *)0x0) {
LAB_0001da0d:
      if (pvVar5 == (void *)0x0) {
        return 0;
      }
      operator_delete(pvVar5);
      *(undefined8 *)(this + 200) = 0;
      return 0;
    }
    *(IOWorkLoop **)(this + 0xa8) = param_1;
    if (this[0x48] == (m_audio_b_FWHSCI)0x0) {
      bVar2 = m_audio_FWb::
              TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
              ::Init<m_audio_b_FWHSCI*,void(m_audio_b_FWHSCI::*)()>
                        ((TFunctor<void,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument,m_audio_FWb::FunctorNilArgument>
                          *)(this + 0x50),this,HSCITimerFired);
      if ((!bVar2) ||
         (cVar3 = m_audio_b_FWKernelTimer::Initialize
                            ((m_audio_b_FWKernelTimer *)(this + 0x20),param_1,
                             (TFunctor *)(this + 0x50)), cVar3 == '\0')) {
        pvVar5 = *(void **)(this + 200);
        goto LAB_0001da0d;
      }
    }
  }
  if (*(int *)(this + 0x1c) == 0) {
    (**(code **)(**(long **)(this + 0x30) + 0x150))();
    (**(code **)(**(long **)(this + 0x30) + 0x1d0))(*(long **)(this + 0x30),0x32);
  }
  return 1;
}
