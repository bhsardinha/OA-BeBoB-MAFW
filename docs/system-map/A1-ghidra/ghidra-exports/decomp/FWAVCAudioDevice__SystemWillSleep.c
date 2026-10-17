// m_audio_b_FWAVCAudioDevice::SystemWillSleep
// Entry: 00022b16
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e2b)

/* m_audio_b_FWAVCAudioDevice::SystemWillSleep() */

void __thiscall m_audio_b_FWAVCAudioDevice::SystemWillSleep(m_audio_b_FWAVCAudioDevice *this)

{
  long *plVar1;
  
  (**(code **)(*(long *)this + 0xb58))(this);
  if (*(long *)(this + 0xe8) != 0) {
    (**(code **)(*(long *)this + 0xb88))(this);
    (**(code **)(**(long **)(this + 0xe8) + 0xa50))();
    plVar1 = *(long **)(this + 0xe8);
    if ((char)plVar1[0x5a] != '\0') {
      (**(code **)(*plVar1 + 0xce8))(plVar1,0);
      plVar1 = *(long **)(this + 0xe8);
    }
                    /* WARNING: Could not recover jumptable at 0x00022b7f. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*plVar1 + 0xcc0))();
    return;
  }
  return;
}
