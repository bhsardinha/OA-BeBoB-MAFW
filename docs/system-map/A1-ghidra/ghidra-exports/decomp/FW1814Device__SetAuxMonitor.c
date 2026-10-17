// com_m_audio_FW1814Device::SetAuxMonitor
// Entry: 0000cfc4
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (m-e)

/* com_m_audio_FW1814Device::SetAuxMonitor(unsigned int, unsigned int) */

void __thiscall
com_m_audio_FW1814Device::SetAuxMonitor(com_m_audio_FW1814Device *this,uint param_1,uint param_2)

{
  uint uVar1;
  
  uVar1 = (uint)(1L << ((char)param_1 - 1U & 0x3f));
  uVar1 = uVar1 >> 0x18 | (uVar1 & 0xff0000) >> 8 | (uVar1 & 0xff00) << 8 | uVar1 << 0x18;
  if (param_2 == 0) {
    uVar1 = *(uint *)(this + 0x1314) & ~uVar1;
  }
  else {
    uVar1 = uVar1 | *(uint *)(this + 0x1314);
  }
  *(uint *)(this + 0x1314) = uVar1;
                    /* WARNING: Could not recover jumptable at 0x0000d012. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (**(code **)(*(long *)this + 0xba8))(this,0x9c,1,this + 0x1314);
  return;
}
