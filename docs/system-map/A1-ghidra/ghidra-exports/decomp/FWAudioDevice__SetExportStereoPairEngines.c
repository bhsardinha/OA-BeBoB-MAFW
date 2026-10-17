// m_audio_b_FWAudioDevice::SetExportStereoPairEngines
// Entry: 00001dd6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e3)

/* m_audio_b_FWAudioDevice::SetExportStereoPairEngines(unsigned int) */

undefined8 __thiscall
m_audio_b_FWAudioDevice::SetExportStereoPairEngines(m_audio_b_FWAudioDevice *this,uint param_1)

{
  long *plVar1;
  
  plVar1 = *(long **)(this + 0xe8);
  if (plVar1 != (long *)0x0) {
    if (param_1 == 0) {
      (**(code **)(*plVar1 + 0xc48))(plVar1);
    }
    else {
      (**(code **)(*(long *)this + 0xa40))();
    }
  }
  return 0;
}
