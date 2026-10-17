// com_m_audio_FWEngine2::stop
// Entry: 000117d4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-D)

/* WARNING: Switch with 1 destination removed at 0x000117e0 */
/* com_m_audio_FWEngine2::stop(IOService*) */

void __thiscall com_m_audio_FWEngine2::stop(com_m_audio_FWEngine2 *this,IOService *param_1)

{
  if (this[0x298] != (com_m_audio_FWEngine2)0x0) {
    (**(code **)(*(long *)this + 0xcf8))(this);
  }
  if (*(m_audio_b_FWAudioDevice **)(this + 0x138) != (m_audio_b_FWAudioDevice *)0x0) {
    m_audio_b_FWBaseEngine::SetMIDIInputPorts
              ((m_audio_b_FWBaseEngine *)this,*(m_audio_b_FWAudioDevice **)(this + 0x138),0,
               (char **)0x0);
    m_audio_b_FWBaseEngine::SetMIDIOutputPorts
              ((m_audio_b_FWBaseEngine *)this,*(m_audio_b_FWAudioDevice **)(this + 0x138),0,
               (char **)0x0);
  }
  if (*(int *)(this + 0x22c) != 0) {
    (**(code **)(*(long *)this + 0xc20))(this);
  }
  if (this[0x2d3] != (com_m_audio_FWEngine2)0x0) {
    (**(code **)(*(long *)this + 0xcc0))(this);
  }
  if (*(long **)(this + 0x218) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x218) + 0x28))();
    *(undefined8 *)(this + 0x218) = 0;
  }
  if (*(long **)(this + 0x210) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x210) + 0x28))();
    *(undefined8 *)(this + 0x210) = 0;
  }
  if (*(long **)(this + 0x178) != (long *)0x0) {
    (**(code **)(**(long **)(this + 0x178) + 0x28))();
    *(undefined8 *)(this + 0x178) = 0;
  }
  operator_delete(*(void **)(this + 600));
  *(undefined8 *)(this + 600) = 0;
                    /* WARNING: Could not recover jumptable at 0x00005dce. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(PTR_vtable_000300b0 + 0x5d8))(this,param_1);
  return;
}
