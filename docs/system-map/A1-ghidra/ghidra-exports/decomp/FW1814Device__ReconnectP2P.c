// com_m_audio_FW1814Device::ReconnectP2P
// Entry: 0000e1ea
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::ReconnectP2P() */

void __thiscall com_m_audio_FW1814Device::ReconnectP2P(com_m_audio_FW1814Device *this)

{
  (**(code **)(**(long **)(this + 0xe8) + 0xcc0))();
  (**(code **)(**(long **)(this + 0xe8) + 0xcb8))();
  _IOSleep(0x44c);
  if ((*(long *)(this + 0xe8) != 0) && (*(char *)(*(long *)(this + 0xe8) + 0x2d0) == '\0')) {
                    /* WARNING: Could not recover jumptable at 0x0000e25a. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*(long *)this + 0xb60))(this,*(undefined4 *)(this + 0x144),1);
    return;
  }
  return;
}
