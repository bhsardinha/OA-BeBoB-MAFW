// com_m_audio_FW1814Device::stop
// Entry: 0000d9ce  Size: 80 bytes
// Prototype: undefined stop(com_m_audio_FW1814Device * this, IOService * param_1)


/* WARNING: Switch with 1 destination removed at 0x0000da18 */
/* WARNING: Switch with 1 destination removed at 0x000136d4 */
/* com_m_audio_FW1814Device::stop(IOService*) */

void __thiscall com_m_audio_FW1814Device::stop(com_m_audio_FW1814Device *this,IOService *param_1)

{
  long *plVar1;
  
  if (*(m_audio_b_FWHSCI **)(this + 0x1270) != (m_audio_b_FWHSCI *)0x0) {
    m_audio_b_FWHSCI::Release(*(m_audio_b_FWHSCI **)(this + 0x1270));
    (**(code **)(**(long **)(this + 0x1270) + 0x28))();
    *(undefined8 *)(this + 0x1270) = 0;
  }
  plVar1 = *(long **)(this + 0x268);
  if (plVar1 != (long *)0x0) {
    (**(code **)(*plVar1 + 0x600))(plVar1,1);
    *(undefined8 *)(this + 0x268) = 0;
  }
  (*(code *)PTR_stop_00031738)(this,param_1);
  return;
}


