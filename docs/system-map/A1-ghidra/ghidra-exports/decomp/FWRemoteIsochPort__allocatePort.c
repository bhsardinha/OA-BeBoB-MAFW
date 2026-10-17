// m_audio_b_FWRemoteIsochPort::allocatePort
// Entry: 000183ee
// Source: Ghidra 12.1.4 headless DecompList.java, 2026-10-15 (M-C)

/* m_audio_b_FWRemoteIsochPort::allocatePort(IOFWSpeed, unsigned int) */

void __thiscall
m_audio_b_FWRemoteIsochPort::allocatePort
          (m_audio_b_FWRemoteIsochPort *this,undefined8 param_2,undefined8 param_3)

{
  code *UNRECOVERED_JUMPTABLE;
  
  UNRECOVERED_JUMPTABLE = *(code **)(**(long **)(this + 0x10) + 0x120);
                    /* WARNING: Could not recover jumptable at 0x00018413. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)
            (*(long **)(this + 0x10),this[0x18] == (m_audio_b_FWRemoteIsochPort)0x0,param_3,
             *(undefined4 *)(this + 0x1c),UNRECOVERED_JUMPTABLE);
  return;
}
