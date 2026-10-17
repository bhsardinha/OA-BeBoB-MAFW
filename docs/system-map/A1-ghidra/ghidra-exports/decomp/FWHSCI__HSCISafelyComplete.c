// m_audio_b_FWHSCI::HSCISafelyComplete
// Entry: 0001d4d6
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e4)

/* m_audio_b_FWHSCI::HSCISafelyComplete() */

void __thiscall m_audio_b_FWHSCI::HSCISafelyComplete(m_audio_b_FWHSCI *this)

{
  *(undefined4 *)(this + 0xd4) = 0;
  (**(code **)**(undefined8 **)(this + 0xb8))
            (*(undefined8 **)(this + 0xb8),*(undefined8 *)(this + 200));
                    /* WARNING: Could not recover jumptable at 0x0001d51c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(**(long **)(this + 0x30) + 0x1d0))
            (*(long **)(this + 0x30),*(int *)(this + 0xd4) + 0x32);
  return;
}
